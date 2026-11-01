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

#include <cctype>
#include <cstdint>
#include <cstring>
#include <type_traits>

#include <zorba/diagnostic_list.h>

#include "ascii_util.h"
#include "diagnostics/xquery_diagnostics.h"
#include "unicode_util.h"
#include "utf8_string.h"
#include "utf8_util.h"
#include "zorba_regex_engine.h"
#include "zorbatypes/chartype.h"
#include <zorba/internal/unique_ptr.h>

#ifndef ZORBA_NO_ICU
#include <unicode/uchar.h>
#include <unicode/uniset.h>
#endif /* ZORBA_NO_ICU */

namespace zorba {
  namespace regex_engine{
//ascii regular expression matching

/*http://www.w3.org/TR/xmlschema-2/#regexs

~regExp    ::=    branch ( '|' branch )*  
~branch    ::=    piece* 
~piece     ::=    atom quantifier? 
~quantifier    ::=    ( [?*+] | ( '{' quantity '}' ) ) '?'? 
~quantity    ::=    quantRange | quantMin | QuantExact 
~quantRange    ::=    QuantExact ',' QuantExact 
~quantMin    ::=    QuantExact ',' 
~QuantExact    ::=    [0-9]+ 
~atom    ::=    Char | charClass | ( '(' '?:'? regExp ')' ) | backReference
backReference ::= "\" [1-9][0-9]*
~Char    ::=    [^.\?*+{}()|^$#x5B#x5D] 
~charClass    ::=    charClassEsc | charClassExpr | WildcardEsc | "^" | "$"
~charClassExpr    ::=    '[' charGroup ']' 
~charGroup    ::=    posCharGroup | negCharGroup | charClassSub 
~posCharGroup    ::=    ( charRange | charClassEsc )+  
~negCharGroup    ::=    '^' posCharGroup 
~charClassSub    ::=    ( posCharGroup | negCharGroup ) '-' charClassExpr 
~charRange    ::=    seRange | XmlCharIncDash 
~seRange    ::=    charOrEsc '-' charOrEsc 
~charOrEsc    ::=    XmlChar | SingleCharEsc 
~XmlChar    ::=    [^\#x2D#x5B#x5D] 
~XmlCharIncDash    ::=    [^\#x5B#x5D] 
~charClassEsc    ::=    ( SingleCharEsc | MultiCharEsc | catEsc | complEsc )  
~SingleCharEsc    ::=    '\' [nrt\|.?*+(){}$#x2D#x5B#x5D#x5E]
~catEsc    ::=    '\p{' charProp '}' 
~complEsc    ::=    '\P{' charProp '}' 
charProp    ::=    IsCategory | IsBlock 
~MultiCharEsc    ::=    '\' [sSiIcCdDwW] 
~WildcardEsc    ::=    '.' 


+ http://www.w3.org/TR/xquery-operators/#regex-syntax (not implemented)
*/


static bool compare_ascii_i(const char *str1, const char *str2)
{
  while(*str1 && *str2)
  {
    if(ascii::to_lower(*str1) != ascii::to_lower(*str2))
      return false;
    str1++;
    str2++;
  }
  if(*str1 || *str2)
    return false;
  return true;
}

//
// XQuery F&O 3.0 5.6.1.1: with the 'i' flag, two characters match if there is
// a default case-insensitive match between them (The Unicode Standard 3.13):
// their case foldings are equal.
//
static unicode::code_point fold_case(unicode::code_point c)
{
#ifndef ZORBA_NO_ICU
  return u_foldCase(c, U_FOLD_CASE_DEFAULT);
#else
  //
  // Without ICU, Zorba's own case mappings: the lower case of the upper case
  // is the simple case folding for all but a few characters (e.g., U+0130
  // folds to itself but becomes 'i' here).
  //
  return unicode::to_lower(unicode::to_upper(c));
#endif /* ZORBA_NO_ICU */
}

//
// Whether c matches a character in [lo,hi] with the 'i' flag: a character of
// the range that case-insensitively matches c.
//
static bool in_range_i(unicode::code_point c, unicode::code_point lo,
                       unicode::code_point hi)
{
  if(c >= lo && c <= hi)
    return true;
#ifndef ZORBA_NO_ICU
  U_NAMESPACE_QUALIFIER UnicodeSet closure(c, c);
  closure.closeOver(USET_CASE_INSENSITIVE);
  return closure.containsSome(lo, hi);
#else
  unicode::code_point const others[] = {
    fold_case(c), unicode::to_upper(c), unicode::to_lower(c)
  };
  for(unicode::code_point o : others)
    if(o >= lo && o <= hi)
      return true;
  return false;
#endif /* ZORBA_NO_ICU */
}
static utf8::size_type myutf8len(const char *source)
{
  try {
    return utf8::char_length(*source);
  }
  catch ( utf8::invalid_byte const& ) {
    return 1;
  }
}
////////////////////////////////////
////Regular expression parsing and building of the tree
////////////////////////////////////

CRegexXQuery_regex* CRegexXQuery_parser::parse(const char *pattern, unsigned int flags)
{
  this->flags = flags;

  //
  // XQuery F&O 3.0 5.6.1.1: with the 'x' flag, whitespace characters are
  // removed from the regular expression prior to matching, except within
  // character class expressions.
  //
  std::string stripped;
  if((flags & REGEX_ASCII_NO_WHITESPACE) && !(flags & REGEX_ASCII_LITERAL))
  {
    int class_depth = 0;
    for(const char *p = pattern; *p; ++p)
    {
      char const c = *p;
      if(!class_depth && ascii::is_space(c))
        continue;
      stripped += c;
      if(c == '\\')
      {
        const char *q = p + 1;
        if(!class_depth)
          while(ascii::is_space(*q))
            ++q;
        if(!*q)
          break;
        stripped += *q;
        p = q;
      }
      else if(c == '[')
        ++class_depth;
      else if(c == ']' && class_depth)
        --class_depth;
    }
    pattern = stripped.c_str();
  }

  int   regex_len;
  CRegexXQuery_regex*  regex = parse_regexp(pattern, &regex_len);
  
  return regex;
}

//until '\0' or ')'
CRegexXQuery_regex* CRegexXQuery_parser::parse_regexp(const char *pattern, 
                                                    int *regex_len)
{
  *regex_len = 0;
  int   branch_len;
  regex_depth++;
  std::unique_ptr<CRegexXQuery_regex>  regex(new CRegexXQuery_regex(current_regex));
  if(!current_regex)
    current_regex = regex.get();
  regex->cap_first = current_regex->subregex.size();
  if(regex_depth >= 2)
  {
    //mark this as group if it does not start with ?:
    if(pattern[0] != '?' || pattern[1] != ':')
    {
      current_regex->subregex.push_back(regex.get());
      regex->capture_index = current_regex->subregex.size();
    }
    else
      *regex_len = 2;
  }
  CRegexXQuery_branch  *branch;
  bool must_read_another_branch = true;
  while(pattern[*regex_len] && (pattern[*regex_len] != ')'))
  {
    branch = parse_branch(pattern+*regex_len, &branch_len);
    if(!branch)
    {
      regex_depth--;
      return NULL;
    }
    regex->add_branch(branch);
    *regex_len += branch_len;
    if(pattern[*regex_len] == '|')
      (*regex_len)++;
    else
      must_read_another_branch = false;
  }
  if((current_regex == regex.get()) && (pattern[*regex_len] == ')'))
  {
    throw XQUERY_EXCEPTION( err::FORX0002, ERROR_PARAMS(pattern, ZED(REGEX_MISMATCHED_PAREN)) );
  }
  if(pattern[*regex_len])
    (*regex_len)++;
  if(must_read_another_branch)
    regex->add_branch(new CRegexXQuery_branch(current_regex));//add empty branch
  regex->cap_end = current_regex->subregex.size();
  regex->flags = 0;//finished initialization
  regex_depth--;
  return regex.release();
}

CRegexXQuery_branch* CRegexXQuery_parser::parse_branch(const char *pattern, int *branch_len)
{
  int piece_len;

  std::unique_ptr<CRegexXQuery_branch>    branch(new CRegexXQuery_branch(current_regex));
  CRegexXQuery_piece     *piece;
  *branch_len = 0;
  while(pattern[*branch_len] && (pattern[*branch_len] != '|') && (pattern[*branch_len] != ')'))
  {
    piece = parse_piece(pattern+*branch_len, &piece_len);
    if(!piece)
    {
      return NULL;
    }
    if(branch->piece_list.size() && dynamic_cast<CRegexXQuery_pinstart*>(piece->atom))
    {
      //found ^ that is not at the beginning of branch
      throw XQUERY_EXCEPTION( err::FORX0002, ERROR_PARAMS(pattern, ZED(REGEX_INVALID_ATOM_CHAR), '^') );
    }
    branch->add_piece(piece);
    *branch_len += piece_len;
  }
  //if(pattern[*branch_len] == '|')
  //  (*branch_len)++;
  return branch.release();
}

//piece = atom + quantifier
CRegexXQuery_piece* CRegexXQuery_parser::parse_piece(const char *pattern, int *piece_len)
{
  std::unique_ptr<CRegexXQuery_piece>  piece(new CRegexXQuery_piece);
  IRegexAtom  *atom;
  *piece_len = 0;

  int   atom_len = 0;
  int   quantif_len = 0;
  atom = read_atom(pattern, &atom_len);
  if(!atom)
  {
    return NULL;
  }
  piece->set_atom(atom);
  if(!(flags & REGEX_ASCII_LITERAL))
    read_quantifier(piece.get(), pattern+atom_len, &quantif_len);

  *piece_len += atom_len + quantif_len;

  return piece.release();
}

char CRegexXQuery_parser::myishex(char c)
{
  if((c >= '0') && (c <= '9'))
    return c-'0'+1;
  else if((c >= 'a') && (c <= 'f'))
    return c-'a'+10+1;
  else if((c >= 'A') && (c <= 'F'))
    return c-'a'+10+1;
  return 0;//not a hex
}

bool CRegexXQuery_parser::myisdigit(char c)
{
  return (c >= '0') && (c <= '9');
}

bool CRegexXQuery_parser::myisletterAZ(char c)
{
  return ((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z'));
}

static const unicode::code_point specials_extcp[] = {0xFFF0, 0xFFFD, 0};

static CRegexXQuery_parser::block_escape_t block_escape[] = 
{
{{0x0000, 0x007F}, NULL, "BasicLatin"},
{{0x0080, 0x00FF}, NULL, "Latin-1Supplement"},
{{0x0100, 0x017F}, NULL, "LatinExtended-A"},
{{0x0180, 0x024F}, NULL, "LatinExtended-B"},
{{0x0250, 0x02AF}, NULL, "IPAExtensions"},
{{0x02B0, 0x02FF}, NULL, "SpacingModifierLetters"},
{{0x0300, 0x036F}, NULL, "CombiningDiacriticalMarks"},
{{0x0370, 0x03FF}, NULL, "Greek"},
{{0x0400, 0x04FF}, NULL, "Cyrillic"},
{{0x0530, 0x058F}, NULL, "Armenian"},
{{0x0590, 0x05FF}, NULL, "Hebrew"},
{{0x0600, 0x06FF}, NULL, "Arabic"},
{{0x0700, 0x074F}, NULL, "Syriac"},
{{0x0780, 0x07BF}, NULL, "Thaana"},
{{0x0900, 0x097F}, NULL, "Devanagari"},
{{0x0980, 0x09FF}, NULL, "Bengali"},
{{0x0A00, 0x0A7F}, NULL, "Gurmukhi"},
{{0x0A80, 0x0AFF}, NULL, "Gujarati"},
{{0x0B00, 0x0B7F}, NULL, "Oriya"},
{{0x0B80, 0x0BFF}, NULL, "Tamil"},
{{0x0C00, 0x0C7F}, NULL, "Telugu"},
{{0x0C80, 0x0CFF}, NULL, "Kannada"},
{{0x0D00, 0x0D7F}, NULL, "Malayalam"},
{{0x0D80, 0x0DFF}, NULL, "Sinhala"},
{{0x0E00, 0x0E7F}, NULL, "Thai"},
{{0x0E80, 0x0EFF}, NULL, "Lao"},
{{0x0F00, 0x0FFF}, NULL, "Tibetan"},
{{0x1000, 0x109F}, NULL, "Myanmar"},
{{0x10A0, 0x10FF}, NULL, "Georgian"},
{{0x1100, 0x11FF}, NULL, "HangulJamo"},
{{0x1200, 0x137F}, NULL, "Ethiopic"},
{{0x13A0, 0x13FF}, NULL, "Cherokee"},
{{0x1400, 0x167F}, NULL, "UnifiedCanadianAboriginalSyllabics"},
{{0x1680, 0x169F}, NULL, "Ogham"},
{{0x16A0, 0x16FF}, NULL, "Runic"},
{{0x1780, 0x17FF}, NULL, "Khmer"},
{{0x1800, 0x18AF}, NULL, "Mongolian"},
{{0x1E00, 0x1EFF}, NULL, "LatinExtendedAdditional"},
{{0x1F00, 0x1FFF}, NULL, "GreekExtended"},
{{0x2000, 0x206F}, NULL, "GeneralPunctuation"},
{{0x2070, 0x209F}, NULL, "SuperscriptsandSubscripts"},
{{0x20A0, 0x20CF}, NULL, "CurrencySymbols"},
{{0x20D0, 0x20FF}, NULL, "CombiningMarksforSymbols"},
{{0x2100, 0x214F}, NULL, "LetterlikeSymbols"},
{{0x2150, 0x218F}, NULL, "NumberForms"},
{{0x2190, 0x21FF}, NULL, "Arrows"},
{{0x2200, 0x22FF}, NULL, "MathematicalOperators"},
{{0x2300, 0x23FF}, NULL, "MiscellaneousTechnical"},
{{0x2400, 0x243F}, NULL, "ControlPictures"},
{{0x2440, 0x245F}, NULL, "OpticalCharacterRecognition"},
{{0x2460, 0x24FF}, NULL, "EnclosedAlphanumerics"},
{{0x2500, 0x257F}, NULL, "BoxDrawing"},
{{0x2580, 0x259F}, NULL, "BlockElements"},
{{0x25A0, 0x25FF}, NULL, "GeometricShapes"},
{{0x2600, 0x26FF}, NULL, "MiscellaneousSymbols"},
{{0x2700, 0x27BF}, NULL, "Dingbats"},
{{0x2800, 0x28FF}, NULL, "BraillePatterns"},
{{0x2E80, 0x2EFF}, NULL, "CJKRadicalsSupplement"},
{{0x2F00, 0x2FDF}, NULL, "KangxiRadicals"},
{{0x2FF0, 0x2FFF}, NULL, "IdeographicDescriptionCharacters"},
{{0x3000, 0x303F}, NULL, "CJKSymbolsandPunctuation"},
{{0x3040, 0x309F}, NULL, "Hiragana"},
{{0x30A0, 0x30FF}, NULL, "Katakana"},
{{0x3100, 0x312F}, NULL, "Bopomofo"},
{{0x3130, 0x318F}, NULL, "HangulCompatibilityJamo"},
{{0x3190, 0x319F}, NULL, "Kanbun"},
{{0x31A0, 0x31BF}, NULL, "BopomofoExtended"},
{{0x3200, 0x32FF}, NULL, "EnclosedCJKLettersandMonths"},
{{0x3300, 0x33FF}, NULL, "CJKCompatibility"},
{{0x3400, 0x4DB5}, NULL, "CJKUnifiedIdeographsExtensionA"},
{{0x4E00, 0x9FFF}, NULL, "CJKUnifiedIdeographs"},
{{0xA000, 0xA48F}, NULL, "YiSyllables"},
{{0xA490, 0xA4CF}, NULL, "YiRadicals"},
{{0xAC00, 0xD7A3}, NULL, "HangulSyllables"},
{{0xE000, 0xF8FF}, NULL, "PrivateUse"},
{{0xF900, 0xFAFF}, NULL, "CJKCompatibilityIdeographs"},
{{0xFB00, 0xFB4F}, NULL, "AlphabeticPresentationForms"},
{{0xFB50, 0xFDFF}, NULL, "ArabicPresentationForms-A"},
{{0xFE20, 0xFE2F}, NULL, "CombiningHalfMarks"},
{{0xFE30, 0xFE4F}, NULL, "CJKCompatibilityForms"},
{{0xFE50, 0xFE6F}, NULL, "SmallFormVariants"},
{{0xFE70, 0xFEFE}, NULL, "ArabicPresentationForms-B"},
{{0xFEFF, 0xFEFF}, specials_extcp, "Specials"},
{{0xFF00, 0xFFEF}, NULL, "HalfwidthandFullwidthForms"}
};

CRegexXQuery_charmatch* CRegexXQuery_parser::readChar(const char *pattern, 
                                  int *char_len,
                                  enum CHARGROUP_t *multichar_type)
{
  char  c = 0;
  *char_len = 0;
  *multichar_type = CHARGROUP_NO_MULTICHAR;
  switch(pattern[*char_len])
  {
  case '\\':
  {
    (*char_len)++;
    switch(pattern[*char_len])
    {
    case 'n': c = '\n';(*char_len)++;return new CRegexXQuery_char_ascii(current_regex, c);
    case 'r': c = '\r';(*char_len)++;return new CRegexXQuery_char_ascii(current_regex, c);
    case 't': c = '\t';(*char_len)++;return new CRegexXQuery_char_ascii(current_regex, c);
    case '\\':
    case '/'://+
    case '|':
    case '.':
    case '?':
    case '*':
    case '+':
    case '(':
    case ')':
    case '{':
    case '}':
    case '-'://#x2D
    case '['://#x5B
    case ']'://#x5D
    case '^'://#x5E
    case '$'://+
       c = pattern[*char_len];
      (*char_len)++;
      *multichar_type = CHARGROUP_FLAGS_ONECHAR_ASCII;
      return new CRegexXQuery_char_ascii(current_regex, c);
    case 'p'://catEsc
    case 'P'://complEsc
    {
      //ignore the prop for now
      *multichar_type = CHARGROUP_FLAGS_MULTICHAR_p;//(CHARGROUP_t)((pattern[*char_len] == 'P') ? 128 : 0);
      bool is_reverse = (pattern[*char_len] == 'P');
      c = 0;
      if(pattern[(*char_len)+1] != '{')
      {
        throw XQUERY_EXCEPTION( err::FORX0002, ERROR_PARAMS(pattern, ZED(REGEX_BROKEN_P_CONSTRUCT)) );
      }
      (*char_len) += 2;
      switch(pattern[*char_len])
      {//IsCategory
      case 'L':
      {
        switch(pattern[(*char_len)+1])
        {
        case '}':
          c = unicode::Ll + 50;break;
        case 'u':
          c = unicode::Lu; (*char_len)++;break;
        case 'l':
          c = unicode::Ll; (*char_len)++;break;
        case 't':
          c = unicode::Lt; (*char_len)++;break;
        case 'm':
          c = unicode::Lm; (*char_len)++;break;
        case 'o':
          c = unicode::Lo; (*char_len)++;break;
        default:
          throw XQUERY_EXCEPTION( err::FORX0002, ERROR_PARAMS(pattern, ZED(REGEX_UNKNOWN_PL_CONSTRUCT)) );
        }
      }break;
      case 'M':
      {
        switch(pattern[(*char_len)+1])
        {
        case '}':
          c = unicode::Mc + 50;break;
        case 'n':
          c = unicode::Mn; (*char_len)++;break;
        case 'c':
          c = unicode::Mc; (*char_len)++;break;
        case 'e':
          c = unicode::Me; (*char_len)++;break;
        default:
          throw XQUERY_EXCEPTION( err::FORX0002, ERROR_PARAMS(pattern, ZED(REGEX_UNKNOWN_PM_CONSTRUCT)) );
        }
      }break;
      case 'N':
      {
        switch(pattern[(*char_len)+1])
        {
        case '}':
          c = unicode::Nd + 50;break;
        case 'd':
          c = unicode::Nd; (*char_len)++;break;
        case 'l':
          c = unicode::Nl; (*char_len)++;break;
        case 'o':
          c = unicode::No; (*char_len)++;break;
        default:
          throw XQUERY_EXCEPTION( err::FORX0002, ERROR_PARAMS(pattern, ZED(REGEX_UNKNOWN_PN_CONSTRUCT)) );
        }
      }break;
      case 'P':
      {
        switch(pattern[(*char_len)+1])
        {
        case '}':
          c = unicode::Pc + 50;break;
        case 'c':
          c = unicode::Pc; (*char_len)++;break;
        case 'd':
          c = unicode::Pd; (*char_len)++;break;
        case 's':
          c = unicode::Ps; (*char_len)++;break;
        case 'e':
          c = unicode::Pe; (*char_len)++;break;
        case 'i':
          c = unicode::Pi; (*char_len)++;break;
        case 'f':
          c = unicode::Pf; (*char_len)++;break;
        case 'o':
          c = unicode::Po; (*char_len)++;break;
        default:
          throw XQUERY_EXCEPTION( err::FORX0002, ERROR_PARAMS(pattern, ZED(REGEX_UNKNOWN_PP_CONSTRUCT)) );
        }
      }break;
      case 'Z':
      {
        switch(pattern[(*char_len)+1])
        {
        case '}':
          c = unicode::Zl + 50;break;
        case 's':
          c = unicode::Zs; (*char_len)++;break;
        case 'l':
          c = unicode::Zl; (*char_len)++;break;
        case 'p':
          c = unicode::Zp; (*char_len)++;break;
        default:
          throw XQUERY_EXCEPTION( err::FORX0002, ERROR_PARAMS(pattern, ZED(REGEX_UNKNOWN_PZ_CONSTRUCT)) );
        }
      }break;
      case 'S':
      {
        switch(pattern[(*char_len)+1])
        {
        case '}':
          c = unicode::Sc + 50;break;
        case 'm':
          c = unicode::Sm; (*char_len)++;break;
        case 'c':
          c = unicode::Sc; (*char_len)++;break;
        case 'k':
          c = unicode::Sk; (*char_len)++;break;
        case 'o':
          c = unicode::So; (*char_len)++;break;
        default:
          throw XQUERY_EXCEPTION( err::FORX0002, ERROR_PARAMS(pattern, ZED(REGEX_UNKNOWN_PS_CONSTRUCT)) );
        }
      }break;
      case 'C':
      {
        switch(pattern[(*char_len)+1])
        {
        case '}':
          c = unicode::Cc + 50;break;
        case 'c':
          c = unicode::Cc; (*char_len)++;break;
        case 'f':
          c = unicode::Cf; (*char_len)++;break;
        case 'o':
          c = unicode::Co; (*char_len)++;break;
        case 'n':
          c = unicode::Cn; (*char_len)++;break;
        default:
          throw XQUERY_EXCEPTION( err::FORX0002, ERROR_PARAMS(pattern, ZED(REGEX_UNKNOWN_PC_CONSTRUCT)) );
        }
      }break;
      }//end switch
      if(c)
      {
        if(pattern[(*char_len) + 1] != '}')
            throw XQUERY_EXCEPTION( err::FORX0002, ERROR_PARAMS(pattern, ZED(REGEX_BROKEN_P_CONSTRUCT)) );
        (*char_len)++;
        (*char_len)++;
        return new CRegexXQuery_multicharP(current_regex, c, is_reverse);
      }
      if(pattern[*char_len] == 'I')
      {
        if(pattern[(*char_len)+1] == 's')//IsBlock
        {
          *multichar_type = CHARGROUP_FLAGS_MULTICHAR_Is;
          (*char_len) += 2;
          zstring block_name;
          char tempc = pattern[(*char_len)];
          while(tempc && (tempc != '}'))
          {
            if(!myisletterAZ(tempc) && !myisdigit(tempc) && (tempc != '-'))
              throw XQUERY_EXCEPTION( err::FORX0002, ERROR_PARAMS(pattern, ZED(REGEX_BROKEN_PIs_CONSTRUCT)) );
            block_name.append(1, tempc);
            (*char_len)++;
            tempc = pattern[(*char_len)];
          }
          if(!tempc)
            throw XQUERY_EXCEPTION( err::FORX0002, ERROR_PARAMS(pattern, ZED(REGEX_BROKEN_PIs_CONSTRUCT)) );
          //search for the block name
          int i;
          int nr_blocks = sizeof(block_escape)/sizeof(CRegexXQuery_parser::block_escape_t);
          for(i=0;i<nr_blocks;i++)
          {
            if(compare_ascii_i(block_name.c_str(), block_escape[i].group_name))
            {
              c = i;
              break;
            }
          }
          if(i==nr_blocks)
            throw XQUERY_EXCEPTION( err::FORX0002, ERROR_PARAMS(pattern, ZED(REGEX_UNKNOWN_PIs_CONSTRUCT)) );
          (*char_len)++;
          return new CRegexXQuery_multicharIs(current_regex, i, is_reverse);
        }
        else
          throw XQUERY_EXCEPTION( err::FORX0002, ERROR_PARAMS(pattern, ZED(REGEX_BROKEN_PIs_CONSTRUCT)) );
      }
      else
      {
        throw XQUERY_EXCEPTION( err::FORX0002, ERROR_PARAMS(pattern, ZED(REGEX_BROKEN_P_CONSTRUCT)) );
      }
      break;//unreachable
    }//end case 'p'
      //multiCharEsc
    case 's':
    case 'S':
    case 'i':
    case 'I':
    case 'c':
    case 'C':
    case 'd':
    case 'D':
    case 'w':
    case 'W':
       *multichar_type = CHARGROUP_FLAGS_MULTICHAR_OTHER;
       c = pattern[*char_len];
       (*char_len)++;
       return new CRegexXQuery_multicharOther(current_regex, c);
    case 'u'://unicode codepoint \uXXXX
    {
      unicode::code_point utf8c = 0;
      (*char_len)++;
      for(int i=0;i<4;i++)
      {
        char hex = myishex(pattern[*char_len]);
        if(!hex)
        {
          throw XQUERY_EXCEPTION( err::FORX0002, ERROR_PARAMS(pattern, ZED(REGEX_INVALID_UNICODE_CODEPOINT_u)) );
        }
        utf8c <<= 4;
        utf8c |= (hex-1) & 0x0f;
        (*char_len)++;
      }
      return create_charmatch(utf8c, NULL, 0, multichar_type);
    }
    case 'U'://unicode codepoint \UXXXXXXXX
    {
      unicode::code_point utf8c = 0;
      (*char_len)++;
      for(int i=0;i<8;i++)
      {
        char hex = myishex(pattern[*char_len]);
        if(!hex)
        {
          throw XQUERY_EXCEPTION( err::FORX0002, ERROR_PARAMS(pattern, ZED(REGEX_INVALID_UNICODE_CODEPOINT_u)) );
        }
        utf8c <<= 4;
        utf8c |= (hex-1) & 0x0f;
        (*char_len)++;
      }
      return create_charmatch(utf8c, NULL, 0, multichar_type);
    }
    default:
      throw XQUERY_EXCEPTION( err::FORX0002, ERROR_PARAMS(pattern, ZED(REGEX_UNKNOWN_ESC_CHAR)) );
    }
    assert(false);
    break;//unreachable
  }//end case '\'
  default:
  {
    const char *temp_pattern = pattern;
    unicode::code_point utf8c = utf8::next_char(temp_pattern);
    (*char_len) = temp_pattern - pattern;
    return create_charmatch(utf8c, pattern, *char_len, multichar_type);
  }
  }
  return NULL;
}

CRegexXQuery_charmatch *CRegexXQuery_parser::create_charmatch(unicode::code_point utf8c,
                                                              const char *pattern, int utf8len,
                                                              enum CHARGROUP_t *multichar_type)
{
  if(utf8c <= 0x7F)
  {
    *multichar_type = CHARGROUP_FLAGS_ONECHAR_ASCII;
    if(flags & REGEX_ASCII_CASE_INSENSITIVE)
      return new CRegexXQuery_char_ascii_i(current_regex, (char)utf8c);
    else
      return new CRegexXQuery_char_ascii(current_regex, (char)utf8c);
  }
  else
  {
    *multichar_type = CHARGROUP_FLAGS_ONECHAR_UNICODE;
    if(flags & REGEX_ASCII_CASE_INSENSITIVE)
      return new CRegexXQuery_char_unicode_i(current_regex, utf8c);
    else
    {
      if(pattern)
        return new CRegexXQuery_char_unicode(current_regex, pattern, utf8len);
      else
        return new CRegexXQuery_char_unicode_cp(current_regex, utf8c);
    }
  }
}

IRegexAtom* CRegexXQuery_parser::read_atom(const char *pattern, int *atom_len)
{
  *atom_len = 0;
  if(flags & REGEX_ASCII_LITERAL)
  {
    unicode::code_point  utf8c;
    //bool is_end_line = false;
    const char *temp_pattern = pattern;
    utf8c = utf8::next_char(temp_pattern);
    *atom_len = temp_pattern - pattern;
    enum CHARGROUP_t multichar_type;
    return create_charmatch(utf8c, pattern, *atom_len, &multichar_type);
  }

  char c = *pattern;
  if(c == '\\')
  {
    //check for back reference
    if(myisdigit(pattern[(*atom_len)+1]))
    {
      (*atom_len)++;
      if(pattern[*atom_len] == '0')
      {
        throw XQUERY_EXCEPTION( err::FORX0002, ERROR_PARAMS(pattern, ZED(REGEX_INVALID_BACK_REF), 0, current_regex->subregex.size()) );
      }
      unsigned int backref = pattern[*atom_len] - '0';
      if((backref > current_regex->subregex.size()) ||
        (current_regex->subregex.at(backref-1)->flags != 0))
      {
        throw XQUERY_EXCEPTION( err::FORX0002, ERROR_PARAMS(pattern, ZED(REGEX_INVALID_BACK_REF), backref, current_regex->subregex.size()) );
      }
      while(current_regex->subregex.size() >= backref*10)
      {
        if(myisdigit(pattern[(*atom_len)+1]))
        {
          if(((backref*10+pattern[(*atom_len)+1]-'0') <= current_regex->subregex.size()) &&
            (current_regex->subregex.at(backref*10+pattern[(*atom_len)+1]-'0'-1)->flags == 0))
          {
            (*atom_len)++;
            backref = backref*10 + pattern[*atom_len]-'0';
          }
          else
            break;
        }
        else
          break;                        // the back-reference ends here
      }
      (*atom_len)++;
      return new CRegexXQuery_backref(current_regex, backref);
    }
  }
  if(c == '^')
  {
    (*atom_len)++;
    return new CRegexXQuery_pinstart(current_regex);
  }
  if((c == '}') || (c == '{') || (c == '?') || (c == '*') || (c == '+') || (c == '|'))
  {
    throw XQUERY_EXCEPTION( err::FORX0002, ERROR_PARAMS(pattern, ZED(REGEX_INVALID_ATOM_CHAR), c) );
  }
  switch(c)
  {
  case '[':
  {
    (*atom_len)++;
    CRegexXQuery_chargroup *chargroup = NULL;
    int chargroup_len;
    chargroup = readchargroup(pattern+*atom_len, &chargroup_len);
    *atom_len += chargroup_len;
    return chargroup;
  }
  case '.'://WildCharEsc
  {
    (*atom_len)++;
    return new CRegexXQuery_wildchar(current_regex);
  }
  case '('://begin an embedded reg exp
  {  
    (*atom_len)++;
    CRegexXQuery_regex *emb_regex = NULL;
    int   regex_len;
    emb_regex = parse_regexp(pattern + *atom_len, &regex_len);
    *atom_len += regex_len;
    return emb_regex;
  }
  case '$'://end line
    //is_end_line = true;
    (*atom_len)++;
    return new CRegexXQuery_endline(current_regex);
  default:
  {  
    //char  c;
    CRegexXQuery_charmatch *charmatch = NULL;
    int   c_len;
    CHARGROUP_t   multichar_type = CHARGROUP_NO_MULTICHAR;
    *atom_len = 0;
    while(pattern[*atom_len])
    {
      charmatch = readChar(pattern+*atom_len, &c_len, &multichar_type);
      *atom_len += c_len;
      if((flags & REGEX_ASCII_NO_WHITESPACE) && (multichar_type == CHARGROUP_FLAGS_ONECHAR_ASCII))
      {
        char c = (char)charmatch->get_c();
        if((c == ' ') || (c == '\t') || (c == '\r') || (c == '\n'))
        {
          //ignore this whitespace
          delete charmatch;
          continue;
        }
        else
          break;
      }
      else
        break;
    }
    /*
    std::unique_ptr<CRegexXQuery_chargroup> chargroup(new CRegexXQuery_chargroup(current_regex));
    if(multichar_type)
      chargroup->addMultiChar(c, multichar_type);
    else if(is_end_line)
      chargroup->addEndLine();
    else
      chargroup->addOneChar(c);
    *atom_len += c_len;
    return chargroup.release();
    */
    return charmatch;
  }
  }
}

//read until ']'
//posCharGroup  ::=   ( charRange | charClassEsc )+  
//charRange     ::=    seRange | XmlCharIncDash
CRegexXQuery_chargroup* CRegexXQuery_parser::readchargroup(const char *pattern, int *chargroup_len)
{
  std::unique_ptr<CRegexXQuery_chargroup> chargroup;
  *chargroup_len = 0;
  if(pattern[*chargroup_len] == '^')//negative group
  {
    (*chargroup_len)++;
    chargroup.reset(new CRegexXQuery_negchargroup(current_regex));
  }
  else
    chargroup.reset(new CRegexXQuery_chargroup(current_regex));
  while(pattern[*chargroup_len] && (pattern[*chargroup_len]!=']'))
  {
    //char  c1, c2;
    CHARGROUP_t  multichar_type = CHARGROUP_NO_MULTICHAR;
    int   c1_len;
    if((pattern[*chargroup_len] == '-') && (pattern[(*chargroup_len)+1] == '['))//charClassSub
    {
      int classsub_len;
      CRegexXQuery_chargroup *classsub = readchargroup(pattern + (*chargroup_len)+1 + 1, &classsub_len);
      if(!classsub)
      {
        throw XQUERY_EXCEPTION( err::FORX0002, ERROR_PARAMS(pattern, ZED(REGEX_INVALID_SUBCLASS)) );
      }
      chargroup->addClassSub(classsub);
      *chargroup_len += 2 + classsub_len + 1;
      if(pattern[*chargroup_len-1] != ']')
      {
        throw XQUERY_EXCEPTION( err::FORX0002, ERROR_PARAMS(pattern, ZED(REGEX_INVALID_USE_OF_SUBCLASS)) );
      }
      return chargroup.release();
    }

    std::unique_ptr<CRegexXQuery_charmatch> charmatch(readChar(pattern+*chargroup_len, &c1_len, &multichar_type));
    if((multichar_type == CHARGROUP_FLAGS_MULTICHAR_p) ||
      (multichar_type == CHARGROUP_FLAGS_MULTICHAR_Is) ||
      (multichar_type == CHARGROUP_FLAGS_MULTICHAR_OTHER))//first char is multichar
    {
      if((pattern[*chargroup_len+c1_len] == '-') &&///should not be a range
        (pattern[*chargroup_len+c1_len+1] != ']'))
      {
        throw XQUERY_EXCEPTION( err::FORX0002, ERROR_PARAMS(pattern, ZED(REGEX_MULTICHAR_IN_CHAR_RANGE)) );
      }
      //chargroup->addMultiChar(c1, multichar_type);
      chargroup->addCharMatch(charmatch.get());
      (void)charmatch.release(); // owned by chargroup now
      *chargroup_len += c1_len;
      continue;
    }
    (*chargroup_len) += c1_len;
    if(pattern[*chargroup_len] == '-')///might be a range
    {
      if(pattern[(*chargroup_len)+1] == ']')//no range, just the last char is '-'
      {
        //chargroup->addOneChar(c1);
        //chargroup->addOneChar('-');
        chargroup->addCharMatch(charmatch.get());
        (void)charmatch.release(); // owned by chargroup now
        chargroup->addCharMatch(new CRegexXQuery_char_ascii(current_regex, '-'));
        (*chargroup_len)++;
        continue;
      }
      else if(pattern[(*chargroup_len)+1] != '[')
      {
        //it is a range
        (*chargroup_len)++;
        std::unique_ptr<CRegexXQuery_charmatch>  charmatch2;
        CHARGROUP_t  multichar_type2 = CHARGROUP_NO_MULTICHAR;
        int  c2_len;
        charmatch2.reset(readChar(pattern+(*chargroup_len), &c2_len, &multichar_type2));
        if((multichar_type2 != CHARGROUP_FLAGS_ONECHAR_ASCII) &&
          (multichar_type2 != CHARGROUP_FLAGS_ONECHAR_ASCII))//second char in range is multichar
        {
          throw XQUERY_EXCEPTION( err::FORX0002, ERROR_PARAMS(pattern, ZED(REGEX_MULTICHAR_IN_CHAR_RANGE)) );
        }
        //chargroup->addCharRange(c1, c3);
        if((multichar_type == CHARGROUP_FLAGS_ONECHAR_ASCII) && (multichar_type2 == CHARGROUP_FLAGS_ONECHAR_ASCII))
        {
          if(flags & REGEX_ASCII_CASE_INSENSITIVE)
            chargroup->addCharMatch(new CRegexXQuery_char_range_ascii_i(current_regex, 
                                                                    (char)charmatch->get_c(),
                                                                    (char)charmatch2->get_c()));
          else
            chargroup->addCharMatch(new CRegexXQuery_char_range_ascii(current_regex, 
                                                                    (char)charmatch->get_c(),
                                                                    (char)charmatch2->get_c()));
        }
        else
        {
          if(flags & REGEX_ASCII_CASE_INSENSITIVE)
            chargroup->addCharMatch(new CRegexXQuery_char_range_unicode_i(current_regex, 
                                                                    charmatch->get_c(),
                                                                    charmatch2->get_c()));
          else
            chargroup->addCharMatch(new CRegexXQuery_char_range_unicode(current_regex, 
                                                                    charmatch->get_c(),
                                                                    charmatch2->get_c()));
        }
        *chargroup_len += c2_len;
        continue;
      }
    }
    //chargroup->addOneChar(c1);
    chargroup->addCharMatch(charmatch.get());
    (void)charmatch.release(); // owned by chargroup now
  }
  if(pattern[*chargroup_len])
    (*chargroup_len)++;
  else
  {
    throw XQUERY_EXCEPTION( err::FORX0002, ERROR_PARAMS(pattern, ZED(REGEX_MISSING_CLOSE_BRACKET)) );
  }
  return chargroup.release();
}

void CRegexXQuery_parser::read_quantifier(CRegexXQuery_piece *piece,
                                         const char *pattern, int *quantif_len)
{
  *quantif_len = 0;
  switch(pattern[*quantif_len])
  {
  case '?':
    piece->set_quantifier_min_max(0, 1, false);
    (*quantif_len)++;
    break;
  case '*':
    piece->set_quantifier_min_max(0, -1, false);
    (*quantif_len)++;
    break;
  case '+':
    piece->set_quantifier_min_max(1, -1, false);
    (*quantif_len)++;
    break;
  case '{':
  {
    int min = 0, max = 0;
    (*quantif_len)++;
    while(pattern[*quantif_len] && (pattern[*quantif_len] != ',') &&
      myisdigit(pattern[*quantif_len]) && (pattern[*quantif_len] != '}'))
    {
      min = min*10 + pattern[*quantif_len] - '0';
      (*quantif_len)++;
    }
    if(pattern[*quantif_len] == '}')
    {
      (*quantif_len)++;
      piece->set_quantifier_min_max(min, min, true);
      break;
    }
    if((pattern[*quantif_len] == ',') && (!isdigit(pattern[*quantif_len+1])))
    {
      (*quantif_len)++;
      piece->set_quantifier_min_max(min, -1, false);
    }
    else
    {
      (*quantif_len)++;
      while(pattern[*quantif_len] &&
        isdigit(pattern[*quantif_len]) && (pattern[*quantif_len] != '}'))
      {
        max = max*10 + pattern[*quantif_len] - '0';
        (*quantif_len)++;
      }
      if(max < min)
      {
        throw XQUERY_EXCEPTION( err::FORX0002, ERROR_PARAMS(pattern, ZED(REGEX_MAX_LT_MIN)) );
      }
      piece->set_quantifier_min_max(min, max, true);
    }
    while(pattern[*quantif_len] && (pattern[*quantif_len] != '}'))
    {
      (*quantif_len)++;
    }
    if(pattern[*quantif_len] == '}')
      (*quantif_len)++;
    break;
  }
  default:
    piece->set_quantifier_min_max(1, 1, false);
    break;
  }
  if(*quantif_len && (pattern[*quantif_len] == '?'))
  {
    piece->set_is_reluctant(true);
    (*quantif_len)++;
  }
  else
    piece->set_is_reluctant(false);
}


////////////////////////////
///Constructors and destructors and internal functions
////////////////////////////

CRegexXQuery_regex::CRegexXQuery_regex(CRegexXQuery_regex *topregex) : IRegexAtom(topregex?topregex:this)
{
  matched_source = NULL;
  matched_len = 0;
  capture_index = 0;
  cap_first = cap_end = 0;
  source_start = NULL;
  reachedEnd = false;
  flags = 128;//set to 0 after initialization
}

CRegexXQuery_regex::~CRegexXQuery_regex()
{
  for(CRegexXQuery_branch *branch : branch_list)
    delete branch;
}

void CRegexXQuery_regex::add_branch(CRegexXQuery_branch *branch)
{
  branch_list.push_back(branch);
}

bool  CRegexXQuery_regex::get_indexed_match(int index, 
                                           const char **matched_source, 
                                           int *matched_len)
{
  if(!index || index > (int)subregex.size())
    return false;
  CRegexXQuery_regex *subr = subregex[index-1];
  *matched_source = subr->matched_source;
  if(!*matched_source)
    return false;
  *matched_len = subr->matched_len;
  return true;
}

unsigned int CRegexXQuery_regex::get_indexed_regex_count()
{
  return subregex.size();
}

CRegexXQuery_branch::CRegexXQuery_branch(CRegexXQuery_regex* regex) 
      //:
      //IRegexMatcher(regex)
{
}

CRegexXQuery_branch::~CRegexXQuery_branch()
{
  for(CRegexXQuery_piece *piece : piece_list)
    delete piece;
}

void CRegexXQuery_branch::add_piece(CRegexXQuery_piece *piece)
{
  piece_list.push_back(piece);
}

CRegexXQuery_piece::CRegexXQuery_piece()
{
  atom = NULL;
  regex_atom = NULL;
}

CRegexXQuery_piece::~CRegexXQuery_piece()
{
  delete atom;
}

void CRegexXQuery_piece::set_atom(IRegexAtom *atom)
{
  this->atom = atom;
  this->regex_atom = dynamic_cast<CRegexXQuery_regex*>(atom);
}

void CRegexXQuery_piece::set_quantifier_min_max(int min, int max, bool strict_max)
{
  this->min = min;
  this->max = max;
  this->strict_max = strict_max;
}
void CRegexXQuery_piece::set_is_reluctant(bool is_reluctant)
{
  this->is_reluctant = is_reluctant;
}
void CRegexXQuery_piece::get_quantifier(int *min, int *max, bool *strict_max)
{
  *min = this->min;
  *max = this->max;
  *strict_max = this->strict_max;
}
bool CRegexXQuery_piece::get_is_reluctant()
{
  return is_reluctant;
}


CRegexXQuery_charmatch::CRegexXQuery_charmatch(CRegexXQuery_regex* regex) :
    IRegexAtom(regex)
{
}
CRegexXQuery_multicharP::CRegexXQuery_multicharP(CRegexXQuery_regex* regex, char type, bool is_reverse) :
    CRegexXQuery_charmatch(regex)
{
  this->multichar_type = type; this->is_reverse = is_reverse;
}
CRegexXQuery_multicharIs::CRegexXQuery_multicharIs(CRegexXQuery_regex* regex, int block_index, bool is_reverse) :
    CRegexXQuery_charmatch(regex)
{
  this->block_index = block_index; this->is_reverse = is_reverse;
}
CRegexXQuery_multicharOther::CRegexXQuery_multicharOther(CRegexXQuery_regex* regex, char type) :
    CRegexXQuery_charmatch(regex)
{
  this->multichar_type = type;
}
CRegexXQuery_char_ascii::CRegexXQuery_char_ascii(CRegexXQuery_regex* regex, char c) :
    CRegexXQuery_charmatch(regex)
{
  this->c = c;
}
CRegexXQuery_char_ascii_i::CRegexXQuery_char_ascii_i(CRegexXQuery_regex* regex, char c) :
    CRegexXQuery_char_ascii(regex, static_cast<char>(fold_case(static_cast<unsigned char>(c))))
{
}
CRegexXQuery_char_range_ascii::CRegexXQuery_char_range_ascii(CRegexXQuery_regex* regex, char c1, char c2) :
    CRegexXQuery_charmatch(regex)
{
  this->c1 = c1; this->c2 = c2;
}
CRegexXQuery_char_range_ascii_i::CRegexXQuery_char_range_ascii_i(CRegexXQuery_regex* regex, char c1, char c2) :
    CRegexXQuery_char_range_ascii(regex, c1, c2)
{
}
CRegexXQuery_char_unicode::CRegexXQuery_char_unicode(CRegexXQuery_regex* regex, const char *source, int len) :
    CRegexXQuery_charmatch(regex)
{
  this->len = len;
  memcpy(c, source, len);
}
CRegexXQuery_char_unicode_cp::CRegexXQuery_char_unicode_cp(CRegexXQuery_regex* regex, unicode::code_point c) :
    CRegexXQuery_charmatch(regex)
{
  this->c = c;
}
CRegexXQuery_char_unicode_i::CRegexXQuery_char_unicode_i(CRegexXQuery_regex* regex, unicode::code_point c) :
    CRegexXQuery_char_unicode_cp(regex, fold_case(c))
{
}
CRegexXQuery_char_range_unicode::CRegexXQuery_char_range_unicode(CRegexXQuery_regex* regex, unicode::code_point c1, unicode::code_point c2) :
    CRegexXQuery_charmatch(regex)
{
  this->c1 = c1; this->c2 = c2;
}
CRegexXQuery_char_range_unicode_i::CRegexXQuery_char_range_unicode_i(CRegexXQuery_regex* regex, unicode::code_point c1, unicode::code_point c2) :
    CRegexXQuery_char_range_unicode(regex, c1, c2)
{
}
CRegexXQuery_endline::CRegexXQuery_endline(CRegexXQuery_regex* regex) :
    CRegexXQuery_charmatch(regex)
{
}

unicode::code_point CRegexXQuery_char_unicode::get_c()
{
  const char *temp_c = (const char*)c;
  return utf8::next_char(temp_c);
}


CRegexXQuery_chargroup::CRegexXQuery_chargroup(CRegexXQuery_regex* regex) :
    IRegexAtom(regex)
{
  classsub = NULL;
}

CRegexXQuery_chargroup::~CRegexXQuery_chargroup()
{
  delete classsub;
  std::list<CRegexXQuery_charmatch* >::iterator charmatch_it;
  for(charmatch_it=chargroup_list.begin(); charmatch_it != chargroup_list.end(); charmatch_it++)
    delete (*charmatch_it);
}

void CRegexXQuery_chargroup::addCharMatch(CRegexXQuery_charmatch *charmatch)
{
  chargroup_list.push_back(charmatch);
}
void CRegexXQuery_chargroup::addClassSub(CRegexXQuery_chargroup* classsub)
{
  this->classsub = classsub;
}

CRegexXQuery_negchargroup::CRegexXQuery_negchargroup(CRegexXQuery_regex* regex) :
  CRegexXQuery_chargroup(regex)
{
}

CRegexXQuery_negchargroup::~CRegexXQuery_negchargroup()
{
}

CRegexXQuery_wildchar::CRegexXQuery_wildchar(CRegexXQuery_regex* regex) :
      IRegexAtom(regex)
{
}

CRegexXQuery_wildchar::~CRegexXQuery_wildchar()
{
}

CRegexXQuery_backref::CRegexXQuery_backref(CRegexXQuery_regex* regex, unsigned int backref_) :
      IRegexAtom(regex),
      backref(backref_)
{
}

CRegexXQuery_backref::~CRegexXQuery_backref()
{
}

CRegexXQuery_pinstart::CRegexXQuery_pinstart(CRegexXQuery_regex* regex):
      IRegexAtom(regex)
{
}

CRegexXQuery_parser::CRegexXQuery_parser()
{
  current_regex = NULL;
  regex_depth = 0;
}

CRegexXQuery_parser::~CRegexXQuery_parser()
{
}


//////////////////////////////////////////
////Matching the pattern on a string
/////////////////////////////////////////
namespace {

/**
 * A non-owning reference to a continuation: what remains to be matched after
 * the current construct, given the position where the construct ended.
 * The referenced callable lives in a caller's stack frame, which outlives
 * every call made through the reference.
 */
class cont_ref {
public:
  template<class F,class = typename std::enable_if<
    !std::is_same<typename std::decay<F>::type,cont_ref>::value>::type>
  cont_ref( F const &f ) :
    obj_( &f ),
    fn_( []( void const *obj, char const *p ) {
      return (*static_cast<F const*>( obj ))( p );
    } )
  {
  }

  bool operator()( char const *p ) const {
    return fn_( obj_, p );
  }

private:
  void const *obj_;
  bool (*fn_)( void const*, char const* );
};

//
// Bound on the work for one search, in matching steps, so that catastrophic
// backtracking in a user pattern raises an error instead of hanging (the ICU
// matcher has a time limit for the same reason).
//
unsigned long const max_match_steps = 100000000ul;

//
// Bound on the stack the recursive matcher may use for one search: each
// iteration of a repeated group nests a few calls.
//
std::uintptr_t const max_match_stack = 2u * 1024u * 1024u;

} // anonymous namespace

/**
 * Backtracking matcher in continuation-passing style: every match_* function
 * matches its construct at \a p in every possible way, in priority order, and
 * calls \a k with the end of each way until \a k returns true.  Captures are
 * stored in the groups (CRegexXQuery_regex::matched_source/matched_len) and
 * restored when a way is abandoned.
 */
class CRegexXQuery_matcher {
public:
  CRegexXQuery_matcher( CRegexXQuery_regex *top ) : top_( top ), steps_( 0 ) {
    char here;
    stack_base_ = reinterpret_cast<std::uintptr_t>( &here );
  }

  bool match_group( CRegexXQuery_regex *re, char const *p, cont_ref k );

private:
  CRegexXQuery_regex *top_;
  unsigned long steps_;
  std::uintptr_t stack_base_;

  typedef std::pair<char const*,int> capture;

  void check_limits();
  void save_and_clear( CRegexXQuery_regex const *re, std::vector<capture> *saved );
  void restore( CRegexXQuery_regex const *re, std::vector<capture> const &saved );
  bool match_seq( CRegexXQuery_branch const *b, size_t i, char const *p, cont_ref k );
  bool match_piece( CRegexXQuery_piece const *piece, char const *p, cont_ref k );
  bool match_repeat( CRegexXQuery_piece const *piece, char const *p, int n, cont_ref k );
};

void CRegexXQuery_matcher::check_limits() {
  if ( ++steps_ > max_match_steps )
    throw XQUERY_EXCEPTION(
      zerr::ZXQP0001_DYNAMIC_RUNTIME_ERROR,
      ERROR_PARAMS( "regular expression match exceeded the time limit" )
    );
  char here;
  std::uintptr_t const sp = reinterpret_cast<std::uintptr_t>( &here );
  std::uintptr_t const used = sp < stack_base_ ? stack_base_ - sp : sp - stack_base_;
  if ( used > max_match_stack )
    throw XQUERY_EXCEPTION(
      zerr::ZXQP0001_DYNAMIC_RUNTIME_ERROR,
      ERROR_PARAMS( "regular expression match too deeply nested" )
    );
}

void CRegexXQuery_matcher::save_and_clear( CRegexXQuery_regex const *re,
                                           std::vector<capture> *saved ) {
  for ( int i = re->cap_first; i < re->cap_end; ++i ) {
    CRegexXQuery_regex *const g = top_->subregex[i];
    saved->push_back( capture( g->matched_source, g->matched_len ) );
    g->matched_source = NULL;
    g->matched_len = 0;
  }
}

void CRegexXQuery_matcher::restore( CRegexXQuery_regex const *re,
                                    std::vector<capture> const &saved ) {
  for ( int i = re->cap_first; i < re->cap_end; ++i ) {
    CRegexXQuery_regex *const g = top_->subregex[i];
    g->matched_source = saved[ i - re->cap_first ].first;
    g->matched_len = saved[ i - re->cap_first ].second;
  }
}

bool CRegexXQuery_matcher::match_group( CRegexXQuery_regex *re, char const *p,
                                        cont_ref k ) {
  check_limits();
  for ( CRegexXQuery_branch const *b : re->branch_list ) {
    if ( re->capture_index ) {
      auto const set_capture = [&]( char const *q ) -> bool {
        char const *const old_source = re->matched_source;
        int const old_len = re->matched_len;
        re->matched_source = p;
        re->matched_len = static_cast<int>( q - p );
        if ( k( q ) )
          return true;
        re->matched_source = old_source;
        re->matched_len = old_len;
        return false;
      };
      if ( match_seq( b, 0, p, set_capture ) )
        return true;
    } else if ( match_seq( b, 0, p, k ) )
      return true;
  }
  return false;
}

bool CRegexXQuery_matcher::match_seq( CRegexXQuery_branch const *b, size_t i,
                                      char const *p, cont_ref k ) {
  if ( i == b->piece_list.size() )
    return k( p );
  auto const rest = [&]( char const *q ) -> bool {
    return match_seq( b, i + 1, q, k );
  };
  return match_piece( b->piece_list[i], p, rest );
}

bool CRegexXQuery_matcher::match_piece( CRegexXQuery_piece const *piece,
                                        char const *p, cont_ref k ) {
  if ( piece->regex_atom )
    return match_repeat( piece, p, 0, k );

  check_limits();
  IRegexAtom *const atom = piece->atom;
  int const min = piece->min;
  int const max = piece->max;
  int len, unused;

  if ( min == 1 && max == 1 ) {
    unused = 0;
    if ( !atom->match_internal( p, &unused, &len ) )
      return false;
    return k( p + len );
  }

  if ( piece->is_reluctant ) {
    char const *q = p;
    for ( int n = 0; ; ++n ) {
      if ( n >= min && k( q ) )
        return true;
      if ( max >= 0 && n >= max )
        return false;
      unused = 0;
      if ( !atom->match_internal( q, &unused, &len ) )
        return false;
      if ( !len ) {
        //
        // An empty match (e.g., a back-reference to an empty group) repeats
        // without moving: the remaining minimum is met right here, and if it
        // already was, k(q) has just failed.
        //
        return n < min && k( q );
      }
      q += len;
      check_limits();
    }
  }

  //
  // Greedy: collect the end of each repetition, then try the longest first.
  //
  std::vector<char const*> ends( 1, p );
  bool empty_match = false;
  while ( max < 0 || static_cast<int>( ends.size() ) - 1 < max ) {
    unused = 0;
    if ( !atom->match_internal( ends.back(), &unused, &len ) )
      break;
    if ( !len ) {
      empty_match = true;               // repeats any number of times
      break;
    }
    ends.push_back( ends.back() + len );
  }
  int const count = static_cast<int>( ends.size() ) - 1;
  if ( empty_match ) {
    if ( k( ends.back() ) )             // also covers any n >= min
      return true;
    for ( int n = count - 1; n >= min; --n )
      if ( k( ends[n] ) )
        return true;
    return false;
  }
  for ( int n = count; n >= min; --n )
    if ( k( ends[n] ) )
      return true;
  return false;
}

bool CRegexXQuery_matcher::match_repeat( CRegexXQuery_piece const *piece,
                                         char const *p, int n, cont_ref k ) {
  check_limits();
  CRegexXQuery_regex *const group = piece->regex_atom;
  int const min = piece->min;
  int const max = piece->max;

  auto const iterate = [&]() -> bool {
    if ( max >= 0 && n >= max )
      return false;
    //
    // XQuery F&O 3.0 5.6.1: the groups inside a repeated group capture what
    // they match in the current iteration; one that does not participate in
    // it has no captured substring, so a back-reference to it matches the
    // empty string.
    //
    std::vector<capture> saved;
    save_and_clear( group, &saved );
    auto const next = [&]( char const *q ) -> bool {
      //
      // Once the minimum is met, an iteration that matched the empty string
      // can not lead anywhere new; repeating it would loop forever.
      //
      if ( q == p && n >= min )
        return false;
      return match_repeat( piece, q, n + 1, k );
    };
    if ( match_group( group, p, next ) )
      return true;
    restore( group, saved );
    return false;
  };

  if ( piece->is_reluctant ) {
    if ( n >= min && k( p ) )
      return true;
    return iterate();
  }
  if ( iterate() )
    return true;
  return n >= min && k( p );
}

bool CRegexXQuery_regex::match_anywhere(const char *source, unsigned int flags,
                                       int *match_pos, int *matched_len)
{
  return search(source, 0, flags, match_pos, matched_len);
}

bool CRegexXQuery_regex::search(const char *subject, int from,
                                unsigned int flags,
                                int *match_pos, int *matched_len)
{
  this->flags = flags;
  this->source_start = subject;
  reachedEnd = false;

  bool const whole = (flags & REGEX_ASCII_WHOLE_MATCH) != 0;
  char const *match_end = NULL;
  auto const done = [&]( char const *q ) -> bool {
    if ( whole && *q )
      return false;
    match_end = q;
    return true;
  };

  CRegexXQuery_matcher matcher( this );
  char const *p = subject + from;
  for ( ;; ) {
    for ( CRegexXQuery_regex *g : subregex ) {
      g->matched_source = NULL;
      g->matched_len = 0;
    }
    if ( matcher.match_group( this, p, done ) ) {
      *match_pos = static_cast<int>( p - subject );
      *matched_len = static_cast<int>( match_end - p );
      return true;
    }
    if ( whole || !*p )
      break;
    // next character (do not step over the terminating NUL)
    ++p;
    while ( (*p & 0xC0) == 0x80 )
      ++p;
  }
  for ( CRegexXQuery_regex *g : subregex ) {
    g->matched_source = NULL;
    g->matched_len = 0;
  }
  return false;
}

bool CRegexXQuery_multicharP::match_internal(const char *source, int *start_from_branch, int *matched_len)
{
  if(!source[0])
  {
    regex_intern->set_reachedEnd(source);
    return false;
  }
  bool found = false;
  const char *temp_source = source;
  unicode::code_point utf8c = utf8::next_char(temp_source);
  switch(multichar_type)
  {
  case unicode::Ll + 50:
    if(unicode::is_category(utf8c, unicode::Ll) ||
        unicode::is_category(utf8c, unicode::Lm) ||
        unicode::is_category(utf8c, unicode::Lo) ||
        unicode::is_category(utf8c, unicode::Lt) ||
        unicode::is_category(utf8c, unicode::Lu))
    {
      if(!is_reverse)
        found = true;
    }
    else
    {
      if(is_reverse)
        found = true;
    }
    break;
  case unicode::Mc + 50:
    if(unicode::is_category(utf8c, unicode::Mn) ||
        unicode::is_category(utf8c, unicode::Mc) ||
        unicode::is_category(utf8c, unicode::Me))
    {
      if(!is_reverse)
        found = true;
    }
    else
    {
      if(is_reverse)
        found = true;
    }
    break;
  case unicode::Nd + 50:
    if(unicode::is_category(utf8c, unicode::Nd) ||
        unicode::is_category(utf8c, unicode::Nl) ||
        unicode::is_category(utf8c, unicode::No))
    {
      if(!is_reverse)
        found = true;
    }
    else
    {
      if(is_reverse)
        found = true;
    }
    break;
  case unicode::Pc + 50:
    if(unicode::is_category(utf8c, unicode::Pc) ||
        unicode::is_category(utf8c, unicode::Pd) ||
        unicode::is_category(utf8c, unicode::Ps) ||
        unicode::is_category(utf8c, unicode::Pe) ||
        unicode::is_category(utf8c, unicode::Pi) ||
        unicode::is_category(utf8c, unicode::Pf) ||
        unicode::is_category(utf8c, unicode::Po))
    {
      if(!is_reverse)
        found = true;
    }
    else
    {
      if(is_reverse)
        found = true;
    }
    break;
  case unicode::Zl + 50:
    if(unicode::is_category(utf8c, unicode::Zs) ||
        unicode::is_category(utf8c, unicode::Zl) ||
        unicode::is_category(utf8c, unicode::Zp))
    {
      if(!is_reverse)
        found = true;
    }
    else
    {
      if(is_reverse)
        found = true;
    }
    break;
  case unicode::Sc + 50:
    if(unicode::is_category(utf8c, unicode::Sm) ||
        unicode::is_category(utf8c, unicode::Sc) ||
        unicode::is_category(utf8c, unicode::Sk) ||
        unicode::is_category(utf8c, unicode::So))
    {
      if(!is_reverse)
        found = true;
    }
    else
    {
      if(is_reverse)
        found = true;
    }
    break;
  case unicode::Cc + 50:
    if(unicode::is_category(utf8c, unicode::Cc) ||
        unicode::is_category(utf8c, unicode::Cf) ||
        unicode::is_category(utf8c, unicode::Co))//ignore unicode::Cn
    {
      if(!is_reverse)
        found = true;
    }
    else
    {
      if(is_reverse)
        found = true;
    }
    break;
  default:
    if(unicode::is_category(utf8c, (unicode::category)multichar_type))
    {
      if(!is_reverse)
        found = true;
    }
    else
    {
      if(is_reverse)
        found = true;
    }
    break;
  }

  if(found)
  {
    *matched_len = temp_source - source;
  }
  return found;
}

bool CRegexXQuery_multicharIs::match_internal(const char *source, int *start_from_branch, int *matched_len)
{
  if(!source[0])
  {
    regex_intern->set_reachedEnd(source);
    return false;
  }
  bool found = false;
  const char *temp_source = source;
  unicode::code_point utf8c = utf8::next_char(temp_source);
  const unicode::code_point *cp = block_escape[block_index].cp;
  if((utf8c >= cp[0]) && (utf8c <= cp[1]))
  {
    if(!is_reverse)
      found = true;
  }
  else if(block_escape[block_index].ext_cp)
  {
    cp = block_escape[block_index].ext_cp;
    while(*cp)
    {
      if((utf8c >= cp[0]) && (utf8c <= cp[1]))
        break;
      cp += 2;
    }
    if(*cp)
    {
      if(!is_reverse)
        found = true;
    }
    else
    {
      if(is_reverse)
        found = true;
    }
  }
  else
  {
    if(is_reverse)
      found = true;
  }
  if(found)
  {
    *matched_len = temp_source - source;
  }
  return found;
}

bool CRegexXQuery_multicharOther::match_internal(const char *source, int *start_from_branch, int *matched_len)
{
  if(!source[0])
  {
    regex_intern->set_reachedEnd(source);
    return false;
  }
  bool found = false;
  bool value_true = true;
  const char *temp_source = source;
  unicode::code_point utf8c = utf8::next_char(temp_source);
  switch(multichar_type)
  {
    case 'S':value_true = false;//[^\s]
    case 's'://[#x20\t\n\r]
      switch(utf8c)
      {
      case '\t':
      case '\r':
      case '\n':
      case ' ':
        found = true;
      default:
        break;
      }
      break;
    case 'I':value_true = false;//[^\i]
    case 'i'://the set of initial name characters, those matched by Letter | '_' | ':'
      if((utf8c == '_') ||
        (utf8c == ':') ||
        XQCharType::isLetter(utf8c))
      {
        found = true;
      }
      break;
    case 'C':value_true = false;//[^\c]
    case 'c'://the set of name characters, those matched by NameChar
      if(XQCharType::isNameChar(utf8c))
      {
        found = true;
      }
      break;
    case 'D':value_true = false;//[^\d]
    case 'd':
      if(unicode::is_category(utf8c, unicode::Nd))
        found = true;
      break;
    case 'W':value_true = false;//[^\w]
    case 'w':
      found = !(unicode::is_category(utf8c, unicode::Pc) ||
                unicode::is_category(utf8c, unicode::Pd) ||
                unicode::is_category(utf8c, unicode::Ps) ||
                unicode::is_category(utf8c, unicode::Pe) ||
                unicode::is_category(utf8c, unicode::Pi) ||
                unicode::is_category(utf8c, unicode::Pf) ||
                unicode::is_category(utf8c, unicode::Po) ||
                unicode::is_category(utf8c, unicode::Zs) ||
                unicode::is_category(utf8c, unicode::Zl) ||
                unicode::is_category(utf8c, unicode::Zp) ||
                unicode::is_category(utf8c, unicode::Cc) ||
                unicode::is_category(utf8c, unicode::Cf) ||
                unicode::is_category(utf8c, unicode::Co));//ignore unicode::Cn
      break;
    default:
      throw XQUERY_EXCEPTION( err::FORX0002, ERROR_PARAMS(source, ZED(REGEX_UNIMPLEMENTED)) );
  }
  if((found && value_true) || (!found && !value_true))
  {
    *matched_len = temp_source - source;
    return true;
  }
  else
  {
    return false;
  }
}

bool CRegexXQuery_char_ascii::match_internal(const char *source, int *start_from_branch, int *matched_len)
{
  if(!source[0])
  {
    regex_intern->set_reachedEnd(source);
    return false;
  }
  if(source[0] == c)
  {
    *matched_len = 1;
    return true;
  }
  else
    return false;
}

bool CRegexXQuery_char_ascii_i::match_internal(const char *source, int *start_from_branch, int *matched_len)
{
  if(!source[0])
  {
    regex_intern->set_reachedEnd(source);
    return false;
  }
  const char *temp_source = source;
  unicode::code_point const sc = utf8::next_char(temp_source);
  if(fold_case(sc) != static_cast<unsigned char>(c))
    return false;
  *matched_len = static_cast<int>(temp_source - source);
  return true;
}

bool CRegexXQuery_char_range_ascii::match_internal(const char *source, int *start_from_branch, int *matched_len)
{
  if(!source[0])
  {
    regex_intern->set_reachedEnd(source);
    return false;
  }
  if((source[0] >= c1) && (source[0] <= c2))
  {
    *matched_len = 1;
    return true;
  }
  else
    return false;
}

bool CRegexXQuery_char_range_ascii_i::match_internal(const char *source, int *start_from_branch, int *matched_len)
{
  if(!source[0])
  {
    regex_intern->set_reachedEnd(source);
    return false;
  }
  const char *temp_source = source;
  unicode::code_point const sc = utf8::next_char(temp_source);
  if(!in_range_i(sc, static_cast<unsigned char>(c1), static_cast<unsigned char>(c2)))
    return false;
  *matched_len = static_cast<int>(temp_source - source);
  return true;
}

bool CRegexXQuery_char_unicode::match_internal(const char *source, int *start_from_branch, int *matched_len)
{
  if(!source[0])
  {
    regex_intern->set_reachedEnd(source);
    return false;
  }
  if(!memcmp(source, c, len))
  {
    *matched_len = len;
    return true;
  }
  else
    return false;
}

bool CRegexXQuery_char_unicode_cp::match_internal(const char *source, int *start_from_branch, int *matched_len)
{
  if(!source[0])
  {
    regex_intern->set_reachedEnd(source);
    return false;
  }
  const char *temp_source = source;
  unicode::code_point  utf8c = utf8::next_char(temp_source);
  if(utf8c == c)
  {
    *matched_len = temp_source - source;
    return true;
  }
  else
    return false;
}

bool CRegexXQuery_char_unicode_i::match_internal(const char *source, int *start_from_branch, int *matched_len)
{
  if(!source[0])
  {
    regex_intern->set_reachedEnd(source);
    return false;
  }
  const char *temp_source = source;
  unicode::code_point const sc = utf8::next_char(temp_source);
  if(fold_case(sc) != c)
    return false;
  *matched_len = static_cast<int>(temp_source - source);
  return true;
}

bool CRegexXQuery_char_range_unicode::match_internal(const char *source, int *start_from_branch, int *matched_len)
{
  if(!source[0])
  {
    regex_intern->set_reachedEnd(source);
    return false;
  }
  const char *temp_source = source;
  unicode::code_point  utf8c = utf8::next_char(temp_source);
  if((utf8c >= c1) && (utf8c <= c2))
  {
    *matched_len = temp_source - source;
    return true;
  }
  else
    return false;
}

bool CRegexXQuery_char_range_unicode_i::match_internal(const char *source, int *start_from_branch, int *matched_len)
{
  if(!source[0])
  {
    regex_intern->set_reachedEnd(source);
    return false;
  }
  const char *temp_source = source;
  unicode::code_point const sc = utf8::next_char(temp_source);
  if(!in_range_i(sc, c1, c2))
    return false;
  *matched_len = static_cast<int>(temp_source - source);
  return true;
}

bool CRegexXQuery_endline::match_internal(const char *source, int *start_from_branch, int *matched_len)
{
  *matched_len = 0;
  if(!source[0])
  {
  //  regex_intern->reachedEnd = true;
    return true;
  }
  if((source[0] == 0x0A) || ((source[0] == 0x0D) && (source[1] == 0x0A)))
  {
    if(regex_intern->get_flags() & REGEX_ASCII_MULTILINE)
    {
    //  regex_intern->reachedEnd = true;
      return true;
    }
  }
  return false;
}


//match any of chargroups
bool CRegexXQuery_chargroup::match_internal(const char *source, int *start_from_branch, int *matched_len)
{
  *matched_len = 0;
  std::list<CRegexXQuery_charmatch* >::iterator  cgt_it;
/*
  if(!source[0])
  {
    regex_intern->reachedEnd = true;
    if(chargroup_list.empty())
      return true;
    else if((chargroup_list.size() == 1) && (chargroup_list.begin()->flags == CHARGROUP_FLAGS_ENDLINE))
      return true;
    else
      return false;
  }

  if((source[0] == 0x0A) || ((source[0] == 0x0D) && (source[1] == 0x0A)))
  {
    if((regex_intern->flags & REGEX_ASCII_MULTILINE) &&
        (chargroup_list.size() == 1) && (chargroup_list.begin()->flags == CHARGROUP_FLAGS_ENDLINE))
    {
      // *matched_len = 1;
      return true;
    }
  }
*/
  //bool found = false;
  for(cgt_it = chargroup_list.begin(); cgt_it != chargroup_list.end(); cgt_it++)
  {
    if((*cgt_it)->match_internal(source, start_from_branch, matched_len))
      break;
  }
  if(cgt_it == chargroup_list.end())
    return false;

  if(classsub)
  {
    int   classsub_len;
    if(classsub->match_internal(source, NULL, &classsub_len))
      return false;
  }

  //*matched_len = 1;
  return true;
}

bool CRegexXQuery_negchargroup::match_internal(const char *source, int *start_from_branch, int *matched_len)
{
  if(!source[0])
  {
    regex_intern->set_reachedEnd(source);
    return false;
  }
  if(!CRegexXQuery_chargroup::match_internal(source, start_from_branch, matched_len))
  {
    *matched_len = myutf8len(source);
    return true;
  }
  return false;
}

bool CRegexXQuery_wildchar::match_internal(const char *source, int *start_from_branch, int *matched_len)
{
  *matched_len = 0;
  if(!source[0])
  {
    regex_intern->set_reachedEnd(source);
    return false;
  }
  if((regex_intern->flags & REGEX_ASCII_DOTALL) || 
    ((source[0] != '\n') && (source[0] != '\r')))
  {
    *matched_len = myutf8len(source);
    return true;
  }
  else
    return false;
}

bool CRegexXQuery_backref::match_internal(const char *source, int *start_from_branch, int *matched_len)
{
  const char *submatch = regex_intern->subregex.at(backref-1)->matched_source;
  if(!submatch)
  {
    *matched_len = 0;
    return true;
  }
  int const sub_len = regex_intern->subregex.at(backref-1)->matched_len;
  char const *const sub_end = submatch + sub_len;
  char const *s = source;
  if(regex_intern->flags & REGEX_ASCII_CASE_INSENSITIVE)
  {
    //
    // Compare case foldings character by character: the matched text may
    // have another length in bytes, e.g., KELVIN SIGN (3 bytes) for 'k'.
    //
    while(submatch < sub_end)
    {
      if(!*s)
      {
        regex_intern->set_reachedEnd(s);
        return false;
      }
      if(fold_case(utf8::next_char(s)) != fold_case(utf8::next_char(submatch)))
        return false;
    }
  }
  else
  {
    for(; submatch < sub_end; ++submatch, ++s)
    {
      if(!*s)
      {
        regex_intern->set_reachedEnd(s);
        return false;
      }
      if(*s != *submatch)
        return false;
    }
  }
  *matched_len = static_cast<int>(s - source);
  return true;
}

bool CRegexXQuery_pinstart::match_internal(const char *source, int *start_from_branch, int *matched_len)
{
  *matched_len = 0;
  if(source == regex_intern->source_start)
    return true;
  if((regex_intern->flags & REGEX_ASCII_MULTILINE) &&
    ((source[-1] == '\n') || (source[-1] == '\r')))
    return true;
  
  return false;
}

  }//end namespace regex_engine
}//end namespace zorba
/* vim:set et sw=2 ts=2: */
