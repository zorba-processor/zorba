// A Bison parser, made by GNU Bison 3.8.2.

// Skeleton interface for Bison LALR(1) parsers in C++

// Copyright (C) 2002-2015, 2018-2021 Free Software Foundation, Inc.

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

// As a special exception, you may create a larger work that contains
// part or all of the Bison parser skeleton and distribute that work
// under terms of your choice, so long as that work isn't itself a
// parser generator using the skeleton or a modified version thereof
// as a parser skeleton.  Alternatively, if you modify or redistribute
// the parser skeleton itself, you may (at your option) remove this
// special exception, which will cause the skeleton and the resulting
// Bison output files to be licensed under the GNU General Public
// License without this special exception.

// This special exception was added by the Free Software Foundation in
// version 2.2 of Bison.


/**
 ** \file xquery_parser.hpp
 ** Define the zorba::parser class.
 */

// C++ LALR(1) parser skeleton written by Akim Demaille.

// DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
// especially those whose name start with YY_ or yy_.  They are
// private implementation details that can be changed or removed.

#ifndef YY_YY_XQUERY_PARSER_HPP_INCLUDED
# define YY_YY_XQUERY_PARSER_HPP_INCLUDED
// "%code requires" blocks.


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

#include <list>
#include <string>
#include <vector>
#include <zorba/internal/unique_ptr.h>

#include "compiler/parsetree/parsenodes.h"
#include "compiler/parser/zorba_parser_error.h"
#include "zorbatypes/zstring.h"

#ifdef __GNUC__
    // disable a warning in location.hh which comes with bison
    // position.hh:141: warning: suggest parentheses around && within ||
#   pragma GCC diagnostic ignored "-Wparentheses"
#endif

#include "location.hh"

#ifdef __GNUC__
#  pragma GCC diagnostic warning "-Wparentheses"
#endif

typedef std::list<zorba::zstring> string_list_t;
typedef std::pair<zorba::zstring,zorba::zstring> string_pair_t;





# include <cstdlib> // std::abort
# include <iostream>
# include <stdexcept>
# include <string>
# include <vector>

#if defined __cplusplus
# define YY_CPLUSPLUS __cplusplus
#else
# define YY_CPLUSPLUS 199711L
#endif

// Support move semantics when possible.
#if 201103L <= YY_CPLUSPLUS
# define YY_MOVE           std::move
# define YY_MOVE_OR_COPY   move
# define YY_MOVE_REF(Type) Type&&
# define YY_RVREF(Type)    Type&&
# define YY_COPY(Type)     Type
#else
# define YY_MOVE
# define YY_MOVE_OR_COPY   copy
# define YY_MOVE_REF(Type) Type&
# define YY_RVREF(Type)    const Type&
# define YY_COPY(Type)     const Type&
#endif

// Support noexcept when possible.
#if 201103L <= YY_CPLUSPLUS
# define YY_NOEXCEPT noexcept
# define YY_NOTHROW
#else
# define YY_NOEXCEPT
# define YY_NOTHROW throw ()
#endif

// Support constexpr when possible.
#if 201703 <= YY_CPLUSPLUS
# define YY_CONSTEXPR constexpr
#else
# define YY_CONSTEXPR
#endif
# include "location.hh"


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif

namespace zorba {




  /// A Bison parser.
  class xquery_parser
  {
  public:
#ifdef YYSTYPE
# ifdef __GNUC__
#  pragma GCC message "bison: do not #define YYSTYPE in C++, use %define api.value.type"
# endif
    typedef YYSTYPE value_type;
#else
    /// Symbol semantic values.
    union value_type
    {

    zorba::parsenode *node;
    zorba::exprnode *expr;
    zorba::FunctionSig *fnsig;
    zorba::VarNameAndType *varnametype;
    off_t sval;
    const char *strval;
    xs_integer *ival;
    xs_double *dval;
    xs_decimal *decval;
    ZorbaParserError *err;
    string_list_t *strlist;
    string_pair_t *strpair;
    std::vector<string_pair_t> *vstrpair;
    CatchExpr::NameTestList *name_test_list;
    FTStopWordOption::incl_excl_list_t *incl_excl_list;
    FTSelection::pos_filter_list_t *pos_filter_list;
    FTThesaurusOption::thesaurus_id_list_t *thesaurus_id_list;
    ft_anyall_mode::type ft_anyall_value;


    };
#endif
    /// Backward compatibility (Bison 3.8).
    typedef value_type semantic_type;

    /// Symbol locations.
    typedef location location_type;

    /// Syntax errors thrown from user actions.
    struct syntax_error : std::runtime_error
    {
      syntax_error (const location_type& l, const std::string& m)
        : std::runtime_error (m)
        , location (l)
      {}

      syntax_error (const syntax_error& s)
        : std::runtime_error (s.what ())
        , location (s.location)
      {}

      ~syntax_error () YY_NOEXCEPT YY_NOTHROW;

      location_type location;
    };

