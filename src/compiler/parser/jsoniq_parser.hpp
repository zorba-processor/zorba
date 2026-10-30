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
 ** \file jsoniq_parser.hpp
 ** Define the zorba::parser class.
 */

// C++ LALR(1) parser skeleton written by Akim Demaille.

// DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
// especially those whose name start with YY_ or yy_.  They are
// private implementation details that can be changed or removed.

#ifndef YY_YY_JSONIQ_PARSER_HPP_INCLUDED
# define YY_YY_JSONIQ_PARSER_HPP_INCLUDED
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
  class jsoniq_parser
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
    COMP_PI_NCNAME_LBRACE = 278,   // "'processing-instruction NCName {'"
    COMP_NS_NCNAME_LBRACE = 279,   // "'namespace NCName {'"
    QNAME_SVAL = 280,              // "'QName'"
    EQNAME_SVAL = 281,             // "'EQName'"
    ANNOTATION_QNAME_SVAL = 282,   // "'%QName'"
    ANNOTATION_EQNAME_SVAL = 283,  // "'%EQName'"
    QUOTE_ATTR_CONTENT = 284,      // "'quote attribute content'"
    STRING_LITERAL = 285,          // "'STRING'"
    XML_COMMENT_LITERAL = 286,     // "'XML comment'"
    DECLARE = 288,                 // "'declare'"
    MODULE = 289,                  // "'module'"
    BLANK = 290,                   // "'<blank>'"
    ALLOWING = 291,                // "'allowing'"
    BASE_URI = 292,                // "'base-uri'"
    BOUNDARY_SPACE = 293,          // "'boundary-space'"
    COMMENT = 294,                 // "'comment'"
    CONSTRUCTION = 295,            // "'construction'"
    COPY_NAMESPACES = 296,         // "'copy-namespaces'"
    COUNT = 297,                   // "'count'"
    DOCUMENT = 298,                // "'document'"
    DOCUMENT_NODE = 299,           // "'document-node'"
    ELEMENT = 300,                 // "'element'"
    FOR = 301,                     // "'for'"
    FUNCTION = 302,                // "'function'"
    IF = 303,                      // "'if'"
    IMPORT = 304,                  // "'import'"
    INSTANCE = 305,                // "'instance'"
    LET = 306,                     // "'let'"
    MOST = 307,                    // "'most'"
    NS_NODE = 308,                 // "'namespace-node'"
    NEXT = 309,                    // "'next'"
    NO = 310,                      // "'no'"
    ONLY = 311,                    // "'only'"
    OPTION = 312,                  // "'option'"
    ORDERING = 313,                // "'ordering'"
    PREVIOUS = 314,                // "'previous'"
    PROCESSING_INSTRUCTION = 315,  // "'processing-instruction'"
    SCHEMA = 316,                  // "'schema'"
    SCHEMA_ATTRIBUTE = 317,        // "'schema-attribute'"
    SCHEMA_ELEMENT = 318,          // "'schema-element'"
    SEQUENTIAL = 319,              // "'sequential'"
    SET = 320,                     // "'set'"
    SIMPLE = 321,                  // "'simple'"
    SLIDING = 322,                 // "'sliding'"
    SOME = 323,                    // "'some'"
    SPACE = 324,                   // "'space'"
    STABLE = 325,                  // "'stable'"
    TEXT = 326,                    // "'text'"
    TUMBLING = 327,                // "'tumbling'"
    SWITCH = 328,                  // "'switch'"
    TYPESWITCH = 329,              // "'typeswitch'"
    UPDATING = 330,                // "'updating'"
    VALIDATE = 331,                // "'validate'"
    TYPE = 332,                    // "'type'"
    WHEN = 333,                    // "'when'"
    WORD = 334,                    // "'word'"
    DECIMAL_FORMAT = 335,          // "'decimal-format'"
    DECIMAL_SEPARATOR = 336,       // "'decimal-separator'"
    GROUPING_SEPARATOR = 337,      // "'grouping-separator'"
    INFINITY_VALUE = 338,          // "'infinity'"
    MINUS_SIGN = 339,              // "'minus-sign'"
    NaN = 340,                     // "'NaN'"
    PERCENT = 341,                 // "'percent'"
    PER_MILLE = 342,               // "'per-mille'"
    ZERO_DIGIT = 343,              // "'zero-digit'"
    DIGIT = 344,                   // "'digit'"
    PATTERN_SEPARATOR = 345,       // "'pattern-separator'"
    ANCESTOR = 346,                // "'ancestor'"
    ANCESTOR_OR_SELF = 347,        // "'ancestor-or-self'"
    AND = 348,                     // "'and'"
    APOS = 349,                    // "'''"
    AS = 350,                      // "'as'"
    ASCENDING = 351,               // "'ascending'"
    AT = 352,                      // "'at'"
    ATTRIBUTE = 353,               // "'attribute'"
    AT_SIGN = 354,                 // "'@'"
    CONCAT = 355,                  // "'||'"
    CASE = 356,                    // "'case'"
    CASTABLE = 357,                // "'castable'"
    CAST = 358,                    // "'cast'"
    CDATA_BEGIN = 359,             // "'CDATA[['"
    CDATA_END = 360,               // "']]'"
    CHILD = 361,                   // "'child'"
    COLLATION = 362,               // "'collation'"
    COMMA = 363,                   // "','"
    COMMENT_BEGIN = 364,           // "'(:'"
    COMMENT_END = 365,             // "':)'"
    DECIMAL_LITERAL = 366,         // "'DECIMAL'"
    CONTEXT = 367,                 // "'context'"
    VARIABLE = 368,                // "'variable'"
    DEFAULT = 369,                 // "'default'"
    DESCENDANT = 370,              // "'descendant'"
    DESCENDANT_OR_SELF = 371,      // "'descendant-or-self'"
    DESCENDING = 372,              // "'descending'"
    DIV = 373,                     // "'div'"
    DOLLAR = 374,                  // "'$'"
    DOLLAR_DOLLAR = 375,           // "'$_'"
    DOT = 376,                     // "'.'"
    DOT_DOT = 377,                 // "'..'"
    COLON = 378,                   // "':'"
    DOUBLE_COLON = 379,            // "'::'"
    DOUBLE_LBRACE = 380,           // "'{{'"
    DOUBLE_LITERAL = 381,          // "'DOUBLE'"
    DOUBLE_RBRACE = 382,           // "'<double {>'"
    ELSE = 383,                    // "'else'"
    _EMPTY = 384,                  // "'empty'"
    GREATEST = 385,                // "'greatest'"
    LEAST = 386,                   // "'least'"
    EMPTY_TAG_END = 387,           // "'/>'"
    ENCODING = 388,                // "'encoding'"
    EQUALS = 389,                  // "'='"
    ESCAPE_APOS = 390,             // "''''"
    ESCAPE_QUOTE = 391,            // "'\"\"'"
    EVERY = 392,                   // "'every'"
    EXCEPT = 393,                  // "'except'"
    EXTERNAL = 394,                // "'external'"
    FOLLOWING = 395,               // "'following'"
    FOLLOWING_SIBLING = 396,       // "'following-sibling'"
    FOLLOWS = 397,                 // "'follows'"
    GE = 398,                      // "'>='"
    GETS = 399,                    // "':='"
    GT = 400,                      // "'>'"
    HOOK = 401,                    // "'?'"
    HASH = 402,                    // "'#'"
    IDIV = 403,                    // "'idiv'"
    _IN = 404,                     // "'in'"
    INHERIT = 405,                 // "'inherit'"
    INTEGER_LITERAL = 406,         // "'INTEGER'"
    INTERSECT = 407,               // "'intersect'"
    IS = 408,                      // "'is'"
    ITEM = 409,                    // "'item'"
    LBRACE = 410,                  // "'{'"
    LBRACK = 411,                  // "'['"
    LE = 412,                      // "'<='"
    LPAR = 413,                    // "'('"
    LT_OR_START_TAG = 414,         // "'<'"
    MINUS = 415,                   // "'-'"
    MOD = 416,                     // "'mod'"
    NAMESPACE = 417,               // "'namespace'"
    _NAN = 418,                    // "'nan'"
    NE = 419,                      // "'!='"
    NODECOMP = 420,                // "'nodecomp'"
    NO_INHERIT = 421,              // "'<no inherit>'"
    NO_PRESERVE = 422,             // "'<no preserve>'"
    OR = 423,                      // "'or'"
    ORDER = 424,                   // "'order'"
    ORDERED = 425,                 // "'ordered'"
    BY = 426,                      // "'by'"
    GROUP = 427,                   // "'group'"
    PARENT = 428,                  // "'parent'"
    PERCENTAGE = 429,              // "'%'"
    PI_BEGIN = 430,                // "'<?'"
    PI_END = 431,                  // "'?>'"
    PLUS = 432,                    // "'+'"
    PRAGMA_BEGIN = 433,            // "'(#'"
    PRAGMA_END = 434,              // "'#)'"
    PRECEDES = 435,                // "'<<'"
    PRECEDING = 436,               // "'preceding'"
    PRECEDING_SIBLING = 437,       // "'preceding-sibling'"
    PRESERVE = 438,                // "'preserve'"
    QUOTE = 439,                   // "'\"'"
    RBRACE = 440,                  // "'}'"
    RBRACK = 441,                  // "']'"
    RETURN = 442,                  // "'return'"
    RPAR = 443,                    // "')'"
    SATISFIES = 444,               // "'satisfies'"
    SELECT = 445,                  // "'select'"
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
    JSONIQ = 473,                  // "'jsoniq'"
    VERSION = 474,                 // "'version'"
    START = 475,                   // "'start'"
    NULL_TOKEN = 476,              // "'null'"
    TRUE_TOKEN = 477,              // "'true'"
    FALSE_TOKEN = 478,             // "'false'"
    AFTER = 479,                   // "'after'"
    BEFORE = 480,                  // "'before'"
    REVALIDATION = 481,            // "'revalidation'"
    _STRICT = 482,                 // "'strict'"
    LAX = 483,                     // "'lax'"
    SKIP = 484,                    // "'skip'"
    _DELETE = 485,                 // "'delete'"
    NODE = 486,                    // "'node'"
    INSERT = 487,                  // "'insert'"
    NODES = 488,                   // "'nodes'"
    RENAME = 489,                  // "'rename'"
    REPLACE = 490,                 // "'replace'"
    VALUE = 491,                   // "'value'"
    OF = 492,                      // "'of'"
    FIRST = 493,                   // "'first'"
    INTO = 494,                    // "'into'"
    LAST = 495,                    // "'last'"
    MODIFY = 496,                  // "'modify'"
    COPY = 497,                    // "'copy'"
    WITH = 498,                    // "'with'"
    BREAK = 499,                   // "'break'"
    CONTINUE = 500,                // "'continue'"
    EXIT = 501,                    // "'exit'"
    LOOP = 502,                    // "'loop'"
    RETURNING = 503,               // "'returning'"
    WHILE = 504,                   // "'while'"
    TRY = 505,                     // "'try'"
    CATCH = 506,                   // "'catch'"
    USING = 507,                   // "'using'"
    ALL = 508,                     // "'all'"
    ANY = 509,                     // "'any'"
    CONTAINS = 510,                // "'contains'"
    CONTENT = 511,                 // "'content'"
    DIACRITICS = 512,              // "'diacritics'"
    DIFFERENT = 513,               // "'different'"
    DISTANCE = 514,                // "'distance'"
    END = 515,                     // "'end'"
    ENTIRE = 516,                  // "'entire'"
    EXACTLY = 517,                 // "'exactly'"
    FROM = 518,                    // "'from'"
    FTAND = 519,                   // "'ftand'"
    FTNOT = 520,                   // "'ftnot'"
    NOT = 521,                     // "'not'"
    FT_OPTION = 522,               // "'ft-option'"
    FTOR = 523,                    // "'ftor'"
    INSENSITIVE = 524,             // "'insensitive'"
    LANGUAGE = 525,                // "'language'"
    LEVELS = 526,                  // "'levels'"
    LOWERCASE = 527,               // "'lowercase'"
    OCCURS = 528,                  // "'occurs'"
    PARAGRAPH = 529,               // "'paragraph'"
    PARAGRAPHS = 530,              // "'paragraphs'"
    PHRASE = 531,                  // "'phrase'"
    RELATIONSHIP = 532,            // "'relationship'"
    SAME = 533,                    // "'same'"
    SCORE = 534,                   // "'score'"
    SENSITIVE = 535,               // "'sensitive'"
    SENTENCE = 536,                // "'sentence'"
    SENTENCES = 537,               // "'sentences'"
    STEMMING = 538,                // "'stemming'"
    STOP = 539,                    // "'stop'"
    THESAURUS = 540,               // "'thesaurus'"
    TIMES = 541,                   // "'times'"
    UPPERCASE = 542,               // "'uppercase'"
    WEIGHT = 543,                  // "'weight'"
    WILDCARDS = 544,               // "'wildcards'"
    WINDOW = 545,                  // "'window'"
    WITHOUT = 546,                 // "'without'"
    WORDS = 547,                   // "'words'"
    COLLECTION = 548,              // "'collection'"
    CONSTOPT = 549,                // "'const'"
    APPEND_ONLY = 550,             // "'append-only'"
    QUEUE = 551,                   // "'queue'"
    MUTABLE = 552,                 // "'mutable'"
    READ_ONLY = 553,               // "'read-only'"
    UNIQUE = 554,                  // "'unique'"
    NON = 555,                     // "'non'"
    INDEX = 556,                   // "'index'"
    MANUALLY = 557,                // "'manually'"
    AUTOMATICALLY = 558,           // "'automatically'"
    MAINTAINED = 559,              // "'maintained'"
    ON = 560,                      // "'on'"
    RANGE = 561,                   // "'range'"
    EQUALITY = 562,                // "'equality'"
    GENERAL = 563,                 // "'general'"
    INTEGRITY = 564,               // "'integrity'"
    CONSTRAINT = 565,              // "'constraint'"
    CHECK = 566,                   // "'check'"
    KEY = 567,                     // "'key'"
    FOREACH = 568,                 // "'foreach'"
    FOREIGN = 569,                 // "'foreign'"
    KEYS = 570,                    // "'keys'"
    L_SIMPLE_OBJ_UNION = 571,      // "'{|'"
    R_SIMPLE_OBJ_UNION = 572,      // "'|}'"
    L_ACCUMULATOR_OBJ_UNION = 573, // "'{['"
    R_ACCUMULATOR_OBJ_UNION = 574, // "']}'"
    JSON = 575,                    // "'json'"
    APPEND = 576,                  // "'append'"
    POSITION = 577,                // "'position'"
    OBJECT = 578,                  // "'object'"
    ARRAY = 579,                   // "'array'"
    JSON_ITEM = 580,               // "'json-item'"
    STRUCTURED_ITEM = 581,         // "'structured-item'"
    BYTE_ORDER_MARK_UTF8 = 582,    // "'BOM_UTF8'"
    SHEBANG = 583,                 // "'#!/shebang"
    RANGE_REDUCE = 584,            // RANGE_REDUCE
    SEQUENCE_TYPE_REDUCE = 585,    // SEQUENCE_TYPE_REDUCE
    ADDITIVE_REDUCE = 586,         // ADDITIVE_REDUCE
    UNION_REDUCE = 587,            // UNION_REDUCE
    INTERSECT_EXCEPT_REDUCE = 588, // INTERSECT_EXCEPT_REDUCE
    QVARINDECLLIST_REDUCE = 589,   // QVARINDECLLIST_REDUCE
    UNARY_PREC = 590,              // UNARY_PREC
    STEP_REDUCE = 591,             // STEP_REDUCE
    SIMPLEMAPEXPR_REDUCE = 592,    // SIMPLEMAPEXPR_REDUCE
    MULTIPLICATIVE_REDUCE = 593,   // MULTIPLICATIVE_REDUCE
    JSONLOOKUPEXPR_REDUCE = 594    // JSONLOOKUPEXPR_REDUCE
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
        YYNTOKENS = 340, ///< Number of tokens.
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
        S_COMP_PI_NCNAME_LBRACE = 23,            // "'processing-instruction NCName {'"
        S_COMP_NS_NCNAME_LBRACE = 24,            // "'namespace NCName {'"
        S_QNAME_SVAL = 25,                       // "'QName'"
        S_EQNAME_SVAL = 26,                      // "'EQName'"
        S_ANNOTATION_QNAME_SVAL = 27,            // "'%QName'"
        S_ANNOTATION_EQNAME_SVAL = 28,           // "'%EQName'"
        S_QUOTE_ATTR_CONTENT = 29,               // "'quote attribute content'"
        S_STRING_LITERAL = 30,                   // "'STRING'"
        S_XML_COMMENT_LITERAL = 31,              // "'XML comment'"
        S_32_URI_ = 32,                          // "'URI'"
        S_DECLARE = 33,                          // "'declare'"
        S_MODULE = 34,                           // "'module'"
        S_BLANK = 35,                            // "'<blank>'"
        S_ALLOWING = 36,                         // "'allowing'"
        S_BASE_URI = 37,                         // "'base-uri'"
        S_BOUNDARY_SPACE = 38,                   // "'boundary-space'"
        S_COMMENT = 39,                          // "'comment'"
        S_CONSTRUCTION = 40,                     // "'construction'"
        S_COPY_NAMESPACES = 41,                  // "'copy-namespaces'"
        S_COUNT = 42,                            // "'count'"
        S_DOCUMENT = 43,                         // "'document'"
        S_DOCUMENT_NODE = 44,                    // "'document-node'"
        S_ELEMENT = 45,                          // "'element'"
        S_FOR = 46,                              // "'for'"
        S_FUNCTION = 47,                         // "'function'"
        S_IF = 48,                               // "'if'"
        S_IMPORT = 49,                           // "'import'"
        S_INSTANCE = 50,                         // "'instance'"
        S_LET = 51,                              // "'let'"
        S_MOST = 52,                             // "'most'"
        S_NS_NODE = 53,                          // "'namespace-node'"
        S_NEXT = 54,                             // "'next'"
        S_NO = 55,                               // "'no'"
        S_ONLY = 56,                             // "'only'"
        S_OPTION = 57,                           // "'option'"
        S_ORDERING = 58,                         // "'ordering'"
        S_PREVIOUS = 59,                         // "'previous'"
        S_PROCESSING_INSTRUCTION = 60,           // "'processing-instruction'"
        S_SCHEMA = 61,                           // "'schema'"
        S_SCHEMA_ATTRIBUTE = 62,                 // "'schema-attribute'"
        S_SCHEMA_ELEMENT = 63,                   // "'schema-element'"
        S_SEQUENTIAL = 64,                       // "'sequential'"
        S_SET = 65,                              // "'set'"
        S_SIMPLE = 66,                           // "'simple'"
        S_SLIDING = 67,                          // "'sliding'"
        S_SOME = 68,                             // "'some'"
        S_SPACE = 69,                            // "'space'"
        S_STABLE = 70,                           // "'stable'"
        S_TEXT = 71,                             // "'text'"
        S_TUMBLING = 72,                         // "'tumbling'"
        S_SWITCH = 73,                           // "'switch'"
        S_TYPESWITCH = 74,                       // "'typeswitch'"
        S_UPDATING = 75,                         // "'updating'"
        S_VALIDATE = 76,                         // "'validate'"
        S_TYPE = 77,                             // "'type'"
        S_WHEN = 78,                             // "'when'"
        S_WORD = 79,                             // "'word'"
        S_DECIMAL_FORMAT = 80,                   // "'decimal-format'"
        S_DECIMAL_SEPARATOR = 81,                // "'decimal-separator'"
        S_GROUPING_SEPARATOR = 82,               // "'grouping-separator'"
        S_INFINITY_VALUE = 83,                   // "'infinity'"
        S_MINUS_SIGN = 84,                       // "'minus-sign'"
        S_NaN = 85,                              // "'NaN'"
        S_PERCENT = 86,                          // "'percent'"
        S_PER_MILLE = 87,                        // "'per-mille'"
        S_ZERO_DIGIT = 88,                       // "'zero-digit'"
        S_DIGIT = 89,                            // "'digit'"
        S_PATTERN_SEPARATOR = 90,                // "'pattern-separator'"
        S_ANCESTOR = 91,                         // "'ancestor'"
        S_ANCESTOR_OR_SELF = 92,                 // "'ancestor-or-self'"
        S_AND = 93,                              // "'and'"
        S_APOS = 94,                             // "'''"
        S_AS = 95,                               // "'as'"
        S_ASCENDING = 96,                        // "'ascending'"
        S_AT = 97,                               // "'at'"
        S_ATTRIBUTE = 98,                        // "'attribute'"
        S_AT_SIGN = 99,                          // "'@'"
        S_CONCAT = 100,                          // "'||'"
        S_CASE = 101,                            // "'case'"
        S_CASTABLE = 102,                        // "'castable'"
        S_CAST = 103,                            // "'cast'"
        S_CDATA_BEGIN = 104,                     // "'CDATA[['"
        S_CDATA_END = 105,                       // "']]'"
        S_CHILD = 106,                           // "'child'"
        S_COLLATION = 107,                       // "'collation'"
        S_COMMA = 108,                           // "','"
        S_COMMENT_BEGIN = 109,                   // "'(:'"
        S_COMMENT_END = 110,                     // "':)'"
        S_DECIMAL_LITERAL = 111,                 // "'DECIMAL'"
        S_CONTEXT = 112,                         // "'context'"
        S_VARIABLE = 113,                        // "'variable'"
        S_DEFAULT = 114,                         // "'default'"
        S_DESCENDANT = 115,                      // "'descendant'"
        S_DESCENDANT_OR_SELF = 116,              // "'descendant-or-self'"
        S_DESCENDING = 117,                      // "'descending'"
        S_DIV = 118,                             // "'div'"
        S_DOLLAR = 119,                          // "'$'"
        S_DOLLAR_DOLLAR = 120,                   // "'$_'"
        S_DOT = 121,                             // "'.'"
        S_DOT_DOT = 122,                         // "'..'"
        S_COLON = 123,                           // "':'"
        S_DOUBLE_COLON = 124,                    // "'::'"
        S_DOUBLE_LBRACE = 125,                   // "'{{'"
        S_DOUBLE_LITERAL = 126,                  // "'DOUBLE'"
        S_DOUBLE_RBRACE = 127,                   // "'<double {>'"
        S_ELSE = 128,                            // "'else'"
        S__EMPTY = 129,                          // "'empty'"
        S_GREATEST = 130,                        // "'greatest'"
        S_LEAST = 131,                           // "'least'"
        S_EMPTY_TAG_END = 132,                   // "'/>'"
        S_ENCODING = 133,                        // "'encoding'"
        S_EQUALS = 134,                          // "'='"
        S_ESCAPE_APOS = 135,                     // "''''"
        S_ESCAPE_QUOTE = 136,                    // "'\"\"'"
        S_EVERY = 137,                           // "'every'"
        S_EXCEPT = 138,                          // "'except'"
        S_EXTERNAL = 139,                        // "'external'"
        S_FOLLOWING = 140,                       // "'following'"
        S_FOLLOWING_SIBLING = 141,               // "'following-sibling'"
        S_FOLLOWS = 142,                         // "'follows'"
        S_GE = 143,                              // "'>='"
        S_GETS = 144,                            // "':='"
        S_GT = 145,                              // "'>'"
        S_HOOK = 146,                            // "'?'"
        S_HASH = 147,                            // "'#'"
        S_IDIV = 148,                            // "'idiv'"
        S__IN = 149,                             // "'in'"
        S_INHERIT = 150,                         // "'inherit'"
        S_INTEGER_LITERAL = 151,                 // "'INTEGER'"
        S_INTERSECT = 152,                       // "'intersect'"
        S_IS = 153,                              // "'is'"
        S_ITEM = 154,                            // "'item'"
        S_LBRACE = 155,                          // "'{'"
        S_LBRACK = 156,                          // "'['"
        S_LE = 157,                              // "'<='"
        S_LPAR = 158,                            // "'('"
        S_LT_OR_START_TAG = 159,                 // "'<'"
        S_MINUS = 160,                           // "'-'"
        S_MOD = 161,                             // "'mod'"
        S_NAMESPACE = 162,                       // "'namespace'"
        S__NAN = 163,                            // "'nan'"
        S_NE = 164,                              // "'!='"
        S_NODECOMP = 165,                        // "'nodecomp'"
        S_NO_INHERIT = 166,                      // "'<no inherit>'"
        S_NO_PRESERVE = 167,                     // "'<no preserve>'"
        S_OR = 168,                              // "'or'"
        S_ORDER = 169,                           // "'order'"
        S_ORDERED = 170,                         // "'ordered'"
        S_BY = 171,                              // "'by'"
        S_GROUP = 172,                           // "'group'"
        S_PARENT = 173,                          // "'parent'"
        S_PERCENTAGE = 174,                      // "'%'"
        S_PI_BEGIN = 175,                        // "'<?'"
        S_PI_END = 176,                          // "'?>'"
        S_PLUS = 177,                            // "'+'"
        S_PRAGMA_BEGIN = 178,                    // "'(#'"
        S_PRAGMA_END = 179,                      // "'#)'"
        S_PRECEDES = 180,                        // "'<<'"
        S_PRECEDING = 181,                       // "'preceding'"
        S_PRECEDING_SIBLING = 182,               // "'preceding-sibling'"
        S_PRESERVE = 183,                        // "'preserve'"
        S_QUOTE = 184,                           // "'\"'"
        S_RBRACE = 185,                          // "'}'"
        S_RBRACK = 186,                          // "']'"
        S_RETURN = 187,                          // "'return'"
        S_RPAR = 188,                            // "')'"
        S_SATISFIES = 189,                       // "'satisfies'"
        S_SELECT = 190,                          // "'select'"
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
        S_JSONIQ = 218,                          // "'jsoniq'"
        S_VERSION = 219,                         // "'version'"
        S_START = 220,                           // "'start'"
        S_NULL_TOKEN = 221,                      // "'null'"
        S_TRUE_TOKEN = 222,                      // "'true'"
        S_FALSE_TOKEN = 223,                     // "'false'"
        S_AFTER = 224,                           // "'after'"
        S_BEFORE = 225,                          // "'before'"
        S_REVALIDATION = 226,                    // "'revalidation'"
        S__STRICT = 227,                         // "'strict'"
        S_LAX = 228,                             // "'lax'"
        S_SKIP = 229,                            // "'skip'"
        S__DELETE = 230,                         // "'delete'"
        S_NODE = 231,                            // "'node'"
        S_INSERT = 232,                          // "'insert'"
        S_NODES = 233,                           // "'nodes'"
        S_RENAME = 234,                          // "'rename'"
        S_REPLACE = 235,                         // "'replace'"
        S_VALUE = 236,                           // "'value'"
        S_OF = 237,                              // "'of'"
        S_FIRST = 238,                           // "'first'"
        S_INTO = 239,                            // "'into'"
        S_LAST = 240,                            // "'last'"
        S_MODIFY = 241,                          // "'modify'"
        S_COPY = 242,                            // "'copy'"
        S_WITH = 243,                            // "'with'"
        S_BREAK = 244,                           // "'break'"
        S_CONTINUE = 245,                        // "'continue'"
        S_EXIT = 246,                            // "'exit'"
        S_LOOP = 247,                            // "'loop'"
        S_RETURNING = 248,                       // "'returning'"
        S_WHILE = 249,                           // "'while'"
        S_TRY = 250,                             // "'try'"
        S_CATCH = 251,                           // "'catch'"
        S_USING = 252,                           // "'using'"
        S_ALL = 253,                             // "'all'"
        S_ANY = 254,                             // "'any'"
        S_CONTAINS = 255,                        // "'contains'"
        S_CONTENT = 256,                         // "'content'"
        S_DIACRITICS = 257,                      // "'diacritics'"
        S_DIFFERENT = 258,                       // "'different'"
        S_DISTANCE = 259,                        // "'distance'"
        S_END = 260,                             // "'end'"
        S_ENTIRE = 261,                          // "'entire'"
        S_EXACTLY = 262,                         // "'exactly'"
        S_FROM = 263,                            // "'from'"
        S_FTAND = 264,                           // "'ftand'"
        S_FTNOT = 265,                           // "'ftnot'"
        S_NOT = 266,                             // "'not'"
        S_FT_OPTION = 267,                       // "'ft-option'"
        S_FTOR = 268,                            // "'ftor'"
        S_INSENSITIVE = 269,                     // "'insensitive'"
        S_LANGUAGE = 270,                        // "'language'"
        S_LEVELS = 271,                          // "'levels'"
        S_LOWERCASE = 272,                       // "'lowercase'"
        S_OCCURS = 273,                          // "'occurs'"
        S_PARAGRAPH = 274,                       // "'paragraph'"
        S_PARAGRAPHS = 275,                      // "'paragraphs'"
        S_PHRASE = 276,                          // "'phrase'"
        S_RELATIONSHIP = 277,                    // "'relationship'"
        S_SAME = 278,                            // "'same'"
        S_SCORE = 279,                           // "'score'"
        S_SENSITIVE = 280,                       // "'sensitive'"
        S_SENTENCE = 281,                        // "'sentence'"
        S_SENTENCES = 282,                       // "'sentences'"
        S_STEMMING = 283,                        // "'stemming'"
        S_STOP = 284,                            // "'stop'"
        S_THESAURUS = 285,                       // "'thesaurus'"
        S_TIMES = 286,                           // "'times'"
        S_UPPERCASE = 287,                       // "'uppercase'"
        S_WEIGHT = 288,                          // "'weight'"
        S_WILDCARDS = 289,                       // "'wildcards'"
        S_WINDOW = 290,                          // "'window'"
        S_WITHOUT = 291,                         // "'without'"
        S_WORDS = 292,                           // "'words'"
        S_COLLECTION = 293,                      // "'collection'"
        S_CONSTOPT = 294,                        // "'const'"
        S_APPEND_ONLY = 295,                     // "'append-only'"
        S_QUEUE = 296,                           // "'queue'"
        S_MUTABLE = 297,                         // "'mutable'"
        S_READ_ONLY = 298,                       // "'read-only'"
        S_UNIQUE = 299,                          // "'unique'"
        S_NON = 300,                             // "'non'"
        S_INDEX = 301,                           // "'index'"
        S_MANUALLY = 302,                        // "'manually'"
        S_AUTOMATICALLY = 303,                   // "'automatically'"
        S_MAINTAINED = 304,                      // "'maintained'"
        S_ON = 305,                              // "'on'"
        S_RANGE = 306,                           // "'range'"
        S_EQUALITY = 307,                        // "'equality'"
        S_GENERAL = 308,                         // "'general'"
        S_INTEGRITY = 309,                       // "'integrity'"
        S_CONSTRAINT = 310,                      // "'constraint'"
        S_CHECK = 311,                           // "'check'"
        S_KEY = 312,                             // "'key'"
        S_FOREACH = 313,                         // "'foreach'"
        S_FOREIGN = 314,                         // "'foreign'"
        S_KEYS = 315,                            // "'keys'"
        S_L_SIMPLE_OBJ_UNION = 316,              // "'{|'"
        S_R_SIMPLE_OBJ_UNION = 317,              // "'|}'"
        S_L_ACCUMULATOR_OBJ_UNION = 318,         // "'{['"
        S_R_ACCUMULATOR_OBJ_UNION = 319,         // "']}'"
        S_JSON = 320,                            // "'json'"
        S_APPEND = 321,                          // "'append'"
        S_POSITION = 322,                        // "'position'"
        S_OBJECT = 323,                          // "'object'"
        S_ARRAY = 324,                           // "'array'"
        S_JSON_ITEM = 325,                       // "'json-item'"
        S_STRUCTURED_ITEM = 326,                 // "'structured-item'"
        S_BYTE_ORDER_MARK_UTF8 = 327,            // "'BOM_UTF8'"
        S_SHEBANG = 328,                         // "'#!/shebang"
        S_RANGE_REDUCE = 329,                    // RANGE_REDUCE
        S_SEQUENCE_TYPE_REDUCE = 330,            // SEQUENCE_TYPE_REDUCE
        S_ADDITIVE_REDUCE = 331,                 // ADDITIVE_REDUCE
        S_UNION_REDUCE = 332,                    // UNION_REDUCE
        S_INTERSECT_EXCEPT_REDUCE = 333,         // INTERSECT_EXCEPT_REDUCE
        S_QVARINDECLLIST_REDUCE = 334,           // QVARINDECLLIST_REDUCE
        S_UNARY_PREC = 335,                      // UNARY_PREC
        S_STEP_REDUCE = 336,                     // STEP_REDUCE
        S_SIMPLEMAPEXPR_REDUCE = 337,            // SIMPLEMAPEXPR_REDUCE
        S_MULTIPLICATIVE_REDUCE = 338,           // MULTIPLICATIVE_REDUCE
        S_JSONLOOKUPEXPR_REDUCE = 339,           // JSONLOOKUPEXPR_REDUCE
        S_YYACCEPT = 340,                        // $accept
        S_Module = 341,                          // Module
        S_ERROR = 342,                           // ERROR
        S_ModuleWithoutBOM = 343,                // ModuleWithoutBOM
        S_VersionDecl = 344,                     // VersionDecl
        S_MainModule = 345,                      // MainModule
        S_LibraryModule = 346,                   // LibraryModule
        S_ModuleDecl = 347,                      // ModuleDecl
        S_SIND_DeclList = 348,                   // SIND_DeclList
        S_SIND_Decl = 349,                       // SIND_Decl
        S_Setter = 350,                          // Setter
        S_BoundarySpaceDecl = 351,               // BoundarySpaceDecl
        S_DefaultCollationDecl = 352,            // DefaultCollationDecl
        S_BaseURIDecl = 353,                     // BaseURIDecl
        S_ConstructionDecl = 354,                // ConstructionDecl
        S_OrderingModeDecl = 355,                // OrderingModeDecl
        S_EmptyOrderDecl = 356,                  // EmptyOrderDecl
        S_CopyNamespacesDecl = 357,              // CopyNamespacesDecl
        S_Import = 358,                          // Import
        S_SchemaImport = 359,                    // SchemaImport
        S_URILiteralList = 360,                  // URILiteralList
        S_SchemaPrefix = 361,                    // SchemaPrefix
        S_ModuleImport = 362,                    // ModuleImport
        S_NamespaceDecl = 363,                   // NamespaceDecl
        S_DefaultNamespaceDecl = 364,            // DefaultNamespaceDecl
        S_VFO_DeclList = 365,                    // VFO_DeclList
        S_VFO_Decl = 366,                        // VFO_Decl
        S_DecimalFormatDecl = 367,               // DecimalFormatDecl
        S_DecimalFormatParamList = 368,          // DecimalFormatParamList
        S_DecimalFormatParam = 369,              // DecimalFormatParam
        S_DecimalFormatParamName = 370,          // DecimalFormatParamName
        S_OptionDecl = 371,                      // OptionDecl
        S_FTOptionDecl = 372,                    // FTOptionDecl
        S_CtxItemDecl = 373,                     // CtxItemDecl
        S_CtxItemDecl2 = 374,                    // CtxItemDecl2
        S_CtxItemDecl3 = 375,                    // CtxItemDecl3
        S_CtxItemDecl4 = 376,                    // CtxItemDecl4
        S_VarDecl = 377,                         // VarDecl
        S_VarNameAndType = 378,                  // VarNameAndType
        S_AnnotationList = 379,                  // AnnotationList
        S_Annotation = 380,                      // Annotation
        S_AnnotationLiteralList = 381,           // AnnotationLiteralList
        S_FunctionDecl = 382,                    // FunctionDecl
        S_FunctionDecl2 = 383,                   // FunctionDecl2
        S_FunctionDeclSimple = 384,              // FunctionDeclSimple
        S_FunctionDeclUpdating = 385,            // FunctionDeclUpdating
        S_FunctionSig = 386,                     // FunctionSig
        S_ParamList = 387,                       // ParamList
        S_Param = 388,                           // Param
        S_CollectionDecl = 389,                  // CollectionDecl
        S_CollectionTypeDecl = 390,              // CollectionTypeDecl
        S_IndexDecl = 391,                       // IndexDecl
        S_IndexKeyList = 392,                    // IndexKeyList
        S_IndexKeySpec = 393,                    // IndexKeySpec
        S_IntegrityConstraintDecl = 394,         // IntegrityConstraintDecl
        S_QueryBody = 395,                       // QueryBody
        S_StatementsAndOptionalExprTop = 396,    // StatementsAndOptionalExprTop
        S_StatementsAndOptionalExpr = 397,       // StatementsAndOptionalExpr
        S_StatementsAndExpr = 398,               // StatementsAndExpr
        S_Statements = 399,                      // Statements
        S_Statement = 400,                       // Statement
        S_BlockStatement = 401,                  // BlockStatement
        S_BlockExpr = 402,                       // BlockExpr
        S_EnclosedStatementsAndOptionalExpr = 403, // EnclosedStatementsAndOptionalExpr
        S_VarDeclStatement = 404,                // VarDeclStatement
        S_BlockVarDeclList = 405,                // BlockVarDeclList
        S_BlockVarDecl = 406,                    // BlockVarDecl
        S_AssignStatement = 407,                 // AssignStatement
        S_ApplyStatement = 408,                  // ApplyStatement
        S_ExitStatement = 409,                   // ExitStatement
        S_WhileStatement = 410,                  // WhileStatement
        S_FlowCtlStatement = 411,                // FlowCtlStatement
        S_FLWORStatement = 412,                  // FLWORStatement
        S_ReturnStatement = 413,                 // ReturnStatement
        S_IfStatement = 414,                     // IfStatement
        S_TryStatement = 415,                    // TryStatement
        S_CatchListStatement = 416,              // CatchListStatement
        S_CatchStatement = 417,                  // CatchStatement
        S_VoidStatement = 418,                   // VoidStatement
        S_Expr = 419,                            // Expr
        S_ExprSingle = 420,                      // ExprSingle
        S_ExprSimple = 421,                      // ExprSimple
        S_FLWORExpr = 422,                       // FLWORExpr
        S_ReturnExpr = 423,                      // ReturnExpr
        S_ReturnOrSelect = 424,                  // ReturnOrSelect
        S_WindowType = 425,                      // WindowType
        S_FLWORWinCondType = 426,                // FLWORWinCondType
        S_FLWORWinCond = 427,                    // FLWORWinCond
        S_WindowClause = 428,                    // WindowClause
        S_CountClause = 429,                     // CountClause
        S_ForLetWinClause = 430,                 // ForLetWinClause
        S_FLWORClause = 431,                     // FLWORClause
        S_FLWORClauseList = 432,                 // FLWORClauseList
        S_ForClause = 433,                       // ForClause
        S_ForOrFrom = 434,                       // ForOrFrom
        S_VarInDeclList = 435,                   // VarInDeclList
        S_VarInDecl = 436,                       // VarInDecl
        S_PositionalVar = 437,                   // PositionalVar
        S_FTScoreVar = 438,                      // FTScoreVar
        S_LetClause = 439,                       // LetClause
        S_VarGetsDeclList = 440,                 // VarGetsDeclList
        S_VarGetsDecl = 441,                     // VarGetsDecl
        S_WindowVarDecl = 442,                   // WindowVarDecl
        S_WindowVars = 443,                      // WindowVars
        S_WindowVars3 = 444,                     // WindowVars3
        S_WindowVars2 = 445,                     // WindowVars2
        S_WhereClause = 446,                     // WhereClause
        S_GroupByClause = 447,                   // GroupByClause
        S_GroupSpecList = 448,                   // GroupSpecList
        S_GroupSpec = 449,                       // GroupSpec
        S_GroupCollationSpec = 450,              // GroupCollationSpec
        S_OrderByClause = 451,                   // OrderByClause
        S_OrderSpecList = 452,                   // OrderSpecList
        S_OrderSpec = 453,                       // OrderSpec
        S_OrderModifier = 454,                   // OrderModifier
        S_OrderDirSpec = 455,                    // OrderDirSpec
        S_OrderEmptySpec = 456,                  // OrderEmptySpec
        S_OrderCollationSpec = 457,              // OrderCollationSpec
        S_QuantifiedExpr = 458,                  // QuantifiedExpr
        S_QVarInDeclList = 459,                  // QVarInDeclList
        S_QVarInDecl = 460,                      // QVarInDecl
        S_SwitchExpr = 461,                      // SwitchExpr
        S_SwitchCaseClauseList = 462,            // SwitchCaseClauseList
        S_SwitchCaseClause = 463,                // SwitchCaseClause
        S_SwitchCaseOperandList = 464,           // SwitchCaseOperandList
        S_SwitchStatement = 465,                 // SwitchStatement
        S_SwitchCaseStatementList = 466,         // SwitchCaseStatementList
        S_SwitchCaseStatement = 467,             // SwitchCaseStatement
        S_TypeswitchExpr = 468,                  // TypeswitchExpr
        S_TypeswitchStatement = 469,             // TypeswitchStatement
        S_CaseClauseList = 470,                  // CaseClauseList
        S_CaseClause = 471,                      // CaseClause
        S_CaseStatementList = 472,               // CaseStatementList
        S_CaseStatement = 473,                   // CaseStatement
        S_SequenceTypeList = 474,                // SequenceTypeList
        S_IfExpr = 475,                          // IfExpr
        S_OrExpr = 476,                          // OrExpr
        S_AndExpr = 477,                         // AndExpr
        S_NotExpr = 478,                         // NotExpr
        S_ComparisonExpr = 479,                  // ComparisonExpr
        S_480_1 = 480,                           // $@1
        S_FTContainsExpr = 481,                  // FTContainsExpr
        S_StringConcatExpr = 482,                // StringConcatExpr
        S_opt_FTIgnoreOption = 483,              // opt_FTIgnoreOption
        S_RangeExpr = 484,                       // RangeExpr
        S_AdditiveExpr = 485,                    // AdditiveExpr
        S_MultiplicativeExpr = 486,              // MultiplicativeExpr
        S_UnionExpr = 487,                       // UnionExpr
        S_IntersectExceptExpr = 488,             // IntersectExceptExpr
        S_InstanceofExpr = 489,                  // InstanceofExpr
        S_TreatExpr = 490,                       // TreatExpr
        S_CastableExpr = 491,                    // CastableExpr
        S_CastExpr = 492,                        // CastExpr
        S_SingleType = 493,                      // SingleType
        S_UnaryExpr = 494,                       // UnaryExpr
        S_SignList = 495,                        // SignList
        S_ValueExpr = 496,                       // ValueExpr
        S_SimpleMapExpr = 497,                   // SimpleMapExpr
        S_ValueComp = 498,                       // ValueComp
        S_NodeComp = 499,                        // NodeComp
        S_ValidateExpr = 500,                    // ValidateExpr
        S_ExtensionExpr = 501,                   // ExtensionExpr
        S_Pragma_list = 502,                     // Pragma_list
        S_Pragma = 503,                          // Pragma
        S_PathExpr = 504,                        // PathExpr
        S_LeadingSlash = 505,                    // LeadingSlash
        S_RelativePathExpr = 506,                // RelativePathExpr
        S_StepExpr = 507,                        // StepExpr
        S_AxisStep = 508,                        // AxisStep
        S_ForwardStep = 509,                     // ForwardStep
        S_ForwardAxis = 510,                     // ForwardAxis
        S_AbbrevForwardStep = 511,               // AbbrevForwardStep
        S_ReverseStep = 512,                     // ReverseStep
        S_ReverseAxis = 513,                     // ReverseAxis
        S_NodeTest = 514,                        // NodeTest
        S_NameTest = 515,                        // NameTest
        S_Wildcard = 516,                        // Wildcard
        S_PostfixExpr = 517,                     // PostfixExpr
        S_PredicateList = 518,                   // PredicateList
        S_Predicate = 519,                       // Predicate
        S_PrimaryExpr = 520,                     // PrimaryExpr
        S_Literal = 521,                         // Literal
        S_NumericLiteral = 522,                  // NumericLiteral
        S_VarRef = 523,                          // VarRef
        S_ParenthesizedExpr = 524,               // ParenthesizedExpr
        S_ContextItemExpr = 525,                 // ContextItemExpr
        S_OrderedExpr = 526,                     // OrderedExpr
        S_UnorderedExpr = 527,                   // UnorderedExpr
        S_FunctionCall = 528,                    // FunctionCall
        S_ArgList = 529,                         // ArgList
        S_FunctionItemExpr = 530,                // FunctionItemExpr
        S_LiteralFunctionItem = 531,             // LiteralFunctionItem
        S_InlineFunction = 532,                  // InlineFunction
        S_Constructor = 533,                     // Constructor
        S_DirectConstructor = 534,               // DirectConstructor
        S_DirElemConstructor = 535,              // DirElemConstructor
        S_DirElemContentList = 536,              // DirElemContentList
        S_DirAttributeList = 537,                // DirAttributeList
        S_DirAttr = 538,                         // DirAttr
        S_OptionalBlank = 539,                   // OptionalBlank
        S_DirAttributeValue = 540,               // DirAttributeValue
        S_opt_QuoteAttrContentList = 541,        // opt_QuoteAttrContentList
        S_QuoteAttrContentList = 542,            // QuoteAttrContentList
        S_opt_AposAttrContentList = 543,         // opt_AposAttrContentList
        S_AposAttrContentList = 544,             // AposAttrContentList
        S_QuoteAttrValueContent = 545,           // QuoteAttrValueContent
        S_AposAttrValueContent = 546,            // AposAttrValueContent
        S_DirElemContent = 547,                  // DirElemContent
        S_CommonContent = 548,                   // CommonContent
        S_DirCommentConstructor = 549,           // DirCommentConstructor
        S_DirPIConstructor = 550,                // DirPIConstructor
        S_CDataSection = 551,                    // CDataSection
        S_ComputedConstructor = 552,             // ComputedConstructor
        S_CompDocConstructor = 553,              // CompDocConstructor
        S_CompElemConstructor = 554,             // CompElemConstructor
        S_CompAttrConstructor = 555,             // CompAttrConstructor
        S_CompTextConstructor = 556,             // CompTextConstructor
        S_CompCommentConstructor = 557,          // CompCommentConstructor
        S_CompPIConstructor = 558,               // CompPIConstructor
        S_CompNamespaceConstructor = 559,        // CompNamespaceConstructor
        S_TypeDeclaration = 560,                 // TypeDeclaration
        S_SequenceType = 561,                    // SequenceType
        S_OccurrenceIndicator = 562,             // OccurrenceIndicator
        S_ItemType = 563,                        // ItemType
        S_TypeList = 564,                        // TypeList
        S_GeneralizedAtomicType = 565,           // GeneralizedAtomicType
        S_SimpleType = 566,                      // SimpleType
        S_KindTest = 567,                        // KindTest
        S_AnyKindTest = 568,                     // AnyKindTest
        S_NODE_KIND_TEST = 569,                  // NODE_KIND_TEST
        S_DocumentTest = 570,                    // DocumentTest
        S_NamespaceTest = 571,                   // NamespaceTest
        S_TextTest = 572,                        // TextTest
        S_CommentTest = 573,                     // CommentTest
        S_PITest = 574,                          // PITest
        S_AttributeTest = 575,                   // AttributeTest
        S_SchemaAttributeTest = 576,             // SchemaAttributeTest
        S_ElementTest = 577,                     // ElementTest
        S_SchemaElementTest = 578,               // SchemaElementTest
        S_TypeName = 579,                        // TypeName
        S_TypeName_WITH_HOOK = 580,              // TypeName_WITH_HOOK
        S_StringLiteral = 581,                   // StringLiteral
        S_FunctionTest = 582,                    // FunctionTest
        S_AnyFunctionTest = 583,                 // AnyFunctionTest
        S_TypedFunctionTest = 584,               // TypedFunctionTest
        S_ParenthesizedItemType = 585,           // ParenthesizedItemType
        S_RevalidationDecl = 586,                // RevalidationDecl
        S_InsertExpr = 587,                      // InsertExpr
        S_DeleteExpr = 588,                      // DeleteExpr
        S_ReplaceExpr = 589,                     // ReplaceExpr
        S_RenameExpr = 590,                      // RenameExpr
        S_TransformExpr = 591,                   // TransformExpr
        S_VarNameList = 592,                     // VarNameList
        S_VarNameDecl = 593,                     // VarNameDecl
        S_TryExpr = 594,                         // TryExpr
        S_CatchListExpr = 595,                   // CatchListExpr
        S_CatchExpr = 596,                       // CatchExpr
        S_BracedExpr = 597,                      // BracedExpr
        S_NameTestList = 598,                    // NameTestList
        S_FTSelection = 599,                     // FTSelection
        S_opt_FTPosFilter_list = 600,            // opt_FTPosFilter_list
        S_FTPosFilter_list = 601,                // FTPosFilter_list
        S_FTOr = 602,                            // FTOr
        S_FTAnd = 603,                           // FTAnd
        S_FTMildNot = 604,                       // FTMildNot
        S_FTUnaryNot = 605,                      // FTUnaryNot
        S_FTPrimaryWithOptions = 606,            // FTPrimaryWithOptions
        S_opt_FTMatchOptions = 607,              // opt_FTMatchOptions
        S_opt_FTWeight = 608,                    // opt_FTWeight
        S_FTWeight = 609,                        // FTWeight
        S_FTPrimary = 610,                       // FTPrimary
        S_opt_FTTimes = 611,                     // opt_FTTimes
        S_FTExtensionSelection = 612,            // FTExtensionSelection
        S_opt_FTSelection = 613,                 // opt_FTSelection
        S_FTWords = 614,                         // FTWords
        S_FTWordsValue = 615,                    // FTWordsValue
        S_opt_FTAnyallOption = 616,              // opt_FTAnyallOption
        S_FTAnyallOption = 617,                  // FTAnyallOption
        S_opt_word = 618,                        // opt_word
        S_opt_words = 619,                       // opt_words
        S_FTPosFilter = 620,                     // FTPosFilter
        S_FTOrder = 621,                         // FTOrder
        S_FTWindow = 622,                        // FTWindow
        S_FTDistance = 623,                      // FTDistance
        S_FTUnit = 624,                          // FTUnit
        S_FTMatchOptions = 625,                  // FTMatchOptions
        S_FTMatchOption = 626,                   // FTMatchOption
        S_FTCaseOption = 627,                    // FTCaseOption
        S_FTDiacriticsOption = 628,              // FTDiacriticsOption
        S_FTExtensionOption = 629,               // FTExtensionOption
        S_FTStemOption = 630,                    // FTStemOption
        S_FTThesaurusOption = 631,               // FTThesaurusOption
        S_FTThesaurusID_or_default = 632,        // FTThesaurusID_or_default
        S_opt_FTThesaurus_list = 633,            // opt_FTThesaurus_list
        S_FTThesaurus_list = 634,                // FTThesaurus_list
        S_FTThesaurusID = 635,                   // FTThesaurusID
        S_opt_relationship = 636,                // opt_relationship
        S_opt_levels = 637,                      // opt_levels
        S_FTStopWordOption = 638,                // FTStopWordOption
        S_FTStopWords = 639,                     // FTStopWords
        S_STRING_LITERAL_list = 640,             // STRING_LITERAL_list
        S_opt_FTStopWordsInclExcl_list = 641,    // opt_FTStopWordsInclExcl_list
        S_FTStopWordsInclExcl_list = 642,        // FTStopWordsInclExcl_list
        S_FTStopWordsInclExcl = 643,             // FTStopWordsInclExcl
        S_FTLanguageOption = 644,                // FTLanguageOption
        S_FTWildCardOption = 645,                // FTWildCardOption
        S_FTContent = 646,                       // FTContent
        S_FTTimes = 647,                         // FTTimes
        S_FTRange = 648,                         // FTRange
        S_FTScope = 649,                         // FTScope
        S_FTBigUnit = 650,                       // FTBigUnit
        S_FTIgnoreOption = 651,                  // FTIgnoreOption
        S_JSONArrayConstructor = 652,            // JSONArrayConstructor
        S_JSONSimpleObjectUnion = 653,           // JSONSimpleObjectUnion
        S_JSONAccumulatorObjectUnion = 654,      // JSONAccumulatorObjectUnion
        S_JSONObjectConstructor = 655,           // JSONObjectConstructor
        S_JSONPairList = 656,                    // JSONPairList
        S_JSONInsertExpr = 657,                  // JSONInsertExpr
        S_JSONAppendExpr = 658,                  // JSONAppendExpr
        S_JSONDeleteExpr = 659,                  // JSONDeleteExpr
        S_JSONRenameExpr = 660,                  // JSONRenameExpr
        S_JSONReplaceExpr = 661,                 // JSONReplaceExpr
        S_JSONTest = 662,                        // JSONTest
        S_URI_LITERAL = 663,                     // URI_LITERAL
        S_QNAME = 664,                           // QNAME
        S_FUNCTION_NAME = 665,                   // FUNCTION_NAME
        S_EQNAME = 666                           // EQNAME
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
        return jsoniq_parser::symbol_name (this->kind ());
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
    jsoniq_parser (xquery_driver& driver_yyarg);
    virtual ~jsoniq_parser ();

