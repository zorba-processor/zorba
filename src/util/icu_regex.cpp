/*
 * Copyright 2006-2016 zorba.io
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 * http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "stdafx.h"

#include <zorba/config.h>

#ifndef ZORBA_NO_ICU

#include <cstring>
#include <vector>

#include <zorba/diagnostic_list.h>
#include <zorba/internal/cxx_util.h>

#include "diagnostics/assert.h"
#include "diagnostics/dict.h"
#include "diagnostics/xquery_exception.h"

#include "ascii_util.h"
#include "regex.h"
#include "stl_util.h"
#include "utf8_util.h"
#include "zorba_regex_engine.h"

#define INVALID_RE_EXCEPTION(...) \
  XQUERY_EXCEPTION( err::FORX0002, ERROR_PARAMS( __VA_ARGS__ ) )

#include <unicode/utf16.h>
#include <unicode/uversion.h>
U_NAMESPACE_USE

#ifndef U_ICU_VERSION_MAJOR_NUM
# error "U_ICU_VERSION_MAJOR_NUM not defined"
#elif U_ICU_VERSION_MAJOR_NUM < 4
    //
    // UREGEX_LITERAL is only in ICU since 4.0.  For earlier versions, we
    // define it ourselves.  Of course it won't have any effect since it's not
    // implemented in ICU, but we implement it ourselves anyway since, even
    // though the constant is defined in 4.0, it's not actually implemented as
    // of 4.4.
    //
# define UREGEX_LITERAL 16
#endif /* U_ICU_VERSION_MAJOR_NUM */

using namespace std;

#define bs_c "\\p{L}_\\d.:\\p{M}-"      /* \c equivalent contents */
#define bs_i "\\p{L}_:"                 /* \i equivalent contents */
#define bs_W "\\p{P}\\p{Z}\\p{C}"       /* \W equivalent contents */

/**
 * Decremements an integer, but no lower that a certain limit (usually zero).
 *
 * @tparam IntegralType The integral type.
 * @param i A pointer to the integer to decrement.
 * @param limit The limit not to go lower than.
 * @return Returns \c true only when the limit has been reached for the first
 * time.
 */
template<typename IntegralType> inline
typename std::enable_if<ZORBA_TR1_NS::is_integral<IntegralType>::value,
                        bool>::type
dec_limit( IntegralType *i, IntegralType limit = 0 ) {
  return *i > limit && --*i == limit;
}

static unsigned digits( long n ) {
  unsigned d = 0;
  do {
    ++d;
  } while ( n /= 10 );
  return d;
}