    /// Token kinds.
    struct token
    {
      enum token_kind_type
      {
        YYEMPTY = -2,
    _EOF = 0,                      // "'end of file'"
    YYerror = 256,                 // error
    YYUNDEF = 257,                 // "invalid token"
    UNRECOGNIZED = 258,            // "'unrecognized'"
    APOS_ATTR_CONTENT = 260,       // "'apos attribute content'"
    CHAR_LITERAL = 261,            // "'char literal'"
    CHAR_LITERAL_AND_CDATA_END = 262, // "'char literal]]>'"
    CHAR_LITERAL_AND_PI_END = 263, // "'char literal and pi end'"
    CHAR_REF_LITERAL = 264,        // "'&#charref;'"
    ELEMENT_CONTENT = 265,         // "'element content'"
    ELEM_WILDCARD = 266,           // "'pref:*'"
    ELEM_EQNAME_WILDCARD = 267,    // "'ns:*'"
    ENTITY_REF = 268,              // "'&entity;'"
    EXPR_COMMENT_LITERAL = 269,    // "'comment literal'"
    PI_NCNAME_LBRACE = 270,        // "'pi <NCName {>'"
    NCNAME_SVAL = 271,             // "'NCName_sval'"
    PRAGMA_LITERAL_AND_END_PRAGMA = 272, // "'pragma literal'"
    QNAME_SVAL_AND_END_PRAGMA = 273, // "'QName #)'"
    EQNAME_SVAL_AND_END_PRAGMA = 274, // "'EQName #)'"
    PREFIX_WILDCARD = 275,         // "'*:QName'"
    COMP_ELEMENT_QNAME_LBRACE = 276, // "'element QName {'"
    COMP_ATTRIBUTE_QNAME_LBRACE = 277, // "'attribute QName {'"
    COMP_ELEMENT_EQNAME_LBRACE = 278, // "'element EQName {'"
    COMP_ATTRIBUTE_EQNAME_LBRACE = 279, // "'attribute EQName {'"
    COMP_PI_NCNAME_LBRACE = 280,   // "'processing-instruction NCName {'"
    COMP_NS_NCNAME_LBRACE = 281,   // "'namespace NCName {'"
    QNAME_SVAL = 282,              // "'QName'"
    EQNAME_SVAL = 283,             // "'EQName'"
    ANNOTATION_QNAME_SVAL = 284,   // "'%QName'"
    ANNOTATION_EQNAME_SVAL = 285,  // "'%EQName'"
    QUOTE_ATTR_CONTENT = 286,      // "'quote attribute content'"
    STRING_LITERAL = 287,          // "'STRING'"
    XML_COMMENT_LITERAL = 288,     // "'XML comment'"
    DECLARE = 290,                 // "'declare'"
    MODULE = 291,                  // "'module'"
    BLANK = 292,                   // "'<blank>'"
    ALLOWING = 293,                // "'allowing'"
    BASE_URI = 294,                // "'base-uri'"
    BOUNDARY_SPACE = 295,          // "'boundary-space'"
    COMMENT = 296,                 // "'comment'"
    CONSTRUCTION = 297,            // "'construction'"
    COPY_NAMESPACES = 298,         // "'copy-namespaces'"
    COUNT = 299,                   // "'count'"
    DOCUMENT = 300,                // "'document'"
    DOCUMENT_NODE = 301,           // "'document-node'"
    ELEMENT = 302,                 // "'element'"
    FOR = 303,                     // "'for'"
    FUNCTION = 304,                // "'function'"
    IF = 305,                      // "'if'"
    IMPORT = 306,                  // "'import'"
    INSTANCE = 307,                // "'instance'"
    LET = 308,                     // "'let'"
    MOST = 309,                    // "'most'"
    NS_NODE = 310,                 // "'namespace-node'"
    NEXT = 311,                    // "'next'"
    NO = 312,                      // "'no'"
    ONLY = 313,                    // "'only'"
    OPTION = 314,                  // "'option'"
    ORDERING = 315,                // "'ordering'"
    PREVIOUS = 316,                // "'previous'"
    PROCESSING_INSTRUCTION = 317,  // "'processing-instruction'"
    SCHEMA = 318,                  // "'schema'"
    SCHEMA_ATTRIBUTE = 319,        // "'schema-attribute'"
    SCHEMA_ELEMENT = 320,          // "'schema-element'"
    SEQUENTIAL = 321,              // "'sequential'"
    SET = 322,                     // "'set'"
    SIMPLE = 323,                  // "'simple'"
    SLIDING = 324,                 // "'sliding'"
    SOME = 325,                    // "'some'"
    SPACE = 326,                   // "'space'"
    STABLE = 327,                  // "'stable'"
    TEXT = 328,                    // "'text'"
    TUMBLING = 329,                // "'tumbling'"
    SWITCH = 330,                  // "'switch'"
    TYPESWITCH = 331,              // "'typeswitch'"
    UPDATING = 332,                // "'updating'"
    VALIDATE = 333,                // "'validate'"
    TYPE = 334,                    // "'type'"
    WHEN = 335,                    // "'when'"
    WORD = 336,                    // "'word'"
    DECIMAL_FORMAT = 337,          // "'decimal-format'"
    DECIMAL_SEPARATOR = 338,       // "'decimal-separator'"
    GROUPING_SEPARATOR = 339,      // "'grouping-separator'"
    INFINITY_VALUE = 340,          // "'infinity'"
    MINUS_SIGN = 341,              // "'minus-sign'"
    NaN = 342,                     // "'NaN'"
    PERCENT = 343,                 // "'percent'"
    PER_MILLE = 344,               // "'per-mille'"
    ZERO_DIGIT = 345,              // "'zero-digit'"
    DIGIT = 346,                   // "'digit'"
    PATTERN_SEPARATOR = 347,       // "'pattern-separator'"
    ANCESTOR = 348,                // "'ancestor'"
    ANCESTOR_OR_SELF = 349,        // "'ancestor-or-self'"
    AND = 350,                     // "'and'"
    APOS = 351,                    // "'''"
    AS = 352,                      // "'as'"
    ASCENDING = 353,               // "'ascending'"
    AT = 354,                      // "'at'"
    ATTRIBUTE = 355,               // "'attribute'"
    AT_SIGN = 356,                 // "'@'"
    CONCAT = 357,                  // "'||'"
    CASE = 358,                    // "'case'"
    CASTABLE = 359,                // "'castable'"
    CAST = 360,                    // "'cast'"
    CDATA_BEGIN = 361,             // "'CDATA[['"
    CDATA_END = 362,               // "']]'"
    CHILD = 363,                   // "'child'"
    COLLATION = 364,               // "'collation'"
    COMMA = 365,                   // "','"
    COMMENT_BEGIN = 366,           // "'(:'"
    COMMENT_END = 367,             // "':)'"
    DECIMAL_LITERAL = 368,         // "'DECIMAL'"
    CONTEXT = 369,                 // "'context'"
    VARIABLE = 370,                // "'variable'"
    DEFAULT = 371,                 // "'default'"
    DESCENDANT = 372,              // "'descendant'"
    DESCENDANT_OR_SELF = 373,      // "'descendant-or-self'"
    DESCENDING = 374,              // "'descending'"
    DIV = 375,                     // "'div'"
    DOLLAR = 376,                  // "'$'"
    DOT = 377,                     // "'.'"
    DOT_DOT = 378,                 // "'..'"
    COLON = 379,                   // "':'"
    DOUBLE_COLON = 380,            // "'::'"
    DOUBLE_LBRACE = 381,           // "'{{'"
    DOUBLE_LITERAL = 382,          // "'DOUBLE'"
    DOUBLE_RBRACE = 383,           // "'<double {>'"
    ELSE = 384,                    // "'else'"
    _EMPTY = 385,                  // "'empty'"
    GREATEST = 386,                // "'greatest'"
    LEAST = 387,                   // "'least'"
    EMPTY_TAG_END = 388,           // "'/>'"
    ENCODING = 389,                // "'encoding'"
    EQUALS = 390,                  // "'='"
    ESCAPE_APOS = 391,             // "''''"
    ESCAPE_QUOTE = 392,            // "'\"\"'"
    EVERY = 393,                   // "'every'"
    EXCEPT = 394,                  // "'except'"
    EXTERNAL = 395,                // "'external'"
    FOLLOWING = 396,               // "'following'"
    FOLLOWING_SIBLING = 397,       // "'following-sibling'"
    FOLLOWS = 398,                 // "'follows'"
    GE = 399,                      // "'>='"
    GETS = 400,                    // "':='"
    GT = 401,                      // "'>'"
    HOOK = 402,                    // "'?'"
    HASH = 403,                    // "'#'"
    IDIV = 404,                    // "'idiv'"
    _IN = 405,                     // "'in'"
    INHERIT = 406,                 // "'inherit'"
    INTEGER_LITERAL = 407,         // "'INTEGER'"
    INTERSECT = 408,               // "'intersect'"
    IS = 409,                      // "'is'"
    ITEM = 410,                    // "'item'"
    LBRACE = 411,                  // "'{'"
    LBRACK = 412,                  // "'['"
    LE = 413,                      // "'<='"
    LPAR = 414,                    // "'('"
    LT_OR_START_TAG = 415,         // "'<'"
    MINUS = 416,                   // "'-'"
    MOD = 417,                     // "'mod'"
    NAMESPACE = 418,               // "'namespace'"
    _NAN = 419,                    // "'nan'"
    NE = 420,                      // "'!='"
    NODECOMP = 421,                // "'nodecomp'"
    NO_INHERIT = 422,              // "'<no inherit>'"
    NO_PRESERVE = 423,             // "'<no preserve>'"
    OR = 424,                      // "'or'"
    ORDER = 425,                   // "'order'"
    ORDERED = 426,                 // "'ordered'"
    BY = 427,                      // "'by'"
    GROUP = 428,                   // "'group'"
    PARENT = 429,                  // "'parent'"
    PERCENTAGE = 430,              // "'%'"
    PI_BEGIN = 431,                // "'<?'"
    PI_END = 432,                  // "'?>'"
    PLUS = 433,                    // "'+'"
    PRAGMA_BEGIN = 434,            // "'(#'"
    PRAGMA_END = 435,              // "'#)'"
    PRECEDES = 436,                // "'<<'"
    PRECEDING = 437,               // "'preceding'"
    PRECEDING_SIBLING = 438,       // "'preceding-sibling'"
    PRESERVE = 439,                // "'preserve'"
    QUOTE = 440,                   // "'\"'"
    RBRACE = 441,                  // "'}'"
    RBRACK = 442,                  // "']'"
    RETURN = 443,                  // "'return'"
    RPAR = 444,                    // "')'"
    SATISFIES = 445,               // "'satisfies'"
    SELF = 446,                    // "'self'"
    SEMI = 447,                    // "';'"
    SLASH = 448,                   // "'/'"
    SLASH_SLASH = 449,             // "'//'"
    BANG = 450,                    // "'!'"
    STAR = 451,                    // "'*'"
    START_TAG_END = 452,           // "'</ (start tag end)'"
    STRIP = 453,                   // "'strip'"
    TAG_END = 454,                 // "'> (tag end)'"
    THEN = 455,                    // "'then'"
    TO = 456,                      // "'to'"
    TREAT = 457,                   // "'treat'"
    UNION = 458,                   // "'union'"
    UNORDERED = 459,               // "'unordered'"
    VAL_EQ = 460,                  // "'eq'"
    VAL_GE = 461,                  // "'ge'"
    VAL_GT = 462,                  // "'gt'"
    VAL_LE = 463,                  // "'le'"
    VAL_LT = 464,                  // "'lt'"
    VAL_NE = 465,                  // "'ne'"
    VALUECOMP = 466,               // "'VALUECOMP'"
    VBAR = 467,                    // "'|'"
    EMPTY_SEQUENCE = 468,          // "'empty-sequence'"
    WHERE = 469,                   // "'where'"
    XML_COMMENT_BEGIN = 470,       // "'<!--'"
    XML_COMMENT_END = 471,         // "'-->'"
    XQUERY = 472,                  // "'xquery'"
    VERSION = 473,                 // "'version'"
    START = 474,                   // "'start'"
    AFTER = 475,                   // "'after'"
    BEFORE = 476,                  // "'before'"
    REVALIDATION = 477,            // "'revalidation'"
    _STRICT = 478,                 // "'strict'"
    LAX = 479,                     // "'lax'"
    SKIP = 480,                    // "'skip'"
    _DELETE = 481,                 // "'delete'"
    NODE = 482,                    // "'node'"
    INSERT = 483,                  // "'insert'"
    NODES = 484,                   // "'nodes'"
    RENAME = 485,                  // "'rename'"
    REPLACE = 486,                 // "'replace'"
    VALUE = 487,                   // "'value'"
    OF = 488,                      // "'of'"
    FIRST = 489,                   // "'first'"
    INTO = 490,                    // "'into'"
    LAST = 491,                    // "'last'"
    MODIFY = 492,                  // "'modify'"
    COPY = 493,                    // "'copy'"
    WITH = 494,                    // "'with'"
    BREAK = 495,                   // "'break'"
    CONTINUE = 496,                // "'continue'"
    EXIT = 497,                    // "'exit'"
    LOOP = 498,                    // "'loop'"
    RETURNING = 499,               // "'returning'"
    WHILE = 500,                   // "'while'"
    TRY = 501,                     // "'try'"
    CATCH = 502,                   // "'catch'"
    USING = 503,                   // "'using'"
    ALL = 504,                     // "'all'"
    ANY = 505,                     // "'any'"
    CONTAINS = 506,                // "'contains'"
    CONTENT = 507,                 // "'content'"
    DIACRITICS = 508,              // "'diacritics'"
    DIFFERENT = 509,               // "'different'"
    DISTANCE = 510,                // "'distance'"
    END = 511,                     // "'end'"
    ENTIRE = 512,                  // "'entire'"
    EXACTLY = 513,                 // "'exactly'"
    FROM = 514,                    // "'from'"
    FTAND = 515,                   // "'ftand'"
    FTNOT = 516,                   // "'ftnot'"
    NOT = 517,                     // "'not'"
    FT_OPTION = 518,               // "'ft-option'"
    FTOR = 519,                    // "'ftor'"
    INSENSITIVE = 520,             // "'insensitive'"
    LANGUAGE = 521,                // "'language'"
    LEVELS = 522,                  // "'levels'"
    LOWERCASE = 523,               // "'lowercase'"
    OCCURS = 524,                  // "'occurs'"
    PARAGRAPH = 525,               // "'paragraph'"
    PARAGRAPHS = 526,              // "'paragraphs'"
    PHRASE = 527,                  // "'phrase'"
    RELATIONSHIP = 528,            // "'relationship'"
    SAME = 529,                    // "'same'"
    SCORE = 530,                   // "'score'"
    SENSITIVE = 531,               // "'sensitive'"
    SENTENCE = 532,                // "'sentence'"
    SENTENCES = 533,               // "'sentences'"
    STEMMING = 534,                // "'stemming'"
    STOP = 535,                    // "'stop'"
    THESAURUS = 536,               // "'thesaurus'"
    TIMES = 537,                   // "'times'"
    UPPERCASE = 538,               // "'uppercase'"
    WEIGHT = 539,                  // "'weight'"
    WILDCARDS = 540,               // "'wildcards'"
    WINDOW = 541,                  // "'window'"
    WITHOUT = 542,                 // "'without'"
    WORDS = 543,                   // "'words'"
    COLLECTION = 544,              // "'collection'"
    CONSTOPT = 545,                // "'const'"
    APPEND_ONLY = 546,             // "'append-only'"
    QUEUE = 547,                   // "'queue'"
    MUTABLE = 548,                 // "'mutable'"
    READ_ONLY = 549,               // "'read-only'"
    UNIQUE = 550,                  // "'unique'"
    NON = 551,                     // "'non'"
    INDEX = 552,                   // "'index'"
    MANUALLY = 553,                // "'manually'"
    AUTOMATICALLY = 554,           // "'automatically'"
    MAINTAINED = 555,              // "'maintained'"
    ON = 556,                      // "'on'"
    RANGE = 557,                   // "'range'"
    EQUALITY = 558,                // "'equality'"
    GENERAL = 559,                 // "'general'"
    INTEGRITY = 560,               // "'integrity'"
    CONSTRAINT = 561,              // "'constraint'"
    CHECK = 562,                   // "'check'"
    KEY = 563,                     // "'key'"
    FOREACH = 564,                 // "'foreach'"
    FOREIGN = 565,                 // "'foreign'"
    KEYS = 566,                    // "'keys'"
    L_SIMPLE_OBJ_UNION = 567,      // "'{|'"
    R_SIMPLE_OBJ_UNION = 568,      // "'|}'"
    L_ACCUMULATOR_OBJ_UNION = 569, // "'{['"
    R_ACCUMULATOR_OBJ_UNION = 570, // "']}'"
    JSON = 571,                    // "'json'"
    APPEND = 572,                  // "'append'"
    POSITION = 573,                // "'position'"
    OBJECT = 574,                  // "'object'"
    ARRAY = 575,                   // "'array'"
    JSON_ITEM = 576,               // "'json-item'"
    STRUCTURED_ITEM = 577,         // "'structured-item'"
    BYTE_ORDER_MARK_UTF8 = 578,    // "'BOM_UTF8'"
    SHEBANG = 579,                 // "'#!/shebang"
    RANGE_REDUCE = 580,            // RANGE_REDUCE
    SEQUENCE_TYPE_REDUCE = 581,    // SEQUENCE_TYPE_REDUCE
    ADDITIVE_REDUCE = 582,         // ADDITIVE_REDUCE
    UNION_REDUCE = 583,            // UNION_REDUCE
    INTERSECT_EXCEPT_REDUCE = 584, // INTERSECT_EXCEPT_REDUCE
    QVARINDECLLIST_REDUCE = 585,   // QVARINDECLLIST_REDUCE
    UNARY_PREC = 586,              // UNARY_PREC
    STEP_REDUCE = 587,             // STEP_REDUCE
    SIMPLEMAPEXPR_REDUCE = 588,    // SIMPLEMAPEXPR_REDUCE
    MULTIPLICATIVE_REDUCE = 589,   // MULTIPLICATIVE_REDUCE
    SELECT = 590                   // SELECT
      };
      /// Backward compatibility alias (Bison 3.6).
      typedef token_kind_type yytokentype;
    };

