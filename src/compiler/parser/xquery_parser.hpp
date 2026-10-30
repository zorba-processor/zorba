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
    DOT = 375,                     // "'.'"
    DOT_DOT = 376,                 // "'..'"
    COLON = 377,                   // "':'"
    DOUBLE_COLON = 378,            // "'::'"
    DOUBLE_LBRACE = 379,           // "'{{'"
    DOUBLE_LITERAL = 380,          // "'DOUBLE'"
    DOUBLE_RBRACE = 381,           // "'<double {>'"
    ELSE = 382,                    // "'else'"
    _EMPTY = 383,                  // "'empty'"
    GREATEST = 384,                // "'greatest'"
    LEAST = 385,                   // "'least'"
    EMPTY_TAG_END = 386,           // "'/>'"
    ENCODING = 387,                // "'encoding'"
    EQUALS = 388,                  // "'='"
    ESCAPE_APOS = 389,             // "''''"
    ESCAPE_QUOTE = 390,            // "'\"\"'"
    EVERY = 391,                   // "'every'"
    EXCEPT = 392,                  // "'except'"
    EXTERNAL = 393,                // "'external'"
    FOLLOWING = 394,               // "'following'"
    FOLLOWING_SIBLING = 395,       // "'following-sibling'"
    FOLLOWS = 396,                 // "'follows'"
    GE = 397,                      // "'>='"
    GETS = 398,                    // "':='"
    GT = 399,                      // "'>'"
    HOOK = 400,                    // "'?'"
    HASH = 401,                    // "'#'"
    IDIV = 402,                    // "'idiv'"
    _IN = 403,                     // "'in'"
    INHERIT = 404,                 // "'inherit'"
    INTEGER_LITERAL = 405,         // "'INTEGER'"
    INTERSECT = 406,               // "'intersect'"
    IS = 407,                      // "'is'"
    ITEM = 408,                    // "'item'"
    LBRACE = 409,                  // "'{'"
    LBRACK = 410,                  // "'['"
    LE = 411,                      // "'<='"
    LPAR = 412,                    // "'('"
    LT_OR_START_TAG = 413,         // "'<'"
    MINUS = 414,                   // "'-'"
    MOD = 415,                     // "'mod'"
    NAMESPACE = 416,               // "'namespace'"
    _NAN = 417,                    // "'nan'"
    NE = 418,                      // "'!='"
    NODECOMP = 419,                // "'nodecomp'"
    NO_INHERIT = 420,              // "'<no inherit>'"
    NO_PRESERVE = 421,             // "'<no preserve>'"
    OR = 422,                      // "'or'"
    ORDER = 423,                   // "'order'"
    ORDERED = 424,                 // "'ordered'"
    BY = 425,                      // "'by'"
    GROUP = 426,                   // "'group'"
    PARENT = 427,                  // "'parent'"
    PERCENTAGE = 428,              // "'%'"
    PI_BEGIN = 429,                // "'<?'"
    PI_END = 430,                  // "'?>'"
    PLUS = 431,                    // "'+'"
    PRAGMA_BEGIN = 432,            // "'(#'"
    PRAGMA_END = 433,              // "'#)'"
    PRECEDES = 434,                // "'<<'"
    PRECEDING = 435,               // "'preceding'"
    PRECEDING_SIBLING = 436,       // "'preceding-sibling'"
    PRESERVE = 437,                // "'preserve'"
    QUOTE = 438,                   // "'\"'"
    RBRACE = 439,                  // "'}'"
    RBRACK = 440,                  // "']'"
    RETURN = 441,                  // "'return'"
    RPAR = 442,                    // "')'"
    SATISFIES = 443,               // "'satisfies'"
    SELF = 444,                    // "'self'"
    SEMI = 445,                    // "';'"
    SLASH = 446,                   // "'/'"
    SLASH_SLASH = 447,             // "'//'"
    BANG = 448,                    // "'!'"
    STAR = 449,                    // "'*'"
    START_TAG_END = 450,           // "'</ (start tag end)'"
    STRIP = 451,                   // "'strip'"
    TAG_END = 452,                 // "'> (tag end)'"
    THEN = 453,                    // "'then'"
    TO = 454,                      // "'to'"
    TREAT = 455,                   // "'treat'"
    UNION = 456,                   // "'union'"
    UNORDERED = 457,               // "'unordered'"
    VAL_EQ = 458,                  // "'eq'"
    VAL_GE = 459,                  // "'ge'"
    VAL_GT = 460,                  // "'gt'"
    VAL_LE = 461,                  // "'le'"
    VAL_LT = 462,                  // "'lt'"
    VAL_NE = 463,                  // "'ne'"
    VALUECOMP = 464,               // "'VALUECOMP'"
    VBAR = 465,                    // "'|'"
    EMPTY_SEQUENCE = 466,          // "'empty-sequence'"
    WHERE = 467,                   // "'where'"
    XML_COMMENT_BEGIN = 468,       // "'<!--'"
    XML_COMMENT_END = 469,         // "'-->'"
    XQUERY = 470,                  // "'xquery'"
    VERSION = 471,                 // "'version'"
    START = 472,                   // "'start'"
    AFTER = 473,                   // "'after'"
    BEFORE = 474,                  // "'before'"
    REVALIDATION = 475,            // "'revalidation'"
    _STRICT = 476,                 // "'strict'"
    LAX = 477,                     // "'lax'"
    SKIP = 478,                    // "'skip'"
    _DELETE = 479,                 // "'delete'"
    NODE = 480,                    // "'node'"
    INSERT = 481,                  // "'insert'"
    NODES = 482,                   // "'nodes'"
    RENAME = 483,                  // "'rename'"
    REPLACE = 484,                 // "'replace'"
    VALUE = 485,                   // "'value'"
    OF = 486,                      // "'of'"
    FIRST = 487,                   // "'first'"
    INTO = 488,                    // "'into'"
    LAST = 489,                    // "'last'"
    MODIFY = 490,                  // "'modify'"
    COPY = 491,                    // "'copy'"
    WITH = 492,                    // "'with'"
    BREAK = 493,                   // "'break'"
    CONTINUE = 494,                // "'continue'"
    EXIT = 495,                    // "'exit'"
    LOOP = 496,                    // "'loop'"
    RETURNING = 497,               // "'returning'"
    WHILE = 498,                   // "'while'"
    TRY = 499,                     // "'try'"
    CATCH = 500,                   // "'catch'"
    USING = 501,                   // "'using'"
    ALL = 502,                     // "'all'"
    ANY = 503,                     // "'any'"
    CONTAINS = 504,                // "'contains'"
    CONTENT = 505,                 // "'content'"
    DIACRITICS = 506,              // "'diacritics'"
    DIFFERENT = 507,               // "'different'"
    DISTANCE = 508,                // "'distance'"
    END = 509,                     // "'end'"
    ENTIRE = 510,                  // "'entire'"
    EXACTLY = 511,                 // "'exactly'"
    FROM = 512,                    // "'from'"
    FTAND = 513,                   // "'ftand'"
    FTNOT = 514,                   // "'ftnot'"
    NOT = 515,                     // "'not'"
    FT_OPTION = 516,               // "'ft-option'"
    FTOR = 517,                    // "'ftor'"
    INSENSITIVE = 518,             // "'insensitive'"
    LANGUAGE = 519,                // "'language'"
    LEVELS = 520,                  // "'levels'"
    LOWERCASE = 521,               // "'lowercase'"
    OCCURS = 522,                  // "'occurs'"
    PARAGRAPH = 523,               // "'paragraph'"
    PARAGRAPHS = 524,              // "'paragraphs'"
    PHRASE = 525,                  // "'phrase'"
    RELATIONSHIP = 526,            // "'relationship'"
    SAME = 527,                    // "'same'"
    SCORE = 528,                   // "'score'"
    SENSITIVE = 529,               // "'sensitive'"
    SENTENCE = 530,                // "'sentence'"
    SENTENCES = 531,               // "'sentences'"
    STEMMING = 532,                // "'stemming'"
    STOP = 533,                    // "'stop'"
    THESAURUS = 534,               // "'thesaurus'"
    TIMES = 535,                   // "'times'"
    UPPERCASE = 536,               // "'uppercase'"
    WEIGHT = 537,                  // "'weight'"
    WILDCARDS = 538,               // "'wildcards'"
    WINDOW = 539,                  // "'window'"
    WITHOUT = 540,                 // "'without'"
    WORDS = 541,                   // "'words'"
    COLLECTION = 542,              // "'collection'"
    CONSTOPT = 543,                // "'const'"
    APPEND_ONLY = 544,             // "'append-only'"
    QUEUE = 545,                   // "'queue'"
    MUTABLE = 546,                 // "'mutable'"
    READ_ONLY = 547,               // "'read-only'"
    UNIQUE = 548,                  // "'unique'"
    NON = 549,                     // "'non'"
    INDEX = 550,                   // "'index'"
    MANUALLY = 551,                // "'manually'"
    AUTOMATICALLY = 552,           // "'automatically'"
    MAINTAINED = 553,              // "'maintained'"
    ON = 554,                      // "'on'"
    RANGE = 555,                   // "'range'"
    EQUALITY = 556,                // "'equality'"
    GENERAL = 557,                 // "'general'"
    INTEGRITY = 558,               // "'integrity'"
    CONSTRAINT = 559,              // "'constraint'"
    CHECK = 560,                   // "'check'"
    KEY = 561,                     // "'key'"
    FOREACH = 562,                 // "'foreach'"
    FOREIGN = 563,                 // "'foreign'"
    KEYS = 564,                    // "'keys'"
    L_SIMPLE_OBJ_UNION = 565,      // "'{|'"
    R_SIMPLE_OBJ_UNION = 566,      // "'|}'"
    L_ACCUMULATOR_OBJ_UNION = 567, // "'{['"
    R_ACCUMULATOR_OBJ_UNION = 568, // "']}'"
    JSON = 569,                    // "'json'"
    APPEND = 570,                  // "'append'"
    POSITION = 571,                // "'position'"
    OBJECT = 572,                  // "'object'"
    ARRAY = 573,                   // "'array'"
    JSON_ITEM = 574,               // "'json-item'"
    STRUCTURED_ITEM = 575,         // "'structured-item'"
    BYTE_ORDER_MARK_UTF8 = 576,    // "'BOM_UTF8'"
    SHEBANG = 577,                 // "'#!/shebang"
    RANGE_REDUCE = 578,            // RANGE_REDUCE
    SEQUENCE_TYPE_REDUCE = 579,    // SEQUENCE_TYPE_REDUCE
    ADDITIVE_REDUCE = 580,         // ADDITIVE_REDUCE
    UNION_REDUCE = 581,            // UNION_REDUCE
    INTERSECT_EXCEPT_REDUCE = 582, // INTERSECT_EXCEPT_REDUCE
    QVARINDECLLIST_REDUCE = 583,   // QVARINDECLLIST_REDUCE
    UNARY_PREC = 584,              // UNARY_PREC
    STEP_REDUCE = 585,             // STEP_REDUCE
    SIMPLEMAPEXPR_REDUCE = 586,    // SIMPLEMAPEXPR_REDUCE
    MULTIPLICATIVE_REDUCE = 587,   // MULTIPLICATIVE_REDUCE
    SELECT = 588                   // SELECT
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
        YYNTOKENS = 334, ///< Number of tokens.
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
        S_DOT = 120,                             // "'.'"
        S_DOT_DOT = 121,                         // "'..'"
        S_COLON = 122,                           // "':'"
        S_DOUBLE_COLON = 123,                    // "'::'"
        S_DOUBLE_LBRACE = 124,                   // "'{{'"
        S_DOUBLE_LITERAL = 125,                  // "'DOUBLE'"
        S_DOUBLE_RBRACE = 126,                   // "'<double {>'"
        S_ELSE = 127,                            // "'else'"
        S__EMPTY = 128,                          // "'empty'"
        S_GREATEST = 129,                        // "'greatest'"
        S_LEAST = 130,                           // "'least'"
        S_EMPTY_TAG_END = 131,                   // "'/>'"
        S_ENCODING = 132,                        // "'encoding'"
        S_EQUALS = 133,                          // "'='"
        S_ESCAPE_APOS = 134,                     // "''''"
        S_ESCAPE_QUOTE = 135,                    // "'\"\"'"
        S_EVERY = 136,                           // "'every'"
        S_EXCEPT = 137,                          // "'except'"
        S_EXTERNAL = 138,                        // "'external'"
        S_FOLLOWING = 139,                       // "'following'"
        S_FOLLOWING_SIBLING = 140,               // "'following-sibling'"
        S_FOLLOWS = 141,                         // "'follows'"
        S_GE = 142,                              // "'>='"
        S_GETS = 143,                            // "':='"
        S_GT = 144,                              // "'>'"
        S_HOOK = 145,                            // "'?'"
        S_HASH = 146,                            // "'#'"
        S_IDIV = 147,                            // "'idiv'"
        S__IN = 148,                             // "'in'"
        S_INHERIT = 149,                         // "'inherit'"
        S_INTEGER_LITERAL = 150,                 // "'INTEGER'"
        S_INTERSECT = 151,                       // "'intersect'"
        S_IS = 152,                              // "'is'"
        S_ITEM = 153,                            // "'item'"
        S_LBRACE = 154,                          // "'{'"
        S_LBRACK = 155,                          // "'['"
        S_LE = 156,                              // "'<='"
        S_LPAR = 157,                            // "'('"
        S_LT_OR_START_TAG = 158,                 // "'<'"
        S_MINUS = 159,                           // "'-'"
        S_MOD = 160,                             // "'mod'"
        S_NAMESPACE = 161,                       // "'namespace'"
        S__NAN = 162,                            // "'nan'"
        S_NE = 163,                              // "'!='"
        S_NODECOMP = 164,                        // "'nodecomp'"
        S_NO_INHERIT = 165,                      // "'<no inherit>'"
        S_NO_PRESERVE = 166,                     // "'<no preserve>'"
        S_OR = 167,                              // "'or'"
        S_ORDER = 168,                           // "'order'"
        S_ORDERED = 169,                         // "'ordered'"
        S_BY = 170,                              // "'by'"
        S_GROUP = 171,                           // "'group'"
        S_PARENT = 172,                          // "'parent'"
        S_PERCENTAGE = 173,                      // "'%'"
        S_PI_BEGIN = 174,                        // "'<?'"
        S_PI_END = 175,                          // "'?>'"
        S_PLUS = 176,                            // "'+'"
        S_PRAGMA_BEGIN = 177,                    // "'(#'"
        S_PRAGMA_END = 178,                      // "'#)'"
        S_PRECEDES = 179,                        // "'<<'"
        S_PRECEDING = 180,                       // "'preceding'"
        S_PRECEDING_SIBLING = 181,               // "'preceding-sibling'"
        S_PRESERVE = 182,                        // "'preserve'"
        S_QUOTE = 183,                           // "'\"'"
        S_RBRACE = 184,                          // "'}'"
        S_RBRACK = 185,                          // "']'"
        S_RETURN = 186,                          // "'return'"
        S_RPAR = 187,                            // "')'"
        S_SATISFIES = 188,                       // "'satisfies'"
        S_SELF = 189,                            // "'self'"
        S_SEMI = 190,                            // "';'"
        S_SLASH = 191,                           // "'/'"
        S_SLASH_SLASH = 192,                     // "'//'"
        S_BANG = 193,                            // "'!'"
        S_STAR = 194,                            // "'*'"
        S_START_TAG_END = 195,                   // "'</ (start tag end)'"
        S_STRIP = 196,                           // "'strip'"
        S_TAG_END = 197,                         // "'> (tag end)'"
        S_THEN = 198,                            // "'then'"
        S_TO = 199,                              // "'to'"
        S_TREAT = 200,                           // "'treat'"
        S_UNION = 201,                           // "'union'"
        S_UNORDERED = 202,                       // "'unordered'"
        S_VAL_EQ = 203,                          // "'eq'"
        S_VAL_GE = 204,                          // "'ge'"
        S_VAL_GT = 205,                          // "'gt'"
        S_VAL_LE = 206,                          // "'le'"
        S_VAL_LT = 207,                          // "'lt'"
        S_VAL_NE = 208,                          // "'ne'"
        S_VALUECOMP = 209,                       // "'VALUECOMP'"
        S_VBAR = 210,                            // "'|'"
        S_EMPTY_SEQUENCE = 211,                  // "'empty-sequence'"
        S_WHERE = 212,                           // "'where'"
        S_XML_COMMENT_BEGIN = 213,               // "'<!--'"
        S_XML_COMMENT_END = 214,                 // "'-->'"
        S_XQUERY = 215,                          // "'xquery'"
        S_VERSION = 216,                         // "'version'"
        S_START = 217,                           // "'start'"
        S_AFTER = 218,                           // "'after'"
        S_BEFORE = 219,                          // "'before'"
        S_REVALIDATION = 220,                    // "'revalidation'"
        S__STRICT = 221,                         // "'strict'"
        S_LAX = 222,                             // "'lax'"
        S_SKIP = 223,                            // "'skip'"
        S__DELETE = 224,                         // "'delete'"
        S_NODE = 225,                            // "'node'"
        S_INSERT = 226,                          // "'insert'"
        S_NODES = 227,                           // "'nodes'"
        S_RENAME = 228,                          // "'rename'"
        S_REPLACE = 229,                         // "'replace'"
        S_VALUE = 230,                           // "'value'"
        S_OF = 231,                              // "'of'"
        S_FIRST = 232,                           // "'first'"
        S_INTO = 233,                            // "'into'"
        S_LAST = 234,                            // "'last'"
        S_MODIFY = 235,                          // "'modify'"
        S_COPY = 236,                            // "'copy'"
        S_WITH = 237,                            // "'with'"
        S_BREAK = 238,                           // "'break'"
        S_CONTINUE = 239,                        // "'continue'"
        S_EXIT = 240,                            // "'exit'"
        S_LOOP = 241,                            // "'loop'"
        S_RETURNING = 242,                       // "'returning'"
        S_WHILE = 243,                           // "'while'"
        S_TRY = 244,                             // "'try'"
        S_CATCH = 245,                           // "'catch'"
        S_USING = 246,                           // "'using'"
        S_ALL = 247,                             // "'all'"
        S_ANY = 248,                             // "'any'"
        S_CONTAINS = 249,                        // "'contains'"
        S_CONTENT = 250,                         // "'content'"
        S_DIACRITICS = 251,                      // "'diacritics'"
        S_DIFFERENT = 252,                       // "'different'"
        S_DISTANCE = 253,                        // "'distance'"
        S_END = 254,                             // "'end'"
        S_ENTIRE = 255,                          // "'entire'"
        S_EXACTLY = 256,                         // "'exactly'"
        S_FROM = 257,                            // "'from'"
        S_FTAND = 258,                           // "'ftand'"
        S_FTNOT = 259,                           // "'ftnot'"
        S_NOT = 260,                             // "'not'"
        S_FT_OPTION = 261,                       // "'ft-option'"
        S_FTOR = 262,                            // "'ftor'"
        S_INSENSITIVE = 263,                     // "'insensitive'"
        S_LANGUAGE = 264,                        // "'language'"
        S_LEVELS = 265,                          // "'levels'"
        S_LOWERCASE = 266,                       // "'lowercase'"
        S_OCCURS = 267,                          // "'occurs'"
        S_PARAGRAPH = 268,                       // "'paragraph'"
        S_PARAGRAPHS = 269,                      // "'paragraphs'"
        S_PHRASE = 270,                          // "'phrase'"
        S_RELATIONSHIP = 271,                    // "'relationship'"
        S_SAME = 272,                            // "'same'"
        S_SCORE = 273,                           // "'score'"
        S_SENSITIVE = 274,                       // "'sensitive'"
        S_SENTENCE = 275,                        // "'sentence'"
        S_SENTENCES = 276,                       // "'sentences'"
        S_STEMMING = 277,                        // "'stemming'"
        S_STOP = 278,                            // "'stop'"
        S_THESAURUS = 279,                       // "'thesaurus'"
        S_TIMES = 280,                           // "'times'"
        S_UPPERCASE = 281,                       // "'uppercase'"
        S_WEIGHT = 282,                          // "'weight'"
        S_WILDCARDS = 283,                       // "'wildcards'"
        S_WINDOW = 284,                          // "'window'"
        S_WITHOUT = 285,                         // "'without'"
        S_WORDS = 286,                           // "'words'"
        S_COLLECTION = 287,                      // "'collection'"
        S_CONSTOPT = 288,                        // "'const'"
        S_APPEND_ONLY = 289,                     // "'append-only'"
        S_QUEUE = 290,                           // "'queue'"
        S_MUTABLE = 291,                         // "'mutable'"
        S_READ_ONLY = 292,                       // "'read-only'"
        S_UNIQUE = 293,                          // "'unique'"
        S_NON = 294,                             // "'non'"
        S_INDEX = 295,                           // "'index'"
        S_MANUALLY = 296,                        // "'manually'"
        S_AUTOMATICALLY = 297,                   // "'automatically'"
        S_MAINTAINED = 298,                      // "'maintained'"
        S_ON = 299,                              // "'on'"
        S_RANGE = 300,                           // "'range'"
        S_EQUALITY = 301,                        // "'equality'"
        S_GENERAL = 302,                         // "'general'"
        S_INTEGRITY = 303,                       // "'integrity'"
        S_CONSTRAINT = 304,                      // "'constraint'"
        S_CHECK = 305,                           // "'check'"
        S_KEY = 306,                             // "'key'"
        S_FOREACH = 307,                         // "'foreach'"
        S_FOREIGN = 308,                         // "'foreign'"
        S_KEYS = 309,                            // "'keys'"
        S_L_SIMPLE_OBJ_UNION = 310,              // "'{|'"
        S_R_SIMPLE_OBJ_UNION = 311,              // "'|}'"
        S_L_ACCUMULATOR_OBJ_UNION = 312,         // "'{['"
        S_R_ACCUMULATOR_OBJ_UNION = 313,         // "']}'"
        S_JSON = 314,                            // "'json'"
        S_APPEND = 315,                          // "'append'"
        S_POSITION = 316,                        // "'position'"
        S_OBJECT = 317,                          // "'object'"
        S_ARRAY = 318,                           // "'array'"
        S_JSON_ITEM = 319,                       // "'json-item'"
        S_STRUCTURED_ITEM = 320,                 // "'structured-item'"
        S_BYTE_ORDER_MARK_UTF8 = 321,            // "'BOM_UTF8'"
        S_SHEBANG = 322,                         // "'#!/shebang"
        S_RANGE_REDUCE = 323,                    // RANGE_REDUCE
        S_SEQUENCE_TYPE_REDUCE = 324,            // SEQUENCE_TYPE_REDUCE
        S_ADDITIVE_REDUCE = 325,                 // ADDITIVE_REDUCE
        S_UNION_REDUCE = 326,                    // UNION_REDUCE
        S_INTERSECT_EXCEPT_REDUCE = 327,         // INTERSECT_EXCEPT_REDUCE
        S_QVARINDECLLIST_REDUCE = 328,           // QVARINDECLLIST_REDUCE
        S_UNARY_PREC = 329,                      // UNARY_PREC
        S_STEP_REDUCE = 330,                     // STEP_REDUCE
        S_SIMPLEMAPEXPR_REDUCE = 331,            // SIMPLEMAPEXPR_REDUCE
        S_MULTIPLICATIVE_REDUCE = 332,           // MULTIPLICATIVE_REDUCE
        S_SELECT = 333,                          // SELECT
        S_YYACCEPT = 334,                        // $accept
        S_Module = 335,                          // Module
        S_ERROR = 336,                           // ERROR
        S_ModuleWithoutBOM = 337,                // ModuleWithoutBOM
        S_VersionDecl = 338,                     // VersionDecl
        S_MainModule = 339,                      // MainModule
        S_LibraryModule = 340,                   // LibraryModule
        S_ModuleDecl = 341,                      // ModuleDecl
        S_SIND_DeclList = 342,                   // SIND_DeclList
        S_SIND_Decl = 343,                       // SIND_Decl
        S_Setter = 344,                          // Setter
        S_BoundarySpaceDecl = 345,               // BoundarySpaceDecl
        S_DefaultCollationDecl = 346,            // DefaultCollationDecl
        S_BaseURIDecl = 347,                     // BaseURIDecl
        S_ConstructionDecl = 348,                // ConstructionDecl
        S_OrderingModeDecl = 349,                // OrderingModeDecl
        S_EmptyOrderDecl = 350,                  // EmptyOrderDecl
        S_CopyNamespacesDecl = 351,              // CopyNamespacesDecl
        S_Import = 352,                          // Import
        S_SchemaImport = 353,                    // SchemaImport
        S_URILiteralList = 354,                  // URILiteralList
        S_SchemaPrefix = 355,                    // SchemaPrefix
        S_ModuleImport = 356,                    // ModuleImport
        S_NamespaceDecl = 357,                   // NamespaceDecl
        S_DefaultNamespaceDecl = 358,            // DefaultNamespaceDecl
        S_VFO_DeclList = 359,                    // VFO_DeclList
        S_VFO_Decl = 360,                        // VFO_Decl
        S_DecimalFormatDecl = 361,               // DecimalFormatDecl
        S_DecimalFormatParamList = 362,          // DecimalFormatParamList
        S_DecimalFormatParam = 363,              // DecimalFormatParam
        S_DecimalFormatParamName = 364,          // DecimalFormatParamName
        S_OptionDecl = 365,                      // OptionDecl
        S_FTOptionDecl = 366,                    // FTOptionDecl
        S_CtxItemDecl = 367,                     // CtxItemDecl
        S_CtxItemDecl2 = 368,                    // CtxItemDecl2
        S_CtxItemDecl3 = 369,                    // CtxItemDecl3
        S_CtxItemDecl4 = 370,                    // CtxItemDecl4
        S_VarDecl = 371,                         // VarDecl
        S_VarNameAndType = 372,                  // VarNameAndType
        S_AnnotationList = 373,                  // AnnotationList
        S_Annotation = 374,                      // Annotation
        S_AnnotationLiteralList = 375,           // AnnotationLiteralList
        S_FunctionDecl = 376,                    // FunctionDecl
        S_FunctionDecl2 = 377,                   // FunctionDecl2
        S_FunctionDeclSimple = 378,              // FunctionDeclSimple
        S_FunctionDeclUpdating = 379,            // FunctionDeclUpdating
        S_FunctionSig = 380,                     // FunctionSig
        S_ParamList = 381,                       // ParamList
        S_Param = 382,                           // Param
        S_CollectionDecl = 383,                  // CollectionDecl
        S_CollectionTypeDecl = 384,              // CollectionTypeDecl
        S_IndexDecl = 385,                       // IndexDecl
        S_IndexKeyList = 386,                    // IndexKeyList
        S_IndexKeySpec = 387,                    // IndexKeySpec
        S_IntegrityConstraintDecl = 388,         // IntegrityConstraintDecl
        S_QueryBody = 389,                       // QueryBody
        S_StatementsAndOptionalExprTop = 390,    // StatementsAndOptionalExprTop
        S_StatementsAndOptionalExpr = 391,       // StatementsAndOptionalExpr
        S_StatementsAndExpr = 392,               // StatementsAndExpr
        S_Statements = 393,                      // Statements
        S_Statement = 394,                       // Statement
        S_BlockStatement = 395,                  // BlockStatement
        S_BlockExpr = 396,                       // BlockExpr
        S_EnclosedStatementsAndOptionalExpr = 397, // EnclosedStatementsAndOptionalExpr
        S_VarDeclStatement = 398,                // VarDeclStatement
        S_BlockVarDeclList = 399,                // BlockVarDeclList
        S_BlockVarDecl = 400,                    // BlockVarDecl
        S_AssignStatement = 401,                 // AssignStatement
        S_ApplyStatement = 402,                  // ApplyStatement
        S_ExitStatement = 403,                   // ExitStatement
        S_WhileStatement = 404,                  // WhileStatement
        S_FlowCtlStatement = 405,                // FlowCtlStatement
        S_FLWORStatement = 406,                  // FLWORStatement
        S_ReturnStatement = 407,                 // ReturnStatement
        S_IfStatement = 408,                     // IfStatement
        S_TryStatement = 409,                    // TryStatement
        S_CatchListStatement = 410,              // CatchListStatement
        S_CatchStatement = 411,                  // CatchStatement
        S_VoidStatement = 412,                   // VoidStatement
        S_Expr = 413,                            // Expr
        S_ExprSingle = 414,                      // ExprSingle
        S_ExprSimple = 415,                      // ExprSimple
        S_FLWORExpr = 416,                       // FLWORExpr
        S_ReturnExpr = 417,                      // ReturnExpr
        S_ReturnOrSelect = 418,                  // ReturnOrSelect
        S_WindowType = 419,                      // WindowType
        S_FLWORWinCondType = 420,                // FLWORWinCondType
        S_FLWORWinCond = 421,                    // FLWORWinCond
        S_WindowClause = 422,                    // WindowClause
        S_CountClause = 423,                     // CountClause
        S_ForLetWinClause = 424,                 // ForLetWinClause
        S_FLWORClause = 425,                     // FLWORClause
        S_FLWORClauseList = 426,                 // FLWORClauseList
        S_ForClause = 427,                       // ForClause
        S_ForOrFrom = 428,                       // ForOrFrom
        S_VarInDeclList = 429,                   // VarInDeclList
        S_VarInDecl = 430,                       // VarInDecl
        S_PositionalVar = 431,                   // PositionalVar
        S_FTScoreVar = 432,                      // FTScoreVar
        S_LetClause = 433,                       // LetClause
        S_VarGetsDeclList = 434,                 // VarGetsDeclList
        S_VarGetsDecl = 435,                     // VarGetsDecl
        S_WindowVarDecl = 436,                   // WindowVarDecl
        S_WindowVars = 437,                      // WindowVars
        S_WindowVars3 = 438,                     // WindowVars3
        S_WindowVars2 = 439,                     // WindowVars2
        S_WhereClause = 440,                     // WhereClause
        S_GroupByClause = 441,                   // GroupByClause
        S_GroupSpecList = 442,                   // GroupSpecList
        S_GroupSpec = 443,                       // GroupSpec
        S_GroupCollationSpec = 444,              // GroupCollationSpec
        S_OrderByClause = 445,                   // OrderByClause
        S_OrderSpecList = 446,                   // OrderSpecList
        S_OrderSpec = 447,                       // OrderSpec
        S_OrderModifier = 448,                   // OrderModifier
        S_OrderDirSpec = 449,                    // OrderDirSpec
        S_OrderEmptySpec = 450,                  // OrderEmptySpec
        S_OrderCollationSpec = 451,              // OrderCollationSpec
        S_QuantifiedExpr = 452,                  // QuantifiedExpr
        S_QVarInDeclList = 453,                  // QVarInDeclList
        S_QVarInDecl = 454,                      // QVarInDecl
        S_SwitchExpr = 455,                      // SwitchExpr
        S_SwitchCaseClauseList = 456,            // SwitchCaseClauseList
        S_SwitchCaseClause = 457,                // SwitchCaseClause
        S_SwitchCaseOperandList = 458,           // SwitchCaseOperandList
        S_SwitchStatement = 459,                 // SwitchStatement
        S_SwitchCaseStatementList = 460,         // SwitchCaseStatementList
        S_SwitchCaseStatement = 461,             // SwitchCaseStatement
        S_TypeswitchExpr = 462,                  // TypeswitchExpr
        S_TypeswitchStatement = 463,             // TypeswitchStatement
        S_CaseClauseList = 464,                  // CaseClauseList
        S_CaseClause = 465,                      // CaseClause
        S_CaseStatementList = 466,               // CaseStatementList
        S_CaseStatement = 467,                   // CaseStatement
        S_SequenceTypeList = 468,                // SequenceTypeList
        S_IfExpr = 469,                          // IfExpr
        S_OrExpr = 470,                          // OrExpr
        S_AndExpr = 471,                         // AndExpr
        S_ComparisonExpr = 472,                  // ComparisonExpr
        S_473_1 = 473,                           // $@1
        S_FTContainsExpr = 474,                  // FTContainsExpr
        S_StringConcatExpr = 475,                // StringConcatExpr
        S_opt_FTIgnoreOption = 476,              // opt_FTIgnoreOption
        S_RangeExpr = 477,                       // RangeExpr
        S_AdditiveExpr = 478,                    // AdditiveExpr
        S_MultiplicativeExpr = 479,              // MultiplicativeExpr
        S_UnionExpr = 480,                       // UnionExpr
        S_IntersectExceptExpr = 481,             // IntersectExceptExpr
        S_InstanceofExpr = 482,                  // InstanceofExpr
        S_TreatExpr = 483,                       // TreatExpr
        S_CastableExpr = 484,                    // CastableExpr
        S_CastExpr = 485,                        // CastExpr
        S_SingleType = 486,                      // SingleType
        S_UnaryExpr = 487,                       // UnaryExpr
        S_SignList = 488,                        // SignList
        S_ValueExpr = 489,                       // ValueExpr
        S_SimpleMapExpr = 490,                   // SimpleMapExpr
        S_ValueComp = 491,                       // ValueComp
        S_NodeComp = 492,                        // NodeComp
        S_ValidateExpr = 493,                    // ValidateExpr
        S_ExtensionExpr = 494,                   // ExtensionExpr
        S_Pragma_list = 495,                     // Pragma_list
        S_Pragma = 496,                          // Pragma
        S_PathExpr = 497,                        // PathExpr
        S_LeadingSlash = 498,                    // LeadingSlash
        S_RelativePathExpr = 499,                // RelativePathExpr
        S_StepExpr = 500,                        // StepExpr
        S_AxisStep = 501,                        // AxisStep
        S_ForwardStep = 502,                     // ForwardStep
        S_ForwardAxis = 503,                     // ForwardAxis
        S_AbbrevForwardStep = 504,               // AbbrevForwardStep
        S_ReverseStep = 505,                     // ReverseStep
        S_ReverseAxis = 506,                     // ReverseAxis
        S_NodeTest = 507,                        // NodeTest
        S_NameTest = 508,                        // NameTest
        S_Wildcard = 509,                        // Wildcard
        S_PostfixExpr = 510,                     // PostfixExpr
        S_PredicateList = 511,                   // PredicateList
        S_Predicate = 512,                       // Predicate
        S_PrimaryExpr = 513,                     // PrimaryExpr
        S_Literal = 514,                         // Literal
        S_NumericLiteral = 515,                  // NumericLiteral
        S_VarRef = 516,                          // VarRef
        S_ParenthesizedExpr = 517,               // ParenthesizedExpr
        S_ContextItemExpr = 518,                 // ContextItemExpr
        S_OrderedExpr = 519,                     // OrderedExpr
        S_UnorderedExpr = 520,                   // UnorderedExpr
        S_FunctionCall = 521,                    // FunctionCall
        S_ArgList = 522,                         // ArgList
        S_FunctionItemExpr = 523,                // FunctionItemExpr
        S_LiteralFunctionItem = 524,             // LiteralFunctionItem
        S_InlineFunction = 525,                  // InlineFunction
        S_Constructor = 526,                     // Constructor
        S_DirectConstructor = 527,               // DirectConstructor
        S_DirElemConstructor = 528,              // DirElemConstructor
        S_DirElemContentList = 529,              // DirElemContentList
        S_DirAttributeList = 530,                // DirAttributeList
        S_DirAttr = 531,                         // DirAttr
        S_OptionalBlank = 532,                   // OptionalBlank
        S_DirAttributeValue = 533,               // DirAttributeValue
        S_opt_QuoteAttrContentList = 534,        // opt_QuoteAttrContentList
        S_QuoteAttrContentList = 535,            // QuoteAttrContentList
        S_opt_AposAttrContentList = 536,         // opt_AposAttrContentList
        S_AposAttrContentList = 537,             // AposAttrContentList
        S_QuoteAttrValueContent = 538,           // QuoteAttrValueContent
        S_AposAttrValueContent = 539,            // AposAttrValueContent
        S_DirElemContent = 540,                  // DirElemContent
        S_CommonContent = 541,                   // CommonContent
        S_DirCommentConstructor = 542,           // DirCommentConstructor
        S_DirPIConstructor = 543,                // DirPIConstructor
        S_CDataSection = 544,                    // CDataSection
        S_ComputedConstructor = 545,             // ComputedConstructor
        S_CompDocConstructor = 546,              // CompDocConstructor
        S_CompElemConstructor = 547,             // CompElemConstructor
        S_CompAttrConstructor = 548,             // CompAttrConstructor
        S_CompTextConstructor = 549,             // CompTextConstructor
        S_CompCommentConstructor = 550,          // CompCommentConstructor
        S_CompPIConstructor = 551,               // CompPIConstructor
        S_CompNamespaceConstructor = 552,        // CompNamespaceConstructor
        S_TypeDeclaration = 553,                 // TypeDeclaration
        S_SequenceType = 554,                    // SequenceType
        S_OccurrenceIndicator = 555,             // OccurrenceIndicator
        S_ItemType = 556,                        // ItemType
        S_TypeList = 557,                        // TypeList
        S_GeneralizedAtomicType = 558,           // GeneralizedAtomicType
        S_SimpleType = 559,                      // SimpleType
        S_KindTest = 560,                        // KindTest
        S_AnyKindTest = 561,                     // AnyKindTest
        S_NODE_KIND_TEST = 562,                  // NODE_KIND_TEST
        S_DocumentTest = 563,                    // DocumentTest
        S_NamespaceTest = 564,                   // NamespaceTest
        S_TextTest = 565,                        // TextTest
        S_CommentTest = 566,                     // CommentTest
        S_PITest = 567,                          // PITest
        S_AttributeTest = 568,                   // AttributeTest
        S_SchemaAttributeTest = 569,             // SchemaAttributeTest
        S_ElementTest = 570,                     // ElementTest
        S_SchemaElementTest = 571,               // SchemaElementTest
        S_TypeName = 572,                        // TypeName
        S_TypeName_WITH_HOOK = 573,              // TypeName_WITH_HOOK
        S_StringLiteral = 574,                   // StringLiteral
        S_FunctionTest = 575,                    // FunctionTest
        S_AnyFunctionTest = 576,                 // AnyFunctionTest
        S_TypedFunctionTest = 577,               // TypedFunctionTest
        S_ParenthesizedItemType = 578,           // ParenthesizedItemType
        S_RevalidationDecl = 579,                // RevalidationDecl
        S_InsertExpr = 580,                      // InsertExpr
        S_DeleteExpr = 581,                      // DeleteExpr
        S_ReplaceExpr = 582,                     // ReplaceExpr
        S_RenameExpr = 583,                      // RenameExpr
        S_TransformExpr = 584,                   // TransformExpr
        S_VarNameList = 585,                     // VarNameList
        S_VarNameDecl = 586,                     // VarNameDecl
        S_TryExpr = 587,                         // TryExpr
        S_CatchListExpr = 588,                   // CatchListExpr
        S_CatchExpr = 589,                       // CatchExpr
        S_BracedExpr = 590,                      // BracedExpr
        S_NameTestList = 591,                    // NameTestList
        S_FTSelection = 592,                     // FTSelection
        S_opt_FTPosFilter_list = 593,            // opt_FTPosFilter_list
        S_FTPosFilter_list = 594,                // FTPosFilter_list
        S_FTOr = 595,                            // FTOr
        S_FTAnd = 596,                           // FTAnd
        S_FTMildNot = 597,                       // FTMildNot
        S_FTUnaryNot = 598,                      // FTUnaryNot
        S_FTPrimaryWithOptions = 599,            // FTPrimaryWithOptions
        S_opt_FTMatchOptions = 600,              // opt_FTMatchOptions
        S_opt_FTWeight = 601,                    // opt_FTWeight
        S_FTWeight = 602,                        // FTWeight
        S_FTPrimary = 603,                       // FTPrimary
        S_opt_FTTimes = 604,                     // opt_FTTimes
        S_FTExtensionSelection = 605,            // FTExtensionSelection
        S_opt_FTSelection = 606,                 // opt_FTSelection
        S_FTWords = 607,                         // FTWords
        S_FTWordsValue = 608,                    // FTWordsValue
        S_opt_FTAnyallOption = 609,              // opt_FTAnyallOption
        S_FTAnyallOption = 610,                  // FTAnyallOption
        S_opt_word = 611,                        // opt_word
        S_opt_words = 612,                       // opt_words
        S_FTPosFilter = 613,                     // FTPosFilter
        S_FTOrder = 614,                         // FTOrder
        S_FTWindow = 615,                        // FTWindow
        S_FTDistance = 616,                      // FTDistance
        S_FTUnit = 617,                          // FTUnit
        S_FTMatchOptions = 618,                  // FTMatchOptions
        S_FTMatchOption = 619,                   // FTMatchOption
        S_FTCaseOption = 620,                    // FTCaseOption
        S_FTDiacriticsOption = 621,              // FTDiacriticsOption
        S_FTExtensionOption = 622,               // FTExtensionOption
        S_FTStemOption = 623,                    // FTStemOption
        S_FTThesaurusOption = 624,               // FTThesaurusOption
        S_FTThesaurusID_or_default = 625,        // FTThesaurusID_or_default
        S_opt_FTThesaurus_list = 626,            // opt_FTThesaurus_list
        S_FTThesaurus_list = 627,                // FTThesaurus_list
        S_FTThesaurusID = 628,                   // FTThesaurusID
        S_opt_relationship = 629,                // opt_relationship
        S_opt_levels = 630,                      // opt_levels
        S_FTStopWordOption = 631,                // FTStopWordOption
        S_FTStopWords = 632,                     // FTStopWords
        S_STRING_LITERAL_list = 633,             // STRING_LITERAL_list
        S_opt_FTStopWordsInclExcl_list = 634,    // opt_FTStopWordsInclExcl_list
        S_FTStopWordsInclExcl_list = 635,        // FTStopWordsInclExcl_list
        S_FTStopWordsInclExcl = 636,             // FTStopWordsInclExcl
        S_FTLanguageOption = 637,                // FTLanguageOption
        S_FTWildCardOption = 638,                // FTWildCardOption
        S_FTContent = 639,                       // FTContent
        S_FTTimes = 640,                         // FTTimes
        S_FTRange = 641,                         // FTRange
        S_FTScope = 642,                         // FTScope
        S_FTBigUnit = 643,                       // FTBigUnit
        S_FTIgnoreOption = 644,                  // FTIgnoreOption
        S_JSONArrayConstructor = 645,            // JSONArrayConstructor
        S_JSONSimpleObjectUnion = 646,           // JSONSimpleObjectUnion
        S_JSONAccumulatorObjectUnion = 647,      // JSONAccumulatorObjectUnion
        S_JSONObjectConstructor = 648,           // JSONObjectConstructor
        S_JSONPairList = 649,                    // JSONPairList
        S_JSONInsertExpr = 650,                  // JSONInsertExpr
        S_JSONAppendExpr = 651,                  // JSONAppendExpr
        S_JSONDeleteExpr = 652,                  // JSONDeleteExpr
        S_JSONRenameExpr = 653,                  // JSONRenameExpr
        S_JSONReplaceExpr = 654,                 // JSONReplaceExpr
        S_JSONTest = 655,                        // JSONTest
        S_URI_LITERAL = 656,                     // URI_LITERAL
        S_QNAME = 657,                           // QNAME
        S_FUNCTION_NAME = 658,                   // FUNCTION_NAME
        S_EQNAME = 659                           // EQNAME
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
      yylast_ = 17222,     ///< Last index in yytable_.
      yynnts_ = 326,  ///< Number of nonterminal symbols.
      yyfinal_ = 613 ///< Termination state number.
    };


    // User arguments.
    xquery_driver& driver;

  };


} // zorba




#endif // !YY_YY_XQUERY_PARSER_HPP_INCLUDED