#if 201103L <= YY_CPLUSPLUS
    /// Non copyable.
    jsoniq_parser (const jsoniq_parser&) = delete;
    /// Non copyable.
    jsoniq_parser& operator= (const jsoniq_parser&) = delete;
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
      context (const jsoniq_parser& yyparser, const symbol_type& yyla);
      const symbol_type& lookahead () const YY_NOEXCEPT { return yyla_; }
      symbol_kind_type token () const YY_NOEXCEPT { return yyla_.kind (); }
      const location_type& location () const YY_NOEXCEPT { return yyla_.location; }

      /// Put in YYARG at most YYARGN of the expected tokens, and return the
      /// number of tokens stored in YYARG.  If YYARG is null, return the
      /// number of expected tokens (guaranteed to be less than YYNTOKENS).
      int expected_tokens (symbol_kind_type yyarg[], int yyargn) const;

    private:
      const jsoniq_parser& yyparser_;
      const symbol_type& yyla_;
    };

  private:
#if YY_CPLUSPLUS < 201103L
    /// Non copyable.
    jsoniq_parser (const jsoniq_parser&);
    /// Non copyable.
    jsoniq_parser& operator= (const jsoniq_parser&);
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
      yylast_ = 19531,     ///< Last index in yytable_.
      yynnts_ = 327,  ///< Number of nonterminal symbols.
      yyfinal_ = 623 ///< Termination state number.
    };


    // User arguments.
    xquery_driver& driver;

  };


} // zorba




#endif // !YY_YY_JSONIQ_PARSER_HPP_INCLUDED