    /// Token kind, as returned by yylex.
    typedef token::token_kind_type token_kind_type;

    /// Backward compatibility alias (Bison 3.6).
    typedef token_kind_type token_type;

    /// Symbol kinds.
    struct symbol_kind
    {
      enum symbol_kind_type
      {
        YYNTOKENS = 336, ///< Number of tokens.
        S_YYEMPTY = -2,
        S_YYEOF = 0,                             // "'end of file'"
        S_YYerror = 1,                           // error
        S_YYUNDEF = 2,                           // "invalid token"
        S_UNRECOGNIZED = 3,                      // "'unrecognized'"
        S_4_error_ = 4,                          // "'error'"
        S_APOS_ATTR_CONTENT = 5,                 // "'apos attribute content'"
        S_CHAR_LITERAL = 6,                      // "'char literal'"
        S_CHAR_LITERAL_AND_CDATA_END = 7,        // "'char literal]]>'"
        S_CHAR_LITERAL_AND_PI_END = 8,           // "'char literal and pi end'"
        S_CHAR_REF_LITERAL = 9,                  // "'&#charref;'"
        S_ELEMENT_CONTENT = 10,                  // "'element content'"
        S_ELEM_WILDCARD = 11,                    // "'pref:*'"
        S_ELEM_EQNAME_WILDCARD = 12,             // "'ns:*'"
        S_ENTITY_REF = 13,                       // "'&entity;'"
        S_EXPR_COMMENT_LITERAL = 14,             // "'comment literal'"
        S_PI_NCNAME_LBRACE = 15,                 // "'pi <NCName {>'"
        S_NCNAME_SVAL = 16,                      // "'NCName_sval'"
        S_PRAGMA_LITERAL_AND_END_PRAGMA = 17,    // "'pragma literal'"
        S_QNAME_SVAL_AND_END_PRAGMA = 18,        // "'QName #)'"
        S_EQNAME_SVAL_AND_END_PRAGMA = 19,       // "'EQName #)'"
        S_PREFIX_WILDCARD = 20,                  // "'*:QName'"
        S_COMP_ELEMENT_QNAME_LBRACE = 21,        // "'element QName {'"
        S_COMP_ATTRIBUTE_QNAME_LBRACE = 22,      // "'attribute QName {'"
        S_COMP_ELEMENT_EQNAME_LBRACE = 23,       // "'element EQName {'"
        S_COMP_ATTRIBUTE_EQNAME_LBRACE = 24,     // "'attribute EQName {'"
        S_COMP_PI_NCNAME_LBRACE = 25,            // "'processing-instruction NCName {'"
        S_COMP_NS_NCNAME_LBRACE = 26,            // "'namespace NCName {'"
        S_QNAME_SVAL = 27,                       // "'QName'"
        S_EQNAME_SVAL = 28,                      // "'EQName'"
        S_ANNOTATION_QNAME_SVAL = 29,            // "'%QName'"
        S_ANNOTATION_EQNAME_SVAL = 30,           // "'%EQName'"
        S_QUOTE_ATTR_CONTENT = 31,               // "'quote attribute content'"
        S_STRING_LITERAL = 32,                   // "'STRING'"
        S_XML_COMMENT_LITERAL = 33,              // "'XML comment'"
        S_34_URI_ = 34,                          // "'URI'"
        S_DECLARE = 35,                          // "'declare'"
        S_MODULE = 36,                           // "'module'"
        S_BLANK = 37,                            // "'<blank>'"
        S_ALLOWING = 38,                         // "'allowing'"
        S_BASE_URI = 39,                         // "'base-uri'"
        S_BOUNDARY_SPACE = 40,                   // "'boundary-space'"
        S_COMMENT = 41,                          // "'comment'"
        S_CONSTRUCTION = 42,                     // "'construction'"
        S_COPY_NAMESPACES = 43,                  // "'copy-namespaces'"
        S_COUNT = 44,                            // "'count'"
        S_DOCUMENT = 45,                         // "'document'"
        S_DOCUMENT_NODE = 46,                    // "'document-node'"
        S_ELEMENT = 47,                          // "'element'"
        S_FOR = 48,                              // "'for'"
        S_FUNCTION = 49,                         // "'function'"
        S_IF = 50,                               // "'if'"
        S_IMPORT = 51,                           // "'import'"
        S_INSTANCE = 52,                         // "'instance'"
        S_LET = 53,                              // "'let'"
        S_MOST = 54,                             // "'most'"
        S_NS_NODE = 55,                          // "'namespace-node'"
        S_NEXT = 56,                             // "'next'"
        S_NO = 57,                               // "'no'"
        S_ONLY = 58,                             // "'only'"
        S_OPTION = 59,                           // "'option'"
        S_ORDERING = 60,                         // "'ordering'"
        S_PREVIOUS = 61,                         // "'previous'"
        S_PROCESSING_INSTRUCTION = 62,           // "'processing-instruction'"
        S_SCHEMA = 63,                           // "'schema'"
        S_SCHEMA_ATTRIBUTE = 64,                 // "'schema-attribute'"
        S_SCHEMA_ELEMENT = 65,                   // "'schema-element'"
        S_SEQUENTIAL = 66,                       // "'sequential'"
        S_SET = 67,                              // "'set'"
        S_SIMPLE = 68,                           // "'simple'"
        S_SLIDING = 69,                          // "'sliding'"
        S_SOME = 70,                             // "'some'"
        S_SPACE = 71,                            // "'space'"
        S_STABLE = 72,                           // "'stable'"
        S_TEXT = 73,                             // "'text'"
        S_TUMBLING = 74,                         // "'tumbling'"
        S_SWITCH = 75,                           // "'switch'"
        S_TYPESWITCH = 76,                       // "'typeswitch'"
        S_UPDATING = 77,                         // "'updating'"
        S_VALIDATE = 78,                         // "'validate'"
        S_TYPE = 79,                             // "'type'"
        S_WHEN = 80,                             // "'when'"
        S_WORD = 81,                             // "'word'"
        S_DECIMAL_FORMAT = 82,                   // "'decimal-format'"
        S_DECIMAL_SEPARATOR = 83,                // "'decimal-separator'"
        S_GROUPING_SEPARATOR = 84,               // "'grouping-separator'"
        S_INFINITY_VALUE = 85,                   // "'infinity'"
        S_MINUS_SIGN = 86,                       // "'minus-sign'"
        S_NaN = 87,                              // "'NaN'"
        S_PERCENT = 88,                          // "'percent'"
        S_PER_MILLE = 89,                        // "'per-mille'"
        S_ZERO_DIGIT = 90,                       // "'zero-digit'"
        S_DIGIT = 91,                            // "'digit'"
        S_PATTERN_SEPARATOR = 92,                // "'pattern-separator'"
        S_ANCESTOR = 93,                         // "'ancestor'"
        S_ANCESTOR_OR_SELF = 94,                 // "'ancestor-or-self'"
        S_AND = 95,                              // "'and'"
        S_APOS = 96,                             // "'''"
        S_AS = 97,                               // "'as'"
        S_ASCENDING = 98,                        // "'ascending'"
        S_AT = 99,                               // "'at'"
        S_ATTRIBUTE = 100,                       // "'attribute'"
        S_AT_SIGN = 101,                         // "'@'"
        S_CONCAT = 102,                          // "'||'"
        S_CASE = 103,                            // "'case'"
        S_CASTABLE = 104,                        // "'castable'"
        S_CAST = 105,                            // "'cast'"
        S_CDATA_BEGIN = 106,                     // "'CDATA[['"
        S_CDATA_END = 107,                       // "']]'"
        S_CHILD = 108,                           // "'child'"
        S_COLLATION = 109,                       // "'collation'"
        S_COMMA = 110,                           // "','"
        S_COMMENT_BEGIN = 111,                   // "'(:'"
        S_COMMENT_END = 112,                     // "':)'"
        S_DECIMAL_LITERAL = 113,                 // "'DECIMAL'"
        S_CONTEXT = 114,                         // "'context'"
        S_VARIABLE = 115,                        // "'variable'"
        S_DEFAULT = 116,                         // "'default'"
        S_DESCENDANT = 117,                      // "'descendant'"
        S_DESCENDANT_OR_SELF = 118,              // "'descendant-or-self'"
        S_DESCENDING = 119,                      // "'descending'"
        S_DIV = 120,                             // "'div'"
        S_DOLLAR = 121,                          // "'$'"
        S_DOT = 122,                             // "'.'"
        S_DOT_DOT = 123,                         // "'..'"
        S_COLON = 124,                           // "':'"
        S_DOUBLE_COLON = 125,                    // "'::'"
        S_DOUBLE_LBRACE = 126,                   // "'{{'"
        S_DOUBLE_LITERAL = 127,                  // "'DOUBLE'"
        S_DOUBLE_RBRACE = 128,                   // "'<double {>'"
        S_ELSE = 129,                            // "'else'"
        S__EMPTY = 130,                          // "'empty'"
        S_GREATEST = 131,                        // "'greatest'"
        S_LEAST = 132,                           // "'least'"
        S_EMPTY_TAG_END = 133,                   // "'/>'"
        S_ENCODING = 134,                        // "'encoding'"
        S_EQUALS = 135,                          // "'='"
        S_ESCAPE_APOS = 136,                     // "''''"
        S_ESCAPE_QUOTE = 137,                    // "'\"\"'"
        S_EVERY = 138,                           // "'every'"
        S_EXCEPT = 139,                          // "'except'"
        S_EXTERNAL = 140,                        // "'external'"
        S_FOLLOWING = 141,                       // "'following'"
        S_FOLLOWING_SIBLING = 142,               // "'following-sibling'"
        S_FOLLOWS = 143,                         // "'follows'"
        S_GE = 144,                              // "'>='"
        S_GETS = 145,                            // "':='"
        S_GT = 146,                              // "'>'"
        S_HOOK = 147,                            // "'?'"
        S_HASH = 148,                            // "'#'"
        S_IDIV = 149,                            // "'idiv'"
        S__IN = 150,                             // "'in'"
        S_INHERIT = 151,                         // "'inherit'"
        S_INTEGER_LITERAL = 152,                 // "'INTEGER'"
        S_INTERSECT = 153,                       // "'intersect'"
        S_IS = 154,                              // "'is'"
        S_ITEM = 155,                            // "'item'"
        S_LBRACE = 156,                          // "'{'"
        S_LBRACK = 157,                          // "'['"
        S_LE = 158,                              // "'<='"
        S_LPAR = 159,                            // "'('"
        S_LT_OR_START_TAG = 160,                 // "'<'"
        S_MINUS = 161,                           // "'-'"
        S_MOD = 162,                             // "'mod'"
        S_NAMESPACE = 163,                       // "'namespace'"
        S__NAN = 164,                            // "'nan'"
        S_NE = 165,                              // "'!='"
        S_NODECOMP = 166,                        // "'nodecomp'"
        S_NO_INHERIT = 167,                      // "'<no inherit>'"
        S_NO_PRESERVE = 168,                     // "'<no preserve>'"
        S_OR = 169,                              // "'or'"
        S_ORDER = 170,                           // "'order'"
        S_ORDERED = 171,                         // "'ordered'"
        S_BY = 172,                              // "'by'"
        S_GROUP = 173,                           // "'group'"
        S_PARENT = 174,                          // "'parent'"
        S_PERCENTAGE = 175,                      // "'%'"
        S_PI_BEGIN = 176,                        // "'<?'"
        S_PI_END = 177,                          // "'?>'"
        S_PLUS = 178,                            // "'+'"
        S_PRAGMA_BEGIN = 179,                    // "'(#'"
        S_PRAGMA_END = 180,                      // "'#)'"
        S_PRECEDES = 181,                        // "'<<'"
        S_PRECEDING = 182,                       // "'preceding'"
        S_PRECEDING_SIBLING = 183,               // "'preceding-sibling'"
        S_PRESERVE = 184,                        // "'preserve'"
        S_QUOTE = 185,                           // "'\"'"
        S_RBRACE = 186,                          // "'}'"
        S_RBRACK = 187,                          // "']'"
        S_RETURN = 188,                          // "'return'"
        S_RPAR = 189,                            // "')'"
        S_SATISFIES = 190,                       // "'satisfies'"
        S_SELF = 191,                            // "'self'"
        S_SEMI = 192,                            // "';'"
        S_SLASH = 193,                           // "'/'"
        S_SLASH_SLASH = 194,                     // "'//'"
        S_BANG = 195,                            // "'!'"
        S_STAR = 196,                            // "'*'"
        S_START_TAG_END = 197,                   // "'</ (start tag end)'"
        S_STRIP = 198,                           // "'strip'"
        S_TAG_END = 199,                         // "'> (tag end)'"
        S_THEN = 200,                            // "'then'"
        S_TO = 201,                              // "'to'"
        S_TREAT = 202,                           // "'treat'"
        S_UNION = 203,                           // "'union'"
        S_UNORDERED = 204,                       // "'unordered'"
        S_VAL_EQ = 205,                          // "'eq'"
        S_VAL_GE = 206,                          // "'ge'"
        S_VAL_GT = 207,                          // "'gt'"
        S_VAL_LE = 208,                          // "'le'"
        S_VAL_LT = 209,                          // "'lt'"
        S_VAL_NE = 210,                          // "'ne'"
        S_VALUECOMP = 211,                       // "'VALUECOMP'"
        S_VBAR = 212,                            // "'|'"
        S_EMPTY_SEQUENCE = 213,                  // "'empty-sequence'"
        S_WHERE = 214,                           // "'where'"
        S_XML_COMMENT_BEGIN = 215,               // "'<!--'"
        S_XML_COMMENT_END = 216,                 // "'-->'"
        S_XQUERY = 217,                          // "'xquery'"
        S_VERSION = 218,                         // "'version'"
        S_START = 219,                           // "'start'"
        S_AFTER = 220,                           // "'after'"
        S_BEFORE = 221,                          // "'before'"
        S_REVALIDATION = 222,                    // "'revalidation'"
        S__STRICT = 223,                         // "'strict'"
        S_LAX = 224,                             // "'lax'"
        S_SKIP = 225,                            // "'skip'"
        S__DELETE = 226,                         // "'delete'"
        S_NODE = 227,                            // "'node'"
        S_INSERT = 228,                          // "'insert'"
        S_NODES = 229,                           // "'nodes'"
        S_RENAME = 230,                          // "'rename'"
        S_REPLACE = 231,                         // "'replace'"
        S_VALUE = 232,                           // "'value'"
        S_OF = 233,                              // "'of'"
        S_FIRST = 234,                           // "'first'"
        S_INTO = 235,                            // "'into'"
        S_LAST = 236,                            // "'last'"
        S_MODIFY = 237,                          // "'modify'"
        S_COPY = 238,                            // "'copy'"
        S_WITH = 239,                            // "'with'"
        S_BREAK = 240,                           // "'break'"
        S_CONTINUE = 241,                        // "'continue'"
        S_EXIT = 242,                            // "'exit'"
        S_LOOP = 243,                            // "'loop'"
        S_RETURNING = 244,                       // "'returning'"
        S_WHILE = 245,                           // "'while'"
        S_TRY = 246,                             // "'try'"
        S_CATCH = 247,                           // "'catch'"
        S_USING = 248,                           // "'using'"
        S_ALL = 249,                             // "'all'"
        S_ANY = 250,                             // "'any'"
        S_CONTAINS = 251,                        // "'contains'"
        S_CONTENT = 252,                         // "'content'"
        S_DIACRITICS = 253,                      // "'diacritics'"
        S_DIFFERENT = 254,                       // "'different'"
        S_DISTANCE = 255,                        // "'distance'"
        S_END = 256,                             // "'end'"
        S_ENTIRE = 257,                          // "'entire'"
        S_EXACTLY = 258,                         // "'exactly'"
        S_FROM = 259,                            // "'from'"
        S_FTAND = 260,                           // "'ftand'"
        S_FTNOT = 261,                           // "'ftnot'"
        S_NOT = 262,                             // "'not'"
        S_FT_OPTION = 263,                       // "'ft-option'"
        S_FTOR = 264,                            // "'ftor'"
        S_INSENSITIVE = 265,                     // "'insensitive'"
        S_LANGUAGE = 266,                        // "'language'"
        S_LEVELS = 267,                          // "'levels'"
        S_LOWERCASE = 268,                       // "'lowercase'"
        S_OCCURS = 269,                          // "'occurs'"
        S_PARAGRAPH = 270,                       // "'paragraph'"
        S_PARAGRAPHS = 271,                      // "'paragraphs'"
        S_PHRASE = 272,                          // "'phrase'"
        S_RELATIONSHIP = 273,                    // "'relationship'"
        S_SAME = 274,                            // "'same'"
        S_SCORE = 275,                           // "'score'"
        S_SENSITIVE = 276,                       // "'sensitive'"
        S_SENTENCE = 277,                        // "'sentence'"
        S_SENTENCES = 278,                       // "'sentences'"
        S_STEMMING = 279,                        // "'stemming'"
        S_STOP = 280,                            // "'stop'"
        S_THESAURUS = 281,                       // "'thesaurus'"
        S_TIMES = 282,                           // "'times'"
        S_UPPERCASE = 283,                       // "'uppercase'"
        S_WEIGHT = 284,                          // "'weight'"
        S_WILDCARDS = 285,                       // "'wildcards'"
        S_WINDOW = 286,                          // "'window'"
        S_WITHOUT = 287,                         // "'without'"
        S_WORDS = 288,                           // "'words'"
        S_COLLECTION = 289,                      // "'collection'"
        S_CONSTOPT = 290,                        // "'const'"
        S_APPEND_ONLY = 291,                     // "'append-only'"
        S_QUEUE = 292,                           // "'queue'"
        S_MUTABLE = 293,                         // "'mutable'"
        S_READ_ONLY = 294,                       // "'read-only'"
        S_UNIQUE = 295,                          // "'unique'"
        S_NON = 296,                             // "'non'"
        S_INDEX = 297,                           // "'index'"
        S_MANUALLY = 298,                        // "'manually'"
        S_AUTOMATICALLY = 299,                   // "'automatically'"
        S_MAINTAINED = 300,                      // "'maintained'"
        S_ON = 301,                              // "'on'"
        S_RANGE = 302,                           // "'range'"
        S_EQUALITY = 303,                        // "'equality'"
        S_GENERAL = 304,                         // "'general'"
        S_INTEGRITY = 305,                       // "'integrity'"
        S_CONSTRAINT = 306,                      // "'constraint'"
        S_CHECK = 307,                           // "'check'"
        S_KEY = 308,                             // "'key'"
        S_FOREACH = 309,                         // "'foreach'"
        S_FOREIGN = 310,                         // "'foreign'"
        S_KEYS = 311,                            // "'keys'"
        S_L_SIMPLE_OBJ_UNION = 312,              // "'{|'"
        S_R_SIMPLE_OBJ_UNION = 313,              // "'|}'"
        S_L_ACCUMULATOR_OBJ_UNION = 314,         // "'{['"
        S_R_ACCUMULATOR_OBJ_UNION = 315,         // "']}'"
        S_JSON = 316,                            // "'json'"
        S_APPEND = 317,                          // "'append'"
        S_POSITION = 318,                        // "'position'"
        S_OBJECT = 319,                          // "'object'"
        S_ARRAY = 320,                           // "'array'"
        S_JSON_ITEM = 321,                       // "'json-item'"
        S_STRUCTURED_ITEM = 322,                 // "'structured-item'"
        S_BYTE_ORDER_MARK_UTF8 = 323,            // "'BOM_UTF8'"
        S_SHEBANG = 324,                         // "'#!/shebang"
        S_RANGE_REDUCE = 325,                    // RANGE_REDUCE
        S_SEQUENCE_TYPE_REDUCE = 326,            // SEQUENCE_TYPE_REDUCE
        S_ADDITIVE_REDUCE = 327,                 // ADDITIVE_REDUCE
        S_UNION_REDUCE = 328,                    // UNION_REDUCE
        S_INTERSECT_EXCEPT_REDUCE = 329,         // INTERSECT_EXCEPT_REDUCE
        S_QVARINDECLLIST_REDUCE = 330,           // QVARINDECLLIST_REDUCE
        S_UNARY_PREC = 331,                      // UNARY_PREC
        S_STEP_REDUCE = 332,                     // STEP_REDUCE
        S_SIMPLEMAPEXPR_REDUCE = 333,            // SIMPLEMAPEXPR_REDUCE
        S_MULTIPLICATIVE_REDUCE = 334,           // MULTIPLICATIVE_REDUCE
        S_SELECT = 335,                          // SELECT
        S_YYACCEPT = 336,                        // $accept
        S_Module = 337,                          // Module
        S_ERROR = 338,                           // ERROR
        S_ModuleWithoutBOM = 339,                // ModuleWithoutBOM
        S_VersionDecl = 340,                     // VersionDecl
        S_MainModule = 341,                      // MainModule
        S_LibraryModule = 342,                   // LibraryModule
        S_ModuleDecl = 343,                      // ModuleDecl
        S_SIND_DeclList = 344,                   // SIND_DeclList
        S_SIND_Decl = 345,                       // SIND_Decl
        S_Setter = 346,                          // Setter
        S_BoundarySpaceDecl = 347,               // BoundarySpaceDecl
        S_DefaultCollationDecl = 348,            // DefaultCollationDecl
        S_BaseURIDecl = 349,                     // BaseURIDecl
        S_ConstructionDecl = 350,                // ConstructionDecl
        S_OrderingModeDecl = 351,                // OrderingModeDecl
        S_EmptyOrderDecl = 352,                  // EmptyOrderDecl
        S_CopyNamespacesDecl = 353,              // CopyNamespacesDecl
        S_Import = 354,                          // Import
        S_SchemaImport = 355,                    // SchemaImport
        S_URILiteralList = 356,                  // URILiteralList
        S_SchemaPrefix = 357,                    // SchemaPrefix
        S_ModuleImport = 358,                    // ModuleImport
        S_NamespaceDecl = 359,                   // NamespaceDecl
        S_DefaultNamespaceDecl = 360,            // DefaultNamespaceDecl
        S_VFO_DeclList = 361,                    // VFO_DeclList
        S_VFO_Decl = 362,                        // VFO_Decl
        S_DecimalFormatDecl = 363,               // DecimalFormatDecl
        S_DecimalFormatParamList = 364,          // DecimalFormatParamList
        S_DecimalFormatParam = 365,              // DecimalFormatParam
        S_DecimalFormatParamName = 366,          // DecimalFormatParamName
        S_OptionDecl = 367,                      // OptionDecl
        S_FTOptionDecl = 368,                    // FTOptionDecl
        S_CtxItemDecl = 369,                     // CtxItemDecl
        S_CtxItemDecl2 = 370,                    // CtxItemDecl2
        S_CtxItemDecl3 = 371,                    // CtxItemDecl3
        S_CtxItemDecl4 = 372,                    // CtxItemDecl4
        S_VarDecl = 373,                         // VarDecl
        S_VarNameAndType = 374,                  // VarNameAndType
        S_AnnotationList = 375,                  // AnnotationList
        S_Annotation = 376,                      // Annotation
        S_AnnotationLiteralList = 377,           // AnnotationLiteralList
        S_FunctionDecl = 378,                    // FunctionDecl
        S_FunctionDecl2 = 379,                   // FunctionDecl2
        S_FunctionDeclSimple = 380,              // FunctionDeclSimple
        S_FunctionDeclUpdating = 381,            // FunctionDeclUpdating
        S_FunctionSig = 382,                     // FunctionSig
        S_ParamList = 383,                       // ParamList
        S_Param = 384,                           // Param
        S_CollectionDecl = 385,                  // CollectionDecl
        S_CollectionTypeDecl = 386,              // CollectionTypeDecl
        S_IndexDecl = 387,                       // IndexDecl
        S_IndexKeyList = 388,                    // IndexKeyList
        S_IndexKeySpec = 389,                    // IndexKeySpec
        S_IntegrityConstraintDecl = 390,         // IntegrityConstraintDecl
        S_QueryBody = 391,                       // QueryBody
        S_StatementsAndOptionalExprTop = 392,    // StatementsAndOptionalExprTop
        S_StatementsAndOptionalExpr = 393,       // StatementsAndOptionalExpr
        S_StatementsAndExpr = 394,               // StatementsAndExpr
        S_Statements = 395,                      // Statements
        S_Statement = 396,                       // Statement
        S_BlockStatement = 397,                  // BlockStatement
        S_BlockExpr = 398,                       // BlockExpr
        S_EnclosedStatementsAndOptionalExpr = 399, // EnclosedStatementsAndOptionalExpr
        S_VarDeclStatement = 400,                // VarDeclStatement
        S_BlockVarDeclList = 401,                // BlockVarDeclList
        S_BlockVarDecl = 402,                    // BlockVarDecl
        S_AssignStatement = 403,                 // AssignStatement
        S_ApplyStatement = 404,                  // ApplyStatement
        S_ExitStatement = 405,                   // ExitStatement
        S_WhileStatement = 406,                  // WhileStatement
        S_FlowCtlStatement = 407,                // FlowCtlStatement
        S_FLWORStatement = 408,                  // FLWORStatement
        S_ReturnStatement = 409,                 // ReturnStatement
        S_IfStatement = 410,                     // IfStatement
        S_TryStatement = 411,                    // TryStatement
        S_CatchListStatement = 412,              // CatchListStatement
        S_CatchStatement = 413,                  // CatchStatement
        S_VoidStatement = 414,                   // VoidStatement
        S_Expr = 415,                            // Expr
        S_ExprSingle = 416,                      // ExprSingle
        S_ExprSimple = 417,                      // ExprSimple
        S_FLWORExpr = 418,                       // FLWORExpr
        S_ReturnExpr = 419,                      // ReturnExpr
        S_ReturnOrSelect = 420,                  // ReturnOrSelect
        S_WindowType = 421,                      // WindowType
        S_FLWORWinCondType = 422,                // FLWORWinCondType
        S_FLWORWinCond = 423,                    // FLWORWinCond
        S_WindowClause = 424,                    // WindowClause
        S_CountClause = 425,                     // CountClause
        S_ForLetWinClause = 426,                 // ForLetWinClause
        S_FLWORClause = 427,                     // FLWORClause
        S_FLWORClauseList = 428,                 // FLWORClauseList
        S_ForClause = 429,                       // ForClause
        S_ForOrFrom = 430,                       // ForOrFrom
        S_VarInDeclList = 431,                   // VarInDeclList
        S_VarInDecl = 432,                       // VarInDecl
        S_PositionalVar = 433,                   // PositionalVar
        S_FTScoreVar = 434,                      // FTScoreVar
        S_LetClause = 435,                       // LetClause
        S_VarGetsDeclList = 436,                 // VarGetsDeclList
        S_VarGetsDecl = 437,                     // VarGetsDecl
        S_WindowVarDecl = 438,                   // WindowVarDecl
        S_WindowVars = 439,                      // WindowVars
        S_WindowVars3 = 440,                     // WindowVars3
        S_WindowVars2 = 441,                     // WindowVars2
        S_WhereClause = 442,                     // WhereClause
        S_GroupByClause = 443,                   // GroupByClause
        S_GroupSpecList = 444,                   // GroupSpecList
        S_GroupSpec = 445,                       // GroupSpec
        S_GroupCollationSpec = 446,              // GroupCollationSpec
        S_OrderByClause = 447,                   // OrderByClause
        S_OrderSpecList = 448,                   // OrderSpecList
        S_OrderSpec = 449,                       // OrderSpec
        S_OrderModifier = 450,                   // OrderModifier
        S_OrderDirSpec = 451,                    // OrderDirSpec
        S_OrderEmptySpec = 452,                  // OrderEmptySpec
        S_OrderCollationSpec = 453,              // OrderCollationSpec
        S_QuantifiedExpr = 454,                  // QuantifiedExpr
        S_QVarInDeclList = 455,                  // QVarInDeclList
        S_QVarInDecl = 456,                      // QVarInDecl
        S_SwitchExpr = 457,                      // SwitchExpr
        S_SwitchCaseClauseList = 458,            // SwitchCaseClauseList
        S_SwitchCaseClause = 459,                // SwitchCaseClause
        S_SwitchCaseOperandList = 460,           // SwitchCaseOperandList
        S_SwitchStatement = 461,                 // SwitchStatement
        S_SwitchCaseStatementList = 462,         // SwitchCaseStatementList
        S_SwitchCaseStatement = 463,             // SwitchCaseStatement
        S_TypeswitchExpr = 464,                  // TypeswitchExpr
        S_TypeswitchStatement = 465,             // TypeswitchStatement
        S_CaseClauseList = 466,                  // CaseClauseList
        S_CaseClause = 467,                      // CaseClause
        S_CaseStatementList = 468,               // CaseStatementList
        S_CaseStatement = 469,                   // CaseStatement
        S_SequenceTypeList = 470,                // SequenceTypeList
        S_IfExpr = 471,                          // IfExpr
        S_OrExpr = 472,                          // OrExpr
        S_AndExpr = 473,                         // AndExpr
        S_ComparisonExpr = 474,                  // ComparisonExpr
        S_475_1 = 475,                           // $@1
        S_FTContainsExpr = 476,                  // FTContainsExpr
        S_StringConcatExpr = 477,                // StringConcatExpr
        S_opt_FTIgnoreOption = 478,              // opt_FTIgnoreOption
        S_RangeExpr = 479,                       // RangeExpr
        S_AdditiveExpr = 480,                    // AdditiveExpr
        S_MultiplicativeExpr = 481,              // MultiplicativeExpr
        S_UnionExpr = 482,                       // UnionExpr
        S_IntersectExceptExpr = 483,             // IntersectExceptExpr
        S_InstanceofExpr = 484,                  // InstanceofExpr
        S_TreatExpr = 485,                       // TreatExpr
        S_CastableExpr = 486,                    // CastableExpr
        S_CastExpr = 487,                        // CastExpr
        S_SingleType = 488,                      // SingleType
        S_UnaryExpr = 489,                       // UnaryExpr
        S_SignList = 490,                        // SignList
        S_ValueExpr = 491,                       // ValueExpr
        S_SimpleMapExpr = 492,                   // SimpleMapExpr
        S_ValueComp = 493,                       // ValueComp
        S_NodeComp = 494,                        // NodeComp
        S_ValidateExpr = 495,                    // ValidateExpr
        S_ExtensionExpr = 496,                   // ExtensionExpr
        S_Pragma_list = 497,                     // Pragma_list
        S_Pragma = 498,                          // Pragma
        S_PathExpr = 499,                        // PathExpr
        S_LeadingSlash = 500,                    // LeadingSlash
        S_RelativePathExpr = 501,                // RelativePathExpr
        S_StepExpr = 502,                        // StepExpr
        S_AxisStep = 503,                        // AxisStep
        S_ForwardStep = 504,                     // ForwardStep
        S_ForwardAxis = 505,                     // ForwardAxis
        S_AbbrevForwardStep = 506,               // AbbrevForwardStep
        S_ReverseStep = 507,                     // ReverseStep
        S_ReverseAxis = 508,                     // ReverseAxis
        S_NodeTest = 509,                        // NodeTest
        S_NameTest = 510,                        // NameTest
        S_Wildcard = 511,                        // Wildcard
        S_PostfixExpr = 512,                     // PostfixExpr
        S_PredicateList = 513,                   // PredicateList
        S_Predicate = 514,                       // Predicate
        S_PrimaryExpr = 515,                     // PrimaryExpr
        S_Literal = 516,                         // Literal
        S_NumericLiteral = 517,                  // NumericLiteral
        S_VarRef = 518,                          // VarRef
        S_ParenthesizedExpr = 519,               // ParenthesizedExpr
        S_ContextItemExpr = 520,                 // ContextItemExpr
        S_OrderedExpr = 521,                     // OrderedExpr
        S_UnorderedExpr = 522,                   // UnorderedExpr
        S_FunctionCall = 523,                    // FunctionCall
        S_ArgList = 524,                         // ArgList
        S_FunctionItemExpr = 525,                // FunctionItemExpr
        S_LiteralFunctionItem = 526,             // LiteralFunctionItem
        S_InlineFunction = 527,                  // InlineFunction
        S_Constructor = 528,                     // Constructor
        S_DirectConstructor = 529,               // DirectConstructor
        S_DirElemConstructor = 530,              // DirElemConstructor
        S_DirElemContentList = 531,              // DirElemContentList
        S_DirAttributeList = 532,                // DirAttributeList
        S_DirAttr = 533,                         // DirAttr
        S_OptionalBlank = 534,                   // OptionalBlank
        S_DirAttributeValue = 535,               // DirAttributeValue
        S_opt_QuoteAttrContentList = 536,        // opt_QuoteAttrContentList
        S_QuoteAttrContentList = 537,            // QuoteAttrContentList
        S_opt_AposAttrContentList = 538,         // opt_AposAttrContentList
        S_AposAttrContentList = 539,             // AposAttrContentList
        S_QuoteAttrValueContent = 540,           // QuoteAttrValueContent
        S_AposAttrValueContent = 541,            // AposAttrValueContent
        S_DirElemContent = 542,                  // DirElemContent
        S_CommonContent = 543,                   // CommonContent
        S_DirCommentConstructor = 544,           // DirCommentConstructor
        S_DirPIConstructor = 545,                // DirPIConstructor
        S_CDataSection = 546,                    // CDataSection
        S_ComputedConstructor = 547,             // ComputedConstructor
        S_CompDocConstructor = 548,              // CompDocConstructor
        S_CompElemConstructor = 549,             // CompElemConstructor
        S_CompAttrConstructor = 550,             // CompAttrConstructor
        S_CompTextConstructor = 551,             // CompTextConstructor
        S_CompCommentConstructor = 552,          // CompCommentConstructor
        S_CompPIConstructor = 553,               // CompPIConstructor
        S_CompNamespaceConstructor = 554,        // CompNamespaceConstructor
        S_TypeDeclaration = 555,                 // TypeDeclaration
        S_SequenceType = 556,                    // SequenceType
        S_OccurrenceIndicator = 557,             // OccurrenceIndicator
        S_ItemType = 558,                        // ItemType
        S_TypeList = 559,                        // TypeList
        S_GeneralizedAtomicType = 560,           // GeneralizedAtomicType
        S_SimpleType = 561,                      // SimpleType
        S_KindTest = 562,                        // KindTest
        S_AnyKindTest = 563,                     // AnyKindTest
        S_NODE_KIND_TEST = 564,                  // NODE_KIND_TEST
        S_DocumentTest = 565,                    // DocumentTest
        S_NamespaceTest = 566,                   // NamespaceTest
        S_TextTest = 567,                        // TextTest
        S_CommentTest = 568,                     // CommentTest
        S_PITest = 569,                          // PITest
        S_AttributeTest = 570,                   // AttributeTest
        S_SchemaAttributeTest = 571,             // SchemaAttributeTest
        S_ElementTest = 572,                     // ElementTest
        S_SchemaElementTest = 573,               // SchemaElementTest
        S_TypeName = 574,                        // TypeName
        S_TypeName_WITH_HOOK = 575,              // TypeName_WITH_HOOK
        S_StringLiteral = 576,                   // StringLiteral
        S_FunctionTest = 577,                    // FunctionTest
        S_AnyFunctionTest = 578,                 // AnyFunctionTest
        S_TypedFunctionTest = 579,               // TypedFunctionTest
        S_ParenthesizedItemType = 580,           // ParenthesizedItemType
        S_RevalidationDecl = 581,                // RevalidationDecl
        S_InsertExpr = 582,                      // InsertExpr
        S_DeleteExpr = 583,                      // DeleteExpr
        S_ReplaceExpr = 584,                     // ReplaceExpr
        S_RenameExpr = 585,                      // RenameExpr
        S_TransformExpr = 586,                   // TransformExpr
        S_VarNameList = 587,                     // VarNameList
        S_VarNameDecl = 588,                     // VarNameDecl
        S_TryExpr = 589,                         // TryExpr
        S_CatchListExpr = 590,                   // CatchListExpr
        S_CatchExpr = 591,                       // CatchExpr
        S_BracedExpr = 592,                      // BracedExpr
        S_NameTestList = 593,                    // NameTestList
        S_FTSelection = 594,                     // FTSelection
        S_opt_FTPosFilter_list = 595,            // opt_FTPosFilter_list
        S_FTPosFilter_list = 596,                // FTPosFilter_list
        S_FTOr = 597,                            // FTOr
        S_FTAnd = 598,                           // FTAnd
        S_FTMildNot = 599,                       // FTMildNot
        S_FTUnaryNot = 600,                      // FTUnaryNot
        S_FTPrimaryWithOptions = 601,            // FTPrimaryWithOptions
        S_opt_FTMatchOptions = 602,              // opt_FTMatchOptions
        S_opt_FTWeight = 603,                    // opt_FTWeight
        S_FTWeight = 604,                        // FTWeight
        S_FTPrimary = 605,                       // FTPrimary
        S_opt_FTTimes = 606,                     // opt_FTTimes
        S_FTExtensionSelection = 607,            // FTExtensionSelection
        S_opt_FTSelection = 608,                 // opt_FTSelection
        S_FTWords = 609,                         // FTWords
        S_FTWordsValue = 610,                    // FTWordsValue
        S_opt_FTAnyallOption = 611,              // opt_FTAnyallOption
        S_FTAnyallOption = 612,                  // FTAnyallOption
        S_opt_word = 613,                        // opt_word
        S_opt_words = 614,                       // opt_words
        S_FTPosFilter = 615,                     // FTPosFilter
        S_FTOrder = 616,                         // FTOrder
        S_FTWindow = 617,                        // FTWindow
        S_FTDistance = 618,                      // FTDistance
        S_FTUnit = 619,                          // FTUnit
        S_FTMatchOptions = 620,                  // FTMatchOptions
        S_FTMatchOption = 621,                   // FTMatchOption
        S_FTCaseOption = 622,                    // FTCaseOption
        S_FTDiacriticsOption = 623,              // FTDiacriticsOption
        S_FTExtensionOption = 624,               // FTExtensionOption
        S_FTStemOption = 625,                    // FTStemOption
        S_FTThesaurusOption = 626,               // FTThesaurusOption
        S_FTThesaurusID_or_default = 627,        // FTThesaurusID_or_default
        S_opt_FTThesaurus_list = 628,            // opt_FTThesaurus_list
        S_FTThesaurus_list = 629,                // FTThesaurus_list
        S_FTThesaurusID = 630,                   // FTThesaurusID
        S_opt_relationship = 631,                // opt_relationship
        S_opt_levels = 632,                      // opt_levels
        S_FTStopWordOption = 633,                // FTStopWordOption
        S_FTStopWords = 634,                     // FTStopWords
        S_STRING_LITERAL_list = 635,             // STRING_LITERAL_list
        S_opt_FTStopWordsInclExcl_list = 636,    // opt_FTStopWordsInclExcl_list
        S_FTStopWordsInclExcl_list = 637,        // FTStopWordsInclExcl_list
        S_FTStopWordsInclExcl = 638,             // FTStopWordsInclExcl
        S_FTLanguageOption = 639,                // FTLanguageOption
        S_FTWildCardOption = 640,                // FTWildCardOption
        S_FTContent = 641,                       // FTContent
        S_FTTimes = 642,                         // FTTimes
        S_FTRange = 643,                         // FTRange
        S_FTScope = 644,                         // FTScope
        S_FTBigUnit = 645,                       // FTBigUnit
        S_FTIgnoreOption = 646,                  // FTIgnoreOption
        S_JSONArrayConstructor = 647,            // JSONArrayConstructor
        S_JSONSimpleObjectUnion = 648,           // JSONSimpleObjectUnion
        S_JSONAccumulatorObjectUnion = 649,      // JSONAccumulatorObjectUnion
        S_JSONObjectConstructor = 650,           // JSONObjectConstructor
        S_JSONPairList = 651,                    // JSONPairList
        S_JSONInsertExpr = 652,                  // JSONInsertExpr
        S_JSONAppendExpr = 653,                  // JSONAppendExpr
        S_JSONDeleteExpr = 654,                  // JSONDeleteExpr
        S_JSONRenameExpr = 655,                  // JSONRenameExpr
        S_JSONReplaceExpr = 656,                 // JSONReplaceExpr
        S_JSONTest = 657,                        // JSONTest
        S_URI_LITERAL = 658,                     // URI_LITERAL
        S_QNAME = 659,                           // QNAME
        S_FUNCTION_NAME = 660,                   // FUNCTION_NAME
        S_EQNAME = 661                           // EQNAME
      };
    };

