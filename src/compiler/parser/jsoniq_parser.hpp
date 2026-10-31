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
    DOLLAR_DOLLAR = 377,           // "'$_'"
    DOT = 378,                     // "'.'"
    DOT_DOT = 379,                 // "'..'"
    COLON = 380,                   // "':'"
    DOUBLE_COLON = 381,            // "'::'"
    DOUBLE_LBRACE = 382,           // "'{{'"
    DOUBLE_LITERAL = 383,          // "'DOUBLE'"
    DOUBLE_RBRACE = 384,           // "'<double {>'"
    ELSE = 385,                    // "'else'"
    _EMPTY = 386,                  // "'empty'"
    GREATEST = 387,                // "'greatest'"
    LEAST = 388,                   // "'least'"
    EMPTY_TAG_END = 389,           // "'/>'"
    ENCODING = 390,                // "'encoding'"
    EQUALS = 391,                  // "'='"
    ESCAPE_APOS = 392,             // "''''"
    ESCAPE_QUOTE = 393,            // "'\"\"'"
    EVERY = 394,                   // "'every'"
    EXCEPT = 395,                  // "'except'"
    EXTERNAL = 396,                // "'external'"
    FOLLOWING = 397,               // "'following'"
    FOLLOWING_SIBLING = 398,       // "'following-sibling'"
    FOLLOWS = 399,                 // "'follows'"
    GE = 400,                      // "'>='"
    GETS = 401,                    // "':='"
    GT = 402,                      // "'>'"
    HOOK = 403,                    // "'?'"
    HASH = 404,                    // "'#'"
    IDIV = 405,                    // "'idiv'"
    _IN = 406,                     // "'in'"
    INHERIT = 407,                 // "'inherit'"
    INTEGER_LITERAL = 408,         // "'INTEGER'"
    INTERSECT = 409,               // "'intersect'"
    IS = 410,                      // "'is'"
    ITEM = 411,                    // "'item'"
    LBRACE = 412,                  // "'{'"
    LBRACK = 413,                  // "'['"
    LE = 414,                      // "'<='"
    LPAR = 415,                    // "'('"
    LT_OR_START_TAG = 416,         // "'<'"
    MINUS = 417,                   // "'-'"
    MOD = 418,                     // "'mod'"
    NAMESPACE = 419,               // "'namespace'"
    _NAN = 420,                    // "'nan'"
    NE = 421,                      // "'!='"
    NODECOMP = 422,                // "'nodecomp'"
    NO_INHERIT = 423,              // "'<no inherit>'"
    NO_PRESERVE = 424,             // "'<no preserve>'"
    OR = 425,                      // "'or'"
    ORDER = 426,                   // "'order'"
    ORDERED = 427,                 // "'ordered'"
    BY = 428,                      // "'by'"
    GROUP = 429,                   // "'group'"
    PARENT = 430,                  // "'parent'"
    PERCENTAGE = 431,              // "'%'"
    PI_BEGIN = 432,                // "'<?'"
    PI_END = 433,                  // "'?>'"
    PLUS = 434,                    // "'+'"
    PRAGMA_BEGIN = 435,            // "'(#'"
    PRAGMA_END = 436,              // "'#)'"
    PRECEDES = 437,                // "'<<'"
    PRECEDING = 438,               // "'preceding'"
    PRECEDING_SIBLING = 439,       // "'preceding-sibling'"
    PRESERVE = 440,                // "'preserve'"
    QUOTE = 441,                   // "'\"'"
    RBRACE = 442,                  // "'}'"
    RBRACK = 443,                  // "']'"
    RETURN = 444,                  // "'return'"
    RPAR = 445,                    // "')'"
    SATISFIES = 446,               // "'satisfies'"
    SELECT = 447,                  // "'select'"
    SELF = 448,                    // "'self'"
    SEMI = 449,                    // "';'"
    SLASH = 450,                   // "'/'"
    SLASH_SLASH = 451,             // "'//'"
    BANG = 452,                    // "'!'"
    STAR = 453,                    // "'*'"
    START_TAG_END = 454,           // "'</ (start tag end)'"
    STRIP = 455,                   // "'strip'"
    TAG_END = 456,                 // "'> (tag end)'"
    THEN = 457,                    // "'then'"
    TO = 458,                      // "'to'"
    TREAT = 459,                   // "'treat'"
    UNION = 460,                   // "'union'"
    UNORDERED = 461,               // "'unordered'"
    VAL_EQ = 462,                  // "'eq'"
    VAL_GE = 463,                  // "'ge'"
    VAL_GT = 464,                  // "'gt'"
    VAL_LE = 465,                  // "'le'"
    VAL_LT = 466,                  // "'lt'"
    VAL_NE = 467,                  // "'ne'"
    VALUECOMP = 468,               // "'VALUECOMP'"
    VBAR = 469,                    // "'|'"
    EMPTY_SEQUENCE = 470,          // "'empty-sequence'"
    WHERE = 471,                   // "'where'"
    XML_COMMENT_BEGIN = 472,       // "'<!--'"
    XML_COMMENT_END = 473,         // "'-->'"
    XQUERY = 474,                  // "'xquery'"
    JSONIQ = 475,                  // "'jsoniq'"
    VERSION = 476,                 // "'version'"
    START = 477,                   // "'start'"
    NULL_TOKEN = 478,              // "'null'"
    TRUE_TOKEN = 479,              // "'true'"
    FALSE_TOKEN = 480,             // "'false'"
    AFTER = 481,                   // "'after'"
    BEFORE = 482,                  // "'before'"
    REVALIDATION = 483,            // "'revalidation'"
    _STRICT = 484,                 // "'strict'"
    LAX = 485,                     // "'lax'"
    SKIP = 486,                    // "'skip'"
    _DELETE = 487,                 // "'delete'"
    NODE = 488,                    // "'node'"
    INSERT = 489,                  // "'insert'"
    NODES = 490,                   // "'nodes'"
    RENAME = 491,                  // "'rename'"
    REPLACE = 492,                 // "'replace'"
    VALUE = 493,                   // "'value'"
    OF = 494,                      // "'of'"
    FIRST = 495,                   // "'first'"
    INTO = 496,                    // "'into'"
    LAST = 497,                    // "'last'"
    MODIFY = 498,                  // "'modify'"
    COPY = 499,                    // "'copy'"
    WITH = 500,                    // "'with'"
    BREAK = 501,                   // "'break'"
    CONTINUE = 502,                // "'continue'"
    EXIT = 503,                    // "'exit'"
    LOOP = 504,                    // "'loop'"
    RETURNING = 505,               // "'returning'"
    WHILE = 506,                   // "'while'"
    TRY = 507,                     // "'try'"
    CATCH = 508,                   // "'catch'"
    USING = 509,                   // "'using'"
    ALL = 510,                     // "'all'"
    ANY = 511,                     // "'any'"
    CONTAINS = 512,                // "'contains'"
    CONTENT = 513,                 // "'content'"
    DIACRITICS = 514,              // "'diacritics'"
    DIFFERENT = 515,               // "'different'"
    DISTANCE = 516,                // "'distance'"
    END = 517,                     // "'end'"
    ENTIRE = 518,                  // "'entire'"
    EXACTLY = 519,                 // "'exactly'"
    FROM = 520,                    // "'from'"
    FTAND = 521,                   // "'ftand'"
    FTNOT = 522,                   // "'ftnot'"
    NOT = 523,                     // "'not'"
    FT_OPTION = 524,               // "'ft-option'"
    FTOR = 525,                    // "'ftor'"
    INSENSITIVE = 526,             // "'insensitive'"
    LANGUAGE = 527,                // "'language'"
    LEVELS = 528,                  // "'levels'"
    LOWERCASE = 529,               // "'lowercase'"
    OCCURS = 530,                  // "'occurs'"
    PARAGRAPH = 531,               // "'paragraph'"
    PARAGRAPHS = 532,              // "'paragraphs'"
    PHRASE = 533,                  // "'phrase'"
    RELATIONSHIP = 534,            // "'relationship'"
    SAME = 535,                    // "'same'"
    SCORE = 536,                   // "'score'"
    SENSITIVE = 537,               // "'sensitive'"
    SENTENCE = 538,                // "'sentence'"
    SENTENCES = 539,               // "'sentences'"
    STEMMING = 540,                // "'stemming'"
    STOP = 541,                    // "'stop'"
    THESAURUS = 542,               // "'thesaurus'"
    TIMES = 543,                   // "'times'"
    UPPERCASE = 544,               // "'uppercase'"
    WEIGHT = 545,                  // "'weight'"
    WILDCARDS = 546,               // "'wildcards'"
    WINDOW = 547,                  // "'window'"
    WITHOUT = 548,                 // "'without'"
    WORDS = 549,                   // "'words'"
    COLLECTION = 550,              // "'collection'"
    CONSTOPT = 551,                // "'const'"
    APPEND_ONLY = 552,             // "'append-only'"
    QUEUE = 553,                   // "'queue'"
    MUTABLE = 554,                 // "'mutable'"
    READ_ONLY = 555,               // "'read-only'"
    UNIQUE = 556,                  // "'unique'"
    NON = 557,                     // "'non'"
    INDEX = 558,                   // "'index'"
    MANUALLY = 559,                // "'manually'"
    AUTOMATICALLY = 560,           // "'automatically'"
    MAINTAINED = 561,              // "'maintained'"
    ON = 562,                      // "'on'"
    RANGE = 563,                   // "'range'"
    EQUALITY = 564,                // "'equality'"
    GENERAL = 565,                 // "'general'"
    INTEGRITY = 566,               // "'integrity'"
    CONSTRAINT = 567,              // "'constraint'"
    CHECK = 568,                   // "'check'"
    KEY = 569,                     // "'key'"
    FOREACH = 570,                 // "'foreach'"
    FOREIGN = 571,                 // "'foreign'"
    KEYS = 572,                    // "'keys'"
    L_SIMPLE_OBJ_UNION = 573,      // "'{|'"
    R_SIMPLE_OBJ_UNION = 574,      // "'|}'"
    L_ACCUMULATOR_OBJ_UNION = 575, // "'{['"
    R_ACCUMULATOR_OBJ_UNION = 576, // "']}'"
    JSON = 577,                    // "'json'"
    APPEND = 578,                  // "'append'"
    POSITION = 579,                // "'position'"
    OBJECT = 580,                  // "'object'"
    ARRAY = 581,                   // "'array'"
    JSON_ITEM = 582,               // "'json-item'"
    STRUCTURED_ITEM = 583,         // "'structured-item'"
    BYTE_ORDER_MARK_UTF8 = 584,    // "'BOM_UTF8'"
    SHEBANG = 585,                 // "'#!/shebang"
    RANGE_REDUCE = 586,            // RANGE_REDUCE
    SEQUENCE_TYPE_REDUCE = 587,    // SEQUENCE_TYPE_REDUCE
    ADDITIVE_REDUCE = 588,         // ADDITIVE_REDUCE
    UNION_REDUCE = 589,            // UNION_REDUCE
    INTERSECT_EXCEPT_REDUCE = 590, // INTERSECT_EXCEPT_REDUCE
    QVARINDECLLIST_REDUCE = 591,   // QVARINDECLLIST_REDUCE
    UNARY_PREC = 592,              // UNARY_PREC
    STEP_REDUCE = 593,             // STEP_REDUCE
    SIMPLEMAPEXPR_REDUCE = 594,    // SIMPLEMAPEXPR_REDUCE
    MULTIPLICATIVE_REDUCE = 595,   // MULTIPLICATIVE_REDUCE
    JSONLOOKUPEXPR_REDUCE = 596    // JSONLOOKUPEXPR_REDUCE
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
        YYNTOKENS = 342, ///< Number of tokens.
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
        S_DOLLAR_DOLLAR = 122,                   // "'$_'"
        S_DOT = 123,                             // "'.'"
        S_DOT_DOT = 124,                         // "'..'"
        S_COLON = 125,                           // "':'"
        S_DOUBLE_COLON = 126,                    // "'::'"
        S_DOUBLE_LBRACE = 127,                   // "'{{'"
        S_DOUBLE_LITERAL = 128,                  // "'DOUBLE'"
        S_DOUBLE_RBRACE = 129,                   // "'<double {>'"
        S_ELSE = 130,                            // "'else'"
        S__EMPTY = 131,                          // "'empty'"
        S_GREATEST = 132,                        // "'greatest'"
        S_LEAST = 133,                           // "'least'"
        S_EMPTY_TAG_END = 134,                   // "'/>'"
        S_ENCODING = 135,                        // "'encoding'"
        S_EQUALS = 136,                          // "'='"
        S_ESCAPE_APOS = 137,                     // "''''"
        S_ESCAPE_QUOTE = 138,                    // "'\"\"'"
        S_EVERY = 139,                           // "'every'"
        S_EXCEPT = 140,                          // "'except'"
        S_EXTERNAL = 141,                        // "'external'"
        S_FOLLOWING = 142,                       // "'following'"
        S_FOLLOWING_SIBLING = 143,               // "'following-sibling'"
        S_FOLLOWS = 144,                         // "'follows'"
        S_GE = 145,                              // "'>='"
        S_GETS = 146,                            // "':='"
        S_GT = 147,                              // "'>'"
        S_HOOK = 148,                            // "'?'"
        S_HASH = 149,                            // "'#'"
        S_IDIV = 150,                            // "'idiv'"
        S__IN = 151,                             // "'in'"
        S_INHERIT = 152,                         // "'inherit'"
        S_INTEGER_LITERAL = 153,                 // "'INTEGER'"
        S_INTERSECT = 154,                       // "'intersect'"
        S_IS = 155,                              // "'is'"
        S_ITEM = 156,                            // "'item'"
        S_LBRACE = 157,                          // "'{'"
        S_LBRACK = 158,                          // "'['"
        S_LE = 159,                              // "'<='"
        S_LPAR = 160,                            // "'('"
        S_LT_OR_START_TAG = 161,                 // "'<'"
        S_MINUS = 162,                           // "'-'"
        S_MOD = 163,                             // "'mod'"
        S_NAMESPACE = 164,                       // "'namespace'"
        S__NAN = 165,                            // "'nan'"
        S_NE = 166,                              // "'!='"
        S_NODECOMP = 167,                        // "'nodecomp'"
        S_NO_INHERIT = 168,                      // "'<no inherit>'"
        S_NO_PRESERVE = 169,                     // "'<no preserve>'"
        S_OR = 170,                              // "'or'"
        S_ORDER = 171,                           // "'order'"
        S_ORDERED = 172,                         // "'ordered'"
        S_BY = 173,                              // "'by'"
        S_GROUP = 174,                           // "'group'"
        S_PARENT = 175,                          // "'parent'"
        S_PERCENTAGE = 176,                      // "'%'"
        S_PI_BEGIN = 177,                        // "'<?'"
        S_PI_END = 178,                          // "'?>'"
        S_PLUS = 179,                            // "'+'"
        S_PRAGMA_BEGIN = 180,                    // "'(#'"
        S_PRAGMA_END = 181,                      // "'#)'"
        S_PRECEDES = 182,                        // "'<<'"
        S_PRECEDING = 183,                       // "'preceding'"
        S_PRECEDING_SIBLING = 184,               // "'preceding-sibling'"
        S_PRESERVE = 185,                        // "'preserve'"
        S_QUOTE = 186,                           // "'\"'"
        S_RBRACE = 187,                          // "'}'"
        S_RBRACK = 188,                          // "']'"
        S_RETURN = 189,                          // "'return'"
        S_RPAR = 190,                            // "')'"
        S_SATISFIES = 191,                       // "'satisfies'"
        S_SELECT = 192,                          // "'select'"
        S_SELF = 193,                            // "'self'"
        S_SEMI = 194,                            // "';'"
        S_SLASH = 195,                           // "'/'"
        S_SLASH_SLASH = 196,                     // "'//'"
        S_BANG = 197,                            // "'!'"
        S_STAR = 198,                            // "'*'"
        S_START_TAG_END = 199,                   // "'</ (start tag end)'"
        S_STRIP = 200,                           // "'strip'"
        S_TAG_END = 201,                         // "'> (tag end)'"
        S_THEN = 202,                            // "'then'"
        S_TO = 203,                              // "'to'"
        S_TREAT = 204,                           // "'treat'"
        S_UNION = 205,                           // "'union'"
        S_UNORDERED = 206,                       // "'unordered'"
        S_VAL_EQ = 207,                          // "'eq'"
        S_VAL_GE = 208,                          // "'ge'"
        S_VAL_GT = 209,                          // "'gt'"
        S_VAL_LE = 210,                          // "'le'"
        S_VAL_LT = 211,                          // "'lt'"
        S_VAL_NE = 212,                          // "'ne'"
        S_VALUECOMP = 213,                       // "'VALUECOMP'"
        S_VBAR = 214,                            // "'|'"
        S_EMPTY_SEQUENCE = 215,                  // "'empty-sequence'"
        S_WHERE = 216,                           // "'where'"
        S_XML_COMMENT_BEGIN = 217,               // "'<!--'"
        S_XML_COMMENT_END = 218,                 // "'-->'"
        S_XQUERY = 219,                          // "'xquery'"
        S_JSONIQ = 220,                          // "'jsoniq'"
        S_VERSION = 221,                         // "'version'"
        S_START = 222,                           // "'start'"
        S_NULL_TOKEN = 223,                      // "'null'"
        S_TRUE_TOKEN = 224,                      // "'true'"
        S_FALSE_TOKEN = 225,                     // "'false'"
        S_AFTER = 226,                           // "'after'"
        S_BEFORE = 227,                          // "'before'"
        S_REVALIDATION = 228,                    // "'revalidation'"
        S__STRICT = 229,                         // "'strict'"
        S_LAX = 230,                             // "'lax'"
        S_SKIP = 231,                            // "'skip'"
        S__DELETE = 232,                         // "'delete'"
        S_NODE = 233,                            // "'node'"
        S_INSERT = 234,                          // "'insert'"
        S_NODES = 235,                           // "'nodes'"
        S_RENAME = 236,                          // "'rename'"
        S_REPLACE = 237,                         // "'replace'"
        S_VALUE = 238,                           // "'value'"
        S_OF = 239,                              // "'of'"
        S_FIRST = 240,                           // "'first'"
        S_INTO = 241,                            // "'into'"
        S_LAST = 242,                            // "'last'"
        S_MODIFY = 243,                          // "'modify'"
        S_COPY = 244,                            // "'copy'"
        S_WITH = 245,                            // "'with'"
        S_BREAK = 246,                           // "'break'"
        S_CONTINUE = 247,                        // "'continue'"
        S_EXIT = 248,                            // "'exit'"
        S_LOOP = 249,                            // "'loop'"
        S_RETURNING = 250,                       // "'returning'"
        S_WHILE = 251,                           // "'while'"
        S_TRY = 252,                             // "'try'"
        S_CATCH = 253,                           // "'catch'"
        S_USING = 254,                           // "'using'"
        S_ALL = 255,                             // "'all'"
        S_ANY = 256,                             // "'any'"
        S_CONTAINS = 257,                        // "'contains'"
        S_CONTENT = 258,                         // "'content'"
        S_DIACRITICS = 259,                      // "'diacritics'"
        S_DIFFERENT = 260,                       // "'different'"
        S_DISTANCE = 261,                        // "'distance'"
        S_END = 262,                             // "'end'"
        S_ENTIRE = 263,                          // "'entire'"
        S_EXACTLY = 264,                         // "'exactly'"
        S_FROM = 265,                            // "'from'"
        S_FTAND = 266,                           // "'ftand'"
        S_FTNOT = 267,                           // "'ftnot'"
        S_NOT = 268,                             // "'not'"
        S_FT_OPTION = 269,                       // "'ft-option'"
        S_FTOR = 270,                            // "'ftor'"
        S_INSENSITIVE = 271,                     // "'insensitive'"
        S_LANGUAGE = 272,                        // "'language'"
        S_LEVELS = 273,                          // "'levels'"
        S_LOWERCASE = 274,                       // "'lowercase'"
        S_OCCURS = 275,                          // "'occurs'"
        S_PARAGRAPH = 276,                       // "'paragraph'"
        S_PARAGRAPHS = 277,                      // "'paragraphs'"
        S_PHRASE = 278,                          // "'phrase'"
        S_RELATIONSHIP = 279,                    // "'relationship'"
        S_SAME = 280,                            // "'same'"
        S_SCORE = 281,                           // "'score'"
        S_SENSITIVE = 282,                       // "'sensitive'"
        S_SENTENCE = 283,                        // "'sentence'"
        S_SENTENCES = 284,                       // "'sentences'"
        S_STEMMING = 285,                        // "'stemming'"
        S_STOP = 286,                            // "'stop'"
        S_THESAURUS = 287,                       // "'thesaurus'"
        S_TIMES = 288,                           // "'times'"
        S_UPPERCASE = 289,                       // "'uppercase'"
        S_WEIGHT = 290,                          // "'weight'"
        S_WILDCARDS = 291,                       // "'wildcards'"
        S_WINDOW = 292,                          // "'window'"
        S_WITHOUT = 293,                         // "'without'"
        S_WORDS = 294,                           // "'words'"
        S_COLLECTION = 295,                      // "'collection'"
        S_CONSTOPT = 296,                        // "'const'"
        S_APPEND_ONLY = 297,                     // "'append-only'"
        S_QUEUE = 298,                           // "'queue'"
        S_MUTABLE = 299,                         // "'mutable'"
        S_READ_ONLY = 300,                       // "'read-only'"
        S_UNIQUE = 301,                          // "'unique'"
        S_NON = 302,                             // "'non'"
        S_INDEX = 303,                           // "'index'"
        S_MANUALLY = 304,                        // "'manually'"
        S_AUTOMATICALLY = 305,                   // "'automatically'"
        S_MAINTAINED = 306,                      // "'maintained'"
        S_ON = 307,                              // "'on'"
        S_RANGE = 308,                           // "'range'"
        S_EQUALITY = 309,                        // "'equality'"
        S_GENERAL = 310,                         // "'general'"
        S_INTEGRITY = 311,                       // "'integrity'"
        S_CONSTRAINT = 312,                      // "'constraint'"
        S_CHECK = 313,                           // "'check'"
        S_KEY = 314,                             // "'key'"
        S_FOREACH = 315,                         // "'foreach'"
        S_FOREIGN = 316,                         // "'foreign'"
        S_KEYS = 317,                            // "'keys'"
        S_L_SIMPLE_OBJ_UNION = 318,              // "'{|'"
        S_R_SIMPLE_OBJ_UNION = 319,              // "'|}'"
        S_L_ACCUMULATOR_OBJ_UNION = 320,         // "'{['"
        S_R_ACCUMULATOR_OBJ_UNION = 321,         // "']}'"
        S_JSON = 322,                            // "'json'"
        S_APPEND = 323,                          // "'append'"
        S_POSITION = 324,                        // "'position'"
        S_OBJECT = 325,                          // "'object'"
        S_ARRAY = 326,                           // "'array'"
        S_JSON_ITEM = 327,                       // "'json-item'"
        S_STRUCTURED_ITEM = 328,                 // "'structured-item'"
        S_BYTE_ORDER_MARK_UTF8 = 329,            // "'BOM_UTF8'"
        S_SHEBANG = 330,                         // "'#!/shebang"
        S_RANGE_REDUCE = 331,                    // RANGE_REDUCE
        S_SEQUENCE_TYPE_REDUCE = 332,            // SEQUENCE_TYPE_REDUCE
        S_ADDITIVE_REDUCE = 333,                 // ADDITIVE_REDUCE
        S_UNION_REDUCE = 334,                    // UNION_REDUCE
        S_INTERSECT_EXCEPT_REDUCE = 335,         // INTERSECT_EXCEPT_REDUCE
        S_QVARINDECLLIST_REDUCE = 336,           // QVARINDECLLIST_REDUCE
        S_UNARY_PREC = 337,                      // UNARY_PREC
        S_STEP_REDUCE = 338,                     // STEP_REDUCE
        S_SIMPLEMAPEXPR_REDUCE = 339,            // SIMPLEMAPEXPR_REDUCE
        S_MULTIPLICATIVE_REDUCE = 340,           // MULTIPLICATIVE_REDUCE
        S_JSONLOOKUPEXPR_REDUCE = 341,           // JSONLOOKUPEXPR_REDUCE
        S_YYACCEPT = 342,                        // $accept
        S_Module = 343,                          // Module
        S_ERROR = 344,                           // ERROR
        S_ModuleWithoutBOM = 345,                // ModuleWithoutBOM
        S_VersionDecl = 346,                     // VersionDecl
        S_MainModule = 347,                      // MainModule
        S_LibraryModule = 348,                   // LibraryModule
        S_ModuleDecl = 349,                      // ModuleDecl
        S_SIND_DeclList = 350,                   // SIND_DeclList
        S_SIND_Decl = 351,                       // SIND_Decl
        S_Setter = 352,                          // Setter
        S_BoundarySpaceDecl = 353,               // BoundarySpaceDecl
        S_DefaultCollationDecl = 354,            // DefaultCollationDecl
        S_BaseURIDecl = 355,                     // BaseURIDecl
        S_ConstructionDecl = 356,                // ConstructionDecl
        S_OrderingModeDecl = 357,                // OrderingModeDecl
        S_EmptyOrderDecl = 358,                  // EmptyOrderDecl
        S_CopyNamespacesDecl = 359,              // CopyNamespacesDecl
        S_Import = 360,                          // Import
        S_SchemaImport = 361,                    // SchemaImport
        S_URILiteralList = 362,                  // URILiteralList
        S_SchemaPrefix = 363,                    // SchemaPrefix
        S_ModuleImport = 364,                    // ModuleImport
        S_NamespaceDecl = 365,                   // NamespaceDecl
        S_DefaultNamespaceDecl = 366,            // DefaultNamespaceDecl
        S_VFO_DeclList = 367,                    // VFO_DeclList
        S_VFO_Decl = 368,                        // VFO_Decl
        S_DecimalFormatDecl = 369,               // DecimalFormatDecl
        S_DecimalFormatParamList = 370,          // DecimalFormatParamList
        S_DecimalFormatParam = 371,              // DecimalFormatParam
        S_DecimalFormatParamName = 372,          // DecimalFormatParamName
        S_OptionDecl = 373,                      // OptionDecl
        S_FTOptionDecl = 374,                    // FTOptionDecl
        S_CtxItemDecl = 375,                     // CtxItemDecl
        S_CtxItemDecl2 = 376,                    // CtxItemDecl2
        S_CtxItemDecl3 = 377,                    // CtxItemDecl3
        S_CtxItemDecl4 = 378,                    // CtxItemDecl4
        S_VarDecl = 379,                         // VarDecl
        S_VarNameAndType = 380,                  // VarNameAndType
        S_AnnotationList = 381,                  // AnnotationList
        S_Annotation = 382,                      // Annotation
        S_AnnotationLiteralList = 383,           // AnnotationLiteralList
        S_FunctionDecl = 384,                    // FunctionDecl
        S_FunctionDecl2 = 385,                   // FunctionDecl2
        S_FunctionDeclSimple = 386,              // FunctionDeclSimple
        S_FunctionDeclUpdating = 387,            // FunctionDeclUpdating
        S_FunctionSig = 388,                     // FunctionSig
        S_ParamList = 389,                       // ParamList
        S_Param = 390,                           // Param
        S_CollectionDecl = 391,                  // CollectionDecl
        S_CollectionTypeDecl = 392,              // CollectionTypeDecl
        S_IndexDecl = 393,                       // IndexDecl
        S_IndexKeyList = 394,                    // IndexKeyList
        S_IndexKeySpec = 395,                    // IndexKeySpec
        S_IntegrityConstraintDecl = 396,         // IntegrityConstraintDecl
        S_QueryBody = 397,                       // QueryBody
        S_StatementsAndOptionalExprTop = 398,    // StatementsAndOptionalExprTop
        S_StatementsAndOptionalExpr = 399,       // StatementsAndOptionalExpr
        S_StatementsAndExpr = 400,               // StatementsAndExpr
        S_Statements = 401,                      // Statements
        S_Statement = 402,                       // Statement
        S_BlockStatement = 403,                  // BlockStatement
        S_BlockExpr = 404,                       // BlockExpr
        S_EnclosedStatementsAndOptionalExpr = 405, // EnclosedStatementsAndOptionalExpr
        S_VarDeclStatement = 406,                // VarDeclStatement
        S_BlockVarDeclList = 407,                // BlockVarDeclList
        S_BlockVarDecl = 408,                    // BlockVarDecl
        S_AssignStatement = 409,                 // AssignStatement
        S_ApplyStatement = 410,                  // ApplyStatement
        S_ExitStatement = 411,                   // ExitStatement
        S_WhileStatement = 412,                  // WhileStatement
        S_FlowCtlStatement = 413,                // FlowCtlStatement
        S_FLWORStatement = 414,                  // FLWORStatement
        S_ReturnStatement = 415,                 // ReturnStatement
        S_IfStatement = 416,                     // IfStatement
        S_TryStatement = 417,                    // TryStatement
        S_CatchListStatement = 418,              // CatchListStatement
        S_CatchStatement = 419,                  // CatchStatement
        S_VoidStatement = 420,                   // VoidStatement
        S_Expr = 421,                            // Expr
        S_ExprSingle = 422,                      // ExprSingle
        S_ExprSimple = 423,                      // ExprSimple
        S_FLWORExpr = 424,                       // FLWORExpr
        S_ReturnExpr = 425,                      // ReturnExpr
        S_ReturnOrSelect = 426,                  // ReturnOrSelect
        S_WindowType = 427,                      // WindowType
        S_FLWORWinCondType = 428,                // FLWORWinCondType
        S_FLWORWinCond = 429,                    // FLWORWinCond
        S_WindowClause = 430,                    // WindowClause
        S_CountClause = 431,                     // CountClause
        S_ForLetWinClause = 432,                 // ForLetWinClause
        S_FLWORClause = 433,                     // FLWORClause
        S_FLWORClauseList = 434,                 // FLWORClauseList
        S_ForClause = 435,                       // ForClause
        S_ForOrFrom = 436,                       // ForOrFrom
        S_VarInDeclList = 437,                   // VarInDeclList
        S_VarInDecl = 438,                       // VarInDecl
        S_PositionalVar = 439,                   // PositionalVar
        S_FTScoreVar = 440,                      // FTScoreVar
        S_LetClause = 441,                       // LetClause
        S_VarGetsDeclList = 442,                 // VarGetsDeclList
        S_VarGetsDecl = 443,                     // VarGetsDecl
        S_WindowVarDecl = 444,                   // WindowVarDecl
        S_WindowVars = 445,                      // WindowVars
        S_WindowVars3 = 446,                     // WindowVars3
        S_WindowVars2 = 447,                     // WindowVars2
        S_WhereClause = 448,                     // WhereClause
        S_GroupByClause = 449,                   // GroupByClause
        S_GroupSpecList = 450,                   // GroupSpecList
        S_GroupSpec = 451,                       // GroupSpec
        S_GroupCollationSpec = 452,              // GroupCollationSpec
        S_OrderByClause = 453,                   // OrderByClause
        S_OrderSpecList = 454,                   // OrderSpecList
        S_OrderSpec = 455,                       // OrderSpec
        S_OrderModifier = 456,                   // OrderModifier
        S_OrderDirSpec = 457,                    // OrderDirSpec
        S_OrderEmptySpec = 458,                  // OrderEmptySpec
        S_OrderCollationSpec = 459,              // OrderCollationSpec
        S_QuantifiedExpr = 460,                  // QuantifiedExpr
        S_QVarInDeclList = 461,                  // QVarInDeclList
        S_QVarInDecl = 462,                      // QVarInDecl
        S_SwitchExpr = 463,                      // SwitchExpr
        S_SwitchCaseClauseList = 464,            // SwitchCaseClauseList
        S_SwitchCaseClause = 465,                // SwitchCaseClause
        S_SwitchCaseOperandList = 466,           // SwitchCaseOperandList
        S_SwitchStatement = 467,                 // SwitchStatement
        S_SwitchCaseStatementList = 468,         // SwitchCaseStatementList
        S_SwitchCaseStatement = 469,             // SwitchCaseStatement
        S_TypeswitchExpr = 470,                  // TypeswitchExpr
        S_TypeswitchStatement = 471,             // TypeswitchStatement
        S_CaseClauseList = 472,                  // CaseClauseList
        S_CaseClause = 473,                      // CaseClause
        S_CaseStatementList = 474,               // CaseStatementList
        S_CaseStatement = 475,                   // CaseStatement
        S_SequenceTypeList = 476,                // SequenceTypeList
        S_IfExpr = 477,                          // IfExpr
        S_OrExpr = 478,                          // OrExpr
        S_AndExpr = 479,                         // AndExpr
        S_NotExpr = 480,                         // NotExpr
        S_ComparisonExpr = 481,                  // ComparisonExpr
        S_482_1 = 482,                           // $@1
        S_FTContainsExpr = 483,                  // FTContainsExpr
        S_StringConcatExpr = 484,                // StringConcatExpr
        S_opt_FTIgnoreOption = 485,              // opt_FTIgnoreOption
        S_RangeExpr = 486,                       // RangeExpr
        S_AdditiveExpr = 487,                    // AdditiveExpr
        S_MultiplicativeExpr = 488,              // MultiplicativeExpr
        S_UnionExpr = 489,                       // UnionExpr
        S_IntersectExceptExpr = 490,             // IntersectExceptExpr
        S_InstanceofExpr = 491,                  // InstanceofExpr
        S_TreatExpr = 492,                       // TreatExpr
        S_CastableExpr = 493,                    // CastableExpr
        S_CastExpr = 494,                        // CastExpr
        S_SingleType = 495,                      // SingleType
        S_UnaryExpr = 496,                       // UnaryExpr
        S_SignList = 497,                        // SignList
        S_ValueExpr = 498,                       // ValueExpr
        S_SimpleMapExpr = 499,                   // SimpleMapExpr
        S_ValueComp = 500,                       // ValueComp
        S_NodeComp = 501,                        // NodeComp
        S_ValidateExpr = 502,                    // ValidateExpr
        S_ExtensionExpr = 503,                   // ExtensionExpr
        S_Pragma_list = 504,                     // Pragma_list
        S_Pragma = 505,                          // Pragma
        S_PathExpr = 506,                        // PathExpr
        S_LeadingSlash = 507,                    // LeadingSlash
        S_RelativePathExpr = 508,                // RelativePathExpr
        S_StepExpr = 509,                        // StepExpr
        S_AxisStep = 510,                        // AxisStep
        S_ForwardStep = 511,                     // ForwardStep
        S_ForwardAxis = 512,                     // ForwardAxis
        S_AbbrevForwardStep = 513,               // AbbrevForwardStep
        S_ReverseStep = 514,                     // ReverseStep
        S_ReverseAxis = 515,                     // ReverseAxis
        S_NodeTest = 516,                        // NodeTest
        S_NameTest = 517,                        // NameTest
        S_Wildcard = 518,                        // Wildcard
        S_PostfixExpr = 519,                     // PostfixExpr
        S_PredicateList = 520,                   // PredicateList
        S_Predicate = 521,                       // Predicate
        S_PrimaryExpr = 522,                     // PrimaryExpr
        S_Literal = 523,                         // Literal
        S_NumericLiteral = 524,                  // NumericLiteral
        S_VarRef = 525,                          // VarRef
        S_ParenthesizedExpr = 526,               // ParenthesizedExpr
        S_ContextItemExpr = 527,                 // ContextItemExpr
        S_OrderedExpr = 528,                     // OrderedExpr
        S_UnorderedExpr = 529,                   // UnorderedExpr
        S_FunctionCall = 530,                    // FunctionCall
        S_ArgList = 531,                         // ArgList
        S_FunctionItemExpr = 532,                // FunctionItemExpr
        S_LiteralFunctionItem = 533,             // LiteralFunctionItem
        S_InlineFunction = 534,                  // InlineFunction
        S_Constructor = 535,                     // Constructor
        S_DirectConstructor = 536,               // DirectConstructor
        S_DirElemConstructor = 537,              // DirElemConstructor
        S_DirElemContentList = 538,              // DirElemContentList
        S_DirAttributeList = 539,                // DirAttributeList
        S_DirAttr = 540,                         // DirAttr
        S_OptionalBlank = 541,                   // OptionalBlank
        S_DirAttributeValue = 542,               // DirAttributeValue
        S_opt_QuoteAttrContentList = 543,        // opt_QuoteAttrContentList
        S_QuoteAttrContentList = 544,            // QuoteAttrContentList
        S_opt_AposAttrContentList = 545,         // opt_AposAttrContentList
        S_AposAttrContentList = 546,             // AposAttrContentList
        S_QuoteAttrValueContent = 547,           // QuoteAttrValueContent
        S_AposAttrValueContent = 548,            // AposAttrValueContent
        S_DirElemContent = 549,                  // DirElemContent
        S_CommonContent = 550,                   // CommonContent
        S_DirCommentConstructor = 551,           // DirCommentConstructor
        S_DirPIConstructor = 552,                // DirPIConstructor
        S_CDataSection = 553,                    // CDataSection
        S_ComputedConstructor = 554,             // ComputedConstructor
        S_CompDocConstructor = 555,              // CompDocConstructor
        S_CompElemConstructor = 556,             // CompElemConstructor
        S_CompAttrConstructor = 557,             // CompAttrConstructor
        S_CompTextConstructor = 558,             // CompTextConstructor
        S_CompCommentConstructor = 559,          // CompCommentConstructor
        S_CompPIConstructor = 560,               // CompPIConstructor
        S_CompNamespaceConstructor = 561,        // CompNamespaceConstructor
        S_TypeDeclaration = 562,                 // TypeDeclaration
        S_SequenceType = 563,                    // SequenceType
        S_OccurrenceIndicator = 564,             // OccurrenceIndicator
        S_ItemType = 565,                        // ItemType
        S_TypeList = 566,                        // TypeList
        S_GeneralizedAtomicType = 567,           // GeneralizedAtomicType
        S_SimpleType = 568,                      // SimpleType
        S_KindTest = 569,                        // KindTest
        S_AnyKindTest = 570,                     // AnyKindTest
        S_NODE_KIND_TEST = 571,                  // NODE_KIND_TEST
        S_DocumentTest = 572,                    // DocumentTest
        S_NamespaceTest = 573,                   // NamespaceTest
        S_TextTest = 574,                        // TextTest
        S_CommentTest = 575,                     // CommentTest
        S_PITest = 576,                          // PITest
        S_AttributeTest = 577,                   // AttributeTest
        S_SchemaAttributeTest = 578,             // SchemaAttributeTest
        S_ElementTest = 579,                     // ElementTest
        S_SchemaElementTest = 580,               // SchemaElementTest
        S_TypeName = 581,                        // TypeName
        S_TypeName_WITH_HOOK = 582,              // TypeName_WITH_HOOK
        S_StringLiteral = 583,                   // StringLiteral
        S_FunctionTest = 584,                    // FunctionTest
        S_AnyFunctionTest = 585,                 // AnyFunctionTest
        S_TypedFunctionTest = 586,               // TypedFunctionTest
        S_ParenthesizedItemType = 587,           // ParenthesizedItemType
        S_RevalidationDecl = 588,                // RevalidationDecl
        S_InsertExpr = 589,                      // InsertExpr
        S_DeleteExpr = 590,                      // DeleteExpr
        S_ReplaceExpr = 591,                     // ReplaceExpr
        S_RenameExpr = 592,                      // RenameExpr
        S_TransformExpr = 593,                   // TransformExpr
        S_VarNameList = 594,                     // VarNameList
        S_VarNameDecl = 595,                     // VarNameDecl
        S_TryExpr = 596,                         // TryExpr
        S_CatchListExpr = 597,                   // CatchListExpr
        S_CatchExpr = 598,                       // CatchExpr
        S_BracedExpr = 599,                      // BracedExpr
        S_NameTestList = 600,                    // NameTestList
        S_FTSelection = 601,                     // FTSelection
        S_opt_FTPosFilter_list = 602,            // opt_FTPosFilter_list
        S_FTPosFilter_list = 603,                // FTPosFilter_list
        S_FTOr = 604,                            // FTOr
        S_FTAnd = 605,                           // FTAnd
        S_FTMildNot = 606,                       // FTMildNot
        S_FTUnaryNot = 607,                      // FTUnaryNot
        S_FTPrimaryWithOptions = 608,            // FTPrimaryWithOptions
        S_opt_FTMatchOptions = 609,              // opt_FTMatchOptions
        S_opt_FTWeight = 610,                    // opt_FTWeight
        S_FTWeight = 611,                        // FTWeight
        S_FTPrimary = 612,                       // FTPrimary
        S_opt_FTTimes = 613,                     // opt_FTTimes
        S_FTExtensionSelection = 614,            // FTExtensionSelection
        S_opt_FTSelection = 615,                 // opt_FTSelection
        S_FTWords = 616,                         // FTWords
        S_FTWordsValue = 617,                    // FTWordsValue
        S_opt_FTAnyallOption = 618,              // opt_FTAnyallOption
        S_FTAnyallOption = 619,                  // FTAnyallOption
        S_opt_word = 620,                        // opt_word
        S_opt_words = 621,                       // opt_words
        S_FTPosFilter = 622,                     // FTPosFilter
        S_FTOrder = 623,                         // FTOrder
        S_FTWindow = 624,                        // FTWindow
        S_FTDistance = 625,                      // FTDistance
        S_FTUnit = 626,                          // FTUnit
        S_FTMatchOptions = 627,                  // FTMatchOptions
        S_FTMatchOption = 628,                   // FTMatchOption
        S_FTCaseOption = 629,                    // FTCaseOption
        S_FTDiacriticsOption = 630,              // FTDiacriticsOption
        S_FTExtensionOption = 631,               // FTExtensionOption
        S_FTStemOption = 632,                    // FTStemOption
        S_FTThesaurusOption = 633,               // FTThesaurusOption
        S_FTThesaurusID_or_default = 634,        // FTThesaurusID_or_default
        S_opt_FTThesaurus_list = 635,            // opt_FTThesaurus_list
        S_FTThesaurus_list = 636,                // FTThesaurus_list
        S_FTThesaurusID = 637,                   // FTThesaurusID
        S_opt_relationship = 638,                // opt_relationship
        S_opt_levels = 639,                      // opt_levels
        S_FTStopWordOption = 640,                // FTStopWordOption
        S_FTStopWords = 641,                     // FTStopWords
        S_STRING_LITERAL_list = 642,             // STRING_LITERAL_list
        S_opt_FTStopWordsInclExcl_list = 643,    // opt_FTStopWordsInclExcl_list
        S_FTStopWordsInclExcl_list = 644,        // FTStopWordsInclExcl_list
        S_FTStopWordsInclExcl = 645,             // FTStopWordsInclExcl
        S_FTLanguageOption = 646,                // FTLanguageOption
        S_FTWildCardOption = 647,                // FTWildCardOption
        S_FTContent = 648,                       // FTContent
        S_FTTimes = 649,                         // FTTimes
        S_FTRange = 650,                         // FTRange
        S_FTScope = 651,                         // FTScope
        S_FTBigUnit = 652,                       // FTBigUnit
        S_FTIgnoreOption = 653,                  // FTIgnoreOption
        S_JSONArrayConstructor = 654,            // JSONArrayConstructor
        S_JSONSimpleObjectUnion = 655,           // JSONSimpleObjectUnion
        S_JSONAccumulatorObjectUnion = 656,      // JSONAccumulatorObjectUnion
        S_JSONObjectConstructor = 657,           // JSONObjectConstructor
        S_JSONPairList = 658,                    // JSONPairList
        S_JSONInsertExpr = 659,                  // JSONInsertExpr
        S_JSONAppendExpr = 660,                  // JSONAppendExpr
        S_JSONDeleteExpr = 661,                  // JSONDeleteExpr
        S_JSONRenameExpr = 662,                  // JSONRenameExpr
        S_JSONReplaceExpr = 663,                 // JSONReplaceExpr
        S_JSONTest = 664,                        // JSONTest
        S_URI_LITERAL = 665,                     // URI_LITERAL
        S_QNAME = 666,                           // QNAME
        S_FUNCTION_NAME = 667,                   // FUNCTION_NAME
        S_EQNAME = 668                           // EQNAME
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
      yylast_ = 19530,     ///< Last index in yytable_.
      yynnts_ = 327,  ///< Number of nonterminal symbols.
      yyfinal_ = 628 ///< Termination state number.
    };


    // User arguments.
    xquery_driver& driver;

  };


} // zorba




#endif // !YY_YY_JSONIQ_PARSER_HPP_INCLUDED