namespace zorba {

///////////////////////////////////////////////////////////////////////////////

typedef uint32_t icu_flags_type;

static icu_flags_type convert_xquery_flags( char const *xq_flags ) {
  icu_flags_type icu_flags = 0;
  if ( xq_flags ) {
    for ( char const *f = xq_flags; *f; ++f ) {
      switch ( *f ) {
        case 'i': icu_flags |= UREGEX_CASE_INSENSITIVE; break;
        case 'm': icu_flags |= UREGEX_MULTILINE       ; break;
        case 'q': icu_flags |= UREGEX_LITERAL         ; break;
        case 's': icu_flags |= UREGEX_DOTALL          ; break;
        case 'x': icu_flags |= UREGEX_COMMENTS        ; break;
        default:
          throw XQUERY_EXCEPTION( err::FORX0001, ERROR_PARAMS( *f ) );
      }
    }
    if ( icu_flags & UREGEX_LITERAL ) {
      //
      // XQuery 3.0 F&O: 5.6.1.1: If [the 'q' flag] is used together with the
      // m, s, or x flag, that flag has no effect.
      //
      icu_flags &= ~(
        UREGEX_MULTILINE  | // 'm'
        UREGEX_DOTALL     | // 's'
        UREGEX_COMMENTS     // 'x'
      );
    }
  }
  return icu_flags;
}

/**
 * Checks whether the given iterator is positioned at the first character in a
 * character range, e.g, the 'a' in "[a-z]" (assuming we're already within a
 * character class [...]).
 *
 * @param s The string on which \a i is iterating.
 * @param i The iterator marking the position of the character to check.
 */
inline bool is_char_range_begin( zstring const &s,
  /* intentionally not const& */ zstring::const_iterator i ) {
  if ( ztd::peek( s, &i ) != '-' )
    return false;
  char const next = ztd::peek( s, &i );
  return next != '[' && next != ']';
}

/**
 * Checks the block name of a \p{IsX} or \P{IsX} block escape.  ICU matches
 * property names loosely (ignoring case, spaces, hyphens and underscores), so
 * it accepts names that are not XSD block names, e.g., "IsBasic-Latin".  An
 * XSD block name is the Unicode block name with the spaces removed, so a '-'
 * can only come from the Unicode name: either "Latin-1" or a single trailing
 * letter as in "LatinExtended-A" or "ArabicPresentationForms-B".  Whether the
 * block exists is left to ICU.
 *
 * @param s The XQuery regular expression.
 * @param i The iterator positioned at the 'p' or 'P'.
 */
static void check_block_escape( zstring const &s_raw,
                                zstring::const_iterator i_raw,
                                bool x_flag ) {
  //
  // With the 'x' flag, whitespace inside the braces is removed too, e.g.,
  // "\p{ I s BasicLatin }" (QT3 K2-MatchesFunc-6).
  //
  zstring s;
  for ( zstring::const_iterator j = i_raw; j != s_raw.end(); ++j ) {
    if ( !(x_flag && ascii::is_space( *j )) )
      s += *j;
    if ( *j == '}' )
      break;
  }
  zstring::const_iterator i = s.begin();
  zstring::const_iterator const end = s.end();
  if ( ++i == end || *i != '{' )
    return;                             // ICU reports it
  zstring::const_iterator const name_begin = i + 1;
  if ( ++i == end || *i != 'I' || ++i == end || *i != 's' ) {
    //
    // XSD 1.0 F.1.1 [28] IsCategory: only these general categories (Cs is
    // excluded).  ICU also accepts other property names, e.g.,
    // \p{Alphabetic} or \p{L_}.
    //
    static char const *const categories[] = {
      "L", "Lu", "Ll", "Lt", "Lm", "Lo",
      "M", "Mn", "Mc", "Me",
      "N", "Nd", "Nl", "No",
      "P", "Pc", "Pd", "Ps", "Pe", "Pi", "Pf", "Po",
      "Z", "Zs", "Zl", "Zp",
      "S", "Sm", "Sc", "Sk", "So",
      "C", "Cc", "Cf", "Co", "Cn"
    };
    zstring name;
    for ( zstring::const_iterator j = name_begin; j != end && *j != '}'; ++j )
      name += *j;
    for ( char const *category : categories )
      if ( name == category )
        return;
    throw INVALID_RE_EXCEPTION( s_raw, ZED( REGEX_UNKNOWN_PIs_CONSTRUCT ) );
  }
  zstring name;
  for ( ++i; i != end && *i != '}'; ++i )
    name += *i;
  if ( i == end )
    return;                             // unbalanced: ICU reports it
  for ( zstring::size_type n = 0; n < name.size(); ++n ) {
    char const c = name[n];
    if ( ascii::is_alnum( c ) )
      continue;
    if ( c == '-' ) {
      bool const latin_1 = n >= 5 && name.compare( n - 5, 5, "Latin" ) == 0 &&
        n + 1 < name.size() && name[ n + 1 ] == '1';
      bool const trailing_letter = n > 0 && n + 2 == name.size() &&
        ascii::is_alpha( name[ n + 1 ] );
      if ( latin_1 || trailing_letter )
        continue;
    }
    throw INVALID_RE_EXCEPTION( s_raw, ZED( REGEX_UNKNOWN_PIs_CONSTRUCT ) );
  }
}

//#define DEBUG_CONVERT_REGEX

//
// Start of the ICU comment that convert_xquery_re() puts in front of a
// pattern with a back-reference; the hex-encoded XQuery pattern follows.
//
static char const backref_marker[] = "(?#zorba-backref:";

#define IS_CHAR_RANGE_BEGIN (in_char_class && is_char_range_begin( xq_re, i ))
#define PEEK_C              ztd::peek( xq_re, i )

void convert_xquery_re( zstring const &xq_re, zstring *icu_re,
                        char const *xq_flags ) {
  icu_flags_type const icu_flags = convert_xquery_flags( xq_flags );
  bool const i_flag = (icu_flags & UREGEX_CASE_INSENSITIVE) != 0;
  bool const m_flag = (icu_flags & UREGEX_MULTILINE) != 0;
  bool const q_flag = (icu_flags & UREGEX_LITERAL) != 0;
  bool const s_flag = (icu_flags & UREGEX_DOTALL) != 0;
  bool const x_flag = (icu_flags & UREGEX_COMMENTS) != 0;

  icu_re->clear();
  icu_re->reserve( xq_re.length() );    // approximate

  if ( q_flag ) {
    //
    // XQuery 3.0 F&O 5.6.1.1: with the 'q' flag, all characters in the
    // regular expression are treated as representing themselves.  ICU's own
    // literal mode is not used (see regex::compile()), so escape every ASCII
    // punctuation character; a lone "\" is then valid too.
    //
    FOR_EACH( zstring, i, xq_re ) {
      if ( ascii::is_punct( *i ) )
        *icu_re += '\\';
      *icu_re += *i;
    }
    return;
  }

  char c;                               // current (raw) XQuery char
  char c_cooked;                        // current cooked XQuery char
  char prev_c_cooked = 0;               // previous c_cooked
  char char_range_begin_cooked = 0;     // the 'a' in [a-b]
  bool char_range_possible = true;      // handles case like [a-h-o-z]

  bool got_backslash = false;
  int  got_quantifier = 0;
  int  in_char_class = 0;               // within [...]
  int  in_char_range = 0;               // within a-b within [...]
  int  char_group_start = 0;            // at first char of a char group
  vector<bool> subtraction;             // char class at depth has -[...]
  bool put_close_bracket = false;       // put another ] for char class

  bool in_backref = false;              // '\'[1-9][0-9]*
  unsigned backref_no = 0;              // 1-based
  bool after_backref = false;           // current char follows a backref
  bool in_quantifier = false;           // within {n,m}
  int  closed_group = 0;                // previous char closed a (...)

  bool got_backref = false;             // the RE has a back-reference

  // capture subgroup: true = open; false = closed
  vector<bool> cap_sub;                 // 0-based

  // open parentheses, innermost last: the 1-based capture subgroup number,
  // or 0 for a non-capturing subgroup
  vector<unsigned> open_parens;

  FOR_EACH( zstring, i, xq_re ) {
    c = c_cooked = *i;
    if ( got_backslash ) {
      if ( x_flag && !in_char_class && ascii::is_space( c ) ) {
        //
        // XQuery 3.0 F&O 5.6.1.1: If [the 'x' flag is] present, whitespace
        // characters ... in the regular expression are removed prior to
        // matching with one exception: whitespace characters within character
        // class expressions ... are not removed.
        //
        goto next;
      }
      got_backslash = false;

      switch ( c ) {

        ////////// Back-References ////////////////////////////////////////////

        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
          backref_no = c - '0';
          if ( !backref_no )          // \0 is illegal
            throw INVALID_RE_EXCEPTION( xq_re, ZED( BackRef0Illegal ) );
          if ( in_char_class ) {
            //
            // XQuery 3.0 F&O 5.6.1: Within a character class expression,
            // \ followed by a digit is invalid.
            //
            throw INVALID_RE_EXCEPTION(
              xq_re, ZED( BackRefIllegalInCharClass )
            );
          }
          in_backref = true;
          got_backref = true;
          [[fallthrough]];

        ////////// Single Character Escapes ///////////////////////////////////

        case '$': // added in XQuery 3.0 F&O 5.6.1
        case '(':
        case ')':
        case '*':
        case '+':
        case '-':
        case '.':
        case '?':
        case '[':
        case '\\':
        case ']':
        case '^':
        case '{':
        case '|':
        case '}':
          *icu_re += '\\';
          break;

        ////////// Multi-Character & Category Escapes /////////////////////////

        case 'n': // newline
          *icu_re += '\\';
          c_cooked = '\n';
          break;
        case 'r': // carriage return
          *icu_re += '\\';
          c_cooked = '\r';
          break;
        case 't': // tab
          *icu_re += '\\';
          c_cooked = '\t';
          break;
        case 's': // whitespace
        case 'S': // [^\s]
          if ( in_char_range || IS_CHAR_RANGE_BEGIN )
            goto not_single_char_esc;
          //
          // XSD 1.0 F.1.1 (Multi-Character Escapes): \s is [#x20\t\n\r].
          // ICU's \s is \p{White_Space}, which also has U+00A0, U+0085,
          // U+2028, etc.  Inside a character class, ICU takes a nested [...]
          // as a union, so this works there too.
          //
          *icu_re += c == 's' ? "[\\x{20}\\t\\n\\r]" : "[^\\x{20}\\t\\n\\r]";
          goto next;
        case 'd': // [0-9]
        case 'D': // [^\d]
        case 'p': // category escape
        case 'P': // [^\p]
          if ( in_char_range || IS_CHAR_RANGE_BEGIN )
            goto not_single_char_esc;
          *icu_re += '\\';
          if ( c == 'p' || c == 'P' ) {
            check_block_escape( xq_re, i, x_flag );
            //
            // Copy the "{name}" as is: its braces are not a quantifier.
            //
            *icu_re += c;
            zstring::const_iterator j = i;
            if ( ++j != xq_re.end() && *j == '{' ) {
              for ( ; j != xq_re.end(); ++j ) {
                if ( x_flag && ascii::is_space( *j ) )
                  continue;
                *icu_re += *j;
                if ( *j == '}' )
                  break;
              }
              if ( j == xq_re.end() )
                throw INVALID_RE_EXCEPTION( xq_re, ZED( UnbalancedChar_3 ), '{' );
              i = j;
            }
            goto next;
          }
          break;
        case 'w': // word char
          if ( in_char_range || IS_CHAR_RANGE_BEGIN )
            goto not_single_char_esc;
          //
          // Note that we can't simply pass \w through to ICU because what it
          // considers a "word character" is different from what XQuery does.
          //
          *icu_re += "[^" bs_W "]";
          goto next;
        case 'W': // [^\w]
          if ( in_char_range || IS_CHAR_RANGE_BEGIN )
            goto not_single_char_esc;
          *icu_re += "[" bs_W "]";
          goto next;
        case 'c': // NameChar
          if ( in_char_range || IS_CHAR_RANGE_BEGIN )
            goto not_single_char_esc;
          *icu_re += "[" bs_c "]";
          goto next;
        case 'C': // [^\c]
          if ( in_char_range || IS_CHAR_RANGE_BEGIN )
            goto not_single_char_esc;
          *icu_re += "[^" bs_c "]";
          goto next;
        case 'i': // initial NameChar
          if ( in_char_range || IS_CHAR_RANGE_BEGIN )
            goto not_single_char_esc;
          *icu_re += "[" bs_i "]";
          goto next;
        case 'I': // [^\i]
          if ( in_char_range || IS_CHAR_RANGE_BEGIN )
            goto not_single_char_esc;
          *icu_re += "[^" bs_i "]";
          goto next;

        default:
          throw INVALID_RE_EXCEPTION( xq_re, ZED( BadRegexEscape_3 ), c );
      }
    } else {
      after_backref = in_backref;
      if ( in_backref ) {
        //
        // XQuery 3.0 F&O 5.6.1: The construct \N where N is a single digit is
        // always recognized as a back-reference; if this is followed by
        // further digits, these digits are taken to be part of the back-
        // reference if and only if the resulting number NN is such that the
        // back-reference is preceded by NN or more unescaped opening
        // parentheses.
        //
        bool prevent_multidigit_backref = false;
        if ( ascii::is_digit( c ) ) {
          if ( backref_no * 10 + (c - '0') <= cap_sub.size() )
            backref_no = backref_no * 10 + (c - '0');
          else {
            in_backref = false;
            //
            // Unlike XQuery, ICU always takes further digits to be part of the
            // backreference so we have to prevent ICU from doing that.  One
            // way to do that is by enclosing said digits in a single-character
            // character class, i.e., [N].
            //
            *icu_re += '[';
            *icu_re += c;
            *icu_re += ']';
            prevent_multidigit_backref = true;
          }
        } else
          in_backref = false;

        //
        // XQuery 3.0 F&O 5.6.1: The regular expression is invalid if a back-
        // reference refers to a subexpression that does not exist or whose
        // closing right parenthesis occurs after the back-reference.
        //
        if ( backref_no > cap_sub.size() )
          throw INVALID_RE_EXCEPTION(
            xq_re, ZED( NonexistentBackRef_3 ), backref_no
          );
        if ( cap_sub[ backref_no - 1 ] )
          throw INVALID_RE_EXCEPTION(
            xq_re, ZED( NonClosedBackRef_3 ), backref_no
          );

        if ( prevent_multidigit_backref )
          goto next;
      }

      switch ( c ) {
        case '$':
          if ( q_flag )
            *icu_re += '\\';
          else if ( !m_flag && !in_char_class ) {
            //
            // XQuery 3.0 F&O 5.6.1: By default, ... $ matches the end of the
            // entire string.  [Newlines are treated as any other character.]
            //
            // However, in ICU, $ always matches before any trailing newlines.
            //
            // To make ICU work as XQuery needs it to, substitute \z for $
            // outside a character class (when multi-line mode is not set).
            // ICU does not accept a quantifier on \z ("alp$?ha" is valid
            // XQuery), so a quantified $ becomes a non-capturing group.
            //
            zstring::const_iterator j = i;
            switch ( ztd::peek( xq_re, &j ) ) {
              case '?': case '*': case '+': case '{':
                icu_re->append( "(?:\\z)" );
                break;
              default:
                icu_re->append( "\\z" );
            }
            goto next;
          }
          break;
        case '(':
          if ( q_flag || in_char_class )
            *icu_re += '\\';          // an ordinary character in a class
          else {
            zstring::const_iterator j = i;
            if ( ++j != xq_re.end() && *j == '?' && ++j != xq_re.end() ) {
              //
              // Got "(?" sequence: potentially start of "(?:", a non-capturing
              // subgroup.  ICU also allows other characters after the "(?"
              // that XQuery does not, so we have to report those as errors.
              //
              if ( *j != ':' )
                throw INVALID_RE_EXCEPTION( xq_re, ZED( BadRegexParen_3 ), *j );
              //
              // Start of non-capturing subgroup.
              //
              open_parens.push_back( 0 );
            } else {
              //
              // Start of capturing subgroup.
              //
              cap_sub.push_back( true );
              open_parens.push_back( static_cast<unsigned>( cap_sub.size() ) );
            }
          }
          break;
        case ')':
          if ( q_flag || in_char_class )
            *icu_re += '\\';
          else {
            if ( open_parens.empty() )
              goto unbalanced_char;
            if ( unsigned const closed = open_parens.back() )
              cap_sub[ closed - 1 ] = false;
            open_parens.pop_back();
            closed_group = 2;
          }
          break;
        case '*':
        case '+':
        case '?':
        case '{':
          if ( q_flag )
            *icu_re += '\\';
          else if ( in_char_class ) {
            //
            // Inside an ICU set, {...} is a multi-character string; in XSD,
            // '{' is an ordinary character there, e.g., "ab[(?{1]".
            //
            if ( c == '{' )
              *icu_re += '\\';
          } else {
            //
            // ICU allows the multiple quantifiers *+, ++, and ?+, but XQuery
            // does not so we have to check for them.
            //
            if ( got_quantifier && c != '?' )
              throw INVALID_RE_EXCEPTION(
                xq_re, ZED( BadQuantifierHere_3 ), c
              );
            got_quantifier = 2;
            if ( c == '{' )
              in_quantifier = true;
            if ( (c == '*' || c == '+') && PEEK_C == '?' &&
                 (closed_group || after_backref) ) {
              //
              // ICU compiles the reluctant quantifiers *? and +? without a
              // check that an iteration consumed input, so a group or back-
              // reference that can match the empty string makes the match
              // loop forever, e.g., "(?:r?)*?r" (QT3 fn-matches-50, test
              // p1030).  ICU's counted loops {0,}? and {1,}? have that check
              // and match the same strings in the same order, so use them.
              //
              icu_re->append( c == '*' ? "{0,}" : "{1,}" );
              goto next;
            }
          }
          break;
        case '-':
          if ( in_char_class && !in_char_range ) {
            char const next_c = PEEK_C;
            if ( next_c == '[' ) {
              if ( put_close_bracket ) {
                //
                // See the comment below for the '[' case.
                //
                *icu_re += ']';
                put_close_bracket = false;
              }
              //
              // ICU uses "--" to indicate range subtraction, e.g.,
              // XQuery [A-Z-[OI]] becomes ICU [A-Z--[OI]].
              //
              *icu_re += '-';
              if ( subtraction.size() <= (size_t)in_char_class )
                subtraction.resize( in_char_class + 1 );
              subtraction[ in_char_class ] = true;
            } else if ( char_group_start || next_c == ']' ) {
              //
              // A literal '-' as the first or last character of a character
              // group.
              //
            } else if ( char_range_possible ) {
              //
              // The '-' indicates a character range.
              //
              char_range_begin_cooked = prev_c_cooked;
              in_char_range = 2;
            } else {
              //
              // XSD 1.0 (Second Edition) F.1 / XSD 1.1 G.4.2.3: a literal '-'
              // is allowed only at the beginning or end of a character group
              // (or before a subtracted class), so "[a-h-o-z]" or
              // "[0-9-.]" is invalid: the '-' after a range is neither.
              //
              goto unescaped_char;
            }
          }
          break;
        case '.':
          if ( !in_char_class ) {
            //
            // XQuery 3.0 F&O 5.6.1: without the 's' flag, '.' matches any
            // character except newline (#x0A) and carriage return (#x0D);
            // with it, '.' matches any character.  ICU's '.' also excludes
            // U+0085, U+2028 and U+2029, and with UREGEX_DOTALL it matches
            // "\r\n" as a single character.
            //
            icu_re->append( s_flag ? "[\\x{0}-\\x{10FFFF}]" : "[^\\n\\r]" );
            goto next;
          }
          [[fallthrough]];
        case '}':
          if ( q_flag || (c == '}' && in_char_class) )
            *icu_re += '\\';
          else if ( c == '}' && in_quantifier ) {
            //
            // A quantifier {n,m} cannot be quantified again: ICU takes
            // "a{1,5}+" as possessive, XQuery does not allow it.
            //
            in_quantifier = false;
            got_quantifier = 2;
          }
          break;
        case '[':
          if ( q_flag )
            *icu_re += '\\';
          else {
            if ( in_char_class && prev_c_cooked != '-' )
              goto unescaped_char;
            if ( !in_char_class++ && PEEK_C == '^' ) {
              //
              // XML Schema Part 2 F.1 [16]: For any positive character group
              // or negative character group G, and any character class
              // expression C, G-C is a valid character class subtraction,
              // identifying the set of all characters in C(G) that are not
              // also in C(C).
              //
              // Hence, in XQuery, [^abcd-[xy]] means "all characters except
              // abcdxy", i.e., the ^ has a higher precedence than -.
              //
              // However, in ICU, the reverse is true.  To make ICU behave like
              // XQuery, we have to wrap the negative character group in [],
              // i.e., [[^abcd]-[xy]].
              //
              *icu_re += '[';
              put_close_bracket = true;
            }
            char_group_start = 2;
          }
          break;
        case '\\':
          got_backslash = true;
          if ( in_char_range )
            ++in_char_range;
          goto next;
        case ']':
          if ( q_flag )
            *icu_re += '\\';
          else {
            if ( !in_char_class )
              goto unbalanced_char;
            if ( put_close_bracket ) {
              *icu_re += ']';
              put_close_bracket = false;
            }
            --in_char_class;
            in_char_range = 0;
            if ( (size_t)in_char_class < subtraction.size() &&
                 subtraction[ in_char_class ] ) {
              //
              // XSD F.1 [12]: a subtracted class expression must be the last
              // part of its character group, e.g., [a-[b] ] is invalid.
              //
              subtraction[ in_char_class ] = false;
              if ( PEEK_C != ']' )
                throw INVALID_RE_EXCEPTION(
                  xq_re, ZED( REGEX_INVALID_USE_OF_SUBCLASS )
                );
            }
          }
          break;
        case '^':
          //
          // XQuery 3.0 F&O 5.6.1: outside a character class, ^ is always an
          // anchor (it is not a normal character there), so "^^" matches at
          // the start of the string.  Inside a character class, ^ negates
          // only as the first character; elsewhere it is an ordinary
          // character (XSD XmlChar).
          //
          if ( in_char_class && !q_flag ) {
            if ( char_group_start && prev_c_cooked == '[' )
              char_group_start = 2;     // negative character group [^...]
            else
              *icu_re += '\\';
          } else if ( q_flag )
            *icu_re += '\\';
          break;
        case '|':
          if ( q_flag )
            *icu_re += '\\';
          else
          break;
        default:
          if ( x_flag && c == '#' ) {
            //
            // The 'x' flag only removes whitespace in XQuery; in ICU's
            // UREGEX_COMMENTS mode, '#' starts a comment.
            //
            *icu_re += '\\';
            break;
          }
          if ( x_flag && ascii::is_space( c ) ) {
            if ( !in_char_class )
              goto next;
            //
            // This is similar to the above case for removing whitespace except
            // ICU removes *all* whitespace (even within character classes)
            // unless the whitespace is escaped; hence we have to escape it.
            //
            *icu_re += '\\';
          }
      } // switch
    } // else

    if ( in_char_range == 1 && c_cooked < char_range_begin_cooked )
      throw INVALID_RE_EXCEPTION(
        xq_re, ZED( BadEndCharInRange_34 ),
        ascii::printable_char( c_cooked ),
        char_range_begin_cooked
      );

    *icu_re += c;

next:
    char_range_possible = !dec_limit( &in_char_range );
    dec_limit( &got_quantifier );
    dec_limit( &closed_group );
    dec_limit( &char_group_start );
    prev_c_cooked = c_cooked;
  } // FOR_EACH

  if ( got_backslash )
    throw INVALID_RE_EXCEPTION( xq_re, ZED( TrailingChar_3 ), '\\' );
  if ( in_char_class )
    throw INVALID_RE_EXCEPTION( xq_re, ZED( UnbalancedChar_3 ), '[' );

  if ( !q_flag ) {
    if ( i_flag ) {
      //
      // XQuery 3.0 F&O 5.6.1.1: All other constructs are unaffected by the "i"
      // flag.  For example, "\p{Lu}" continues to match upper-case letters
      // only.
      //
      // However, ICU lower-cases everything for the 'i' flag; hence we have to
      // turn off the 'i' flag for the \p{Lu} and \P{Lu}.
      //
      // Note that the "6" and "12" below are correct since "\\" represents a
      // single '\'.
      //
      ascii::replace_all( *icu_re, "\\p{Lu}", 6, "(?-i:\\p{Lu})", 12 );
      ascii::replace_all( *icu_re, "\\P{Lu}", 6, "(?-i:\\P{Lu})", 12 );
    }

    //
    // XML Schema Part 2 F.1.1: [Unicode Database] groups code points into a
    // number of blocks such as Basic Latin (i.e., ASCII), Latin-1 Supplement,
    // Hangul Jamo, CJK Compatibility, etc. The set containing all characters
    // that have block name X (with all white space stripped out), can be
    // identified with a block escape \p{IsX}.
    //
    // However, ICU uses \p{InX} rather than \p{IsX}.
    //
    // Note that the "5" below is correct since "\\" represents a single '\'.
    //
    ascii::replace_all( *icu_re, "\\p{Is", 5, "\\p{In", 5 );
    ascii::replace_all( *icu_re, "\\P{Is", 5, "\\P{In", 5 );

    //
    // Apparently, ICU doesn't recognize InPrivateUse, so change it to Co.
    //
    // Note that the "16" and "6" below are correct since "\\" represents a
    // single '\'.
    //
    ascii::replace_all( *icu_re, "\\p{InPrivateUse}", 16, "\\p{Co}", 6 );
    ascii::replace_all( *icu_re, "\\P{InPrivateUse}", 16, "\\P{Co}", 6 );
  } // q_flag

  if ( got_backref ) {
    //
    // ICU makes a back-reference to a capturing group that did not
    // participate in the match fail, and a group inside a repeated group keeps
    // what it captured in an earlier iteration.  XQuery F&O 3.0 5.6.1 has such
    // a back-reference match the empty string, so regex::compile() matches
    // these patterns with Zorba's own engine.  It needs the XQuery pattern for
    // that: put it, hex-encoded, in a leading ICU comment, which ICU ignores.
    //
    static char const hex[] = "0123456789abcdef";
    zstring marker( backref_marker );
    FOR_EACH( zstring, j, xq_re ) {
      unsigned char const b = static_cast<unsigned char>( *j );
      marker += hex[ b >> 4 ];
      marker += hex[ b & 0x0F ];
    }
    marker += ')';
    icu_re->insert( 0, marker );
  }

#ifdef DEBUG_CONVERT_REGEX
  cout << "XQ : " << xq_re   << endl;
  cout << "ICU: " << *icu_re << endl;
#endif /* DEBUG_CONVERT_REGEX */
  return;

not_single_char_esc:
  throw INVALID_RE_EXCEPTION( xq_re, ZED( NotSingleCharEsc_3 ), c );
unbalanced_char:
  throw INVALID_RE_EXCEPTION( xq_re, ZED( UnbalancedChar_3 ), c );
unescaped_char:
  throw INVALID_RE_EXCEPTION( xq_re, ZED( UnescapedChar_3 ), c );
}

///////////////////////////////////////////////////////////////////////////////

namespace unicode {

//
// Bound on the work ICU may do for one match operation, in ICU match-engine
// steps (on the order of milliseconds each), so that catastrophic
// backtracking in a user pattern raises an error instead of hanging.
//
static int32_t const regex_time_limit = 10000;

static void check_regex_status( UErrorCode status ) {
  if ( status == U_REGEX_TIME_OUT )
    throw XQUERY_EXCEPTION(
      zerr::ZXQP0001_DYNAMIC_RUNTIME_ERROR,
      ERROR_PARAMS( "regular expression match exceeded the time limit" )
    );
}

/**
 * Gets the XQuery pattern that convert_xquery_re() put in front of a pattern
 * with a back-reference.
 *
 * @param pattern The converted pattern.
 * @param xq_pattern Set to the XQuery pattern.
 * @return Returns \c true only if \a pattern has one.
 */
static bool get_backref_xq_pattern( char const *pattern, zstring *xq_pattern ) {
  size_t const marker_len = sizeof( backref_marker ) - 1;
  if ( !pattern || std::strncmp( pattern, backref_marker, marker_len ) != 0 )
    return false;
  xq_pattern->clear();
  for ( char const *p = pattern + marker_len; *p && *p != ')'; p += 2 ) {
    if ( !ascii::is_xdigit( p[0] ) || !ascii::is_xdigit( p[1] ) )
      return false;
    unsigned char const hi = ascii::is_digit( p[0] ) ? p[0] - '0' : (p[0] | 0x20) - 'a' + 10;
    unsigned char const lo = ascii::is_digit( p[1] ) ? p[1] - '0' : (p[1] | 0x20) - 'a' + 10;
    *xq_pattern += static_cast<char>( (hi << 4) | lo );
  }
  return true;
}

regex::~regex() {
  delete matcher_;
  delete zre_;
}

void regex::compile( string const &u_pattern, char const *flags,
                     char const *pattern ) {
  zstring xq_pattern;
  bool const has_backref = get_backref_xq_pattern( pattern, &xq_pattern );
  if ( has_backref )
    pattern = xq_pattern.c_str();       // for error messages

  icu_flags_type const icu_flags =
    convert_xquery_flags( flags ) & ~UREGEX_LITERAL;
  delete matcher_;
  delete zre_;
  zre_ = nullptr;
  UErrorCode status = U_ZERO_ERROR;
  matcher_ = new RegexMatcher( u_pattern, icu_flags, status );
  if ( U_FAILURE( status ) ) {
    delete matcher_;
    matcher_ = nullptr;

    zstring icu_error_key;
    if ( status > U_REGEX_ERROR_START && status < U_REGEX_ERROR_LIMIT ) {
      icu_error_key = ZED_PREFIX;
      icu_error_key += u_errorName( status );
    }
    throw INVALID_RE_EXCEPTION( pattern, icu_error_key );
  }
  matcher_->setTimeLimit( regex_time_limit, status );

  if ( has_backref ) {
    zre_flags_ = 0;
    if ( icu_flags & UREGEX_CASE_INSENSITIVE )
      zre_flags_ |= REGEX_ASCII_CASE_INSENSITIVE;
    if ( icu_flags & UREGEX_DOTALL )
      zre_flags_ |= REGEX_ASCII_DOTALL;
    if ( icu_flags & UREGEX_MULTILINE )
      zre_flags_ |= REGEX_ASCII_MULTILINE;
    if ( icu_flags & UREGEX_COMMENTS )
      zre_flags_ |= REGEX_ASCII_NO_WHITESPACE;
    try {
      regex_engine::CRegexXQuery_parser parser;
      zre_ = parser.parse( xq_pattern.c_str(), zre_flags_ );
    }
    catch ( XQueryException const& ) {
      //
      // The pattern is valid (convert_xquery_re() and ICU accepted it), but
      // Zorba's engine rejects a few valid constructs, e.g., a '^' that is not
      // at the start of a branch: keep matching with ICU.
      //
      zre_ = nullptr;
    }
  }
}

void regex::z_set_subject( string const &s, size_type from ) {
  //
  // Convert from the character before "from" on: earlier text can not be
  // part of a match, but the engine needs that character to know whether a
  // '^' matches at "from" in multi-line mode.
  //
  size_type base = 0;
  if ( from > 0 ) {
    base = from - 1;
    if ( base > 0 && U16_IS_TRAIL( s.charAt( base ) ) &&
         U16_IS_LEAD( s.charAt( base - 1 ) ) )
      --base;
  }
  z_s_.clear();
  utf8::to_string( s.getBuffer() + base, s.length() - base, &z_s_ );
  z_base_ = base;
  z_off_byte_ = 0;
  z_off_u16_ = base;
  z_start_ = z_end_ = -1;
  //
  // The byte offset of "from".
  //
  int byte_off = 0;
  for ( size_type u = base; u < from; ++byte_off )
    if ( (z_s_[ byte_off ] & 0xC0) != 0x80 )
      u += static_cast<unsigned char>( z_s_[ byte_off ] ) >= 0xF0 ? 2 : 1;
  while ( byte_off < static_cast<int>( z_s_.size() ) &&
          (z_s_[ byte_off ] & 0xC0) == 0x80 )
    ++byte_off;
  z_pos_ = byte_off;
}

bool regex::z_search( int from, bool whole ) {
  int match_pos, matched_len;
  unsigned flags = zre_flags_;
  if ( whole )
    flags |= REGEX_ASCII_WHOLE_MATCH;
  if ( from > static_cast<int>( z_s_.size() ) ||
       !zre_->search( z_s_.c_str(), from, flags, &match_pos, &matched_len ) ) {
    z_start_ = z_end_ = -1;
    return false;
  }
  z_start_ = match_pos;
  z_end_ = match_pos + matched_len;
  return true;
}

size_type regex::z_to_u16( int byte_off ) const {
  if ( byte_off < z_off_byte_ ) {
    z_off_byte_ = 0;
    z_off_u16_ = z_base_;
  }
  for ( ; z_off_byte_ < byte_off; ++z_off_byte_ ) {
    unsigned char const b = static_cast<unsigned char>( z_s_[ z_off_byte_ ] );
    if ( (b & 0xC0) != 0x80 )
      z_off_u16_ += b >= 0xF0 ? 2 : 1;
  }
  return z_off_u16_;
}

bool regex::z_replace_all( string const &in, string const &replacement,
                           string *out ) {
  z_set_subject( in, 0 );
  zstring repl;
  utf8::to_string( replacement.getBuffer(), replacement.length(), &repl );
  int const groups = static_cast<int>( zre_->get_indexed_regex_count() );
  char const *const s = z_s_.c_str();
  int const s_len = static_cast<int>( z_s_.size() );
  zstring result;
  int copied = 0, from = 0;

  while ( z_search( from ) ) {
    result.append( s + copied, z_start_ - copied );
    //
    // The replacement uses the ICU syntax (FnReplaceIterator converted it):
    // "\" escapes the next character; "$" and digits is a group, taking as
    // many digits as still make a group number.
    //
    for ( zstring::size_type i = 0; i < repl.size(); ++i ) {
      char const c = repl[i];
      if ( c == '\\' && i + 1 < repl.size() ) {
        result += repl[ ++i ];
        continue;
      }
      if ( c == '$' && i + 1 < repl.size() && ascii::is_digit( repl[ i + 1 ] ) ) {
        int group = repl[ ++i ] - '0';
        while ( i + 1 < repl.size() && ascii::is_digit( repl[ i + 1 ] ) &&
                group * 10 + (repl[ i + 1 ] - '0') <= groups )
          group = group * 10 + (repl[ ++i ] - '0');
        if ( !group )
          result.append( s + z_start_, z_end_ - z_start_ );
        else {
          char const *g;
          int g_len;
          // a group that did not participate is the empty string
          if ( group <= groups && zre_->get_indexed_match( group, &g, &g_len ) )
            result.append( g, g_len );
        }
        continue;
      }
      result += c;
    }
    copied = from = z_end_;
    if ( z_start_ == z_end_ ) {
      // an empty match: the next search starts after the next character
      if ( from == s_len )
        break;
      ++from;
      while ( from < s_len && (s[ from ] & 0xC0) == 0x80 )
        ++from;
    }
  }
  result.append( s + copied, s_len - copied );
  return to_string( result, out );
}

int regex::get_group_count() const {
  if ( zre_ )
    return static_cast<int>( zre_->get_indexed_regex_count() );
  ZORBA_ASSERT( matcher_ );
  return matcher_->groupCount();
}

int regex::get_group_start( int group ) const {
  if ( zre_ ) {
    if ( z_start_ < 0 )
      return -1;
    if ( !group )
      return static_cast<int>( z_to_u16( z_start_ ) );
    char const *g;
    int g_len;
    if ( !zre_->get_indexed_match( group, &g, &g_len ) )
      return -1;
    return static_cast<int>( z_to_u16( static_cast<int>( g - z_s_.c_str() ) ) );
  }
  ZORBA_ASSERT( matcher_ );
  UErrorCode status = U_ZERO_ERROR;
  return matcher_->start( group, status );
}

int regex::get_group_end( int group ) const {
  if ( zre_ ) {
    if ( z_end_ < 0 )
      return -1;
    if ( !group )
      return static_cast<int>( z_to_u16( z_end_ ) );
    char const *g;
    int g_len;
    if ( !zre_->get_indexed_match( group, &g, &g_len ) )
      return -1;
    return static_cast<int>(
      z_to_u16( static_cast<int>( g - z_s_.c_str() ) + g_len )
    );
  }
  ZORBA_ASSERT( matcher_ );
  UErrorCode status = U_ZERO_ERROR;
  return matcher_->end( group, status );
}

bool regex::get_group_start_end( int *start, int *end, int group ) const {
  int const temp = get_group_start( group );
  if ( temp == -1 ) 
    return false;
  *start = temp;
  *end = get_group_end( group );
  return true;
}

bool regex::match_part( string const &s ) {
  if ( zre_ ) {
    z_set_subject( s, 0 );
    return z_search( 0 );
  }
  ZORBA_ASSERT( matcher_ );
  matcher_->reset( s );
  UErrorCode status = U_ZERO_ERROR;
  bool const found = !!matcher_->find( status );
  check_regex_status( status );
  return found;
}

bool regex::match_whole( string const &s ) {
  if ( zre_ ) {
    z_set_subject( s, 0 );
    return z_search( 0, true );
  }
  ZORBA_ASSERT( matcher_ );
  matcher_->reset( s );
  UErrorCode status = U_ZERO_ERROR;
  bool const matched = matcher_->matches( status );
  check_regex_status( status );
  return matched && U_SUCCESS( status );
}

bool regex::next( re_type_t re_type, string const &s, size_type *pos,
                  string *substring, bool *matched ) {
  ZORBA_ASSERT( matcher_ );
  ZORBA_ASSERT( pos );
  unicode::size_type const s_len = s.length();
  if ( *pos < s_len ) {
    bool found;
    size_type start = 0, end = 0;
    if ( zre_ ) {
      z_set_subject( s, *pos );
      found = z_search( z_pos_ );
      if ( found ) {
        start = z_to_u16( z_start_ );
        end = z_to_u16( z_end_ );
      }
    } else {
      matcher_->reset( s );
      UErrorCode status = U_ZERO_ERROR;
      found = !!matcher_->find( *pos, status );
      check_regex_status( status );
      if ( found ) {
        start = matcher_->start( status );
        end = matcher_->end( status );
      }
    }
    if ( found ) {
      if ( substring ) {
        switch ( re_type ) {
          case re_is_match:
            substring->setTo( s, start, end - start );
            break;
          case re_is_separator:
            substring->setTo( s, *pos, start - *pos );
            break;
        }
      }
      *pos = end;
      if ( matched )
        *matched = true;
      return true;
    }
    if ( re_type == re_is_separator ) {
      //
      // Special case: the RE did not match starting at pos, but there *is* a
      // last token.  For example, given the RE of "," and:
      //
      //    s   = "a,b,c"
      //    pos =  01234
      //
      // then calling next_token() with a pos of 4 will not match another ","
      // but we should return the final token after the last "," (the "c").
      //
      if ( substring )
        substring->setTo( s, *pos, s_len - *pos );
      *pos = s_len;
      if ( matched )
        *matched = false;
      return true;
    }
  }
  if ( matched )
    *matched = false;
  return false;
}

bool regex::next_match( bool *reached_end ) {
  if ( zre_ ) {
    bool const found = z_search( z_pos_ );
    if ( found ) {
      z_pos_ = z_end_;
      if ( z_start_ == z_end_ ) {
        // an empty match: the next search starts after the next character
        ++z_pos_;
        while ( z_pos_ < static_cast<int>( z_s_.size() ) &&
                (z_s_[ z_pos_ ] & 0xC0) == 0x80 )
          ++z_pos_;
      }
    }
    if ( reached_end )
      *reached_end = zre_->get_reachedEnd();
    return found;
  }
  ZORBA_ASSERT( matcher_ );
  UErrorCode status = U_ZERO_ERROR;
  bool const found = !!matcher_->find( status );
  check_regex_status( status );
  if ( reached_end ) {
#if U_ICU_VERSION_MAJOR_NUM >= 4
    *reached_end = !!matcher_->hitEnd();
#else
    *reached_end = true;
#endif /* U_ICU_VERSION_MAJOR_NUM */
  }
  return found;
}

bool regex::replace_all( string const &in, string const &replacement,
                         string *out ) {
  ZORBA_ASSERT( out );
  if ( zre_ )
    return z_replace_all( in, replacement, out );
  ZORBA_ASSERT( matcher_ );
  matcher_->reset( in );
  UErrorCode status = U_ZERO_ERROR;
  *out = matcher_->replaceAll( replacement, status );
  check_regex_status( status );
  return U_SUCCESS( status ) == true;
}

bool regex::replace_all( char const *in, char const *replacement,
                         string *out ) {
  string u_in, u_replacement;
  return  to_string( in, &u_in ) &&
          to_string( replacement, &u_replacement ) &&
          replace_all( u_in, u_replacement, out );
}

void regex::set_string( char const *s, size_type s_len ) {
  if ( zre_ ) {
    z_s_.assign( s, s_len );
    z_base_ = 0;
    z_off_byte_ = 0;
    z_off_u16_ = 0;
    z_pos_ = 0;
    z_start_ = z_end_ = -1;
    return;
  }
  ZORBA_ASSERT( matcher_ );
  to_string( s, s_len, &s_ );
  matcher_->reset( s_ );
}

///////////////////////////////////////////////////////////////////////////////

} // namespace unicode
} // namespace zorba

#endif /* ZORBA_NO_ICU */
/* vim:set et sw=2 ts=2: */