    /// (Internal) symbol kind.
    typedef symbol_kind::symbol_kind_type symbol_kind_type;

    /// The number of tokens.
    static const symbol_kind_type YYNTOKENS = symbol_kind::YYNTOKENS;

    /// A complete symbol.
    ///
    /// Expects its Base type to provide access to the symbol kind
    /// via kind ().
    ///
    /// Provide access to semantic value and location.
    template <typename Base>
    struct basic_symbol : Base
    {
      /// Alias to Base.
      typedef Base super_type;

      /// Default constructor.
      basic_symbol () YY_NOEXCEPT
        : value ()
        , location ()
      {}

#if 201103L <= YY_CPLUSPLUS
      /// Move constructor.
      basic_symbol (basic_symbol&& that)
        : Base (std::move (that))
        , value (std::move (that.value))
        , location (std::move (that.location))
      {}
#endif

      /// Copy constructor.
      basic_symbol (const basic_symbol& that);
      /// Constructor for valueless symbols.
      basic_symbol (typename Base::kind_type t,
                    YY_MOVE_REF (location_type) l);

      /// Constructor for symbols with semantic value.
      basic_symbol (typename Base::kind_type t,
                    YY_RVREF (value_type) v,
                    YY_RVREF (location_type) l);

      /// Destroy the symbol.
      ~basic_symbol ()
      {
        clear ();
      }



      /// Destroy contents, and record that is empty.
      void clear () YY_NOEXCEPT
      {
        Base::clear ();
      }

      /// The user-facing name of this symbol.
      std::string name () const YY_NOEXCEPT
      {
        return xquery_parser::symbol_name (this->kind ());
      }

      /// Backward compatibility (Bison 3.6).
      symbol_kind_type type_get () const YY_NOEXCEPT;

      /// Whether empty.
      bool empty () const YY_NOEXCEPT;

      /// Destructive move, \a s is emptied into this.
      void move (basic_symbol& s);

      /// The semantic value.
      value_type value;

      /// The location.
      location_type location;

    private:
#if YY_CPLUSPLUS < 201103L
      /// Assignment operator.
      basic_symbol& operator= (const basic_symbol& that);
#endif
    };

    /// Type access provider for token (enum) based symbols.
    struct by_kind
    {
      /// The symbol kind as needed by the constructor.
      typedef token_kind_type kind_type;

      /// Default constructor.
      by_kind () YY_NOEXCEPT;

#if 201103L <= YY_CPLUSPLUS
      /// Move constructor.
      by_kind (by_kind&& that) YY_NOEXCEPT;
#endif

      /// Copy constructor.
      by_kind (const by_kind& that) YY_NOEXCEPT;

      /// Constructor from (external) token numbers.
      by_kind (kind_type t) YY_NOEXCEPT;



      /// Record that this symbol is empty.
      void clear () YY_NOEXCEPT;

      /// Steal the symbol kind from \a that.
      void move (by_kind& that);

      /// The (internal) type number (corresponding to \a type).
      /// \a empty when empty.
      symbol_kind_type kind () const YY_NOEXCEPT;

      /// Backward compatibility (Bison 3.6).
      symbol_kind_type type_get () const YY_NOEXCEPT;

      /// The symbol kind.
      /// \a S_YYEMPTY when empty.
      symbol_kind_type kind_;
    };

    /// Backward compatibility for a private implementation detail (Bison 3.6).
    typedef by_kind by_type;

    /// "External" symbols: returned by the scanner.
    struct symbol_type : basic_symbol<by_kind>
    {};

    /// Build a parser object.
    xquery_parser (xquery_driver& driver_yyarg);
    virtual ~xquery_parser ();

#if 201103L <= YY_CPLUSPLUS
    /// Non copyable.
    xquery_parser (const xquery_parser&) = delete;
    /// Non copyable.
    xquery_parser& operator= (const xquery_parser&) = delete;
#endif

    /// Parse.  An alias for parse ().
    /// \returns  0 iff parsing succeeded.
    int operator() ();

    /// Parse.
    /// \returns  0 iff parsing succeeded.
    virtual int parse ();

#if YYDEBUG
    /// The current debugging stream.
    std::ostream& debug_stream () const YY_ATTRIBUTE_PURE;
    /// Set the current debugging stream.
    void set_debug_stream (std::ostream &);

    /// Type for debugging levels.
    typedef int debug_level_type;
    /// The current debugging level.
    debug_level_type debug_level () const YY_ATTRIBUTE_PURE;
    /// Set the current debugging level.
    void set_debug_level (debug_level_type l);
#endif

    /// Report a syntax error.
    /// \param loc    where the syntax error is found.
    /// \param msg    a description of the syntax error.
    virtual void error (const location_type& loc, const std::string& msg);

    /// Report a syntax error.
    void error (const syntax_error& err);

    /// The user-facing name of the symbol whose (internal) number is
    /// YYSYMBOL.  No bounds checking.
    static std::string symbol_name (symbol_kind_type yysymbol);



    class context
    {
    public:
      context (const xquery_parser& yyparser, const symbol_type& yyla);
      const symbol_type& lookahead () const YY_NOEXCEPT { return yyla_; }
      symbol_kind_type token () const YY_NOEXCEPT { return yyla_.kind (); }
      const location_type& location () const YY_NOEXCEPT { return yyla_.location; }

      /// Put in YYARG at most YYARGN of the expected tokens, and return the
      /// number of tokens stored in YYARG.  If YYARG is null, return the
      /// number of expected tokens (guaranteed to be less than YYNTOKENS).
      int expected_tokens (symbol_kind_type yyarg[], int yyargn) const;

    private:
      const xquery_parser& yyparser_;
      const symbol_type& yyla_;
    };

  private:
#if YY_CPLUSPLUS < 201103L
    /// Non copyable.
    xquery_parser (const xquery_parser&);
    /// Non copyable.
    xquery_parser& operator= (const xquery_parser&);
#endif


    /// Stored state numbers (used for stacks).
    typedef short state_type;

    /// The arguments of the error message.
    int yy_syntax_error_arguments_ (const context& yyctx,
                                    symbol_kind_type yyarg[], int yyargn) const;

    /// Generate an error message.
    /// \param yyctx     the context in which the error occurred.
    virtual std::string yysyntax_error_ (const context& yyctx) const;
    /// Compute post-reduction state.
    /// \param yystate   the current state
    /// \param yysym     the nonterminal to push on the stack
    static state_type yy_lr_goto_state_ (state_type yystate, int yysym);

    /// Whether the given \c yypact_ value indicates a defaulted state.
    /// \param yyvalue   the value to check
    static bool yy_pact_value_is_default_ (int yyvalue) YY_NOEXCEPT;

    /// Whether the given \c yytable_ value indicates a syntax error.
    /// \param yyvalue   the value to check
    static bool yy_table_value_is_error_ (int yyvalue) YY_NOEXCEPT;

    static const short yypact_ninf_;
    static const short yytable_ninf_;

    /// Convert a scanner token kind \a t to a symbol kind.
    /// In theory \a t should be a token_kind_type, but character literals
    /// are valid, yet not members of the token_kind_type enum.
    static symbol_kind_type yytranslate_ (int t) YY_NOEXCEPT;

    /// Convert the symbol name \a n to a form suitable for a diagnostic.
    static std::string yytnamerr_ (const char *yystr);

    /// For a symbol, its name in clear.
    static const char* const yytname_[];


    // Tables.
    // YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
    // STATE-NUM.
    static const short yypact_[];

    // YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
    // Performed when YYTABLE does not specify something else to do.  Zero
    // means the default is an error.
    static const short yydefact_[];

    // YYPGOTO[NTERM-NUM].
    static const short yypgoto_[];

    // YYDEFGOTO[NTERM-NUM].
    static const short yydefgoto_[];

    // YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
    // positive, shift that token.  If negative, reduce the rule whose
    // number is the opposite.  If YYTABLE_NINF, syntax error.
    static const short yytable_[];

    static const short yycheck_[];

    // YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
    // state STATE-NUM.
    static const short yystos_[];

    // YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.
    static const short yyr1_[];

    // YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.
    static const signed char yyr2_[];


#if YYDEBUG
    // YYRLINE[YYN] -- Source line where rule number YYN was defined.
    static const short yyrline_[];
    /// Report on the debug stream that the rule \a r is going to be reduced.
    virtual void yy_reduce_print_ (int r) const;
    /// Print the state stack on the debug stream.
    virtual void yy_stack_print_ () const;

    /// Debugging level.
    int yydebug_;
    /// Debug stream.
    std::ostream* yycdebug_;

    /// \brief Display a symbol kind, value and location.
    /// \param yyo    The output stream.
    /// \param yysym  The symbol.
    template <typename Base>
    void yy_print_ (std::ostream& yyo, const basic_symbol<Base>& yysym) const;
#endif

    /// \brief Reclaim the memory associated to a symbol.
    /// \param yymsg     Why this token is reclaimed.
    ///                  If null, print nothing.
    /// \param yysym     The symbol.
    template <typename Base>
    void yy_destroy_ (const char* yymsg, basic_symbol<Base>& yysym) const;

  private:
    /// Type access provider for state based symbols.
    struct by_state
    {
      /// Default constructor.
      by_state () YY_NOEXCEPT;

      /// The symbol kind as needed by the constructor.
      typedef state_type kind_type;

      /// Constructor.
      by_state (kind_type s) YY_NOEXCEPT;

      /// Copy constructor.
      by_state (const by_state& that) YY_NOEXCEPT;

      /// Record that this symbol is empty.
      void clear () YY_NOEXCEPT;

      /// Steal the symbol kind from \a that.
      void move (by_state& that);

      /// The symbol kind (corresponding to \a state).
      /// \a symbol_kind::S_YYEMPTY when empty.
      symbol_kind_type kind () const YY_NOEXCEPT;

      /// The state number used to denote an empty symbol.
      /// We use the initial state, as it does not have a value.
      enum { empty_state = 0 };

      /// The state.
      /// \a empty when empty.
      state_type state;
    };

    /// "Internal" symbol: element of the stack.
    struct stack_symbol_type : basic_symbol<by_state>
    {
      /// Superclass.
      typedef basic_symbol<by_state> super_type;
      /// Construct an empty symbol.
      stack_symbol_type ();
      /// Move or copy construction.
      stack_symbol_type (YY_RVREF (stack_symbol_type) that);
      /// Steal the contents from \a sym to build this.
      stack_symbol_type (state_type s, YY_MOVE_REF (symbol_type) sym);
#if YY_CPLUSPLUS < 201103L
      /// Assignment, needed by push_back by some old implementations.
      /// Moves the contents of that.
      stack_symbol_type& operator= (stack_symbol_type& that);

      /// Assignment, needed by push_back by other implementations.
      /// Needed by some other old implementations.
      stack_symbol_type& operator= (const stack_symbol_type& that);
#endif
    };

    /// A stack with random access from its top.
    template <typename T, typename S = std::vector<T> >
    class stack
    {
    public:
      // Hide our reversed order.
      typedef typename S::iterator iterator;
      typedef typename S::const_iterator const_iterator;
      typedef typename S::size_type size_type;
      typedef typename std::ptrdiff_t index_type;

      stack (size_type n = 200) YY_NOEXCEPT
        : seq_ (n)
      {}

#if 201103L <= YY_CPLUSPLUS
      /// Non copyable.
      stack (const stack&) = delete;
      /// Non copyable.
      stack& operator= (const stack&) = delete;
#endif

      /// Random access.
      ///
      /// Index 0 returns the topmost element.
      const T&
      operator[] (index_type i) const
      {
        return seq_[size_type (size () - 1 - i)];
      }

      /// Random access.
      ///
      /// Index 0 returns the topmost element.
      T&
      operator[] (index_type i)
      {
        return seq_[size_type (size () - 1 - i)];
      }

      /// Steal the contents of \a t.
      ///
      /// Close to move-semantics.
      void
      push (YY_MOVE_REF (T) t)
      {
        seq_.push_back (T ());
        operator[] (0).move (t);
      }

      /// Pop elements from the stack.
      void
      pop (std::ptrdiff_t n = 1) YY_NOEXCEPT
      {
        for (; 0 < n; --n)
          seq_.pop_back ();
      }

      /// Pop all elements from the stack.
      void
      clear () YY_NOEXCEPT
      {
        seq_.clear ();
      }

      /// Number of elements on the stack.
      index_type
      size () const YY_NOEXCEPT
      {
        return index_type (seq_.size ());
      }

      /// Iterator on top of the stack (going downwards).
      const_iterator
      begin () const YY_NOEXCEPT
      {
        return seq_.begin ();
      }

      /// Bottom of the stack.
      const_iterator
      end () const YY_NOEXCEPT
      {
        return seq_.end ();
      }

      /// Present a slice of the top of a stack.
      class slice
      {
      public:
        slice (const stack& stack, index_type range) YY_NOEXCEPT
          : stack_ (stack)
          , range_ (range)
        {}

        const T&
        operator[] (index_type i) const
        {
          return stack_[range_ - i];
        }

      private:
        const stack& stack_;
        index_type range_;
      };

    private:
#if YY_CPLUSPLUS < 201103L
      /// Non copyable.
      stack (const stack&);
      /// Non copyable.
      stack& operator= (const stack&);
#endif
      /// The wrapped container.
      S seq_;
    };


    /// Stack type.
    typedef stack<stack_symbol_type> stack_type;

    /// The stack.
    stack_type yystack_;

    /// Push a new state on the stack.
    /// \param m    a debug message to display
    ///             if null, no trace is output.
    /// \param sym  the symbol
    /// \warning the contents of \a s.value is stolen.
    void yypush_ (const char* m, YY_MOVE_REF (stack_symbol_type) sym);

    /// Push a new look ahead token on the state on the stack.
    /// \param m    a debug message to display
    ///             if null, no trace is output.
    /// \param s    the state
    /// \param sym  the symbol (for its value and location).
    /// \warning the contents of \a sym.value is stolen.
    void yypush_ (const char* m, state_type s, YY_MOVE_REF (symbol_type) sym);

    /// Pop \a n symbols from the stack.
    void yypop_ (int n = 1) YY_NOEXCEPT;

    /// Constants.
    enum
    {
      yylast_ = 18075,     ///< Last index in yytable_.
      yynnts_ = 326,  ///< Number of nonterminal symbols.
      yyfinal_ = 618 ///< Termination state number.
    };


    // User arguments.
    xquery_driver& driver;

  };


} // zorba




#endif // !YY_YY_XQUERY_PARSER_HPP_INCLUDED
