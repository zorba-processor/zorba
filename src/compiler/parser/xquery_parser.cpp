// A Bison parser, made by GNU Bison 3.8.2.

// Skeleton implementation for Bison LALR(1) parsers in C++

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

// DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
// especially those whose name start with YY_ or yy_.  They are
// private implementation details that can be changed or removed.



// First part of user prologue.

#define XQUERY_PARSER


#include "common/common.h"

#ifdef WIN32
#pragma warning(disable: 4786)
#endif

#include "compiler/api/compilercb.h"
#include "compiler/parser/parse_constants.h"
#include "compiler/parser/parser_helpers.h"
#include "compiler/parser/xquery_driver.h"
#include "compiler/parsetree/parsenodes.h"
#include "store/api/update_consts.h"
#include "zorbatypes/decimal.h"
#include "zorbatypes/integer.h"

#define SYMTAB( n ) driver.symtab.get( (off_t)n )
#define SYMTAB_PUT( s ) driver.symtab.put( s )
#define LOC( p ) driver.createQueryLoc( p )

// The optional trailing arguments are the rule's other RHS semantic values:
// YYERROR skips their %destructor, so they are released here with the QName.
#define ERROR_IF_QNAME_NOT_NCNAME(qname, location, ...)           \
  do {                                                            \
    if ( ! static_cast<QName*>(qname)->is_ncname()) {             \
      error((location), "A NCName is expected, found a QName");   \
      release_hack_all( (qname) __VA_OPT__(,) __VA_ARGS__ );      \
      YYERROR;                                                    \
  }                                                               \
  } while (0);          


#define YYDEBUG 1

using namespace std;
using namespace zorba;



#ifdef XQUERY_PARSER
#include "compiler/parser/xquery_scanner.h"
#else
#include "compiler/parser/jsoniq_scanner.h"
#endif

#undef yylex
#ifdef XQUERY_PARSER
#define yylex driver.getXqueryLexer()->lex
#else
#define yylex driver.getJsoniqLexer()->lex
#endif




#include "xquery_parser.hpp"

// Second part of user prologue.

template<typename T> inline void release_hack( T *ref ) {
    if ( ref ) {
        RCHelper::addReference( ref );
        RCHelper::removeReference( ref );
    }
}
template<typename... T> inline void release_hack_all( T*... refs ) {
    ( release_hack( refs ), ... );
}

namespace {
bool convert_postfix_to_target_and_selector(
    exprnode* aPostfixExpr,
    rchandle<exprnode>* aTargetExpr,
    rchandle<exprnode>* aSelectorExpr,
    string* anError,
    bool allowArray = true)
{
  rchandle<DynamicFunctionInvocation> lDynamicFunctionInvocation =
      dynamic_cast<DynamicFunctionInvocation*>(aPostfixExpr);
  rchandle<FilterExpr> lFilterExpr = dynamic_cast<FilterExpr*>(aPostfixExpr);
  rchandle<JSONObjectLookup> lObjectLookup = dynamic_cast<JSONObjectLookup*>(aPostfixExpr);
  if (lDynamicFunctionInvocation != NULL) {
    if (lDynamicFunctionInvocation->getArgList() == NULL ||
        lDynamicFunctionInvocation->getArgList()->size() != 1)
    {
      *anError = "An object or array lookup with exactly one argument is expected. Zero or more than one argument were found.";
      return false;
    }
    *aTargetExpr = lDynamicFunctionInvocation->getPrimaryExpr(),
    *aSelectorExpr = lDynamicFunctionInvocation->getArgList()->operator[](0);
    return true;
  }
  else
  {
    *anError = "An object or array lookup is expected.";
    return false;
  }
};
}




#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> // FIXME: INFRINGES ON USER NAME SPACE.
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif


// Whether we are compiled with exception support.
#ifndef YY_EXCEPTIONS
# if defined __GNUC__ && !defined __EXCEPTIONS
#  define YY_EXCEPTIONS 0
# else
#  define YY_EXCEPTIONS 1
# endif
#endif

#define YYRHSLOC(Rhs, K) ((Rhs)[K].location)
/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

# ifndef YYLLOC_DEFAULT
#  define YYLLOC_DEFAULT(Current, Rhs, N)                               \
    do                                                                  \
      if (N)                                                            \
        {                                                               \
          (Current).begin  = YYRHSLOC (Rhs, 1).begin;                   \
          (Current).end    = YYRHSLOC (Rhs, N).end;                     \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).begin = (Current).end = YYRHSLOC (Rhs, 0).end;      \
        }                                                               \
    while (false)
# endif


// Enable debugging if requested.
#if YYDEBUG

// A pseudo ostream that takes yydebug_ into account.
# define YYCDEBUG if (yydebug_) (*yycdebug_)

# define YY_SYMBOL_PRINT(Title, Symbol)         \
  do {                                          \
    if (yydebug_)                               \
    {                                           \
      *yycdebug_ << Title << ' ';               \
      yy_print_ (*yycdebug_, Symbol);           \
      *yycdebug_ << '\n';                       \
    }                                           \
  } while (false)

# define YY_REDUCE_PRINT(Rule)          \
  do {                                  \
    if (yydebug_)                       \
      yy_reduce_print_ (Rule);          \
  } while (false)

# define YY_STACK_PRINT()               \
  do {                                  \
    if (yydebug_)                       \
      yy_stack_print_ ();                \
  } while (false)

#else // !YYDEBUG

# define YYCDEBUG if (false) std::cerr
# define YY_SYMBOL_PRINT(Title, Symbol)  YY_USE (Symbol)
# define YY_REDUCE_PRINT(Rule)           static_cast<void> (0)
# define YY_STACK_PRINT()                static_cast<void> (0)

#endif // !YYDEBUG

#define yyerrok         (yyerrstatus_ = 0)
#define yyclearin       (yyla.clear ())

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYRECOVERING()  (!!yyerrstatus_)

namespace zorba {

  /// Build a parser object.
  xquery_parser::xquery_parser (xquery_driver& driver_yyarg)
#if YYDEBUG
    : yydebug_ (false),
      yycdebug_ (&std::cerr),
#else
    :
#endif
      driver (driver_yyarg)
  {}

  xquery_parser::~xquery_parser ()
  {}

  xquery_parser::syntax_error::~syntax_error () YY_NOEXCEPT YY_NOTHROW
  {}

  /*---------.
  | symbol.  |
  `---------*/

  // basic_symbol.
  template <typename Base>
  xquery_parser::basic_symbol<Base>::basic_symbol (const basic_symbol& that)
    : Base (that)
    , value (that.value)
    , location (that.location)
  {}


  /// Constructor for valueless symbols.
  template <typename Base>
  xquery_parser::basic_symbol<Base>::basic_symbol (typename Base::kind_type t, YY_MOVE_REF (location_type) l)
    : Base (t)
    , value ()
    , location (l)
  {}

  template <typename Base>
  xquery_parser::basic_symbol<Base>::basic_symbol (typename Base::kind_type t, YY_RVREF (value_type) v, YY_RVREF (location_type) l)
    : Base (t)
    , value (YY_MOVE (v))
    , location (YY_MOVE (l))
  {}


  template <typename Base>
  xquery_parser::symbol_kind_type
  xquery_parser::basic_symbol<Base>::type_get () const YY_NOEXCEPT
  {
    return this->kind ();
  }


  template <typename Base>
  bool
  xquery_parser::basic_symbol<Base>::empty () const YY_NOEXCEPT
  {
    return this->kind () == symbol_kind::S_YYEMPTY;
  }

  template <typename Base>
  void
  xquery_parser::basic_symbol<Base>::move (basic_symbol& s)
  {
    super_type::move (s);
    value = YY_MOVE (s.value);
    location = YY_MOVE (s.location);
  }

  // by_kind.
  xquery_parser::by_kind::by_kind () YY_NOEXCEPT
    : kind_ (symbol_kind::S_YYEMPTY)
  {}

#if 201103L <= YY_CPLUSPLUS
  xquery_parser::by_kind::by_kind (by_kind&& that) YY_NOEXCEPT
    : kind_ (that.kind_)
  {
    that.clear ();
  }
#endif

  xquery_parser::by_kind::by_kind (const by_kind& that) YY_NOEXCEPT
    : kind_ (that.kind_)
  {}

  xquery_parser::by_kind::by_kind (token_kind_type t) YY_NOEXCEPT
    : kind_ (yytranslate_ (t))
  {}



  void
  xquery_parser::by_kind::clear () YY_NOEXCEPT
  {
    kind_ = symbol_kind::S_YYEMPTY;
  }

  void
  xquery_parser::by_kind::move (by_kind& that)
  {
    kind_ = that.kind_;
    that.clear ();
  }

  xquery_parser::symbol_kind_type
  xquery_parser::by_kind::kind () const YY_NOEXCEPT
  {
    return kind_;
  }


  xquery_parser::symbol_kind_type
  xquery_parser::by_kind::type_get () const YY_NOEXCEPT
  {
    return this->kind ();
  }



  // by_state.
  xquery_parser::by_state::by_state () YY_NOEXCEPT
    : state (empty_state)
  {}

  xquery_parser::by_state::by_state (const by_state& that) YY_NOEXCEPT
    : state (that.state)
  {}

  void
  xquery_parser::by_state::clear () YY_NOEXCEPT
  {
    state = empty_state;
  }

  void
  xquery_parser::by_state::move (by_state& that)
  {
    state = that.state;
    that.clear ();
  }

  xquery_parser::by_state::by_state (state_type s) YY_NOEXCEPT
    : state (s)
  {}

  xquery_parser::symbol_kind_type
  xquery_parser::by_state::kind () const YY_NOEXCEPT
  {
    if (state == empty_state)
      return symbol_kind::S_YYEMPTY;
    else
      return YY_CAST (symbol_kind_type, yystos_[+state]);
  }

  xquery_parser::stack_symbol_type::stack_symbol_type ()
  {}

  xquery_parser::stack_symbol_type::stack_symbol_type (YY_RVREF (stack_symbol_type) that)
    : super_type (YY_MOVE (that.state), YY_MOVE (that.value), YY_MOVE (that.location))
  {
#if 201103L <= YY_CPLUSPLUS
    // that is emptied.
    that.state = empty_state;
#endif
  }

  xquery_parser::stack_symbol_type::stack_symbol_type (state_type s, YY_MOVE_REF (symbol_type) that)
    : super_type (s, YY_MOVE (that.value), YY_MOVE (that.location))
  {
    // that is emptied.
    that.kind_ = symbol_kind::S_YYEMPTY;
  }

#if YY_CPLUSPLUS < 201103L
  xquery_parser::stack_symbol_type&
  xquery_parser::stack_symbol_type::operator= (const stack_symbol_type& that)
  {
    state = that.state;
    value = that.value;
    location = that.location;
    return *this;
  }

  xquery_parser::stack_symbol_type&
  xquery_parser::stack_symbol_type::operator= (stack_symbol_type& that)
  {
    state = that.state;
    value = that.value;
    location = that.location;
    // that is emptied.
    that.state = empty_state;
    return *this;
  }
#endif

  template <typename Base>
  void
  xquery_parser::yy_destroy_ (const char* yymsg, basic_symbol<Base>& yysym) const
  {
    if (yymsg)
      YY_SYMBOL_PRINT (yymsg, yysym);

    // User destructor.
    switch (yysym.kind ())
    {
      case symbol_kind::S_DECIMAL_LITERAL: // "'DECIMAL'"
                    { delete (yysym.value.decval); }
        break;

      case symbol_kind::S_DOUBLE_LITERAL: // "'DOUBLE'"
                    { delete (yysym.value.dval); }
        break;

      case symbol_kind::S_INTEGER_LITERAL: // "'INTEGER'"
                    { delete (yysym.value.ival); }
        break;

      case symbol_kind::S_VersionDecl: // VersionDecl
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_MainModule: // MainModule
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_LibraryModule: // LibraryModule
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_ModuleDecl: // ModuleDecl
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_SIND_DeclList: // SIND_DeclList
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_SIND_Decl: // SIND_Decl
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_Setter: // Setter
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_BoundarySpaceDecl: // BoundarySpaceDecl
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_DefaultCollationDecl: // DefaultCollationDecl
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_BaseURIDecl: // BaseURIDecl
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_ConstructionDecl: // ConstructionDecl
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_OrderingModeDecl: // OrderingModeDecl
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_EmptyOrderDecl: // EmptyOrderDecl
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_CopyNamespacesDecl: // CopyNamespacesDecl
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_Import: // Import
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_SchemaImport: // SchemaImport
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_URILiteralList: // URILiteralList
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_SchemaPrefix: // SchemaPrefix
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_ModuleImport: // ModuleImport
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_NamespaceDecl: // NamespaceDecl
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_DefaultNamespaceDecl: // DefaultNamespaceDecl
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_VFO_DeclList: // VFO_DeclList
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_VFO_Decl: // VFO_Decl
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_DecimalFormatDecl: // DecimalFormatDecl
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_DecimalFormatParamList: // DecimalFormatParamList
                    { delete (yysym.value.vstrpair); }
        break;

      case symbol_kind::S_DecimalFormatParam: // DecimalFormatParam
                    { delete (yysym.value.strpair); }
        break;

      case symbol_kind::S_OptionDecl: // OptionDecl
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_FTOptionDecl: // FTOptionDecl
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_CtxItemDecl: // CtxItemDecl
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_CtxItemDecl2: // CtxItemDecl2
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_CtxItemDecl3: // CtxItemDecl3
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_CtxItemDecl4: // CtxItemDecl4
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_VarDecl: // VarDecl
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_VarNameAndType: // VarNameAndType
                    { delete (yysym.value.varnametype); }
        break;

      case symbol_kind::S_AnnotationList: // AnnotationList
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_Annotation: // Annotation
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_AnnotationLiteralList: // AnnotationLiteralList
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_FunctionDecl: // FunctionDecl
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_FunctionDecl2: // FunctionDecl2
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_FunctionDeclSimple: // FunctionDeclSimple
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_FunctionDeclUpdating: // FunctionDeclUpdating
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_FunctionSig: // FunctionSig
                    { delete (yysym.value.fnsig); }
        break;

      case symbol_kind::S_ParamList: // ParamList
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_Param: // Param
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_CollectionDecl: // CollectionDecl
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_IndexDecl: // IndexDecl
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_IndexKeyList: // IndexKeyList
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_IndexKeySpec: // IndexKeySpec
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_IntegrityConstraintDecl: // IntegrityConstraintDecl
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_QueryBody: // QueryBody
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_StatementsAndOptionalExprTop: // StatementsAndOptionalExprTop
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_StatementsAndOptionalExpr: // StatementsAndOptionalExpr
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_StatementsAndExpr: // StatementsAndExpr
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_Statements: // Statements
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_Statement: // Statement
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_BlockStatement: // BlockStatement
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_BlockExpr: // BlockExpr
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_EnclosedStatementsAndOptionalExpr: // EnclosedStatementsAndOptionalExpr
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_VarDeclStatement: // VarDeclStatement
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_AssignStatement: // AssignStatement
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_ApplyStatement: // ApplyStatement
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_ExitStatement: // ExitStatement
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_WhileStatement: // WhileStatement
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_FlowCtlStatement: // FlowCtlStatement
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_FLWORStatement: // FLWORStatement
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_ReturnStatement: // ReturnStatement
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_IfStatement: // IfStatement
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_TryStatement: // TryStatement
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_CatchListStatement: // CatchListStatement
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_CatchStatement: // CatchStatement
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_VoidStatement: // VoidStatement
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_Expr: // Expr
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_ExprSingle: // ExprSingle
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_ExprSimple: // ExprSimple
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_FLWORExpr: // FLWORExpr
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_ReturnExpr: // ReturnExpr
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_FLWORWinCond: // FLWORWinCond
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_WindowClause: // WindowClause
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_CountClause: // CountClause
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_ForLetWinClause: // ForLetWinClause
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_FLWORClauseList: // FLWORClauseList
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_ForClause: // ForClause
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_VarInDeclList: // VarInDeclList
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_VarInDecl: // VarInDecl
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_PositionalVar: // PositionalVar
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_FTScoreVar: // FTScoreVar
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_LetClause: // LetClause
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_VarGetsDeclList: // VarGetsDeclList
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_VarGetsDecl: // VarGetsDecl
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_WindowVarDecl: // WindowVarDecl
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_WindowVars: // WindowVars
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_WindowVars3: // WindowVars3
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_WindowVars2: // WindowVars2
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_WhereClause: // WhereClause
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_GroupByClause: // GroupByClause
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_GroupSpecList: // GroupSpecList
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_GroupSpec: // GroupSpec
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_GroupCollationSpec: // GroupCollationSpec
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_OrderByClause: // OrderByClause
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_OrderSpecList: // OrderSpecList
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_OrderSpec: // OrderSpec
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_OrderModifier: // OrderModifier
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_OrderDirSpec: // OrderDirSpec
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_OrderEmptySpec: // OrderEmptySpec
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_OrderCollationSpec: // OrderCollationSpec
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_QuantifiedExpr: // QuantifiedExpr
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_QVarInDeclList: // QVarInDeclList
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_QVarInDecl: // QVarInDecl
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_SwitchExpr: // SwitchExpr
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_SwitchCaseClauseList: // SwitchCaseClauseList
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_SwitchCaseClause: // SwitchCaseClause
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_SwitchCaseOperandList: // SwitchCaseOperandList
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_SwitchStatement: // SwitchStatement
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_TypeswitchExpr: // TypeswitchExpr
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_TypeswitchStatement: // TypeswitchStatement
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_CaseClauseList: // CaseClauseList
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_CaseClause: // CaseClause
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_SequenceTypeList: // SequenceTypeList
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_IfExpr: // IfExpr
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_OrExpr: // OrExpr
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_AndExpr: // AndExpr
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_ComparisonExpr: // ComparisonExpr
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_FTContainsExpr: // FTContainsExpr
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_StringConcatExpr: // StringConcatExpr
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_opt_FTIgnoreOption: // opt_FTIgnoreOption
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_RangeExpr: // RangeExpr
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_AdditiveExpr: // AdditiveExpr
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_MultiplicativeExpr: // MultiplicativeExpr
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_UnionExpr: // UnionExpr
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_IntersectExceptExpr: // IntersectExceptExpr
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_InstanceofExpr: // InstanceofExpr
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_TreatExpr: // TreatExpr
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_CastableExpr: // CastableExpr
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_CastExpr: // CastExpr
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_SingleType: // SingleType
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_UnaryExpr: // UnaryExpr
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_SignList: // SignList
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_ValueExpr: // ValueExpr
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_SimpleMapExpr: // SimpleMapExpr
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_ValueComp: // ValueComp
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_NodeComp: // NodeComp
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_ValidateExpr: // ValidateExpr
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_ExtensionExpr: // ExtensionExpr
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_Pragma_list: // Pragma_list
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_Pragma: // Pragma
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_PathExpr: // PathExpr
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_RelativePathExpr: // RelativePathExpr
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_StepExpr: // StepExpr
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_AxisStep: // AxisStep
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_ForwardStep: // ForwardStep
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_ForwardAxis: // ForwardAxis
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_AbbrevForwardStep: // AbbrevForwardStep
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_ReverseStep: // ReverseStep
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_ReverseAxis: // ReverseAxis
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_NodeTest: // NodeTest
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_NameTest: // NameTest
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_Wildcard: // Wildcard
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_PostfixExpr: // PostfixExpr
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_PredicateList: // PredicateList
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_Predicate: // Predicate
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_PrimaryExpr: // PrimaryExpr
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_Literal: // Literal
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_NumericLiteral: // NumericLiteral
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_VarRef: // VarRef
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_ParenthesizedExpr: // ParenthesizedExpr
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_ContextItemExpr: // ContextItemExpr
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_OrderedExpr: // OrderedExpr
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_UnorderedExpr: // UnorderedExpr
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_FunctionCall: // FunctionCall
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_ArgList: // ArgList
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_Constructor: // Constructor
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_DirectConstructor: // DirectConstructor
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_DirElemConstructor: // DirElemConstructor
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_DirElemContentList: // DirElemContentList
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_DirAttributeList: // DirAttributeList
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_DirAttr: // DirAttr
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_DirAttributeValue: // DirAttributeValue
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_opt_QuoteAttrContentList: // opt_QuoteAttrContentList
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_QuoteAttrContentList: // QuoteAttrContentList
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_opt_AposAttrContentList: // opt_AposAttrContentList
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_AposAttrContentList: // AposAttrContentList
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_QuoteAttrValueContent: // QuoteAttrValueContent
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_AposAttrValueContent: // AposAttrValueContent
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_DirElemContent: // DirElemContent
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_CommonContent: // CommonContent
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_DirCommentConstructor: // DirCommentConstructor
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_DirPIConstructor: // DirPIConstructor
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_CDataSection: // CDataSection
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_ComputedConstructor: // ComputedConstructor
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_CompDocConstructor: // CompDocConstructor
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_CompElemConstructor: // CompElemConstructor
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_CompAttrConstructor: // CompAttrConstructor
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_CompTextConstructor: // CompTextConstructor
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_CompCommentConstructor: // CompCommentConstructor
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_CompPIConstructor: // CompPIConstructor
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_CompNamespaceConstructor: // CompNamespaceConstructor
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_TypeDeclaration: // TypeDeclaration
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_SequenceType: // SequenceType
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_OccurrenceIndicator: // OccurrenceIndicator
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_ItemType: // ItemType
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_TypeList: // TypeList
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_GeneralizedAtomicType: // GeneralizedAtomicType
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_SimpleType: // SimpleType
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_KindTest: // KindTest
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_AnyKindTest: // AnyKindTest
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_DocumentTest: // DocumentTest
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_NamespaceTest: // NamespaceTest
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_TextTest: // TextTest
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_CommentTest: // CommentTest
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_PITest: // PITest
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_AttributeTest: // AttributeTest
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_SchemaAttributeTest: // SchemaAttributeTest
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_ElementTest: // ElementTest
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_SchemaElementTest: // SchemaElementTest
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_TypeName: // TypeName
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_TypeName_WITH_HOOK: // TypeName_WITH_HOOK
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_StringLiteral: // StringLiteral
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_AnyFunctionTest: // AnyFunctionTest
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_TypedFunctionTest: // TypedFunctionTest
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_InsertExpr: // InsertExpr
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_DeleteExpr: // DeleteExpr
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_ReplaceExpr: // ReplaceExpr
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_RenameExpr: // RenameExpr
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_TransformExpr: // TransformExpr
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_VarNameList: // VarNameList
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_VarNameDecl: // VarNameDecl
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_TryExpr: // TryExpr
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_CatchListExpr: // CatchListExpr
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_CatchExpr: // CatchExpr
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_BracedExpr: // BracedExpr
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_NameTestList: // NameTestList
                    { delete (yysym.value.name_test_list); }
        break;

      case symbol_kind::S_FTSelection: // FTSelection
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_FTOr: // FTOr
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_FTAnd: // FTAnd
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_FTMildNot: // FTMildNot
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_FTUnaryNot: // FTUnaryNot
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_FTPrimaryWithOptions: // FTPrimaryWithOptions
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_opt_FTMatchOptions: // opt_FTMatchOptions
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_FTWeight: // FTWeight
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_FTPrimary: // FTPrimary
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_opt_FTTimes: // opt_FTTimes
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_FTExtensionSelection: // FTExtensionSelection
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_FTWords: // FTWords
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_FTWordsValue: // FTWordsValue
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_FTAnyallOption: // FTAnyallOption
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_FTPosFilter: // FTPosFilter
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_FTOrder: // FTOrder
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_FTWindow: // FTWindow
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_FTDistance: // FTDistance
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_FTUnit: // FTUnit
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_FTMatchOptions: // FTMatchOptions
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_FTMatchOption: // FTMatchOption
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_FTCaseOption: // FTCaseOption
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_FTDiacriticsOption: // FTDiacriticsOption
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_FTExtensionOption: // FTExtensionOption
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_FTStemOption: // FTStemOption
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_FTThesaurusOption: // FTThesaurusOption
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_FTThesaurusID: // FTThesaurusID
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_FTStopWordOption: // FTStopWordOption
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_FTStopWords: // FTStopWords
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_FTStopWordsInclExcl: // FTStopWordsInclExcl
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_FTLanguageOption: // FTLanguageOption
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_FTWildCardOption: // FTWildCardOption
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_FTContent: // FTContent
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_FTTimes: // FTTimes
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_FTRange: // FTRange
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_FTScope: // FTScope
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_FTBigUnit: // FTBigUnit
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_FTIgnoreOption: // FTIgnoreOption
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_JSONArrayConstructor: // JSONArrayConstructor
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_JSONSimpleObjectUnion: // JSONSimpleObjectUnion
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_JSONAccumulatorObjectUnion: // JSONAccumulatorObjectUnion
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_JSONObjectConstructor: // JSONObjectConstructor
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_JSONPairList: // JSONPairList
                    { release_hack( (yysym.value.node) ); }
        break;

      case symbol_kind::S_JSONInsertExpr: // JSONInsertExpr
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_JSONAppendExpr: // JSONAppendExpr
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_JSONDeleteExpr: // JSONDeleteExpr
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_JSONRenameExpr: // JSONRenameExpr
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_JSONReplaceExpr: // JSONReplaceExpr
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_QNAME: // QNAME
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_FUNCTION_NAME: // FUNCTION_NAME
                    { release_hack( (yysym.value.expr) ); }
        break;

      case symbol_kind::S_EQNAME: // EQNAME
                    { release_hack( (yysym.value.expr) ); }
        break;

      default:
        break;
    }
  }

#if YYDEBUG
  template <typename Base>
  void
  xquery_parser::yy_print_ (std::ostream& yyo, const basic_symbol<Base>& yysym) const
  {
    std::ostream& yyoutput = yyo;
    YY_USE (yyoutput);
    if (yysym.empty ())
      yyo << "empty symbol";
    else
      {
        symbol_kind_type yykind = yysym.kind ();
        yyo << (yykind < YYNTOKENS ? "token" : "nterm")
            << ' ' << yysym.name () << " ("
            << yysym.location << ": ";
        YY_USE (yykind);
        yyo << ')';
      }
  }
#endif

  void
  xquery_parser::yypush_ (const char* m, YY_MOVE_REF (stack_symbol_type) sym)
  {
    if (m)
      YY_SYMBOL_PRINT (m, sym);
    yystack_.push (YY_MOVE (sym));
  }

  void
  xquery_parser::yypush_ (const char* m, state_type s, YY_MOVE_REF (symbol_type) sym)
  {
#if 201103L <= YY_CPLUSPLUS
    yypush_ (m, stack_symbol_type (s, std::move (sym)));
#else
    stack_symbol_type ss (s, sym);
    yypush_ (m, ss);
#endif
  }

  void
  xquery_parser::yypop_ (int n) YY_NOEXCEPT
  {
    yystack_.pop (n);
  }

#if YYDEBUG
  std::ostream&
  xquery_parser::debug_stream () const
  {
    return *yycdebug_;
  }

  void
  xquery_parser::set_debug_stream (std::ostream& o)
  {
    yycdebug_ = &o;
  }


  xquery_parser::debug_level_type
  xquery_parser::debug_level () const
  {
    return yydebug_;
  }

  void
  xquery_parser::set_debug_level (debug_level_type l)
  {
    yydebug_ = l;
  }
#endif // YYDEBUG

  xquery_parser::state_type
  xquery_parser::yy_lr_goto_state_ (state_type yystate, int yysym)
  {
    int yyr = yypgoto_[yysym - YYNTOKENS] + yystate;
    if (0 <= yyr && yyr <= yylast_ && yycheck_[yyr] == yystate)
      return yytable_[yyr];
    else
      return yydefgoto_[yysym - YYNTOKENS];
  }

  bool
  xquery_parser::yy_pact_value_is_default_ (int yyvalue) YY_NOEXCEPT
  {
    return yyvalue == yypact_ninf_;
  }

  bool
  xquery_parser::yy_table_value_is_error_ (int yyvalue) YY_NOEXCEPT
  {
    return yyvalue == yytable_ninf_;
  }

  int
  xquery_parser::operator() ()
  {
    return parse ();
  }

  int
  xquery_parser::parse ()
  {
    int yyn;
    /// Length of the RHS of the rule being reduced.
    int yylen = 0;

    // Error handling.
    int yynerrs_ = 0;
    int yyerrstatus_ = 0;

    /// The lookahead symbol.
    symbol_type yyla;

    /// The locations where the error started and ended.
    stack_symbol_type yyerror_range[3];

    /// The return value of parse ().
    int yyresult;

#if YY_EXCEPTIONS
    try
#endif // YY_EXCEPTIONS
      {
    YYCDEBUG << "Starting parse\n";


    // User initialization code.
{
    yyla.location.begin.filename = yyla.location.end.filename = &(driver.theFilename2);
    static_cast<void>(yynerrs_);
}



    /* Initialize the stack.  The initial state will be set in
       yynewstate, since the latter expects the semantical and the
       location values to have been already stored, initialize these
       stacks with a primary value.  */
    yystack_.clear ();
    yypush_ (YY_NULLPTR, 0, YY_MOVE (yyla));

  /*-----------------------------------------------.
  | yynewstate -- push a new symbol on the stack.  |
  `-----------------------------------------------*/
  yynewstate:
    YYCDEBUG << "Entering state " << int (yystack_[0].state) << '\n';
    YY_STACK_PRINT ();

    // Accept?
    if (yystack_[0].state == yyfinal_)
      YYACCEPT;

    goto yybackup;


  /*-----------.
  | yybackup.  |
  `-----------*/
  yybackup:
    // Try to take a decision without lookahead.
    yyn = yypact_[+yystack_[0].state];
    if (yy_pact_value_is_default_ (yyn))
      goto yydefault;

    // Read a lookahead token.
    if (yyla.empty ())
      {
        YYCDEBUG << "Reading a token\n";
#if YY_EXCEPTIONS
        try
#endif // YY_EXCEPTIONS
          {
            yyla.kind_ = yytranslate_ (yylex (&yyla.value, &yyla.location));
          }
#if YY_EXCEPTIONS
        catch (const syntax_error& yyexc)
          {
            YYCDEBUG << "Caught exception: " << yyexc.what() << '\n';
            error (yyexc);
            goto yyerrlab1;
          }
#endif // YY_EXCEPTIONS
      }
    YY_SYMBOL_PRINT ("Next token is", yyla);

    if (yyla.kind () == symbol_kind::S_YYerror)
    {
      // The scanner already issued an error message, process directly
      // to error recovery.  But do not keep the error token as
      // lookahead, it is too special and may lead us to an endless
      // loop in error recovery. */
      yyla.kind_ = symbol_kind::S_YYUNDEF;
      goto yyerrlab1;
    }

    /* If the proper action on seeing token YYLA.TYPE is to reduce or
       to detect an error, take that action.  */
    yyn += yyla.kind ();
    if (yyn < 0 || yylast_ < yyn || yycheck_[yyn] != yyla.kind ())
      {
        goto yydefault;
      }

    // Reduce or error.
    yyn = yytable_[yyn];
    if (yyn <= 0)
      {
        if (yy_table_value_is_error_ (yyn))
          goto yyerrlab;
        yyn = -yyn;
        goto yyreduce;
      }

    // Count tokens shifted since error; after three, turn off error status.
    if (yyerrstatus_)
      --yyerrstatus_;

    // Shift the lookahead token.
    yypush_ ("Shifting", state_type (yyn), YY_MOVE (yyla));
    goto yynewstate;


  /*-----------------------------------------------------------.
  | yydefault -- do the default action for the current state.  |
  `-----------------------------------------------------------*/
  yydefault:
    yyn = yydefact_[+yystack_[0].state];
    if (yyn == 0)
      goto yyerrlab;
    goto yyreduce;


  /*-----------------------------.
  | yyreduce -- do a reduction.  |
  `-----------------------------*/
  yyreduce:
    yylen = yyr2_[yyn];
    {
      stack_symbol_type yylhs;
      yylhs.state = yy_lr_goto_state_ (yystack_[yylen].state, yyr1_[yyn]);
      /* If YYLEN is nonzero, implement the default value of the
         action: '$$ = $1'.  Otherwise, use the top of the stack.

         Otherwise, the following line sets YYLHS.VALUE to garbage.
         This behavior is undocumented and Bison users should not rely
         upon it.  */
      if (yylen)
        yylhs.value = yystack_[yylen - 1].value;
      else
        yylhs.value = yystack_[0].value;

      // Default location.
      {
        stack_type::slice range (yystack_, yylen);
        YYLLOC_DEFAULT (yylhs.location, range, yylen);
        yyerror_range[1].location = yylhs.location;
      }

      // Perform the reduction.
      YY_REDUCE_PRINT (yyn);
#if YY_EXCEPTIONS
      try
#endif // YY_EXCEPTIONS
        {
          switch (yyn)
            {
  case 2: // Module: ModuleWithoutBOM
      { (yylhs.value.node) = (yystack_[0].value.node); }
    break;

  case 3: // Module: "'BOM_UTF8'" ModuleWithoutBOM
      {
        (yylhs.value.node) = (yystack_[0].value.node);
      }
    break;

  case 4: // Module: "'#!/shebang" ModuleWithoutBOM
      {
        (yylhs.value.node) = (yystack_[0].value.node);
      }
    break;

  case 5: // Module: "'BOM_UTF8'" "'#!/shebang" ModuleWithoutBOM
      {
        (yylhs.value.node) = (yystack_[0].value.node);
      }
    break;

  case 6: // ERROR: error
      {
        (yylhs.value.node) = NULL;
      }
    break;

  case 7: // ERROR: error "'unrecognized'"
      {
        (yylhs.value.node) = NULL; YYABORT;
      }
    break;

  case 8: // ERROR: ERROR error
      {
        (yylhs.value.node) = NULL; YYABORT;
      }
    break;

  case 9: // ModuleWithoutBOM: MainModule
    {
      (yylhs.value.node) = (yystack_[0].value.node);
      driver.set_expr( (yylhs.value.node) );
    }
    break;

  case 10: // ModuleWithoutBOM: VersionDecl MainModule
    {
      MainModule* mm = dynamic_cast<MainModule*>((yystack_[0].value.node));
      mm->set_version_decl( static_cast<VersionDecl*>((yystack_[1].value.node)) );
      (yylhs.value.node) = (yystack_[0].value.node);
      driver.set_expr( (yylhs.value.node) );
    }
    break;

  case 11: // ModuleWithoutBOM: LibraryModule
    {
      (yylhs.value.node) = (yystack_[0].value.node);
      driver.set_expr( (yylhs.value.node) );
    }
    break;

  case 12: // ModuleWithoutBOM: VersionDecl LibraryModule
    {
      LibraryModule* lm = dynamic_cast<LibraryModule*>((yystack_[0].value.node));
      lm->set_version_decl( static_cast<VersionDecl*>((yystack_[1].value.node)) );
      (yylhs.value.node) = (yystack_[0].value.node);
      driver.set_expr( (yylhs.value.node) );
    }
    break;

  case 13: // VersionDecl: "'xquery'" "'version'" "'STRING'" "';'"
    {
      (yylhs.value.node) = new VersionDecl( LOC(yylhs.location), SYMTAB((yystack_[1].value.sval)), "utf-8" );
    }
    break;

  case 14: // VersionDecl: "'xquery'" "'version'" "'STRING'" "'encoding'" "'STRING'" "';'"
    {
      (yylhs.value.node) = new VersionDecl( LOC(yylhs.location), SYMTAB((yystack_[3].value.sval)), SYMTAB((yystack_[1].value.sval)) );
    }
    break;

  case 15: // MainModule: SIND_DeclList "';'" QueryBody
    {
      Prolog* prolog = new Prolog(LOC(yylhs.location), static_cast<SIND_DeclList*>((yystack_[2].value.node)), NULL);
      (yylhs.value.node) = new MainModule(LOC(yylhs.location), static_cast<QueryBody*>((yystack_[0].value.expr)), prolog);
    }
    break;

  case 16: // MainModule: VFO_DeclList "';'" QueryBody
    {
      Prolog* prolog = new Prolog(LOC(yylhs.location), NULL, static_cast<VFO_DeclList*>((yystack_[2].value.node)));
      (yylhs.value.node) = new MainModule(LOC(yylhs.location), static_cast<QueryBody*>((yystack_[0].value.expr)), prolog);
    }
    break;

  case 17: // MainModule: SIND_DeclList "';'" VFO_DeclList "';'" QueryBody
    {
      Prolog* prolog = new Prolog(LOC(yylhs.location),
                                  static_cast<SIND_DeclList*>((yystack_[4].value.node)),
                                  static_cast<VFO_DeclList*>((yystack_[2].value.node)));
      (yylhs.value.node) = new MainModule(LOC(yylhs.location), static_cast<QueryBody*>((yystack_[0].value.expr)), prolog);
    }
    break;

  case 18: // MainModule: QueryBody
    {
      (yylhs.value.node) = new MainModule( LOC(yylhs.location), static_cast<QueryBody*>((yystack_[0].value.expr)), NULL );
    }
    break;

  case 19: // MainModule: SIND_DeclList ERROR QueryBody
    {
      (yylhs.value.node) = (yystack_[2].value.node); (yylhs.value.node) = (yystack_[0].value.expr);
      yystack_[2].location.step();
      error(yystack_[2].location, "syntax error, missing semicolon \";\" after statement");
      release_hack( (yystack_[2].value.node) );
      release_hack( (yystack_[0].value.expr) );
      YYERROR;
    }
    break;

  case 20: // MainModule: VFO_DeclList ERROR QueryBody
    {
      (yylhs.value.node) = (yystack_[2].value.node); (yylhs.value.node) = (yystack_[0].value.expr);
      yystack_[2].location.step();
      error(yystack_[2].location, "syntax error, missing semicolon \";\" after declaration");
      release_hack( (yystack_[2].value.node) );
      release_hack( (yystack_[0].value.expr) );
      YYERROR;
    }
    break;

  case 21: // MainModule: SIND_DeclList "';'" VFO_DeclList ERROR QueryBody
    {
      (yylhs.value.node) = (yystack_[4].value.node); (yylhs.value.node) = (yystack_[2].value.node); (yylhs.value.node) = (yystack_[0].value.expr);
      yystack_[2].location.step();
      error(yystack_[2].location, "syntax error, missing semicolon \";\" after declaration");
      release_hack( (yystack_[4].value.node) );
      release_hack( (yystack_[2].value.node) );
      release_hack( (yystack_[0].value.expr) );
      YYERROR;
    }
    break;

  case 22: // MainModule: SIND_DeclList ERROR VFO_DeclList "';'" QueryBody
    {
      (yylhs.value.node) = (yystack_[4].value.node); (yylhs.value.node) = (yystack_[2].value.node); (yylhs.value.node) = (yystack_[0].value.expr);
      yystack_[4].location.step();
      error(yystack_[4].location, "syntax error, missing semicolon \";\" after statement");
      release_hack( (yystack_[4].value.node) );
      release_hack( (yystack_[2].value.node) );
      release_hack( (yystack_[0].value.expr) );
      YYERROR;
    }
    break;

  case 23: // LibraryModule: ModuleDecl
    {
      (yylhs.value.node) = new LibraryModule(LOC(yylhs.location), static_cast<ModuleDecl*>((yystack_[0].value.node)), NULL);
    }
    break;

  case 24: // LibraryModule: ModuleDecl SIND_DeclList "';'"
    {
      Prolog* prolog = new Prolog(LOC(yylhs.location), static_cast<SIND_DeclList*>((yystack_[1].value.node)), NULL);
      (yylhs.value.node) = new LibraryModule(LOC(yylhs.location), static_cast<ModuleDecl*>((yystack_[2].value.node)), prolog);
    }
    break;

  case 25: // LibraryModule: ModuleDecl VFO_DeclList "';'"
    {
      Prolog* prolog = new Prolog(LOC(yylhs.location), NULL, static_cast<VFO_DeclList*>((yystack_[1].value.node)));
      (yylhs.value.node) = new LibraryModule(LOC(yylhs.location), static_cast<ModuleDecl*>((yystack_[2].value.node)), prolog);
    }
    break;

  case 26: // LibraryModule: ModuleDecl SIND_DeclList "';'" VFO_DeclList "';'"
    {
      Prolog* prolog = new Prolog(LOC(yylhs.location),
                                  static_cast<SIND_DeclList*>((yystack_[3].value.node)),
                                  static_cast<VFO_DeclList*>((yystack_[1].value.node)));
      (yylhs.value.node) = new LibraryModule(LOC(yylhs.location), static_cast<ModuleDecl*>((yystack_[4].value.node)), prolog);
    }
    break;

  case 27: // ModuleDecl: "'module'" "'namespace'" QNAME "'='" URI_LITERAL "';'"
    {
      ERROR_IF_QNAME_NOT_NCNAME((yystack_[3].value.expr), yystack_[3].location);
      (yylhs.value.node) = new ModuleDecl( LOC(yylhs.location), static_cast<QName*>((yystack_[3].value.expr)), SYMTAB((yystack_[1].value.sval)) );
      dynamic_cast<ModuleDecl*>((yylhs.value.node))->setComment( SYMTAB((yystack_[5].value.sval)) );
    }
    break;

  case 28: // SIND_DeclList: SIND_Decl
    {
      SIND_DeclList *sdl = new SIND_DeclList( LOC(yylhs.location) );
      sdl->push_back( (yystack_[0].value.node) );
      (yylhs.value.node) = sdl;
    }
    break;

  case 29: // SIND_DeclList: SIND_DeclList "';'" SIND_Decl
    {
      ((SIND_DeclList*)(yystack_[2].value.node))->push_back( (yystack_[0].value.node) );
      (yylhs.value.node) = (yystack_[2].value.node);
    }
    break;

  case 30: // SIND_DeclList: SIND_DeclList ERROR SIND_Decl
    {
      (yylhs.value.node) = (yystack_[2].value.node); (yylhs.value.node) = (yystack_[0].value.node);
      yystack_[2].location.step();
      error(yystack_[2].location, "syntax error, missing semicolon \";\" after declaration");
      release_hack( (yystack_[2].value.node) );
      release_hack( (yystack_[0].value.node) );
      YYERROR;
    }
    break;

  case 31: // SIND_Decl: Setter
    { (yylhs.value.node) = (yystack_[0].value.node); }
    break;

  case 32: // SIND_Decl: Import
    { (yylhs.value.node) = (yystack_[0].value.node); }
    break;

  case 33: // SIND_Decl: NamespaceDecl
    { (yylhs.value.node) = (yystack_[0].value.node); }
    break;

  case 34: // SIND_Decl: DefaultNamespaceDecl
    { (yylhs.value.node) = (yystack_[0].value.node); }
    break;

  case 35: // SIND_Decl: FTOptionDecl
    { (yylhs.value.node) = (yystack_[0].value.node); }
    break;

  case 36: // Setter: BoundarySpaceDecl
    { (yylhs.value.node) = (yystack_[0].value.node); }
    break;

  case 37: // Setter: DefaultCollationDecl
    { (yylhs.value.node) = (yystack_[0].value.node); }
    break;

  case 38: // Setter: BaseURIDecl
    { (yylhs.value.node) = (yystack_[0].value.node); }
    break;

  case 39: // Setter: ConstructionDecl
    { (yylhs.value.node) = (yystack_[0].value.node); }
    break;

  case 40: // Setter: OrderingModeDecl
    { (yylhs.value.node) = (yystack_[0].value.node); }
    break;

  case 41: // Setter: EmptyOrderDecl
    { (yylhs.value.node) = (yystack_[0].value.node); }
    break;

  case 42: // Setter: CopyNamespacesDecl
    { (yylhs.value.node) = (yystack_[0].value.node); }
    break;

  case 43: // Setter: RevalidationDecl
    { (yylhs.value.node) = (yystack_[0].value.node); }
    break;

  case 44: // BoundarySpaceDecl: "'declare'" "'boundary-space'" "'preserve'"
    {
      (yylhs.value.node) = new BoundarySpaceDecl(LOC(yylhs.location), StaticContextConsts::preserve_space);
    }
    break;

  case 45: // BoundarySpaceDecl: "'declare'" "'boundary-space'" "'strip'"
    {
      (yylhs.value.node) = new BoundarySpaceDecl(LOC(yylhs.location), StaticContextConsts::strip_space);
    }
    break;

  case 46: // DefaultCollationDecl: "'declare'" "'default'" "'collation'" URI_LITERAL
    {
      (yylhs.value.node) = new DefaultCollationDecl( LOC(yylhs.location), SYMTAB((yystack_[0].value.sval)) );
    }
    break;

  case 47: // BaseURIDecl: "'declare'" "'base-uri'" URI_LITERAL
    {
      (yylhs.value.node) = new BaseURIDecl( LOC(yylhs.location), SYMTAB((yystack_[0].value.sval)) );
    }
    break;

  case 48: // ConstructionDecl: "'declare'" "'construction'" "'preserve'"
    {
      (yylhs.value.node) = new ConstructionDecl(LOC(yylhs.location), StaticContextConsts::cons_preserve);
    }
    break;

  case 49: // ConstructionDecl: "'declare'" "'construction'" "'strip'"
    {
      (yylhs.value.node) = new ConstructionDecl(LOC(yylhs.location), StaticContextConsts::cons_strip);
    }
    break;

  case 50: // OrderingModeDecl: "'declare'" "'ordering'" "'ordered'"
    {
      (yylhs.value.node) = new OrderingModeDecl(LOC(yylhs.location), StaticContextConsts::ordered);
    }
    break;

  case 51: // OrderingModeDecl: "'declare'" "'ordering'" "'unordered'"
    {
      (yylhs.value.node) = new OrderingModeDecl(LOC(yylhs.location), StaticContextConsts::unordered);
    }
    break;

  case 52: // EmptyOrderDecl: "'declare'" "'default'" "'order'" "'empty'" "'greatest'"
    {
      (yylhs.value.node) = new EmptyOrderDecl(LOC(yylhs.location), StaticContextConsts::empty_greatest);
    }
    break;

  case 53: // EmptyOrderDecl: "'declare'" "'default'" "'order'" "'empty'" "'least'"
    {
      (yylhs.value.node) = new EmptyOrderDecl(LOC(yylhs.location), StaticContextConsts::empty_least);
    }
    break;

  case 54: // CopyNamespacesDecl: "'declare'" "'copy-namespaces'" "'preserve'" "','" "'inherit'"
    {
      (yylhs.value.node) = new CopyNamespacesDecl(LOC(yylhs.location), true, true);
    }
    break;

  case 55: // CopyNamespacesDecl: "'declare'" "'copy-namespaces'" "'preserve'" "','" "'<no inherit>'"
    {
      (yylhs.value.node) = new CopyNamespacesDecl(LOC(yylhs.location), true, false);
    }
    break;

  case 56: // CopyNamespacesDecl: "'declare'" "'copy-namespaces'" "'<no preserve>'" "','" "'inherit'"
    {
      (yylhs.value.node) = new CopyNamespacesDecl(LOC(yylhs.location), false, true);
    }
    break;

  case 57: // CopyNamespacesDecl: "'declare'" "'copy-namespaces'" "'<no preserve>'" "','" "'<no inherit>'"
    {
      (yylhs.value.node) = new CopyNamespacesDecl(LOC(yylhs.location), false, false);
    }
    break;

  case 58: // Import: SchemaImport
    { (yylhs.value.node) = (yystack_[0].value.node); }
    break;

  case 59: // Import: ModuleImport
    { (yylhs.value.node) = (yystack_[0].value.node); }
    break;

  case 60: // Import: "'import'" "'QName'" error
    {
      (yylhs.value.node) = (yylhs.value.node);
      error(yystack_[1].location, "syntax error, \"import\" should be followed by either \"schema\" or \"module\"");
      YYERROR;
    }
    break;

  case 61: // SchemaImport: "'import'" "'schema'" URI_LITERAL
    {
      (yylhs.value.node) = new SchemaImport( LOC(yylhs.location), NULL, SYMTAB((yystack_[0].value.sval)), NULL );
    }
    break;

  case 62: // SchemaImport: "'import'" "'schema'" SchemaPrefix URI_LITERAL
    {
      (yylhs.value.node) = new SchemaImport(LOC(yylhs.location),
                            dynamic_cast<SchemaPrefix*>((yystack_[1].value.node)),
                            SYMTAB((yystack_[0].value.sval)),
                            NULL);
    }
    break;

  case 63: // SchemaImport: "'import'" "'schema'" URI_LITERAL "'at'" URILiteralList
    {
      (yylhs.value.node) = new SchemaImport(LOC(yylhs.location),
                            NULL,
                            SYMTAB((yystack_[2].value.sval)),
                            dynamic_cast<URILiteralList*>((yystack_[0].value.node)));
    }
    break;

  case 64: // SchemaImport: "'import'" "'schema'" SchemaPrefix URI_LITERAL "'at'" URILiteralList
    {
      (yylhs.value.node) = new SchemaImport(LOC(yylhs.location),
                            dynamic_cast<SchemaPrefix*>((yystack_[3].value.node)),
                            SYMTAB((yystack_[2].value.sval)),
                            dynamic_cast<URILiteralList*>((yystack_[0].value.node)));
    }
    break;

  case 65: // URILiteralList: URI_LITERAL
    {
      URILiteralList *ull = new URILiteralList( LOC(yylhs.location));
      ull->push_back( SYMTAB((yystack_[0].value.sval)) );
      (yylhs.value.node) = ull;
    }
    break;

  case 66: // URILiteralList: URILiteralList "','" URI_LITERAL
    {
      if ( URILiteralList *ull = dynamic_cast<URILiteralList*>((yystack_[2].value.node)) )
        ull->push_back( SYMTAB((yystack_[0].value.sval)) );
      (yylhs.value.node) = (yystack_[2].value.node);
    }
    break;

  case 67: // SchemaPrefix: "'namespace'" QNAME "'='"
    {
      ERROR_IF_QNAME_NOT_NCNAME((yystack_[1].value.expr), yystack_[1].location);
      (yylhs.value.node) = new SchemaPrefix( LOC(yylhs.location), static_cast<QName*>((yystack_[1].value.expr)));
    }
    break;

  case 68: // SchemaPrefix: "'default'" "'element'" "'namespace'"
    {
      (yylhs.value.node) = new SchemaPrefix( LOC(yylhs.location), true );
    }
    break;

  case 69: // ModuleImport: "'import'" "'module'" URI_LITERAL
    {
      (yylhs.value.node) = new ModuleImport(LOC(yylhs.location), SYMTAB((yystack_[0].value.sval)), NULL);
      dynamic_cast<ModuleImport *>((yylhs.value.node))->setComment(SYMTAB((yystack_[1].value.sval)));
    }
    break;

  case 70: // ModuleImport: "'import'" "'module'" "'namespace'" QNAME "'='" URI_LITERAL
    {
      ERROR_IF_QNAME_NOT_NCNAME((yystack_[2].value.expr), yystack_[2].location);
      (yylhs.value.node) = new ModuleImport(LOC(yylhs.location), static_cast<QName*>((yystack_[2].value.expr)), SYMTAB((yystack_[0].value.sval)), NULL);
      dynamic_cast<ModuleImport *>((yylhs.value.node))->setComment(SYMTAB((yystack_[4].value.sval)));
    }
    break;

  case 71: // ModuleImport: "'import'" "'module'" URI_LITERAL "'at'" URILiteralList
    {
      (yylhs.value.node) = new ModuleImport(LOC(yylhs.location),
                            SYMTAB((yystack_[2].value.sval)),
                            dynamic_cast<URILiteralList*>((yystack_[0].value.node)));
      dynamic_cast<ModuleImport *>((yylhs.value.node))->setComment(SYMTAB((yystack_[3].value.sval)));
    }
    break;

  case 72: // ModuleImport: "'import'" "'module'" "'namespace'" QNAME "'='" URI_LITERAL "'at'" URILiteralList
    {
      ERROR_IF_QNAME_NOT_NCNAME((yystack_[4].value.expr), yystack_[4].location, (yystack_[0].value.node));
      (yylhs.value.node) = new ModuleImport(LOC(yylhs.location),
                            static_cast<QName*>((yystack_[4].value.expr)),
                            SYMTAB((yystack_[2].value.sval)),
                            dynamic_cast<URILiteralList*>((yystack_[0].value.node)));
      dynamic_cast<ModuleImport *>((yylhs.value.node))->setComment(SYMTAB((yystack_[6].value.sval)));
    }
    break;

  case 73: // NamespaceDecl: "'declare'" "'namespace'" QNAME "'='" URI_LITERAL
    {
      ERROR_IF_QNAME_NOT_NCNAME((yystack_[2].value.expr), yystack_[2].location);
      (yylhs.value.node) = new NamespaceDecl( LOC(yylhs.location), static_cast<QName*>((yystack_[2].value.expr)), SYMTAB((yystack_[0].value.sval)) );
    }
    break;

  case 74: // DefaultNamespaceDecl: "'declare'" "'default'" "'element'" "'namespace'" URI_LITERAL
    {
      (yylhs.value.node) = new DefaultNamespaceDecl(LOC(yylhs.location),
                                    ParseConstants::ns_element_default,
                                    SYMTAB((yystack_[0].value.sval)));
    }
    break;

  case 75: // DefaultNamespaceDecl: "'declare'" "'default'" "'function'" "'namespace'" URI_LITERAL
    {
      (yylhs.value.node) = new DefaultNamespaceDecl(LOC(yylhs.location),
                                    ParseConstants::ns_function_default,
                                    SYMTAB((yystack_[0].value.sval)));
    }
    break;

  case 76: // VFO_DeclList: VFO_Decl
    {
      VFO_DeclList *vdl = new VFO_DeclList( LOC(yylhs.location));
      vdl->push_back( (yystack_[0].value.node) );
      (yylhs.value.node) = vdl;
    }
    break;

  case 77: // VFO_DeclList: VFO_DeclList "';'" VFO_Decl
    {
      ((VFO_DeclList*)(yystack_[2].value.node))->push_back( (yystack_[0].value.node) );
      (yylhs.value.node) = (yystack_[2].value.node);
    }
    break;

  case 78: // VFO_DeclList: VFO_DeclList ERROR VFO_Decl
    {
      (yylhs.value.node) = (yystack_[2].value.node); (yylhs.value.node) = (yystack_[0].value.node);
      yystack_[2].location.step();
      error(yystack_[2].location, "syntax error, missing semicolon \";\" after declaration");
      release_hack( (yystack_[2].value.node) );
      release_hack( (yystack_[0].value.node) );
      YYERROR;
    }
    break;

  case 79: // VFO_Decl: DecimalFormatDecl
    { (yylhs.value.node) = (yystack_[0].value.node); }
    break;

  case 80: // VFO_Decl: OptionDecl
    { (yylhs.value.node) = (yystack_[0].value.node); }
    break;

  case 81: // VFO_Decl: CtxItemDecl
    { (yylhs.value.node) = (yystack_[0].value.node); }
    break;

  case 82: // VFO_Decl: VarDecl
    { (yylhs.value.node) = (yystack_[0].value.node); }
    break;

  case 83: // VFO_Decl: FunctionDecl
    { (yylhs.value.node) = (yystack_[0].value.node); }
    break;

  case 84: // VFO_Decl: CollectionDecl
    { (yylhs.value.node) = (yystack_[0].value.node); }
    break;

  case 85: // VFO_Decl: IndexDecl
    { (yylhs.value.node) = (yystack_[0].value.node); }
    break;

  case 86: // VFO_Decl: IntegrityConstraintDecl
    { (yylhs.value.node) = (yystack_[0].value.node); }
    break;

  case 87: // DecimalFormatDecl: "'declare'" "'default'" "'decimal-format'" DecimalFormatParamList
    {
      (yylhs.value.node) = new DecimalFormatNode(LOC(yylhs.location), (yystack_[0].value.vstrpair));
      delete (yystack_[0].value.vstrpair);
    }
    break;

  case 88: // DecimalFormatDecl: "'declare'" "'decimal-format'" QNAME DecimalFormatParamList
    {
      (yylhs.value.node) = new DecimalFormatNode(LOC(yylhs.location), static_cast<QName*>((yystack_[1].value.expr)), (yystack_[0].value.vstrpair));
      delete (yystack_[0].value.vstrpair);
    }
    break;

  case 89: // DecimalFormatParamList: DecimalFormatParam
    {
      (yylhs.value.vstrpair) = new vector<string_pair_t>();
      (yylhs.value.vstrpair)->push_back( *(yystack_[0].value.strpair) );
      delete (yystack_[0].value.strpair);
    }
    break;

  case 90: // DecimalFormatParamList: DecimalFormatParamList DecimalFormatParam
    {
      (yystack_[1].value.vstrpair)->push_back( *(yystack_[0].value.strpair) );
      delete (yystack_[0].value.strpair);
      (yylhs.value.vstrpair) = (yystack_[1].value.vstrpair);
    }
    break;

  case 91: // DecimalFormatParam: DecimalFormatParamName "'='" StringLiteral
    {
      StringLiteral *sl = static_cast<StringLiteral*>((yystack_[0].value.expr));
      (yylhs.value.strpair) = new string_pair_t( (yystack_[2].value.strval), sl->get_strval().str() );
      delete sl;
    }
    break;

  case 92: // DecimalFormatParamName: "'decimal-separator'"
                        { (yylhs.value.strval) = "decimal-separator"; }
    break;

  case 93: // DecimalFormatParamName: "'digit'"
            { (yylhs.value.strval) = "digit"; }
    break;

  case 94: // DecimalFormatParamName: "'grouping-separator'"
                         { (yylhs.value.strval) = "grouping-separator"; }
    break;

  case 95: // DecimalFormatParamName: "'infinity'"
                     { (yylhs.value.strval) = "infinity"; }
    break;

  case 96: // DecimalFormatParamName: "'minus-sign'"
                 { (yylhs.value.strval) = "minus-sign"; }
    break;

  case 97: // DecimalFormatParamName: "'NaN'"
          { (yylhs.value.strval) = "NaN"; }
    break;

  case 98: // DecimalFormatParamName: "'pattern-separator'"
                        { (yylhs.value.strval) = "pattern-separator"; }
    break;

  case 99: // DecimalFormatParamName: "'percent'"
              { (yylhs.value.strval) = "percent"; }
    break;

  case 100: // DecimalFormatParamName: "'per-mille'"
                { (yylhs.value.strval) = "per-mille"; }
    break;

  case 101: // DecimalFormatParamName: "'zero-digit'"
                 { (yylhs.value.strval) = "zero-digit"; }
    break;

  case 102: // OptionDecl: "'declare'" "'option'" QNAME "'STRING'"
    {
      (yylhs.value.node) = new OptionDecl(LOC(yylhs.location), static_cast<QName*>((yystack_[1].value.expr)), SYMTAB((yystack_[0].value.sval)));
      if (static_cast<QName*>((yystack_[1].value.expr))->get_localname() == "enable")
      {
        zstring opt = SYMTAB((yystack_[0].value.sval));
        if (opt.find(":") != zstring::npos)
          opt = opt.substr(opt.find(":")+1);
        if (opt == "common-language")
          driver.enableCommonLanguage();
      }
    }
    break;

  case 103: // FTOptionDecl: "'declare'" "'ft-option'" FTMatchOptions
    {
      (yylhs.value.node) = new FTOptionDecl( LOC(yylhs.location), dynamic_cast<FTMatchOptions*>((yystack_[0].value.node)) );
    }
    break;

  case 104: // CtxItemDecl: "'declare'" "'context'" "'item'" CtxItemDecl2
    {
      (yylhs.value.node) = (yystack_[0].value.node);
    }
    break;

  case 105: // CtxItemDecl2: "'as'" ItemType CtxItemDecl3
    {
      CtxItemDecl* d = dynamic_cast<CtxItemDecl*>((yystack_[0].value.node));
      d->theType = (yystack_[1].value.node);
      (yylhs.value.node) = d;
    }
    break;

  case 106: // CtxItemDecl2: CtxItemDecl3
    {
      (yylhs.value.node) = (yystack_[0].value.node);
    }
    break;

  case 107: // CtxItemDecl3: CtxItemDecl4
    {
      CtxItemDecl* d = dynamic_cast<CtxItemDecl*>((yystack_[0].value.node));
      d->theIsExternal = false;
      (yylhs.value.node) = d;
    }
    break;

  case 108: // CtxItemDecl3: "'external'"
    {
      (yylhs.value.node) = new CtxItemDecl(LOC(yylhs.location), NULL);
    }
    break;

  case 109: // CtxItemDecl3: "'external'" CtxItemDecl4
    {
      (yylhs.value.node) = (yystack_[0].value.node);
    }
    break;

  case 110: // CtxItemDecl4: "':='" ExprSingle
    {
      (yylhs.value.node) = new CtxItemDecl(LOC(yylhs.location), (yystack_[0].value.expr));
    }
    break;

  case 111: // VarDecl: "'declare'" VarNameAndType "':='" ExprSingle
    {
      std::unique_ptr<VarNameAndType> nt(dynamic_cast<VarNameAndType *>((yystack_[2].value.varnametype)));
      (yylhs.value.node) = new GlobalVarDecl(LOC(yylhs.location),
                             nt->theName,
                             nt->theType,
                             (yystack_[0].value.expr),
                             nt->get_annotations(),
                             false);
      static_cast<GlobalVarDecl*>((yylhs.value.node))->setComment(SYMTAB((yystack_[3].value.sval)));
    }
    break;

  case 112: // VarDecl: "'declare'" VarNameAndType "'external'"
    {
      std::unique_ptr<VarNameAndType> nt(dynamic_cast<VarNameAndType *>((yystack_[1].value.varnametype)));
      (yylhs.value.node) = new GlobalVarDecl(LOC(yylhs.location),
                             nt->theName,
                             nt->theType,
                             NULL,
                             nt->get_annotations(),
                             true);
      static_cast<GlobalVarDecl*>((yylhs.value.node))->setComment(SYMTAB((yystack_[2].value.sval)));
    }
    break;

  case 113: // VarDecl: "'declare'" VarNameAndType "'external'" "':='" ExprSingle
    {
      std::unique_ptr<VarNameAndType> nt(dynamic_cast<VarNameAndType *>((yystack_[3].value.varnametype)));
      (yylhs.value.node) = new GlobalVarDecl(LOC(yylhs.location),
                             nt->theName,
                             nt->theType,
                             (yystack_[0].value.expr),
                             nt->get_annotations(),
                             true);
      static_cast<GlobalVarDecl*>((yylhs.value.node))->setComment(SYMTAB((yystack_[4].value.sval)));
    }
    break;

  case 114: // VarNameAndType: "'variable'" "'$'" QNAME
    {
      (yylhs.value.varnametype) = new VarNameAndType(LOC(yylhs.location), static_cast<QName*>((yystack_[0].value.expr)), NULL, NULL);
    }
    break;

  case 115: // VarNameAndType: "'variable'" "'$'" QNAME TypeDeclaration
    {
      (yylhs.value.varnametype) = new VarNameAndType(LOC(yylhs.location),
                              static_cast<QName*>((yystack_[1].value.expr)),
                              dynamic_cast<SequenceTypeAST *>((yystack_[0].value.node)),
                              NULL);
    }
    break;

  case 116: // VarNameAndType: AnnotationList "'variable'" "'$'" QNAME
    {
      (yylhs.value.varnametype) = new VarNameAndType(LOC(yylhs.location),
                              static_cast<QName*>((yystack_[0].value.expr)),
                              NULL,
                              static_cast<AnnotationListParsenode*>((yystack_[3].value.node)));
    }
    break;

  case 117: // VarNameAndType: AnnotationList "'variable'" "'$'" QNAME TypeDeclaration
    {
      (yylhs.value.varnametype) = new VarNameAndType(LOC(yylhs.location),
                              static_cast<QName*>((yystack_[1].value.expr)),
                              dynamic_cast<SequenceTypeAST *>((yystack_[0].value.node)),
                              static_cast<AnnotationListParsenode*>((yystack_[4].value.node)));
    }
    break;

  case 118: // AnnotationList: Annotation
    {
      (yylhs.value.node) = new AnnotationListParsenode(LOC(yylhs.location), static_cast<AnnotationParsenode*>((yystack_[0].value.node)));
    }
    break;

  case 119: // AnnotationList: AnnotationList Annotation
    {
      static_cast<AnnotationListParsenode*>((yystack_[1].value.node))->push_back(static_cast<AnnotationParsenode*>((yystack_[0].value.node)));
      (yylhs.value.node) = (yystack_[1].value.node);
    }
    break;

  case 120: // Annotation: "'%QName'"
    {
      (yylhs.value.node) = new AnnotationParsenode(LOC(yylhs.location), new QName(LOC(yylhs.location), SYMTAB((yystack_[0].value.sval))), NULL);
    }
    break;

  case 121: // Annotation: "'%QName'" "'('" AnnotationLiteralList "')'"
    {
      (yylhs.value.node) = new AnnotationParsenode(LOC(yylhs.location),
                                   new QName(LOC(yylhs.location), SYMTAB((yystack_[3].value.sval))),
                                   static_cast<AnnotationLiteralListParsenode*>((yystack_[1].value.node)));
    }
    break;

  case 122: // Annotation: "'%EQName'"
    {
      (yylhs.value.node) = new AnnotationParsenode(LOC(yylhs.location), new QName(LOC(yylhs.location), SYMTAB((yystack_[0].value.sval)), true), NULL);
    }
    break;

  case 123: // Annotation: "'%EQName'" "'('" AnnotationLiteralList "')'"
    {
      (yylhs.value.node) = new AnnotationParsenode(LOC(yylhs.location),
                                   new QName(LOC(yylhs.location), SYMTAB((yystack_[3].value.sval)), true),
                                   static_cast<AnnotationLiteralListParsenode*>((yystack_[1].value.node)));
    }
    break;

  case 124: // AnnotationLiteralList: Literal
    {
      (yylhs.value.node) = new AnnotationLiteralListParsenode(LOC(yylhs.location), (yystack_[0].value.expr));
    }
    break;

  case 125: // AnnotationLiteralList: AnnotationLiteralList "','" Literal
    {
      static_cast<AnnotationLiteralListParsenode*>((yystack_[2].value.node))->push_back((yystack_[0].value.expr));
      (yylhs.value.node) = (yystack_[2].value.node);
    }
    break;

  case 126: // FunctionDecl: "'declare'" FunctionDecl2
    {
      static_cast<FunctionDecl*>((yystack_[0].value.node))->setComment( SYMTAB((yystack_[1].value.sval)) );
      (yylhs.value.node) = (yystack_[0].value.node);
    }
    break;

  case 127: // FunctionDecl: "'declare'" AnnotationList FunctionDecl2
    {
      FunctionDecl* fdecl = static_cast<FunctionDecl*>((yystack_[0].value.node));
      fdecl->setComment( SYMTAB((yystack_[2].value.sval)) );
      fdecl->set_annotations(static_cast<AnnotationListParsenode*>((yystack_[1].value.node)));
      (yylhs.value.node) = (yystack_[0].value.node);
    }
    break;

  case 128: // FunctionDecl2: FunctionDeclSimple
    { (yylhs.value.node) = (yystack_[0].value.node); }
    break;

  case 129: // FunctionDecl2: FunctionDeclUpdating
    { (yylhs.value.node) = (yystack_[0].value.node); }
    break;

  case 130: // FunctionDeclSimple: "'function'" FUNCTION_NAME FunctionSig EnclosedStatementsAndOptionalExpr
    {
      exprnode* body = (yystack_[0].value.expr);
      if (body == NULL)
        body = new BlockBody(LOC(yystack_[0].location));
      (yylhs.value.node) = new FunctionDecl(LOC(yylhs.location),
                            static_cast<QName*>((yystack_[2].value.expr)),
                            (yystack_[1].value.fnsig)->theParams.getp(),
                            (yystack_[1].value.fnsig)->theReturnType.getp(),
                            body,
                            false,
                            false);
      delete (yystack_[1].value.fnsig);
    }
    break;

  case 131: // FunctionDeclSimple: "'function'" FUNCTION_NAME FunctionSig "'external'"
    {
      (yylhs.value.node) = new FunctionDecl(LOC (yylhs.location),
                            static_cast<QName*>((yystack_[2].value.expr)),
                            (yystack_[1].value.fnsig)->theParams.getp(),
                            (yystack_[1].value.fnsig)->theReturnType.getp(),
                            NULL,
                            false,
                            true);
      delete (yystack_[1].value.fnsig);
    }
    break;

  case 132: // FunctionDeclUpdating: "'updating'" "'function'" FUNCTION_NAME FunctionSig EnclosedStatementsAndOptionalExpr
    {
      exprnode* body = (yystack_[0].value.expr);
      if (body == NULL)
        body = new BlockBody(LOC(yystack_[0].location));
      (yylhs.value.node) = new FunctionDecl(LOC (yylhs.location),
                            static_cast<QName*>((yystack_[2].value.expr)),
                            (yystack_[1].value.fnsig)->theParams.getp(),
                            (yystack_[1].value.fnsig)->theReturnType.getp(),
                            (yystack_[0].value.expr),
                            true,
                            false);
      delete (yystack_[1].value.fnsig);
    }
    break;

  case 133: // FunctionDeclUpdating: "'updating'" "'function'" FUNCTION_NAME FunctionSig "'external'"
    {
      (yylhs.value.node) = new FunctionDecl(LOC(yylhs.location),
                            static_cast<QName*>((yystack_[2].value.expr)),
                            (yystack_[1].value.fnsig)->theParams.getp(),
                            (yystack_[1].value.fnsig)->theReturnType.getp(),
                            NULL,
                            true,
                            true);
      delete (yystack_[1].value.fnsig);
    }
    break;

  case 134: // FunctionSig: "'('" "')'"
    {
      (yylhs.value.fnsig) = new FunctionSig(NULL);
    }
    break;

  case 135: // FunctionSig: "'('" ParamList "')'"
    {
      (yylhs.value.fnsig) = new FunctionSig(dynamic_cast<ParamList*>((yystack_[1].value.node)));
    }
    break;

  case 136: // FunctionSig: "'('" "')'" "'as'" SequenceType
    {
      (yylhs.value.fnsig) = new FunctionSig(NULL, dynamic_cast<SequenceTypeAST*>((yystack_[0].value.node)));
    }
    break;

  case 137: // FunctionSig: "'('" ParamList "')'" "'as'" SequenceType
    {
      (yylhs.value.fnsig) = new FunctionSig(dynamic_cast<ParamList*>((yystack_[3].value.node)), dynamic_cast<SequenceTypeAST*>((yystack_[0].value.node)));
    }
    break;

  case 138: // ParamList: Param
    {
      ParamList *pl = new ParamList( LOC(yylhs.location) );
      pl->push_back( dynamic_cast<Param*>((yystack_[0].value.node)) );
      (yylhs.value.node) = pl;
    }
    break;

  case 139: // ParamList: ParamList "','" Param
    {
      if ( ParamList *pl = dynamic_cast<ParamList*>((yystack_[2].value.node)) )
        pl->push_back( dynamic_cast<Param*>((yystack_[0].value.node)) );
      (yylhs.value.node) = (yystack_[2].value.node);
    }
    break;

  case 140: // Param: "'$'" QNAME
    {
      (yylhs.value.node) = new Param(LOC(yylhs.location), static_cast<QName*>((yystack_[0].value.expr)), NULL);
    }
    break;

  case 141: // Param: "'$'" QNAME TypeDeclaration
    {
      (yylhs.value.node) = new Param(LOC(yylhs.location),
                     static_cast<QName*>((yystack_[1].value.expr)),
                     dynamic_cast<SequenceTypeAST *>((yystack_[0].value.node)));
    }
    break;

  case 142: // CollectionDecl: "'declare'" "'collection'" QNAME
    {
      (yylhs.value.node) = new CollectionDecl( LOC(yylhs.location),
                              static_cast<QName*>((yystack_[0].value.expr)),
                              NULL,
                              NULL);
      static_cast<CollectionDecl*>((yylhs.value.node))->setComment(SYMTAB((yystack_[2].value.sval)));
    }
    break;

  case 143: // CollectionDecl: "'declare'" "'collection'" QNAME "'as'" CollectionTypeDecl
    {
      (yylhs.value.node) = new CollectionDecl( LOC(yylhs.location),
                              static_cast<QName*>((yystack_[2].value.expr)),
                              0,
                              static_cast<SequenceTypeAST*>((yystack_[0].value.node)));
      static_cast<CollectionDecl*>((yylhs.value.node))->setComment(SYMTAB((yystack_[4].value.sval)));
    }
    break;

  case 144: // CollectionDecl: "'declare'" AnnotationList "'collection'" QNAME
    {
      (yylhs.value.node) = new CollectionDecl( LOC(yylhs.location),
                               static_cast<QName*>((yystack_[0].value.expr)),
                               static_cast<AnnotationListParsenode*>((yystack_[2].value.node)),
                               0);
      static_cast<CollectionDecl*>((yylhs.value.node))->setComment(SYMTAB((yystack_[3].value.sval)));
    }
    break;

  case 145: // CollectionDecl: "'declare'" AnnotationList "'collection'" QNAME "'as'" CollectionTypeDecl
    {
      (yylhs.value.node) = new CollectionDecl( LOC(yylhs.location),
                               static_cast<QName*>((yystack_[2].value.expr)),
                               static_cast<AnnotationListParsenode*>((yystack_[4].value.node)),
                               static_cast<SequenceTypeAST*>((yystack_[0].value.node)));
      static_cast<CollectionDecl*>((yylhs.value.node))->setComment(SYMTAB((yystack_[5].value.sval)));
    }
    break;

  case 146: // CollectionTypeDecl: KindTest
    {
      (yylhs.value.node) = static_cast<parsenode*>(new SequenceTypeAST(LOC(yylhs.location), (yystack_[0].value.node), NULL));
    }
    break;

  case 147: // CollectionTypeDecl: KindTest OccurrenceIndicator
    {
      (yylhs.value.node) = static_cast<parsenode*>(new SequenceTypeAST(LOC(yylhs.location),
                                                    (yystack_[1].value.node),
                                                    dynamic_cast<OccurrenceIndicator*>((yystack_[0].value.node))));
    }
    break;

  case 148: // CollectionTypeDecl: JSONTest
    {
      (yylhs.value.node) = static_cast<parsenode*>(new SequenceTypeAST(LOC(yylhs.location), (yystack_[0].value.node), NULL));
    }
    break;

  case 149: // CollectionTypeDecl: JSONTest OccurrenceIndicator
    {
      (yylhs.value.node) = static_cast<parsenode*>(new SequenceTypeAST(LOC(yylhs.location),
                                                    (yystack_[1].value.node),
                                                    dynamic_cast<OccurrenceIndicator*>((yystack_[0].value.node))));
    }
    break;

  case 150: // IndexDecl: "'declare'" "'index'" QNAME "'on'" "'nodes'" PathExpr "'by'" IndexKeyList
    {
      (yylhs.value.node) = new AST_IndexDecl(LOC(yylhs.location),
                             static_cast<QName*>((yystack_[5].value.expr)),
                             (yystack_[2].value.expr),
                             dynamic_cast<IndexKeyList*>((yystack_[0].value.node)),
                             NULL);
      static_cast<AST_IndexDecl*>((yylhs.value.node))->setComment( SYMTAB((yystack_[7].value.sval)) );
    }
    break;

  case 151: // IndexDecl: "'declare'" AnnotationList "'index'" QNAME "'on'" "'nodes'" PathExpr "'by'" IndexKeyList
    {
      (yylhs.value.node) = new AST_IndexDecl(LOC(yylhs.location),
                             static_cast<QName*>((yystack_[5].value.expr)),
                             (yystack_[2].value.expr),
                             dynamic_cast<IndexKeyList*>((yystack_[0].value.node)),
                             static_cast<AnnotationListParsenode*>((yystack_[7].value.node)));
      static_cast<AST_IndexDecl*>((yylhs.value.node))->setComment( SYMTAB((yystack_[8].value.sval)) );
    }
    break;

  case 152: // IndexKeyList: IndexKeySpec
    {
      IndexKeyList* keyList = new IndexKeyList(LOC(yylhs.location));
      keyList->addKeySpec(dynamic_cast<IndexKeySpec*>((yystack_[0].value.node)));
      (yylhs.value.node) = keyList;
    }
    break;

  case 153: // IndexKeyList: IndexKeyList "','" IndexKeySpec
    {
      dynamic_cast<IndexKeyList*>((yystack_[2].value.node))->addKeySpec(dynamic_cast<IndexKeySpec*>((yystack_[0].value.node)));
      (yylhs.value.node) = (yystack_[2].value.node);
    }
    break;

  case 154: // IndexKeySpec: PathExpr
    {
      (yylhs.value.node) = new IndexKeySpec(LOC(yylhs.location), (yystack_[0].value.expr), NULL, NULL);
    }
    break;

  case 155: // IndexKeySpec: PathExpr TypeDeclaration
    {
      (yylhs.value.node) = new IndexKeySpec(LOC(yylhs.location),
                            (yystack_[1].value.expr),
                            dynamic_cast<SequenceTypeAST*>((yystack_[0].value.node)),
                            NULL);
    }
    break;

  case 156: // IndexKeySpec: PathExpr OrderCollationSpec
    {
      (yylhs.value.node) = new IndexKeySpec(LOC(yylhs.location),
                            (yystack_[1].value.expr),
                            NULL,
                            dynamic_cast<OrderCollationSpec*>((yystack_[0].value.node)));
    }
    break;

  case 157: // IndexKeySpec: PathExpr TypeDeclaration OrderCollationSpec
    {
      (yylhs.value.node) = new IndexKeySpec(LOC(yylhs.location),
                            (yystack_[2].value.expr),
                            dynamic_cast<SequenceTypeAST*>((yystack_[1].value.node)),
                            dynamic_cast<OrderCollationSpec*>((yystack_[0].value.node)));
    }
    break;

  case 158: // IntegrityConstraintDecl: "'declare'" "'integrity'" "'constraint'" QNAME "'on'" "'collection'" QNAME "'$'" QNAME "'check'" ExprSingle
    {
      (yylhs.value.node) = new ICCollSimpleCheck(LOC(yylhs.location),
                                 static_cast<QName*>((yystack_[7].value.expr)),
                                 static_cast<QName*>((yystack_[4].value.expr)),
                                 static_cast<QName*>((yystack_[2].value.expr)),
                                 (yystack_[0].value.expr));
    }
    break;

  case 159: // IntegrityConstraintDecl: "'declare'" "'integrity'" "'constraint'" QNAME "'on'" "'collection'" QNAME "'node'" "'$'" QNAME "'check'" "'unique'" "'key'" PathExpr
    {
      (yylhs.value.node) = new ICCollUniqueKeyCheck(LOC(yylhs.location),
                                    static_cast<QName*>((yystack_[10].value.expr)),
                                    static_cast<QName*>((yystack_[7].value.expr)),
                                    static_cast<QName*>((yystack_[4].value.expr)),
                                    (yystack_[0].value.expr));
    }
    break;

  case 160: // IntegrityConstraintDecl: "'declare'" "'integrity'" "'constraint'" QNAME "'on'" "'collection'" QNAME "'foreach'" "'node'" "'$'" QNAME "'check'" ExprSingle
    {
      (yylhs.value.node) = new ICCollForeachNode(LOC(yylhs.location),
                                 static_cast<QName*>((yystack_[9].value.expr)),
                                 static_cast<QName*>((yystack_[6].value.expr)),
                                 static_cast<QName*>((yystack_[2].value.expr)),
                                 (yystack_[0].value.expr));
    }
    break;

  case 161: // IntegrityConstraintDecl: "'declare'" "'integrity'" "'constraint'" QNAME "'foreign'" "'key'" "'from'" "'collection'" QNAME "'node'" "'$'" QNAME "'key'" PathExpr "'to'" "'collection'" QNAME "'node'" "'$'" QNAME "'key'" PathExpr
    {
      (yylhs.value.node) = new ICForeignKey( LOC(yylhs.location),
                            static_cast<QName*>((yystack_[18].value.expr)),
                            static_cast<QName*>((yystack_[13].value.expr)),
                            static_cast<QName*>((yystack_[10].value.expr)),
                            (yystack_[8].value.expr),
                            static_cast<QName*>((yystack_[5].value.expr)),
                            static_cast<QName*>((yystack_[2].value.expr)),
                            (yystack_[0].value.expr));
    }
    break;

  case 162: // QueryBody: StatementsAndOptionalExprTop
    {
      if ((yystack_[0].value.expr) == NULL)
      {
        error(yystack_[0].location, "syntax error, unexpected end of file, the query body should not be empty");
        YYERROR;
      }
      if (dynamic_cast<BlockBody*>((yystack_[0].value.expr)) != NULL)
      {
        BlockBody* blk = static_cast<BlockBody*>((yystack_[0].value.expr));
        blk->setTopLevel(true);
      }
      (yylhs.value.expr) = new QueryBody(LOC(yylhs.location), (yystack_[0].value.expr));
    }
    break;

  case 163: // StatementsAndOptionalExprTop: StatementsAndExpr
    {
      (yylhs.value.expr) = (yystack_[0].value.expr);
    }
    break;

  case 164: // StatementsAndOptionalExprTop: Statements
    {
      (yylhs.value.expr) = (yystack_[0].value.expr);
    }
    break;

  case 165: // StatementsAndOptionalExprTop: %empty
    {
      (yylhs.value.expr) = NULL;
    }
    break;

  case 166: // StatementsAndOptionalExpr: StatementsAndExpr
    {
      (yylhs.value.expr) = (yystack_[0].value.expr);
    }
    break;

  case 167: // StatementsAndOptionalExpr: Statements
    {
      (yylhs.value.expr) = (yystack_[0].value.expr);
    }
    break;

  case 168: // StatementsAndOptionalExpr: %empty
    {
      (yylhs.value.expr) = NULL;
    }
    break;

  case 169: // StatementsAndExpr: Expr
    {
      (yylhs.value.expr) = (yystack_[0].value.expr);
    }
    break;

  case 170: // StatementsAndExpr: Statements Expr
    {
      BlockBody* blk = static_cast<BlockBody*>((yystack_[1].value.expr));
      blk->add((yystack_[0].value.expr));
      (yylhs.value.expr) = blk;
    }
    break;

  case 171: // Statements: Statement
    {
      BlockBody* blk = new BlockBody(LOC(yylhs.location));
      blk->add((yystack_[0].value.expr));
      (yylhs.value.expr) = blk;
    }
    break;

  case 172: // Statements: Statements Statement
    {
      BlockBody* blk = static_cast<BlockBody*>((yystack_[1].value.expr));
      blk->add((yystack_[0].value.expr));
      (yylhs.value.expr) = blk;
    }
    break;

  case 173: // Statements: Statements Expr ERROR Statement
    {
      (yylhs.value.expr) = (yystack_[3].value.expr);
      (yylhs.value.expr) = (yystack_[2].value.expr);
      (yylhs.value.expr) = (yystack_[0].value.expr);
      error(yystack_[1].location, "syntax error, unexpected statement (missing semicolon \";\" between statements?)");
      delete (yystack_[3].value.expr);
      delete (yystack_[2].value.expr);
      delete (yystack_[0].value.expr);
      YYERROR;
    }
    break;

  case 174: // Statement: BlockStatement
    { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 175: // Statement: VarDeclStatement
    { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 176: // Statement: AssignStatement
    { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 177: // Statement: ApplyStatement
    { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 178: // Statement: ExitStatement
    { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 179: // Statement: WhileStatement
    { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 180: // Statement: FlowCtlStatement
    { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 181: // Statement: FLWORStatement
    { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 182: // Statement: IfStatement
    { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 183: // Statement: TypeswitchStatement
    { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 184: // Statement: SwitchStatement
    { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 185: // Statement: TryStatement
    { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 186: // Statement: VoidStatement
    { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 187: // BlockStatement: "'{'" Statements "'}'"
    {
      (yylhs.value.expr) = (yystack_[1].value.expr);
    }
    break;

  case 188: // BlockStatement: "'{'" "'}'"
    {
      driver.addCommonLanguageWarning(yystack_[1].location, ZED(ZWST0009_EMPTY_BLOCK));
      (yylhs.value.expr) = new BlockBody(LOC(yylhs.location));
    }
    break;

  case 189: // BlockExpr: "'{'" StatementsAndOptionalExpr "'}'"
    {
      BlockBody* block = dynamic_cast<BlockBody*>((yystack_[1].value.expr));
      if ((yystack_[1].value.expr) == NULL)
      {
        (yylhs.value.expr) = new BlockBody(LOC(yylhs.location));
      }
      else if (block == NULL)
      {
        BlockBody* blk = new BlockBody(LOC(yylhs.location));
        blk->add((yystack_[1].value.expr));
        (yylhs.value.expr) = blk;
      }
      else
      {
        (yylhs.value.expr) = (yystack_[1].value.expr);
      }
    }
    break;

  case 190: // EnclosedStatementsAndOptionalExpr: "'{'" StatementsAndOptionalExpr "'}'"
    {
      (yylhs.value.expr) = (yystack_[1].value.expr);
    }
    break;

  case 191: // VarDeclStatement: BlockVarDeclList "';'"
    {
      (yylhs.value.expr) = (yystack_[1].value.expr);
    }
    break;

  case 192: // BlockVarDeclList: BlockVarDeclList "','" BlockVarDecl
    {
      VarDeclStmt* vdecl = static_cast<VarDeclStmt*>((yystack_[2].value.expr));
      vdecl->add((yystack_[0].value.node));
      (yylhs.value.expr) = vdecl;
    }
    break;

  case 193: // BlockVarDeclList: "'variable'" BlockVarDecl
    {
      VarDeclStmt* vdecl = new VarDeclStmt(LOC(yylhs.location), NULL);
      vdecl->add((yystack_[0].value.node));
      (yylhs.value.expr) = vdecl;
    }
    break;

  case 194: // BlockVarDeclList: AnnotationList "'variable'" BlockVarDecl
    {
      VarDeclStmt* vdecl = new VarDeclStmt(LOC(yylhs.location),
                                           static_cast<AnnotationListParsenode*>((yystack_[2].value.node)));
      vdecl->add((yystack_[0].value.node));
      (yylhs.value.expr) = vdecl;
    }
    break;

  case 195: // BlockVarDecl: "'$'" QNAME
    {
      LocalVarDecl* vd = new LocalVarDecl(LOC(yylhs.location),
                                          static_cast<QName*>((yystack_[0].value.expr)),
                                          NULL,
                                          NULL,
                                          NULL);
      (yylhs.value.node) = vd;
    }
    break;

  case 196: // BlockVarDecl: "'$'" QNAME TypeDeclaration
    {
      LocalVarDecl* vd = new LocalVarDecl(LOC(yylhs.location),
                                          static_cast<QName*>((yystack_[1].value.expr)),
                                          dynamic_cast<SequenceTypeAST*>((yystack_[0].value.node)),
                                          NULL,
                                          NULL);
      (yylhs.value.node) = vd;
    }
    break;

  case 197: // BlockVarDecl: "'$'" QNAME "':='" ExprSingle
    {
      LocalVarDecl* vd = new LocalVarDecl(LOC(yylhs.location),
                                          static_cast<QName*>((yystack_[2].value.expr)),
                                          NULL,
                                          (yystack_[0].value.expr),
                                          NULL);
      (yylhs.value.node) = vd;
    }
    break;

  case 198: // BlockVarDecl: "'$'" QNAME TypeDeclaration "':='" ExprSingle
    {
      LocalVarDecl* vd = new LocalVarDecl(LOC(yylhs.location),
                                          static_cast<QName*>((yystack_[3].value.expr)),
                                          dynamic_cast<SequenceTypeAST*>((yystack_[2].value.node)),
                                          (yystack_[0].value.expr),
                                          NULL);
      (yylhs.value.node) = vd;
    }
    break;

  case 199: // AssignStatement: "'$'" QNAME "':='" ExprSingle "';'"
    {
      (yylhs.value.expr) = new AssignExpr(LOC(yylhs.location), static_cast<QName*>((yystack_[3].value.expr)), (yystack_[1].value.expr));
    }
    break;

  case 200: // ApplyStatement: ExprSimple "';'"
    {
      (yylhs.value.expr) = new ApplyExpr(LOC(yylhs.location), (yystack_[1].value.expr));
    }
    break;

  case 201: // ExitStatement: "'exit'" "'returning'" ExprSingle "';'"
    {
      (yylhs.value.expr) = new ExitExpr(LOC(yylhs.location), (yystack_[1].value.expr));
    }
    break;

  case 202: // WhileStatement: "'while'" "'('" Expr "')'" Statement
    {
      BlockBody* bb = dynamic_cast<BlockBody *>((yystack_[0].value.expr));
      if (bb == NULL)
      {
        bb = new BlockBody((yystack_[0].value.expr)->get_location());
        bb->add((yystack_[0].value.expr));
      }
      (yylhs.value.expr) = new WhileExpr(LOC(yylhs.location), (yystack_[2].value.expr), bb);
    }
    break;

  case 203: // FlowCtlStatement: "'break'" "'loop'" "';'"
    {
      (yylhs.value.expr) = new FlowCtlStatement(LOC(yylhs.location), FlowCtlStatement::BREAK);
    }
    break;

  case 204: // FlowCtlStatement: "'continue'" "'loop'" "';'"
    {
      (yylhs.value.expr) = new FlowCtlStatement( LOC(yylhs.location), FlowCtlStatement::CONTINUE );
    }
    break;

  case 205: // FLWORStatement: FLWORClauseList ReturnStatement
    {
      ReturnExpr* re = dynamic_cast<ReturnExpr*>((yystack_[0].value.expr));
      (yylhs.value.expr) = new FLWORExpr(LOC(yylhs.location),
                         dynamic_cast<FLWORClauseList*>((yystack_[1].value.node)),
                         re->get_return_val(),
                         re->get_location(),
                         driver.theCompilerCB->theConfig.force_gflwor);
      delete (yystack_[0].value.expr);
    }
    break;

  case 206: // ReturnStatement: ReturnOrSelect Statement
    {
      exprnode* retExpr = (yystack_[0].value.expr);
      if (dynamic_cast<VarDeclStmt*>(retExpr) != NULL)
      {
        BlockBody* blk = new BlockBody(LOC(yylhs.location));
        blk->add(retExpr);
        retExpr = blk;
      }
      (yylhs.value.expr) = new ReturnExpr(LOC(yylhs.location), retExpr);
    }
    break;

  case 207: // IfStatement: "'if'" "'('" Expr "')'" "'then'" Statement "'else'" Statement
    {
      exprnode* thenExpr = (yystack_[2].value.expr);
      exprnode* elseExpr = (yystack_[0].value.expr);
      if (dynamic_cast<VarDeclStmt*>(thenExpr) != NULL)
      {
        BlockBody* blk = new BlockBody(LOC(yylhs.location));
        blk->add(thenExpr);
        thenExpr = blk;
      }
      if (dynamic_cast<VarDeclStmt*>(elseExpr) != NULL)
      {
        BlockBody* blk = new BlockBody(LOC(yylhs.location));
        blk->add(elseExpr);
        elseExpr = blk;
      }
      (yylhs.value.expr) = new IfExpr(LOC(yylhs.location), (yystack_[5].value.expr), thenExpr, elseExpr);
    }
    break;

  case 208: // TryStatement: "'try'" BlockStatement CatchListStatement
    {
      (yylhs.value.expr) = new TryExpr(LOC(yylhs.location), (yystack_[1].value.expr), (yystack_[0].value.expr));
    }
    break;

  case 209: // CatchListStatement: CatchStatement
    {
      CatchListExpr* cle = new CatchListExpr( LOC(yylhs.location) );
      cle->push_back( static_cast<CatchExpr*>((yystack_[0].value.expr)) );
      (yylhs.value.expr) = cle;
    }
    break;

  case 210: // CatchListStatement: CatchListStatement CatchStatement
    {
      CatchListExpr *cle = dynamic_cast<CatchListExpr*>((yystack_[1].value.expr));
      if ( cle )
        cle->push_back( static_cast<CatchExpr*>((yystack_[0].value.expr)) );
      (yylhs.value.expr) = (yystack_[1].value.expr);
    }
    break;

  case 211: // CatchStatement: "'catch'" NameTestList BlockStatement
    {
       (yylhs.value.expr) = new CatchExpr(LOC(yylhs.location), *(yystack_[1].value.name_test_list), (yystack_[0].value.expr));
       delete (yystack_[1].value.name_test_list);
    }
    break;

  case 212: // VoidStatement: "';'"
    {
      (yylhs.value.expr) = new BlockBody(LOC(yylhs.location));
    }
    break;

  case 213: // Expr: ExprSingle
    {
      (yylhs.value.expr) = (yystack_[0].value.expr);
    }
    break;

  case 214: // Expr: Expr "','" ExprSingle
    {
      Expr* expr = dynamic_cast<Expr*>((yystack_[2].value.expr));
      if ( !expr )
      {
        expr = new Expr( LOC(yylhs.location) );
        expr->push_back( (yystack_[2].value.expr) );
      }
      expr->push_back( (yystack_[0].value.expr) );
      (yylhs.value.expr) = expr;
    }
    break;

  case 215: // Expr: Expr ERROR ExprSingle
    {
      (yylhs.value.expr) = (yystack_[2].value.expr);
      (yylhs.value.expr) = (yystack_[0].value.expr);
      if (dynamic_cast<PathExpr*>((yystack_[2].value.expr)) == NULL)
        error(yystack_[1].location, "syntax error, unexpected expression (missing comma \",\" between expressions?)");
      delete (yystack_[2].value.expr);
      delete (yystack_[0].value.expr);
      YYERROR;
    }
    break;

  case 216: // ExprSingle: FLWORExpr
        { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 217: // ExprSingle: SwitchExpr
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 218: // ExprSingle: TypeswitchExpr
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 219: // ExprSingle: IfExpr
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 220: // ExprSingle: TryExpr
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 221: // ExprSingle: ExprSimple
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 222: // ExprSimple: OrExpr
        { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 223: // ExprSimple: QuantifiedExpr
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 224: // ExprSimple: InsertExpr
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 225: // ExprSimple: DeleteExpr
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 226: // ExprSimple: RenameExpr
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 227: // ExprSimple: ReplaceExpr
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 228: // ExprSimple: TransformExpr
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 229: // ExprSimple: JSONDeleteExpr
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 230: // ExprSimple: JSONInsertExpr
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 231: // ExprSimple: JSONRenameExpr
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 232: // ExprSimple: JSONReplaceExpr
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 233: // ExprSimple: JSONAppendExpr
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 234: // FLWORExpr: FLWORClauseList ReturnExpr
    {
      ReturnExpr *re = dynamic_cast<ReturnExpr*>((yystack_[0].value.expr));
      (yylhs.value.expr) = new FLWORExpr(LOC(yylhs.location),
                         dynamic_cast<FLWORClauseList*>((yystack_[1].value.node)),
                         re->get_return_val(),
                         re->get_location(),
                         driver.theCompilerCB->theConfig.force_gflwor);
      delete (yystack_[0].value.expr);
    }
    break;

  case 235: // ReturnExpr: ReturnOrSelect ExprSingle
    {
      (yylhs.value.expr) = new ReturnExpr( LOC(yylhs.location), (yystack_[0].value.expr) );
    }
    break;

  case 236: // ReturnOrSelect: "'return'"
    {
    }
    break;

  case 237: // WindowType: "'sliding'" "'window'"
    {
      (yylhs.value.strval) = parser::the_sliding;
    }
    break;

  case 238: // WindowType: "'tumbling'" "'window'"
    {
      (yylhs.value.strval) = parser::the_tumbling;
    }
    break;

  case 239: // FLWORWinCondType: "'start'"
    {
      (yylhs.value.strval) = parser::the_start;
    }
    break;

  case 240: // FLWORWinCondType: "'end'"
    {
      (yylhs.value.strval) = parser::the_end;
    }
    break;

  case 241: // FLWORWinCondType: "'only'" "'end'"
  {
    (yylhs.value.strval) = parser::the_only_end;
  }
    break;

  case 242: // FLWORWinCond: FLWORWinCondType WindowVars "'when'" ExprSingle
    {
      (yylhs.value.node) = new FLWORWinCond(LOC(yylhs.location),
                            dynamic_cast<WindowVars*>((yystack_[2].value.node)),
                            (yystack_[0].value.expr),
                            (yystack_[3].value.strval) == parser::the_start,
                            (yystack_[3].value.strval) == parser::the_only_end);
    }
    break;

  case 243: // FLWORWinCond: FLWORWinCondType "'when'" ExprSingle
    {
      (yylhs.value.node) = new FLWORWinCond(LOC(yylhs.location),
                            NULL,
                            (yystack_[0].value.expr),
                            (yystack_[2].value.strval) == parser::the_start,
                            (yystack_[2].value.strval) == parser::the_only_end);
    }
    break;

  case 244: // WindowClause: ForOrFrom WindowType WindowVarDecl FLWORWinCond FLWORWinCond
    {
      (yylhs.value.node) = new WindowClause (LOC (yylhs.location),
                             ((yystack_[3].value.strval) == parser::the_tumbling ?
                              WindowClause::tumbling_window :
                              WindowClause::sliding_window),
                             dynamic_cast<WindowVarDecl *> ((yystack_[2].value.node)),
                             dynamic_cast<FLWORWinCond *> ((yystack_[1].value.node)),
                             dynamic_cast<FLWORWinCond *> ((yystack_[0].value.node)));
    }
    break;

  case 245: // WindowClause: ForOrFrom WindowType WindowVarDecl FLWORWinCond
    {
      (yylhs.value.node) = new WindowClause (LOC (yylhs.location),
                             ((yystack_[2].value.strval) == parser::the_tumbling ?
                              WindowClause::tumbling_window :
                              WindowClause::sliding_window),
                             dynamic_cast<WindowVarDecl *> ((yystack_[1].value.node)),
                             dynamic_cast<FLWORWinCond *> ((yystack_[0].value.node)), NULL);
    }
    break;

  case 246: // CountClause: "'count'" "'$'" QNAME
    {
      (yylhs.value.node) = new CountClause(LOC(yylhs.location), static_cast<QName*>((yystack_[0].value.expr)));
    }
    break;

  case 247: // ForLetWinClause: ForClause
    { (yylhs.value.node) = (yystack_[0].value.node); }
    break;

  case 248: // ForLetWinClause: LetClause
    { (yylhs.value.node) = (yystack_[0].value.node); }
    break;

  case 249: // ForLetWinClause: WindowClause
    { (yylhs.value.node) = (yystack_[0].value.node); }
    break;

  case 250: // FLWORClause: ForLetWinClause
    { (yylhs.value.node) = (yystack_[0].value.node); }
    break;

  case 251: // FLWORClause: WhereClause
    { (yylhs.value.node) = (yystack_[0].value.node); }
    break;

  case 252: // FLWORClause: OrderByClause
    { (yylhs.value.node) = (yystack_[0].value.node); }
    break;

  case 253: // FLWORClause: GroupByClause
    { (yylhs.value.node) = (yystack_[0].value.node); }
    break;

  case 254: // FLWORClause: CountClause
    { (yylhs.value.node) = (yystack_[0].value.node); }
    break;

  case 255: // FLWORClauseList: ForLetWinClause
    {
      FLWORClauseList *fcl = new FLWORClauseList( LOC(yylhs.location) );
      fcl->push_back( dynamic_cast<FLWORClause*>((yystack_[0].value.node)) );
      (yylhs.value.node) = fcl;
    }
    break;

  case 256: // FLWORClauseList: FLWORClauseList FLWORClause
    {
      FLWORClauseList *fcl = dynamic_cast<FLWORClauseList*>((yystack_[1].value.node));
      fcl->push_back( dynamic_cast<FLWORClause*>((yystack_[0].value.node)) );
      (yylhs.value.node) = fcl;
    }
    break;

  case 257: // ForClause: ForOrFrom "'$'" VarInDeclList
    {
      (yylhs.value.node) = new ForClause(LOC(yylhs.location), dynamic_cast<VarInDeclList*>((yystack_[0].value.node)));
    }
    break;

  case 258: // ForClause: "'for'" error VarInDeclList
    {
      (yylhs.value.node) = (yystack_[0].value.node);
      error(yystack_[1].location, "syntax error, unexpected qualified name \""
          + static_cast<VarInDeclList*>((yystack_[0].value.node))->operator[](0)->get_var_name()->get_qname().str() + "\" (missing \"$\" sign?)");
      delete (yystack_[0].value.node);
      YYERROR;
    }
    break;

  case 259: // ForClause: ForOrFrom "'unrecognized'"
    {
      (yylhs.value.node) = NULL;
      error(yystack_[0].location, "");
      YYERROR;
    }
    break;

  case 260: // ForOrFrom: "'for'"
    {
    }
    break;

  case 261: // VarInDeclList: VarInDecl
    {
      VarInDeclList* vdl = new VarInDeclList( LOC(yylhs.location) );
      vdl->push_back( dynamic_cast<VarInDecl*>((yystack_[0].value.node)) );
      (yylhs.value.node) = vdl;
    }
    break;

  case 262: // VarInDeclList: VarInDeclList "','" "'$'" VarInDecl
    {
      if ( VarInDeclList* vdl = dynamic_cast<VarInDeclList*>((yystack_[3].value.node)) )
        vdl->push_back( dynamic_cast<VarInDecl*>((yystack_[0].value.node)) );
      (yylhs.value.node) = (yystack_[3].value.node);
    }
    break;

  case 263: // VarInDeclList: VarInDeclList "','" VarInDecl
    {
      (yylhs.value.node) = (yystack_[2].value.node);
      error(yystack_[0].location, "syntax error, unexpected QName \""
          + static_cast<VarInDecl*>((yystack_[0].value.node))->get_var_name()->get_qname().str() + "\" (missing \"$\" sign?)");
      delete (yystack_[2].value.node);
      release_hack( (yystack_[0].value.node) );
      YYERROR;
    }
    break;

  case 264: // VarInDecl: QNAME "'in'" ExprSingle
    {
      (yylhs.value.node) = new VarInDecl(LOC(yylhs.location),
                         static_cast<QName*>((yystack_[2].value.expr)),
                         NULL,
                         NULL,
                         NULL,
                         (yystack_[0].value.expr),
                         false);
    }
    break;

  case 265: // VarInDecl: QNAME "'allowing'" "'empty'" "'in'" ExprSingle
    {
      (yylhs.value.node) = new VarInDecl(LOC(yylhs.location),
                         static_cast<QName*>((yystack_[4].value.expr)),
                         NULL,
                         NULL,
                         NULL,
                         (yystack_[0].value.expr),
                         true);
    }
    break;

  case 266: // VarInDecl: QNAME TypeDeclaration "'in'" ExprSingle
    {
      (yylhs.value.node) = new VarInDecl(LOC(yylhs.location),
                         static_cast<QName*>((yystack_[3].value.expr)),
                         dynamic_cast<SequenceTypeAST *>((yystack_[2].value.node)),
                         NULL,
                         NULL,
                         (yystack_[0].value.expr),
                         false);
    }
    break;

  case 267: // VarInDecl: QNAME TypeDeclaration "'allowing'" "'empty'" "'in'" ExprSingle
    {
      (yylhs.value.node) = new VarInDecl(LOC(yylhs.location),
                         static_cast<QName*>((yystack_[5].value.expr)),
                         dynamic_cast<SequenceTypeAST *>((yystack_[4].value.node)),
                         NULL,
                         NULL,
                         (yystack_[0].value.expr),
                         true);
    }
    break;

  case 268: // VarInDecl: QNAME PositionalVar "'in'" ExprSingle
    {
      (yylhs.value.node) = new VarInDecl(LOC(yylhs.location),
                         static_cast<QName*>((yystack_[3].value.expr)),
                         NULL,
                         dynamic_cast<PositionalVar*>((yystack_[2].value.node)),
                         NULL,
                         (yystack_[0].value.expr),
                         false);
    }
    break;

  case 269: // VarInDecl: QNAME "'allowing'" "'empty'" PositionalVar "'in'" ExprSingle
    {
      (yylhs.value.node) = new VarInDecl(LOC(yylhs.location),
                         static_cast<QName*>((yystack_[5].value.expr)),
                         NULL,
                         dynamic_cast<PositionalVar*>((yystack_[2].value.node)),
                         NULL,
                         (yystack_[0].value.expr),
                         true);
    }
    break;

  case 270: // VarInDecl: QNAME TypeDeclaration PositionalVar "'in'" ExprSingle
    {
      (yylhs.value.node) = new VarInDecl(LOC(yylhs.location),
                         static_cast<QName*>((yystack_[4].value.expr)),
                         dynamic_cast<SequenceTypeAST *>((yystack_[3].value.node)),
                         dynamic_cast<PositionalVar*>((yystack_[2].value.node)),
                         NULL,
                         (yystack_[0].value.expr),
                         false);
    }
    break;

  case 271: // VarInDecl: QNAME TypeDeclaration "'allowing'" "'empty'" PositionalVar "'in'" ExprSingle
    {
      (yylhs.value.node) = new VarInDecl(LOC(yylhs.location),
                         static_cast<QName*>((yystack_[6].value.expr)),
                         dynamic_cast<SequenceTypeAST *>((yystack_[5].value.node)),
                         dynamic_cast<PositionalVar*>((yystack_[2].value.node)),
                         NULL,
                         (yystack_[0].value.expr),
                         true);
    }
    break;

  case 272: // VarInDecl: QNAME FTScoreVar "'in'" ExprSingle
    {
      (yylhs.value.node) = new VarInDecl(LOC(yylhs.location),
                         static_cast<QName*>((yystack_[3].value.expr)),
                         NULL,NULL,
                         dynamic_cast<FTScoreVar*>((yystack_[2].value.node)),
                         (yystack_[0].value.expr),
                         false);
    }
    break;

  case 273: // VarInDecl: QNAME TypeDeclaration FTScoreVar "'in'" ExprSingle
    {
      (yylhs.value.node) = new VarInDecl(LOC(yylhs.location),
                         static_cast<QName*>((yystack_[4].value.expr)),
                         dynamic_cast<SequenceTypeAST *>((yystack_[3].value.node)),
                         NULL,
                         dynamic_cast<FTScoreVar*>((yystack_[2].value.node)),
                         (yystack_[0].value.expr),
                         false);
    }
    break;

  case 274: // VarInDecl: QNAME PositionalVar FTScoreVar "'in'" ExprSingle
    {
      (yylhs.value.node) = new VarInDecl(LOC (yylhs.location),
                         static_cast<QName*>((yystack_[4].value.expr)),
                         NULL,
                         dynamic_cast<PositionalVar*>((yystack_[3].value.node)),
                         dynamic_cast<FTScoreVar*>((yystack_[2].value.node)),
                         (yystack_[0].value.expr),
                         false);
    }
    break;

  case 275: // VarInDecl: QNAME TypeDeclaration PositionalVar FTScoreVar "'in'" ExprSingle
    {
      (yylhs.value.node) = new VarInDecl(LOC (yylhs.location),
                         static_cast<QName*>((yystack_[5].value.expr)),
                         dynamic_cast<SequenceTypeAST *>((yystack_[4].value.node)),
                         dynamic_cast<PositionalVar*>((yystack_[3].value.node)),
                         dynamic_cast<FTScoreVar*>((yystack_[2].value.node)),
                         (yystack_[0].value.expr),
                         false);
    }
    break;

  case 276: // PositionalVar: "'at'" "'$'" QNAME
    {
      (yylhs.value.node) = new PositionalVar(LOC(yylhs.location), static_cast<QName*>((yystack_[0].value.expr)));
    }
    break;

  case 277: // FTScoreVar: "'score'" "'$'" QNAME
    {
      (yylhs.value.node) = new FTScoreVar(LOC(yylhs.location), static_cast<QName*>((yystack_[0].value.expr)));
    }
    break;

  case 278: // LetClause: "'let'" VarGetsDeclList
        {
            (yylhs.value.node) = new LetClause( LOC(yylhs.location), dynamic_cast<VarGetsDeclList*>((yystack_[0].value.node)) );
        }
    break;

  case 279: // VarGetsDeclList: VarGetsDecl
        {
            VarGetsDeclList *vgdl = new VarGetsDeclList( LOC(yylhs.location) );
            vgdl->push_back( dynamic_cast<VarGetsDecl*>((yystack_[0].value.node)) );
            (yylhs.value.node) = vgdl;
        }
    break;

  case 280: // VarGetsDeclList: VarGetsDeclList "','" VarGetsDecl
        {
            if( VarGetsDeclList *vgdl = dynamic_cast<VarGetsDeclList*>((yystack_[2].value.node)) )
                vgdl->push_back( dynamic_cast<VarGetsDecl*>((yystack_[0].value.node)) );
            (yylhs.value.node) = (yystack_[2].value.node);
        }
    break;

  case 281: // VarGetsDecl: "'$'" QNAME "':='" ExprSingle
    {
      (yylhs.value.node) = new VarGetsDecl(LOC (yylhs.location),
                           static_cast<QName*>((yystack_[2].value.expr)),
                           NULL,
                           NULL,
                           (yystack_[0].value.expr));
    }
    break;

  case 282: // VarGetsDecl: "'$'" QNAME TypeDeclaration "':='" ExprSingle
    {
      (yylhs.value.node) = new VarGetsDecl(LOC (yylhs.location),
                           static_cast<QName*>((yystack_[3].value.expr)),
                           dynamic_cast<SequenceTypeAST *>((yystack_[2].value.node)),
                           NULL,
                           (yystack_[0].value.expr));
    }
    break;

  case 283: // VarGetsDecl: FTScoreVar "':='" ExprSingle
    {
      (yylhs.value.node) = new VarGetsDecl(LOC (yylhs.location),
                           dynamic_cast<FTScoreVar*>((yystack_[2].value.node))->get_var_name(),
                           NULL,
                           dynamic_cast<FTScoreVar*>((yystack_[2].value.node)),
                           (yystack_[0].value.expr));
     }
    break;

  case 284: // VarGetsDecl: "'$'" QNAME TypeDeclaration FTScoreVar "':='" ExprSingle
    {
      (yylhs.value.node) = new VarGetsDecl(LOC (yylhs.location),
                           static_cast<QName*>((yystack_[4].value.expr)),
                           dynamic_cast<SequenceTypeAST *>((yystack_[3].value.node)),
                           dynamic_cast<FTScoreVar*>((yystack_[2].value.node)),
                           (yystack_[0].value.expr));
    }
    break;

  case 285: // WindowVarDecl: "'$'" QNAME "'in'" ExprSingle
    {
      (yylhs.value.node) = new WindowVarDecl(LOC (yylhs.location),
                             static_cast<QName*>((yystack_[2].value.expr)),
                             NULL, (yystack_[0].value.expr));
    }
    break;

  case 286: // WindowVarDecl: "'$'" QNAME TypeDeclaration "'in'" ExprSingle
    {
      (yylhs.value.node) = new WindowVarDecl(LOC (yylhs.location),
                             static_cast<QName*>((yystack_[3].value.expr)),
                             dynamic_cast<SequenceTypeAST *>((yystack_[2].value.node)),
                             (yystack_[0].value.expr));
    }
    break;

  case 287: // WindowVars: WindowVars3
    { (yylhs.value.node) = (yystack_[0].value.node); }
    break;

  case 288: // WindowVars: "'$'" QNAME
    {
      (yylhs.value.node) = new WindowVars(LOC(yylhs.location), NULL, static_cast<QName*>((yystack_[0].value.expr)), NULL, NULL);
    }
    break;

  case 289: // WindowVars: "'$'" QNAME WindowVars3
    {
      (yylhs.value.node) = (yystack_[0].value.node);
      dynamic_cast<WindowVars *>((yylhs.value.node))->set_curr(static_cast<QName*>((yystack_[1].value.expr)));
    }
    break;

  case 290: // WindowVars3: PositionalVar
    {
      (yylhs.value.node) = new WindowVars(LOC(yylhs.location), dynamic_cast<PositionalVar*>((yystack_[0].value.node)), NULL, NULL, NULL);
    }
    break;

  case 291: // WindowVars3: PositionalVar WindowVars2
    {
      (yylhs.value.node) = (yystack_[0].value.node);
      dynamic_cast<WindowVars *>((yylhs.value.node))->set_posvar(dynamic_cast<PositionalVar*>((yystack_[1].value.node)));
    }
    break;

  case 292: // WindowVars3: WindowVars2
    { (yylhs.value.node) = (yystack_[0].value.node); }
    break;

  case 293: // WindowVars2: "'previous'" "'$'" QNAME "'next'" "'$'" QNAME
    {
      (yylhs.value.node) = new WindowVars(LOC(yylhs.location), NULL, NULL, static_cast<QName*>((yystack_[3].value.expr)), static_cast<QName*>((yystack_[0].value.expr)));
    }
    break;

  case 294: // WindowVars2: "'next'" "'$'" QNAME
    {
      (yylhs.value.node) = new WindowVars(LOC(yylhs.location), NULL, NULL, NULL, static_cast<QName*>((yystack_[0].value.expr)));
    }
    break;

  case 295: // WindowVars2: "'previous'" "'$'" QNAME
    {
      (yylhs.value.node) = new WindowVars(LOC(yylhs.location), NULL, NULL, static_cast<QName*>((yystack_[0].value.expr)), NULL);
    }
    break;

  case 296: // WhereClause: "'where'" ExprSingle
    {
      (yylhs.value.node) = new WhereClause(LOC (yylhs.location), (yystack_[0].value.expr));
    }
    break;

  case 297: // GroupByClause: "'group'" "'by'" GroupSpecList
    {
      (yylhs.value.node) = new GroupByClause(LOC(yylhs.location), dynamic_cast<GroupSpecList*>((yystack_[0].value.node)));
    }
    break;

  case 298: // GroupSpecList: GroupSpec
    {
      GroupSpecList* gsl = new GroupSpecList(LOC(yylhs.location));
      gsl->push_back(static_cast<GroupSpec*>((yystack_[0].value.node)));
      (yylhs.value.node) = gsl;
    }
    break;

  case 299: // GroupSpecList: GroupSpecList "','" GroupSpec
    {
      GroupSpecList* gsl = static_cast<GroupSpecList*>((yystack_[2].value.node));
      gsl->push_back(static_cast<GroupSpec*>((yystack_[0].value.node)));
      (yylhs.value.node) = gsl;
    }
    break;

  case 300: // GroupSpec: "'$'" QNAME "':='" ExprSingle
    {
      (yylhs.value.node) = new GroupSpec(LOC(yylhs.location), static_cast<QName*>((yystack_[2].value.expr)), NULL, (yystack_[0].value.expr), NULL);
    }
    break;

  case 301: // GroupSpec: "'$'" QNAME TypeDeclaration "':='" ExprSingle
    {
      (yylhs.value.node) = new GroupSpec(LOC(yylhs.location),
                         static_cast<QName*>((yystack_[3].value.expr)),
                         static_cast<SequenceTypeAST*>((yystack_[2].value.node)),
                         (yystack_[0].value.expr),
                         NULL);
    }
    break;

  case 302: // GroupSpec: "'$'" QNAME TypeDeclaration "':='" ExprSingle GroupCollationSpec
    {
      (yylhs.value.node) = new GroupSpec(LOC(yylhs.location),
                         static_cast<QName*>((yystack_[4].value.expr)),
                         static_cast<SequenceTypeAST*>((yystack_[3].value.node)),
                         (yystack_[1].value.expr),
                         static_cast<GroupCollationSpec*>((yystack_[0].value.node)));
    }
    break;

  case 303: // GroupSpec: "'$'" QNAME "':='" ExprSingle GroupCollationSpec
    {
      (yylhs.value.node) = new GroupSpec(LOC(yylhs.location),
                         static_cast<QName*>((yystack_[3].value.expr)),
                         NULL,
                         (yystack_[1].value.expr),
                         static_cast<GroupCollationSpec*>((yystack_[0].value.node)));
    }
    break;

  case 304: // GroupSpec: ExprSingle
    {
      VarRef* varRef = dynamic_cast<VarRef*>((yystack_[0].value.expr));
      if (varRef != NULL)
        (yylhs.value.node) = new GroupSpec(LOC(yylhs.location), varRef, NULL, NULL, NULL);
      else
        (yylhs.value.node) = new GroupSpec(LOC(yylhs.location), NULL, NULL, (yystack_[0].value.expr), NULL);
    }
    break;

  case 305: // GroupSpec: ExprSingle GroupCollationSpec
    {
      VarRef* varRef = dynamic_cast<VarRef*>((yystack_[1].value.expr));
      if (varRef != NULL)
        (yylhs.value.node) = new GroupSpec(LOC(yylhs.location), varRef, NULL, NULL, static_cast<GroupCollationSpec*>((yystack_[0].value.node)));
      else
        (yylhs.value.node) = new GroupSpec(LOC(yylhs.location), NULL, NULL, (yystack_[1].value.expr), static_cast<GroupCollationSpec*>((yystack_[0].value.node)));
    }
    break;

  case 306: // GroupCollationSpec: "'collation'" URI_LITERAL
    {
      (yylhs.value.node) = new GroupCollationSpec( LOC(yylhs.location), SYMTAB((yystack_[0].value.sval)) );
    }
    break;

  case 307: // OrderByClause: "'order'" "'by'" OrderSpecList
        {
            (yylhs.value.node) = new OrderByClause(
                LOC(yylhs.location), dynamic_cast<OrderSpecList*>((yystack_[0].value.node))
            );
        }
    break;

  case 308: // OrderByClause: "'stable'" "'order'" "'by'" OrderSpecList
        {
            (yylhs.value.node) = new OrderByClause(
                LOC(yylhs.location), dynamic_cast<OrderSpecList*>((yystack_[0].value.node)), true
            );
        }
    break;

  case 309: // OrderSpecList: OrderSpec
        {
            OrderSpecList *osl = new OrderSpecList( LOC(yylhs.location) );
            osl->push_back( dynamic_cast<OrderSpec*>((yystack_[0].value.node)) );
            (yylhs.value.node) = osl;
        }
    break;

  case 310: // OrderSpecList: OrderSpecList "','" OrderSpec
        {
            if ( OrderSpecList* osl = dynamic_cast<OrderSpecList*>((yystack_[2].value.node)) )
                osl->push_back( dynamic_cast<OrderSpec*>((yystack_[0].value.node)) );
            (yylhs.value.node) = (yystack_[2].value.node);
        }
    break;

  case 311: // OrderSpec: ExprSingle
        {
            (yylhs.value.node) = new OrderSpec( LOC(yylhs.location), (yystack_[0].value.expr), NULL );
        }
    break;

  case 312: // OrderSpec: ExprSingle OrderModifier
        {
            (yylhs.value.node) = new OrderSpec(
                LOC(yylhs.location), (yystack_[1].value.expr), dynamic_cast<OrderModifierPN*>((yystack_[0].value.node))
            );
        }
    break;

  case 313: // OrderModifier: OrderDirSpec
        {
            (yylhs.value.node) = new OrderModifierPN(
                LOC(yylhs.location), dynamic_cast<OrderDirSpec*>((yystack_[0].value.node)), NULL, NULL
            );
        }
    break;

  case 314: // OrderModifier: OrderEmptySpec
        {
            (yylhs.value.node) = new OrderModifierPN(
                LOC(yylhs.location), NULL, dynamic_cast<OrderEmptySpec*>((yystack_[0].value.node)), NULL
            );
        }
    break;

  case 315: // OrderModifier: OrderCollationSpec
        {
            (yylhs.value.node) = new OrderModifierPN(
                LOC(yylhs.location), NULL, NULL, dynamic_cast<OrderCollationSpec*>((yystack_[0].value.node))
            );
        }
    break;

  case 316: // OrderModifier: OrderDirSpec OrderEmptySpec
        {
            (yylhs.value.node) = new OrderModifierPN(
                LOC(yylhs.location),
                dynamic_cast<OrderDirSpec*>((yystack_[1].value.node)),
                dynamic_cast<OrderEmptySpec*>((yystack_[0].value.node)),
                NULL
            );
        }
    break;

  case 317: // OrderModifier: OrderDirSpec OrderCollationSpec
        {
            (yylhs.value.node) = new OrderModifierPN(
                LOC(yylhs.location),
                dynamic_cast<OrderDirSpec*>((yystack_[1].value.node)),
                NULL,
                dynamic_cast<OrderCollationSpec*>((yystack_[0].value.node))
            );
        }
    break;

  case 318: // OrderModifier: OrderEmptySpec OrderCollationSpec
        {
            (yylhs.value.node) = new OrderModifierPN(
                LOC(yylhs.location),
                NULL,
                dynamic_cast<OrderEmptySpec*>((yystack_[1].value.node)),
                dynamic_cast<OrderCollationSpec*>((yystack_[0].value.node))
            );
        }
    break;

  case 319: // OrderModifier: OrderDirSpec OrderEmptySpec OrderCollationSpec
        {
            (yylhs.value.node) = new OrderModifierPN(
                LOC(yylhs.location),
                dynamic_cast<OrderDirSpec*>((yystack_[2].value.node)),
                dynamic_cast<OrderEmptySpec*>((yystack_[1].value.node)),
                dynamic_cast<OrderCollationSpec*>((yystack_[0].value.node))
            );
        }
    break;

  case 320: // OrderDirSpec: "'ascending'"
        {
            (yylhs.value.node) = new OrderDirSpec( LOC(yylhs.location), ParseConstants::dir_ascending );
        }
    break;

  case 321: // OrderDirSpec: "'descending'"
        {
            (yylhs.value.node) = new OrderDirSpec( LOC(yylhs.location), ParseConstants::dir_descending );
        }
    break;

  case 322: // OrderEmptySpec: "'empty'" "'greatest'"
        {
            (yylhs.value.node) = new OrderEmptySpec(
                LOC(yylhs.location), StaticContextConsts::empty_greatest
            );
        }
    break;

  case 323: // OrderEmptySpec: "'empty'" "'least'"
        {
            (yylhs.value.node) = new OrderEmptySpec(
                LOC(yylhs.location), StaticContextConsts::empty_least
            );
        }
    break;

  case 324: // OrderCollationSpec: "'collation'" URI_LITERAL
        {
            (yylhs.value.node) = new OrderCollationSpec( LOC(yylhs.location), SYMTAB((yystack_[0].value.sval)) );
        }
    break;

  case 325: // QuantifiedExpr: "'some'" "'$'" QVarInDeclList "'satisfies'" ExprSingle
        {
            (yylhs.value.expr) = new QuantifiedExpr(
                LOC(yylhs.location),
                ParseConstants::quant_some,
                dynamic_cast<QVarInDeclList*>((yystack_[2].value.node)),
                (yystack_[0].value.expr)
            );
        }
    break;

  case 326: // QuantifiedExpr: "'every'" "'$'" QVarInDeclList "'satisfies'" ExprSingle
        {
            (yylhs.value.expr) = new QuantifiedExpr(
                LOC(yylhs.location),
                ParseConstants::quant_every,
                dynamic_cast<QVarInDeclList*>((yystack_[2].value.node)),
                (yystack_[0].value.expr)
            );
        }
    break;

  case 327: // QVarInDeclList: QVarInDecl
    {
      QVarInDeclList *qvidl = new QVarInDeclList( LOC(yylhs.location) );
      qvidl->push_back( dynamic_cast<QVarInDecl*>((yystack_[0].value.node)) );
      (yylhs.value.node) = qvidl;
    }
    break;

  case 328: // QVarInDeclList: QVarInDeclList "','" "'$'" QVarInDecl
    {
      QVarInDeclList *qvidl = dynamic_cast<QVarInDeclList*>((yystack_[3].value.node));
      qvidl->push_back( dynamic_cast<QVarInDecl*>((yystack_[0].value.node)) );
      (yylhs.value.node) = (yystack_[3].value.node);
    }
    break;

  case 329: // QVarInDecl: QNAME "'in'" ExprSingle
    {
      (yylhs.value.node) = new QVarInDecl(LOC(yylhs.location), static_cast<QName*>((yystack_[2].value.expr)), (yystack_[0].value.expr));
    }
    break;

  case 330: // QVarInDecl: QNAME TypeDeclaration "'in'" ExprSingle
    {
      (yylhs.value.node) = new QVarInDecl(LOC(yylhs.location),
                          static_cast<QName*>((yystack_[3].value.expr)),
                          dynamic_cast<SequenceTypeAST *>((yystack_[2].value.node)),
                          (yystack_[0].value.expr));
    }
    break;

  case 331: // SwitchExpr: "'switch'" "'('" Expr "')'" SwitchCaseClauseList "'default'" ReturnOrSelect ExprSingle
    {
      (yylhs.value.expr) = new SwitchExpr(LOC(yylhs.location), (yystack_[5].value.expr), static_cast<SwitchCaseClauseList*>((yystack_[3].value.node)), (yystack_[0].value.expr));
    }
    break;

  case 332: // SwitchCaseClauseList: SwitchCaseClause
    {
      SwitchCaseClauseList* scc_list_p = new SwitchCaseClauseList(LOC(yylhs.location));
      scc_list_p->push_back(static_cast<SwitchCaseClause*>((yystack_[0].value.node)));
      (yylhs.value.node) = scc_list_p;
    }
    break;

  case 333: // SwitchCaseClauseList: SwitchCaseClauseList SwitchCaseClause
    {
      SwitchCaseClauseList* scc_list_p = static_cast<SwitchCaseClauseList*>((yystack_[1].value.node));
      scc_list_p->push_back(static_cast<SwitchCaseClause*>((yystack_[0].value.node)));
      (yylhs.value.node) = (yystack_[1].value.node);
    }
    break;

  case 334: // SwitchCaseClause: SwitchCaseOperandList ReturnOrSelect ExprSingle
    {
      (yylhs.value.node) = new SwitchCaseClause(LOC(yylhs.location), dynamic_cast<SwitchCaseOperandList*>((yystack_[2].value.node)), (yystack_[0].value.expr));
    }
    break;

  case 335: // SwitchCaseOperandList: "'case'" ExprSingle
    {
      SwitchCaseOperandList* sco_list_p = new SwitchCaseOperandList(LOC(yylhs.location));
      sco_list_p->push_back((yystack_[0].value.expr));
      (yylhs.value.node) = sco_list_p;
    }
    break;

  case 336: // SwitchCaseOperandList: SwitchCaseOperandList "'case'" ExprSingle
    {
      SwitchCaseOperandList* sco_list_p = static_cast<SwitchCaseOperandList*>((yystack_[2].value.node));
      sco_list_p->push_back((yystack_[0].value.expr));
      (yylhs.value.node) = (yystack_[2].value.node);
    }
    break;

  case 337: // SwitchStatement: "'switch'" "'('" Expr "')'" SwitchCaseStatementList "'default'" ReturnOrSelect Statement
    {
      (yylhs.value.expr) = new SwitchExpr(LOC(yylhs.location), (yystack_[5].value.expr), static_cast<SwitchCaseClauseList*>((yystack_[3].value.node)), (yystack_[0].value.expr));
    }
    break;

  case 338: // SwitchCaseStatementList: SwitchCaseStatement
    {
      SwitchCaseClauseList* scc_list_p = new SwitchCaseClauseList(LOC(yylhs.location));
      scc_list_p->push_back(static_cast<SwitchCaseClause*>((yystack_[0].value.node)));
      (yylhs.value.node) = scc_list_p;
    }
    break;

  case 339: // SwitchCaseStatementList: SwitchCaseStatementList SwitchCaseStatement
    {
      SwitchCaseClauseList* scc_list_p = static_cast<SwitchCaseClauseList*>((yystack_[1].value.node));
      scc_list_p->push_back(static_cast<SwitchCaseClause*>((yystack_[0].value.node)));
      (yylhs.value.node) = (yystack_[1].value.node);
    }
    break;

  case 340: // SwitchCaseStatement: SwitchCaseOperandList ReturnOrSelect Statement
    {
      (yylhs.value.node) = new SwitchCaseClause(LOC(yylhs.location), dynamic_cast<SwitchCaseOperandList*>((yystack_[2].value.node)), (yystack_[0].value.expr));
    }
    break;

  case 341: // TypeswitchExpr: "'typeswitch'" "'('" Expr "')'" CaseClauseList "'default'" ReturnOrSelect ExprSingle
    {
      (yylhs.value.expr) = new TypeswitchExpr(LOC(yylhs.location),
                              (yystack_[5].value.expr),
                              static_cast<CaseClauseList*>((yystack_[3].value.node)),
                              (yystack_[0].value.expr));
    }
    break;

  case 342: // TypeswitchExpr: "'typeswitch'" "'('" Expr "')'" CaseClauseList "'default'" "'$'" QNAME ReturnOrSelect ExprSingle
    {
      (yylhs.value.expr) = new TypeswitchExpr(LOC (yylhs.location),
                              (yystack_[7].value.expr),
                              static_cast<CaseClauseList*>((yystack_[5].value.node)),
                              static_cast<QName*>((yystack_[2].value.expr)),
                              (yystack_[0].value.expr));
    }
    break;

  case 343: // TypeswitchStatement: "'typeswitch'" "'('" Expr "')'" CaseStatementList "'default'" ReturnOrSelect Statement
    {
      (yylhs.value.expr) = new TypeswitchExpr(LOC(yylhs.location),
                              (yystack_[5].value.expr),
                              static_cast<CaseClauseList*>((yystack_[3].value.node)),
                              (yystack_[0].value.expr));
    }
    break;

  case 344: // TypeswitchStatement: "'typeswitch'" "'('" Expr "')'" CaseClauseList "'default'" "'$'" QNAME ReturnOrSelect Statement
    {
      (yylhs.value.expr) = new TypeswitchExpr(LOC (yylhs.location),
                              (yystack_[7].value.expr),
                              static_cast<CaseClauseList*>((yystack_[5].value.node)),
                              static_cast<QName*>((yystack_[2].value.expr)),
                              (yystack_[0].value.expr));
    }
    break;

  case 345: // CaseClauseList: CaseClause
    {
      CaseClauseList* cc_list_p = new CaseClauseList(LOC (yylhs.location));
      cc_list_p->push_back(static_cast<CaseClause*>((yystack_[0].value.node)));
      (yylhs.value.node) = cc_list_p;
    }
    break;

  case 346: // CaseClauseList: CaseClauseList CaseClause
    {
      CaseClauseList* cc_list_p = dynamic_cast<CaseClauseList*>((yystack_[1].value.node));
      cc_list_p->push_back(static_cast<CaseClause*>((yystack_[0].value.node)));
      (yylhs.value.node) = (yystack_[1].value.node);
    }
    break;

  case 347: // CaseClause: "'case'" SequenceTypeList ReturnOrSelect ExprSingle
    {
      (yylhs.value.node) = new CaseClause(LOC (yylhs.location),
                          static_cast<SequenceTypeList*>((yystack_[2].value.node)),
                          (yystack_[0].value.expr));
    }
    break;

  case 348: // CaseClause: "'case'" "'$'" QNAME "'as'" SequenceTypeList ReturnOrSelect ExprSingle
    {
      (yylhs.value.node) = new CaseClause(LOC (yylhs.location),
                          static_cast<QName*>((yystack_[4].value.expr)),
                          static_cast<SequenceTypeList*>((yystack_[2].value.node)),
                          (yystack_[0].value.expr));
     }
    break;

  case 349: // CaseStatementList: CaseStatement
    {
      CaseClauseList* cc_list_p = new CaseClauseList(LOC (yylhs.location));
      cc_list_p->push_back(static_cast<CaseClause*>((yystack_[0].value.node)));
      (yylhs.value.node) = cc_list_p;
    }
    break;

  case 350: // CaseStatementList: CaseStatementList CaseStatement
    {
      CaseClauseList* cc_list_p = static_cast<CaseClauseList*>((yystack_[1].value.node));
      cc_list_p->push_back(static_cast<CaseClause*>((yystack_[0].value.node)));
      (yylhs.value.node) = (yystack_[1].value.node);
    }
    break;

  case 351: // CaseStatement: "'case'" SequenceTypeList ReturnOrSelect Statement
    {
      (yylhs.value.node) = new CaseClause(LOC (yylhs.location),
                          static_cast<SequenceTypeList*>((yystack_[2].value.node)),
                          (yystack_[0].value.expr));
    }
    break;

  case 352: // CaseStatement: "'case'" "'$'" QNAME "'as'" SequenceTypeList ReturnOrSelect Statement
    {
      (yylhs.value.node) = new CaseClause(LOC (yylhs.location),
                          static_cast<QName*>((yystack_[4].value.expr)),
                          static_cast<SequenceTypeList*>((yystack_[2].value.node)),
                          (yystack_[0].value.expr));
     }
    break;

  case 353: // SequenceTypeList: SequenceType
    {
      SequenceTypeList* seqList = new SequenceTypeList(LOC(yylhs.location));
      seqList->push_back(static_cast<SequenceTypeAST*>((yystack_[0].value.node)));
      (yylhs.value.node) = seqList;
    }
    break;

  case 354: // SequenceTypeList: SequenceTypeList "'|'" SequenceType
    {
      SequenceTypeList* seqList = static_cast<SequenceTypeList*>((yystack_[2].value.node));
      seqList->push_back(static_cast<SequenceTypeAST*>((yystack_[0].value.node)));
      (yylhs.value.node) = (yystack_[2].value.node);
    }
    break;

  case 355: // IfExpr: "'if'" "'('" Expr "')'" "'then'" ExprSingle "'else'" ExprSingle
    {
      (yylhs.value.expr) = new IfExpr(LOC (yylhs.location), (yystack_[5].value.expr), (yystack_[2].value.expr), (yystack_[0].value.expr));
    }
    break;

  case 356: // OrExpr: AndExpr
    {
      (yylhs.value.expr) = (yystack_[0].value.expr);
    }
    break;

  case 357: // OrExpr: OrExpr "'or'" AndExpr
    {
      (yylhs.value.expr) = new OrExpr( LOC(yylhs.location), (yystack_[2].value.expr), (yystack_[0].value.expr) );
    }
    break;

  case 358: // AndExpr: ComparisonExpr
        {
            (yylhs.value.expr) = (yystack_[0].value.expr);
        }
    break;

  case 359: // AndExpr: AndExpr "'and'" ComparisonExpr
        {
            (yylhs.value.expr) = new AndExpr( LOC(yylhs.location), (yystack_[2].value.expr), (yystack_[0].value.expr) );
        }
    break;

  case 360: // ComparisonExpr: FTContainsExpr
        {
            (yylhs.value.expr) = (yystack_[0].value.expr);
        }
    break;

  case 361: // ComparisonExpr: FTContainsExpr ValueComp FTContainsExpr
        {
            (yylhs.value.expr) = new ComparisonExpr(
                LOC(yylhs.location),
                dynamic_cast<ValueComp*>((yystack_[1].value.node)),
                (yystack_[2].value.expr),
                (yystack_[0].value.expr)
            );
        }
    break;

  case 362: // ComparisonExpr: FTContainsExpr NodeComp FTContainsExpr
        {
            (yylhs.value.expr) = new ComparisonExpr(
                LOC(yylhs.location), dynamic_cast<NodeComp*>((yystack_[1].value.node)), (yystack_[2].value.expr), (yystack_[0].value.expr)
            );
        }
    break;

  case 363: // ComparisonExpr: FTContainsExpr "'='" FTContainsExpr
        {
            (yylhs.value.expr) = new ComparisonExpr(
                LOC(yylhs.location),
                new GeneralComp( LOC(yylhs.location), ParseConstants::op_eq ),
                (yystack_[2].value.expr),
                (yystack_[0].value.expr)
            );
        }
    break;

  case 364: // ComparisonExpr: FTContainsExpr "'!='" FTContainsExpr
        {
            (yylhs.value.expr) = new ComparisonExpr(
                LOC(yylhs.location),
                new GeneralComp( LOC(yylhs.location), ParseConstants::op_ne ),
                (yystack_[2].value.expr),
                (yystack_[0].value.expr)
            );
        }
    break;

  case 365: // $@1: %empty
        {
            driver.getXqueryLexer()->interpretAsLessThan();
        }
    break;

  case 366: // ComparisonExpr: FTContainsExpr "'<'" $@1 FTContainsExpr
        {
            (yylhs.value.expr) = new ComparisonExpr(
                LOC(yylhs.location),
                new GeneralComp( LOC(yylhs.location), ParseConstants::op_lt ),
                (yystack_[3].value.expr),
                (yystack_[0].value.expr)
            );
        }
    break;

  case 367: // ComparisonExpr: FTContainsExpr "'<='" FTContainsExpr
        {
            (yylhs.value.expr) = new ComparisonExpr(
                LOC(yylhs.location),
                new GeneralComp( LOC(yylhs.location), ParseConstants::op_le ),
                (yystack_[2].value.expr),
                (yystack_[0].value.expr)
            );
        }
    break;

  case 368: // ComparisonExpr: FTContainsExpr "'>'" FTContainsExpr
        {
            (yylhs.value.expr) = new ComparisonExpr(
                LOC(yylhs.location),
                new GeneralComp( LOC(yylhs.location), ParseConstants::op_gt ),
                (yystack_[2].value.expr),
                (yystack_[0].value.expr)
            );
        }
    break;

  case 369: // ComparisonExpr: FTContainsExpr "'>='" FTContainsExpr
        {
            (yylhs.value.expr) = new ComparisonExpr(
                LOC(yylhs.location),
                new GeneralComp( LOC(yylhs.location), ParseConstants::op_ge ),
                (yystack_[2].value.expr),
                (yystack_[0].value.expr)
            );
        }
    break;

  case 370: // FTContainsExpr: StringConcatExpr
        {
            (yylhs.value.expr) = (yystack_[0].value.expr);
        }
    break;

  case 371: // FTContainsExpr: StringConcatExpr "'contains'" "'text'" FTSelection opt_FTIgnoreOption
        {
            (yylhs.value.expr) = new FTContainsExpr(
                LOC(yylhs.location),
                (yystack_[4].value.expr),
                dynamic_cast<FTSelection*>((yystack_[1].value.node)),
                dynamic_cast<FTIgnoreOption*>((yystack_[0].value.node))
            );
        }
    break;

  case 372: // StringConcatExpr: RangeExpr
        {
          (yylhs.value.expr) = (yystack_[0].value.expr);
        }
    break;

  case 373: // StringConcatExpr: RangeExpr "'||'" StringConcatExpr
        {
          (yylhs.value.expr) = new StringConcatExpr(LOC(yylhs.location), (yystack_[2].value.expr), (yystack_[0].value.expr));
        }
    break;

  case 374: // opt_FTIgnoreOption: %empty
        {
            (yylhs.value.node) = NULL;
        }
    break;

  case 375: // opt_FTIgnoreOption: FTIgnoreOption
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 376: // RangeExpr: AdditiveExpr
        {
            (yylhs.value.expr) = (yystack_[0].value.expr);
        }
    break;

  case 377: // RangeExpr: AdditiveExpr "'to'" AdditiveExpr
        {
            (yylhs.value.expr) = new RangeExpr( LOC(yylhs.location), (yystack_[2].value.expr), (yystack_[0].value.expr) );
        }
    break;

  case 378: // AdditiveExpr: MultiplicativeExpr
        {
            (yylhs.value.expr) = (yystack_[0].value.expr);
        }
    break;

  case 379: // AdditiveExpr: AdditiveExpr "'+'" MultiplicativeExpr
        {
            (yylhs.value.expr) = new AdditiveExpr( LOC(yylhs.location), ParseConstants::op_plus, (yystack_[2].value.expr), (yystack_[0].value.expr) );
        }
    break;

  case 380: // AdditiveExpr: AdditiveExpr "'-'" MultiplicativeExpr
        {
            (yylhs.value.expr) = new AdditiveExpr( LOC(yylhs.location), ParseConstants::op_minus, (yystack_[2].value.expr), (yystack_[0].value.expr) );
        }
    break;

  case 381: // MultiplicativeExpr: UnionExpr
        {
            (yylhs.value.expr) = (yystack_[0].value.expr);
        }
    break;

  case 382: // MultiplicativeExpr: MultiplicativeExpr "'*'" UnionExpr
        {
            (yylhs.value.expr) = new MultiplicativeExpr(
                LOC(yylhs.location), ParseConstants::op_mul, (yystack_[2].value.expr), (yystack_[0].value.expr)
            );
        }
    break;

  case 383: // MultiplicativeExpr: MultiplicativeExpr "'div'" UnionExpr
        {
            (yylhs.value.expr) = new MultiplicativeExpr(
                LOC(yylhs.location), ParseConstants::op_div, (yystack_[2].value.expr), (yystack_[0].value.expr)
            );
        }
    break;

  case 384: // MultiplicativeExpr: MultiplicativeExpr "'idiv'" UnionExpr
        {
            (yylhs.value.expr) = new MultiplicativeExpr(
                LOC(yylhs.location), ParseConstants::op_idiv, (yystack_[2].value.expr), (yystack_[0].value.expr)
            );
        }
    break;

  case 385: // MultiplicativeExpr: MultiplicativeExpr "'mod'" UnionExpr
        {
            (yylhs.value.expr) = new MultiplicativeExpr(
                LOC(yylhs.location), ParseConstants::op_mod, (yystack_[2].value.expr), (yystack_[0].value.expr)
            );
        }
    break;

  case 386: // UnionExpr: IntersectExceptExpr
        {
            (yylhs.value.expr) = (yystack_[0].value.expr);
        }
    break;

  case 387: // UnionExpr: UnionExpr "'union'" IntersectExceptExpr
        {
            (yylhs.value.expr) = new UnionExpr( LOC(yylhs.location), (yystack_[2].value.expr), (yystack_[0].value.expr) );
        }
    break;

  case 388: // UnionExpr: UnionExpr "'|'" IntersectExceptExpr
        {
            (yylhs.value.expr) = new UnionExpr( LOC(yylhs.location), (yystack_[2].value.expr), (yystack_[0].value.expr) );
        }
    break;

  case 389: // IntersectExceptExpr: InstanceofExpr
        {
            (yylhs.value.expr) = (yystack_[0].value.expr);
        }
    break;

  case 390: // IntersectExceptExpr: IntersectExceptExpr "'intersect'" InstanceofExpr
        {
            (yylhs.value.expr) = new IntersectExceptExpr(
                LOC(yylhs.location), ParseConstants::op_intersect, (yystack_[2].value.expr), (yystack_[0].value.expr)
            );
        }
    break;

  case 391: // IntersectExceptExpr: IntersectExceptExpr "'except'" InstanceofExpr
        {
            (yylhs.value.expr) = new IntersectExceptExpr(
                LOC(yylhs.location), ParseConstants::op_except, (yystack_[2].value.expr), (yystack_[0].value.expr)
            );
        }
    break;

  case 392: // InstanceofExpr: TreatExpr
        {
            (yylhs.value.expr) = (yystack_[0].value.expr);
        }
    break;

  case 393: // InstanceofExpr: TreatExpr "'instance'" "'of'" SequenceType
        {
            (yylhs.value.expr) = new InstanceofExpr(
                LOC(yylhs.location), (yystack_[3].value.expr), dynamic_cast<SequenceTypeAST*>((yystack_[0].value.node))
            );
        }
    break;

  case 394: // TreatExpr: CastableExpr
        {
            (yylhs.value.expr) = (yystack_[0].value.expr);
        }
    break;

  case 395: // TreatExpr: CastableExpr "'treat'" "'as'" SequenceType
        {
            (yylhs.value.expr) = new TreatExpr(
                LOC(yylhs.location), (yystack_[3].value.expr), dynamic_cast<SequenceTypeAST*>((yystack_[0].value.node))
            );
        }
    break;

  case 396: // CastableExpr: CastExpr
        {
            (yylhs.value.expr) = (yystack_[0].value.expr);
        }
    break;

  case 397: // CastableExpr: CastExpr "'castable'" "'as'" SingleType
        {
            (yylhs.value.expr) = new CastableExpr(
                LOC(yylhs.location), (yystack_[3].value.expr), dynamic_cast<SingleType*>((yystack_[0].value.node))
            );
        }
    break;

  case 398: // CastExpr: UnaryExpr
        {
            (yylhs.value.expr) = (yystack_[0].value.expr);
        }
    break;

  case 399: // CastExpr: UnaryExpr "'cast'" "'as'" SingleType
        {
            (yylhs.value.expr) = new CastExpr(
                LOC(yylhs.location), (yystack_[3].value.expr), dynamic_cast<SingleType*>((yystack_[0].value.node))
            );
        }
    break;

  case 400: // SingleType: SimpleType
    {
      (yylhs.value.node) = new SingleType(LOC(yylhs.location), dynamic_cast<SimpleType*>((yystack_[0].value.node)), false);
    }
    break;

  case 401: // SingleType: SimpleType "'?'"
    {
      (yylhs.value.node) = new SingleType(LOC(yylhs.location), dynamic_cast<SimpleType*>((yystack_[1].value.node)), true);
    }
    break;

  case 402: // UnaryExpr: ValueExpr
        {
            (yylhs.value.expr) = (yystack_[0].value.expr);
        }
    break;

  case 403: // UnaryExpr: SignList ValueExpr
        {
            (yylhs.value.expr) = new UnaryExpr( LOC(yylhs.location), dynamic_cast<SignList*>((yystack_[1].value.node)), (yystack_[0].value.expr) );
        }
    break;

  case 404: // SignList: "'+'"
        {
            (yylhs.value.node) = new SignList( LOC(yylhs.location), true );
        }
    break;

  case 405: // SignList: "'-'"
        {
            (yylhs.value.node) = new SignList( LOC(yylhs.location), false );
        }
    break;

  case 406: // SignList: SignList "'+'"
        {
            (yylhs.value.node) = (yystack_[1].value.node);
        }
    break;

  case 407: // SignList: SignList "'-'"
        {
            if ( SignList *sl = dynamic_cast<SignList*>((yystack_[1].value.node)) )
                sl->negate();
            (yylhs.value.node) = (yystack_[1].value.node);
        }
    break;

  case 408: // ValueExpr: ValidateExpr
        { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 409: // ValueExpr: SimpleMapExpr
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 410: // ValueExpr: ExtensionExpr
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 411: // SimpleMapExpr: PathExpr
      {
        (yylhs.value.expr) = (yystack_[0].value.expr);
      }
    break;

  case 412: // SimpleMapExpr: SimpleMapExpr "'!'" PathExpr
      {
        (yylhs.value.expr) = new SimpleMapExpr(LOC(yylhs.location), (yystack_[2].value.expr), (yystack_[0].value.expr));
      }
    break;

  case 413: // ValueComp: "'eq'"
        {
            (yylhs.value.node) = new ValueComp( LOC(yylhs.location), ParseConstants::op_val_eq );
        }
    break;

  case 414: // ValueComp: "'ne'"
        {
            (yylhs.value.node) = new ValueComp( LOC(yylhs.location), ParseConstants::op_val_ne );
        }
    break;

  case 415: // ValueComp: "'lt'"
        {
            (yylhs.value.node) = new ValueComp( LOC(yylhs.location), ParseConstants::op_val_lt );
        }
    break;

  case 416: // ValueComp: "'le'"
        {
            (yylhs.value.node) = new ValueComp( LOC(yylhs.location), ParseConstants::op_val_le );
        }
    break;

  case 417: // ValueComp: "'gt'"
        {
            (yylhs.value.node) = new ValueComp( LOC(yylhs.location), ParseConstants::op_val_gt );
        }
    break;

  case 418: // ValueComp: "'ge'"
        {
            (yylhs.value.node) = new ValueComp( LOC(yylhs.location), ParseConstants::op_val_ge );
        }
    break;

  case 419: // NodeComp: "'is'"
        {
            (yylhs.value.node) = new NodeComp( LOC(yylhs.location), ParseConstants::op_is );
        }
    break;

  case 420: // NodeComp: "'<<'"
        {
            (yylhs.value.node) = new NodeComp( LOC(yylhs.location), ParseConstants::op_precedes );
        }
    break;

  case 421: // NodeComp: "'follows'"
        {
            (yylhs.value.node) = new NodeComp( LOC(yylhs.location), ParseConstants::op_follows );
        }
    break;

  case 422: // ValidateExpr: "'validate'" "'{'" StatementsAndExpr "'}'"
        {
            (yylhs.value.expr) = new ValidateExpr( LOC(yylhs.location), "strict", (yystack_[1].value.expr) );
        }
    break;

  case 423: // ValidateExpr: "'validate'" "'lax'" "'{'" StatementsAndExpr "'}'"
        {
            (yylhs.value.expr) = new ValidateExpr( LOC(yylhs.location), "lax", (yystack_[1].value.expr) );
        }
    break;

  case 424: // ValidateExpr: "'validate'" "'strict'" "'{'" StatementsAndExpr "'}'"
        {
            (yylhs.value.expr) = new ValidateExpr( LOC(yylhs.location), "strict", (yystack_[1].value.expr) );
        }
    break;

  case 425: // ValidateExpr: "'validate'" "'type'" TypeName "'{'" StatementsAndExpr "'}'"
        {
            (yylhs.value.expr) = new ValidateExpr(
                LOC(yylhs.location), dynamic_cast<TypeName*>((yystack_[3].value.node))->get_name(), (yystack_[1].value.expr)
            );
            delete (yystack_[3].value.node);
        }
    break;

  case 426: // ExtensionExpr: Pragma_list "'{'" StatementsAndOptionalExpr "'}'"
        {
            (yylhs.value.expr) = new ExtensionExpr(
                LOC(yylhs.location), dynamic_cast<PragmaList*>((yystack_[3].value.node)), (yystack_[1].value.expr)
            );
        }
    break;

  case 427: // Pragma_list: Pragma
        {
            PragmaList *pl = new PragmaList( LOC(yylhs.location) );
            pl->push_back( dynamic_cast<Pragma*>((yystack_[0].value.node)) );
            (yylhs.value.node) = pl;
        }
    break;

  case 428: // Pragma_list: Pragma_list Pragma
        {
            if ( PragmaList *pl = dynamic_cast<PragmaList*>((yystack_[1].value.node)) )
                pl->push_back( dynamic_cast<Pragma*>((yystack_[0].value.node)) );
            (yylhs.value.node) = (yystack_[1].value.node);
        }
    break;

  case 429: // Pragma: "'(#'" QNAME "'pragma literal'"
        {
            (yylhs.value.node) = new Pragma( LOC(yylhs.location), static_cast<QName*>((yystack_[1].value.expr)), SYMTAB((yystack_[0].value.sval)) );
        }
    break;

  case 430: // Pragma: "'(#'" "'QName #)'"
        {
            (yylhs.value.node) = new Pragma( LOC(yylhs.location), new QName( LOC(yylhs.location), SYMTAB((yystack_[0].value.sval)) ), "" );
        }
    break;

  case 431: // Pragma: "'(#'" "'EQName #)'"
        {
            (yylhs.value.node) = new Pragma( LOC(yylhs.location), new QName( LOC(yylhs.location), SYMTAB((yystack_[0].value.sval)), true ), "" );
        }
    break;

  case 432: // PathExpr: LeadingSlash
    {
      (yylhs.value.expr) = new PathExpr(LOC(yylhs.location), ParseConstants::path_leading_lone_slash, NULL);
    }
    break;

  case 433: // PathExpr: LeadingSlash RelativePathExpr
    {
      RelativePathExpr* rpe;
      rpe = new RelativePathExpr(LOC(yylhs.location), ParseConstants::st_slash, NULL, (yystack_[0].value.expr), false);
      (yylhs.value.expr) = new PathExpr(LOC(yylhs.location),
                        ParseConstants::path_leading_slash,
                        rpe);
    }
    break;

  case 434: // PathExpr: "'//'" RelativePathExpr
    {
      RelativePathExpr* rpe;
      rpe = new RelativePathExpr(LOC(yylhs.location), ParseConstants::st_slashslash, NULL, (yystack_[0].value.expr), false);
      (yylhs.value.expr) = new PathExpr(LOC(yylhs.location),
                        ParseConstants::path_leading_slashslash,
                        rpe);
    }
    break;

  case 435: // PathExpr: RelativePathExpr
    {
      (yylhs.value.expr) = NULL;
      RelativePathExpr* rpe = dynamic_cast<RelativePathExpr*>((yystack_[0].value.expr));
      if (rpe != NULL &&
         ( (dynamic_cast<ContextItemExpr*>(rpe->get_step_expr()) != NULL &&
             dynamic_cast<ContextItemExpr*>(rpe->get_step_expr())->is_placeholder() &&
             dynamic_cast<AxisStep*>(rpe->get_relpath_expr()) != NULL)
           ||
             dynamic_cast<AxisStep*>(rpe->get_step_expr()) != NULL))
      {
        driver.addCommonLanguageWarning(yystack_[0].location, ZED(ZWST0009_AXIS_STEP));
      }
      if ((yylhs.value.expr) == NULL)
        (yylhs.value.expr) = (rpe ?
              new PathExpr( LOC(yylhs.location), ParseConstants::path_relative, (yystack_[0].value.expr)) :
              (yystack_[0].value.expr));
    }
    break;

  case 436: // LeadingSlash: "'/'"
    {
      (yylhs.value.expr) = NULL;
    }
    break;

  case 437: // RelativePathExpr: StepExpr
    {
      AxisStep* as = dynamic_cast<AxisStep*>((yystack_[0].value.expr));
      (yylhs.value.expr) = (as ?
            new RelativePathExpr(LOC(yylhs.location),
                                 ParseConstants::st_slash,
                                 new ContextItemExpr( LOC(yylhs.location), true ), (yystack_[0].value.expr), true)
            :
            (yystack_[0].value.expr));
    }
    break;

  case 438: // RelativePathExpr: StepExpr "'/'" RelativePathExpr
    {
      (yylhs.value.expr) = new RelativePathExpr(LOC(yylhs.location), ParseConstants::st_slash, (yystack_[2].value.expr), (yystack_[0].value.expr), false);
    }
    break;

  case 439: // RelativePathExpr: StepExpr "'//'" RelativePathExpr
    {
      (yylhs.value.expr) = new RelativePathExpr(LOC(yylhs.location), ParseConstants::st_slashslash, (yystack_[2].value.expr), (yystack_[0].value.expr), false);
    }
    break;

  case 440: // StepExpr: AxisStep
    { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 441: // StepExpr: PostfixExpr
    { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 442: // AxisStep: ForwardStep
        {
            (yylhs.value.expr) = new AxisStep(
                LOC(yylhs.location), dynamic_cast<ForwardStep*>((yystack_[0].value.node)), NULL
            );
        }
    break;

  case 443: // AxisStep: ForwardStep PredicateList
        {
            (yylhs.value.expr) = new AxisStep(
                LOC(yylhs.location),
                dynamic_cast<ForwardStep*>((yystack_[1].value.node)),
                dynamic_cast<PredicateList*>((yystack_[0].value.node))
            );
        }
    break;

  case 444: // AxisStep: ReverseStep
        {
            (yylhs.value.expr) = new AxisStep(
                LOC(yylhs.location), dynamic_cast<ReverseStep*>((yystack_[0].value.node)), NULL
            );
        }
    break;

  case 445: // AxisStep: ReverseStep PredicateList
        {
            (yylhs.value.expr) = new AxisStep(
                LOC(yylhs.location),
                dynamic_cast<ReverseStep*>((yystack_[1].value.node)),
                dynamic_cast<PredicateList*>((yystack_[0].value.node))
            );
        }
    break;

  case 446: // ForwardStep: ForwardAxis NodeTest
        {
            (yylhs.value.node) = new ForwardStep(
                LOC(yylhs.location), dynamic_cast<ForwardAxis*>((yystack_[1].value.node)), (yystack_[0].value.node)
            );
        }
    break;

  case 447: // ForwardStep: AbbrevForwardStep
        {
            (yylhs.value.node) = new ForwardStep(
                LOC(yylhs.location), dynamic_cast<AbbrevForwardStep*>((yystack_[0].value.node))
            );
        }
    break;

  case 448: // ForwardAxis: "'child'" "'::'"
        {
            (yylhs.value.node) = new ForwardAxis( LOC(yylhs.location), ParseConstants::axis_child );
        }
    break;

  case 449: // ForwardAxis: "'descendant'" "'::'"
        {
            (yylhs.value.node) = new ForwardAxis( LOC(yylhs.location), ParseConstants::axis_descendant);
        }
    break;

  case 450: // ForwardAxis: "'attribute'" "'::'"
        {
            (yylhs.value.node) = new ForwardAxis( LOC(yylhs.location), ParseConstants::axis_attribute );
        }
    break;

  case 451: // ForwardAxis: "'self'" "'::'"
        {
            (yylhs.value.node) = new ForwardAxis( LOC(yylhs.location), ParseConstants::axis_self );
        }
    break;

  case 452: // ForwardAxis: "'descendant-or-self'" "'::'"
        {
            (yylhs.value.node) = new ForwardAxis(
                LOC(yylhs.location), ParseConstants::axis_descendant_or_self
            );
        }
    break;

  case 453: // ForwardAxis: "'following-sibling'" "'::'"
        {
            (yylhs.value.node) = new ForwardAxis(
                LOC(yylhs.location), ParseConstants::axis_following_sibling
            );
        }
    break;

  case 454: // ForwardAxis: "'following'" "'::'"
        {
            (yylhs.value.node) = new ForwardAxis( LOC(yylhs.location), ParseConstants::axis_following );
        }
    break;

  case 455: // AbbrevForwardStep: NodeTest
        {
            (yylhs.value.node) = new AbbrevForwardStep( LOC(yylhs.location), (yystack_[0].value.node), false );
        }
    break;

  case 456: // AbbrevForwardStep: "'@'" NodeTest
        {
            (yylhs.value.node) = new AbbrevForwardStep( LOC(yylhs.location), (yystack_[0].value.node), true );
        }
    break;

  case 457: // ReverseStep: ReverseAxis NodeTest
        {
            (yylhs.value.node) = new ReverseStep( LOC(yylhs.location), dynamic_cast<ReverseAxis*>((yystack_[1].value.node)), (yystack_[0].value.node) );
        }
    break;

  case 458: // ReverseStep: "'..'"
        {
            ReverseAxis *ra = new ReverseAxis(
                LOC(yylhs.location), ParseConstants::axis_parent
            );
            (yylhs.value.node) = new ReverseStep( LOC(yylhs.location), ra, NULL );
        }
    break;

  case 459: // ReverseAxis: "'parent'" "'::'"
        {
            (yylhs.value.node) = new ReverseAxis( LOC(yylhs.location), ParseConstants::axis_parent );
        }
    break;

  case 460: // ReverseAxis: "'ancestor'" "'::'"
        {
            (yylhs.value.node) = new ReverseAxis( LOC(yylhs.location), ParseConstants::axis_ancestor );
        }
    break;

  case 461: // ReverseAxis: "'preceding-sibling'" "'::'"
        {
            (yylhs.value.node) = new ReverseAxis(
                LOC(yylhs.location), ParseConstants::axis_preceding_sibling
            );
        }
    break;

  case 462: // ReverseAxis: "'preceding'" "'::'"
        {
            (yylhs.value.node) = new ReverseAxis( LOC(yylhs.location), ParseConstants::axis_preceding );
        }
    break;

  case 463: // ReverseAxis: "'ancestor-or-self'" "'::'"
        {
            (yylhs.value.node) = new ReverseAxis(
                LOC(yylhs.location), ParseConstants::axis_ancestor_or_self
            );
        }
    break;

  case 464: // NodeTest: KindTest
        { (yylhs.value.node) = (yystack_[0].value.node); }
    break;

  case 465: // NodeTest: NameTest
      { (yylhs.value.node) = (yystack_[0].value.node); }
    break;

  case 466: // NameTest: QNAME
        {
            (yylhs.value.node) = new NameTest( LOC(yylhs.location), static_cast<QName*>((yystack_[0].value.expr)) );
        }
    break;

  case 467: // NameTest: Wildcard
        {
            (yylhs.value.node) = new NameTest( LOC(yylhs.location), dynamic_cast<Wildcard*>((yystack_[0].value.node)) );
        }
    break;

  case 468: // Wildcard: "'*'"
    {
      (yylhs.value.node) = new Wildcard(LOC(yylhs.location), "", "", ParseConstants::wild_all, false);
    }
    break;

  case 469: // Wildcard: "'pref:*'"
    {
      (yylhs.value.node) = new Wildcard(LOC(yylhs.location), SYMTAB((yystack_[0].value.sval)), "", ParseConstants::wild_elem, false);
    }
    break;

  case 470: // Wildcard: "'ns:*'"
    {
      (yylhs.value.node) = new Wildcard(LOC(yylhs.location), SYMTAB((yystack_[0].value.sval)), "", ParseConstants::wild_elem, true);
    }
    break;

  case 471: // Wildcard: "'*:QName'"
    {
      (yylhs.value.node) = new Wildcard(LOC(yylhs.location), "", SYMTAB((yystack_[0].value.sval)), ParseConstants::wild_prefix, false);
    }
    break;

  case 472: // PostfixExpr: PrimaryExpr
     {
       (yylhs.value.expr) = (yystack_[0].value.expr);
     }
    break;

  case 473: // PostfixExpr: PostfixExpr PredicateList
     {
       (yylhs.value.expr) = new FilterExpr(LOC(yylhs.location), (yystack_[1].value.expr), dynamic_cast<PredicateList*>((yystack_[0].value.node)));
     }
    break;

  case 474: // PostfixExpr: PostfixExpr "'('" "')'"
     {
       (yylhs.value.expr) = new DynamicFunctionInvocation(LOC(yylhs.location), (yystack_[2].value.expr), false);
     }
    break;

  case 475: // PostfixExpr: PostfixExpr "'('" ArgList "')'"
     {
       (yylhs.value.expr) = new DynamicFunctionInvocation(LOC(yylhs.location), (yystack_[3].value.expr), dynamic_cast<ArgList*>((yystack_[1].value.node)), false);
     }
    break;

  case 476: // PredicateList: Predicate
    {
      PredicateList* pl = new PredicateList( LOC(yylhs.location) );
      pl->push_back(dynamic_cast<exprnode*>((yystack_[0].value.expr)));
      (yylhs.value.node) = pl;
    }
    break;

  case 477: // PredicateList: PredicateList Predicate
    {
      if (PredicateList* pl = dynamic_cast<PredicateList*>((yystack_[1].value.node)))
        pl->push_back(dynamic_cast<exprnode*>((yystack_[0].value.expr)));
      (yylhs.value.node) = (yystack_[1].value.node);
    }
    break;

  case 478: // Predicate: "'['" Expr "']'"
    {
      (yylhs.value.expr) = (yystack_[1].value.expr);
    }
    break;

  case 479: // PrimaryExpr: Literal
        { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 480: // PrimaryExpr: VarRef
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 481: // PrimaryExpr: ParenthesizedExpr
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 482: // PrimaryExpr: ContextItemExpr
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 483: // PrimaryExpr: FunctionCall
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 484: // PrimaryExpr: Constructor
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 485: // PrimaryExpr: OrderedExpr
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 486: // PrimaryExpr: UnorderedExpr
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 487: // PrimaryExpr: FunctionItemExpr
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 488: // PrimaryExpr: BlockExpr
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 489: // PrimaryExpr: JSONObjectConstructor
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 490: // PrimaryExpr: JSONArrayConstructor
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 491: // PrimaryExpr: JSONSimpleObjectUnion
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 492: // PrimaryExpr: JSONAccumulatorObjectUnion
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 493: // Literal: NumericLiteral
        {
            (yylhs.value.expr) = (yystack_[0].value.expr);
        }
    break;

  case 494: // Literal: StringLiteral
        {
            (yylhs.value.expr) = (yystack_[0].value.expr);
        }
    break;

  case 495: // NumericLiteral: "'DECIMAL'"
        {
            (yylhs.value.expr) = NumericLiteral::new_literal(
                LOC(yylhs.location), ParseConstants::num_decimal, *(yystack_[0].value.decval)
            );
            delete (yystack_[0].value.decval);
        }
    break;

  case 496: // NumericLiteral: "'INTEGER'"
        {
            (yylhs.value.expr) = NumericLiteral::new_literal(
                LOC(yylhs.location), ParseConstants::num_integer, *(yystack_[0].value.ival)
            );
            delete (yystack_[0].value.ival);
        }
    break;

  case 497: // NumericLiteral: "'DOUBLE'"
        {
            (yylhs.value.expr) = NumericLiteral::new_literal(
                LOC(yylhs.location), ParseConstants::num_double, *(yystack_[0].value.dval)
            );
            delete (yystack_[0].value.dval);
        }
    break;

  case 498: // VarRef: "'$'" QNAME
        {
            (yylhs.value.expr) = new VarRef(LOC(yylhs.location), static_cast<QName*>((yystack_[0].value.expr)));
        }
    break;

  case 499: // ParenthesizedExpr: "'('" "')'"
        {
            (yylhs.value.expr) = new ParenthesizedExpr( LOC(yylhs.location), NULL);
        }
    break;

  case 500: // ParenthesizedExpr: "'('" Expr "')'"
        {
            (yylhs.value.expr) = new ParenthesizedExpr( LOC(yylhs.location), (yystack_[1].value.expr) );
        }
    break;

  case 501: // ContextItemExpr: "'.'"
        {
            driver.addCommonLanguageWarning(yystack_[0].location, ZED(ZWST0009_CONTEXT_ITEM_EXPR));
            (yylhs.value.expr) = new ContextItemExpr( LOC(yylhs.location) );
        }
    break;

  case 502: // OrderedExpr: "'ordered'" "'{'" Expr "'}'"
        {
            (yylhs.value.expr) = new OrderedExpr( LOC(yylhs.location), (yystack_[1].value.expr) );
        }
    break;

  case 503: // UnorderedExpr: "'unordered'" "'{'" Expr "'}'"
        {
            (yylhs.value.expr) = new UnorderedExpr( LOC(yylhs.location), (yystack_[1].value.expr) );
        }
    break;

  case 504: // FunctionCall: FUNCTION_NAME "'('" "')'"
    {
      (yylhs.value.expr) = new FunctionCall(LOC(yylhs.location), static_cast<QName*>((yystack_[2].value.expr)), NULL);
    }
    break;

  case 505: // FunctionCall: FUNCTION_NAME "'('" ArgList "')'"
    {
      ArgList* argList = dynamic_cast<ArgList*>((yystack_[1].value.node));
      if (argList->has_placeholder())
      {
        LiteralFunctionItem* lfi =
        new LiteralFunctionItem(LOC(yylhs.location),
                                dynamic_cast<QName*>((yystack_[3].value.expr)),
                                new Integer(argList->size()));
        (yylhs.value.expr) = new DynamicFunctionInvocation(LOC(yylhs.location), lfi, argList, true);
      }
      else
      {
        (yylhs.value.expr) = new FunctionCall(LOC(yylhs.location), static_cast<QName*>((yystack_[3].value.expr)), argList);
      }
    }
    break;

  case 506: // ArgList: "'?'"
    {
      ArgList* al = new ArgList(LOC(yylhs.location));
      al->push_back(new ArgumentPlaceholder(LOC(yylhs.location)));
      (yylhs.value.node) = al;
    }
    break;

  case 507: // ArgList: ArgList "','" "'?'"
    {
      if (ArgList* al = dynamic_cast<ArgList*>((yystack_[2].value.node)))
        al->push_back(new ArgumentPlaceholder(LOC(yylhs.location)));
      (yylhs.value.node) = (yystack_[2].value.node);
    }
    break;

  case 508: // ArgList: ExprSingle
    {
      ArgList *al = new ArgList( LOC(yylhs.location) );
      al->push_back( (yystack_[0].value.expr) );
      (yylhs.value.node) = al;
    }
    break;

  case 509: // ArgList: ArgList "','" ExprSingle
    {
      if ( ArgList *al = dynamic_cast<ArgList*>((yystack_[2].value.node)) )
        al->push_back( (yystack_[0].value.expr) );
      (yylhs.value.node) = (yystack_[2].value.node);
    }
    break;

  case 510: // FunctionItemExpr: LiteralFunctionItem
    {
      (yylhs.value.expr) = (yystack_[0].value.expr);
    }
    break;

  case 511: // FunctionItemExpr: InlineFunction
    {
      (yylhs.value.expr) = (yystack_[0].value.expr);
    }
    break;

  case 512: // LiteralFunctionItem: FUNCTION_NAME "'#'" "'INTEGER'"
    {
      (yylhs.value.expr) = new LiteralFunctionItem(LOC (yylhs.location), dynamic_cast<QName*>((yystack_[2].value.expr)), (yystack_[0].value.ival));
    }
    break;

  case 513: // InlineFunction: "'function'" FunctionSig EnclosedStatementsAndOptionalExpr
    {
      (yylhs.value.expr) = new InlineFunction(LOC(yylhs.location),
                              (yystack_[1].value.fnsig)->theParams.getp(),
                              (yystack_[1].value.fnsig)->theReturnType.getp(),
                              (yystack_[0].value.expr));
      delete (yystack_[1].value.fnsig);
    }
    break;

  case 514: // Constructor: DirectConstructor
    { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 515: // Constructor: ComputedConstructor
  { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 516: // DirectConstructor: DirElemConstructor
    { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 517: // DirectConstructor: DirCommentConstructor
    { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 518: // DirectConstructor: DirPIConstructor
    { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 519: // DirElemConstructor: "'<'" QNAME OptionalBlank "'/>'"
    {
      (yylhs.value.expr) = new DirElemConstructor(LOC(yylhs.location),
                                  static_cast<QName*>((yystack_[2].value.expr)),
                                  NULL,
                                  NULL,
                                  NULL);
    }
    break;

  case 520: // DirElemConstructor: "'<'" QNAME OptionalBlank "'> (tag end)'" "'</ (start tag end)'" QNAME OptionalBlank "'> (tag end)'"
    {
      if (static_cast<QName*>((yystack_[6].value.expr))->get_qname() != static_cast<QName*>((yystack_[2].value.expr))->get_qname())
      {
        error(yystack_[3].location, "syntax error, end tag </" +
                  static_cast<QName*>((yystack_[2].value.expr))->get_qname().str() +
                  "> does not match start tag <" +
                  static_cast<QName*>((yystack_[6].value.expr))->get_qname().str() + ">");
        release_hack_all( (yystack_[6].value.expr), (yystack_[2].value.expr) );
        YYERROR;
      }
      (yylhs.value.expr) = new DirElemConstructor(LOC(yylhs.location),
                                  static_cast<QName*>((yystack_[6].value.expr)),
                                  static_cast<QName*>((yystack_[2].value.expr)),
                                  NULL,
                                  NULL);
    }
    break;

  case 521: // DirElemConstructor: "'<'" QNAME DirAttributeList OptionalBlank "'/>'"
    {
      (yylhs.value.expr) = new DirElemConstructor(LOC(yylhs.location),
                                  static_cast<QName*>((yystack_[3].value.expr)),
                                  NULL,
                                  dynamic_cast<DirAttributeList*>((yystack_[2].value.node)),
                                  NULL);
    }
    break;

  case 522: // DirElemConstructor: "'<'" QNAME DirAttributeList OptionalBlank "'> (tag end)'" "'</ (start tag end)'" QNAME OptionalBlank "'> (tag end)'"
    {
      if (static_cast<QName*>((yystack_[7].value.expr))->get_qname() != static_cast<QName*>((yystack_[2].value.expr))->get_qname())
      {
        error(yystack_[4].location, "syntax error, end tag </" +
                  static_cast<QName*>((yystack_[2].value.expr))->get_qname().str() +
                  "> does not match start tag <" +
                  static_cast<QName*>((yystack_[7].value.expr))->get_qname().str() + ">");
        release_hack_all( (yystack_[7].value.expr), (yystack_[6].value.node), (yystack_[2].value.expr) );
        YYERROR;
      }
      (yylhs.value.expr) = new DirElemConstructor(LOC(yylhs.location),
                                  static_cast<QName*>((yystack_[7].value.expr)),
                                  static_cast<QName*>((yystack_[2].value.expr)),
                                  dynamic_cast<DirAttributeList*>((yystack_[6].value.node)),
                                  NULL);
    }
    break;

  case 523: // DirElemConstructor: "'<'" QNAME OptionalBlank "'> (tag end)'" DirElemContentList "'</ (start tag end)'" QNAME OptionalBlank "'> (tag end)'"
    {
      if (static_cast<QName*>((yystack_[7].value.expr))->get_qname() != static_cast<QName*>((yystack_[2].value.expr))->get_qname())
      {
        error(yystack_[4].location, "syntax error, end tag </" +
                  static_cast<QName*>((yystack_[2].value.expr))->get_qname().str() +
                  "> does not match start tag <" +
                  static_cast<QName*>((yystack_[7].value.expr))->get_qname().str() + ">");
        release_hack_all( (yystack_[7].value.expr), (yystack_[4].value.node), (yystack_[2].value.expr) );
        YYERROR;
      }
      (yylhs.value.expr) = new DirElemConstructor(LOC(yylhs.location),
                                  static_cast<QName*>((yystack_[7].value.expr)),
                                  static_cast<QName*>((yystack_[2].value.expr)),
                                  NULL,
                                  dynamic_cast<DirElemContentList*>((yystack_[4].value.node)));
    }
    break;

  case 524: // DirElemConstructor: "'<'" QNAME DirAttributeList OptionalBlank "'> (tag end)'" DirElemContentList "'</ (start tag end)'" QNAME OptionalBlank "'> (tag end)'"
    {
      if (static_cast<QName*>((yystack_[8].value.expr))->get_qname() != static_cast<QName*>((yystack_[2].value.expr))->get_qname())
      {
        error(yystack_[5].location, "syntax error, end tag </" +
                  static_cast<QName*>((yystack_[2].value.expr))->get_qname().str() +
                  "> does not match start tag <" +
                  static_cast<QName*>((yystack_[8].value.expr))->get_qname().str() + ">");
        release_hack_all( (yystack_[8].value.expr), (yystack_[7].value.node), (yystack_[4].value.node), (yystack_[2].value.expr) );
        YYERROR;
      }
      (yylhs.value.expr) = new DirElemConstructor(LOC(yylhs.location),
                                  static_cast<QName*>((yystack_[8].value.expr)),
                                  static_cast<QName*>((yystack_[2].value.expr)),
                                  dynamic_cast<DirAttributeList*>((yystack_[7].value.node)),
                                  dynamic_cast<DirElemContentList*>((yystack_[4].value.node)));
    }
    break;

  case 525: // DirElemContentList: DirElemContent
    {
      DirElemContentList *decl = new DirElemContentList( LOC(yylhs.location) );
      decl->push_back( dynamic_cast<DirElemContent*>((yystack_[0].value.expr)) );
      (yylhs.value.node) = decl;
    }
    break;

  case 526: // DirElemContentList: DirElemContentList DirElemContent
    {
      DirElemContentList *decl = dynamic_cast<DirElemContentList*>((yystack_[1].value.node));
      if ( decl )
        decl->push_back( dynamic_cast<DirElemContent*>((yystack_[0].value.expr)) );
      (yylhs.value.node) = (yystack_[1].value.node);
    }
    break;

  case 527: // DirAttributeList: DirAttr
    {
      DirAttributeList *dal = new DirAttributeList( LOC(yylhs.location) );
      dal->push_back( dynamic_cast<DirAttr*>((yystack_[0].value.node)) );
      (yylhs.value.node) = dal;
    }
    break;

  case 528: // DirAttributeList: DirAttributeList DirAttr
    {
      DirAttributeList *dal = dynamic_cast<DirAttributeList*>((yystack_[1].value.node));
      if ( dal )
        dal->push_back( dynamic_cast<DirAttr*>((yystack_[0].value.node)) );
      (yylhs.value.node) = (yystack_[1].value.node);
    }
    break;

  case 529: // DirAttr: "'<blank>'" QNAME OptionalBlank "'='" OptionalBlank DirAttributeValue
    {
      (yylhs.value.node) = new DirAttr(LOC(yylhs.location),
                       static_cast<QName*>((yystack_[4].value.expr)),
                       dynamic_cast<DirAttributeValue*>((yystack_[0].value.node)));
    }
    break;

  case 532: // DirAttributeValue: "'\"'" opt_QuoteAttrContentList "'\"'"
    {
      (yylhs.value.node) = new DirAttributeValue(LOC(yylhs.location),
                                 dynamic_cast<QuoteAttrContentList*>((yystack_[1].value.node)));
    }
    break;

  case 533: // DirAttributeValue: "'''" opt_AposAttrContentList "'''"
    {
      (yylhs.value.node) = new DirAttributeValue( LOC(yylhs.location),
                                  dynamic_cast<AposAttrContentList*>((yystack_[1].value.node)));
    }
    break;

  case 534: // opt_QuoteAttrContentList: %empty
    {
      (yylhs.value.node) = new QuoteAttrContentList( LOC(yylhs.location) );
    }
    break;

  case 535: // opt_QuoteAttrContentList: QuoteAttrContentList
    {
      (yylhs.value.node) = (yystack_[0].value.node);
    }
    break;

  case 536: // QuoteAttrContentList: "'\"\"'"
    {
      QuoteAttrContentList *qacl = new QuoteAttrContentList( LOC(yylhs.location) );
      qacl->push_back( new QuoteAttrValueContent( LOC(yylhs.location), "\"" ) );
      (yylhs.value.node) = qacl;
    }
    break;

  case 537: // QuoteAttrContentList: QuoteAttrValueContent
    {
      QuoteAttrContentList *qacl = new QuoteAttrContentList( LOC(yylhs.location) );
      qacl->push_back( dynamic_cast<QuoteAttrValueContent*>((yystack_[0].value.node)) );
      (yylhs.value.node) = qacl;
    }
    break;

  case 538: // QuoteAttrContentList: QuoteAttrContentList "'\"\"'"
    {
      QuoteAttrContentList* qacl = dynamic_cast<QuoteAttrContentList*>((yystack_[1].value.node));
      if ( qacl )
        qacl->push_back( new QuoteAttrValueContent( LOC(yylhs.location), "\"" ) );
      (yylhs.value.node) = (yystack_[1].value.node);
    }
    break;

  case 539: // QuoteAttrContentList: QuoteAttrContentList QuoteAttrValueContent
    {
      QuoteAttrContentList *qacl = dynamic_cast<QuoteAttrContentList*>((yystack_[1].value.node));
      if ( qacl )
        qacl->push_back( dynamic_cast<QuoteAttrValueContent*>((yystack_[0].value.node)) );
      (yylhs.value.node) = (yystack_[1].value.node);
    }
    break;

  case 540: // opt_AposAttrContentList: %empty
    {
      (yylhs.value.node) = new AposAttrContentList( LOC(yylhs.location) );
    }
    break;

  case 541: // opt_AposAttrContentList: AposAttrContentList
    {
      (yylhs.value.node) = (yystack_[0].value.node);
    }
    break;

  case 542: // AposAttrContentList: "''''"
    {
      AposAttrContentList *aacl = new AposAttrContentList( LOC(yylhs.location) );
      aacl->push_back( new AposAttrValueContent( LOC(yylhs.location),"'") );
      (yylhs.value.node) = aacl;
    }
    break;

  case 543: // AposAttrContentList: AposAttrValueContent
    {
      AposAttrContentList *aacl = new AposAttrContentList( LOC(yylhs.location) );
      aacl->push_back( dynamic_cast<AposAttrValueContent*>((yystack_[0].value.node)) );
      (yylhs.value.node) = aacl;
    }
    break;

  case 544: // AposAttrContentList: AposAttrContentList "''''"
    {
      AposAttrContentList *aacl = dynamic_cast<AposAttrContentList*>((yystack_[1].value.node));
      if (aacl)
        aacl->push_back( new AposAttrValueContent( LOC(yylhs.location),"'") );
      (yylhs.value.node) = (yystack_[1].value.node);
    }
    break;

  case 545: // AposAttrContentList: AposAttrContentList AposAttrValueContent
    {
      AposAttrContentList *aacl = dynamic_cast<AposAttrContentList*>((yystack_[1].value.node));
      if ( aacl )
        aacl->push_back( dynamic_cast<AposAttrValueContent*>((yystack_[0].value.node)) );
      (yylhs.value.node) = (yystack_[1].value.node);
    }
    break;

  case 546: // QuoteAttrValueContent: "'quote attribute content'"
    {
      (yylhs.value.node) = new QuoteAttrValueContent( LOC(yylhs.location), SYMTAB((yystack_[0].value.sval)) );
    }
    break;

  case 547: // QuoteAttrValueContent: CommonContent
    {
      (yylhs.value.node) = new QuoteAttrValueContent(LOC(yylhs.location), dynamic_cast<CommonContent*>((yystack_[0].value.expr)));
    }
    break;

  case 548: // AposAttrValueContent: "'apos attribute content'"
    {
      (yylhs.value.node) = new AposAttrValueContent( LOC(yylhs.location), SYMTAB((yystack_[0].value.sval)) );
    }
    break;

  case 549: // AposAttrValueContent: CommonContent
    {
      (yylhs.value.node) = new AposAttrValueContent(LOC(yylhs.location), dynamic_cast<CommonContent*>((yystack_[0].value.expr)));
    }
    break;

  case 550: // DirElemContent: DirectConstructor
    {
      (yylhs.value.expr) = new DirElemContent( LOC(yylhs.location), (yystack_[0].value.expr) );
    }
    break;

  case 551: // DirElemContent: "'element content'"
    {
      (yylhs.value.expr) = new DirElemContent( LOC(yylhs.location), SYMTAB((yystack_[0].value.sval)) );
    }
    break;

  case 552: // DirElemContent: CDataSection
    {
      rchandle<CDataSection> cdata_h = dynamic_cast<CDataSection*>((yystack_[0].value.expr));
      (yylhs.value.expr) = new DirElemContent( LOC(yylhs.location), cdata_h );
    }
    break;

  case 553: // DirElemContent: CommonContent
    {
      rchandle<CommonContent> cont_h = dynamic_cast<CommonContent*>((yystack_[0].value.expr));
      (yylhs.value.expr) = new DirElemContent( LOC(yylhs.location), cont_h );
    }
    break;

  case 554: // CommonContent: "'&#charref;'"
    {
      (yylhs.value.expr) = new CommonContent(LOC(yylhs.location), ParseConstants::cont_charref, SYMTAB((yystack_[0].value.sval)));
    }
    break;

  case 555: // CommonContent: "'{{'"
    {
      (yylhs.value.expr) = new CommonContent(LOC(yylhs.location), ParseConstants::cont_escape_lbrace);
    }
    break;

  case 556: // CommonContent: "'<double {>'"
    {
      (yylhs.value.expr) = new CommonContent(LOC(yylhs.location), ParseConstants::cont_escape_rbrace);
    }
    break;

  case 557: // CommonContent: "'{'" StatementsAndExpr "'}'"
    {
      (yylhs.value.expr) = new CommonContent(LOC(yylhs.location), new EnclosedExpr(LOC(yylhs.location), (yystack_[1].value.expr)));
    }
    break;

  case 558: // DirCommentConstructor: "'<!--'" "'XML comment'" "'-->'"
    {
      (yylhs.value.expr) = new DirCommentConstructor( LOC(yylhs.location), SYMTAB((yystack_[1].value.sval)) );
    }
    break;

  case 559: // DirCommentConstructor: "'<!--'" "'-->'"
    {
      (yylhs.value.expr) = new DirCommentConstructor( LOC(yylhs.location), "" );
    }
    break;

  case 560: // DirPIConstructor: "'<?'" QNAME "'?>'"
    {
      ERROR_IF_QNAME_NOT_NCNAME((yystack_[1].value.expr), yystack_[1].location);
      (yylhs.value.expr) = new DirPIConstructor( LOC(yylhs.location), static_cast<QName*>((yystack_[1].value.expr)) );
    }
    break;

  case 561: // DirPIConstructor: "'<?'" QNAME "'char literal and pi end'"
    {
      ERROR_IF_QNAME_NOT_NCNAME((yystack_[1].value.expr), yystack_[1].location);
      (yylhs.value.expr) = new DirPIConstructor( LOC(yylhs.location), static_cast<QName*>((yystack_[1].value.expr)), SYMTAB((yystack_[0].value.sval)) );
    }
    break;

  case 562: // CDataSection: "'CDATA[['" "'char literal]]>'"
    {
      (yylhs.value.expr) = new CDataSection( LOC(yylhs.location),SYMTAB((yystack_[0].value.sval)) );
    }
    break;

  case 563: // ComputedConstructor: CompDocConstructor
    {
      (yylhs.value.expr) = (yystack_[0].value.expr);
    }
    break;

  case 564: // ComputedConstructor: CompElemConstructor
    {
      (yylhs.value.expr) = (yystack_[0].value.expr);
    }
    break;

  case 565: // ComputedConstructor: CompAttrConstructor
    {
      (yylhs.value.expr) = (yystack_[0].value.expr);
    }
    break;

  case 566: // ComputedConstructor: CompTextConstructor
    {
      (yylhs.value.expr) = (yystack_[0].value.expr);
    }
    break;

  case 567: // ComputedConstructor: CompCommentConstructor
    {
      (yylhs.value.expr) = (yystack_[0].value.expr);
    }
    break;

  case 568: // ComputedConstructor: CompPIConstructor
    {
      (yylhs.value.expr) = (yystack_[0].value.expr);
    }
    break;

  case 569: // ComputedConstructor: CompNamespaceConstructor
    { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 570: // CompDocConstructor: "'document'" "'{'" StatementsAndExpr "'}'"
    {
      (yylhs.value.expr) = new CompDocConstructor( LOC(yylhs.location), (yystack_[1].value.expr) );
    }
    break;

  case 571: // CompElemConstructor: "'element QName {'" StatementsAndOptionalExpr "'}'"
    {
      (yylhs.value.expr) = new CompElemConstructor(LOC(yylhs.location), new QName(LOC(yylhs.location), SYMTAB((yystack_[2].value.sval))), (yystack_[1].value.expr));
    }
    break;

  case 572: // CompElemConstructor: "'element'" "'{'" Expr "'}'" "'{'" StatementsAndOptionalExpr "'}'"
    {
      (yylhs.value.expr) = new CompElemConstructor( LOC(yylhs.location), (yystack_[4].value.expr), (yystack_[1].value.expr) );
    }
    break;

  case 573: // CompAttrConstructor: "'attribute QName {'" StatementsAndOptionalExpr "'}'"
    {
      (yylhs.value.expr) = new CompAttrConstructor( LOC(yylhs.location), new QName(LOC(yylhs.location), SYMTAB((yystack_[2].value.sval))), (yystack_[1].value.expr) );
    }
    break;

  case 574: // CompAttrConstructor: "'attribute'" "'{'" Expr "'}'" "'{'" StatementsAndOptionalExpr "'}'"
    {
      (yylhs.value.expr) = new CompAttrConstructor( LOC(yylhs.location), (yystack_[4].value.expr), (yystack_[1].value.expr) );
    }
    break;

  case 575: // CompTextConstructor: "'text'" "'{'" StatementsAndExpr "'}'"
    {
      (yylhs.value.expr) = new CompTextConstructor( LOC(yylhs.location), (yystack_[1].value.expr) );
    }
    break;

  case 576: // CompCommentConstructor: "'comment'" "'{'" StatementsAndExpr "'}'"
    {
      (yylhs.value.expr) = new CompCommentConstructor( LOC(yylhs.location), (yystack_[1].value.expr) );
    }
    break;

  case 577: // CompPIConstructor: "'processing-instruction NCName {'" StatementsAndOptionalExpr "'}'"
    {
      (yylhs.value.expr) = new CompPIConstructor(LOC(yylhs.location), SYMTAB((yystack_[2].value.sval)), (yystack_[1].value.expr));
    }
    break;

  case 578: // CompPIConstructor: "'processing-instruction'" "'{'" Expr "'}'" "'{'" StatementsAndOptionalExpr "'}'"
    {
      (yylhs.value.expr) = new CompPIConstructor(LOC(yylhs.location), (yystack_[4].value.expr), (yystack_[1].value.expr));
    }
    break;

  case 579: // CompNamespaceConstructor: "'namespace NCName {'" StatementsAndExpr "'}'"
    {
      (yylhs.value.expr) = new CompNamespaceConstructor(LOC(yylhs.location), SYMTAB((yystack_[2].value.sval)), (yystack_[1].value.expr));
    }
    break;

  case 580: // CompNamespaceConstructor: "'namespace'" "'{'" Expr "'}'" "'{'" StatementsAndExpr "'}'"
    {
      (yylhs.value.expr) = new CompNamespaceConstructor(LOC(yylhs.location), (yystack_[4].value.expr), (yystack_[1].value.expr));
    }
    break;

  case 581: // TypeDeclaration: "'as'" SequenceType
    {
      (yylhs.value.node) = (yystack_[0].value.node);
    }
    break;

  case 582: // SequenceType: ItemType
        {
            (yylhs.value.node) = new SequenceTypeAST( LOC(yylhs.location), (yystack_[0].value.node), NULL );
        }
    break;

  case 583: // SequenceType: ItemType OccurrenceIndicator
        {
            (yylhs.value.node) = new SequenceTypeAST(LOC(yylhs.location), (yystack_[1].value.node), dynamic_cast<OccurrenceIndicator*>((yystack_[0].value.node)));
        }
    break;

  case 584: // SequenceType: "'empty-sequence'" "'('" "')'"
        {
            (yylhs.value.node) = new SequenceTypeAST( LOC(yylhs.location), NULL, NULL );
        }
    break;

  case 585: // OccurrenceIndicator: "'?'"
        {
            (yylhs.value.node) = new OccurrenceIndicator(
                LOC(yylhs.location), ParseConstants::occurs_optionally
            );
        }
    break;

  case 586: // OccurrenceIndicator: "'*'"
        {
            (yylhs.value.node) = new OccurrenceIndicator(
                LOC(yylhs.location), ParseConstants::occurs_zero_or_more
            );
        }
    break;

  case 587: // OccurrenceIndicator: "'+'"
        {
            (yylhs.value.node) = new OccurrenceIndicator(
                LOC(yylhs.location), ParseConstants::occurs_one_or_more
            );
        }
    break;

  case 588: // ItemType: GeneralizedAtomicType
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 589: // ItemType: KindTest
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 590: // ItemType: "'item'" "'('" "')'"
        {
            (yylhs.value.node) = new ItemType( LOC(yylhs.location), true );
        }
    break;

  case 591: // ItemType: "'structured-item'" "'('" "')'"
        {
            (yylhs.value.node) = new StructuredItemType(LOC(yylhs.location));
        }
    break;

  case 592: // ItemType: FunctionTest
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 593: // ItemType: ParenthesizedItemType
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 594: // ItemType: JSONTest
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 595: // TypeList: SequenceType
        {
          TypeList* aTypeList = new TypeList(LOC (yylhs.location));
          aTypeList->push_back(dynamic_cast<SequenceTypeAST *>((yystack_[0].value.node)));
          (yylhs.value.node) = aTypeList;
        }
    break;

  case 596: // TypeList: TypeList "','" SequenceType
        {
          TypeList* aTypeList = dynamic_cast<TypeList *>((yystack_[2].value.node));
          aTypeList->push_back(dynamic_cast<SequenceTypeAST *>((yystack_[0].value.node)));
          (yylhs.value.node) = (yystack_[2].value.node);
        }
    break;

  case 597: // GeneralizedAtomicType: QNAME
    {
      (yylhs.value.node) = new GeneralizedAtomicType( LOC(yylhs.location), static_cast<QName*>((yystack_[0].value.expr)) );
    }
    break;

  case 598: // SimpleType: QNAME
    {
      (yylhs.value.node) = new SimpleType( LOC(yylhs.location), static_cast<QName*>((yystack_[0].value.expr)) );
    }
    break;

  case 599: // KindTest: DocumentTest
    {
      (yylhs.value.node) = (yystack_[0].value.node);
    }
    break;

  case 600: // KindTest: ElementTest
    {
      (yylhs.value.node) = (yystack_[0].value.node);
    }
    break;

  case 601: // KindTest: AttributeTest
    {
      (yylhs.value.node) = (yystack_[0].value.node);
    }
    break;

  case 602: // KindTest: SchemaElementTest
    {
      (yylhs.value.node) = (yystack_[0].value.node);
    }
    break;

  case 603: // KindTest: SchemaAttributeTest
    {
      (yylhs.value.node) = (yystack_[0].value.node);
    }
    break;

  case 604: // KindTest: PITest
    {
      (yylhs.value.node) = (yystack_[0].value.node);
    }
    break;

  case 605: // KindTest: CommentTest
    {
      (yylhs.value.node) = (yystack_[0].value.node);
    }
    break;

  case 606: // KindTest: TextTest
    {
      (yylhs.value.node) = (yystack_[0].value.node);
    }
    break;

  case 607: // KindTest: NamespaceTest
    {
      (yylhs.value.node) = (yystack_[0].value.node);
    }
    break;

  case 608: // KindTest: AnyKindTest
    {
      (yylhs.value.node) = (yystack_[0].value.node);
    }
    break;

  case 609: // AnyKindTest: NODE_KIND_TEST "'('" "')'"
    {
      (yylhs.value.node) = new AnyKindTest( LOC(yylhs.location) );
    }
    break;

  case 610: // NODE_KIND_TEST: "'node'"
    {
      (yylhs.value.expr) = NULL;
    }
    break;

  case 611: // DocumentTest: "'document-node'" "'('" "')'"
    {
      (yylhs.value.node) = new DocumentTest(LOC(yylhs.location));
    }
    break;

  case 612: // DocumentTest: "'document-node'" "'('" ElementTest "')'"
    {
      (yylhs.value.node) = new DocumentTest(LOC(yylhs.location), dynamic_cast<ElementTest*>((yystack_[1].value.node)));
    }
    break;

  case 613: // DocumentTest: "'document-node'" "'('" SchemaElementTest "')'"
    {
      (yylhs.value.node) = new DocumentTest(LOC(yylhs.location), dynamic_cast<SchemaElementTest*>((yystack_[1].value.node)));
    }
    break;

  case 614: // NamespaceTest: "'namespace-node'" "'('" "')'"
    {
      (yylhs.value.node) = new NamespaceTest(LOC(yylhs.location));
    }
    break;

  case 615: // TextTest: "'text'" "'('" "')'"
    {
      (yylhs.value.node) = new TextTest(LOC(yylhs.location));
    }
    break;

  case 616: // CommentTest: "'comment'" "'('" "')'"
    {
      (yylhs.value.node) = new CommentTest(LOC(yylhs.location));
    }
    break;

  case 617: // PITest: "'processing-instruction'" "'('" "')'"
        {
            (yylhs.value.node) = new PITest( LOC(yylhs.location), "" );
        }
    break;

  case 618: // PITest: "'processing-instruction'" "'('" QNAME "')'"
        {
            ERROR_IF_QNAME_NOT_NCNAME((yystack_[1].value.expr), yystack_[1].location);
            (yylhs.value.node) = new PITest( LOC(yylhs.location), static_cast<QName*>((yystack_[1].value.expr)) );
        }
    break;

  case 619: // PITest: "'processing-instruction'" "'('" "'STRING'" "')'"
        {
            (yylhs.value.node) = new PITest( LOC(yylhs.location), SYMTAB((yystack_[1].value.sval)) );
        }
    break;

  case 620: // AttributeTest: "'attribute'" "'('" "')'"
        {
            (yylhs.value.node) = new AttributeTest( LOC(yylhs.location), NULL, NULL );
        }
    break;

  case 621: // AttributeTest: "'attribute'" "'('" QNAME "')'"
        {
            (yylhs.value.node) = new AttributeTest(
                LOC(yylhs.location), static_cast<QName*>((yystack_[1].value.expr)), NULL
            );
        }
    break;

  case 622: // AttributeTest: "'attribute'" "'('" QNAME "','" TypeName "')'"
        {
            (yylhs.value.node) = new AttributeTest(
                LOC(yylhs.location), static_cast<QName*>((yystack_[3].value.expr)), dynamic_cast<TypeName*>((yystack_[1].value.node))
            );
        }
    break;

  case 623: // AttributeTest: "'attribute'" "'('" "'*'" "')'"
        {
            (yylhs.value.node) = new AttributeTest( LOC(yylhs.location), NULL, NULL );
        }
    break;

  case 624: // AttributeTest: "'attribute'" "'('" "'*'" "','" TypeName "')'"
        {
            (yylhs.value.node) = new AttributeTest(
                LOC(yylhs.location), NULL, dynamic_cast<TypeName*>((yystack_[1].value.node))
            );
        }
    break;

  case 625: // SchemaAttributeTest: "'schema-attribute'" "'('" QNAME "')'"
        {
            (yylhs.value.node) = new SchemaAttributeTest( LOC(yylhs.location), static_cast<QName*>((yystack_[1].value.expr)) );
        }
    break;

  case 626: // ElementTest: "'element'" "'('" "')'"
        {
            (yylhs.value.node) = new ElementTest( LOC(yylhs.location), NULL, NULL, true );
        }
    break;

  case 627: // ElementTest: "'element'" "'('" QNAME "')'"
        {
            (yylhs.value.node) = new ElementTest(
                LOC(yylhs.location), static_cast<QName*>((yystack_[1].value.expr)), NULL, true
            );
        }
    break;

  case 628: // ElementTest: "'element'" "'('" QNAME "','" TypeName "')'"
        {
            (yylhs.value.node) = new ElementTest(
                LOC(yylhs.location),
                static_cast<QName*>((yystack_[3].value.expr)),
                dynamic_cast<TypeName*>((yystack_[1].value.node)),
                false
            );
        }
    break;

  case 629: // ElementTest: "'element'" "'('" QNAME "','" TypeName_WITH_HOOK "')'"
        {
            (yylhs.value.node) = new ElementTest(
                LOC(yylhs.location),
                static_cast<QName*>((yystack_[3].value.expr)),
                dynamic_cast<TypeName*>((yystack_[1].value.node)),
                true
            );
        }
    break;

  case 630: // ElementTest: "'element'" "'('" "'*'" "')'"
        {
            (yylhs.value.node) = new ElementTest(
                LOC(yylhs.location), NULL, NULL, true
            );
        }
    break;

  case 631: // ElementTest: "'element'" "'('" "'*'" "','" TypeName "')'"
        {
            (yylhs.value.node) = new ElementTest(
                LOC(yylhs.location), NULL, dynamic_cast<TypeName*>((yystack_[1].value.node)), false
            );
        }
    break;

  case 632: // ElementTest: "'element'" "'('" "'*'" "','" TypeName_WITH_HOOK "')'"
        {
            (yylhs.value.node) = new ElementTest(
                LOC(yylhs.location), NULL, dynamic_cast<TypeName*>((yystack_[1].value.node)), true
            );
        }
    break;

  case 633: // SchemaElementTest: "'schema-element'" "'('" QNAME "')'"
        {
            (yylhs.value.node) = new SchemaElementTest( LOC(yylhs.location), static_cast<QName*>((yystack_[1].value.expr)) );
        }
    break;

  case 634: // TypeName: QNAME
        {
            (yylhs.value.node) = new TypeName( LOC(yylhs.location), static_cast<QName*>((yystack_[0].value.expr)) );
        }
    break;

  case 635: // TypeName_WITH_HOOK: QNAME "'?'"
        {
            (yylhs.value.node) = new TypeName( LOC(yylhs.location), static_cast<QName*>((yystack_[1].value.expr)), true );
        }
    break;

  case 636: // StringLiteral: "'STRING'"
        {
            (yylhs.value.expr) = new StringLiteral( LOC(yylhs.location), SYMTAB((yystack_[0].value.sval)) );
        }
    break;

  case 637: // FunctionTest: AnyFunctionTest
    {
      (yylhs.value.node) = (yystack_[0].value.node);
    }
    break;

  case 638: // FunctionTest: TypedFunctionTest
    {
      (yylhs.value.node) = (yystack_[0].value.node);
    }
    break;

  case 639: // AnyFunctionTest: "'function'" "'('" "'*'" "')'"
    {
      (yylhs.value.node) = new AnyFunctionTest(LOC(yylhs.location));
    }
    break;

  case 640: // TypedFunctionTest: "'function'" "'('" "')'" "'as'" SequenceType
        {
          (yylhs.value.node) = new TypedFunctionTest(LOC (yylhs.location), dynamic_cast<SequenceTypeAST *>((yystack_[0].value.node)));
        }
    break;

  case 641: // TypedFunctionTest: "'function'" "'('" TypeList "')'" "'as'" SequenceType
        {
          (yylhs.value.node) = new TypedFunctionTest(LOC (yylhs.location),
              dynamic_cast<TypeList *>((yystack_[3].value.node)),
              dynamic_cast<SequenceTypeAST *>((yystack_[0].value.node)));
        }
    break;

  case 642: // ParenthesizedItemType: "'('" ItemType "')'"
        {
           (yylhs.value.node) = (yystack_[1].value.node);
        }
    break;

  case 643: // RevalidationDecl: "'declare'" "'revalidation'" "'strict'"
        {
            (yylhs.value.node) = new RevalidationDecl(
                LOC(yylhs.location), StaticContextConsts::strict_validation
            );
        }
    break;

  case 644: // RevalidationDecl: "'declare'" "'revalidation'" "'lax'"
        {
            (yylhs.value.node) = new RevalidationDecl(
                LOC(yylhs.location), StaticContextConsts::lax_validation
            );
        }
    break;

  case 645: // RevalidationDecl: "'declare'" "'revalidation'" "'skip'"
        {
            (yylhs.value.node) = new RevalidationDecl(
                LOC(yylhs.location), StaticContextConsts::skip_validation
            );
        }
    break;

  case 646: // InsertExpr: "'insert'" "'node'" ExprSingle "'into'" ExprSingle
        {
            (yylhs.value.expr) = new InsertExpr( LOC(yylhs.location), store::UpdateConsts::INTO, (yystack_[2].value.expr), (yystack_[0].value.expr) );
        }
    break;

  case 647: // InsertExpr: "'insert'" "'node'" ExprSingle "'as'" "'first'" "'into'" ExprSingle
        {
            (yylhs.value.expr) = new InsertExpr(
                LOC(yylhs.location), store::UpdateConsts::AS_FIRST_INTO, (yystack_[4].value.expr), (yystack_[0].value.expr)
            );
        }
    break;

  case 648: // InsertExpr: "'insert'" "'node'" ExprSingle "'as'" "'last'" "'into'" ExprSingle
        {
            (yylhs.value.expr) = new InsertExpr(
                LOC(yylhs.location), store::UpdateConsts::AS_LAST_INTO, (yystack_[4].value.expr), (yystack_[0].value.expr)
            );
        }
    break;

  case 649: // InsertExpr: "'insert'" "'node'" ExprSingle "'after'" ExprSingle
        {
            (yylhs.value.expr) = new InsertExpr( LOC(yylhs.location), store::UpdateConsts::AFTER, (yystack_[2].value.expr), (yystack_[0].value.expr) );
        }
    break;

  case 650: // InsertExpr: "'insert'" "'node'" ExprSingle "'before'" ExprSingle
        {
            (yylhs.value.expr) = new InsertExpr(
                LOC (yylhs.location), store::UpdateConsts::BEFORE, (yystack_[2].value.expr), (yystack_[0].value.expr)
            );
        }
    break;

  case 651: // InsertExpr: "'insert'" "'nodes'" ExprSingle "'into'" ExprSingle
        {
            (yylhs.value.expr) = new InsertExpr( LOC(yylhs.location), store::UpdateConsts::INTO, (yystack_[2].value.expr), (yystack_[0].value.expr) );
        }
    break;

  case 652: // InsertExpr: "'insert'" "'nodes'" ExprSingle "'as'" "'first'" "'into'" ExprSingle
        {
            (yylhs.value.expr) = new InsertExpr(
                LOC(yylhs.location), store::UpdateConsts::AS_FIRST_INTO, (yystack_[4].value.expr), (yystack_[0].value.expr)
            );
        }
    break;

  case 653: // InsertExpr: "'insert'" "'nodes'" ExprSingle "'as'" "'last'" "'into'" ExprSingle
        {
            (yylhs.value.expr) = new InsertExpr(
                  LOC(yylhs.location), store::UpdateConsts::AS_LAST_INTO, (yystack_[4].value.expr), (yystack_[0].value.expr)
            );
        }
    break;

  case 654: // InsertExpr: "'insert'" "'nodes'" ExprSingle "'after'" ExprSingle
        {
            (yylhs.value.expr) = new InsertExpr(
                LOC (yylhs.location),
                store::UpdateConsts::AFTER, (yystack_[2].value.expr), (yystack_[0].value.expr)
            );
        }
    break;

  case 655: // InsertExpr: "'insert'" "'nodes'" ExprSingle "'before'" ExprSingle
        {
            (yylhs.value.expr) = new InsertExpr(
                LOC (yylhs.location), store::UpdateConsts::BEFORE, (yystack_[2].value.expr), (yystack_[0].value.expr)
            );
        }
    break;

  case 656: // DeleteExpr: "'delete'" "'node'" ExprSingle
        {
            (yylhs.value.expr) = new DeleteExpr( LOC (yylhs.location), (yystack_[0].value.expr) );
        }
    break;

  case 657: // DeleteExpr: "'delete'" "'nodes'" ExprSingle
        {
            (yylhs.value.expr) = new DeleteExpr( LOC (yylhs.location), (yystack_[0].value.expr) );
        }
    break;

  case 658: // ReplaceExpr: "'replace'" "'node'" ExprSingle "'with'" ExprSingle
        {
            (yylhs.value.expr) = new ReplaceExpr(
                LOC(yylhs.location), store::UpdateConsts::NODE, (yystack_[2].value.expr), (yystack_[0].value.expr)
            );
        }
    break;

  case 659: // ReplaceExpr: "'replace'" "'value'" "'of'" "'node'" ExprSingle "'with'" ExprSingle
        {
            (yylhs.value.expr) = new ReplaceExpr(
                LOC(yylhs.location), store::UpdateConsts::VALUE_OF_NODE, (yystack_[2].value.expr), (yystack_[0].value.expr)
            );
        }
    break;

  case 660: // RenameExpr: "'rename'" "'node'" ExprSingle "'as'" ExprSingle
        {
            (yylhs.value.expr) = new RenameExpr( LOC (yylhs.location), (yystack_[2].value.expr), (yystack_[0].value.expr) );
        }
    break;

  case 661: // TransformExpr: "'copy'" "'$'" VarNameList "'modify'" ExprSingle ReturnOrSelect ExprSingle
    {
      CopyVarList *cvl = dynamic_cast<CopyVarList*>((yystack_[4].value.expr));
      (yylhs.value.expr) = new TransformExpr( LOC(yylhs.location), cvl, (yystack_[2].value.expr), (yystack_[0].value.expr) );
    }
    break;

  case 662: // VarNameList: VarNameDecl
    {
      CopyVarList* lList = new CopyVarList(LOC(yylhs.location));
      lList->push_back (dynamic_cast<VarBinding*> ((yystack_[0].value.expr)));
      (yylhs.value.expr) = lList;
    }
    break;

  case 663: // VarNameList: VarNameList "','" "'$'" VarNameDecl
    {
      CopyVarList* lList = dynamic_cast<CopyVarList*>((yystack_[3].value.expr));
      VarBinding* lBinding = dynamic_cast<VarBinding*>((yystack_[0].value.expr));
      lList->push_back(lBinding);
      (yylhs.value.expr) = lList;
    }
    break;

  case 664: // VarNameDecl: QNAME "':='" ExprSingle
    {
       (yylhs.value.expr) = new VarBinding(LOC(yylhs.location), static_cast<QName*>((yystack_[2].value.expr)), (yystack_[0].value.expr));
    }
    break;

  case 665: // TryExpr: "'try'" "'{'" Expr "'}'" CatchListExpr
        {
            (yylhs.value.expr) = new TryExpr( LOC(yylhs.location), (yystack_[2].value.expr), (yystack_[0].value.expr) );
        }
    break;

  case 666: // CatchListExpr: CatchExpr
        {
            CatchListExpr *cle = new CatchListExpr( LOC(yylhs.location) );
            cle->push_back( static_cast<CatchExpr*>((yystack_[0].value.expr)) );
            (yylhs.value.expr) = cle;
        }
    break;

  case 667: // CatchListExpr: CatchListExpr CatchExpr
        {
            CatchListExpr *cle = dynamic_cast<CatchListExpr*>((yystack_[1].value.expr));
            if ( cle )
                cle->push_back( static_cast<CatchExpr*>((yystack_[0].value.expr)) );
            (yylhs.value.expr) = (yystack_[1].value.expr);
        }
    break;

  case 668: // CatchExpr: "'catch'" NameTestList BracedExpr
    {
       (yylhs.value.expr) = new CatchExpr(LOC(yylhs.location), *(yystack_[1].value.name_test_list), (yystack_[0].value.expr));
       delete (yystack_[1].value.name_test_list);
    }
    break;

  case 669: // BracedExpr: "'{'" Expr "'}'"
    {
      (yylhs.value.expr) = (yystack_[1].value.expr);
    }
    break;

  case 670: // NameTestList: NameTest
        {
            CatchExpr::NameTestList *ntl = new CatchExpr::NameTestList;
            ntl->push_back( static_cast<NameTest*>((yystack_[0].value.node)) );
            (yylhs.value.name_test_list) = ntl;
        }
    break;

  case 671: // NameTestList: NameTestList "'|'" NameTest
        {
            CatchExpr::NameTestList *ntl =
                static_cast<CatchExpr::NameTestList*>((yystack_[2].value.name_test_list));
            ntl->push_back( static_cast<NameTest*>((yystack_[0].value.node)) );
            (yylhs.value.name_test_list) = ntl;
        }
    break;

  case 672: // FTSelection: FTOr opt_FTPosFilter_list
        {
            (yylhs.value.node) = new FTSelection( LOC(yylhs.location), (yystack_[1].value.node), (yystack_[0].value.pos_filter_list) );
            delete (yystack_[0].value.pos_filter_list);
        }
    break;

  case 673: // opt_FTPosFilter_list: %empty
        {
            (yylhs.value.pos_filter_list) = NULL;
        }
    break;

  case 674: // opt_FTPosFilter_list: FTPosFilter_list
        {
            (yylhs.value.pos_filter_list) = (yystack_[0].value.pos_filter_list);
        }
    break;

  case 675: // FTPosFilter_list: FTPosFilter
        {
            (yylhs.value.pos_filter_list) = new FTSelection::pos_filter_list_t;
            (yylhs.value.pos_filter_list)->push_back( dynamic_cast<FTPosFilter*>((yystack_[0].value.node)) );
        }
    break;

  case 676: // FTPosFilter_list: FTPosFilter_list FTPosFilter
        {
            (yystack_[1].value.pos_filter_list)->push_back( dynamic_cast<FTPosFilter*>((yystack_[0].value.node)) );
            (yylhs.value.pos_filter_list) = (yystack_[1].value.pos_filter_list);
        }
    break;

  case 677: // FTOr: FTAnd
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 678: // FTOr: FTOr "'ftor'" FTAnd
        {
            (yylhs.value.node) = new FTOr( LOC(yylhs.location), (yystack_[2].value.node), (yystack_[0].value.node) );
        }
    break;

  case 679: // FTAnd: FTMildNot
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 680: // FTAnd: FTAnd "'ftand'" FTMildNot
        {
            (yylhs.value.node) = new FTAnd( LOC(yylhs.location), (yystack_[2].value.node), (yystack_[0].value.node) );
        }
    break;

  case 681: // FTMildNot: FTUnaryNot
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 682: // FTMildNot: FTMildNot "'not'" "'in'" FTUnaryNot
        {
            (yylhs.value.node) = new FTMildNot( LOC(yylhs.location), (yystack_[3].value.node), (yystack_[0].value.node) );
        }
    break;

  case 683: // FTUnaryNot: FTPrimaryWithOptions
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 684: // FTUnaryNot: "'ftnot'" FTPrimaryWithOptions
        {
            (yylhs.value.node) = new FTUnaryNot(
                LOC(yylhs.location), dynamic_cast<FTPrimaryWithOptions*>((yystack_[0].value.node))
            );
        }
    break;

  case 685: // FTPrimaryWithOptions: FTPrimary opt_FTMatchOptions opt_FTWeight
        {
            (yylhs.value.node) = new FTPrimaryWithOptions(
                LOC(yylhs.location),
                dynamic_cast<FTPrimary*>((yystack_[2].value.node)),
                dynamic_cast<FTMatchOptions*>((yystack_[1].value.node)),
                dynamic_cast<FTWeight*>((yystack_[0].value.node))
            );
        }
    break;

  case 686: // opt_FTMatchOptions: %empty
        {
            (yylhs.value.node) = NULL;
        }
    break;

  case 687: // opt_FTMatchOptions: FTMatchOptions
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 688: // opt_FTWeight: %empty
        {
            (yylhs.value.node) = NULL;
        }
    break;

  case 689: // opt_FTWeight: FTWeight
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 690: // FTWeight: "'weight'" "'{'" Expr "'}'"
        {
            (yylhs.value.node) = new FTWeight( LOC(yylhs.location), dynamic_cast<exprnode*>((yystack_[1].value.expr)) );
        }
    break;

  case 691: // FTPrimary: FTWords opt_FTTimes
        {
            (yylhs.value.node) = new FTWordsTimes(
                LOC(yylhs.location),
                dynamic_cast<FTWords*>((yystack_[1].value.node)),
                dynamic_cast<FTTimes*>((yystack_[0].value.node))
            );
        }
    break;

  case 692: // FTPrimary: "'('" FTSelection "')'"
        {
            (yylhs.value.node) = (yystack_[1].value.node);
        }
    break;

  case 693: // FTPrimary: FTExtensionSelection
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 694: // opt_FTTimes: %empty
        {
            (yylhs.value.node) = NULL;
        }
    break;

  case 695: // opt_FTTimes: FTTimes
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 696: // FTExtensionSelection: Pragma_list "'{'" opt_FTSelection "'}'"
        {
            (yylhs.value.node) = new FTExtensionSelection(
                LOC(yylhs.location),
                dynamic_cast<PragmaList*>((yystack_[3].value.node)),
                dynamic_cast<FTSelection*>((yystack_[1].value.node))
            );
        }
    break;

  case 697: // opt_FTSelection: %empty
        {
            (yylhs.value.node) = NULL;
        }
    break;

  case 698: // opt_FTSelection: FTSelection
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 699: // FTWords: FTWordsValue opt_FTAnyallOption
        {
            (yylhs.value.node) = new FTWords(
                LOC(yylhs.location),
                dynamic_cast<FTWordsValue*>((yystack_[1].value.node)),
                dynamic_cast<FTAnyallOption*>((yystack_[0].value.node))
            );
        }
    break;

  case 700: // FTWordsValue: Literal
        {
            (yylhs.value.node) = new FTWordsValue(
                LOC(yylhs.location), static_cast<StringLiteral*>((yystack_[0].value.expr)), NULL
            );
        }
    break;

  case 701: // FTWordsValue: "'{'" Expr "'}'"
        {
            (yylhs.value.node) = new FTWordsValue(
                LOC(yylhs.location), NULL, dynamic_cast<exprnode*>((yystack_[1].value.expr))
            );
        }
    break;

  case 702: // opt_FTAnyallOption: %empty
        {
            (yylhs.value.node) = new FTAnyallOption( LOC(yylhs.location), ft_anyall_mode::any );
        }
    break;

  case 703: // opt_FTAnyallOption: FTAnyallOption
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 704: // FTAnyallOption: "'any'" opt_word
        {
            (yylhs.value.node) = new FTAnyallOption( LOC(yylhs.location), (yystack_[0].value.ft_anyall_value) );
        }
    break;

  case 705: // FTAnyallOption: "'all'" opt_words
        {
            (yylhs.value.node) = new FTAnyallOption( LOC(yylhs.location), (yystack_[0].value.ft_anyall_value) );
        }
    break;

  case 706: // FTAnyallOption: "'phrase'"
        {
            (yylhs.value.node) = new FTAnyallOption( LOC(yylhs.location), ft_anyall_mode::phrase );
        }
    break;

  case 707: // opt_word: %empty
        {
            (yylhs.value.ft_anyall_value) = ft_anyall_mode::any;
        }
    break;

  case 708: // opt_word: "'word'"
        {
            (yylhs.value.ft_anyall_value) = ft_anyall_mode::any_word;
        }
    break;

  case 709: // opt_words: %empty
        {
            (yylhs.value.ft_anyall_value) = ft_anyall_mode::all;
        }
    break;

  case 710: // opt_words: "'words'"
        {
            (yylhs.value.ft_anyall_value) = ft_anyall_mode::all_words;
        }
    break;

  case 711: // FTPosFilter: FTOrder
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 712: // FTPosFilter: FTWindow
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 713: // FTPosFilter: FTDistance
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 714: // FTPosFilter: FTScope
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 715: // FTPosFilter: FTContent
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 716: // FTOrder: "'ordered'"
        {
            (yylhs.value.node) = new FTOrder( LOC(yylhs.location) );
        }
    break;

  case 717: // FTWindow: "'window'" AdditiveExpr FTUnit
        {
            (yylhs.value.node) = new FTWindow(
                LOC(yylhs.location),
                static_cast<AdditiveExpr*>((yystack_[1].value.expr)),
                static_cast<FTUnit*>((yystack_[0].value.node))
            );
        }
    break;

  case 718: // FTDistance: "'distance'" FTRange FTUnit
        {
            (yylhs.value.node) = new FTDistance(
                LOC(yylhs.location),
                dynamic_cast<FTRange*>((yystack_[1].value.node)),
                dynamic_cast<FTUnit*>((yystack_[0].value.node))
            );
        }
    break;

  case 719: // FTUnit: "'words'"
        {
            (yylhs.value.node) = new FTUnit( LOC(yylhs.location), ft_unit::words );
        }
    break;

  case 720: // FTUnit: "'sentences'"
        {
            (yylhs.value.node) = new FTUnit( LOC(yylhs.location), ft_unit::sentences );
        }
    break;

  case 721: // FTUnit: "'paragraphs'"
        {
            (yylhs.value.node) = new FTUnit( LOC(yylhs.location), ft_unit::paragraphs );
        }
    break;

  case 722: // FTMatchOptions: "'using'" FTMatchOption
        {
            FTMatchOptions *mo = new FTMatchOptions( LOC(yylhs.location) );
            mo->push_back( dynamic_cast<FTMatchOption*>((yystack_[0].value.node)) );
            (yylhs.value.node) = mo;
        }
    break;

  case 723: // FTMatchOptions: FTMatchOptions "'using'" FTMatchOption
        {
            FTMatchOptions *mo = dynamic_cast<FTMatchOptions*>((yystack_[2].value.node));
            mo->push_back( dynamic_cast<FTMatchOption*>((yystack_[0].value.node)) );
            (yylhs.value.node) = (yystack_[2].value.node);
        }
    break;

  case 724: // FTMatchOption: FTCaseOption
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 725: // FTMatchOption: FTDiacriticsOption
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 726: // FTMatchOption: FTExtensionOption
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 727: // FTMatchOption: FTLanguageOption
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 728: // FTMatchOption: FTStemOption
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 729: // FTMatchOption: FTStopWordOption
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 730: // FTMatchOption: FTThesaurusOption
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 731: // FTMatchOption: FTWildCardOption
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 732: // FTCaseOption: "'case'" "'sensitive'"
        {
            (yylhs.value.node) = new FTCaseOption( LOC(yylhs.location), ft_case_mode::sensitive );
        }
    break;

  case 733: // FTCaseOption: "'case'" "'insensitive'"
        {
            (yylhs.value.node) = new FTCaseOption( LOC(yylhs.location), ft_case_mode::insensitive );
        }
    break;

  case 734: // FTCaseOption: "'lowercase'"
        {
            (yylhs.value.node) = new FTCaseOption( LOC(yylhs.location), ft_case_mode::lower );
        }
    break;

  case 735: // FTCaseOption: "'uppercase'"
        {
            (yylhs.value.node) = new FTCaseOption( LOC(yylhs.location), ft_case_mode::upper );
        }
    break;

  case 736: // FTDiacriticsOption: "'diacritics'" "'sensitive'"
        {
            (yylhs.value.node) = new FTDiacriticsOption(
                LOC(yylhs.location), ft_diacritics_mode::sensitive
            );
        }
    break;

  case 737: // FTDiacriticsOption: "'diacritics'" "'insensitive'"
        {
            (yylhs.value.node) = new FTDiacriticsOption(
                LOC(yylhs.location), ft_diacritics_mode::insensitive
            );
        }
    break;

  case 738: // FTExtensionOption: "'option'" QNAME "'STRING'"
        {
            (yylhs.value.node) = new FTExtensionOption(
                LOC(yylhs.location), static_cast<QName*>((yystack_[1].value.expr)), SYMTAB((yystack_[0].value.sval))
            );
        }
    break;

  case 739: // FTStemOption: "'stemming'"
        {
            (yylhs.value.node) = new FTStemOption( LOC(yylhs.location), ft_stem_mode::stemming );
        }
    break;

  case 740: // FTStemOption: "'no'" "'stemming'"
        {
            (yylhs.value.node) = new FTStemOption( LOC(yylhs.location), ft_stem_mode::no_stemming );
        }
    break;

  case 741: // FTThesaurusOption: "'thesaurus'" FTThesaurusID_or_default
        {
            FTThesaurusOption::thesaurus_id_list_t *til = NULL;
            if ( (yystack_[0].value.node) ) {
                til = new FTThesaurusOption::thesaurus_id_list_t;
                til->push_back( dynamic_cast<FTThesaurusID*>((yystack_[0].value.node)) );
            }
            (yylhs.value.node) = new FTThesaurusOption( LOC(yylhs.location), til, !til );
            delete til;
        }
    break;

  case 742: // FTThesaurusOption: "'thesaurus'" "'('" FTThesaurusID_or_default opt_FTThesaurus_list "')'"
        {
            FTThesaurusOption::thesaurus_id_list_t *til = (yystack_[1].value.thesaurus_id_list);
            if ( (yystack_[2].value.node) ) {
                if ( !til )
                    til = new FTThesaurusOption::thesaurus_id_list_t;
                til->push_back( dynamic_cast<FTThesaurusID*>((yystack_[2].value.node)) );
            }
            (yylhs.value.node) = new FTThesaurusOption( LOC(yylhs.location), til, !(yystack_[2].value.node) );
            delete til;
        }
    break;

  case 743: // FTThesaurusOption: "'no'" "'thesaurus'"
        {
            (yylhs.value.node) = new FTThesaurusOption( LOC(yylhs.location), NULL, false, true );
        }
    break;

  case 744: // FTThesaurusID_or_default: FTThesaurusID
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 745: // FTThesaurusID_or_default: "'default'"
        {
            (yylhs.value.node) = NULL;
        }
    break;

  case 746: // opt_FTThesaurus_list: %empty
        {
            (yylhs.value.thesaurus_id_list) = NULL;
        }
    break;

  case 747: // opt_FTThesaurus_list: "','" FTThesaurus_list
        {
            (yylhs.value.thesaurus_id_list) = (yystack_[0].value.thesaurus_id_list);
        }
    break;

  case 748: // FTThesaurus_list: FTThesaurusID
        {
            (yylhs.value.thesaurus_id_list) = new FTThesaurusOption::thesaurus_id_list_t;
            (yylhs.value.thesaurus_id_list)->push_back( dynamic_cast<FTThesaurusID*>((yystack_[0].value.node)) );
        }
    break;

  case 749: // FTThesaurus_list: FTThesaurus_list "','" FTThesaurusID
        {
            (yystack_[2].value.thesaurus_id_list)->push_back( dynamic_cast<FTThesaurusID*>((yystack_[0].value.node)) );
            (yylhs.value.thesaurus_id_list) = (yystack_[2].value.thesaurus_id_list);
        }
    break;

  case 750: // FTThesaurusID: "'at'" "'STRING'" opt_relationship opt_levels
        {
            (yylhs.value.node) = new FTThesaurusID(
                LOC(yylhs.location), SYMTAB((yystack_[2].value.sval)), SYMTAB((yystack_[1].value.sval)), dynamic_cast<FTRange*>((yystack_[0].value.node))
            );
        }
    break;

  case 751: // opt_relationship: %empty
        {
            (yylhs.value.sval) = 0;
        }
    break;

  case 752: // opt_relationship: "'relationship'" "'STRING'"
        {
            (yylhs.value.sval) = (yystack_[0].value.sval);
        }
    break;

  case 753: // opt_levels: %empty
        {
            (yylhs.value.node) = NULL;
        }
    break;

  case 754: // opt_levels: FTRange "'levels'"
        {
            (yylhs.value.node) = (yystack_[1].value.node);
        }
    break;

  case 755: // FTStopWordOption: "'stop'" "'words'" FTStopWords opt_FTStopWordsInclExcl_list
        {
            (yylhs.value.node) = new FTStopWordOption(
                LOC(yylhs.location),
                dynamic_cast<FTStopWords*>((yystack_[1].value.node)), (yystack_[0].value.incl_excl_list),
                ft_stop_words_mode::with
            );
            delete (yystack_[0].value.incl_excl_list);
        }
    break;

  case 756: // FTStopWordOption: "'stop'" "'words'" "'default'" opt_FTStopWordsInclExcl_list
        {
            (yylhs.value.node) = new FTStopWordOption(
                LOC(yylhs.location), NULL, (yystack_[0].value.incl_excl_list), ft_stop_words_mode::with_default
            );
            delete (yystack_[0].value.incl_excl_list);
        }
    break;

  case 757: // FTStopWordOption: "'no'" "'stop'" "'words'"
        {
            (yylhs.value.node) = new FTStopWordOption(
                LOC(yylhs.location), NULL, NULL, ft_stop_words_mode::without
            );
        }
    break;

  case 758: // FTStopWords: "'at'" "'STRING'"
        {
            (yylhs.value.node) = new FTStopWords( LOC(yylhs.location), SYMTAB((yystack_[0].value.sval)), NULL );
        }
    break;

  case 759: // FTStopWords: "'('" STRING_LITERAL_list "')'"
        {
            (yylhs.value.node) = new FTStopWords( LOC(yylhs.location), "", (yystack_[1].value.strlist) );
        }
    break;

  case 760: // STRING_LITERAL_list: "'STRING'"
        {
            string_list_t *sl = new string_list_t;
            sl->push_back( SYMTAB((yystack_[0].value.sval)) );
            (yylhs.value.strlist) = sl;
        }
    break;

  case 761: // STRING_LITERAL_list: STRING_LITERAL_list "','" "'STRING'"
        {
            if ( (yystack_[2].value.strlist) )
                (yystack_[2].value.strlist)->push_back( SYMTAB((yystack_[0].value.sval)) );
            (yylhs.value.strlist) = (yystack_[2].value.strlist);
        }
    break;

  case 762: // opt_FTStopWordsInclExcl_list: %empty
        {
            (yylhs.value.incl_excl_list) = NULL;
        }
    break;

  case 763: // opt_FTStopWordsInclExcl_list: FTStopWordsInclExcl_list
        {
            (yylhs.value.incl_excl_list) = (yystack_[0].value.incl_excl_list);
        }
    break;

  case 764: // FTStopWordsInclExcl_list: FTStopWordsInclExcl
        {
            (yylhs.value.incl_excl_list) = new FTStopWordOption::incl_excl_list_t;
            (yylhs.value.incl_excl_list)->push_back( dynamic_cast<FTStopWordsInclExcl*>((yystack_[0].value.node)) );
        }
    break;

  case 765: // FTStopWordsInclExcl_list: FTStopWordsInclExcl_list FTStopWordsInclExcl
        {
            FTStopWordOption::incl_excl_list_t *iel = (yystack_[1].value.incl_excl_list);
            if ( !iel )
                iel = new FTStopWordOption::incl_excl_list_t;
            iel->push_back( dynamic_cast<FTStopWordsInclExcl*>((yystack_[0].value.node)) );
            (yylhs.value.incl_excl_list) = iel;
        }
    break;

  case 766: // FTStopWordsInclExcl: "'union'" FTStopWords
        {
            (yylhs.value.node) = new FTStopWordsInclExcl(
                LOC(yylhs.location),
                dynamic_cast<FTStopWords*>((yystack_[0].value.node)),
                ft_stop_words_unex::union_
            );
        }
    break;

  case 767: // FTStopWordsInclExcl: "'except'" FTStopWords
        {
            (yylhs.value.node) = new FTStopWordsInclExcl(
                LOC(yylhs.location),
                dynamic_cast<FTStopWords*>((yystack_[0].value.node)),
                ft_stop_words_unex::except
            );
        }
    break;

  case 768: // FTLanguageOption: "'language'" "'STRING'"
        {
            (yylhs.value.node) = new FTLanguageOption( LOC(yylhs.location), SYMTAB((yystack_[0].value.sval)) );
        }
    break;

  case 769: // FTWildCardOption: "'wildcards'"
        {
            (yylhs.value.node) = new FTWildCardOption( LOC(yylhs.location), ft_wild_card_mode::with );
        }
    break;

  case 770: // FTWildCardOption: "'no'" "'wildcards'"
        {
            (yylhs.value.node) = new FTWildCardOption( LOC(yylhs.location), ft_wild_card_mode::without );
        }
    break;

  case 771: // FTContent: "'at'" "'start'"
        {
            (yylhs.value.node) = new FTContent( LOC(yylhs.location), ft_content_mode::at_start );
        }
    break;

  case 772: // FTContent: "'at'" "'end'"
        {
            (yylhs.value.node) = new FTContent( LOC(yylhs.location), ft_content_mode::at_end );
        }
    break;

  case 773: // FTContent: "'entire'" "'content'"
        {
            (yylhs.value.node) = new FTContent( LOC(yylhs.location), ft_content_mode::entire );
        }
    break;

  case 774: // FTTimes: "'occurs'" FTRange "'times'"
        {
            (yylhs.value.node) = new FTTimes( LOC(yylhs.location), dynamic_cast<FTRange*>((yystack_[1].value.node)) );
        }
    break;

  case 775: // FTRange: "'exactly'" AdditiveExpr
        {
            (yylhs.value.node) = new FTRange( LOC(yylhs.location), ft_range_mode::exactly, (yystack_[0].value.expr) );
        }
    break;

  case 776: // FTRange: "'at'" "'least'" AdditiveExpr
        {
            (yylhs.value.node) = new FTRange( LOC(yylhs.location), ft_range_mode::at_least, (yystack_[0].value.expr) );
        }
    break;

  case 777: // FTRange: "'at'" "'most'" AdditiveExpr
        {
            (yylhs.value.node) = new FTRange( LOC(yylhs.location), ft_range_mode::at_most, (yystack_[0].value.expr) );
        }
    break;

  case 778: // FTRange: "'from'" AdditiveExpr "'to'" AdditiveExpr
        {
            (yylhs.value.node) = new FTRange( LOC(yylhs.location), ft_range_mode::from_to, (yystack_[2].value.expr), (yystack_[0].value.expr) );
        }
    break;

  case 779: // FTScope: "'same'" FTBigUnit
        {
            (yylhs.value.node) = new FTScope(
                LOC(yylhs.location),
                ft_scope::same,
                dynamic_cast<FTBigUnit*>((yystack_[0].value.node))
            );
        }
    break;

  case 780: // FTScope: "'different'" FTBigUnit
        {
            (yylhs.value.node) = new FTScope(
                LOC(yylhs.location),
                ft_scope::different,
                dynamic_cast<FTBigUnit*>((yystack_[0].value.node))
            );
        }
    break;

  case 781: // FTBigUnit: "'sentence'"
        {
            (yylhs.value.node) = new FTBigUnit( LOC(yylhs.location), ft_big_unit::sentence );
        }
    break;

  case 782: // FTBigUnit: "'paragraph'"
        {
            (yylhs.value.node) = new FTBigUnit( LOC(yylhs.location), ft_big_unit::paragraph );
        }
    break;

  case 783: // FTIgnoreOption: "'without'" "'content'" UnionExpr
        {
            (yylhs.value.node) = new FTIgnoreOption( LOC(yylhs.location), static_cast<UnionExpr*>((yystack_[0].value.expr)) );
        }
    break;

  case 784: // JSONArrayConstructor: "'['" "']'"
        {
          (yylhs.value.expr) = new JSONArrayConstructor( LOC(yylhs.location), NULL );
        }
    break;

  case 785: // JSONArrayConstructor: "'['" Expr "']'"
        {
          (yylhs.value.expr) = new JSONArrayConstructor( LOC(yylhs.location), (yystack_[1].value.expr) );
        }
    break;

  case 786: // JSONSimpleObjectUnion: "'{|'" "'|}'"
        {
          (yylhs.value.expr) = new JSONObjectConstructor(LOC(yylhs.location), NULL, false);
        }
    break;

  case 787: // JSONSimpleObjectUnion: "'{|'" Expr "'|}'"
        {
          (yylhs.value.expr) = new JSONObjectConstructor(LOC(yylhs.location), (yystack_[1].value.expr), false);
        }
    break;

  case 788: // JSONAccumulatorObjectUnion: "'{['" "']}'"
        {
          (yylhs.value.expr) = new JSONObjectConstructor(LOC(yylhs.location), NULL, true);
        }
    break;

  case 789: // JSONAccumulatorObjectUnion: "'{['" Expr "']}'"
        {
          (yylhs.value.expr) = new JSONObjectConstructor(LOC(yylhs.location), (yystack_[1].value.expr), true);
        }
    break;

  case 790: // JSONObjectConstructor: "'{'" JSONPairList "'}'"
        {
          (yylhs.value.expr) = new JSONDirectObjectConstructor(LOC(yylhs.location),
                                               dynamic_cast<JSONPairList*>((yystack_[1].value.node)));
        }
    break;

  case 791: // JSONPairList: ExprSingle "':'" ExprSingle
        {
          JSONPairList* jpl = new JSONPairList(LOC(yylhs.location));
          jpl->push_back(new JSONPairConstructor(LOC(yylhs.location), (yystack_[2].value.expr), (yystack_[0].value.expr)));
          (yylhs.value.node) = jpl;
        }
    break;

  case 792: // JSONPairList: JSONPairList "','" ExprSingle "':'" ExprSingle
        {
          JSONPairList* jpl = dynamic_cast<JSONPairList*>((yystack_[4].value.node));
          assert(jpl);
          jpl->push_back(new JSONPairConstructor(LOC(yylhs.location), (yystack_[2].value.expr), (yystack_[0].value.expr)));
          (yylhs.value.node) = jpl;
        }
    break;

  case 793: // JSONInsertExpr: "'insert'" "'json'" ExprSingle "'into'" ExprSingle
        {
          (yylhs.value.expr) = new JSONObjectInsertExpr(LOC(yylhs.location),
                                        (yystack_[2].value.expr),
                                        (yystack_[0].value.expr));
        }
    break;

  case 794: // JSONInsertExpr: "'insert'" "'json'" JSONPairList "'into'" ExprSingle
        {
          JSONPairList* jpl = dynamic_cast<JSONPairList*>((yystack_[2].value.node));
          (yylhs.value.expr) = new JSONObjectInsertExpr(
              LOC(yylhs.location),
              new JSONDirectObjectConstructor(
                  LOC(yylhs.location),
                  jpl),
              (yystack_[0].value.expr));
        }
    break;

  case 795: // JSONInsertExpr: "'insert'" "'json'" ExprSingle "'into'" ExprSingle "'at'" "'position'" ExprSingle
        {
          (yylhs.value.expr) = new JSONArrayInsertExpr(LOC(yylhs.location), (yystack_[5].value.expr), (yystack_[3].value.expr), (yystack_[0].value.expr));
        }
    break;

  case 796: // JSONAppendExpr: "'append'" "'json'" ExprSingle "'into'" ExprSingle
        {
          (yylhs.value.expr) = new JSONArrayAppendExpr(LOC(yylhs.location), (yystack_[2].value.expr), (yystack_[0].value.expr));
        }
    break;

  case 797: // JSONDeleteExpr: "'delete'" "'json'" PostfixExpr
        {
          rchandle<exprnode> lPostfixExpr = (yystack_[0].value.expr);
          rchandle<exprnode> lTargetExpr;
          rchandle<exprnode> lSelectorExpr;
          string lError;
          if (!convert_postfix_to_target_and_selector((yystack_[0].value.expr), &lTargetExpr, &lSelectorExpr, &lError))
          {
            error(yystack_[1].location, lError);
            YYERROR;
          }
          (yylhs.value.expr) = new JSONDeleteExpr(
              LOC(yylhs.location),
              lTargetExpr,
              lSelectorExpr);
        }
    break;

  case 798: // JSONRenameExpr: "'rename'" "'json'" PostfixExpr "'as'" ExprSingle
        {
          rchandle<exprnode> lPostfixExpr = (yystack_[2].value.expr);
          rchandle<exprnode> lTargetExpr;
          rchandle<exprnode> lSelectorExpr;
          string lError;
          if (!convert_postfix_to_target_and_selector(
              (yystack_[2].value.expr),
              &lTargetExpr,
              &lSelectorExpr,
              &lError,
              false))
          {
            error(yystack_[2].location, lError);
            delete (yystack_[0].value.expr);
            YYERROR;
          }
          (yylhs.value.expr) = new JSONRenameExpr(
                LOC(yylhs.location),
                lTargetExpr,
                lSelectorExpr,
                (yystack_[0].value.expr));
        }
    break;

  case 799: // JSONReplaceExpr: "'replace'" "'value'" "'of'" "'json'" PostfixExpr "'with'" ExprSingle
        {
          rchandle<exprnode> lPostfixExpr = (yystack_[2].value.expr);
          rchandle<exprnode> lTargetExpr;
          rchandle<exprnode> lSelectorExpr;
          string lError;
          if (!convert_postfix_to_target_and_selector((yystack_[2].value.expr), &lTargetExpr, &lSelectorExpr, &lError))
          {
            error(yystack_[2].location, lError);
            delete (yystack_[0].value.expr);
            YYERROR;
          }
          (yylhs.value.expr) = new JSONReplaceExpr(
                LOC(yylhs.location),
                lTargetExpr,
                lSelectorExpr,
                (yystack_[0].value.expr));
        }
    break;

  case 800: // JSONTest: "'json-item'" "'('" "')'"
        {
          (yylhs.value.node) = new JSON_Test(LOC(yylhs.location), store::StoreConsts::jsonItem);
        }
    break;

  case 801: // JSONTest: "'array'" "'('" "')'"
        {
          (yylhs.value.node) = new JSON_Test(LOC(yylhs.location), store::StoreConsts::jsonArray);
        }
    break;

  case 802: // JSONTest: "'object'" "'('" "')'"
        {
          (yylhs.value.node) = new JSON_Test(LOC(yylhs.location), store::StoreConsts::jsonObject);
        }
    break;

  case 803: // URI_LITERAL: "'STRING'"
        { (yylhs.value.sval) = (yystack_[0].value.sval); }
    break;

  case 804: // QNAME: FUNCTION_NAME
        { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 805: // QNAME: "'NCName_sval'"
                  { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB((yystack_[0].value.sval))); }
    break;

  case 806: // QNAME: "'attribute'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("attribute"))); }
    break;

  case 807: // QNAME: "'comment'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("comment"))); }
    break;

  case 808: // QNAME: "'document-node'"
                    { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("document-node"))); }
    break;

  case 809: // QNAME: "'namespace-node'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("namespace-node"))); }
    break;

  case 810: // QNAME: "'element'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("element"))); }
    break;

  case 811: // QNAME: "'item'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("item"))); }
    break;

  case 812: // QNAME: "'if'"
         { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("if"))); }
    break;

  case 813: // QNAME: "'node'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("node"))); }
    break;

  case 814: // QNAME: "'processing-instruction'"
                             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("processing-instruction"))); }
    break;

  case 815: // QNAME: "'schema-attribute'"
                       { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("schema-attribute"))); }
    break;

  case 816: // QNAME: "'schema-element'"
                     { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("schema-element"))); }
    break;

  case 817: // QNAME: "'text'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("text"))); }
    break;

  case 818: // QNAME: "'typeswitch'"
                 { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("typeswitch"))); }
    break;

  case 819: // QNAME: "'switch'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("switch"))); }
    break;

  case 820: // QNAME: "'empty-sequence'"
                     { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("empty-sequence"))); }
    break;

  case 821: // QNAME: "'while'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("while"))); }
    break;

  case 822: // QNAME: "'json'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("json"))); }
    break;

  case 823: // QNAME: "'array'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("array"))); }
    break;

  case 824: // QNAME: "'object'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("object"))); }
    break;

  case 825: // QNAME: "'json-item'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("json-item"))); }
    break;

  case 826: // QNAME: "'structured-item'"
                      { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("structured-item"))); }
    break;

  case 827: // FUNCTION_NAME: EQNAME
        { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 828: // FUNCTION_NAME: "'QName'"
                 { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB((yystack_[0].value.sval))); }
    break;

  case 829: // FUNCTION_NAME: "'xquery'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("xquery"))); }
    break;

  case 830: // FUNCTION_NAME: "'empty'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("empty"))); }
    break;

  case 831: // FUNCTION_NAME: "'boundary-space'"
                     { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("boundary-space"))); }
    break;

  case 832: // FUNCTION_NAME: "'ft-option'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("ft-option"))); }
    break;

  case 833: // FUNCTION_NAME: "'base-uri'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("base-uri"))); }
    break;

  case 834: // FUNCTION_NAME: "'lax'"
          { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("lax"))); }
    break;

  case 835: // FUNCTION_NAME: "'strict'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("strict"))); }
    break;

  case 836: // FUNCTION_NAME: "'idiv'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("idiv"))); }
    break;

  case 837: // FUNCTION_NAME: "'document'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("document"))); }
    break;

  case 838: // FUNCTION_NAME: "'ftnot'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("ftnot"))); }
    break;

  case 839: // FUNCTION_NAME: "'not'"
          { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("not"))); }
    break;

  case 840: // FUNCTION_NAME: "'sensitive'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("sensitive"))); }
    break;

  case 841: // FUNCTION_NAME: "'insensitive'"
                  { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("insensitive"))); }
    break;

  case 842: // FUNCTION_NAME: "'diacritics'"
                 { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("diacritics"))); }
    break;

  case 843: // FUNCTION_NAME: "'without'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("without"))); }
    break;

  case 844: // FUNCTION_NAME: "'stemming'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("stemming"))); }
    break;

  case 845: // FUNCTION_NAME: "'thesaurus'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("thesaurus"))); }
    break;

  case 846: // FUNCTION_NAME: "'stop'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("stop"))); }
    break;

  case 847: // FUNCTION_NAME: "'wildcards'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("wildcards"))); }
    break;

  case 848: // FUNCTION_NAME: "'entire'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("entire"))); }
    break;

  case 849: // FUNCTION_NAME: "'content'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("content"))); }
    break;

  case 850: // FUNCTION_NAME: "'word'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("word"))); }
    break;

  case 851: // FUNCTION_NAME: "'start'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("start"))); }
    break;

  case 852: // FUNCTION_NAME: "'end'"
          { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("end"))); }
    break;

  case 853: // FUNCTION_NAME: "'most'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("most"))); }
    break;

  case 854: // FUNCTION_NAME: "'skip'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("skip"))); }
    break;

  case 855: // FUNCTION_NAME: "'copy'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("copy"))); }
    break;

  case 856: // FUNCTION_NAME: "'general'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("general"))); }
    break;

  case 857: // FUNCTION_NAME: "'value'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("value"))); }
    break;

  case 858: // FUNCTION_NAME: "'eq'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("eq"))); }
    break;

  case 859: // FUNCTION_NAME: "'ne'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("ne"))); }
    break;

  case 860: // FUNCTION_NAME: "'lt'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("lt"))); }
    break;

  case 861: // FUNCTION_NAME: "'le'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("le"))); }
    break;

  case 862: // FUNCTION_NAME: "'gt'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("gt"))); }
    break;

  case 863: // FUNCTION_NAME: "'ge'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("ge"))); }
    break;

  case 864: // FUNCTION_NAME: "'at'"
         { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("at"))); }
    break;

  case 865: // FUNCTION_NAME: "'context'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("context"))); }
    break;

  case 866: // FUNCTION_NAME: "'variable'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("variable"))); }
    break;

  case 867: // FUNCTION_NAME: "'return'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("return"))); }
    break;

  case 868: // FUNCTION_NAME: "'for'"
          { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("for"))); }
    break;

  case 869: // FUNCTION_NAME: "'from'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("from"))); }
    break;

  case 870: // FUNCTION_NAME: "'allowing'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("allowing"))); }
    break;

  case 871: // FUNCTION_NAME: "'sliding'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("sliding"))); }
    break;

  case 872: // FUNCTION_NAME: "'tumbling'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("tumbling"))); }
    break;

  case 873: // FUNCTION_NAME: "'previous'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("previous"))); }
    break;

  case 874: // FUNCTION_NAME: "'next'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("next"))); }
    break;

  case 875: // FUNCTION_NAME: "'only'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("only"))); }
    break;

  case 876: // FUNCTION_NAME: "'when'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("when"))); }
    break;

  case 877: // FUNCTION_NAME: "'count'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("count"))); }
    break;

  case 878: // FUNCTION_NAME: "'in'"
          { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("in"))); }
    break;

  case 879: // FUNCTION_NAME: "'let'"
          { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("let"))); }
    break;

  case 880: // FUNCTION_NAME: "'where'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("where"))); }
    break;

  case 881: // FUNCTION_NAME: "'by'"
         { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("by"))); }
    break;

  case 882: // FUNCTION_NAME: "'group'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("group"))); }
    break;

  case 883: // FUNCTION_NAME: "'order'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("order"))); }
    break;

  case 884: // FUNCTION_NAME: "'stable'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("stable"))); }
    break;

  case 885: // FUNCTION_NAME: "'ascending'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("ascending"))); }
    break;

  case 886: // FUNCTION_NAME: "'descending'"
                 { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("descending"))); }
    break;

  case 887: // FUNCTION_NAME: "'greatest'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("greatest"))); }
    break;

  case 888: // FUNCTION_NAME: "'least'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("least"))); }
    break;

  case 889: // FUNCTION_NAME: "'collation'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("collation"))); }
    break;

  case 890: // FUNCTION_NAME: "'some'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("some"))); }
    break;

  case 891: // FUNCTION_NAME: "'every'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("every"))); }
    break;

  case 892: // FUNCTION_NAME: "'satisfies'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("satisfies"))); }
    break;

  case 893: // FUNCTION_NAME: "'case'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("case"))); }
    break;

  case 894: // FUNCTION_NAME: "'as'"
         { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("as"))); }
    break;

  case 895: // FUNCTION_NAME: "'then'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("then"))); }
    break;

  case 896: // FUNCTION_NAME: "'else'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("else"))); }
    break;

  case 897: // FUNCTION_NAME: "'or'"
         { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("or"))); }
    break;

  case 898: // FUNCTION_NAME: "'and'"
          { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("and"))); }
    break;

  case 899: // FUNCTION_NAME: "'instance'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("instance"))); }
    break;

  case 900: // FUNCTION_NAME: "'of'"
         { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("of"))); }
    break;

  case 901: // FUNCTION_NAME: "'castable'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("castable"))); }
    break;

  case 902: // FUNCTION_NAME: "'to'"
         { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("to"))); }
    break;

  case 903: // FUNCTION_NAME: "'div'"
          { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("div"))); }
    break;

  case 904: // FUNCTION_NAME: "'mod'"
          { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("mod"))); }
    break;

  case 905: // FUNCTION_NAME: "'union'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("union"))); }
    break;

  case 906: // FUNCTION_NAME: "'intersect'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("intersect"))); }
    break;

  case 907: // FUNCTION_NAME: "'except'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("except"))); }
    break;

  case 908: // FUNCTION_NAME: "'validate'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("validate"))); }
    break;

  case 909: // FUNCTION_NAME: "'type'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("type"))); }
    break;

  case 910: // FUNCTION_NAME: "'cast'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("cast"))); }
    break;

  case 911: // FUNCTION_NAME: "'treat'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("treat"))); }
    break;

  case 912: // FUNCTION_NAME: "'is'"
         { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("is"))); }
    break;

  case 913: // FUNCTION_NAME: "'preserve'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("preserve"))); }
    break;

  case 914: // FUNCTION_NAME: "'strip'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("strip"))); }
    break;

  case 915: // FUNCTION_NAME: "'namespace'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("namespace"))); }
    break;

  case 916: // FUNCTION_NAME: "'external'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("external"))); }
    break;

  case 917: // FUNCTION_NAME: "'encoding'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("encoding"))); }
    break;

  case 918: // FUNCTION_NAME: "'<no preserve>'"
                  { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("no-preserve"))); }
    break;

  case 919: // FUNCTION_NAME: "'inherit'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("inherit"))); }
    break;

  case 920: // FUNCTION_NAME: "'<no inherit>'"
                 { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("no-inherit"))); }
    break;

  case 921: // FUNCTION_NAME: "'declare'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("declare"))); }
    break;

  case 922: // FUNCTION_NAME: "'construction'"
                   { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("construction"))); }
    break;

  case 923: // FUNCTION_NAME: "'ordering'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("ordering"))); }
    break;

  case 924: // FUNCTION_NAME: "'default'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("default"))); }
    break;

  case 925: // FUNCTION_NAME: "'copy-namespaces'"
                      { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("copy-namespaces"))); }
    break;

  case 926: // FUNCTION_NAME: "'option'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("option"))); }
    break;

  case 927: // FUNCTION_NAME: "'version'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("version"))); }
    break;

  case 928: // FUNCTION_NAME: "'import'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("import"))); }
    break;

  case 929: // FUNCTION_NAME: "'schema'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("schema"))); }
    break;

  case 930: // FUNCTION_NAME: "'module'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("module"))); }
    break;

  case 931: // FUNCTION_NAME: "'function'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("function"))); }
    break;

  case 932: // FUNCTION_NAME: "'score'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("score"))); }
    break;

  case 933: // FUNCTION_NAME: "'contains'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("contains"))); }
    break;

  case 934: // FUNCTION_NAME: "'weight'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("weight"))); }
    break;

  case 935: // FUNCTION_NAME: "'window'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("window"))); }
    break;

  case 936: // FUNCTION_NAME: "'distance'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("distance"))); }
    break;

  case 937: // FUNCTION_NAME: "'occurs'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("occurs"))); }
    break;

  case 938: // FUNCTION_NAME: "'times'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("times"))); }
    break;

  case 939: // FUNCTION_NAME: "'same'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("same"))); }
    break;

  case 940: // FUNCTION_NAME: "'different'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("different"))); }
    break;

  case 941: // FUNCTION_NAME: "'lowercase'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("lowercase"))); }
    break;

  case 942: // FUNCTION_NAME: "'uppercase'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("uppercase"))); }
    break;

  case 943: // FUNCTION_NAME: "'relationship'"
                   { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("relationship"))); }
    break;

  case 944: // FUNCTION_NAME: "'levels'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("levels"))); }
    break;

  case 945: // FUNCTION_NAME: "'language'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("language"))); }
    break;

  case 946: // FUNCTION_NAME: "'any'"
          { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("any"))); }
    break;

  case 947: // FUNCTION_NAME: "'all'"
          { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("all"))); }
    break;

  case 948: // FUNCTION_NAME: "'exactly'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("exactly"))); }
    break;

  case 949: // FUNCTION_NAME: "'words'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("words"))); }
    break;

  case 950: // FUNCTION_NAME: "'sentences'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("sentences"))); }
    break;

  case 951: // FUNCTION_NAME: "'sentence'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("sentence"))); }
    break;

  case 952: // FUNCTION_NAME: "'phrase'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("phrase"))); }
    break;

  case 953: // FUNCTION_NAME: "'paragraph'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("paragraph"))); }
    break;

  case 954: // FUNCTION_NAME: "'paragraphs'"
                 { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("paragraphs"))); }
    break;

  case 955: // FUNCTION_NAME: "'modify'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("modify"))); }
    break;

  case 956: // FUNCTION_NAME: "'first'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("first"))); }
    break;

  case 957: // FUNCTION_NAME: "'append'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("append"))); }
    break;

  case 958: // FUNCTION_NAME: "'insert'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("insert"))); }
    break;

  case 959: // FUNCTION_NAME: "'replace'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("replace"))); }
    break;

  case 960: // FUNCTION_NAME: "'rename'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("rename"))); }
    break;

  case 961: // FUNCTION_NAME: "'delete'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("delete"))); }
    break;

  case 962: // FUNCTION_NAME: "'before'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("before"))); }
    break;

  case 963: // FUNCTION_NAME: "'after'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("after"))); }
    break;

  case 964: // FUNCTION_NAME: "'revalidation'"
                   { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("revalidation"))); }
    break;

  case 965: // FUNCTION_NAME: "'with'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("with"))); }
    break;

  case 966: // FUNCTION_NAME: "'nodes'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("nodes"))); }
    break;

  case 967: // FUNCTION_NAME: "'last'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("last"))); }
    break;

  case 968: // FUNCTION_NAME: "'into'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("into"))); }
    break;

  case 969: // FUNCTION_NAME: "'simple'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("simple"))); }
    break;

  case 970: // FUNCTION_NAME: "'sequential'"
                 { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("sequential"))); }
    break;

  case 971: // FUNCTION_NAME: "'updating'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("updating"))); }
    break;

  case 972: // FUNCTION_NAME: "'ordered'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("ordered"))); }
    break;

  case 973: // FUNCTION_NAME: "'unordered'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("unordered"))); }
    break;

  case 974: // FUNCTION_NAME: "'returning'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("returning"))); }
    break;

  case 975: // FUNCTION_NAME: "'exit'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("exit"))); }
    break;

  case 976: // FUNCTION_NAME: "'loop'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("loop"))); }
    break;

  case 977: // FUNCTION_NAME: "'break'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("break"))); }
    break;

  case 978: // FUNCTION_NAME: "'continue'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("continue"))); }
    break;

  case 979: // FUNCTION_NAME: "'try'"
          { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("try"))); }
    break;

  case 980: // FUNCTION_NAME: "'catch'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("catch"))); }
    break;

  case 981: // FUNCTION_NAME: "'using'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("using"))); }
    break;

  case 982: // FUNCTION_NAME: "'set'"
          { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("set"))); }
    break;

  case 983: // FUNCTION_NAME: "'index'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("index"))); }
    break;

  case 984: // FUNCTION_NAME: "'unique'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("unique"))); }
    break;

  case 985: // FUNCTION_NAME: "'non'"
          { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("non"))); }
    break;

  case 986: // FUNCTION_NAME: "'on'"
         { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("on"))); }
    break;

  case 987: // FUNCTION_NAME: "'range'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("range"))); }
    break;

  case 988: // FUNCTION_NAME: "'equality'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("equality"))); }
    break;

  case 989: // FUNCTION_NAME: "'manually'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("manually"))); }
    break;

  case 990: // FUNCTION_NAME: "'automatically'"
                    { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("automatically"))); }
    break;

  case 991: // FUNCTION_NAME: "'maintained'"
                 { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("maintained"))); }
    break;

  case 992: // FUNCTION_NAME: "'decimal-format'"
                     { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("decimal-format"))); }
    break;

  case 993: // FUNCTION_NAME: "'decimal-separator'"
                        { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("decimal-separator"))); }
    break;

  case 994: // FUNCTION_NAME: "'grouping-separator'"
                         { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("grouping-separator"))); }
    break;

  case 995: // FUNCTION_NAME: "'infinity'"
                     { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("infinity"))); }
    break;

  case 996: // FUNCTION_NAME: "'minus-sign'"
                 { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("minus-sign"))); }
    break;

  case 997: // FUNCTION_NAME: "'NaN'"
          { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("NaN"))); }
    break;

  case 998: // FUNCTION_NAME: "'percent'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("percent"))); }
    break;

  case 999: // FUNCTION_NAME: "'per-mille'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("per-mille"))); }
    break;

  case 1000: // FUNCTION_NAME: "'zero-digit'"
                 { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("zero-digit"))); }
    break;

  case 1001: // FUNCTION_NAME: "'digit'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("digit"))); }
    break;

  case 1002: // FUNCTION_NAME: "'pattern-separator'"
                        { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("pattern-separator"))); }
    break;

  case 1003: // FUNCTION_NAME: "'collection'"
                 { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("collection"))); }
    break;

  case 1004: // FUNCTION_NAME: "'const'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("const"))); }
    break;

  case 1005: // FUNCTION_NAME: "'append-only'"
                  { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("append-only"))); }
    break;

  case 1006: // FUNCTION_NAME: "'queue'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("queue"))); }
    break;

  case 1007: // FUNCTION_NAME: "'mutable'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("mutable"))); }
    break;

  case 1008: // FUNCTION_NAME: "'read-only'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("read-only"))); }
    break;

  case 1009: // FUNCTION_NAME: "'integrity'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("integrity"))); }
    break;

  case 1010: // FUNCTION_NAME: "'constraint'"
                 { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("constraint"))); }
    break;

  case 1011: // FUNCTION_NAME: "'check'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("check"))); }
    break;

  case 1012: // FUNCTION_NAME: "'key'"
          { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("key"))); }
    break;

  case 1013: // FUNCTION_NAME: "'foreach'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("foreach"))); }
    break;

  case 1014: // FUNCTION_NAME: "'foreign'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("foreign"))); }
    break;

  case 1015: // FUNCTION_NAME: "'keys'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("keys"))); }
    break;

  case 1016: // FUNCTION_NAME: "'ancestor'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("ancestor"))); }
    break;

  case 1017: // FUNCTION_NAME: "'child'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("child"))); }
    break;

  case 1018: // FUNCTION_NAME: "'descendant'"
                 { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("descendant"))); }
    break;

  case 1019: // FUNCTION_NAME: "'parent'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("parent"))); }
    break;

  case 1020: // FUNCTION_NAME: "'preceding'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("preceding"))); }
    break;

  case 1021: // FUNCTION_NAME: "'self'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("self"))); }
    break;

  case 1022: // FUNCTION_NAME: "'following'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("following"))); }
    break;

  case 1023: // FUNCTION_NAME: "'ancestor-or-self'"
                       { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("ancestor-or-self"))); }
    break;

  case 1024: // FUNCTION_NAME: "'descendant-or-self'"
                         { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("descendant-or-self"))); }
    break;

  case 1025: // FUNCTION_NAME: "'following-sibling'"
                        { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("following-sibling"))); }
    break;

  case 1026: // FUNCTION_NAME: "'preceding-sibling'"
                        { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("preceding-sibling"))); }
    break;

  case 1027: // FUNCTION_NAME: "'position'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("position"))); }
    break;

  case 1028: // EQNAME: "'EQName'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB((yystack_[0].value.sval)), true); }
    break;



            default:
              break;
            }
        }
#if YY_EXCEPTIONS
      catch (const syntax_error& yyexc)
        {
          YYCDEBUG << "Caught exception: " << yyexc.what() << '\n';
          error (yyexc);
          YYERROR;
        }
#endif // YY_EXCEPTIONS
      YY_SYMBOL_PRINT ("-> $$ =", yylhs);
      yypop_ (yylen);
      yylen = 0;

      // Shift the result of the reduction.
      yypush_ (YY_NULLPTR, YY_MOVE (yylhs));
    }
    goto yynewstate;


  /*--------------------------------------.
  | yyerrlab -- here on detecting error.  |
  `--------------------------------------*/
  yyerrlab:
    // If not already recovering from an error, report this error.
    if (!yyerrstatus_)
      {
        ++yynerrs_;
        context yyctx (*this, yyla);
        std::string msg = yysyntax_error_ (yyctx);
        error (yyla.location, YY_MOVE (msg));
      }


    yyerror_range[1].location = yyla.location;
    if (yyerrstatus_ == 3)
      {
        /* If just tried and failed to reuse lookahead token after an
           error, discard it.  */

        // Return failure if at end of input.
        if (yyla.kind () == symbol_kind::S_YYEOF)
          YYABORT;
        else if (!yyla.empty ())
          {
            yy_destroy_ ("Error: discarding", yyla);
            yyla.clear ();
          }
      }

    // Else will try to reuse lookahead token after shifting the error token.
    goto yyerrlab1;


  /*---------------------------------------------------.
  | yyerrorlab -- error raised explicitly by YYERROR.  |
  `---------------------------------------------------*/
  yyerrorlab:
    /* Pacify compilers when the user code never invokes YYERROR and
       the label yyerrorlab therefore never appears in user code.  */
    if (false)
      YYERROR;

    /* Do not reclaim the symbols of the rule whose action triggered
       this YYERROR.  */
    yypop_ (yylen);
    yylen = 0;
    YY_STACK_PRINT ();
    goto yyerrlab1;


  /*-------------------------------------------------------------.
  | yyerrlab1 -- common code for both syntax error and YYERROR.  |
  `-------------------------------------------------------------*/
  yyerrlab1:
    yyerrstatus_ = 3;   // Each real token shifted decrements this.
    // Pop stack until we find a state that shifts the error token.
    for (;;)
      {
        yyn = yypact_[+yystack_[0].state];
        if (!yy_pact_value_is_default_ (yyn))
          {
            yyn += symbol_kind::S_YYerror;
            if (0 <= yyn && yyn <= yylast_
                && yycheck_[yyn] == symbol_kind::S_YYerror)
              {
                yyn = yytable_[yyn];
                if (0 < yyn)
                  break;
              }
          }

        // Pop the current state because it cannot handle the error token.
        if (yystack_.size () == 1)
          YYABORT;

        yyerror_range[1].location = yystack_[0].location;
        yy_destroy_ ("Error: popping", yystack_[0]);
        yypop_ ();
        YY_STACK_PRINT ();
      }
    {
      stack_symbol_type error_token;

      yyerror_range[2].location = yyla.location;
      YYLLOC_DEFAULT (error_token.location, yyerror_range, 2);

      // Shift the error token.
      error_token.state = state_type (yyn);
      yypush_ ("Shifting", YY_MOVE (error_token));
    }
    goto yynewstate;


  /*-------------------------------------.
  | yyacceptlab -- YYACCEPT comes here.  |
  `-------------------------------------*/
  yyacceptlab:
    yyresult = 0;
    goto yyreturn;


  /*-----------------------------------.
  | yyabortlab -- YYABORT comes here.  |
  `-----------------------------------*/
  yyabortlab:
    yyresult = 1;
    goto yyreturn;


  /*-----------------------------------------------------.
  | yyreturn -- parsing is finished, return the result.  |
  `-----------------------------------------------------*/
  yyreturn:
    if (!yyla.empty ())
      yy_destroy_ ("Cleanup: discarding lookahead", yyla);

    /* Do not reclaim the symbols of the rule whose action triggered
       this YYABORT or YYACCEPT.  */
    yypop_ (yylen);
    YY_STACK_PRINT ();
    while (1 < yystack_.size ())
      {
        yy_destroy_ ("Cleanup: popping", yystack_[0]);
        yypop_ ();
      }

    return yyresult;
  }
#if YY_EXCEPTIONS
    catch (...)
      {
        YYCDEBUG << "Exception caught: cleaning lookahead and stack\n";
        // Do not try to display the values of the reclaimed symbols,
        // as their printers might throw an exception.
        if (!yyla.empty ())
          yy_destroy_ (YY_NULLPTR, yyla);

        while (1 < yystack_.size ())
          {
            yy_destroy_ (YY_NULLPTR, yystack_[0]);
            yypop_ ();
          }
        throw;
      }
#endif // YY_EXCEPTIONS
  }

  void
  xquery_parser::error (const syntax_error& yyexc)
  {
    error (yyexc.location, yyexc.what ());
  }

  /* Return YYSTR after stripping away unnecessary quotes and
     backslashes, so that it's suitable for yyerror.  The heuristic is
     that double-quoting is unnecessary unless the string contains an
     apostrophe, a comma, or backslash (other than backslash-backslash).
     YYSTR is taken from yytname.  */
  std::string
  xquery_parser::yytnamerr_ (const char *yystr)
  {
    if (*yystr == '"')
      {
        std::string yyr;
        char const *yyp = yystr;

        for (;;)
          switch (*++yyp)
            {
            case '\'':
            case ',':
              goto do_not_strip_quotes;

            case '\\':
              if (*++yyp != '\\')
                goto do_not_strip_quotes;
              else
                goto append;

            append:
            default:
              yyr += *yyp;
              break;

            case '"':
              return yyr;
            }
      do_not_strip_quotes: ;
      }

    return yystr;
  }

  std::string
  xquery_parser::symbol_name (symbol_kind_type yysymbol)
  {
    return yytnamerr_ (yytname_[yysymbol]);
  }



  // xquery_parser::context.
  xquery_parser::context::context (const xquery_parser& yyparser, const symbol_type& yyla)
    : yyparser_ (yyparser)
    , yyla_ (yyla)
  {}

  int
  xquery_parser::context::expected_tokens (symbol_kind_type yyarg[], int yyargn) const
  {
    // Actual number of expected tokens
    int yycount = 0;

    const int yyn = yypact_[+yyparser_.yystack_[0].state];
    if (!yy_pact_value_is_default_ (yyn))
      {
        /* Start YYX at -YYN if negative to avoid negative indexes in
           YYCHECK.  In other words, skip the first -YYN actions for
           this state because they are default actions.  */
        const int yyxbegin = yyn < 0 ? -yyn : 0;
        // Stay within bounds of both yycheck and yytname.
        const int yychecklim = yylast_ - yyn + 1;
        const int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
        for (int yyx = yyxbegin; yyx < yyxend; ++yyx)
          if (yycheck_[yyx + yyn] == yyx && yyx != symbol_kind::S_YYerror
              && !yy_table_value_is_error_ (yytable_[yyx + yyn]))
            {
              if (!yyarg)
                ++yycount;
              else if (yycount == yyargn)
                return 0;
              else
                yyarg[yycount++] = YY_CAST (symbol_kind_type, yyx);
            }
      }

    if (yyarg && yycount == 0 && 0 < yyargn)
      yyarg[0] = symbol_kind::S_YYEMPTY;
    return yycount;
  }






  int
  xquery_parser::yy_syntax_error_arguments_ (const context& yyctx,
                                                 symbol_kind_type yyarg[], int yyargn) const
  {
    /* There are many possibilities here to consider:
       - If this state is a consistent state with a default action, then
         the only way this function was invoked is if the default action
         is an error action.  In that case, don't check for expected
         tokens because there are none.
       - The only way there can be no lookahead present (in yyla) is
         if this state is a consistent state with a default action.
         Thus, detecting the absence of a lookahead is sufficient to
         determine that there is no unexpected or expected token to
         report.  In that case, just report a simple "syntax error".
       - Don't assume there isn't a lookahead just because this state is
         a consistent state with a default action.  There might have
         been a previous inconsistent state, consistent state with a
         non-default action, or user semantic action that manipulated
         yyla.  (However, yyla is currently not documented for users.)
       - Of course, the expected token list depends on states to have
         correct lookahead information, and it depends on the parser not
         to perform extra reductions after fetching a lookahead from the
         scanner and before detecting a syntax error.  Thus, state merging
         (from LALR or IELR) and default reductions corrupt the expected
         token list.  However, the list is correct for canonical LR with
         one exception: it will still contain any token that will not be
         accepted due to an error action in a later state.
    */

    if (!yyctx.lookahead ().empty ())
      {
        if (yyarg)
          yyarg[0] = yyctx.token ();
        int yyn = yyctx.expected_tokens (yyarg ? yyarg + 1 : yyarg, yyargn - 1);
        return yyn + 1;
      }
    return 0;
  }

  // Generate an error message.
  std::string
  xquery_parser::yysyntax_error_ (const context& yyctx) const
  {
    // Its maximum.
    enum { YYARGS_MAX = 5 };
    // Arguments of yyformat.
    symbol_kind_type yyarg[YYARGS_MAX];
    int yycount = yy_syntax_error_arguments_ (yyctx, yyarg, YYARGS_MAX);

    char const* yyformat = YY_NULLPTR;
    switch (yycount)
      {
#define YYCASE_(N, S)                         \
        case N:                               \
          yyformat = S;                       \
        break
      default: // Avoid compiler warnings.
        YYCASE_ (0, YY_("syntax error"));
        YYCASE_ (1, YY_("syntax error, unexpected %s"));
        YYCASE_ (2, YY_("syntax error, unexpected %s, expecting %s"));
        YYCASE_ (3, YY_("syntax error, unexpected %s, expecting %s or %s"));
        YYCASE_ (4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
        YYCASE_ (5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
#undef YYCASE_
      }

    std::string yyres;
    // Argument number.
    std::ptrdiff_t yyi = 0;
    for (char const* yyp = yyformat; *yyp; ++yyp)
      if (yyp[0] == '%' && yyp[1] == 's' && yyi < yycount)
        {
          yyres += symbol_name (yyarg[yyi++]);
          ++yyp;
        }
      else
        yyres += *yyp;
    return yyres;
  }


  const short xquery_parser::yypact_ninf_ = -1425;

  const short xquery_parser::yytable_ninf_ = -869;

  const short
  xquery_parser::yypact_[] =
  {
    4693, -1425, -1425, -1425, -1425,  6247,  6247,  6247,  6247, -1425,
   -1425,   197,   260, -1425,  1422,    58, -1425, -1425, -1425,   806,
   -1425, -1425, -1425,   292,   360,   810,  4411,   529,   644,   850,
   -1425,    78, -1425,   691, -1425, -1425, -1425, -1425, -1425,   814,
   -1425,   695,   737, -1425, -1425, -1425, -1425,   372, -1425,   815,
   -1425,   780,   832, -1425,   427, -1425, -1425, -1425, -1425, -1425,
   -1425, -1425, -1425, -1425, -1425, -1425, -1425, -1425, -1425,   533,
     660, -1425, -1425, -1425, -1425,   690, 12447, -1425, -1425, -1425,
     853, -1425, -1425, -1425,   863, -1425,   869,   871, -1425, -1425,
   16318, -1425, -1425, -1425, -1425, -1425, -1425, -1425, -1425,   879,
   -1425, -1425,   877,   887, -1425, -1425, -1425, -1425, -1425, -1425,
   -1425,  5317,  8727,  9037, 16318, -1425, -1425,   860, -1425, -1425,
   -1425, -1425,   865, -1425, -1425,   899, 16318, -1425, 13358,   901,
     902, -1425, -1425, -1425,   903, -1425, -1425, 11827, -1425, -1425,
   -1425, -1425, -1425, -1425,   876, -1425, -1425, -1425, -1425, -1425,
   -1425, -1425, -1425,    77,   811, -1425, -1425, -1425, -1425, -1425,
   -1425, -1425, -1425,   -72,   875,   -51, -1425,  -101,   390, -1425,
   -1425, -1425, -1425, -1425, -1425,   915, -1425,   792,   794,   795,
   -1425, -1425,   882,   889, -1425, -1425, -1425, -1425, -1425, -1425,
   -1425, -1425, -1425, -1425, -1425, -1425, -1425, -1425, -1425, -1425,
   -1425, -1425, -1425, -1425, -1425, -1425, -1425, -1425, -1425, -1425,
   -1425, -1425, -1425, -1425, -1425, -1425, -1425, -1425, -1425, -1425,
   -1425, -1425, -1425, -1425, -1425, -1425, -1425, -1425, -1425, -1425,
   -1425, -1425, -1425, -1425, -1425, -1425, -1425, -1425, -1425, -1425,
   -1425, -1425, -1425, -1425, -1425, -1425, -1425,  9347,  9657, -1425,
     727, -1425, -1425, -1425, -1425, -1425,  5005,  6557,  1044, -1425,
    6867, -1425, -1425,   332,    76, -1425, -1425, -1425, -1425, -1425,
   -1425, -1425, -1425, -1425, -1425, -1425, -1425, -1425, -1425,    95,
   -1425, -1425, -1425, -1425, -1425, -1425,   482, -1425, -1425, -1425,
   -1425, -1425, -1425, -1425, -1425,  6247, -1425, -1425, -1425, -1425,
     351, -1425, -1425, -1425, -1425, -1425, -1425, -1425, -1425, -1425,
     180, -1425,   866, -1425, -1425, -1425,   675, -1425,   532, -1425,
   -1425, -1425, -1425, -1425, -1425, -1425,   890,   966, -1425,   950,
     813,   960,   713,   588,   632,    36, -1425,  1013,   864,   963,
     964, 10277, -1425,   878, -1425, -1425,   528, -1425, -1425, 12137,
   -1425,   755, -1425,   911, 12447, -1425,   911, 12447, -1425, -1425,
   -1425,   801, -1425, -1425, -1425, -1425, -1425, -1425, -1425, -1425,
   -1425, -1425, -1425, -1425, -1425, -1425, -1425, -1425, -1425, -1425,
   -1425, -1425, -1425, -1425, -1425, -1425, -1425, -1425, -1425,   912,
   -1425, -1425, -1425, -1425, -1425, -1425, -1425, -1425, -1425, -1425,
   -1425, -1425, -1425, -1425, -1425, -1425, -1425, -1425, -1425, -1425,
   -1425, -1425, -1425, -1425, -1425, -1425, -1425,   785, -1425, -1425,
   -1425, -1425, -1425,   884, -1425,  6247,   888,   891,   894,  6247,
     548,   548,  1043,   717,   747,   480, 16906, 16318,   609,  1032,
   16318,   927,   962,   640, 16318,   763,   836, 16318, 16318,   786,
     507,    82, -1425, -1425, -1425, 16318,  6247,   897,  6247,   140,
   10587, 13654, 16318,   -19,   933, 10587,  1087,   182,   135, 16318,
     974,   953,   989, -1425,   913, 10587, 13950, 16318, 16318, 16318,
    6247,   916, 10587, 10587, 16318,  6247,   944,   945, -1425, -1425,
   -1425, 10587, 14246,   947, -1425,   952, -1425, -1425, -1425, -1425,
     954, -1425,   955, -1425, -1425, -1425, -1425, -1425,   957, -1425,
   -1425, -1425, -1425, -1425, -1425, -1425, -1425, -1425, -1425, -1425,
   -1425, -1425, -1425, -1425, -1425, -1425, -1425, -1425, -1425, -1425,
   -1425, -1425, -1425, -1425, -1425, -1425, -1425, 16318, -1425, -1425,
   -1425, -1425, -1425, -1425, -1425, -1425, -1425, -1425, -1425, -1425,
   -1425,   958, 16318, -1425, -1425, -1425,   923,  5627,   988,   303,
     961,   967,   969, 16318,  6247, -1425,   968,   234, -1425,   675,
   -1425,   131,  1084, 10587, 10587, -1425,   112, -1425, -1425,  1103,
   -1425, -1425, -1425, -1425, 10587,   909, -1425,  1097, 10587, 10587,
   16618, 10587, 10587, 10587, 10587, 16618, 10587,   904, 16318,   938,
     940, 10587, 10587,  5317,   892, -1425,    67, -1425,    54, 10587,
    6557, -1425, -1425, -1425, -1425, -1425,  1422,   850,   102,   104,
    1131,  7177,  3149,  7487,  3470,   863, -1425, -1425,   204,   863,
   -1425, 10587,  4090, -1425,  1017,   757,    78,   970,   971,   972,
   -1425, 10587, -1425, -1425,  6247, -1425, -1425, -1425, -1425, -1425,
   -1425, -1425,   855,   856, 16318,  1024, 10897, 10897, 10897, -1425,
   10897, 10897, -1425, 10897, -1425, 10897, -1425, -1425, -1425, -1425,
   -1425, -1425, -1425, 10897, 10897,  1073, 10897, 10897, 10897, 10897,
   10897, 10897, 10897, 10897, 10897, 10897, 10897, 10897,   920,  1050,
    1051,  1052, -1425, -1425, -1425, 11517,  6247, -1425, -1425, 11827,
   11827, 10587,   911, -1425, -1425,   911, -1425,  8107,   911,   976,
    1002,  8417, -1425, -1425, -1425, -1425,   369, -1425,   381, -1425,
   -1425, -1425, -1425, -1425, -1425,  1053,  1056,   529,  1129, -1425,
   -1425, 16906,   965,   694, 16318,  1004,  1005,   965,  1043,  1039,
    1035, -1425, -1425, -1425,   188,   924,  1074,   883, 16318,  1029,
   10587,  1055, 16318, 16318, -1425,  1042,   994, -1425,   997,   952,
     737, -1425,   998,  1000,   215, -1425,   419,   443,  1076, -1425,
      46, 16318,  1093,   478, -1425,  6247, -1425,   150, -1425, 16318,
    1092,  1145, 16318,  1043,  1094,   697, 16318, 10587,    78, -1425,
     334,  1006, -1425,  1007,  1009,  1010,   250, -1425,   621,  1008,
   -1425,   151,   162,  1045, -1425,  1014,  6247,  6247,   335, -1425,
     493,   505,   702, 10587,   263, -1425, -1425, 10587, 10587, -1425,
   10587, 10587, 10587, -1425, 10587, -1425, 10587, -1425, 16318,  1084,
   -1425,   458,   339,   374, -1425, -1425, -1425,   386, -1425,   479,
   -1425, -1425,  1046,  1047,  1048,  1049,  1054,   801,   785,   230,
     287,   274,    13,  1109,   517,   973,  -100,    27, -1425,  1062,
   -1425, -1425,  1016,   169,  5627,   432, 12757,   892, -1425, -1425,
   -1425,   978, -1425,   332,   825,  1174,   166, -1425, -1425,   117,
   -1425, -1425, -1425,   125, -1425,    65, -1425, -1425, -1425, -1425,
   -1425,  3780, -1425, -1425, -1425, 16318,  1057, 10587, 11207, -1425,
   -1425, -1425, -1425, -1425,  1076, 16318,    63,   966, -1425, -1425,
   -1425, -1425, -1425, 10897, -1425, -1425, -1425,    71, -1425,   588,
     588,    10,   632,   632,   632,   632,    36,    36, -1425, -1425,
   15430, 15430, 16318, 16318, -1425,  1028, -1425, -1425,   237, -1425,
   -1425, -1425, -1425,   544, -1425, -1425, -1425,   549,   548, -1425,
   -1425,   602,   606,   543, -1425,   529, -1425, -1425, -1425, -1425,
   -1425, -1425, -1425, -1425, -1425, -1425,   965, -1425,  1080, 15726,
    1071, 10587, -1425, -1425, -1425,  1120,  1043,  1043,   965, -1425,
     867,  1043,   696, 16318,   676,   678,  1186, -1425, -1425,   931,
      52, -1425, -1425, -1425, -1425, -1425, -1425, -1425, -1425, -1425,
   -1425, -1425,   188,    44,   991,   646, 10587, -1425, 16318,  1125,
     922,  1043, -1425, -1425, -1425, -1425,  1068, 16318, -1425, 16318,
   -1425, 16022,  1095, 15430,  1105, 10587,   -20,  1077,    59,  1120,
   15430,  1107,  1133,  1058,  1031,  1098,  1043,  1069,  1099,  1136,
    1043, 10587,    80, -1425, -1425, -1425,  1081, -1425, -1425, -1425,
   -1425,  1115, 10587, 10587,  1088, -1425,  1137,  1138,  6247, -1425,
    1059,  1060,  1086, 16318, -1425, 16318, -1425, 10587,  1102,  1061,
   10587, -1425,  1124,   192,   221,   223,  1212, -1425,   474, -1425,
     107,  1096, -1425, -1425,  1222, -1425,   756, 10587, 10587, 10587,
     759, 10587, 10587, 10587, 10587, 10587, 10587, 10587, 10587, 10587,
   16618,  1135, 10587, 10587, -1425,  7797,   129,  1011, -1425,   499,
   -1425, 10587,   136,   189,    65,  7487,  3470,  7487,  1175, -1425,
   -1425, 10587,   797,  1149, -1425, 16318,  1151,  1152, -1425,   682,
    1012, -1425, -1425,   731,    63, -1425, 10587,    71,   738,   589,
   -1425,   977,   293,  1001,  1003, -1425, -1425,   836, -1425,  1015,
     638,  1111,  1112, 15726,  1113,  1114,  1117,  1118,  1121, -1425,
      14, -1425, -1425, -1425, -1425, -1425, -1425, -1425, -1425, -1425,
   -1425,  1119, -1425, -1425, -1425, -1425,  9967, -1425, -1425, -1425,
   -1425, -1425, -1425, -1425, -1425, -1425,   700, -1425,  1249,   643,
   -1425, -1425, -1425, -1425, -1425, -1425, -1425, -1425, -1425,   995,
   -1425, -1425,  1250, -1425, -1425, -1425, -1425, -1425,   642,  1255,
   -1425,   784, -1425, -1425, -1425,   947,   360,   691,   954,   695,
     955,   957, -1425,  1114,  1117,  1118, -1425,    14,    14, 11517,
     999,   983, -1425,  1120,    44,  1063,  1104,  6247,  1106,  1110,
    1147,  1122,  1126, 16318, -1425,   382, -1425, 16318, -1425, 10587,
    1148, 10587,  1170, 10587,    86,  1153, -1425, -1425, -1425, 15430,
   -1425,  6247,  1043,  1191, -1425, -1425, -1425,  1043,  1191, -1425,
   10587,  1159,  6247, 16318, -1425, -1425, 10587, 10587,   449, -1425,
     343,   796, -1425, 14542,   819, -1425,   820, -1425,  1128, -1425,
   -1425,  6247,  1127,  1130, -1425, 10587, -1425, -1425, 10587,  1108,
    1137,  1203, -1425,  1172, -1425,   575, -1425, -1425,  1301, -1425,
   -1425,  6247, 16318, -1425,   599, -1425, -1425, -1425,  6247,  1132,
    1083,  1100, -1425, -1425, -1425,  1116,  1134, -1425, -1425, -1425,
    1223, -1425, -1425, -1425, -1425,  1089,   338, 16318,  1139, -1425,
    1164,  1178,  1179,  1177, -1425,   866,   675, 12757,  1011, -1425,
    5937, 12757, -1425, -1425,  1174,   348, -1425, -1425, -1425,  1149,
   -1425,  1043, -1425,   873, -1425,   688,  1225, -1425, 10587,   722,
    1043, -1425, 11207, 10587,  1181, -1425,  1218,  1219, 10587, 16318,
     847,  1262, -1425, -1425, -1425,   434,  1154, -1425,    71,  1123,
   -1425, -1425,   -53, -1425,   641,   340,  1144,    71,   641, 10897,
   -1425,   307, -1425, -1425, -1425, -1425, -1425, -1425,    71,  1194,
    1065,   924,   340, -1425, -1425,  1066,  1264, -1425, -1425, -1425,
   13053,  1157,  1158,  1161,  1166,  1168,  1173,  1176, -1425, -1425,
   -1425, -1425, -1425, -1425, -1425, -1425, -1425, -1425, -1425, -1425,
   -1425,  1327,    17,  1329,    17,  1101,  1256, -1425, -1425,  1195,
   16318,  1140, -1425, -1425, 11517, -1425,  1184, -1425, -1425, -1425,
   -1425, -1425, -1425, 10587,  1226, -1425, -1425, 10587, -1425,   662,
   -1425, 10587,  1229, 10587, -1425,  1243,  1244,  1282,  1043,  1191,
   -1425, 10587,  1196, -1425, -1425, -1425,  1139, -1425,   343, 10587,
    6247,  1139,   343, -1425, 16318,   -26, -1425, 14838,    29, -1425,
   15134,  1139, -1425, -1425,  1197, -1425, -1425, -1425, -1425, 10587,
     834,  1212, 16318,   626, -1425,  1198,  1212, 16318, -1425,  1200,
   -1425, 10587, 10587, 10587, 10587,  1072, 10587, 10587, -1425, 10587,
   10587, 10587, 10587,  7797,   586, -1425, -1425, -1425, -1425, -1425,
    1225, -1425, -1425, -1425, 10587,  1242, -1425, -1425, -1425, 10587,
   16318, 16318, -1425,   739, -1425, 10587, -1425, -1425, -1425,  1205,
   10897, -1425, -1425, -1425, -1425, -1425,    62, 10897, 10897,   584,
   -1425,  1001, -1425,   407, -1425,  1003,    71,  1233, -1425, -1425,
    1142, -1425, -1425, -1425, -1425,  1300,  1209, -1425,   550, -1425,
   -1425, -1425, -1425, -1425, -1425, -1425, -1425,   520,   520, -1425,
      17, -1425, -1425,   555, -1425,  1370,   340,  1304,  1215, 11517,
     -25,  1141,  1234, -1425, -1425, 10587, -1425, 10587,  1257, -1425,
   10587, -1425,  7797, 10587,  1043, -1425, -1425, -1425, 10587, 10587,
   -1425, -1425, -1425,  7797,  7797,  1308, 15430,  6247, 16318,   -26,
   16318, 10587, 16318,   -26,  7797, -1425,    31,   298,  1212, 16318,
   -1425,  1210,  1212, -1425, -1425, -1425, -1425, -1425, 10587, -1425,
   -1425, -1425,   231,   239,   249, 10587, -1425, -1425,  1151, 10587,
   -1425, -1425,  1354, -1425, -1425, -1425,   632, 10897, 10897,    10,
     724, -1425, -1425, -1425, -1425, -1425, -1425, 10587, -1425, 15430,
   -1425, 15430,  1314, -1425, -1425, -1425,  1383, -1425, -1425, -1425,
    1150,  1309, -1425, -1425,  1310, -1425,   783, 16318,  1297,  1199,
   16318, 11517, -1425, -1425, 10587, -1425, -1425, -1425,  1191, -1425,
   -1425, 15430, -1425, -1425, -1425,  1324, 10587,  1139, -1425,  1325,
    7797, -1425, 16318,   448,   540, -1425,  1228,  1212, -1425,  1230,
   -1425,  1231,  1137,  1138,   459, -1425,  1151,  1307,    10,    10,
   10897,   463, -1425, -1425, 15430, -1425, -1425,  1304, 11517, -1425,
    1225,  1156, 16318,  1312,  1207,  1310, -1425,   -26, 15430,  6247,
   15430,  1139, -1425, -1425,  1340,   620, -1425, -1425, -1425, -1425,
    1240,   578, -1425, -1425, -1425,  1238, -1425,  7797,   835, -1425,
   -1425, 16318,    10, -1425, -1425, -1425, -1425, -1425, 10587,  1160,
   16318,  1318,  6247,   -26, -1425, -1425,   -26, 10587, -1425, -1425,
   -1425, -1425, -1425, -1425, -1425,  1319, -1425, -1425,  1146,  1162,
   16318, -1425, -1425, 10587,  7797, 16318,  1169, 10587,  1183,  1139,
   11517, -1425, 11517,  7797, -1425,  1241,  1155, 16318,  1216,  1326,
   16318,  1187, 11517, -1425
  };

  const short
  xquery_parser::yydefact_[] =
  {
     165,   469,   470,   805,   471,   168,   168,   168,     0,   828,
    1028,   120,   122,   636,   921,   930,   870,   833,   831,   807,
     922,   925,   877,   837,   808,   810,     0,   931,   812,   928,
     899,   879,   853,   809,   874,   875,   926,   923,   873,   814,
     929,   815,   816,   970,   982,   969,   871,   890,   884,   817,
     872,   819,   818,   971,   908,   909,   876,   850,   992,   993,
     994,   995,   996,   997,   998,   999,  1000,  1001,  1002,  1016,
    1023,   898,   894,   885,   864,   806,     0,   893,   901,   910,
    1017,   889,   495,   865,   866,   924,  1018,  1024,   886,   903,
       0,   501,   458,   497,   896,   830,   887,   888,   917,   891,
     907,   916,  1022,  1025,   836,   878,   919,   496,   906,   912,
     811,     0,     0,     0,     0,   405,   904,   915,   920,   918,
     897,   883,   972,   881,   882,  1019,     0,   404,     0,  1020,
    1026,   913,   867,   892,  1021,   212,   436,     0,   468,   914,
     895,   902,   911,   905,   973,   858,   863,   862,   861,   860,
     859,   820,   880,     0,   829,   927,   851,   963,   962,   964,
     835,   834,   854,   961,   813,   958,   966,   960,   959,   857,
     900,   956,   968,   967,   955,   855,   965,   977,   978,   975,
     976,   974,   821,   979,   980,   981,   947,   946,   933,   849,
     842,   940,   936,   852,   848,   948,   869,   838,   839,   832,
     841,   945,   944,   941,   937,   953,   954,   952,   943,   939,
     932,   840,   951,   950,   844,   846,   845,   938,   942,   934,
     847,   935,   843,   949,  1003,  1004,  1005,  1006,  1007,  1008,
     984,   985,   983,   989,   990,   991,   986,   987,   988,   856,
    1009,  1010,  1011,  1012,  1013,  1014,  1015,     0,     0,   822,
     957,  1027,   824,   823,   825,   826,   165,   165,     0,     2,
     165,     9,    11,    23,     0,    28,    31,    36,    37,    38,
      39,    40,    41,    42,    32,    58,    59,    33,    34,     0,
      76,    79,    80,    35,    81,    82,     0,   118,    83,    84,
      85,    86,    18,   162,   163,   164,   171,   174,   488,   175,
       0,   176,   177,   178,   179,   180,   181,   182,   185,   186,
       0,   213,   221,   216,   249,   255,     0,   247,     0,   248,
     223,   217,   184,   218,   183,   219,   222,   356,   358,   360,
     370,   372,   376,   378,   381,   386,   389,   392,   394,   396,
     398,     0,   402,   409,   408,   410,     0,   427,   411,   432,
     435,   437,   440,   442,     0,   447,   444,     0,   455,   465,
     467,   441,   472,   479,   493,   480,   481,   482,   485,   486,
     483,   487,   510,   511,   484,   514,   516,   517,   518,   515,
     563,   564,   565,   566,   567,   568,   569,   464,   608,     0,
     599,   607,   606,   605,   604,   601,   603,   600,   602,   494,
      43,   224,   225,   227,   226,   228,   220,   490,   491,   492,
     489,   230,   233,   229,   231,   232,   466,   804,   827,   921,
     930,   928,   829,     0,   166,   167,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   126,   128,   129,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   278,   279,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   460,   463,
     450,     0,     0,   807,   837,   810,   868,   931,   812,   879,
     814,   890,   817,   819,   818,   908,  1016,  1023,   806,  1017,
     866,  1018,  1024,   891,  1022,  1025,   915,   972,  1019,  1020,
    1026,  1021,   973,   961,   958,   960,   959,   855,   977,   978,
     975,   821,   979,   957,   456,   804,   448,     0,   193,   449,
     452,   807,   808,   810,   809,   814,   815,   816,   817,   806,
     813,   498,     0,   454,   453,   188,     0,     0,   213,     0,
     812,   819,   818,     0,   168,   784,   979,     0,   221,     0,
     499,     0,   530,     0,     0,   459,     0,   430,   431,     0,
     462,   461,   451,   434,     0,     0,   559,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   786,     0,   788,     0,     0,
     165,     3,     4,     1,    10,    12,     0,     0,     0,     0,
       6,   165,     0,   165,     0,     0,   119,   172,     0,     0,
     191,     0,     0,   200,     0,     0,     0,     0,     0,     0,
     236,     0,   205,   234,     0,   254,   250,   256,   251,   253,
     252,   259,     0,     0,     0,     0,     0,     0,     0,   421,
       0,     0,   419,     0,   365,     0,   420,   413,   418,   417,
     416,   415,   414,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   407,   406,   403,     0,   168,   428,   433,     0,
       0,     0,   443,   476,   446,   445,   457,     0,   473,     0,
       0,     0,   571,   573,   577,   579,     0,   124,     0,   803,
      47,    44,    45,    48,    49,     0,     0,     0,     0,    50,
      51,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   643,   644,   645,     0,   103,   142,     0,     0,   112,
       0,     0,     0,     0,   127,     0,     0,   616,     0,     0,
       0,   611,     0,     0,     0,   626,     0,     0,   258,   261,
       0,     0,   134,     0,   138,   168,   513,     0,    60,     0,
      69,     0,     0,     0,    61,     0,     0,     0,     0,   614,
       0,     0,   617,     0,     0,     0,     0,   327,     0,     0,
     615,     0,     0,     0,   634,     0,     0,     0,     0,   620,
       0,     0,   195,     0,     0,   189,   187,     0,     0,   790,
       0,     0,     0,   498,     0,   785,     0,   500,   531,   530,
     527,     0,     0,     0,   561,   560,   429,     0,   558,     0,
     656,   657,     0,     0,     0,     0,     0,   797,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   662,     0,
     203,   204,     0,     0,     0,     0,     0,   208,   209,   787,
     789,     0,     5,    24,     0,    25,     0,     7,    29,     0,
      15,     8,    30,     0,    19,   921,    77,    16,    78,    20,
     194,     0,   192,   214,   215,     0,     0,     0,     0,   296,
     206,   235,   237,   238,   257,     0,     0,   357,   359,   363,
     369,   368,   367,     0,   364,   361,   362,     0,   373,   380,
     379,   377,   383,   384,   385,   382,   387,   388,   391,   390,
       0,     0,     0,     0,   412,     0,   438,   439,     0,   477,
     506,   474,   508,     0,   609,   512,   504,     0,     0,   121,
     123,     0,     0,     0,   102,     0,    92,    94,    95,    96,
      97,    99,   100,   101,    93,    98,    88,    89,     0,     0,
     108,     0,   104,   106,   107,   114,     0,     0,    87,    46,
       0,     0,     0,     0,     0,     0,     0,   734,   739,     0,
       0,   735,   769,   722,   724,   725,   726,   728,   730,   729,
     727,   731,     0,     0,     0,     0,     0,   111,     0,   144,
       0,     0,   576,   570,   612,   613,     0,     0,   630,     0,
     627,     0,     0,     0,     0,     0,     0,     0,     0,   140,
       0,     0,   135,     0,     0,     0,     0,     0,     0,    62,
       0,     0,     0,   277,   283,   280,     0,   619,   618,   625,
     633,     0,     0,     0,     0,   575,     0,     0,     0,   422,
       0,     0,     0,     0,   623,     0,   621,     0,   196,     0,
       0,   791,     0,     0,     0,     0,   530,   528,     0,   519,
       0,     0,   502,   503,     0,    13,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   201,     0,     0,     0,   670,     0,
     210,     0,     0,     0,     0,   165,     0,   165,     0,   173,
     246,     0,   311,   307,   309,     0,   304,   297,   298,     0,
       0,   239,   240,     0,   245,   366,     0,     0,     0,     0,
     700,   374,   673,   677,   679,   681,   683,   686,   693,   694,
     702,   931,   811,     0,   820,   824,   823,   825,   826,   393,
     582,   588,   589,   592,   637,   638,   593,   594,   597,   395,
     397,   400,   598,   399,   426,   478,     0,   475,   505,   125,
      56,    57,    54,    55,   131,   130,     0,    90,     0,     0,
     109,   110,   115,    74,    75,    52,    53,    73,   740,     0,
     743,   770,     0,   733,   732,   737,   736,   768,     0,     0,
     745,     0,   741,   744,   723,     0,     0,     0,     0,     0,
       0,     0,   610,     0,     0,     0,   143,   146,   148,     0,
       0,     0,   113,   116,     0,     0,     0,   168,     0,     0,
     634,     0,     0,     0,   263,     0,   581,     0,   264,     0,
       0,     0,     0,     0,     0,     0,   141,   136,   139,     0,
     190,     0,     0,    71,    65,    68,    67,     0,    63,   281,
       0,     0,   168,     0,   325,   329,     0,     0,     0,   332,
       0,     0,   338,     0,     0,   345,     0,   349,     0,   424,
     423,   168,     0,     0,   197,     0,   199,   326,     0,     0,
       0,     0,   531,     0,   521,     0,   554,   551,     0,   555,
     556,     0,     0,   550,     0,   525,   553,   552,     0,     0,
       0,     0,   649,   650,   646,     0,     0,   654,   655,   651,
     793,   794,   660,   798,   658,     0,     0,     0,     0,   664,
     812,   819,   818,   979,   202,     0,     0,     0,   665,   666,
       0,     0,   211,   796,    26,     0,    17,    21,    22,   308,
     320,     0,   321,     0,   312,   313,   314,   315,     0,   498,
       0,   305,     0,     0,     0,   241,     0,     0,     0,     0,
     290,     0,   287,   292,   244,     0,     0,   684,   697,     0,
     371,   375,     0,   716,     0,     0,     0,     0,     0,     0,
     672,   674,   675,   711,   712,   713,   715,   714,     0,     0,
     688,   687,     0,   691,   695,   709,   707,   706,   699,   703,
       0,     0,     0,     0,     0,     0,     0,     0,   585,   587,
     586,   583,   401,   507,   509,   133,   132,    91,   105,   757,
     738,     0,   762,     0,   762,   751,   746,   147,   149,     0,
       0,     0,   117,   145,     0,    27,     0,   631,   632,   635,
     628,   629,   262,     0,     0,   276,   268,     0,   272,     0,
     266,     0,     0,     0,   137,     0,     0,    70,     0,    64,
     282,     0,     0,   328,   330,   335,     0,   333,     0,     0,
       0,     0,     0,   339,     0,     0,   353,     0,     0,   346,
       0,     0,   350,   425,     0,   624,   622,   198,   792,     0,
       0,   530,     0,     0,   562,     0,   530,     0,   526,     0,
      14,     0,     0,     0,     0,     0,     0,     0,   663,     0,
       0,     0,     0,     0,     0,   667,   671,   324,   322,   323,
     316,   317,   318,   310,     0,     0,   306,   299,   285,     0,
       0,     0,   243,   288,   291,     0,   701,   692,   698,     0,
       0,   771,   772,   782,   781,   780,     0,     0,     0,     0,
     773,   678,   779,     0,   676,   680,     0,     0,   685,   689,
       0,   710,   705,   708,   704,     0,     0,   595,     0,   590,
     642,   584,   802,   801,   800,   591,   758,     0,     0,   756,
     763,   764,   760,     0,   755,     0,   753,     0,     0,     0,
       0,     0,     0,   572,   265,     0,   274,     0,     0,   270,
       0,   273,     0,     0,     0,    66,   284,   578,     0,     0,
     336,   340,   334,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   574,     0,     0,   530,     0,
     557,     0,   530,   580,   647,   648,   652,   653,     0,   659,
     799,   661,     0,     0,     0,     0,   668,   319,   300,     0,
     286,   294,   295,   289,   242,   696,   783,     0,     0,   775,
       0,   721,   720,   719,   718,   717,   682,     0,   774,     0,
     639,     0,     0,   767,   766,   765,     0,   759,   752,   750,
       0,   747,   748,   742,   150,   152,   154,     0,     0,     0,
       0,     0,   269,   267,     0,   275,   207,   355,    72,   331,
     337,     0,   354,   351,   347,     0,     0,     0,   341,     0,
       0,   343,     0,   540,   534,   529,     0,   530,   520,     0,
     795,     0,     0,     0,     0,   303,   301,     0,   777,   776,
       0,     0,   640,   596,     0,   761,   754,     0,     0,   156,
     155,     0,     0,     0,     0,   151,   271,     0,     0,     0,
       0,     0,   548,   542,     0,   541,   543,   549,   546,   536,
       0,   535,   537,   547,   522,     0,   523,     0,     0,   669,
     302,     0,   778,   690,   641,   749,   153,   157,     0,     0,
       0,     0,     0,     0,   344,   342,     0,     0,   533,   544,
     545,   532,   538,   539,   524,     0,   293,   158,     0,     0,
       0,   352,   348,     0,     0,     0,     0,     0,     0,     0,
       0,   160,     0,     0,   159,     0,     0,     0,     0,     0,
       0,     0,     0,   161
  };

  const short
  xquery_parser::yypgoto_[] =
  {
   -1425, -1425,  -188,  -176, -1425,  1204,  1214, -1425,  1180,  -552,
   -1425, -1425, -1425, -1425, -1425, -1425, -1425, -1425, -1425, -1425,
    -992, -1425, -1425, -1425, -1425,  -219,  -578, -1425,   707,   -40,
   -1425, -1425, -1425, -1425, -1425,   257,   477, -1425, -1425,    11,
    -173,  1023, -1425,  1019, -1425, -1425,  -641, -1425,   424, -1425,
     224, -1425,  -254,  -282, -1425,  -563, -1425,     4,    12,    28,
    -223,  -181, -1425,  -880, -1425, -1425,  -240, -1425, -1425, -1425,
   -1425, -1425, -1425, -1425, -1425, -1425, -1425,   611, -1425,   -59,
    1357,     0, -1425, -1425,  -315, -1425, -1425,   347, -1425, -1425,
    -301, -1425,    16, -1425, -1425,   828,  -934,  -742,  -718, -1425,
   -1425,   704, -1425, -1425,   -70,   105, -1425, -1425, -1425,   122,
   -1068, -1425,   363,   127, -1425, -1425,   133, -1299, -1425,   948,
     226, -1425, -1425,   218, -1014, -1425, -1425,   220, -1425, -1425,
   -1260, -1255, -1425,   217, -1422, -1425, -1425,   848,   851, -1425,
    -527,   829, -1425, -1425,  -665,   328,  -643,   323,   329, -1425,
   -1425, -1425,   576, -1425, -1425,  1182, -1425, -1425, -1425, -1425,
   -1425,  -886,  -319,  -682, -1425,   -66, -1425, -1425, -1425, -1425,
   -1425, -1425, -1425,   -13,  -819, -1425,  -539,  -195,   275, -1425,
    -427, -1425, -1425, -1425, -1425, -1425, -1425, -1425,   799, -1425,
   -1425, -1425, -1425, -1001, -1425,   206, -1425,   684,  -797, -1425,
   -1425, -1425, -1425, -1425,  -266,  -259, -1212,  -919, -1425, -1425,
   -1425, -1425, -1425, -1425, -1425, -1425, -1425, -1425, -1425,  -755,
    -832,  -210,  -825, -1425, -1425, -1425,  -759, -1425, -1425, -1425,
   -1425, -1425, -1425, -1425, -1425, -1425,  1067,  1070,  -192,   498,
     330, -1425, -1425, -1425, -1425, -1425, -1425, -1425, -1425, -1425,
   -1425, -1425,   183, -1425, -1425,   171, -1425,   174, -1072, -1425,
   -1425, -1425,   128,   114,   -52,   389, -1425, -1425, -1425, -1425,
   -1425, -1425, -1425, -1425, -1425, -1425, -1425, -1425, -1425,   130,
   -1425, -1425, -1425,   -45,   383,   530, -1425, -1425, -1425, -1425,
   -1425,   326, -1425, -1425, -1424, -1425, -1425, -1425,  -577, -1425,
      94, -1425,   -61, -1425, -1425, -1425, -1425, -1322, -1425,   142,
   -1425, -1425, -1425, -1425, -1425,   949, -1425, -1425, -1425, -1425,
   -1425,  -929,  -383,   914,   -64, -1425
  };

  const short
  xquery_parser::yydefgoto_[] =
  {
       0,   258,   632,   259,   260,   261,   262,   263,   264,   265,
     266,   267,   268,   269,   270,   271,   272,   273,   274,   275,
    1263,   783,   276,   277,   278,   279,   280,   281,   966,   967,
     968,   282,   283,   284,   972,   973,   974,   285,   450,   286,
     287,   716,   288,   452,   453,   454,   464,   773,   774,   289,
    1226,   290,  1704,  1705,   291,   292,   293,   556,   294,   295,
     296,   297,   298,   776,   299,   300,   538,   301,   302,   303,
     304,   305,   306,   642,   307,   308,   867,   868,   309,   310,
     311,   568,   313,   643,  1641,   655,  1133,  1134,   314,   645,
     315,   647,   569,   317,   318,   768,   769,  1380,   471,   319,
     472,   473,   906,  1381,  1382,  1383,   648,   649,  1127,  1128,
    1371,   650,  1123,  1124,  1364,  1365,  1366,  1367,   320,   796,
     797,   321,  1278,  1279,  1488,   322,  1281,  1282,   323,   324,
    1284,  1285,  1286,  1287,  1495,   325,   326,   327,   328,   913,
     329,   330,  1390,   331,   332,   333,   334,   335,   336,   337,
     338,   339,  1170,   340,   341,   342,   343,   673,   674,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   708,   703,   362,
     363,   364,   365,   366,   367,   368,   369,   370,   943,   371,
     372,   373,   374,   375,   376,  1314,   829,   830,   831,  1735,
    1780,  1781,  1774,  1775,  1782,  1776,  1315,  1316,   377,   378,
    1317,   379,   380,   381,   382,   383,   384,   385,   386,  1028,
    1496,  1431,  1160,  1588,  1161,  1171,   387,   388,   389,   390,
     391,   392,   393,   394,   395,   396,   397,   398,   803,  1239,
     399,  1163,  1164,  1165,  1166,   400,   401,   402,   403,   404,
     405,   857,   858,   406,  1348,  1349,  1666,  1109,  1141,  1400,
    1401,  1142,  1143,  1144,  1145,  1146,  1410,  1578,  1579,  1147,
    1413,  1148,  1559,  1149,  1150,  1418,  1419,  1584,  1582,  1402,
    1403,  1404,  1405,  1684,   745,   993,   994,   995,   996,   997,
     998,  1212,  1608,  1701,  1213,  1606,  1699,   999,  1444,  1603,
    1599,  1600,  1601,  1000,  1001,  1406,  1414,  1569,  1407,  1565,
    1391,   407,   408,   409,   410,   559,   411,   412,   413,   414,
     415,  1167,  1264,   416,   417,   418
  };

  const short
  xquery_parser::yytable_[] =
  {
     312,   644,   604,   717,   717,   312,   312,   312,   312,   423,
     426,   427,   535,   934,   921,   646,   316,   424,   424,   424,
     428,   316,   316,   316,   316,   451,   535,   697,  1026,  1499,
    1042,  1139,  1078,   425,   425,   425,   429,   922,   923,   924,
     925,  1510,  1280,  1054,   619,   886,   888,  1108,  1268,   720,
     535,   847,  1027,   567,   571,   620,   854,  1068,   880,   884,
     887,   889,   535,   534,   535,  1386,  1541,  1542,   620,   878,
     882,   583,   627,  1185,  1228,  1639,   622,   620,  1643,  1313,
     611,   612,  1022,  1215,   780,   784,   953,  1244,  1216,   759,
    1580,   624,    11,    12,  1707,  1252,   620,  1217,  1159,  1169,
     771,    13,  1518,   620,  1218,   620,  1219,   760,   585,    11,
      12,   312,   436,   626,  1677,  1220,  1306,  1307,   620,  1130,
     834,   818,   437,   424,   594,  1099,   620,   316,  1249,   436,
     620,   909,   620,   910,   911,  1101,   912,   620,   914,   557,
     439,  1023,  1221,  1024,  1189,   440,   915,   916,  1640,  1209,
    1732,   620,   620,   588,  1597,   589,  1024,   439,   702,  1428,
     640,   705,   631,   620,  1561,   719,  1210,   881,   772,   677,
     620,  1162,  1162,   686,   591,   631,   592,   441,   442,  1118,
    -169,   620,    82,  1702,  1636,   759,   678,   687,   606,   608,
    1429,  1246,  1678,   620,  1025,   751,    93,   469,  1257,  1114,
    1708,  1562,   627,   760,  -170,   620,   627,  1253,  1430,  1211,
    1162,  1308,   719,   595,  1100,   640,   620,   640,  1598,   455,
    1192,   107,   620,  1270,   620,  1136,   432,   433,  1137,   434,
     435,  1309,   620,  1310,  1471,   620,   628,   631,   620,   631,
     620,  1667,   590,   982,  1227,   983,  1095,   438,   128,   781,
     620,  1139,  1139,   470,   826,  1499,   312,   312,   631,   631,
     312,  1311,  1102,   593,  1162,   114,   621,  1492,   646,  1222,
     631,  1162,   316,   316,  1256,  1479,   316,   631,   626,  1303,
    1131,   126,  1709,   698,  1700,   623,  1254,   835,   631,   984,
     535,   586,   873,   535,   875,   312,   782,   886,   888,  1767,
     631,  1518,  1312,  1355,  1313,  1228,  1436,  1115,  1250,  1462,
    1255,   316,   631,  1313,  1186,  1117,  1558,  1132,   827,   470,
     153,   878,   882,   631,  1271,  1086,  1354,   761,  1422,   631,
    1138,   631,   470,  1795,   627,   620,   620,  1034,  1056,   631,
     620,   704,   631,   779,   706,   631,  1803,   631,  1806,  1057,
     444,   470,   447,   470,   430,   979,  1105,   631,  1051,   470,
     448,  1223,  1224,  1225,  -169,   616,   628,   870,   449,   752,
     628,  1051,   727,   535,  1374,   620,   535,   753,   869,  1299,
     535,   617,  1090,   535,   535,   890,  1135,   620,  -170,   892,
    1392,   535,  1733,   735,  1162,   736,   817,   535,   535,  1016,
    1039,   764,   879,   883,  1392,   535,   777,  1759,  1300,   445,
    1301,   818,   535,   535,   535,   535,   790,   431,  1741,   825,
     535,   900,  1175,   801,   802,   312,  1742,  1474,   535,   312,
     874,   876,   808,   620,   872,   620,  1743,  1566,  1052,   985,
     891,   316,   631,   631,  1489,   316,   458,   631,  1087,  1088,
     446,  1070,   986,  1772,   987,   738,   312,  1306,   312,   629,
     620,  1797,  1393,  1089,   620,   988,   989,   990,   756,   991,
     758,   992,   316,   535,   316,  1227,  1393,   948,  1452,  1024,
     312,  1734,   631,  1788,   429,   312,   429,   819,   535,   948,
    1140,   479,   799,   701,   631,   707,   316,   805,   628,   535,
    1162,   316,  1139,  1464,   484,  1091,  1092,  1094,   429,    11,
      12,  1139,  1313,   429,   832,   833,   739,   459,  1046,  1062,
    1093,  1179,  1139,  1081,  1162,   837,   848,  1017,  1108,   640,
    1463,   848,  1536,  1499,   535,   651,  1472,   886,   888,   886,
     631,   630,   631,   863,   865,  1394,  1395,  1449,  1396,  1306,
    1277,  1019,  1356,  1357,  1358,  1397,   949,   312,  1082,  1394,
    1395,  1336,  1396,  1486,   312,  1398,   677,   631,   950,  1778,
    1083,   631,  1309,   316,  1310,  1527,   424,  1399,    13,  1398,
     316,   485,  1773,   678,  1306,  1307,  1031,  1306,  1587,  1079,
     535,  1399,   425,  1193,  1194,   625,  1567,  1568,  1197,   652,
    1745,  1063,  1311,   312,   653,  1304,  1018,  1778,  1306,  1307,
     312,  1084,  1097,  1065,  1545,   596,  1107,  1441,  1556,   316,
     597,   312,   312,   312,   312,  1772,   316,   451,  1236,  1306,
    1020,   864,  1718,   936,   937,  1306,  1307,   316,   316,   316,
     316,   627,   938,  1789,   312,   749,   725,  1793,   486,   487,
     750,   654,  1176,  1350,  1112,  1080,   488,  1176,  1691,    82,
     316,  1162,   726,  1696,  1309,  1032,  1310,   955,  1119,  1085,
     535,  1305,   701,    93,   707,  1779,  1681,  1443,  1790,  1308,
    1064,  1184,   696,  1682,   535,   735,   463,   736,   535,   535,
    1139,  1116,  1066,  1683,  1311,   876,   312,   775,   107,  1309,
     935,  1310,  1309,  1308,  1310,   128,   680,   535,   424,  1351,
    1140,  1140,   316,  1812,  1647,   535,  1023,   634,   535,  1651,
     737,   635,   535,  1309,   425,  1310,   636,  1618,  1492,  1311,
    1308,  1177,  1311,   114,  1573,   681,  1178,  1692,  1162,  1441,
    1665,  1162,  1697,  1388,  1309,   637,  1310,   738,   682,   126,
    1309,  1180,  1310,  1311,  1809,  1182,  1442,   114,   462,  1024,
    -260,  1073,  1074,  1075,   535,   865,   128,  1181,    13,  1053,
    1512,  1183,  1612,   126,  1311,   312,   886,  1023,   729,  1033,
    1311,   970,   683,   489,   114,  1376,   971,   424,   153,   969,
    1377,   316,  1023,  1376,  1517,  1361,  1351,  1023,  1377,  1443,
     126,   465,   535,   425,  1722,  1106,   312,   312,   739,  1378,
    1617,   730,   153,   490,  1777,  1783,  1363,  1023,  1060,  1061,
     697,  1649,   316,   316,  -260,  1238,   881,  1241,  1024,  -260,
    1373,   535,   970,   684,   429,   429,  1024,   971,  1435,   153,
    1041,   535,   685,   638,   491,  1067,   639,   492,   474,    82,
    1379,  1736,   477,  1681,   775,  1739,  1777,  1752,  1113,  1753,
    1682,   640,  1783,    93,   312,  1544,   535,   535,   535,   535,
    1683,  1292,   677,  1293,   617,   466,  -260,  1162,  1023,  1477,
     316,  1209,  1344,   677,   467,  1415,  1416,   641,   107,   678,
    1361,   312,  1136,  1360,   478,  1137,   451,  1277,  1210,   721,
     678,  1376,  1679,  1680,  1361,   535,  1377,   316,  1417,  1563,
    1491,   468,   679,   722,  1362,   128,  1564,  1676,   891,   535,
    1497,  1500,  1794,  1750,   876,  1363,  1187,  1706,  1352,   723,
    1162,   710,  1162,  1498,  1501,  1497,  1497,   482,  1187,  1203,
    1785,  1205,   711,   724,   535,  1230,   699,   700,  1646,  1815,
    1204,  1760,  1206,   535,  1231,   535,   701,   535,   707,   535,
     456,  1140,  1162,   457,   460,  1490,   535,   461,   475,   480,
    1140,   476,   481,  1198,  1199,  1200,   536,   939,  1537,  1201,
     939,  1140,   537,   939,   741,   742,   743,  1546,  1320,   483,
    1321,  1325,   539,  1326,   540,  1162,  1195,  1196,   552,   535,
     553,   535,  1538,  1539,   551,   919,   920,   926,   927,  1162,
     554,  1162,  1748,  1749,   573,   928,   929,  1447,  1448,   574,
    1693,  1694,   575,  1529,   580,   581,   582,   587,   572,  1706,
     584,  1533,  -610,   599,   598,   600,   848,   601,  1475,   602,
     576,   609,   579,   603,   613,   646,   956,   957,   958,   959,
     960,   961,   962,   963,   964,   965,   633,   656,   312,   657,
     676,   535,   675,   688,   689,   690,   701,   691,   712,   709,
    1288,   695,   713,   719,   316,   714,  1706,  1385,   715,   731,
     733,   734,   744,   658,   757,  1792,   429,   775,   778,   535,
     748,   659,   660,   786,   661,  1625,   787,   788,   806,   807,
     789,   813,   662,   800,   457,  1345,   663,   815,   664,   461,
     817,   476,   481,   665,   492,   312,   312,   312,   820,   828,
     836,  1346,   824,   838,   821,   451,   822,   839,   860,   666,
     861,   316,   316,   316,   877,   856,   895,   866,   896,   902,
     903,   897,   898,   905,   917,   931,   932,   933,  1834,  1140,
    1835,   930,   945,   667,   668,   669,   670,   671,   672,   954,
    1843,   951,   604,   944,   952,   976,   977,   980,   981,  1003,
    1002,  1628,  1006,  1629,  1008,  1011,  1633,  1634,  1012,   535,
    1637,  1013,  1004,   535,  1021,  1014,  1644,  1015,  1030,  1036,
    1037,  1040,  1055,  1047,  1048,   535,  1049,  1050,  1059,  1058,
     456,   460,   475,   480,  1096,  1103,  1104,  1114,   491,   535,
    1098,  1111,  1174,  1188,   971,  1023,  1207,  1208,  1229,   535,
    1234,  1235,  1237,  1245,  1247,  1251,   771,  1121,  1259,  1261,
    1265,  1262,  1266,  1267,  1273,  1272,  1276,   312,  1277,  1283,
    1291,  1456,  1260,  1289,  1290,  1295,  1298,  1302,   535,   424,
    1318,  1296,  1319,   316,  1337,   737,  1347,  1368,  1370,  1408,
    1372,   312,  1389,  1409,  1432,   425,  1375,  1631,  1420,  1421,
    1423,  1424,   312,   535,  1425,  1426,  1482,   316,  1427,    13,
    1440,  1439,  1412,   535,   424,  1445,  1450,   535,   316,  1451,
    1454,   312,  1459,  1457,  1455,  1504,  1467,  1458,  1469,  1478,
     425,  1473,  1481,   424,  1497,  1511,  1509,   316,  1514,  1460,
     900,   312,  1503,  1461,  1505,   535,  1521,  1506,   312,   425,
    1525,  1530,  1520,  1515,  1726,   640,  1526,   316,  1730,  1549,
    1519,  1350,  1361,  1522,   316,  1531,  1532,  1550,  1551,   429,
    1555,  1557,  1576,  1583,  1589,  1590,   429,  1577,  1591,  1523,
    1345,   728,  1581,  1592,   732,  1593,   535,  1596,   740,  1602,
    1594,   746,   747,  1595,  1607,  1609,  1346,  1524,  1613,   755,
    1622,  1623,  1605,  1560,  1615,   767,   770,  1620,   864,  1624,
    1627,  1645,  1650,   785,  1653,  1669,   535,  1687,  1658,  1675,
     793,   794,   795,   798,  1570,  1689,  1690,  1611,   804,  1716,
    1698,  1209,  1703,  1721,  1711,  1714,   811,  1738,  1747,  1754,
    1720,  1631,  1769,  1755,  1723,  1756,  1762,  1757,  1758,  1768,
    1770,  1731,  1688,  1811,  1763,  1784,  1791,  1786,  1710,  1787,
     535,  1800,  1801,   535,  1808,  1814,   535,  1820,  1825,  1826,
    1836,  1839,  1837,   618,   978,  1840,  1438,  1190,   535,    11,
      12,   812,  1802,   535,   718,  1258,  1807,  1765,  1453,   432,
     433,  1798,   434,   435,   614,  1818,   798,  1827,   558,   436,
     754,  1662,  1663,  1664,   615,  1830,  1796,   823,  1110,   437,
     438,  1384,   904,  1673,  1359,  1554,   535,   535,  1823,  1832,
     312,  1824,  1045,  1842,  1547,  1543,  1487,   439,  1540,  1483,
     814,  1493,   440,  1502,   907,   918,   316,  1723,   908,  1173,
     947,  1513,   859,  1077,  1833,  1813,  1810,  1242,  1437,  1535,
    1528,  1534,  1575,   694,  1686,  1571,   762,  1387,  1685,   763,
    1411,  1574,  1214,  1345,   441,   442,   443,  1446,  1604,  1695,
    1572,     0,   852,     0,     0,     0,  1804,     0,     0,  1346,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1475,     0,     0,     0,   770,     0,
       0,     0,   535,     0,   535,     0,   535,     0,   535,  1821,
       0,     0,     0,   444,     0,   535,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1821,     0,     0,     0,     0,  1744,     0,     0,     0,
    1804,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1345,     0,     0,   535,     0,   535,  1751,     0,
       0,     0,     0,  1345,  1345,     0,     0,   312,  1346,     0,
       0,     0,   445,   535,  1345,     0,   535,     0,   975,  1346,
    1346,     0,     0,   316,     0,     0,     0,   535,     0,     0,
    1346,     0,  1005,     0,     0,     0,  1009,  1010,   535,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   446,     0,  1029,     0,     0,     0,     0,
     535,     0,     0,  1035,     0,     0,  1038,     0,   535,     0,
    1043,     0,     0,     0,   535,     0,   535,     0,     0,   447,
       0,     0,     0,     0,     0,     0,     0,   448,     0,     0,
       0,     0,     0,     0,     0,   449,     0,   535,     0,     0,
    1345,     0,     0,     0,     0,     0,   535,     0,     0,     0,
       0,     0,  1076,     0,     0,     0,  1346,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   535,     0,     0,     0,
       0,   535,     0,     0,     0,     0,     0,     0,     0,   312,
       0,     0,     0,   535,     0,     0,   535,     0,     0,     0,
       0,     0,     0,     0,     0,   316,     0,  1345,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   312,  1346,     0,     0,     0,     0,     0,  1120,
       0,     0,     0,     0,     0,     0,     0,     0,   316,  1129,
       0,     0,     0,     0,  1345,     0,     0,     0,     0,     0,
       0,     0,     0,  1345,     0,     0,     0,     0,     0,     0,
    1346,     0,     0,     0,  1168,  1168,  1172,  1172,     0,  1346,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1168,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1202,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   558,  1233,     0,     0,     0,     0,     0,     0,     0,
       0,  1240,     0,  1240,     0,   770,     0,  1168,     0,     0,
       0,     0,     0,     0,  1168,   840,   841,     0,   849,   850,
     851,   853,     0,   855,     0,     0,     0,     0,   862,     0,
       0,     0,     0,     0,     0,     0,   871,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   804,     0,   804,
       0,     0,     0,     0,     0,     0,     0,     0,   893,   894,
       0,     0,     0,     0,     0,     0,     0,     0,   899,     0,
       0,   901,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1369,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   942,     0,     0,  1168,   942,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1007,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1044,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   770,     0,     0,
       0,  1465,     0,     0,     0,     0,     0,     0,     0,     0,
    1069,     0,     0,  1168,  1071,  1072,     0,     0,     0,     0,
       0,     0,     0,   901,     0,     0,     0,   798,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1168,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1516,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   894,     0,
       0,   859,     0,     0,  1122,  1126,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1553,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1191,     0,
       0,     0,     0,     0,  1168,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1232,  1610,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1248,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1269,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1635,  1274,
    1275,  1168,     0,     0,  1168,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1294,     0,  1648,  1297,     0,     0,
       0,  1652,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1322,  1323,  1324,     0,  1327,  1328,
    1329,  1330,  1331,  1332,  1333,  1334,  1335,     0,     0,  1338,
    1339,     0,     0,     0,  1671,  1672,     0,     0,  1353,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1122,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1434,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1168,     0,  1725,     0,  1727,     0,  1729,     0,     0,     0,
       0,     0,     0,  1737,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1168,     0,  1168,  1466,     0,  1468,     0,
    1470,     0,     0,     0,     0,     0,     0,     0,  1476,     0,
       0,  1761,     0,     0,  1764,     0,     0,  1480,     0,     0,
       0,     0,     0,  1484,  1485,  1168,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1771,     0,     0,     0,
       0,     0,  1507,     0,     0,  1508,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1168,     0,
       0,     0,     0,     0,     0,     0,  1799,     0,     0,     0,
       0,     0,  1168,     0,  1168,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1816,     0,     0,     0,     0,
       0,     0,     0,     0,  1819,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1122,     0,     0,     0,  1126,
    1548,     0,     0,     0,  1828,  1552,     0,     0,     0,  1829,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1838,     0,     0,  1841,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1614,     0,     0,     0,  1616,     0,     0,     0,  1619,     0,
    1621,     0,     0,     0,     0,     0,     0,     0,  1626,     0,
       0,     0,     0,     0,     0,     0,  1630,  1632,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1476,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1654,  1655,
    1656,  1657,     0,  1659,  1660,     0,  1661,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1668,     0,     0,     0,     0,  1670,     0,     0,     0,
       0,     0,  1674,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1712,     0,  1713,     0,     0,  1715,     0,     0,
    1717,     0,     0,     0,     0,  1719,  1632,     0,     0,     0,
       0,     0,     0,     0,  1724,     0,     0,     0,  1728,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1740,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1746,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1766,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1724,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1805,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  -165,
     881,     0,     0,     0,     0,  1817,     0,     0,     0,  1822,
       1,     2,     0,     0,  1805,     3,     0,     0,     0,     4,
       5,     6,     7,     8,     9,    10,    11,    12,     0,    13,
    1822,     0,    14,   420,  1831,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,     0,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,     0,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,     0,    72,    73,    74,    75,    76,     0,
      77,    78,    79,     0,     0,    80,    81,     0,     0,     0,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,     0,     0,     0,    93,     0,    94,    95,    96,    97,
       0,    98,     0,     0,     0,    99,   100,   101,   102,   103,
       0,     0,     0,     0,     0,     0,   104,   105,   106,   107,
     108,   109,   110,   111,   112,     0,   113,   114,   115,   116,
     117,     0,     0,     0,   118,   119,   120,   121,   122,   123,
     124,   125,     0,   126,     0,   127,   128,     0,     0,   129,
     130,   131,     0,     0,     0,   132,     0,   133,   134,   135,
     136,   137,     0,   138,     0,   139,     0,   140,   141,   142,
     143,   144,   145,   146,   147,   148,   149,   150,     0,     0,
     151,   152,   153,     0,   422,   155,   156,   157,   158,   159,
     160,   161,   162,   163,   164,   165,   166,   167,   168,   169,
     170,   171,   172,   173,   174,   175,   176,   177,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,     0,   197,   198,
     199,     0,   200,   201,   202,   203,   204,   205,   206,   207,
     208,   209,   210,   211,   212,   213,   214,   215,   216,   217,
     218,   219,   220,   221,   222,   223,   224,   225,   226,   227,
     228,   229,   230,   231,   232,   233,   234,   235,   236,   237,
     238,   239,   240,   241,   242,   243,   244,   245,   246,   247,
       0,   248,     0,   249,   250,   251,   252,   253,   254,   255,
    -165,   881,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     1,     2,     0,     0,     0,     3,     0,     0,     0,
       4,     5,     6,     7,     8,     9,    10,    11,    12,     0,
      13,     0,     0,   885,   420,     0,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,   421,
      30,    31,    32,    33,    34,     0,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,     0,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,     0,    72,    73,    74,    75,    76,
       0,    77,    78,    79,     0,     0,    80,    81,     0,     0,
       0,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,     0,     0,     0,    93,     0,    94,    95,    96,
      97,     0,    98,     0,     0,     0,    99,   100,   101,   102,
     103,     0,     0,     0,     0,     0,     0,   104,   105,   106,
     107,   108,   109,   110,   111,   112,     0,   113,   114,   115,
     116,   117,     0,     0,     0,   118,   119,   120,   121,   122,
     123,   124,   125,     0,   126,     0,   127,   128,     0,     0,
     129,   130,   131,     0,     0,     0,   132,     0,   133,   134,
     135,   136,   137,     0,   138,     0,   139,     0,   140,   141,
     142,   143,   144,   145,   146,   147,   148,   149,   150,     0,
       0,   151,   152,   153,     0,   422,   155,   156,   157,   158,
     159,   160,   161,   162,   163,   164,   165,   166,   167,   168,
     169,   170,   171,   172,   173,   174,   175,   176,   177,   178,
     179,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,     0,   197,
     198,   199,     0,   200,   201,   202,   203,   204,   205,   206,
     207,   208,   209,   210,   211,   212,   213,   214,   215,   216,
     217,   218,   219,   220,   221,   222,   223,   224,   225,   226,
     227,   228,   229,   230,   231,   232,   233,   234,   235,   236,
     237,   238,   239,   240,   241,   242,   243,   244,   245,   246,
     247,   881,   248,     0,   249,   250,   251,   252,   253,   254,
     255,     1,     2,     0,     0,     0,     3,     0,     0,     0,
       4,     5,     6,     7,     8,     9,    10,    11,    12,     0,
      13,     0,     0,   419,   420,     0,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,   421,
      30,    31,    32,    33,    34,     0,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,     0,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,     0,    72,    73,    74,    75,    76,
       0,    77,    78,    79,     0,     0,    80,    81,     0,     0,
       0,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,     0,     0,     0,    93,     0,    94,    95,    96,
      97,     0,    98,     0,     0,     0,    99,   100,   101,   102,
     103,     0,     0,     0,     0,     0,     0,   104,   105,   106,
     107,   108,   109,   110,   111,   112,     0,   113,   114,   115,
     116,   117,     0,     0,     0,   118,   119,   120,   121,   122,
     123,   124,   125,     0,   126,     0,   127,   128,     0,     0,
     129,   130,   131,     0,     0,     0,   132,     0,   133,   134,
     135,   136,   137,     0,   138,     0,   139,     0,   140,   141,
     142,   143,   144,   145,   146,   147,   148,   149,   150,     0,
       0,   151,   152,   153,     0,   422,   155,   156,   157,   158,
     159,   160,   161,   162,   163,   164,   165,   166,   167,   168,
     169,   170,   171,   172,   173,   174,   175,   176,   177,   178,
     179,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,     0,   197,
     198,   199,     0,   200,   201,   202,   203,   204,   205,   206,
     207,   208,   209,   210,   211,   212,   213,   214,   215,   216,
     217,   218,   219,   220,   221,   222,   223,   224,   225,   226,
     227,   228,   229,   230,   231,   232,   233,   234,   235,   236,
     237,   238,   239,   240,   241,   242,   243,   244,   245,   246,
     247,   881,   248,     0,   249,   250,   251,   252,   253,   254,
     255,     1,     2,     0,     0,     0,     3,     0,     0,     0,
       4,     5,     6,     7,     8,     9,    10,     0,     0,     0,
      13,     0,     0,   419,   420,     0,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,   560,   421,
      30,    31,    32,    33,    34,     0,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,     0,
      48,    49,    50,   561,   562,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,     0,    72,    73,    74,    75,    76,
       0,    77,    78,    79,     0,     0,    80,    81,     0,     0,
       0,    82,    83,   510,    85,    86,    87,    88,    89,   563,
      91,    92,     0,     0,     0,    93,     0,    94,    95,    96,
      97,     0,    98,     0,     0,     0,    99,   100,   101,   102,
     103,     0,     0,     0,     0,     0,     0,   104,   105,   106,
     107,   108,   109,   110,   564,   112,     0,   113,   114,   115,
     116,   117,     0,     0,     0,   118,   119,   120,   121,   122,
     123,   124,   125,     0,   126,     0,   127,   128,     0,     0,
     129,   130,   131,     0,     0,     0,   132,     0,   133,   134,
       0,   136,   137,     0,   138,     0,   139,     0,   140,   141,
     142,   143,   144,   145,   146,   147,   148,   149,   150,     0,
       0,   151,   152,   153,     0,   422,   155,   156,   157,   158,
     159,   160,   161,   162,   163,   164,   165,   166,   167,   168,
     169,   170,   171,   172,   173,   174,   175,   176,   528,   529,
     530,   180,   181,   531,   566,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,     0,   197,
     198,   199,     0,   200,   201,   202,   203,   204,   205,   206,
     207,   208,   209,   210,   211,   212,   213,   214,   215,   216,
     217,   218,   219,   220,   221,   222,   223,   224,   225,   226,
     227,   228,   229,   230,   231,   232,   233,   234,   235,   236,
     237,   238,   239,   240,   241,   242,   243,   244,   245,   246,
     247,     0,   248,     0,   249,   250,   251,   252,   253,   254,
     255,  -868,   462,     0,  -260,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  -868,     0,     0,     0,  -868,     0,     0,
       0,  -868,  -868,     0,     0,     0,     0,  -868,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  -260,     0,
       0,  -868,     0,  -260,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  -868,     0,  -868,  -868,  -868,     0,
       0,  -868,  -868,  -868,  -868,     0,     0,     0,  -868,  -868,
       0,     0,     0,     0,     0,  -868,     0,     0,  -868,  -868,
    -260,     0,     0,  -868,     0,     0,     0,     0,  -868,  -868,
       0,     0,     0,     0,  -868,     0,     0,     0,  -868,     0,
       0,     0,  -868,  -868,     0,  -868,     0,  -868,  -868,     0,
       0,     0,  -868,  -868,     0,     0,  -868,  -868,  -868,  -868,
    -868,  -868,     0,     0,  -868,     0,     0,     0,  -868,  -868,
       0,     0,  -868,     0,     0,     0,     0,  -868,     0,     0,
    -868,     0,     0,     0,     0,  -868,  -868,  -868,  -868,  -868,
       0,  -868,  -868,  -868,  -868,  -868,     0,     0,     0,     0,
    -868,  -868,  -868,     0,  -868,  -868,  -868,  -868,  -868,  -868,
       0,  -868,     0,  -868,     0,     0,     0,     0,  -868,  -868,
    -868,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  -868,     0,  -868,     0,  -868,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    -868,     0,     0,     0,     0,  -868,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     1,     2,     0,     0,     0,     3,
       0,     0,     0,     4,     5,     6,     7,     8,     9,    10,
      11,    12,  -868,    13,  -868,     0,    14,    15,     0,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,     0,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,     0,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,     0,    72,    73,
      74,    75,    76,     0,    77,    78,    79,     0,     0,    80,
      81,     0,     0,     0,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,     0,     0,     0,    93,     0,
      94,    95,    96,    97,     0,    98,     0,     0,     0,    99,
     100,   101,   102,   103,     0,     0,     0,     0,     0,     0,
     104,   105,   106,   107,   108,   109,   110,   111,   112,     0,
     113,   114,   115,   116,   117,     0,     0,     0,   118,   119,
     120,   121,   122,   123,   124,   125,     0,   126,     0,   127,
     128,     0,     0,   129,   130,   131,     0,     0,     0,   132,
       0,   133,   134,   135,   136,   137,     0,   138,     0,   139,
       0,   140,   141,   142,   143,   144,   145,   146,   147,   148,
     149,   150,     0,     0,   151,   152,   153,     0,   154,   155,
     156,   157,   158,   159,   160,   161,   162,   163,   164,   165,
     166,   167,   168,   169,   170,   171,   172,   173,   174,   175,
     176,   177,   178,   179,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,     0,   197,   198,   199,     0,   200,   201,   202,   203,
     204,   205,   206,   207,   208,   209,   210,   211,   212,   213,
     214,   215,   216,   217,   218,   219,   220,   221,   222,   223,
     224,   225,   226,   227,   228,   229,   230,   231,   232,   233,
     234,   235,   236,   237,   238,   239,   240,   241,   242,   243,
     244,   245,   246,   247,     0,   248,     0,   249,   250,   251,
     252,   253,   254,   255,   256,   257,     1,     2,     0,     0,
       0,     3,     0,     0,     0,     4,     5,     6,     7,     8,
       9,    10,    11,    12,     0,    13,     0,     0,    14,    15,
       0,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
       0,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,     0,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,     0,
      72,    73,    74,    75,    76,     0,    77,    78,    79,     0,
       0,    80,    81,     0,     0,     0,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,    92,     0,     0,     0,
      93,     0,    94,    95,    96,    97,     0,    98,     0,     0,
       0,    99,   100,   101,   102,   103,     0,     0,     0,     0,
       0,     0,   104,   105,   106,   107,   108,   109,   110,   111,
     112,     0,   113,   114,   115,   116,   117,     0,     0,     0,
     118,   119,   120,   121,   122,   123,   124,   125,     0,   126,
       0,   127,   128,     0,     0,   129,   130,   131,     0,     0,
       0,   132,     0,   133,   134,   135,   136,   137,     0,   138,
       0,   139,     0,   140,   141,   142,   143,   144,   145,   146,
     147,   148,   149,   150,     0,     0,   151,   152,   153,     0,
     154,   155,   156,   157,   158,   159,   160,   161,   162,   163,
     164,   165,   166,   167,   168,   169,   170,   171,   172,   173,
     174,   175,   176,   177,   178,   179,   180,   181,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,     0,   197,   198,   199,     0,   200,   201,
     202,   203,   204,   205,   206,   207,   208,   209,   210,   211,
     212,   213,   214,   215,   216,   217,   218,   219,   220,   221,
     222,   223,   224,   225,   226,   227,   228,   229,   230,   231,
     232,   233,   234,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,   245,   246,   247,     0,   248,     0,   249,
     250,   251,   252,   253,   254,   255,     0,   610,     1,     2,
       0,     0,     0,     3,     0,     0,     0,     4,     5,     6,
       7,     8,     9,    10,    11,    12,     0,    13,     0,     0,
     419,   420,     0,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,   421,    30,    31,    32,
      33,    34,     0,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,     0,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,     0,    72,    73,    74,    75,    76,     0,    77,    78,
      79,     0,     0,    80,    81,     0,     0,     0,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,     0,
       0,     0,    93,     0,    94,    95,    96,    97,     0,    98,
       0,     0,     0,    99,   100,   101,   102,   103,     0,     0,
       0,     0,     0,     0,   104,   105,   106,   107,   108,   109,
     110,   111,   112,     0,   113,   114,   115,   116,   117,     0,
       0,     0,   118,   119,   120,   121,   122,   123,   124,   125,
       0,   126,     0,   127,   128,     0,     0,   129,   130,   131,
       0,   555,     0,   132,     0,   133,   134,   135,   136,   137,
       0,   138,     0,   139,     0,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,     0,     0,   151,   152,
     153,     0,   422,   155,   156,   157,   158,   159,   160,   161,
     162,   163,   164,   165,   166,   167,   168,   169,   170,   171,
     172,   173,   174,   175,   176,   177,   178,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,     0,   197,   198,   199,     0,
     200,   201,   202,   203,   204,   205,   206,   207,   208,   209,
     210,   211,   212,   213,   214,   215,   216,   217,   218,   219,
     220,   221,   222,   223,   224,   225,   226,   227,   228,   229,
     230,   231,   232,   233,   234,   235,   236,   237,   238,   239,
     240,   241,   242,   243,   244,   245,   246,   247,     0,   248,
       0,   249,   250,   251,   252,   253,   254,   255,     1,     2,
       0,     0,     0,     3,     0,     0,     0,     4,     5,     6,
       7,     8,     9,    10,    11,    12,     0,    13,     0,     0,
     419,   420,     0,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,   421,    30,    31,    32,
      33,    34,     0,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,     0,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,     0,    72,    73,    74,    75,    76,     0,    77,    78,
      79,     0,     0,    80,    81,     0,     0,     0,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,     0,
       0,     0,    93,     0,    94,    95,    96,    97,     0,    98,
       0,     0,     0,    99,   100,   101,   102,   103,     0,     0,
       0,     0,     0,     0,   104,   105,   106,   107,   108,   109,
     110,   111,   112,     0,   113,   114,   115,   116,   117,     0,
       0,     0,   118,   119,   120,   121,   122,   123,   124,   125,
       0,   126,     0,   127,   128,     0,     0,   129,   130,   131,
       0,   816,     0,   132,     0,   133,   134,   135,   136,   137,
       0,   138,     0,   139,     0,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,     0,     0,   151,   152,
     153,     0,   422,   155,   156,   157,   158,   159,   160,   161,
     162,   163,   164,   165,   166,   167,   168,   169,   170,   171,
     172,   173,   174,   175,   176,   177,   178,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,     0,   197,   198,   199,     0,
     200,   201,   202,   203,   204,   205,   206,   207,   208,   209,
     210,   211,   212,   213,   214,   215,   216,   217,   218,   219,
     220,   221,   222,   223,   224,   225,   226,   227,   228,   229,
     230,   231,   232,   233,   234,   235,   236,   237,   238,   239,
     240,   241,   242,   243,   244,   245,   246,   247,     0,   248,
       0,   249,   250,   251,   252,   253,   254,   255,     1,     2,
       0,     0,     0,     3,     0,     0,     0,     4,     5,     6,
       7,     8,     9,    10,    11,    12,     0,    13,     0,     0,
     419,   420,     0,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,  1340,   421,    30,    31,    32,
      33,    34,     0,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,     0,    48,    49,    50,
    1341,  1342,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,     0,    72,    73,    74,    75,    76,     0,    77,    78,
      79,     0,     0,    80,    81,     0,     0,     0,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,     0,
       0,     0,    93,     0,    94,    95,    96,    97,     0,    98,
       0,     0,     0,    99,   100,   101,   102,   103,     0,     0,
       0,     0,     0,     0,   104,   105,   106,   107,   108,   109,
     110,   111,   112,     0,   113,   114,   115,   116,   117,     0,
       0,     0,   118,   119,   120,   121,   122,   123,   124,   125,
       0,   126,     0,   127,   128,     0,     0,   129,   130,   131,
       0,   555,     0,   132,     0,   133,   134,   135,   136,   137,
       0,   138,     0,   139,     0,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,     0,     0,   151,   152,
     153,     0,   422,   155,   156,   157,   158,   159,   160,   161,
     162,   163,   164,   165,   166,   167,   168,   169,   170,   171,
     172,   173,   174,   175,   176,   177,   178,   179,   180,   181,
     182,  1343,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,     0,   197,   198,   199,     0,
     200,   201,   202,   203,   204,   205,   206,   207,   208,   209,
     210,   211,   212,   213,   214,   215,   216,   217,   218,   219,
     220,   221,   222,   223,   224,   225,   226,   227,   228,   229,
     230,   231,   232,   233,   234,   235,   236,   237,   238,   239,
     240,   241,   242,   243,   244,   245,   246,   247,     0,   248,
       0,   249,   250,   251,   252,   253,   254,   255,     1,     2,
       0,     0,     0,     3,     0,     0,     0,     4,     5,     6,
       7,     8,     9,    10,    11,    12,     0,    13,     0,     0,
     419,   420,     0,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,   421,    30,    31,    32,
      33,    34,     0,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,     0,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,     0,    72,    73,    74,    75,    76,     0,    77,    78,
      79,     0,     0,    80,    81,     0,     0,     0,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,     0,
       0,     0,    93,     0,    94,    95,    96,    97,     0,    98,
       0,     0,     0,    99,   100,   101,   102,   103,     0,     0,
       0,     0,     0,     0,   104,   105,   106,   107,   108,   109,
     110,   111,   112,     0,   113,   114,   115,   116,   117,     0,
       0,     0,   118,   119,   120,   121,   122,   123,   124,   125,
       0,   126,     0,   127,   128,     0,     0,   129,   130,   131,
       0,     0,     0,   132,     0,   133,   134,   135,   136,   137,
       0,   138,     0,   139,     0,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,     0,     0,   151,   152,
     153,     0,   422,   155,   156,   157,   158,   159,   160,   161,
     162,   163,   164,   165,   166,   167,   168,   169,   170,   171,
     172,   173,   174,   175,   176,   177,   178,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,     0,   197,   198,   199,     0,
     200,   201,   202,   203,   204,   205,   206,   207,   208,   209,
     210,   211,   212,   213,   214,   215,   216,   217,   218,   219,
     220,   221,   222,   223,   224,   225,   226,   227,   228,   229,
     230,   231,   232,   233,   234,   235,   236,   237,   238,   239,
     240,   241,   242,   243,   244,   245,   246,   247,     0,   248,
       0,   249,   250,   251,   252,   253,   254,   255,     1,     2,
       0,     0,     0,     3,     0,     0,     0,     4,     5,     6,
       7,     8,     9,    10,    11,    12,     0,    13,     0,     0,
      14,    15,     0,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,     0,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,     0,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,     0,    72,    73,    74,    75,    76,     0,    77,    78,
      79,     0,     0,    80,    81,     0,     0,     0,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,     0,
       0,     0,    93,     0,    94,    95,    96,    97,     0,    98,
       0,     0,     0,    99,   100,   101,   102,   103,     0,     0,
       0,     0,     0,     0,   104,   105,   106,   107,   108,   109,
     110,   111,   112,     0,   113,   114,   115,   116,   117,     0,
       0,     0,   118,   119,   120,   121,   122,   123,   124,   125,
       0,   126,     0,   127,   128,     0,     0,   129,   130,   131,
       0,     0,     0,   132,     0,   133,   134,   135,   136,   137,
       0,   138,     0,   139,     0,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,     0,     0,   151,   152,
     153,     0,   154,   155,   156,   157,   158,   159,   160,   161,
     162,   163,   164,   165,   166,   167,   168,   169,   170,   171,
     172,   173,   174,   175,   176,   177,   178,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,     0,   197,   198,   199,     0,
     200,   201,   202,   203,   204,   205,   206,   207,   208,   209,
     210,   211,   212,   213,   214,   215,   216,   217,   218,   219,
     220,   221,   222,   223,   224,   225,   226,   227,   228,   229,
     230,   231,   232,   233,   234,   235,   236,   237,   238,   239,
     240,   241,   242,   243,   244,   245,   246,   247,     0,   248,
       0,   249,   250,   251,   252,   253,   254,   255,     1,     2,
       0,     0,     0,     3,     0,     0,     0,     4,     5,     6,
       7,     8,     9,    10,    11,    12,     0,    13,     0,     0,
      14,    15,     0,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,     0,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,     0,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,     0,    72,    73,    74,    75,    76,     0,    77,    78,
      79,     0,     0,    80,    81,     0,     0,     0,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,     0,
       0,     0,    93,     0,    94,    95,    96,    97,     0,    98,
       0,     0,     0,    99,   100,   101,   102,   103,     0,     0,
       0,     0,     0,     0,   104,   105,   106,   107,   108,   109,
     110,   111,   112,     0,   113,   114,   115,   116,   117,     0,
       0,     0,   118,   119,   120,   121,   122,   123,   124,   125,
       0,   126,     0,   127,   128,     0,     0,   129,   130,   131,
       0,     0,     0,   132,     0,   133,   134,   135,   136,   137,
       0,   138,     0,   139,     0,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,     0,     0,   151,   152,
     153,     0,   422,   155,   156,   157,   158,   159,   160,   161,
     162,   163,   164,   165,   166,   167,   168,   169,   170,   171,
     172,   173,   174,   175,   176,   177,   178,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,     0,   197,   198,   199,     0,
     200,   201,   202,   203,   204,   205,   206,   207,   208,   209,
     210,   211,   212,   213,   214,   215,   216,   217,   218,   219,
     220,   221,   222,   223,   224,   225,   226,   227,   228,   229,
     230,   231,   232,   233,   234,   235,   236,   237,   238,   239,
     240,   241,   242,   243,   244,   245,   246,   247,     0,   248,
       0,   249,   250,   251,   252,   253,   254,   255,     1,     2,
       0,     0,     0,     3,     0,     0,     0,     4,     5,     6,
       7,     8,     9,    10,    11,    12,     0,    13,     0,     0,
      14,   420,     0,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,     0,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,     0,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,     0,    72,    73,    74,    75,    76,     0,    77,    78,
      79,     0,     0,    80,    81,     0,     0,     0,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,     0,
       0,     0,    93,     0,    94,    95,    96,    97,     0,    98,
       0,     0,     0,    99,   100,   101,   102,   103,     0,     0,
       0,     0,     0,     0,   104,   105,   106,   107,   108,   109,
     110,   111,   112,     0,   113,   114,   115,   116,   117,     0,
       0,     0,   118,   119,   120,   121,   122,   123,   124,   125,
       0,   126,     0,   127,   128,     0,     0,   129,   130,   131,
       0,     0,     0,   132,     0,   133,   134,   135,   136,   137,
       0,   138,     0,   139,     0,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,     0,     0,   151,   152,
     153,     0,   422,   155,   156,   157,   158,   159,   160,   161,
     162,   163,   164,   165,   166,   167,   168,   169,   170,   171,
     172,   173,   174,   175,   176,   177,   178,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,     0,   197,   198,   199,     0,
     200,   201,   202,   203,   204,   205,   206,   207,   208,   209,
     210,   211,   212,   213,   214,   215,   216,   217,   218,   219,
     220,   221,   222,   223,   224,   225,   226,   227,   228,   229,
     230,   231,   232,   233,   234,   235,   236,   237,   238,   239,
     240,   241,   242,   243,   244,   245,   246,   247,     0,   248,
       0,   249,   250,   251,   252,   253,   254,   255,     1,     2,
       0,     0,     0,     3,     0,     0,     0,     4,     5,     6,
       7,     8,     9,    10,    11,    12,     0,    13,     0,     0,
     885,   420,     0,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,   421,    30,    31,    32,
      33,    34,     0,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,     0,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,     0,    72,    73,    74,    75,    76,     0,    77,    78,
      79,     0,     0,    80,    81,     0,     0,     0,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,     0,
       0,     0,    93,     0,    94,    95,    96,    97,     0,    98,
       0,     0,     0,    99,   100,   101,   102,   103,     0,     0,
       0,     0,     0,     0,   104,   105,   106,   107,   108,   109,
     110,   111,   112,     0,   113,   114,   115,   116,   117,     0,
       0,     0,   118,   119,   120,   121,   122,   123,   124,   125,
       0,   126,     0,   127,   128,     0,     0,   129,   130,   131,
       0,     0,     0,   132,     0,   133,   134,   135,   136,   137,
       0,   138,     0,   139,     0,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,     0,     0,   151,   152,
     153,     0,   422,   155,   156,   157,   158,   159,   160,   161,
     162,   163,   164,   165,   166,   167,   168,   169,   170,   171,
     172,   173,   174,   175,   176,   177,   178,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,     0,   197,   198,   199,     0,
     200,   201,   202,   203,   204,   205,   206,   207,   208,   209,
     210,   211,   212,   213,   214,   215,   216,   217,   218,   219,
     220,   221,   222,   223,   224,   225,   226,   227,   228,   229,
     230,   231,   232,   233,   234,   235,   236,   237,   238,   239,
     240,   241,   242,   243,   244,   245,   246,   247,     0,   248,
       0,   249,   250,   251,   252,   253,   254,   255,     1,     2,
       0,     0,     0,     3,     0,     0,     0,     4,     5,     6,
       7,     8,     9,    10,    11,    12,     0,    13,     0,     0,
     419,   420,     0,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,  1340,   421,    30,    31,    32,
      33,    34,     0,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,     0,    48,    49,    50,
    1341,  1342,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,     0,    72,    73,    74,    75,    76,     0,    77,    78,
      79,     0,     0,    80,    81,     0,     0,     0,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,     0,
       0,     0,    93,     0,    94,    95,    96,    97,     0,    98,
       0,     0,     0,    99,   100,   101,   102,   103,     0,     0,
       0,     0,     0,     0,   104,   105,   106,   107,   108,   109,
     110,   111,   112,     0,   113,   114,   115,   116,   117,     0,
       0,     0,   118,   119,   120,   121,   122,   123,   124,   125,
       0,   126,     0,   127,   128,     0,     0,   129,   130,   131,
       0,     0,     0,   132,     0,   133,   134,   135,   136,   137,
       0,   138,     0,   139,     0,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,     0,     0,   151,   152,
     153,     0,   422,   155,   156,   157,   158,   159,   160,   161,
     162,   163,   164,   165,   166,   167,   168,   169,   170,   171,
     172,   173,   174,   175,   176,   177,   178,   179,   180,   181,
     182,  1343,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,     0,   197,   198,   199,     0,
     200,   201,   202,   203,   204,   205,   206,   207,   208,   209,
     210,   211,   212,   213,   214,   215,   216,   217,   218,   219,
     220,   221,   222,   223,   224,   225,   226,   227,   228,   229,
     230,   231,   232,   233,   234,   235,   236,   237,   238,   239,
     240,   241,   242,   243,   244,   245,   246,   247,     0,   248,
       0,   249,   250,   251,   252,   253,   254,   255,     1,     2,
       0,     0,     0,     3,     0,     0,     0,     4,     5,     6,
       7,     8,     9,    10,     0,     0,     0,    13,     0,     0,
     419,   420,     0,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,   560,   421,    30,    31,    32,
      33,    34,     0,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,     0,    48,    49,    50,
     561,   562,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,     0,    72,    73,    74,    75,    76,     0,    77,    78,
      79,     0,     0,    80,    81,     0,     0,     0,    82,    83,
     510,    85,    86,    87,    88,    89,   563,    91,    92,     0,
       0,     0,    93,     0,    94,    95,    96,    97,     0,    98,
       0,     0,     0,    99,   100,   101,   102,   103,     0,     0,
       0,     0,   940,     0,   104,   105,   106,   107,   108,   109,
     110,   564,   112,     0,   113,   114,   115,   116,   117,     0,
       0,     0,   118,   119,   120,   121,   122,   123,   124,   125,
       0,   126,     0,   127,   128,     0,     0,   129,   130,   131,
       0,     0,     0,   132,   941,   133,   134,     0,   136,   137,
       0,   138,     0,   139,     0,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,     0,     0,   151,   152,
     153,     0,   422,   155,   156,   157,   158,   159,   160,   161,
     162,   163,   164,   165,   166,   167,   168,   169,   170,   171,
     172,   173,   174,   175,   176,   528,   529,   530,   180,   181,
     531,   566,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,     0,   197,   198,   199,     0,
     200,   201,   202,   203,   204,   205,   206,   207,   208,   209,
     210,   211,   212,   213,   214,   215,   216,   217,   218,   219,
     220,   221,   222,   223,   224,   225,   226,   227,   228,   229,
     230,   231,   232,   233,   234,   235,   236,   237,   238,   239,
     240,   241,   242,   243,   244,   245,   246,   247,     0,   248,
       0,   249,   250,   251,   252,   253,   254,   255,     1,     2,
       0,     0,     0,     3,     0,     0,     0,     4,     5,     6,
       7,     8,     9,    10,     0,     0,     0,    13,     0,     0,
     419,   420,     0,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,   560,   421,    30,    31,    32,
      33,    34,     0,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,     0,    48,    49,    50,
     561,   562,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,     0,    72,    73,    74,    75,    76,     0,    77,    78,
      79,     0,     0,    80,    81,     0,     0,     0,    82,    83,
     510,    85,    86,    87,    88,    89,   563,    91,    92,     0,
       0,     0,    93,     0,    94,    95,    96,    97,     0,    98,
       0,     0,     0,    99,   100,   101,   102,   103,     0,     0,
       0,     0,   940,     0,   104,   105,   106,   107,   108,   109,
     110,   564,   112,     0,   113,   114,   115,   116,   117,     0,
       0,     0,   118,   119,   120,   121,   122,   123,   124,   125,
       0,   126,     0,   127,   128,     0,     0,   129,   130,   131,
       0,     0,     0,   132,   946,   133,   134,     0,   136,   137,
       0,   138,     0,   139,     0,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,     0,     0,   151,   152,
     153,     0,   422,   155,   156,   157,   158,   159,   160,   161,
     162,   163,   164,   165,   166,   167,   168,   169,   170,   171,
     172,   173,   174,   175,   176,   528,   529,   530,   180,   181,
     531,   566,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,     0,   197,   198,   199,     0,
     200,   201,   202,   203,   204,   205,   206,   207,   208,   209,
     210,   211,   212,   213,   214,   215,   216,   217,   218,   219,
     220,   221,   222,   223,   224,   225,   226,   227,   228,   229,
     230,   231,   232,   233,   234,   235,   236,   237,   238,   239,
     240,   241,   242,   243,   244,   245,   246,   247,     0,   248,
       0,   249,   250,   251,   252,   253,   254,   255,     1,     2,
       0,     0,     0,     3,     0,     0,     0,     4,     5,     6,
       7,     8,     9,    10,     0,     0,     0,    13,     0,     0,
     419,   420,     0,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,   560,   421,    30,    31,    32,
      33,    34,     0,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,     0,    48,    49,    50,
     561,   562,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,     0,    72,    73,    74,    75,    76,     0,    77,    78,
      79,     0,     0,    80,    81,     0,     0,     0,    82,    83,
     510,    85,    86,    87,    88,    89,   563,    91,    92,     0,
       0,     0,    93,     0,    94,    95,    96,    97,     0,    98,
       0,     0,     0,    99,   100,   101,   102,   103,     0,     0,
       0,     0,     0,     0,   104,   105,   106,   107,   108,   109,
     110,   564,   112,     0,   113,   114,   115,   116,   117,     0,
       0,     0,   118,   119,   120,   121,   122,   123,   124,   125,
       0,   126,     0,   127,   128,     0,     0,   129,   130,   131,
       0,     0,   565,   132,     0,   133,   134,     0,   136,   137,
       0,   138,     0,   139,     0,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,     0,     0,   151,   152,
     153,     0,   422,   155,   156,   157,   158,   159,   160,   161,
     162,   163,   164,   165,   166,   167,   168,   169,   170,   171,
     172,   173,   174,   175,   176,   528,   529,   530,   180,   181,
     531,   566,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,     0,   197,   198,   199,     0,
     200,   201,   202,   203,   204,   205,   206,   207,   208,   209,
     210,   211,   212,   213,   214,   215,   216,   217,   218,   219,
     220,   221,   222,   223,   224,   225,   226,   227,   228,   229,
     230,   231,   232,   233,   234,   235,   236,   237,   238,   239,
     240,   241,   242,   243,   244,   245,   246,   247,     0,   248,
       0,   249,   250,   251,   252,   253,   254,   255,     1,     2,
       0,     0,     0,     3,     0,     0,     0,     4,     5,     6,
       7,     8,     9,    10,     0,     0,     0,    13,     0,     0,
     419,   420,     0,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,   560,   421,    30,    31,    32,
      33,    34,     0,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,     0,    48,    49,    50,
     561,   562,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,     0,    72,    73,    74,    75,    76,     0,    77,    78,
      79,     0,     0,    80,    81,     0,     0,     0,    82,    83,
     510,    85,    86,    87,    88,    89,   563,    91,    92,     0,
       0,     0,    93,     0,    94,    95,    96,    97,     0,    98,
       0,     0,     0,    99,   100,   101,   102,   103,     0,     0,
       0,     0,     0,     0,   104,   105,   106,   107,   108,   109,
     110,   564,   112,     0,   113,   114,   115,   116,   117,     0,
       0,     0,   118,   119,   120,   121,   122,   123,   124,   125,
       0,   126,     0,   127,   128,     0,     0,   129,   130,   131,
       0,     0,     0,   132,   570,   133,   134,     0,   136,   137,
       0,   138,     0,   139,     0,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,     0,     0,   151,   152,
     153,     0,   422,   155,   156,   157,   158,   159,   160,   161,
     162,   163,   164,   165,   166,   167,   168,   169,   170,   171,
     172,   173,   174,   175,   176,   528,   529,   530,   180,   181,
     531,   566,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,     0,   197,   198,   199,     0,
     200,   201,   202,   203,   204,   205,   206,   207,   208,   209,
     210,   211,   212,   213,   214,   215,   216,   217,   218,   219,
     220,   221,   222,   223,   224,   225,   226,   227,   228,   229,
     230,   231,   232,   233,   234,   235,   236,   237,   238,   239,
     240,   241,   242,   243,   244,   245,   246,   247,     0,   248,
       0,   249,   250,   251,   252,   253,   254,   255,     1,     2,
       0,     0,     0,     3,     0,     0,     0,     4,     5,     6,
       7,     8,     9,    10,     0,     0,     0,    13,     0,     0,
     419,   420,     0,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,   560,   421,    30,    31,    32,
      33,    34,     0,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,     0,    48,    49,    50,
     561,   562,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,     0,    72,    73,    74,    75,    76,     0,    77,    78,
      79,     0,     0,    80,    81,     0,     0,     0,    82,    83,
     510,    85,    86,    87,    88,    89,   563,    91,    92,     0,
       0,     0,    93,     0,    94,    95,    96,    97,     0,    98,
       0,     0,     0,    99,   100,   101,   102,   103,     0,     0,
       0,     0,     0,     0,   104,   105,   106,   107,   108,   109,
     110,   564,   112,     0,   113,   114,   115,   116,   117,     0,
       0,     0,   118,   119,   120,   121,   122,   123,   124,   125,
       0,   126,     0,   127,   128,     0,     0,   129,   130,   131,
       0,     0,     0,   132,     0,   133,   134,     0,   136,   137,
       0,   138,     0,   139,     0,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,     0,     0,   151,   152,
     153,     0,   422,   155,   156,   157,   158,   159,   160,   161,
     162,   163,   164,   165,   166,   167,   168,   169,   170,   171,
     172,   173,   174,   175,   176,   528,   529,   530,   180,   181,
     531,   566,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,     0,   197,   198,   199,     0,
     200,   201,   202,   203,   204,   205,   206,   207,   208,   209,
     210,   211,   212,   213,   214,   215,   216,   217,   218,   219,
     220,   221,   222,   223,   224,   225,   226,   227,   228,   229,
     230,   231,   232,   233,   234,   235,   236,   237,   238,   239,
     240,   241,   242,   243,   244,   245,   246,   247,   605,   248,
       0,   249,   250,   251,   252,   253,   254,   255,     1,     2,
       0,     0,     0,     3,     0,     0,     0,     4,     5,     6,
       7,     8,     9,    10,     0,     0,     0,    13,     0,     0,
     419,   420,     0,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,   560,   421,    30,    31,    32,
      33,    34,     0,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,     0,    48,    49,    50,
     561,   562,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,     0,    72,    73,    74,    75,    76,     0,    77,    78,
      79,     0,     0,    80,    81,     0,     0,     0,    82,    83,
     510,    85,    86,    87,    88,    89,   563,    91,    92,     0,
       0,     0,    93,     0,    94,    95,    96,    97,     0,    98,
       0,     0,     0,    99,   100,   101,   102,   103,     0,     0,
       0,     0,     0,     0,   104,   105,   106,   107,   108,   109,
     110,   564,   112,     0,   113,   114,   115,   116,   117,     0,
       0,     0,   118,   119,   120,   121,   122,   123,   124,   125,
       0,   126,     0,   127,   128,     0,     0,   129,   130,   131,
       0,     0,     0,   132,     0,   133,   134,     0,   136,   137,
       0,   138,     0,   139,     0,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,     0,     0,   151,   152,
     153,     0,   422,   155,   156,   157,   158,   159,   160,   161,
     162,   163,   164,   165,   166,   167,   168,   169,   170,   171,
     172,   173,   174,   175,   176,   528,   529,   530,   180,   181,
     531,   566,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,     0,   197,   198,   199,     0,
     200,   201,   202,   203,   204,   205,   206,   207,   208,   209,
     210,   211,   212,   213,   214,   215,   216,   217,   218,   219,
     220,   221,   222,   223,   224,   225,   226,   227,   228,   229,
     230,   231,   232,   233,   234,   235,   236,   237,   238,   239,
     240,   241,   242,   243,   244,   245,   246,   247,     0,   248,
     607,   249,   250,   251,   252,   253,   254,   255,     1,     2,
       0,     0,     0,     3,     0,     0,     0,     4,     5,     6,
       7,     8,     9,    10,     0,     0,     0,    13,     0,     0,
     419,   420,     0,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,   560,   421,    30,    31,    32,
      33,    34,     0,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,     0,    48,    49,    50,
     561,   562,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,     0,    72,    73,    74,    75,    76,     0,    77,    78,
      79,     0,     0,    80,    81,     0,     0,     0,    82,    83,
     510,    85,    86,    87,    88,    89,   563,    91,    92,     0,
       0,     0,    93,     0,    94,    95,    96,    97,     0,    98,
       0,     0,     0,    99,   100,   101,   102,   103,     0,     0,
       0,     0,  1433,     0,   104,   105,   106,   107,   108,   109,
     110,   564,   112,     0,   113,   114,   115,   116,   117,     0,
       0,     0,   118,   119,   120,   121,   122,   123,   124,   125,
       0,   126,     0,   127,   128,     0,     0,   129,   130,   131,
       0,     0,     0,   132,     0,   133,   134,     0,   136,   137,
       0,   138,     0,   139,     0,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,     0,     0,   151,   152,
     153,     0,   422,   155,   156,   157,   158,   159,   160,   161,
     162,   163,   164,   165,   166,   167,   168,   169,   170,   171,
     172,   173,   174,   175,   176,   528,   529,   530,   180,   181,
     531,   566,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,     0,   197,   198,   199,     0,
     200,   201,   202,   203,   204,   205,   206,   207,   208,   209,
     210,   211,   212,   213,   214,   215,   216,   217,   218,   219,
     220,   221,   222,   223,   224,   225,   226,   227,   228,   229,
     230,   231,   232,   233,   234,   235,   236,   237,   238,   239,
     240,   241,   242,   243,   244,   245,   246,   247,     0,   248,
       0,   249,   250,   251,   252,   253,   254,   255,     1,     2,
       0,     0,     0,     3,     0,     0,     0,     4,     5,     6,
       7,     8,     9,    10,     0,     0,     0,    13,     0,     0,
     419,   420,     0,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,   496,    27,   498,   421,    30,   499,    32,
      33,    34,     0,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,   501,     0,    48,    49,    50,
     503,   504,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,     0,    72,    73,    74,    75,    76,     0,    77,    78,
      79,     0,     0,    80,    81,     0,     0,     0,    82,    83,
     510,    85,    86,    87,    88,    89,   563,    91,    92,     0,
       0,     0,    93,     0,    94,    95,    96,    97,     0,    98,
       0,     0,     0,   513,   100,   101,   102,   103,     0,     0,
       0,     0,     0,     0,   104,   105,   106,   107,   108,   109,
     110,   564,   112,     0,   113,   114,   692,   116,   117,     0,
       0,     0,   118,   119,   120,   121,   122,   123,   124,   125,
       0,   126,     0,   693,   128,     0,     0,   129,   130,   131,
       0,     0,     0,   132,     0,   133,   134,     0,   136,   137,
       0,   138,     0,   139,     0,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,     0,     0,   151,   152,
     153,     0,   422,   155,   156,   157,   158,   159,   160,   161,
     162,   523,   164,   524,   166,   525,   526,   169,   170,   171,
     172,   173,   174,   527,   176,   528,   529,   530,   180,   181,
     531,   532,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,     0,   197,   198,   199,     0,
     200,   201,   202,   203,   204,   205,   206,   207,   208,   209,
     210,   211,   212,   213,   214,   215,   216,   217,   218,   219,
     220,   221,   222,   223,   224,   225,   226,   227,   228,   229,
     230,   231,   232,   233,   234,   235,   236,   237,   238,   239,
     240,   241,   242,   243,   244,   245,   246,   247,     0,   248,
       0,   249,   533,   251,   252,   253,   254,   255,     1,     2,
       0,     0,     0,     3,     0,     0,     0,     4,     5,     6,
       7,     8,     9,    10,     0,     0,     0,    13,     0,     0,
     419,   420,     0,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,   560,   421,    30,    31,    32,
      33,    34,     0,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,     0,    48,    49,    50,
     561,   562,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,     0,    72,    73,    74,    75,    76,     0,    77,    78,
      79,     0,     0,    80,    81,     0,     0,     0,    82,    83,
     510,    85,    86,    87,    88,    89,   563,    91,    92,     0,
       0,     0,    93,     0,    94,    95,    96,    97,     0,    98,
       0,     0,     0,    99,   100,   101,   102,   103,     0,     0,
       0,     0,     0,     0,   104,   105,   106,   107,   108,   109,
     110,   564,   112,     0,   113,   114,   115,   116,   117,     0,
       0,     0,   118,   119,   120,   121,   122,   123,   124,   125,
       0,   126,     0,   127,   128,     0,     0,   129,   130,   131,
       0,     0,     0,   132,     0,   133,   134,     0,   136,   137,
       0,   138,     0,   139,     0,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,     0,     0,   151,   152,
     153,     0,   422,   155,   156,   157,   158,   159,   160,   161,
     162,   163,   164,   165,   166,   167,   168,   169,   170,   171,
     172,   173,   174,   175,   176,   528,   529,   530,   180,   181,
     531,   566,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,     0,   197,   198,   199,     0,
     200,   201,   202,   203,   204,   205,   206,   207,   208,   209,
     210,   211,   212,   213,   214,   215,   216,   217,   218,   219,
     220,   221,   222,   223,   224,   225,   226,   227,   228,   229,
     230,   231,   232,   233,   234,   235,   236,   237,   238,   239,
     240,   241,   242,   243,   244,   245,   246,   247,     0,   248,
       0,   249,   250,   251,   252,   253,   254,   255,     1,     2,
       0,     0,     0,     3,     0,     0,     0,     4,     5,     6,
       7,     8,     9,    10,     0,     0,     0,    13,     0,     0,
     419,   420,     0,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,   496,    27,   498,   421,    30,   499,    32,
      33,    34,     0,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,   501,     0,    48,    49,    50,
     503,   504,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,     0,    72,    73,    74,    75,    76,     0,    77,    78,
      79,     0,     0,    80,    81,     0,     0,     0,    82,    83,
     510,    85,    86,    87,    88,    89,   563,    91,    92,     0,
       0,     0,    93,     0,    94,    95,    96,    97,     0,    98,
       0,     0,     0,   513,   100,   101,   102,   103,     0,     0,
       0,     0,     0,     0,   104,   105,   106,   107,   108,   109,
     110,   564,   112,     0,   113,   114,   115,   116,   117,     0,
       0,     0,   118,   119,   120,   121,   122,   123,   124,   125,
       0,   126,     0,   127,   128,     0,     0,   129,   130,   131,
       0,     0,     0,   132,     0,   133,   134,     0,   136,   137,
       0,   138,     0,   139,     0,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,     0,     0,   151,   152,
     153,     0,   422,   155,   156,   157,   158,   159,   160,   161,
     162,   523,   164,   524,   166,   525,   526,   169,   170,   171,
     172,   173,   174,   527,   176,   528,   529,   530,   180,   181,
     531,   532,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,     0,   197,   198,   199,     0,
     200,   201,   202,   203,   204,   205,   206,   207,   208,   209,
     210,   211,   212,   213,   214,   215,   216,   217,   218,   219,
     220,   221,   222,   223,   224,   225,   226,   227,   228,   229,
     230,   231,   232,   233,   234,   235,   236,   237,   238,   239,
     240,   241,   242,   243,   244,   245,   246,   247,     0,   248,
       0,   249,   533,   251,   252,   253,   254,   255,     1,     2,
       0,     0,     0,     3,     0,     0,     0,     4,     5,     6,
       7,     8,     9,    10,     0,     0,     0,    13,     0,     0,
     419,   420,     0,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,   560,   421,    30,    31,    32,
      33,    34,     0,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,     0,    48,    49,    50,
     561,   562,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,     0,    72,    73,    74,    75,    76,     0,    77,    78,
      79,     0,     0,    80,    81,     0,     0,     0,    82,    83,
     510,    85,    86,    87,    88,    89,  1125,    91,    92,     0,
       0,     0,    93,     0,    94,    95,    96,    97,     0,    98,
       0,     0,     0,    99,   100,   101,   102,   103,     0,     0,
       0,     0,     0,     0,   104,   105,   106,   107,   108,   109,
     110,   564,   112,     0,   113,   114,   115,   116,   117,     0,
       0,     0,   118,   119,   120,   121,   122,   123,   124,   125,
       0,   126,     0,   127,   128,     0,     0,   129,   130,   131,
       0,     0,     0,   132,     0,   133,   134,     0,   136,   137,
       0,   138,     0,   139,     0,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,     0,     0,   151,   152,
     153,     0,   422,   155,   156,   157,   158,   159,   160,   161,
     162,   163,   164,   165,   166,   167,   168,   169,   170,   171,
     172,   173,   174,   175,   176,   528,   529,   530,   180,   181,
     531,   566,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,     0,   197,   198,   199,     0,
     200,   201,   202,   203,   204,   205,   206,   207,   208,   209,
     210,   211,   212,   213,   214,   215,   216,   217,   218,   219,
     220,   221,   222,   223,   224,   225,   226,   227,   228,   229,
     230,   231,   232,   233,   234,   235,   236,   237,   238,   239,
     240,   241,   242,   243,   244,   245,   246,   247,     0,   248,
       0,   249,   250,   251,   252,   253,   254,   255,     1,     2,
       0,     0,     0,     3,     0,     0,     0,     4,     5,     6,
       7,     8,     9,    10,     0,     0,     0,    13,     0,     0,
     419,   420,     0,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,   496,    27,   498,   421,    30,   499,    32,
      33,    34,     0,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,   501,     0,    48,    49,    50,
     503,   504,    53,   505,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,     0,    72,    73,    74,    75,    76,     0,    77,    78,
      79,     0,     0,    80,    81,     0,     0,     0,    82,    83,
     510,    85,    86,    87,    88,    89,   563,    91,    92,     0,
       0,     0,    93,     0,    94,    95,    96,    97,     0,    98,
       0,     0,     0,   513,   100,   101,   102,   103,     0,     0,
       0,     0,     0,     0,   104,   105,   106,   107,   108,   109,
     110,   564,   112,     0,   113,   114,     0,   116,   117,     0,
       0,     0,   118,   119,   120,   121,   122,   123,   124,   125,
       0,   126,     0,     0,     0,     0,     0,   129,   130,   131,
       0,     0,     0,   132,     0,   133,   134,     0,   136,   137,
       0,   138,     0,   139,     0,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,     0,     0,   151,   152,
     153,     0,   422,   155,   156,   157,   158,   159,   160,   161,
     162,   523,   164,   524,   166,   525,   526,   169,   170,   171,
     172,   173,   174,   527,   176,   528,   529,   530,   180,   181,
     531,   532,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,     0,   197,   198,   199,     0,
     200,   201,   202,   203,   204,   205,   206,   207,   208,   209,
     210,   211,   212,   213,   214,   215,   216,   217,   218,   219,
     220,   221,   222,   223,   224,   225,   226,   227,   228,   229,
     230,   231,   232,   233,   234,   235,   236,   237,   238,   239,
     240,   241,   242,   243,   244,   245,   246,   247,     0,   248,
       0,   249,   533,   251,   252,   253,   254,   255,     1,     2,
       0,     0,     0,     3,     0,     0,     0,     4,     5,     6,
       7,     8,     9,    10,     0,     0,     0,    13,     0,     0,
     419,   420,     0,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,   496,    27,   498,   421,    30,   499,    32,
      33,    34,     0,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,   501,     0,    48,    49,    50,
     503,   504,    53,   505,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,     0,    72,    73,    74,    75,    76,     0,    77,    78,
      79,     0,     0,    80,    81,     0,     0,     0,    82,    83,
     510,    85,    86,    87,    88,    89,   563,    91,    92,     0,
       0,     0,    93,     0,    94,    95,    96,    97,     0,    98,
       0,     0,     0,   513,   100,   101,   102,   103,     0,     0,
       0,     0,     0,     0,   104,   105,   106,   107,   108,   109,
     110,   564,   112,     0,   113,   114,     0,   116,   117,     0,
       0,     0,   118,   119,   120,   121,   122,   123,   124,   125,
       0,   126,     0,     0,     0,     0,     0,   129,   130,   131,
       0,     0,     0,   132,     0,   133,   134,     0,     0,     0,
       0,   138,     0,   139,     0,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,     0,     0,   151,   152,
     153,     0,   422,   155,   156,   157,   158,   159,   160,   161,
     162,   523,   164,   524,   166,   525,   526,   169,   170,   171,
     172,   173,   174,   527,   176,   528,   529,   530,   180,   181,
     531,   532,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,     0,   197,   198,   199,     0,
     200,   201,   202,   203,   204,   205,   206,   207,   208,   209,
     210,   211,   212,   213,   214,   215,   216,   217,   218,   219,
     220,   221,   222,   223,   224,   225,   226,   227,   228,   229,
     230,   231,   232,   233,   234,   235,   236,   237,   238,   239,
     240,   241,   242,   243,   244,   245,   246,   247,     0,   248,
       0,   249,   533,   251,   252,   253,   254,   255,     1,     2,
       0,     0,     0,     3,     0,     0,     0,     4,     5,     6,
       7,     8,     9,    10,     0,     0,     0,    13,     0,     0,
     419,   420,     0,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,   496,    27,   498,   421,    30,   499,    32,
      33,    34,     0,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,   501,     0,    48,    49,    50,
     503,   504,    53,   505,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,     0,    72,    73,    74,    75,    76,     0,    77,    78,
      79,     0,     0,    80,    81,     0,     0,     0,    82,    83,
     510,    85,    86,    87,    88,    89,   563,    91,    92,     0,
       0,     0,    93,     0,    94,    95,    96,    97,     0,    98,
       0,     0,     0,   513,     0,   101,   102,   103,     0,     0,
       0,     0,     0,     0,   104,   105,   106,   107,     0,   109,
     110,   564,   112,     0,   113,   114,     0,   116,   117,     0,
       0,     0,   118,   119,   120,   121,   122,   123,   124,   125,
       0,   126,     0,     0,     0,     0,     0,   129,   130,   131,
       0,     0,     0,   132,     0,   133,   134,     0,     0,     0,
       0,   138,     0,   139,     0,   140,     0,   142,     0,   144,
     145,   146,   147,   148,   149,   150,     0,     0,   151,   152,
     153,     0,   422,   155,   156,   157,   158,   159,   160,   161,
     162,   523,   164,   524,   166,   525,   526,   169,   170,   171,
     172,   173,   174,   527,   176,   528,   529,   530,   180,   181,
     531,   532,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,     0,   197,   198,   199,     0,
     200,   201,   202,   203,   204,   205,   206,   207,   208,   209,
     210,   211,   212,   213,   214,   215,   216,   217,   218,   219,
     220,   221,   222,   223,   224,   225,   226,   227,   228,   229,
     230,   231,   232,   233,   234,   235,   236,   237,   238,   239,
     240,   241,   242,   243,   244,   245,   246,   247,     0,   248,
       0,   249,   533,   251,   252,   253,   254,   255,     1,     2,
       0,     0,     0,     3,     0,     0,     0,     4,     0,     0,
       0,     0,     9,    10,     0,     0,     0,     0,     0,     0,
     419,   420,     0,    16,    17,    18,   493,    20,    21,    22,
     494,    24,   495,   496,   497,   498,   421,    30,   499,    32,
      33,    34,     0,    35,    36,    37,    38,   500,    40,    41,
      42,    43,    44,    45,    46,   501,     0,    48,   502,    50,
     503,   504,    53,   505,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,   506,   507,
      71,     0,    72,    73,    74,   508,     0,     0,    77,    78,
      79,     0,     0,   509,    81,     0,     0,     0,     0,    83,
     510,    85,   511,   512,    88,    89,     0,     0,     0,     0,
       0,     0,     0,     0,    94,    95,    96,    97,     0,    98,
       0,     0,     0,   513,   100,   101,   514,   515,     0,     0,
       0,     0,     0,     0,   104,   105,   106,     0,   108,   109,
     110,     0,     0,     0,     0,     0,     0,   116,   516,     0,
       0,     0,   118,   119,   120,   121,   517,   123,   124,   518,
       0,     0,     0,     0,     0,     0,     0,   519,   520,   131,
       0,     0,     0,   132,     0,   133,   521,     0,     0,     0,
       0,   138,     0,   139,     0,   140,   141,   142,   143,   522,
     145,   146,   147,   148,   149,   150,     0,     0,   151,   152,
       0,     0,   422,   155,   156,   157,   158,   159,   160,   161,
     162,   523,   164,   524,   166,   525,   526,   169,   170,   171,
     172,   173,   174,   527,   176,   528,   529,   530,   180,   181,
     531,   532,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,     0,   197,   198,   199,     0,
     200,   201,   202,   203,   204,   205,   206,   207,   208,   209,
     210,   211,   212,   213,   214,   215,   216,   217,   218,   219,
     220,   221,   222,   223,   224,   225,   226,   227,   228,   229,
     230,   231,   232,   233,   234,   235,   236,   237,   238,   239,
     240,   241,   242,   243,   244,   245,   246,     0,     0,     0,
       0,   249,   533,   251,   252,   253,   254,   255,     1,     2,
       0,     0,     0,     3,     0,     0,     0,     4,     0,     0,
       0,     0,     9,    10,     0,     0,     0,     0,     0,     0,
     419,   420,     0,    16,    17,    18,   541,    20,    21,    22,
     494,   542,   543,   496,   497,   498,   421,    30,   499,    32,
     544,    34,     0,    35,    36,    37,    38,   545,    40,   546,
     547,    43,    44,    45,    46,   501,     0,    48,   548,    50,
     503,   504,    53,   505,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,   506,   507,
      71,     0,    72,    73,    74,   549,     0,     0,    77,    78,
      79,     0,     0,   509,    81,     0,     0,     0,     0,    83,
     510,    85,   511,   512,    88,    89,     0,     0,     0,     0,
       0,     0,     0,     0,    94,    95,    96,    97,     0,    98,
       0,     0,     0,   513,   100,   101,   514,   515,     0,     0,
       0,     0,     0,     0,   104,   105,   106,     0,   108,   109,
     110,     0,     0,     0,     0,     0,     0,   116,   516,     0,
       0,     0,   118,   119,   120,   121,   517,   123,   124,   518,
       0,     0,     0,     0,     0,     0,     0,   519,   520,   131,
       0,     0,     0,   132,     0,   133,   521,     0,     0,     0,
       0,   138,     0,   139,     0,   140,   141,   142,   143,   522,
     145,   146,   147,   148,   149,   150,     0,     0,   151,   152,
       0,     0,   422,   155,   156,   157,   158,   159,   160,   161,
     162,   523,   550,   524,   166,   525,   526,   169,   170,   171,
     172,   173,   174,   527,   176,   528,   529,   530,   180,   181,
     531,   532,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,     0,   197,   198,   199,     0,
     200,   201,   202,   203,   204,   205,   206,   207,   208,   209,
     210,   211,   212,   213,   214,   215,   216,   217,   218,   219,
     220,   221,   222,   223,   224,   225,   226,   227,   228,   229,
     230,   231,   232,   233,   234,   235,   236,   237,   238,   239,
     240,   241,   242,   243,   244,   245,   246,     0,     0,     3,
       0,   249,   533,   251,   252,   253,   254,   255,     9,    10,
       0,     0,     0,     0,     0,     0,   419,   420,     0,    16,
      17,    18,   493,    20,    21,    22,   494,    24,   495,   496,
    1151,   498,   421,    30,   499,    32,    33,    34,     0,    35,
      36,    37,    38,   500,    40,    41,    42,    43,    44,    45,
      46,   501,     0,    48,   502,    50,   503,   504,    53,   505,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,   506,   507,    71,     0,    72,    73,
      74,   508,     0,     0,    77,    78,    79,     0,     0,   509,
      81,     0,     0,     0,     0,    83,   510,    85,   511,   512,
      88,    89,     0,     0,     0,     0,     0,     0,     0,     0,
      94,    95,    96,    97,     0,    98,     0,     0,     0,   513,
     100,   101,   514,   515,     0,     0,     0,     0,     0,     0,
     104,   105,   106,     0,   108,   109,  1152,     0,     0,     0,
    1153,     0,     0,   116,   516,     0,     0,     0,   118,   119,
     120,   121,   517,   123,   124,   518,     0,     0,     0,     0,
       0,     0,     0,   519,   520,   131,     0,     0,     0,   132,
    1585,   133,   521,     0,     0,     0,     0,  1586,     0,   139,
       0,   140,   141,   142,   143,   522,   145,   146,   147,   148,
     149,   150,     0,     0,  1154,   152,     0,     0,   422,   155,
     156,   157,   158,   159,   160,   161,   162,   523,   164,   524,
     166,   525,   526,   169,   170,   171,   172,   173,   174,   527,
     176,   528,   529,   530,   180,   181,   531,   532,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,     0,   197,   198,   199,     0,   200,   201,   202,   203,
     204,   205,   206,   207,   208,   209,   210,   211,   212,   213,
     214,   215,   216,   217,   218,   219,   220,   221,   222,   223,
     224,   225,   226,   227,   228,   229,   230,   231,   232,   233,
     234,   235,   236,   237,   238,   239,   240,   241,   242,   243,
     244,   245,   246,     0,     0,     0,     0,   249,   533,   251,
    1155,  1156,  1157,  1158,     3,     0,   577,   578,     0,     0,
       0,     0,     0,     9,    10,     0,     0,     0,     0,     0,
       0,   419,   420,     0,    16,    17,    18,   541,    20,    21,
      22,   494,   542,   543,   496,   497,   498,   421,    30,   499,
      32,   544,    34,     0,    35,    36,    37,    38,   545,    40,
     546,   547,    43,    44,    45,    46,   501,     0,    48,   548,
      50,   503,   504,    53,   505,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,   506,
     507,    71,     0,    72,    73,    74,   549,     0,     0,    77,
      78,    79,     0,     0,   509,    81,     0,     0,     0,     0,
      83,   510,    85,   511,   512,    88,    89,     0,     0,     0,
       0,     0,     0,     0,     0,    94,    95,    96,    97,     0,
      98,     0,     0,     0,   513,   100,   101,   514,   515,     0,
       0,     0,     0,     0,     0,   104,   105,   106,     0,   108,
     109,   110,     0,     0,     0,     0,     0,     0,   116,   516,
       0,     0,     0,   118,   119,   120,   121,   517,   123,   124,
     518,     0,     0,     0,     0,     0,     0,     0,   519,   520,
     131,     0,     0,     0,   132,     0,   133,   521,     0,     0,
       0,     0,     0,     0,   139,     0,   140,   141,   142,   143,
     522,   145,   146,   147,   148,   149,   150,     0,     0,   151,
     152,     0,     0,   422,   155,   156,   157,   158,   159,   160,
     161,   162,   523,   550,   524,   166,   525,   526,   169,   170,
     171,   172,   173,   174,   527,   176,   528,   529,   530,   180,
     181,   531,   532,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,     0,   197,   198,   199,
       0,   200,   201,   202,   203,   204,   205,   206,   207,   208,
     209,   210,   211,   212,   213,   214,   215,   216,   217,   218,
     219,   220,   221,   222,   223,   224,   225,   226,   227,   228,
     229,   230,   231,   232,   233,   234,   235,   236,   237,   238,
     239,   240,   241,   242,   243,   244,   245,   246,     0,     0,
       3,     0,   249,   533,   251,   252,   253,   254,   255,     9,
      10,     0,     0,     0,     0,     0,     0,   419,   420,     0,
      16,    17,    18,   541,    20,    21,    22,   494,   542,   543,
     496,   497,   498,   421,    30,   499,    32,   544,    34,     0,
      35,    36,    37,    38,   545,    40,   546,   547,    43,    44,
      45,    46,   501,     0,    48,   548,    50,   503,   504,    53,
     505,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,   506,   507,    71,     0,    72,
      73,    74,   549,     0,     0,    77,    78,    79,     0,     0,
     509,    81,     0,     0,     0,     0,    83,   510,    85,   511,
     512,    88,    89,     0,     0,     0,     0,     0,     0,     0,
       0,    94,    95,    96,    97,     0,    98,     0,     0,     0,
     513,   100,   101,   514,   515,     0,     0,     0,     0,     0,
       0,   104,   105,   106,     0,   108,   109,   110,     0,     0,
       0,     0,     0,     0,   116,   516,     0,     0,     0,   118,
     119,   120,   121,   517,   123,   124,   518,     0,     0,     0,
       0,     0,     0,     0,   519,   520,   131,     0,     0,     0,
     132,   765,   133,   521,     0,     0,     0,     0,   766,     0,
     139,     0,   140,   141,   142,   143,   522,   145,   146,   147,
     148,   149,   150,     0,     0,   151,   152,     0,     0,   422,
     155,   156,   157,   158,   159,   160,   161,   162,   523,   550,
     524,   166,   525,   526,   169,   170,   171,   172,   173,   174,
     527,   176,   528,   529,   530,   180,   181,   531,   532,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,     0,   197,   198,   199,     0,   200,   201,   202,
     203,   204,   205,   206,   207,   208,   209,   210,   211,   212,
     213,   214,   215,   216,   217,   218,   219,   220,   221,   222,
     223,   224,   225,   226,   227,   228,   229,   230,   231,   232,
     233,   234,   235,   236,   237,   238,   239,   240,   241,   242,
     243,   244,   245,   246,     0,     0,     3,     0,   249,   533,
     251,   252,   253,   254,   255,     9,    10,     0,     0,     0,
     791,     0,     0,   419,   420,     0,    16,    17,    18,   541,
      20,    21,    22,   494,   542,   543,   496,   497,   498,   421,
      30,   499,    32,   544,    34,     0,    35,    36,    37,    38,
     545,    40,   546,   547,    43,    44,    45,    46,   501,     0,
      48,   548,    50,   503,   504,    53,   505,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,   506,   507,    71,     0,    72,    73,    74,   549,     0,
       0,    77,    78,    79,     0,     0,   509,    81,     0,     0,
       0,     0,    83,   510,    85,   511,   512,    88,    89,     0,
       0,     0,     0,     0,     0,     0,     0,    94,    95,    96,
      97,     0,    98,     0,     0,     0,   513,   100,   101,   514,
     515,     0,     0,     0,     0,     0,     0,   104,   105,   106,
       0,   108,   109,   110,     0,     0,     0,     0,     0,     0,
     116,   516,     0,     0,     0,   118,   119,   120,   121,   517,
     123,   124,   518,     0,     0,     0,     0,     0,     0,     0,
     519,   520,   131,     0,     0,     0,   132,   792,   133,   521,
       0,     0,     0,     0,     0,     0,   139,     0,   140,   141,
     142,   143,   522,   145,   146,   147,   148,   149,   150,     0,
       0,   151,   152,     0,     0,   422,   155,   156,   157,   158,
     159,   160,   161,   162,   523,   550,   524,   166,   525,   526,
     169,   170,   171,   172,   173,   174,   527,   176,   528,   529,
     530,   180,   181,   531,   532,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,     0,   197,
     198,   199,     0,   200,   201,   202,   203,   204,   205,   206,
     207,   208,   209,   210,   211,   212,   213,   214,   215,   216,
     217,   218,   219,   220,   221,   222,   223,   224,   225,   226,
     227,   228,   229,   230,   231,   232,   233,   234,   235,   236,
     237,   238,   239,   240,   241,   242,   243,   244,   245,   246,
       0,     0,     3,     0,   249,   533,   251,   252,   253,   254,
     255,     9,    10,     0,     0,     0,     0,     0,     0,   419,
     420,     0,    16,    17,    18,   541,    20,    21,    22,   494,
     542,   543,   496,   497,   498,   421,    30,   499,    32,   544,
      34,     0,    35,    36,    37,    38,   545,    40,   546,   547,
      43,    44,    45,    46,   501,     0,    48,   548,    50,   503,
     504,    53,   505,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,   506,   507,    71,
       0,    72,    73,    74,   549,     0,     0,    77,    78,    79,
       0,     0,   509,    81,     0,     0,     0,     0,    83,   510,
      85,   511,   512,    88,    89,     0,     0,     0,     0,     0,
       0,     0,     0,    94,    95,    96,    97,     0,    98,     0,
       0,     0,   513,   100,   101,   514,   515,     0,     0,     0,
       0,     0,     0,   104,   105,   106,     0,   108,   109,   110,
       0,     0,     0,     0,     0,     0,   116,   516,     0,     0,
       0,   118,   119,   120,   121,   517,   123,   124,   518,     0,
       0,     0,     0,     0,     0,     0,   519,   520,   131,     0,
       0,     0,   132,   809,   133,   521,     0,     0,     0,     0,
     810,     0,   139,     0,   140,   141,   142,   143,   522,   145,
     146,   147,   148,   149,   150,     0,     0,   151,   152,     0,
       0,   422,   155,   156,   157,   158,   159,   160,   161,   162,
     523,   550,   524,   166,   525,   526,   169,   170,   171,   172,
     173,   174,   527,   176,   528,   529,   530,   180,   181,   531,
     532,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,     0,   197,   198,   199,     0,   200,
     201,   202,   203,   204,   205,   206,   207,   208,   209,   210,
     211,   212,   213,   214,   215,   216,   217,   218,   219,   220,
     221,   222,   223,   224,   225,   226,   227,   228,   229,   230,
     231,   232,   233,   234,   235,   236,   237,   238,   239,   240,
     241,   242,   243,   244,   245,   246,     0,     0,     3,     0,
     249,   533,   251,   252,   253,   254,   255,     9,    10,     0,
       0,     0,     0,     0,     0,   419,   420,     0,    16,    17,
      18,   493,    20,    21,    22,   494,    24,   495,   496,  1151,
     498,   421,    30,   499,    32,    33,    34,     0,    35,    36,
      37,    38,   500,    40,    41,    42,    43,    44,    45,    46,
     501,     0,    48,   502,    50,   503,   504,    53,   505,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,   506,   507,    71,     0,    72,    73,    74,
     508,     0,     0,    77,    78,    79,     0,     0,   509,    81,
       0,     0,     0,     0,    83,   510,    85,   511,   512,    88,
      89,  1494,     0,     0,     0,     0,     0,     0,     0,    94,
      95,    96,    97,     0,    98,     0,     0,     0,   513,   100,
     101,   514,   515,     0,     0,     0,     0,     0,     0,   104,
     105,   106,     0,   108,   109,  1152,     0,     0,     0,  1153,
       0,     0,   116,   516,     0,     0,     0,   118,   119,   120,
     121,   517,   123,   124,   518,     0,     0,     0,     0,     0,
       0,     0,   519,   520,   131,     0,     0,     0,   132,     0,
     133,   521,     0,     0,     0,     0,     0,     0,   139,     0,
     140,   141,   142,   143,   522,   145,   146,   147,   148,   149,
     150,     0,     0,  1154,   152,     0,     0,   422,   155,   156,
     157,   158,   159,   160,   161,   162,   523,   164,   524,   166,
     525,   526,   169,   170,   171,   172,   173,   174,   527,   176,
     528,   529,   530,   180,   181,   531,   532,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
       0,   197,   198,   199,     0,   200,   201,   202,   203,   204,
     205,   206,   207,   208,   209,   210,   211,   212,   213,   214,
     215,   216,   217,   218,   219,   220,   221,   222,   223,   224,
     225,   226,   227,   228,   229,   230,   231,   232,   233,   234,
     235,   236,   237,   238,   239,   240,   241,   242,   243,   244,
     245,   246,     0,     0,     3,     0,   249,   533,   251,  1155,
    1156,  1157,  1158,     9,    10,     0,     0,     0,     0,     0,
       0,   419,   420,     0,    16,    17,    18,   493,    20,    21,
      22,   494,    24,   495,   496,  1151,   498,   421,    30,   499,
      32,    33,    34,     0,    35,    36,    37,    38,   500,    40,
      41,    42,    43,    44,    45,    46,   501,     0,    48,   502,
      50,   503,   504,    53,   505,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,   506,
     507,    71,     0,    72,    73,    74,   508,     0,     0,    77,
      78,    79,     0,     0,   509,    81,     0,     0,     0,     0,
      83,   510,    85,   511,   512,    88,    89,  1638,     0,     0,
       0,     0,     0,     0,     0,    94,    95,    96,    97,     0,
      98,     0,     0,     0,   513,   100,   101,   514,   515,     0,
       0,     0,     0,     0,     0,   104,   105,   106,     0,   108,
     109,  1152,     0,     0,     0,  1153,     0,     0,   116,   516,
       0,     0,     0,   118,   119,   120,   121,   517,   123,   124,
     518,     0,     0,     0,     0,     0,     0,     0,   519,   520,
     131,     0,     0,     0,   132,     0,   133,   521,     0,     0,
       0,     0,     0,     0,   139,     0,   140,   141,   142,   143,
     522,   145,   146,   147,   148,   149,   150,     0,     0,  1154,
     152,     0,     0,   422,   155,   156,   157,   158,   159,   160,
     161,   162,   523,   164,   524,   166,   525,   526,   169,   170,
     171,   172,   173,   174,   527,   176,   528,   529,   530,   180,
     181,   531,   532,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,     0,   197,   198,   199,
       0,   200,   201,   202,   203,   204,   205,   206,   207,   208,
     209,   210,   211,   212,   213,   214,   215,   216,   217,   218,
     219,   220,   221,   222,   223,   224,   225,   226,   227,   228,
     229,   230,   231,   232,   233,   234,   235,   236,   237,   238,
     239,   240,   241,   242,   243,   244,   245,   246,     0,     0,
       3,     0,   249,   533,   251,  1155,  1156,  1157,  1158,     9,
      10,     0,     0,     0,     0,     0,     0,   419,   420,     0,
      16,    17,    18,   493,    20,    21,    22,   494,    24,   495,
     496,  1151,   498,   421,    30,   499,    32,    33,    34,     0,
      35,    36,    37,    38,   500,    40,    41,    42,    43,    44,
      45,    46,   501,     0,    48,   502,    50,   503,   504,    53,
     505,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,   506,   507,    71,     0,    72,
      73,    74,   508,     0,     0,    77,    78,    79,     0,     0,
     509,    81,     0,     0,     0,     0,    83,   510,    85,   511,
     512,    88,    89,  1642,     0,     0,     0,     0,     0,     0,
       0,    94,    95,    96,    97,     0,    98,     0,     0,     0,
     513,   100,   101,   514,   515,     0,     0,     0,     0,     0,
       0,   104,   105,   106,     0,   108,   109,  1152,     0,     0,
       0,  1153,     0,     0,   116,   516,     0,     0,     0,   118,
     119,   120,   121,   517,   123,   124,   518,     0,     0,     0,
       0,     0,     0,     0,   519,   520,   131,     0,     0,     0,
     132,     0,   133,   521,     0,     0,     0,     0,     0,     0,
     139,     0,   140,   141,   142,   143,   522,   145,   146,   147,
     148,   149,   150,     0,     0,  1154,   152,     0,     0,   422,
     155,   156,   157,   158,   159,   160,   161,   162,   523,   164,
     524,   166,   525,   526,   169,   170,   171,   172,   173,   174,
     527,   176,   528,   529,   530,   180,   181,   531,   532,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,     0,   197,   198,   199,     0,   200,   201,   202,
     203,   204,   205,   206,   207,   208,   209,   210,   211,   212,
     213,   214,   215,   216,   217,   218,   219,   220,   221,   222,
     223,   224,   225,   226,   227,   228,   229,   230,   231,   232,
     233,   234,   235,   236,   237,   238,   239,   240,   241,   242,
     243,   244,   245,   246,     0,     0,     3,     0,   249,   533,
     251,  1155,  1156,  1157,  1158,     9,    10,     0,     0,     0,
       0,     0,     0,   419,   420,     0,    16,    17,    18,   493,
      20,    21,    22,   494,    24,   495,   496,  1151,   498,   421,
      30,   499,    32,    33,    34,     0,    35,    36,    37,    38,
     500,    40,    41,    42,    43,    44,    45,    46,   501,     0,
      48,   502,    50,   503,   504,    53,   505,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,   506,   507,    71,     0,    72,    73,    74,   508,     0,
       0,    77,    78,    79,     0,     0,   509,    81,     0,     0,
       0,     0,    83,   510,    85,   511,   512,    88,    89,     0,
       0,     0,     0,     0,     0,     0,     0,    94,    95,    96,
      97,     0,    98,     0,     0,     0,   513,   100,   101,   514,
     515,     0,     0,     0,     0,     0,     0,   104,   105,   106,
       0,   108,   109,  1152,     0,     0,     0,  1153,     0,     0,
     116,   516,     0,     0,     0,   118,   119,   120,   121,   517,
     123,   124,   518,     0,     0,     0,     0,     0,     0,     0,
     519,   520,   131,     0,     0,     0,   132,     0,   133,   521,
       0,     0,     0,     0,     0,     0,   139,     0,   140,   141,
     142,   143,   522,   145,   146,   147,   148,   149,   150,     0,
       0,  1154,   152,     0,     0,   422,   155,   156,   157,   158,
     159,   160,   161,   162,   523,   164,   524,   166,   525,   526,
     169,   170,   171,   172,   173,   174,   527,   176,   528,   529,
     530,   180,   181,   531,   532,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,     0,   197,
     198,   199,     0,   200,   201,   202,   203,   204,   205,   206,
     207,   208,   209,   210,   211,   212,   213,   214,   215,   216,
     217,   218,   219,   220,   221,   222,   223,   224,   225,   226,
     227,   228,   229,   230,   231,   232,   233,   234,   235,   236,
     237,   238,   239,   240,   241,   242,   243,   244,   245,   246,
       0,     0,     3,     0,   249,   533,   251,  1155,  1156,  1157,
    1158,     9,    10,     0,     0,     0,     0,     0,     0,   419,
     420,     0,    16,    17,    18,   493,    20,    21,    22,   494,
      24,   495,   496,  1151,   498,   421,    30,   499,    32,    33,
      34,     0,    35,    36,    37,    38,   500,    40,    41,    42,
      43,    44,    45,    46,   501,     0,    48,   502,    50,   503,
     504,    53,   505,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,   506,   507,    71,
       0,    72,    73,    74,   508,     0,     0,    77,    78,    79,
       0,     0,   509,    81,     0,     0,     0,     0,    83,   510,
      85,   511,   512,    88,    89,     0,     0,     0,     0,     0,
       0,     0,     0,    94,    95,    96,    97,     0,    98,     0,
       0,     0,   513,   100,   101,   514,   515,     0,     0,     0,
       0,     0,     0,   104,   105,   106,     0,   108,   109,  1152,
       0,     0,     0,  1153,     0,     0,   116,   516,     0,     0,
       0,   118,   119,   120,   121,   517,   123,   124,   518,     0,
       0,     0,     0,     0,     0,     0,   519,   520,   131,     0,
       0,     0,   132,     0,   133,   521,     0,     0,     0,     0,
       0,     0,   139,     0,   140,   141,   142,   143,   522,   145,
     146,   147,   148,   149,   150,     0,     0,   151,   152,     0,
       0,   422,   155,   156,   157,   158,   159,   160,   161,   162,
     523,   164,   524,   166,   525,   526,   169,   170,   171,   172,
     173,   174,   527,   176,   528,   529,   530,   180,   181,   531,
     532,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,     0,   197,   198,   199,     0,   200,
     201,   202,   203,   204,   205,   206,   207,   208,   209,   210,
     211,   212,   213,   214,   215,   216,   217,   218,   219,   220,
     221,   222,   223,   224,   225,   226,   227,   228,   229,   230,
     231,   232,   233,   234,   235,   236,   237,   238,   239,   240,
     241,   242,   243,   244,   245,   246,     0,     0,     3,     0,
     249,   533,   251,  1155,  1156,  1157,  1158,     9,    10,     0,
       0,     0,     0,     0,     0,   419,   420,     0,    16,    17,
      18,   541,    20,    21,    22,   494,   542,   543,   496,   497,
     498,   421,    30,   499,    32,   544,    34,     0,    35,    36,
      37,    38,   545,    40,   546,   547,    43,    44,    45,    46,
     501,     0,    48,   548,    50,   503,   504,    53,   505,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,   506,   507,    71,     0,    72,    73,    74,
     549,     0,     0,    77,    78,    79,     0,     0,   509,    81,
       0,     0,     0,     0,    83,   510,    85,   511,   512,    88,
      89,  1243,     0,     0,     0,     0,     0,     0,     0,    94,
      95,    96,    97,     0,    98,     0,     0,     0,   513,   100,
     101,   514,   515,     0,     0,     0,     0,     0,     0,   104,
     105,   106,     0,   108,   109,   110,     0,     0,     0,     0,
       0,     0,   116,   516,     0,     0,     0,   118,   119,   120,
     121,   517,   123,   124,   518,     0,     0,     0,     0,     0,
       0,     0,   519,   520,   131,     0,     0,     0,   132,     0,
     133,   521,     0,     0,     0,     0,     0,     0,   139,     0,
     140,   141,   142,   143,   522,   145,   146,   147,   148,   149,
     150,     0,     0,   151,   152,     0,     0,   422,   155,   156,
     157,   158,   159,   160,   161,   162,   523,   550,   524,   166,
     525,   526,   169,   170,   171,   172,   173,   174,   527,   176,
     528,   529,   530,   180,   181,   531,   532,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
       0,   197,   198,   199,     0,   200,   201,   202,   203,   204,
     205,   206,   207,   208,   209,   210,   211,   212,   213,   214,
     215,   216,   217,   218,   219,   220,   221,   222,   223,   224,
     225,   226,   227,   228,   229,   230,   231,   232,   233,   234,
     235,   236,   237,   238,   239,   240,   241,   242,   243,   244,
     245,   246,     0,     0,     3,     0,   249,   533,   251,   252,
     253,   254,   255,     9,    10,     0,     0,     0,     0,     0,
       0,   419,   420,     0,    16,    17,    18,   541,    20,    21,
      22,   494,   542,   543,   496,   497,   498,   421,    30,   499,
      32,   544,    34,     0,    35,    36,    37,    38,   545,    40,
     546,   547,    43,    44,    45,    46,   501,     0,    48,   548,
      50,   503,   504,    53,   505,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,   506,
     507,    71,     0,    72,    73,    74,   549,     0,     0,    77,
      78,    79,     0,     0,   509,    81,     0,     0,     0,     0,
      83,   510,    85,   511,   512,    88,    89,     0,     0,     0,
       0,     0,     0,     0,     0,    94,    95,    96,    97,     0,
      98,     0,     0,     0,   513,   100,   101,   514,   515,     0,
       0,     0,     0,     0,     0,   104,   105,   106,     0,   108,
     109,   110,     0,     0,     0,     0,     0,     0,   116,   516,
       0,     0,     0,   118,   119,   120,   121,   517,   123,   124,
     518,     0,     0,     0,     0,     0,     0,     0,   519,   520,
     131,     0,     0,     0,   132,     0,   133,   521,     0,     0,
       0,     0,     0,     0,   139,     0,   140,   141,   142,   143,
     522,   145,   146,   147,   148,   149,   150,     0,     0,   151,
     152,     0,     0,   422,   155,   156,   157,   158,   159,   160,
     161,   162,   523,   550,   524,   166,   525,   526,   169,   170,
     171,   172,   173,   174,   527,   176,   528,   529,   530,   180,
     181,   531,   532,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,     0,   197,   198,   199,
       0,   200,   201,   202,   203,   204,   205,   206,   207,   208,
     209,   210,   211,   212,   213,   214,   215,   216,   217,   218,
     219,   220,   221,   222,   223,   224,   225,   226,   227,   228,
     229,   230,   231,   232,   233,   234,   235,   236,   237,   238,
     239,   240,   241,   242,   243,   244,   245,   246,     0,     0,
       0,     0,   249,   533,   251,   252,   253,   254,   255,     5,
       6,     7,     8,     9,    10,     0,     0,     0,    13,     0,
       0,   419,   420,     0,    16,    17,    18,   842,    20,    21,
      22,    23,     0,   843,   496,    27,     0,   421,    30,   499,
      32,     0,    34,     0,    35,    36,    37,    38,   844,    40,
       0,     0,    43,    44,    45,    46,   501,     0,    48,   845,
      50,     0,     0,    53,   505,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,   506,
     507,    71,     0,    72,    73,    74,   846,     0,     0,    77,
      78,    79,     0,     0,   509,    81,     0,     0,     0,    82,
      83,   510,    85,   511,   512,    88,    89,   563,    91,     0,
       0,     0,     0,    93,     0,    94,    95,    96,    97,     0,
      98,     0,     0,     0,   513,   100,   101,   514,   515,     0,
       0,     0,     0,     0,     0,   104,   105,   106,   107,   108,
     109,     0,   564,   112,     0,   113,   114,     0,   116,   117,
       0,     0,     0,   118,   119,   120,   121,   122,   123,   124,
     518,     0,   126,     0,     0,     0,     0,     0,   519,   520,
     131,     0,     0,     0,   132,     0,   133,   521,     0,     0,
       0,     0,     0,     0,   139,     0,   140,   141,   142,   143,
     144,   145,   146,   147,   148,   149,   150,     0,     0,     0,
     152,   153,     0,   422,   155,   156,   157,   158,   159,   160,
     161,   162,   523,     0,   524,   166,   525,   526,   169,   170,
     171,   172,   173,   174,   527,   176,   528,   529,   530,   180,
     181,     0,   532,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,     0,   197,   198,   199,
       0,   200,   201,   202,   203,   204,   205,   206,   207,   208,
     209,   210,   211,   212,   213,   214,   215,   216,   217,   218,
     219,   220,   221,   222,   223,   224,   225,   226,   227,   228,
     229,   230,   231,   232,   233,   234,   235,   236,   237,   238,
     239,   240,   241,   242,   243,   244,   245,   246,   247,     0,
     248,     9,    10,   533,   251,     0,     0,     0,     0,   419,
     420,     0,    16,    17,    18,     0,    20,    21,    22,   494,
       0,     0,   496,   497,     0,   421,    30,   499,    32,     0,
      34,     0,    35,    36,    37,    38,     0,    40,     0,     0,
      43,    44,    45,    46,   501,     0,    48,     0,    50,     0,
       0,    53,   505,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,   506,   507,    71,
       0,    72,    73,    74,     0,     0,     0,    77,    78,    79,
       0,     0,   509,    81,     0,     0,     0,     0,    83,   510,
      85,   511,   512,    88,    89,     0,     0,     0,     0,     0,
       0,     0,     0,    94,    95,    96,    97,     0,    98,     0,
       0,     0,   513,   100,   101,   514,   515,     0,     0,     0,
       0,     0,     0,   104,   105,   106,     0,   108,   109,     0,
       0,     0,     0,     0,     0,     0,   116,   516,     0,     0,
       0,   118,   119,   120,   121,   517,   123,   124,   518,     0,
       0,     0,     0,     0,     0,     0,   519,   520,   131,     0,
       0,     0,   132,     0,   133,   521,     0,     0,     0,     0,
       0,     0,   139,     0,   140,   141,   142,   143,   522,   145,
     146,   147,   148,   149,   150,     0,     0,     0,   152,     0,
       0,   422,   155,   156,   157,   158,   159,   160,   161,   162,
     523,     0,   524,   166,   525,   526,   169,   170,   171,   172,
     173,   174,   527,   176,   528,   529,   530,   180,   181,     0,
     532,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,     0,   197,   198,   199,     0,   200,
     201,   202,   203,   204,   205,   206,   207,   208,   209,   210,
     211,   212,   213,   214,   215,   216,   217,   218,   219,   220,
     221,   222,   223,   224,   225,   226,   227,   228,   229,   230,
     231,   232,   233,   234,   235,   236,   237,   238,   239,   240,
     241,   242,   243,   244,   245,   246,     0,     0,     0,     0,
       0,   533,   251
  };

  const short
  xquery_parser::yycheck_[] =
  {
       0,   316,   183,   430,   431,     5,     6,     7,     8,     5,
       6,     7,    76,   695,   679,   316,     0,     5,     6,     7,
       8,     5,     6,     7,     8,    14,    90,   346,   770,  1284,
     785,   917,   829,     5,     6,     7,     8,   680,   681,   682,
     683,  1301,  1056,   798,   263,   623,   624,   866,  1040,   432,
     114,   590,   770,   112,   113,     1,   595,   812,   621,   622,
     623,   624,   126,    76,   128,  1137,  1365,  1366,     1,   621,
     622,   137,   295,   953,  1003,  1497,   264,     1,  1500,  1080,
     256,   257,    36,    39,   467,   468,   727,  1021,    44,    45,
    1412,   279,    27,    28,   119,    36,     1,    53,   930,   931,
     119,    30,  1314,     1,    60,     1,    62,    63,    31,    27,
      28,   111,    47,   286,    52,    71,     9,    10,     1,    56,
       8,   108,    57,   111,   225,   225,     1,   111,   148,    47,
       1,   658,     1,   660,   661,   108,   663,     1,   665,   111,
      75,    95,    98,    97,   969,    80,   673,   674,   119,    97,
     119,     1,     1,   225,   137,   227,    97,    75,   353,   145,
     186,   356,   108,     1,   217,    30,   114,     1,   187,   159,
       1,   930,   931,   137,   225,   108,   227,   112,   113,   114,
       0,     1,   111,  1607,   210,    45,   176,   151,   247,   248,
     176,  1023,   130,     1,   148,   113,   125,   119,  1030,    33,
     225,   254,   425,    63,     0,     1,   429,   148,   194,   157,
     969,   104,    30,   314,   314,   186,     1,   186,   201,   161,
     975,   150,     1,   143,     1,   154,    37,    38,   157,    40,
      41,   124,     1,   126,   148,     1,   295,   108,     1,   108,
       1,  1540,   314,    55,  1003,    57,   233,    58,   177,   114,
       1,  1137,  1138,   273,   569,  1510,   256,   257,   108,   108,
     260,   154,   235,   314,  1023,   158,   190,  1281,   569,   225,
     108,  1030,   256,   257,  1029,  1267,   260,   108,   451,  1076,
     217,   174,   307,   349,  1606,   190,  1028,   175,   108,   101,
     354,   214,   190,   357,   190,   295,   161,   875,   876,  1721,
     108,  1513,   195,   114,  1305,  1234,  1186,   190,  1026,  1243,
    1028,   295,   108,  1314,   955,   190,  1388,   254,   187,   273,
     213,   873,   874,   108,  1042,    95,   190,   187,  1153,   108,
     259,   108,   273,  1757,   557,     1,     1,   187,   187,   108,
       1,   354,   108,   161,   357,   108,  1768,   108,  1770,   187,
     161,   273,   287,   273,   157,   738,   187,   108,   108,   273,
     295,   317,   318,   319,   184,    33,   425,   313,   303,   287,
     429,   108,   436,   437,  1129,     1,   440,   295,   311,   187,
     444,    49,    95,   447,   448,   625,   913,     1,   184,   629,
      97,   455,    94,    45,  1153,    47,   122,   461,   462,   184,
     783,   460,   621,   622,    97,   469,   465,  1706,   187,   220,
     187,   108,   476,   477,   478,   479,   475,   157,   187,   185,
     484,   644,   185,   482,   483,   425,   187,  1259,   492,   429,
     618,   619,   491,     1,   610,     1,   187,    97,   188,   251,
     628,   425,   108,   108,   101,   429,   154,   108,   218,   219,
     261,   188,   264,     5,   266,   107,   456,     9,   458,   108,
       1,  1760,   169,   233,     1,   277,   278,   279,   456,   281,
     458,   283,   456,   537,   458,  1234,   169,   108,  1233,    97,
     480,   183,   108,  1743,   456,   485,   458,   184,   552,   108,
     917,   119,   480,   155,   108,   157,   480,   485,   557,   563,
    1259,   485,  1388,  1245,    77,   218,   219,   233,   480,    27,
      28,  1397,  1513,   485,   573,   574,   168,   157,   184,   184,
     233,   948,  1408,   184,  1283,   584,   590,   108,  1347,   186,
     148,   595,  1351,  1788,   598,     3,  1254,  1115,  1116,  1117,
     108,   190,   108,   602,   603,   252,   253,  1229,   255,     9,
     101,   108,  1115,  1116,  1117,   262,   187,   557,   184,   252,
     253,  1100,   255,   114,   564,   272,   159,   108,   187,    29,
     184,   108,   124,   557,   126,   237,   564,   284,    30,   272,
     564,   154,   134,   176,     9,    10,   108,     9,  1420,   131,
     654,   284,   564,   976,   977,   113,   256,   257,   981,    67,
    1668,   108,   154,   603,    72,   131,   187,    29,     9,    10,
     610,   132,    95,   108,  1369,   225,   184,    97,   184,   603,
     230,   621,   622,   623,   624,     5,   610,   616,  1011,     9,
     187,   603,  1624,   699,   700,     9,    10,   621,   622,   623,
     624,   864,   701,   184,   644,   138,   166,   184,   221,   222,
     143,   119,   108,   154,   873,   197,   123,   108,   108,   111,
     644,  1420,   182,   108,   124,   187,   126,   731,   891,   190,
     734,   197,   155,   125,   157,   135,   269,   157,  1746,   104,
     187,   138,   154,   276,   748,    45,   157,    47,   752,   753,
    1576,   879,   187,   286,   154,   883,   696,   154,   150,   124,
     696,   126,   124,   104,   126,   177,   118,   771,   696,   210,
    1137,  1138,   696,   135,  1511,   779,    95,    42,   782,  1516,
      80,    46,   786,   124,   696,   126,    51,  1469,  1742,   154,
     104,   187,   154,   158,  1399,   147,   187,   187,  1497,    97,
     154,  1500,   187,   154,   124,    70,   126,   107,   160,   174,
     124,   149,   126,   154,   134,   149,   114,   158,     1,    97,
       3,   820,   821,   822,   828,   824,   177,   165,    30,   148,
     195,   165,  1454,   174,   154,   775,  1354,    95,   169,   775,
     154,   138,   194,   123,   158,    54,   143,   775,   213,    95,
      59,   775,    95,    54,   195,   107,   210,    95,    59,   157,
     174,   157,   866,   775,  1636,   864,   806,   807,   168,    78,
     148,   202,   213,   123,  1733,  1734,   128,    95,   806,   807,
    1139,   195,   806,   807,    67,  1017,     1,  1019,    97,    72,
     148,   895,   138,   201,   806,   807,    97,   143,   138,   213,
     143,   905,   210,   168,   154,   143,   171,   157,   157,   111,
     119,  1648,   157,   269,   154,  1652,  1775,  1689,    33,  1691,
     276,   186,  1781,   125,   864,   143,   930,   931,   932,   933,
     286,  1063,   159,  1065,    49,    25,   119,  1636,    95,  1262,
     864,    97,  1105,   159,    34,   247,   248,   212,   150,   176,
     107,   891,   154,    96,   157,   157,   885,   101,   114,   182,
     176,    54,  1567,  1568,   107,   969,    59,   891,   270,   268,
     114,    61,   199,   196,   117,   177,   275,  1560,  1106,   983,
     101,   101,  1754,   199,  1112,   128,   966,  1609,  1109,   182,
    1689,   146,  1691,   114,   114,   101,   101,   157,   978,   263,
    1737,   263,   157,   196,  1008,   299,   191,   192,   114,   114,
     274,  1706,   274,  1017,   308,  1019,   155,  1021,   157,  1023,
     154,  1388,  1721,   157,   154,  1280,  1030,   157,   154,   154,
    1397,   157,   157,   277,   278,   279,   123,   702,  1361,   283,
     705,  1408,   119,   708,   221,   222,   223,  1370,   232,   157,
     234,   232,   123,   234,   123,  1754,   129,   130,   119,  1063,
     123,  1065,   129,   130,    90,   677,   678,   684,   685,  1768,
     123,  1770,  1677,  1678,   154,   686,   687,  1227,  1228,   154,
    1597,  1598,   123,  1338,   123,   123,   123,   216,   114,  1711,
     154,  1346,   157,   241,   119,   241,  1100,   242,  1261,   157,
     126,   314,   128,   154,     0,  1346,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,   190,   167,  1058,    93,
     100,  1125,   249,    50,   200,   102,   155,   103,   184,   157,
    1058,   193,   184,    30,  1058,   184,  1758,  1136,   184,    47,
     153,   119,   246,   133,   187,  1750,  1058,   154,     1,  1153,
     304,   141,   142,   119,   144,  1478,   143,   108,   154,   154,
     187,   143,   152,   187,   157,  1105,   156,   184,   158,   157,
     122,   157,   157,   163,   157,  1115,  1116,  1117,   157,    35,
      17,  1105,   154,   214,   157,  1114,   157,    30,   190,   179,
     190,  1115,  1116,  1117,     3,   231,   119,   245,   168,   284,
     284,   170,   170,   119,    71,    95,    95,    95,  1830,  1576,
    1832,   231,   150,   203,   204,   205,   206,   207,   208,    30,
    1842,   108,  1343,   187,   108,   161,   161,   128,   133,    95,
     246,  1486,   143,  1488,   119,   133,  1491,  1492,   184,  1243,
    1495,   184,   299,  1247,   108,   187,  1501,   187,    95,    97,
      45,    97,   184,   187,   187,  1259,   187,   187,   184,   154,
     154,   154,   154,   154,    95,   143,   190,    33,   154,  1273,
     237,   233,   184,   133,   143,    95,    30,   286,   227,  1283,
      95,   299,   154,   128,   119,   148,   119,   170,    95,   198,
     161,   133,   133,    97,   119,   154,   148,  1237,   101,   101,
     154,  1237,   184,   184,   184,   143,   122,    35,  1312,  1237,
     154,   190,    30,  1237,   119,    80,   245,   108,   107,   258,
     108,  1261,   285,   260,   145,  1237,   254,  1490,   157,   157,
     157,   157,  1272,  1337,   157,   157,  1272,  1261,   157,    30,
      30,   286,   267,  1347,  1272,    30,   287,  1351,  1272,   306,
     227,  1291,   145,   187,   190,  1291,   148,   187,   128,   108,
    1272,   148,   143,  1291,   101,   133,   198,  1291,     7,   187,
    1533,  1311,   184,   187,   187,  1379,   233,   187,  1318,  1291,
      97,   157,   190,  1311,  1639,   186,   237,  1311,  1643,   148,
    1318,   154,   107,   233,  1318,   157,   157,   119,   119,  1311,
      78,   187,   148,    79,   187,   187,  1318,   282,   187,   233,
    1350,   437,   286,   187,   440,   187,  1420,    30,   444,    30,
     187,   447,   448,   187,   108,   170,  1350,   233,   184,   455,
     127,   127,   271,   250,   148,   461,   462,   148,  1350,    97,
     184,   184,   184,   469,   184,   143,  1450,   154,   316,   184,
     476,   477,   478,   479,   250,    95,   187,   257,   484,  1622,
      30,    97,   187,    95,   170,   148,   492,   197,    54,    95,
    1633,  1634,  1727,    30,  1637,   265,   119,   108,   108,    95,
      95,  1644,   280,   183,   225,   197,   119,   197,   287,   198,
    1494,   119,   225,  1497,    94,   197,  1500,   119,   119,   293,
     199,   225,   287,   263,   737,   119,  1189,   970,  1512,    27,
      28,   537,  1767,  1517,   431,  1031,  1771,  1711,  1234,    37,
      38,   305,    40,    41,   260,   305,   552,   305,   111,    47,
     451,  1530,  1531,  1532,   260,   306,  1758,   563,   867,    57,
      58,  1134,   654,  1553,  1121,  1380,  1550,  1551,  1803,   306,
    1490,  1806,   788,   306,  1372,  1368,  1278,    75,  1365,  1273,
     552,  1281,    80,  1286,   656,   676,  1490,  1730,   657,   933,
     711,  1305,   598,   829,  1829,  1781,  1775,  1019,  1188,  1348,
    1337,  1347,  1408,   341,  1576,  1397,   459,  1138,  1573,   459,
    1147,  1401,  1002,  1533,   112,   113,   114,  1211,  1444,  1600,
    1398,    -1,   593,    -1,    -1,    -1,  1769,    -1,    -1,  1533,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1787,    -1,    -1,    -1,   654,    -1,
      -1,    -1,  1636,    -1,  1638,    -1,  1640,    -1,  1642,  1802,
      -1,    -1,    -1,   161,    -1,  1649,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1824,    -1,    -1,    -1,    -1,  1665,    -1,    -1,    -1,
    1833,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1622,    -1,    -1,  1689,    -1,  1691,  1687,    -1,
      -1,    -1,    -1,  1633,  1634,    -1,    -1,  1637,  1622,    -1,
      -1,    -1,   220,  1707,  1644,    -1,  1710,    -1,   734,  1633,
    1634,    -1,    -1,  1637,    -1,    -1,    -1,  1721,    -1,    -1,
    1644,    -1,   748,    -1,    -1,    -1,   752,   753,  1732,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   261,    -1,   771,    -1,    -1,    -1,    -1,
    1754,    -1,    -1,   779,    -1,    -1,   782,    -1,  1762,    -1,
     786,    -1,    -1,    -1,  1768,    -1,  1770,    -1,    -1,   287,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   295,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   303,    -1,  1791,    -1,    -1,
    1730,    -1,    -1,    -1,    -1,    -1,  1800,    -1,    -1,    -1,
      -1,    -1,   828,    -1,    -1,    -1,  1730,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1820,    -1,    -1,    -1,
      -1,  1825,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1769,
      -1,    -1,    -1,  1837,    -1,    -1,  1840,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1769,    -1,  1787,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1802,  1787,    -1,    -1,    -1,    -1,    -1,   895,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1802,   905,
      -1,    -1,    -1,    -1,  1824,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1833,    -1,    -1,    -1,    -1,    -1,    -1,
    1824,    -1,    -1,    -1,   930,   931,   932,   933,    -1,  1833,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   969,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   983,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   564,  1008,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1017,    -1,  1019,    -1,  1021,    -1,  1023,    -1,    -1,
      -1,    -1,    -1,    -1,  1030,   588,   589,    -1,   591,   592,
     593,   594,    -1,   596,    -1,    -1,    -1,    -1,   601,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   609,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1063,    -1,  1065,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   631,   632,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   641,    -1,
      -1,   644,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1125,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   707,    -1,    -1,  1153,   711,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   750,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   787,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1243,    -1,    -1,
      -1,  1247,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     813,    -1,    -1,  1259,   817,   818,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   826,    -1,    -1,    -1,  1273,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1283,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1312,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   891,    -1,
      -1,  1337,    -1,    -1,   897,   898,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1379,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   971,    -1,
      -1,    -1,    -1,    -1,  1420,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1006,  1450,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1025,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1041,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1494,  1052,
    1053,  1497,    -1,    -1,  1500,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1067,    -1,  1512,  1070,    -1,    -1,
      -1,  1517,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1087,  1088,  1089,    -1,  1091,  1092,
    1093,  1094,  1095,  1096,  1097,  1098,  1099,    -1,    -1,  1102,
    1103,    -1,    -1,    -1,  1550,  1551,    -1,    -1,  1111,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1121,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1176,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1636,    -1,  1638,    -1,  1640,    -1,  1642,    -1,    -1,    -1,
      -1,    -1,    -1,  1649,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1689,    -1,  1691,  1249,    -1,  1251,    -1,
    1253,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1261,    -1,
      -1,  1707,    -1,    -1,  1710,    -1,    -1,  1270,    -1,    -1,
      -1,    -1,    -1,  1276,  1277,  1721,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1732,    -1,    -1,    -1,
      -1,    -1,  1295,    -1,    -1,  1298,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1754,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1762,    -1,    -1,    -1,
      -1,    -1,  1768,    -1,  1770,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1791,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1800,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1368,    -1,    -1,    -1,  1372,
    1373,    -1,    -1,    -1,  1820,  1378,    -1,    -1,    -1,  1825,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1837,    -1,    -1,  1840,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1463,    -1,    -1,    -1,  1467,    -1,    -1,    -1,  1471,    -1,
    1473,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1481,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1489,  1490,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1509,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1521,  1522,
    1523,  1524,    -1,  1526,  1527,    -1,  1529,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1544,    -1,    -1,    -1,    -1,  1549,    -1,    -1,    -1,
      -1,    -1,  1555,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1615,    -1,  1617,    -1,    -1,  1620,    -1,    -1,
    1623,    -1,    -1,    -1,    -1,  1628,  1629,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1637,    -1,    -1,    -1,  1641,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1658,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1669,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1714,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1726,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1769,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     0,
       1,    -1,    -1,    -1,    -1,  1798,    -1,    -1,    -1,  1802,
      11,    12,    -1,    -1,  1807,    16,    -1,    -1,    -1,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    -1,    30,
    1823,    -1,    33,    34,  1827,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    -1,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    -1,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    -1,    95,    96,    97,    98,    99,    -1,
     101,   102,   103,    -1,    -1,   106,   107,    -1,    -1,    -1,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,    -1,    -1,    -1,   125,    -1,   127,   128,   129,   130,
      -1,   132,    -1,    -1,    -1,   136,   137,   138,   139,   140,
      -1,    -1,    -1,    -1,    -1,    -1,   147,   148,   149,   150,
     151,   152,   153,   154,   155,    -1,   157,   158,   159,   160,
     161,    -1,    -1,    -1,   165,   166,   167,   168,   169,   170,
     171,   172,    -1,   174,    -1,   176,   177,    -1,    -1,   180,
     181,   182,    -1,    -1,    -1,   186,    -1,   188,   189,   190,
     191,   192,    -1,   194,    -1,   196,    -1,   198,   199,   200,
     201,   202,   203,   204,   205,   206,   207,   208,    -1,    -1,
     211,   212,   213,    -1,   215,   216,   217,   218,   219,   220,
     221,   222,   223,   224,   225,   226,   227,   228,   229,   230,
     231,   232,   233,   234,   235,   236,   237,   238,   239,   240,
     241,   242,   243,   244,   245,   246,   247,   248,   249,   250,
     251,   252,   253,   254,   255,   256,   257,    -1,   259,   260,
     261,    -1,   263,   264,   265,   266,   267,   268,   269,   270,
     271,   272,   273,   274,   275,   276,   277,   278,   279,   280,
     281,   282,   283,   284,   285,   286,   287,   288,   289,   290,
     291,   292,   293,   294,   295,   296,   297,   298,   299,   300,
     301,   302,   303,   304,   305,   306,   307,   308,   309,   310,
      -1,   312,    -1,   314,   315,   316,   317,   318,   319,   320,
       0,     1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    11,    12,    -1,    -1,    -1,    16,    -1,    -1,    -1,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    -1,
      30,    -1,    -1,    33,    34,    -1,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    -1,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    -1,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    -1,    95,    96,    97,    98,    99,
      -1,   101,   102,   103,    -1,    -1,   106,   107,    -1,    -1,
      -1,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,    -1,    -1,    -1,   125,    -1,   127,   128,   129,
     130,    -1,   132,    -1,    -1,    -1,   136,   137,   138,   139,
     140,    -1,    -1,    -1,    -1,    -1,    -1,   147,   148,   149,
     150,   151,   152,   153,   154,   155,    -1,   157,   158,   159,
     160,   161,    -1,    -1,    -1,   165,   166,   167,   168,   169,
     170,   171,   172,    -1,   174,    -1,   176,   177,    -1,    -1,
     180,   181,   182,    -1,    -1,    -1,   186,    -1,   188,   189,
     190,   191,   192,    -1,   194,    -1,   196,    -1,   198,   199,
     200,   201,   202,   203,   204,   205,   206,   207,   208,    -1,
      -1,   211,   212,   213,    -1,   215,   216,   217,   218,   219,
     220,   221,   222,   223,   224,   225,   226,   227,   228,   229,
     230,   231,   232,   233,   234,   235,   236,   237,   238,   239,
     240,   241,   242,   243,   244,   245,   246,   247,   248,   249,
     250,   251,   252,   253,   254,   255,   256,   257,    -1,   259,
     260,   261,    -1,   263,   264,   265,   266,   267,   268,   269,
     270,   271,   272,   273,   274,   275,   276,   277,   278,   279,
     280,   281,   282,   283,   284,   285,   286,   287,   288,   289,
     290,   291,   292,   293,   294,   295,   296,   297,   298,   299,
     300,   301,   302,   303,   304,   305,   306,   307,   308,   309,
     310,     1,   312,    -1,   314,   315,   316,   317,   318,   319,
     320,    11,    12,    -1,    -1,    -1,    16,    -1,    -1,    -1,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    -1,
      30,    -1,    -1,    33,    34,    -1,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    -1,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    -1,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    -1,    95,    96,    97,    98,    99,
      -1,   101,   102,   103,    -1,    -1,   106,   107,    -1,    -1,
      -1,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,    -1,    -1,    -1,   125,    -1,   127,   128,   129,
     130,    -1,   132,    -1,    -1,    -1,   136,   137,   138,   139,
     140,    -1,    -1,    -1,    -1,    -1,    -1,   147,   148,   149,
     150,   151,   152,   153,   154,   155,    -1,   157,   158,   159,
     160,   161,    -1,    -1,    -1,   165,   166,   167,   168,   169,
     170,   171,   172,    -1,   174,    -1,   176,   177,    -1,    -1,
     180,   181,   182,    -1,    -1,    -1,   186,    -1,   188,   189,
     190,   191,   192,    -1,   194,    -1,   196,    -1,   198,   199,
     200,   201,   202,   203,   204,   205,   206,   207,   208,    -1,
      -1,   211,   212,   213,    -1,   215,   216,   217,   218,   219,
     220,   221,   222,   223,   224,   225,   226,   227,   228,   229,
     230,   231,   232,   233,   234,   235,   236,   237,   238,   239,
     240,   241,   242,   243,   244,   245,   246,   247,   248,   249,
     250,   251,   252,   253,   254,   255,   256,   257,    -1,   259,
     260,   261,    -1,   263,   264,   265,   266,   267,   268,   269,
     270,   271,   272,   273,   274,   275,   276,   277,   278,   279,
     280,   281,   282,   283,   284,   285,   286,   287,   288,   289,
     290,   291,   292,   293,   294,   295,   296,   297,   298,   299,
     300,   301,   302,   303,   304,   305,   306,   307,   308,   309,
     310,     1,   312,    -1,   314,   315,   316,   317,   318,   319,
     320,    11,    12,    -1,    -1,    -1,    16,    -1,    -1,    -1,
      20,    21,    22,    23,    24,    25,    26,    -1,    -1,    -1,
      30,    -1,    -1,    33,    34,    -1,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    -1,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    -1,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    -1,    95,    96,    97,    98,    99,
      -1,   101,   102,   103,    -1,    -1,   106,   107,    -1,    -1,
      -1,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,    -1,    -1,    -1,   125,    -1,   127,   128,   129,
     130,    -1,   132,    -1,    -1,    -1,   136,   137,   138,   139,
     140,    -1,    -1,    -1,    -1,    -1,    -1,   147,   148,   149,
     150,   151,   152,   153,   154,   155,    -1,   157,   158,   159,
     160,   161,    -1,    -1,    -1,   165,   166,   167,   168,   169,
     170,   171,   172,    -1,   174,    -1,   176,   177,    -1,    -1,
     180,   181,   182,    -1,    -1,    -1,   186,    -1,   188,   189,
      -1,   191,   192,    -1,   194,    -1,   196,    -1,   198,   199,
     200,   201,   202,   203,   204,   205,   206,   207,   208,    -1,
      -1,   211,   212,   213,    -1,   215,   216,   217,   218,   219,
     220,   221,   222,   223,   224,   225,   226,   227,   228,   229,
     230,   231,   232,   233,   234,   235,   236,   237,   238,   239,
     240,   241,   242,   243,   244,   245,   246,   247,   248,   249,
     250,   251,   252,   253,   254,   255,   256,   257,    -1,   259,
     260,   261,    -1,   263,   264,   265,   266,   267,   268,   269,
     270,   271,   272,   273,   274,   275,   276,   277,   278,   279,
     280,   281,   282,   283,   284,   285,   286,   287,   288,   289,
     290,   291,   292,   293,   294,   295,   296,   297,   298,   299,
     300,   301,   302,   303,   304,   305,   306,   307,   308,   309,
     310,    -1,   312,    -1,   314,   315,   316,   317,   318,   319,
     320,     0,     1,    -1,     3,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    42,    -1,    -1,    -1,    46,    -1,    -1,
      -1,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,    -1,
      -1,    70,    -1,    72,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    93,    -1,    95,    96,    97,    -1,
      -1,   100,   101,   102,   103,    -1,    -1,    -1,   107,   108,
      -1,    -1,    -1,    -1,    -1,   114,    -1,    -1,   117,   118,
     119,    -1,    -1,   122,    -1,    -1,    -1,    -1,   127,   128,
      -1,    -1,    -1,    -1,   133,    -1,    -1,    -1,   137,    -1,
      -1,    -1,   141,   142,    -1,   144,    -1,   146,   147,    -1,
      -1,    -1,   151,   152,    -1,    -1,   155,   156,   157,   158,
     159,   160,    -1,    -1,   163,    -1,    -1,    -1,   167,   168,
      -1,    -1,   171,    -1,    -1,    -1,    -1,   176,    -1,    -1,
     179,    -1,    -1,    -1,    -1,   184,   185,   186,   187,   188,
      -1,   190,   191,   192,   193,   194,    -1,    -1,    -1,    -1,
     199,   200,   201,    -1,   203,   204,   205,   206,   207,   208,
      -1,   210,    -1,   212,    -1,    -1,    -1,    -1,   217,   218,
     219,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   233,    -1,   235,    -1,   237,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     249,    -1,    -1,    -1,    -1,   254,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    11,    12,    -1,    -1,    -1,    16,
      -1,    -1,    -1,    20,    21,    22,    23,    24,    25,    26,
      27,    28,   311,    30,   313,    -1,    33,    34,    -1,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    -1,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    -1,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    -1,    95,    96,
      97,    98,    99,    -1,   101,   102,   103,    -1,    -1,   106,
     107,    -1,    -1,    -1,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,    -1,    -1,    -1,   125,    -1,
     127,   128,   129,   130,    -1,   132,    -1,    -1,    -1,   136,
     137,   138,   139,   140,    -1,    -1,    -1,    -1,    -1,    -1,
     147,   148,   149,   150,   151,   152,   153,   154,   155,    -1,
     157,   158,   159,   160,   161,    -1,    -1,    -1,   165,   166,
     167,   168,   169,   170,   171,   172,    -1,   174,    -1,   176,
     177,    -1,    -1,   180,   181,   182,    -1,    -1,    -1,   186,
      -1,   188,   189,   190,   191,   192,    -1,   194,    -1,   196,
      -1,   198,   199,   200,   201,   202,   203,   204,   205,   206,
     207,   208,    -1,    -1,   211,   212,   213,    -1,   215,   216,
     217,   218,   219,   220,   221,   222,   223,   224,   225,   226,
     227,   228,   229,   230,   231,   232,   233,   234,   235,   236,
     237,   238,   239,   240,   241,   242,   243,   244,   245,   246,
     247,   248,   249,   250,   251,   252,   253,   254,   255,   256,
     257,    -1,   259,   260,   261,    -1,   263,   264,   265,   266,
     267,   268,   269,   270,   271,   272,   273,   274,   275,   276,
     277,   278,   279,   280,   281,   282,   283,   284,   285,   286,
     287,   288,   289,   290,   291,   292,   293,   294,   295,   296,
     297,   298,   299,   300,   301,   302,   303,   304,   305,   306,
     307,   308,   309,   310,    -1,   312,    -1,   314,   315,   316,
     317,   318,   319,   320,   321,   322,    11,    12,    -1,    -1,
      -1,    16,    -1,    -1,    -1,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    -1,    30,    -1,    -1,    33,    34,
      -1,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      -1,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    -1,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    -1,
      95,    96,    97,    98,    99,    -1,   101,   102,   103,    -1,
      -1,   106,   107,    -1,    -1,    -1,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,    -1,    -1,    -1,
     125,    -1,   127,   128,   129,   130,    -1,   132,    -1,    -1,
      -1,   136,   137,   138,   139,   140,    -1,    -1,    -1,    -1,
      -1,    -1,   147,   148,   149,   150,   151,   152,   153,   154,
     155,    -1,   157,   158,   159,   160,   161,    -1,    -1,    -1,
     165,   166,   167,   168,   169,   170,   171,   172,    -1,   174,
      -1,   176,   177,    -1,    -1,   180,   181,   182,    -1,    -1,
      -1,   186,    -1,   188,   189,   190,   191,   192,    -1,   194,
      -1,   196,    -1,   198,   199,   200,   201,   202,   203,   204,
     205,   206,   207,   208,    -1,    -1,   211,   212,   213,    -1,
     215,   216,   217,   218,   219,   220,   221,   222,   223,   224,
     225,   226,   227,   228,   229,   230,   231,   232,   233,   234,
     235,   236,   237,   238,   239,   240,   241,   242,   243,   244,
     245,   246,   247,   248,   249,   250,   251,   252,   253,   254,
     255,   256,   257,    -1,   259,   260,   261,    -1,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,    -1,   312,    -1,   314,
     315,   316,   317,   318,   319,   320,    -1,   322,    11,    12,
      -1,    -1,    -1,    16,    -1,    -1,    -1,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    -1,    30,    -1,    -1,
      33,    34,    -1,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    -1,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    -1,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    -1,    95,    96,    97,    98,    99,    -1,   101,   102,
     103,    -1,    -1,   106,   107,    -1,    -1,    -1,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,    -1,
      -1,    -1,   125,    -1,   127,   128,   129,   130,    -1,   132,
      -1,    -1,    -1,   136,   137,   138,   139,   140,    -1,    -1,
      -1,    -1,    -1,    -1,   147,   148,   149,   150,   151,   152,
     153,   154,   155,    -1,   157,   158,   159,   160,   161,    -1,
      -1,    -1,   165,   166,   167,   168,   169,   170,   171,   172,
      -1,   174,    -1,   176,   177,    -1,    -1,   180,   181,   182,
      -1,   184,    -1,   186,    -1,   188,   189,   190,   191,   192,
      -1,   194,    -1,   196,    -1,   198,   199,   200,   201,   202,
     203,   204,   205,   206,   207,   208,    -1,    -1,   211,   212,
     213,    -1,   215,   216,   217,   218,   219,   220,   221,   222,
     223,   224,   225,   226,   227,   228,   229,   230,   231,   232,
     233,   234,   235,   236,   237,   238,   239,   240,   241,   242,
     243,   244,   245,   246,   247,   248,   249,   250,   251,   252,
     253,   254,   255,   256,   257,    -1,   259,   260,   261,    -1,
     263,   264,   265,   266,   267,   268,   269,   270,   271,   272,
     273,   274,   275,   276,   277,   278,   279,   280,   281,   282,
     283,   284,   285,   286,   287,   288,   289,   290,   291,   292,
     293,   294,   295,   296,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,    -1,   312,
      -1,   314,   315,   316,   317,   318,   319,   320,    11,    12,
      -1,    -1,    -1,    16,    -1,    -1,    -1,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    -1,    30,    -1,    -1,
      33,    34,    -1,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    -1,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    -1,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    -1,    95,    96,    97,    98,    99,    -1,   101,   102,
     103,    -1,    -1,   106,   107,    -1,    -1,    -1,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,    -1,
      -1,    -1,   125,    -1,   127,   128,   129,   130,    -1,   132,
      -1,    -1,    -1,   136,   137,   138,   139,   140,    -1,    -1,
      -1,    -1,    -1,    -1,   147,   148,   149,   150,   151,   152,
     153,   154,   155,    -1,   157,   158,   159,   160,   161,    -1,
      -1,    -1,   165,   166,   167,   168,   169,   170,   171,   172,
      -1,   174,    -1,   176,   177,    -1,    -1,   180,   181,   182,
      -1,   184,    -1,   186,    -1,   188,   189,   190,   191,   192,
      -1,   194,    -1,   196,    -1,   198,   199,   200,   201,   202,
     203,   204,   205,   206,   207,   208,    -1,    -1,   211,   212,
     213,    -1,   215,   216,   217,   218,   219,   220,   221,   222,
     223,   224,   225,   226,   227,   228,   229,   230,   231,   232,
     233,   234,   235,   236,   237,   238,   239,   240,   241,   242,
     243,   244,   245,   246,   247,   248,   249,   250,   251,   252,
     253,   254,   255,   256,   257,    -1,   259,   260,   261,    -1,
     263,   264,   265,   266,   267,   268,   269,   270,   271,   272,
     273,   274,   275,   276,   277,   278,   279,   280,   281,   282,
     283,   284,   285,   286,   287,   288,   289,   290,   291,   292,
     293,   294,   295,   296,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,    -1,   312,
      -1,   314,   315,   316,   317,   318,   319,   320,    11,    12,
      -1,    -1,    -1,    16,    -1,    -1,    -1,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    -1,    30,    -1,    -1,
      33,    34,    -1,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    -1,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    -1,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    -1,    95,    96,    97,    98,    99,    -1,   101,   102,
     103,    -1,    -1,   106,   107,    -1,    -1,    -1,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,    -1,
      -1,    -1,   125,    -1,   127,   128,   129,   130,    -1,   132,
      -1,    -1,    -1,   136,   137,   138,   139,   140,    -1,    -1,
      -1,    -1,    -1,    -1,   147,   148,   149,   150,   151,   152,
     153,   154,   155,    -1,   157,   158,   159,   160,   161,    -1,
      -1,    -1,   165,   166,   167,   168,   169,   170,   171,   172,
      -1,   174,    -1,   176,   177,    -1,    -1,   180,   181,   182,
      -1,   184,    -1,   186,    -1,   188,   189,   190,   191,   192,
      -1,   194,    -1,   196,    -1,   198,   199,   200,   201,   202,
     203,   204,   205,   206,   207,   208,    -1,    -1,   211,   212,
     213,    -1,   215,   216,   217,   218,   219,   220,   221,   222,
     223,   224,   225,   226,   227,   228,   229,   230,   231,   232,
     233,   234,   235,   236,   237,   238,   239,   240,   241,   242,
     243,   244,   245,   246,   247,   248,   249,   250,   251,   252,
     253,   254,   255,   256,   257,    -1,   259,   260,   261,    -1,
     263,   264,   265,   266,   267,   268,   269,   270,   271,   272,
     273,   274,   275,   276,   277,   278,   279,   280,   281,   282,
     283,   284,   285,   286,   287,   288,   289,   290,   291,   292,
     293,   294,   295,   296,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,    -1,   312,
      -1,   314,   315,   316,   317,   318,   319,   320,    11,    12,
      -1,    -1,    -1,    16,    -1,    -1,    -1,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    -1,    30,    -1,    -1,
      33,    34,    -1,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    -1,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    -1,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    -1,    95,    96,    97,    98,    99,    -1,   101,   102,
     103,    -1,    -1,   106,   107,    -1,    -1,    -1,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,    -1,
      -1,    -1,   125,    -1,   127,   128,   129,   130,    -1,   132,
      -1,    -1,    -1,   136,   137,   138,   139,   140,    -1,    -1,
      -1,    -1,    -1,    -1,   147,   148,   149,   150,   151,   152,
     153,   154,   155,    -1,   157,   158,   159,   160,   161,    -1,
      -1,    -1,   165,   166,   167,   168,   169,   170,   171,   172,
      -1,   174,    -1,   176,   177,    -1,    -1,   180,   181,   182,
      -1,    -1,    -1,   186,    -1,   188,   189,   190,   191,   192,
      -1,   194,    -1,   196,    -1,   198,   199,   200,   201,   202,
     203,   204,   205,   206,   207,   208,    -1,    -1,   211,   212,
     213,    -1,   215,   216,   217,   218,   219,   220,   221,   222,
     223,   224,   225,   226,   227,   228,   229,   230,   231,   232,
     233,   234,   235,   236,   237,   238,   239,   240,   241,   242,
     243,   244,   245,   246,   247,   248,   249,   250,   251,   252,
     253,   254,   255,   256,   257,    -1,   259,   260,   261,    -1,
     263,   264,   265,   266,   267,   268,   269,   270,   271,   272,
     273,   274,   275,   276,   277,   278,   279,   280,   281,   282,
     283,   284,   285,   286,   287,   288,   289,   290,   291,   292,
     293,   294,   295,   296,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,    -1,   312,
      -1,   314,   315,   316,   317,   318,   319,   320,    11,    12,
      -1,    -1,    -1,    16,    -1,    -1,    -1,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    -1,    30,    -1,    -1,
      33,    34,    -1,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    -1,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    -1,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    -1,    95,    96,    97,    98,    99,    -1,   101,   102,
     103,    -1,    -1,   106,   107,    -1,    -1,    -1,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,    -1,
      -1,    -1,   125,    -1,   127,   128,   129,   130,    -1,   132,
      -1,    -1,    -1,   136,   137,   138,   139,   140,    -1,    -1,
      -1,    -1,    -1,    -1,   147,   148,   149,   150,   151,   152,
     153,   154,   155,    -1,   157,   158,   159,   160,   161,    -1,
      -1,    -1,   165,   166,   167,   168,   169,   170,   171,   172,
      -1,   174,    -1,   176,   177,    -1,    -1,   180,   181,   182,
      -1,    -1,    -1,   186,    -1,   188,   189,   190,   191,   192,
      -1,   194,    -1,   196,    -1,   198,   199,   200,   201,   202,
     203,   204,   205,   206,   207,   208,    -1,    -1,   211,   212,
     213,    -1,   215,   216,   217,   218,   219,   220,   221,   222,
     223,   224,   225,   226,   227,   228,   229,   230,   231,   232,
     233,   234,   235,   236,   237,   238,   239,   240,   241,   242,
     243,   244,   245,   246,   247,   248,   249,   250,   251,   252,
     253,   254,   255,   256,   257,    -1,   259,   260,   261,    -1,
     263,   264,   265,   266,   267,   268,   269,   270,   271,   272,
     273,   274,   275,   276,   277,   278,   279,   280,   281,   282,
     283,   284,   285,   286,   287,   288,   289,   290,   291,   292,
     293,   294,   295,   296,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,    -1,   312,
      -1,   314,   315,   316,   317,   318,   319,   320,    11,    12,
      -1,    -1,    -1,    16,    -1,    -1,    -1,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    -1,    30,    -1,    -1,
      33,    34,    -1,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    -1,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    -1,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    -1,    95,    96,    97,    98,    99,    -1,   101,   102,
     103,    -1,    -1,   106,   107,    -1,    -1,    -1,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,    -1,
      -1,    -1,   125,    -1,   127,   128,   129,   130,    -1,   132,
      -1,    -1,    -1,   136,   137,   138,   139,   140,    -1,    -1,
      -1,    -1,    -1,    -1,   147,   148,   149,   150,   151,   152,
     153,   154,   155,    -1,   157,   158,   159,   160,   161,    -1,
      -1,    -1,   165,   166,   167,   168,   169,   170,   171,   172,
      -1,   174,    -1,   176,   177,    -1,    -1,   180,   181,   182,
      -1,    -1,    -1,   186,    -1,   188,   189,   190,   191,   192,
      -1,   194,    -1,   196,    -1,   198,   199,   200,   201,   202,
     203,   204,   205,   206,   207,   208,    -1,    -1,   211,   212,
     213,    -1,   215,   216,   217,   218,   219,   220,   221,   222,
     223,   224,   225,   226,   227,   228,   229,   230,   231,   232,
     233,   234,   235,   236,   237,   238,   239,   240,   241,   242,
     243,   244,   245,   246,   247,   248,   249,   250,   251,   252,
     253,   254,   255,   256,   257,    -1,   259,   260,   261,    -1,
     263,   264,   265,   266,   267,   268,   269,   270,   271,   272,
     273,   274,   275,   276,   277,   278,   279,   280,   281,   282,
     283,   284,   285,   286,   287,   288,   289,   290,   291,   292,
     293,   294,   295,   296,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,    -1,   312,
      -1,   314,   315,   316,   317,   318,   319,   320,    11,    12,
      -1,    -1,    -1,    16,    -1,    -1,    -1,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    -1,    30,    -1,    -1,
      33,    34,    -1,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    -1,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    -1,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    -1,    95,    96,    97,    98,    99,    -1,   101,   102,
     103,    -1,    -1,   106,   107,    -1,    -1,    -1,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,    -1,
      -1,    -1,   125,    -1,   127,   128,   129,   130,    -1,   132,
      -1,    -1,    -1,   136,   137,   138,   139,   140,    -1,    -1,
      -1,    -1,    -1,    -1,   147,   148,   149,   150,   151,   152,
     153,   154,   155,    -1,   157,   158,   159,   160,   161,    -1,
      -1,    -1,   165,   166,   167,   168,   169,   170,   171,   172,
      -1,   174,    -1,   176,   177,    -1,    -1,   180,   181,   182,
      -1,    -1,    -1,   186,    -1,   188,   189,   190,   191,   192,
      -1,   194,    -1,   196,    -1,   198,   199,   200,   201,   202,
     203,   204,   205,   206,   207,   208,    -1,    -1,   211,   212,
     213,    -1,   215,   216,   217,   218,   219,   220,   221,   222,
     223,   224,   225,   226,   227,   228,   229,   230,   231,   232,
     233,   234,   235,   236,   237,   238,   239,   240,   241,   242,
     243,   244,   245,   246,   247,   248,   249,   250,   251,   252,
     253,   254,   255,   256,   257,    -1,   259,   260,   261,    -1,
     263,   264,   265,   266,   267,   268,   269,   270,   271,   272,
     273,   274,   275,   276,   277,   278,   279,   280,   281,   282,
     283,   284,   285,   286,   287,   288,   289,   290,   291,   292,
     293,   294,   295,   296,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,    -1,   312,
      -1,   314,   315,   316,   317,   318,   319,   320,    11,    12,
      -1,    -1,    -1,    16,    -1,    -1,    -1,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    -1,    30,    -1,    -1,
      33,    34,    -1,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    -1,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    -1,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    -1,    95,    96,    97,    98,    99,    -1,   101,   102,
     103,    -1,    -1,   106,   107,    -1,    -1,    -1,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,    -1,
      -1,    -1,   125,    -1,   127,   128,   129,   130,    -1,   132,
      -1,    -1,    -1,   136,   137,   138,   139,   140,    -1,    -1,
      -1,    -1,    -1,    -1,   147,   148,   149,   150,   151,   152,
     153,   154,   155,    -1,   157,   158,   159,   160,   161,    -1,
      -1,    -1,   165,   166,   167,   168,   169,   170,   171,   172,
      -1,   174,    -1,   176,   177,    -1,    -1,   180,   181,   182,
      -1,    -1,    -1,   186,    -1,   188,   189,   190,   191,   192,
      -1,   194,    -1,   196,    -1,   198,   199,   200,   201,   202,
     203,   204,   205,   206,   207,   208,    -1,    -1,   211,   212,
     213,    -1,   215,   216,   217,   218,   219,   220,   221,   222,
     223,   224,   225,   226,   227,   228,   229,   230,   231,   232,
     233,   234,   235,   236,   237,   238,   239,   240,   241,   242,
     243,   244,   245,   246,   247,   248,   249,   250,   251,   252,
     253,   254,   255,   256,   257,    -1,   259,   260,   261,    -1,
     263,   264,   265,   266,   267,   268,   269,   270,   271,   272,
     273,   274,   275,   276,   277,   278,   279,   280,   281,   282,
     283,   284,   285,   286,   287,   288,   289,   290,   291,   292,
     293,   294,   295,   296,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,    -1,   312,
      -1,   314,   315,   316,   317,   318,   319,   320,    11,    12,
      -1,    -1,    -1,    16,    -1,    -1,    -1,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    -1,    30,    -1,    -1,
      33,    34,    -1,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    -1,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    -1,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    -1,    95,    96,    97,    98,    99,    -1,   101,   102,
     103,    -1,    -1,   106,   107,    -1,    -1,    -1,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,    -1,
      -1,    -1,   125,    -1,   127,   128,   129,   130,    -1,   132,
      -1,    -1,    -1,   136,   137,   138,   139,   140,    -1,    -1,
      -1,    -1,    -1,    -1,   147,   148,   149,   150,   151,   152,
     153,   154,   155,    -1,   157,   158,   159,   160,   161,    -1,
      -1,    -1,   165,   166,   167,   168,   169,   170,   171,   172,
      -1,   174,    -1,   176,   177,    -1,    -1,   180,   181,   182,
      -1,    -1,    -1,   186,    -1,   188,   189,   190,   191,   192,
      -1,   194,    -1,   196,    -1,   198,   199,   200,   201,   202,
     203,   204,   205,   206,   207,   208,    -1,    -1,   211,   212,
     213,    -1,   215,   216,   217,   218,   219,   220,   221,   222,
     223,   224,   225,   226,   227,   228,   229,   230,   231,   232,
     233,   234,   235,   236,   237,   238,   239,   240,   241,   242,
     243,   244,   245,   246,   247,   248,   249,   250,   251,   252,
     253,   254,   255,   256,   257,    -1,   259,   260,   261,    -1,
     263,   264,   265,   266,   267,   268,   269,   270,   271,   272,
     273,   274,   275,   276,   277,   278,   279,   280,   281,   282,
     283,   284,   285,   286,   287,   288,   289,   290,   291,   292,
     293,   294,   295,   296,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,    -1,   312,
      -1,   314,   315,   316,   317,   318,   319,   320,    11,    12,
      -1,    -1,    -1,    16,    -1,    -1,    -1,    20,    21,    22,
      23,    24,    25,    26,    -1,    -1,    -1,    30,    -1,    -1,
      33,    34,    -1,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    -1,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    -1,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    -1,    95,    96,    97,    98,    99,    -1,   101,   102,
     103,    -1,    -1,   106,   107,    -1,    -1,    -1,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,    -1,
      -1,    -1,   125,    -1,   127,   128,   129,   130,    -1,   132,
      -1,    -1,    -1,   136,   137,   138,   139,   140,    -1,    -1,
      -1,    -1,   145,    -1,   147,   148,   149,   150,   151,   152,
     153,   154,   155,    -1,   157,   158,   159,   160,   161,    -1,
      -1,    -1,   165,   166,   167,   168,   169,   170,   171,   172,
      -1,   174,    -1,   176,   177,    -1,    -1,   180,   181,   182,
      -1,    -1,    -1,   186,   187,   188,   189,    -1,   191,   192,
      -1,   194,    -1,   196,    -1,   198,   199,   200,   201,   202,
     203,   204,   205,   206,   207,   208,    -1,    -1,   211,   212,
     213,    -1,   215,   216,   217,   218,   219,   220,   221,   222,
     223,   224,   225,   226,   227,   228,   229,   230,   231,   232,
     233,   234,   235,   236,   237,   238,   239,   240,   241,   242,
     243,   244,   245,   246,   247,   248,   249,   250,   251,   252,
     253,   254,   255,   256,   257,    -1,   259,   260,   261,    -1,
     263,   264,   265,   266,   267,   268,   269,   270,   271,   272,
     273,   274,   275,   276,   277,   278,   279,   280,   281,   282,
     283,   284,   285,   286,   287,   288,   289,   290,   291,   292,
     293,   294,   295,   296,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,    -1,   312,
      -1,   314,   315,   316,   317,   318,   319,   320,    11,    12,
      -1,    -1,    -1,    16,    -1,    -1,    -1,    20,    21,    22,
      23,    24,    25,    26,    -1,    -1,    -1,    30,    -1,    -1,
      33,    34,    -1,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    -1,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    -1,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    -1,    95,    96,    97,    98,    99,    -1,   101,   102,
     103,    -1,    -1,   106,   107,    -1,    -1,    -1,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,    -1,
      -1,    -1,   125,    -1,   127,   128,   129,   130,    -1,   132,
      -1,    -1,    -1,   136,   137,   138,   139,   140,    -1,    -1,
      -1,    -1,   145,    -1,   147,   148,   149,   150,   151,   152,
     153,   154,   155,    -1,   157,   158,   159,   160,   161,    -1,
      -1,    -1,   165,   166,   167,   168,   169,   170,   171,   172,
      -1,   174,    -1,   176,   177,    -1,    -1,   180,   181,   182,
      -1,    -1,    -1,   186,   187,   188,   189,    -1,   191,   192,
      -1,   194,    -1,   196,    -1,   198,   199,   200,   201,   202,
     203,   204,   205,   206,   207,   208,    -1,    -1,   211,   212,
     213,    -1,   215,   216,   217,   218,   219,   220,   221,   222,
     223,   224,   225,   226,   227,   228,   229,   230,   231,   232,
     233,   234,   235,   236,   237,   238,   239,   240,   241,   242,
     243,   244,   245,   246,   247,   248,   249,   250,   251,   252,
     253,   254,   255,   256,   257,    -1,   259,   260,   261,    -1,
     263,   264,   265,   266,   267,   268,   269,   270,   271,   272,
     273,   274,   275,   276,   277,   278,   279,   280,   281,   282,
     283,   284,   285,   286,   287,   288,   289,   290,   291,   292,
     293,   294,   295,   296,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,    -1,   312,
      -1,   314,   315,   316,   317,   318,   319,   320,    11,    12,
      -1,    -1,    -1,    16,    -1,    -1,    -1,    20,    21,    22,
      23,    24,    25,    26,    -1,    -1,    -1,    30,    -1,    -1,
      33,    34,    -1,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    -1,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    -1,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    -1,    95,    96,    97,    98,    99,    -1,   101,   102,
     103,    -1,    -1,   106,   107,    -1,    -1,    -1,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,    -1,
      -1,    -1,   125,    -1,   127,   128,   129,   130,    -1,   132,
      -1,    -1,    -1,   136,   137,   138,   139,   140,    -1,    -1,
      -1,    -1,    -1,    -1,   147,   148,   149,   150,   151,   152,
     153,   154,   155,    -1,   157,   158,   159,   160,   161,    -1,
      -1,    -1,   165,   166,   167,   168,   169,   170,   171,   172,
      -1,   174,    -1,   176,   177,    -1,    -1,   180,   181,   182,
      -1,    -1,   185,   186,    -1,   188,   189,    -1,   191,   192,
      -1,   194,    -1,   196,    -1,   198,   199,   200,   201,   202,
     203,   204,   205,   206,   207,   208,    -1,    -1,   211,   212,
     213,    -1,   215,   216,   217,   218,   219,   220,   221,   222,
     223,   224,   225,   226,   227,   228,   229,   230,   231,   232,
     233,   234,   235,   236,   237,   238,   239,   240,   241,   242,
     243,   244,   245,   246,   247,   248,   249,   250,   251,   252,
     253,   254,   255,   256,   257,    -1,   259,   260,   261,    -1,
     263,   264,   265,   266,   267,   268,   269,   270,   271,   272,
     273,   274,   275,   276,   277,   278,   279,   280,   281,   282,
     283,   284,   285,   286,   287,   288,   289,   290,   291,   292,
     293,   294,   295,   296,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,    -1,   312,
      -1,   314,   315,   316,   317,   318,   319,   320,    11,    12,
      -1,    -1,    -1,    16,    -1,    -1,    -1,    20,    21,    22,
      23,    24,    25,    26,    -1,    -1,    -1,    30,    -1,    -1,
      33,    34,    -1,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    -1,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    -1,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    -1,    95,    96,    97,    98,    99,    -1,   101,   102,
     103,    -1,    -1,   106,   107,    -1,    -1,    -1,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,    -1,
      -1,    -1,   125,    -1,   127,   128,   129,   130,    -1,   132,
      -1,    -1,    -1,   136,   137,   138,   139,   140,    -1,    -1,
      -1,    -1,    -1,    -1,   147,   148,   149,   150,   151,   152,
     153,   154,   155,    -1,   157,   158,   159,   160,   161,    -1,
      -1,    -1,   165,   166,   167,   168,   169,   170,   171,   172,
      -1,   174,    -1,   176,   177,    -1,    -1,   180,   181,   182,
      -1,    -1,    -1,   186,   187,   188,   189,    -1,   191,   192,
      -1,   194,    -1,   196,    -1,   198,   199,   200,   201,   202,
     203,   204,   205,   206,   207,   208,    -1,    -1,   211,   212,
     213,    -1,   215,   216,   217,   218,   219,   220,   221,   222,
     223,   224,   225,   226,   227,   228,   229,   230,   231,   232,
     233,   234,   235,   236,   237,   238,   239,   240,   241,   242,
     243,   244,   245,   246,   247,   248,   249,   250,   251,   252,
     253,   254,   255,   256,   257,    -1,   259,   260,   261,    -1,
     263,   264,   265,   266,   267,   268,   269,   270,   271,   272,
     273,   274,   275,   276,   277,   278,   279,   280,   281,   282,
     283,   284,   285,   286,   287,   288,   289,   290,   291,   292,
     293,   294,   295,   296,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,    -1,   312,
      -1,   314,   315,   316,   317,   318,   319,   320,    11,    12,
      -1,    -1,    -1,    16,    -1,    -1,    -1,    20,    21,    22,
      23,    24,    25,    26,    -1,    -1,    -1,    30,    -1,    -1,
      33,    34,    -1,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    -1,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    -1,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    -1,    95,    96,    97,    98,    99,    -1,   101,   102,
     103,    -1,    -1,   106,   107,    -1,    -1,    -1,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,    -1,
      -1,    -1,   125,    -1,   127,   128,   129,   130,    -1,   132,
      -1,    -1,    -1,   136,   137,   138,   139,   140,    -1,    -1,
      -1,    -1,    -1,    -1,   147,   148,   149,   150,   151,   152,
     153,   154,   155,    -1,   157,   158,   159,   160,   161,    -1,
      -1,    -1,   165,   166,   167,   168,   169,   170,   171,   172,
      -1,   174,    -1,   176,   177,    -1,    -1,   180,   181,   182,
      -1,    -1,    -1,   186,    -1,   188,   189,    -1,   191,   192,
      -1,   194,    -1,   196,    -1,   198,   199,   200,   201,   202,
     203,   204,   205,   206,   207,   208,    -1,    -1,   211,   212,
     213,    -1,   215,   216,   217,   218,   219,   220,   221,   222,
     223,   224,   225,   226,   227,   228,   229,   230,   231,   232,
     233,   234,   235,   236,   237,   238,   239,   240,   241,   242,
     243,   244,   245,   246,   247,   248,   249,   250,   251,   252,
     253,   254,   255,   256,   257,    -1,   259,   260,   261,    -1,
     263,   264,   265,   266,   267,   268,   269,   270,   271,   272,
     273,   274,   275,   276,   277,   278,   279,   280,   281,   282,
     283,   284,   285,   286,   287,   288,   289,   290,   291,   292,
     293,   294,   295,   296,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,   311,   312,
      -1,   314,   315,   316,   317,   318,   319,   320,    11,    12,
      -1,    -1,    -1,    16,    -1,    -1,    -1,    20,    21,    22,
      23,    24,    25,    26,    -1,    -1,    -1,    30,    -1,    -1,
      33,    34,    -1,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    -1,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    -1,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    -1,    95,    96,    97,    98,    99,    -1,   101,   102,
     103,    -1,    -1,   106,   107,    -1,    -1,    -1,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,    -1,
      -1,    -1,   125,    -1,   127,   128,   129,   130,    -1,   132,
      -1,    -1,    -1,   136,   137,   138,   139,   140,    -1,    -1,
      -1,    -1,    -1,    -1,   147,   148,   149,   150,   151,   152,
     153,   154,   155,    -1,   157,   158,   159,   160,   161,    -1,
      -1,    -1,   165,   166,   167,   168,   169,   170,   171,   172,
      -1,   174,    -1,   176,   177,    -1,    -1,   180,   181,   182,
      -1,    -1,    -1,   186,    -1,   188,   189,    -1,   191,   192,
      -1,   194,    -1,   196,    -1,   198,   199,   200,   201,   202,
     203,   204,   205,   206,   207,   208,    -1,    -1,   211,   212,
     213,    -1,   215,   216,   217,   218,   219,   220,   221,   222,
     223,   224,   225,   226,   227,   228,   229,   230,   231,   232,
     233,   234,   235,   236,   237,   238,   239,   240,   241,   242,
     243,   244,   245,   246,   247,   248,   249,   250,   251,   252,
     253,   254,   255,   256,   257,    -1,   259,   260,   261,    -1,
     263,   264,   265,   266,   267,   268,   269,   270,   271,   272,
     273,   274,   275,   276,   277,   278,   279,   280,   281,   282,
     283,   284,   285,   286,   287,   288,   289,   290,   291,   292,
     293,   294,   295,   296,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,    -1,   312,
     313,   314,   315,   316,   317,   318,   319,   320,    11,    12,
      -1,    -1,    -1,    16,    -1,    -1,    -1,    20,    21,    22,
      23,    24,    25,    26,    -1,    -1,    -1,    30,    -1,    -1,
      33,    34,    -1,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    -1,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    -1,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    -1,    95,    96,    97,    98,    99,    -1,   101,   102,
     103,    -1,    -1,   106,   107,    -1,    -1,    -1,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,    -1,
      -1,    -1,   125,    -1,   127,   128,   129,   130,    -1,   132,
      -1,    -1,    -1,   136,   137,   138,   139,   140,    -1,    -1,
      -1,    -1,   145,    -1,   147,   148,   149,   150,   151,   152,
     153,   154,   155,    -1,   157,   158,   159,   160,   161,    -1,
      -1,    -1,   165,   166,   167,   168,   169,   170,   171,   172,
      -1,   174,    -1,   176,   177,    -1,    -1,   180,   181,   182,
      -1,    -1,    -1,   186,    -1,   188,   189,    -1,   191,   192,
      -1,   194,    -1,   196,    -1,   198,   199,   200,   201,   202,
     203,   204,   205,   206,   207,   208,    -1,    -1,   211,   212,
     213,    -1,   215,   216,   217,   218,   219,   220,   221,   222,
     223,   224,   225,   226,   227,   228,   229,   230,   231,   232,
     233,   234,   235,   236,   237,   238,   239,   240,   241,   242,
     243,   244,   245,   246,   247,   248,   249,   250,   251,   252,
     253,   254,   255,   256,   257,    -1,   259,   260,   261,    -1,
     263,   264,   265,   266,   267,   268,   269,   270,   271,   272,
     273,   274,   275,   276,   277,   278,   279,   280,   281,   282,
     283,   284,   285,   286,   287,   288,   289,   290,   291,   292,
     293,   294,   295,   296,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,    -1,   312,
      -1,   314,   315,   316,   317,   318,   319,   320,    11,    12,
      -1,    -1,    -1,    16,    -1,    -1,    -1,    20,    21,    22,
      23,    24,    25,    26,    -1,    -1,    -1,    30,    -1,    -1,
      33,    34,    -1,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    -1,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    -1,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    -1,    95,    96,    97,    98,    99,    -1,   101,   102,
     103,    -1,    -1,   106,   107,    -1,    -1,    -1,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,    -1,
      -1,    -1,   125,    -1,   127,   128,   129,   130,    -1,   132,
      -1,    -1,    -1,   136,   137,   138,   139,   140,    -1,    -1,
      -1,    -1,    -1,    -1,   147,   148,   149,   150,   151,   152,
     153,   154,   155,    -1,   157,   158,   159,   160,   161,    -1,
      -1,    -1,   165,   166,   167,   168,   169,   170,   171,   172,
      -1,   174,    -1,   176,   177,    -1,    -1,   180,   181,   182,
      -1,    -1,    -1,   186,    -1,   188,   189,    -1,   191,   192,
      -1,   194,    -1,   196,    -1,   198,   199,   200,   201,   202,
     203,   204,   205,   206,   207,   208,    -1,    -1,   211,   212,
     213,    -1,   215,   216,   217,   218,   219,   220,   221,   222,
     223,   224,   225,   226,   227,   228,   229,   230,   231,   232,
     233,   234,   235,   236,   237,   238,   239,   240,   241,   242,
     243,   244,   245,   246,   247,   248,   249,   250,   251,   252,
     253,   254,   255,   256,   257,    -1,   259,   260,   261,    -1,
     263,   264,   265,   266,   267,   268,   269,   270,   271,   272,
     273,   274,   275,   276,   277,   278,   279,   280,   281,   282,
     283,   284,   285,   286,   287,   288,   289,   290,   291,   292,
     293,   294,   295,   296,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,    -1,   312,
      -1,   314,   315,   316,   317,   318,   319,   320,    11,    12,
      -1,    -1,    -1,    16,    -1,    -1,    -1,    20,    21,    22,
      23,    24,    25,    26,    -1,    -1,    -1,    30,    -1,    -1,
      33,    34,    -1,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    -1,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    -1,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    -1,    95,    96,    97,    98,    99,    -1,   101,   102,
     103,    -1,    -1,   106,   107,    -1,    -1,    -1,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,    -1,
      -1,    -1,   125,    -1,   127,   128,   129,   130,    -1,   132,
      -1,    -1,    -1,   136,   137,   138,   139,   140,    -1,    -1,
      -1,    -1,    -1,    -1,   147,   148,   149,   150,   151,   152,
     153,   154,   155,    -1,   157,   158,   159,   160,   161,    -1,
      -1,    -1,   165,   166,   167,   168,   169,   170,   171,   172,
      -1,   174,    -1,   176,   177,    -1,    -1,   180,   181,   182,
      -1,    -1,    -1,   186,    -1,   188,   189,    -1,   191,   192,
      -1,   194,    -1,   196,    -1,   198,   199,   200,   201,   202,
     203,   204,   205,   206,   207,   208,    -1,    -1,   211,   212,
     213,    -1,   215,   216,   217,   218,   219,   220,   221,   222,
     223,   224,   225,   226,   227,   228,   229,   230,   231,   232,
     233,   234,   235,   236,   237,   238,   239,   240,   241,   242,
     243,   244,   245,   246,   247,   248,   249,   250,   251,   252,
     253,   254,   255,   256,   257,    -1,   259,   260,   261,    -1,
     263,   264,   265,   266,   267,   268,   269,   270,   271,   272,
     273,   274,   275,   276,   277,   278,   279,   280,   281,   282,
     283,   284,   285,   286,   287,   288,   289,   290,   291,   292,
     293,   294,   295,   296,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,    -1,   312,
      -1,   314,   315,   316,   317,   318,   319,   320,    11,    12,
      -1,    -1,    -1,    16,    -1,    -1,    -1,    20,    21,    22,
      23,    24,    25,    26,    -1,    -1,    -1,    30,    -1,    -1,
      33,    34,    -1,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    -1,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    -1,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    -1,    95,    96,    97,    98,    99,    -1,   101,   102,
     103,    -1,    -1,   106,   107,    -1,    -1,    -1,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,    -1,
      -1,    -1,   125,    -1,   127,   128,   129,   130,    -1,   132,
      -1,    -1,    -1,   136,   137,   138,   139,   140,    -1,    -1,
      -1,    -1,    -1,    -1,   147,   148,   149,   150,   151,   152,
     153,   154,   155,    -1,   157,   158,   159,   160,   161,    -1,
      -1,    -1,   165,   166,   167,   168,   169,   170,   171,   172,
      -1,   174,    -1,   176,   177,    -1,    -1,   180,   181,   182,
      -1,    -1,    -1,   186,    -1,   188,   189,    -1,   191,   192,
      -1,   194,    -1,   196,    -1,   198,   199,   200,   201,   202,
     203,   204,   205,   206,   207,   208,    -1,    -1,   211,   212,
     213,    -1,   215,   216,   217,   218,   219,   220,   221,   222,
     223,   224,   225,   226,   227,   228,   229,   230,   231,   232,
     233,   234,   235,   236,   237,   238,   239,   240,   241,   242,
     243,   244,   245,   246,   247,   248,   249,   250,   251,   252,
     253,   254,   255,   256,   257,    -1,   259,   260,   261,    -1,
     263,   264,   265,   266,   267,   268,   269,   270,   271,   272,
     273,   274,   275,   276,   277,   278,   279,   280,   281,   282,
     283,   284,   285,   286,   287,   288,   289,   290,   291,   292,
     293,   294,   295,   296,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,    -1,   312,
      -1,   314,   315,   316,   317,   318,   319,   320,    11,    12,
      -1,    -1,    -1,    16,    -1,    -1,    -1,    20,    21,    22,
      23,    24,    25,    26,    -1,    -1,    -1,    30,    -1,    -1,
      33,    34,    -1,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    -1,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    -1,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    -1,    95,    96,    97,    98,    99,    -1,   101,   102,
     103,    -1,    -1,   106,   107,    -1,    -1,    -1,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,    -1,
      -1,    -1,   125,    -1,   127,   128,   129,   130,    -1,   132,
      -1,    -1,    -1,   136,   137,   138,   139,   140,    -1,    -1,
      -1,    -1,    -1,    -1,   147,   148,   149,   150,   151,   152,
     153,   154,   155,    -1,   157,   158,   159,   160,   161,    -1,
      -1,    -1,   165,   166,   167,   168,   169,   170,   171,   172,
      -1,   174,    -1,   176,   177,    -1,    -1,   180,   181,   182,
      -1,    -1,    -1,   186,    -1,   188,   189,    -1,   191,   192,
      -1,   194,    -1,   196,    -1,   198,   199,   200,   201,   202,
     203,   204,   205,   206,   207,   208,    -1,    -1,   211,   212,
     213,    -1,   215,   216,   217,   218,   219,   220,   221,   222,
     223,   224,   225,   226,   227,   228,   229,   230,   231,   232,
     233,   234,   235,   236,   237,   238,   239,   240,   241,   242,
     243,   244,   245,   246,   247,   248,   249,   250,   251,   252,
     253,   254,   255,   256,   257,    -1,   259,   260,   261,    -1,
     263,   264,   265,   266,   267,   268,   269,   270,   271,   272,
     273,   274,   275,   276,   277,   278,   279,   280,   281,   282,
     283,   284,   285,   286,   287,   288,   289,   290,   291,   292,
     293,   294,   295,   296,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,    -1,   312,
      -1,   314,   315,   316,   317,   318,   319,   320,    11,    12,
      -1,    -1,    -1,    16,    -1,    -1,    -1,    20,    21,    22,
      23,    24,    25,    26,    -1,    -1,    -1,    30,    -1,    -1,
      33,    34,    -1,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    -1,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    -1,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    -1,    95,    96,    97,    98,    99,    -1,   101,   102,
     103,    -1,    -1,   106,   107,    -1,    -1,    -1,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,    -1,
      -1,    -1,   125,    -1,   127,   128,   129,   130,    -1,   132,
      -1,    -1,    -1,   136,   137,   138,   139,   140,    -1,    -1,
      -1,    -1,    -1,    -1,   147,   148,   149,   150,   151,   152,
     153,   154,   155,    -1,   157,   158,    -1,   160,   161,    -1,
      -1,    -1,   165,   166,   167,   168,   169,   170,   171,   172,
      -1,   174,    -1,    -1,    -1,    -1,    -1,   180,   181,   182,
      -1,    -1,    -1,   186,    -1,   188,   189,    -1,   191,   192,
      -1,   194,    -1,   196,    -1,   198,   199,   200,   201,   202,
     203,   204,   205,   206,   207,   208,    -1,    -1,   211,   212,
     213,    -1,   215,   216,   217,   218,   219,   220,   221,   222,
     223,   224,   225,   226,   227,   228,   229,   230,   231,   232,
     233,   234,   235,   236,   237,   238,   239,   240,   241,   242,
     243,   244,   245,   246,   247,   248,   249,   250,   251,   252,
     253,   254,   255,   256,   257,    -1,   259,   260,   261,    -1,
     263,   264,   265,   266,   267,   268,   269,   270,   271,   272,
     273,   274,   275,   276,   277,   278,   279,   280,   281,   282,
     283,   284,   285,   286,   287,   288,   289,   290,   291,   292,
     293,   294,   295,   296,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,    -1,   312,
      -1,   314,   315,   316,   317,   318,   319,   320,    11,    12,
      -1,    -1,    -1,    16,    -1,    -1,    -1,    20,    21,    22,
      23,    24,    25,    26,    -1,    -1,    -1,    30,    -1,    -1,
      33,    34,    -1,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    -1,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    -1,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    -1,    95,    96,    97,    98,    99,    -1,   101,   102,
     103,    -1,    -1,   106,   107,    -1,    -1,    -1,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,    -1,
      -1,    -1,   125,    -1,   127,   128,   129,   130,    -1,   132,
      -1,    -1,    -1,   136,   137,   138,   139,   140,    -1,    -1,
      -1,    -1,    -1,    -1,   147,   148,   149,   150,   151,   152,
     153,   154,   155,    -1,   157,   158,    -1,   160,   161,    -1,
      -1,    -1,   165,   166,   167,   168,   169,   170,   171,   172,
      -1,   174,    -1,    -1,    -1,    -1,    -1,   180,   181,   182,
      -1,    -1,    -1,   186,    -1,   188,   189,    -1,    -1,    -1,
      -1,   194,    -1,   196,    -1,   198,   199,   200,   201,   202,
     203,   204,   205,   206,   207,   208,    -1,    -1,   211,   212,
     213,    -1,   215,   216,   217,   218,   219,   220,   221,   222,
     223,   224,   225,   226,   227,   228,   229,   230,   231,   232,
     233,   234,   235,   236,   237,   238,   239,   240,   241,   242,
     243,   244,   245,   246,   247,   248,   249,   250,   251,   252,
     253,   254,   255,   256,   257,    -1,   259,   260,   261,    -1,
     263,   264,   265,   266,   267,   268,   269,   270,   271,   272,
     273,   274,   275,   276,   277,   278,   279,   280,   281,   282,
     283,   284,   285,   286,   287,   288,   289,   290,   291,   292,
     293,   294,   295,   296,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,    -1,   312,
      -1,   314,   315,   316,   317,   318,   319,   320,    11,    12,
      -1,    -1,    -1,    16,    -1,    -1,    -1,    20,    21,    22,
      23,    24,    25,    26,    -1,    -1,    -1,    30,    -1,    -1,
      33,    34,    -1,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    -1,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    -1,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    -1,    95,    96,    97,    98,    99,    -1,   101,   102,
     103,    -1,    -1,   106,   107,    -1,    -1,    -1,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,    -1,
      -1,    -1,   125,    -1,   127,   128,   129,   130,    -1,   132,
      -1,    -1,    -1,   136,    -1,   138,   139,   140,    -1,    -1,
      -1,    -1,    -1,    -1,   147,   148,   149,   150,    -1,   152,
     153,   154,   155,    -1,   157,   158,    -1,   160,   161,    -1,
      -1,    -1,   165,   166,   167,   168,   169,   170,   171,   172,
      -1,   174,    -1,    -1,    -1,    -1,    -1,   180,   181,   182,
      -1,    -1,    -1,   186,    -1,   188,   189,    -1,    -1,    -1,
      -1,   194,    -1,   196,    -1,   198,    -1,   200,    -1,   202,
     203,   204,   205,   206,   207,   208,    -1,    -1,   211,   212,
     213,    -1,   215,   216,   217,   218,   219,   220,   221,   222,
     223,   224,   225,   226,   227,   228,   229,   230,   231,   232,
     233,   234,   235,   236,   237,   238,   239,   240,   241,   242,
     243,   244,   245,   246,   247,   248,   249,   250,   251,   252,
     253,   254,   255,   256,   257,    -1,   259,   260,   261,    -1,
     263,   264,   265,   266,   267,   268,   269,   270,   271,   272,
     273,   274,   275,   276,   277,   278,   279,   280,   281,   282,
     283,   284,   285,   286,   287,   288,   289,   290,   291,   292,
     293,   294,   295,   296,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,    -1,   312,
      -1,   314,   315,   316,   317,   318,   319,   320,    11,    12,
      -1,    -1,    -1,    16,    -1,    -1,    -1,    20,    -1,    -1,
      -1,    -1,    25,    26,    -1,    -1,    -1,    -1,    -1,    -1,
      33,    34,    -1,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    -1,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    -1,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    -1,    95,    96,    97,    98,    -1,    -1,   101,   102,
     103,    -1,    -1,   106,   107,    -1,    -1,    -1,    -1,   112,
     113,   114,   115,   116,   117,   118,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   127,   128,   129,   130,    -1,   132,
      -1,    -1,    -1,   136,   137,   138,   139,   140,    -1,    -1,
      -1,    -1,    -1,    -1,   147,   148,   149,    -1,   151,   152,
     153,    -1,    -1,    -1,    -1,    -1,    -1,   160,   161,    -1,
      -1,    -1,   165,   166,   167,   168,   169,   170,   171,   172,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   180,   181,   182,
      -1,    -1,    -1,   186,    -1,   188,   189,    -1,    -1,    -1,
      -1,   194,    -1,   196,    -1,   198,   199,   200,   201,   202,
     203,   204,   205,   206,   207,   208,    -1,    -1,   211,   212,
      -1,    -1,   215,   216,   217,   218,   219,   220,   221,   222,
     223,   224,   225,   226,   227,   228,   229,   230,   231,   232,
     233,   234,   235,   236,   237,   238,   239,   240,   241,   242,
     243,   244,   245,   246,   247,   248,   249,   250,   251,   252,
     253,   254,   255,   256,   257,    -1,   259,   260,   261,    -1,
     263,   264,   265,   266,   267,   268,   269,   270,   271,   272,
     273,   274,   275,   276,   277,   278,   279,   280,   281,   282,
     283,   284,   285,   286,   287,   288,   289,   290,   291,   292,
     293,   294,   295,   296,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,    -1,    -1,    -1,
      -1,   314,   315,   316,   317,   318,   319,   320,    11,    12,
      -1,    -1,    -1,    16,    -1,    -1,    -1,    20,    -1,    -1,
      -1,    -1,    25,    26,    -1,    -1,    -1,    -1,    -1,    -1,
      33,    34,    -1,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    -1,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    -1,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    -1,    95,    96,    97,    98,    -1,    -1,   101,   102,
     103,    -1,    -1,   106,   107,    -1,    -1,    -1,    -1,   112,
     113,   114,   115,   116,   117,   118,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   127,   128,   129,   130,    -1,   132,
      -1,    -1,    -1,   136,   137,   138,   139,   140,    -1,    -1,
      -1,    -1,    -1,    -1,   147,   148,   149,    -1,   151,   152,
     153,    -1,    -1,    -1,    -1,    -1,    -1,   160,   161,    -1,
      -1,    -1,   165,   166,   167,   168,   169,   170,   171,   172,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   180,   181,   182,
      -1,    -1,    -1,   186,    -1,   188,   189,    -1,    -1,    -1,
      -1,   194,    -1,   196,    -1,   198,   199,   200,   201,   202,
     203,   204,   205,   206,   207,   208,    -1,    -1,   211,   212,
      -1,    -1,   215,   216,   217,   218,   219,   220,   221,   222,
     223,   224,   225,   226,   227,   228,   229,   230,   231,   232,
     233,   234,   235,   236,   237,   238,   239,   240,   241,   242,
     243,   244,   245,   246,   247,   248,   249,   250,   251,   252,
     253,   254,   255,   256,   257,    -1,   259,   260,   261,    -1,
     263,   264,   265,   266,   267,   268,   269,   270,   271,   272,
     273,   274,   275,   276,   277,   278,   279,   280,   281,   282,
     283,   284,   285,   286,   287,   288,   289,   290,   291,   292,
     293,   294,   295,   296,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,    -1,    -1,    16,
      -1,   314,   315,   316,   317,   318,   319,   320,    25,    26,
      -1,    -1,    -1,    -1,    -1,    -1,    33,    34,    -1,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    -1,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    -1,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    -1,    95,    96,
      97,    98,    -1,    -1,   101,   102,   103,    -1,    -1,   106,
     107,    -1,    -1,    -1,    -1,   112,   113,   114,   115,   116,
     117,   118,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     127,   128,   129,   130,    -1,   132,    -1,    -1,    -1,   136,
     137,   138,   139,   140,    -1,    -1,    -1,    -1,    -1,    -1,
     147,   148,   149,    -1,   151,   152,   153,    -1,    -1,    -1,
     157,    -1,    -1,   160,   161,    -1,    -1,    -1,   165,   166,
     167,   168,   169,   170,   171,   172,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   180,   181,   182,    -1,    -1,    -1,   186,
     187,   188,   189,    -1,    -1,    -1,    -1,   194,    -1,   196,
      -1,   198,   199,   200,   201,   202,   203,   204,   205,   206,
     207,   208,    -1,    -1,   211,   212,    -1,    -1,   215,   216,
     217,   218,   219,   220,   221,   222,   223,   224,   225,   226,
     227,   228,   229,   230,   231,   232,   233,   234,   235,   236,
     237,   238,   239,   240,   241,   242,   243,   244,   245,   246,
     247,   248,   249,   250,   251,   252,   253,   254,   255,   256,
     257,    -1,   259,   260,   261,    -1,   263,   264,   265,   266,
     267,   268,   269,   270,   271,   272,   273,   274,   275,   276,
     277,   278,   279,   280,   281,   282,   283,   284,   285,   286,
     287,   288,   289,   290,   291,   292,   293,   294,   295,   296,
     297,   298,   299,   300,   301,   302,   303,   304,   305,   306,
     307,   308,   309,    -1,    -1,    -1,    -1,   314,   315,   316,
     317,   318,   319,   320,    16,    -1,    18,    19,    -1,    -1,
      -1,    -1,    -1,    25,    26,    -1,    -1,    -1,    -1,    -1,
      -1,    33,    34,    -1,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    -1,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    -1,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    -1,    95,    96,    97,    98,    -1,    -1,   101,
     102,   103,    -1,    -1,   106,   107,    -1,    -1,    -1,    -1,
     112,   113,   114,   115,   116,   117,   118,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   127,   128,   129,   130,    -1,
     132,    -1,    -1,    -1,   136,   137,   138,   139,   140,    -1,
      -1,    -1,    -1,    -1,    -1,   147,   148,   149,    -1,   151,
     152,   153,    -1,    -1,    -1,    -1,    -1,    -1,   160,   161,
      -1,    -1,    -1,   165,   166,   167,   168,   169,   170,   171,
     172,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   180,   181,
     182,    -1,    -1,    -1,   186,    -1,   188,   189,    -1,    -1,
      -1,    -1,    -1,    -1,   196,    -1,   198,   199,   200,   201,
     202,   203,   204,   205,   206,   207,   208,    -1,    -1,   211,
     212,    -1,    -1,   215,   216,   217,   218,   219,   220,   221,
     222,   223,   224,   225,   226,   227,   228,   229,   230,   231,
     232,   233,   234,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,   245,   246,   247,   248,   249,   250,   251,
     252,   253,   254,   255,   256,   257,    -1,   259,   260,   261,
      -1,   263,   264,   265,   266,   267,   268,   269,   270,   271,
     272,   273,   274,   275,   276,   277,   278,   279,   280,   281,
     282,   283,   284,   285,   286,   287,   288,   289,   290,   291,
     292,   293,   294,   295,   296,   297,   298,   299,   300,   301,
     302,   303,   304,   305,   306,   307,   308,   309,    -1,    -1,
      16,    -1,   314,   315,   316,   317,   318,   319,   320,    25,
      26,    -1,    -1,    -1,    -1,    -1,    -1,    33,    34,    -1,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    -1,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    -1,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    -1,    95,
      96,    97,    98,    -1,    -1,   101,   102,   103,    -1,    -1,
     106,   107,    -1,    -1,    -1,    -1,   112,   113,   114,   115,
     116,   117,   118,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   127,   128,   129,   130,    -1,   132,    -1,    -1,    -1,
     136,   137,   138,   139,   140,    -1,    -1,    -1,    -1,    -1,
      -1,   147,   148,   149,    -1,   151,   152,   153,    -1,    -1,
      -1,    -1,    -1,    -1,   160,   161,    -1,    -1,    -1,   165,
     166,   167,   168,   169,   170,   171,   172,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   180,   181,   182,    -1,    -1,    -1,
     186,   187,   188,   189,    -1,    -1,    -1,    -1,   194,    -1,
     196,    -1,   198,   199,   200,   201,   202,   203,   204,   205,
     206,   207,   208,    -1,    -1,   211,   212,    -1,    -1,   215,
     216,   217,   218,   219,   220,   221,   222,   223,   224,   225,
     226,   227,   228,   229,   230,   231,   232,   233,   234,   235,
     236,   237,   238,   239,   240,   241,   242,   243,   244,   245,
     246,   247,   248,   249,   250,   251,   252,   253,   254,   255,
     256,   257,    -1,   259,   260,   261,    -1,   263,   264,   265,
     266,   267,   268,   269,   270,   271,   272,   273,   274,   275,
     276,   277,   278,   279,   280,   281,   282,   283,   284,   285,
     286,   287,   288,   289,   290,   291,   292,   293,   294,   295,
     296,   297,   298,   299,   300,   301,   302,   303,   304,   305,
     306,   307,   308,   309,    -1,    -1,    16,    -1,   314,   315,
     316,   317,   318,   319,   320,    25,    26,    -1,    -1,    -1,
      30,    -1,    -1,    33,    34,    -1,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    -1,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    -1,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    -1,    95,    96,    97,    98,    -1,
      -1,   101,   102,   103,    -1,    -1,   106,   107,    -1,    -1,
      -1,    -1,   112,   113,   114,   115,   116,   117,   118,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   127,   128,   129,
     130,    -1,   132,    -1,    -1,    -1,   136,   137,   138,   139,
     140,    -1,    -1,    -1,    -1,    -1,    -1,   147,   148,   149,
      -1,   151,   152,   153,    -1,    -1,    -1,    -1,    -1,    -1,
     160,   161,    -1,    -1,    -1,   165,   166,   167,   168,   169,
     170,   171,   172,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     180,   181,   182,    -1,    -1,    -1,   186,   187,   188,   189,
      -1,    -1,    -1,    -1,    -1,    -1,   196,    -1,   198,   199,
     200,   201,   202,   203,   204,   205,   206,   207,   208,    -1,
      -1,   211,   212,    -1,    -1,   215,   216,   217,   218,   219,
     220,   221,   222,   223,   224,   225,   226,   227,   228,   229,
     230,   231,   232,   233,   234,   235,   236,   237,   238,   239,
     240,   241,   242,   243,   244,   245,   246,   247,   248,   249,
     250,   251,   252,   253,   254,   255,   256,   257,    -1,   259,
     260,   261,    -1,   263,   264,   265,   266,   267,   268,   269,
     270,   271,   272,   273,   274,   275,   276,   277,   278,   279,
     280,   281,   282,   283,   284,   285,   286,   287,   288,   289,
     290,   291,   292,   293,   294,   295,   296,   297,   298,   299,
     300,   301,   302,   303,   304,   305,   306,   307,   308,   309,
      -1,    -1,    16,    -1,   314,   315,   316,   317,   318,   319,
     320,    25,    26,    -1,    -1,    -1,    -1,    -1,    -1,    33,
      34,    -1,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    -1,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    -1,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      -1,    95,    96,    97,    98,    -1,    -1,   101,   102,   103,
      -1,    -1,   106,   107,    -1,    -1,    -1,    -1,   112,   113,
     114,   115,   116,   117,   118,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   127,   128,   129,   130,    -1,   132,    -1,
      -1,    -1,   136,   137,   138,   139,   140,    -1,    -1,    -1,
      -1,    -1,    -1,   147,   148,   149,    -1,   151,   152,   153,
      -1,    -1,    -1,    -1,    -1,    -1,   160,   161,    -1,    -1,
      -1,   165,   166,   167,   168,   169,   170,   171,   172,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   180,   181,   182,    -1,
      -1,    -1,   186,   187,   188,   189,    -1,    -1,    -1,    -1,
     194,    -1,   196,    -1,   198,   199,   200,   201,   202,   203,
     204,   205,   206,   207,   208,    -1,    -1,   211,   212,    -1,
      -1,   215,   216,   217,   218,   219,   220,   221,   222,   223,
     224,   225,   226,   227,   228,   229,   230,   231,   232,   233,
     234,   235,   236,   237,   238,   239,   240,   241,   242,   243,
     244,   245,   246,   247,   248,   249,   250,   251,   252,   253,
     254,   255,   256,   257,    -1,   259,   260,   261,    -1,   263,
     264,   265,   266,   267,   268,   269,   270,   271,   272,   273,
     274,   275,   276,   277,   278,   279,   280,   281,   282,   283,
     284,   285,   286,   287,   288,   289,   290,   291,   292,   293,
     294,   295,   296,   297,   298,   299,   300,   301,   302,   303,
     304,   305,   306,   307,   308,   309,    -1,    -1,    16,    -1,
     314,   315,   316,   317,   318,   319,   320,    25,    26,    -1,
      -1,    -1,    -1,    -1,    -1,    33,    34,    -1,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    -1,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    -1,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    -1,    95,    96,    97,
      98,    -1,    -1,   101,   102,   103,    -1,    -1,   106,   107,
      -1,    -1,    -1,    -1,   112,   113,   114,   115,   116,   117,
     118,   119,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   127,
     128,   129,   130,    -1,   132,    -1,    -1,    -1,   136,   137,
     138,   139,   140,    -1,    -1,    -1,    -1,    -1,    -1,   147,
     148,   149,    -1,   151,   152,   153,    -1,    -1,    -1,   157,
      -1,    -1,   160,   161,    -1,    -1,    -1,   165,   166,   167,
     168,   169,   170,   171,   172,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   180,   181,   182,    -1,    -1,    -1,   186,    -1,
     188,   189,    -1,    -1,    -1,    -1,    -1,    -1,   196,    -1,
     198,   199,   200,   201,   202,   203,   204,   205,   206,   207,
     208,    -1,    -1,   211,   212,    -1,    -1,   215,   216,   217,
     218,   219,   220,   221,   222,   223,   224,   225,   226,   227,
     228,   229,   230,   231,   232,   233,   234,   235,   236,   237,
     238,   239,   240,   241,   242,   243,   244,   245,   246,   247,
     248,   249,   250,   251,   252,   253,   254,   255,   256,   257,
      -1,   259,   260,   261,    -1,   263,   264,   265,   266,   267,
     268,   269,   270,   271,   272,   273,   274,   275,   276,   277,
     278,   279,   280,   281,   282,   283,   284,   285,   286,   287,
     288,   289,   290,   291,   292,   293,   294,   295,   296,   297,
     298,   299,   300,   301,   302,   303,   304,   305,   306,   307,
     308,   309,    -1,    -1,    16,    -1,   314,   315,   316,   317,
     318,   319,   320,    25,    26,    -1,    -1,    -1,    -1,    -1,
      -1,    33,    34,    -1,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    -1,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    -1,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    -1,    95,    96,    97,    98,    -1,    -1,   101,
     102,   103,    -1,    -1,   106,   107,    -1,    -1,    -1,    -1,
     112,   113,   114,   115,   116,   117,   118,   119,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   127,   128,   129,   130,    -1,
     132,    -1,    -1,    -1,   136,   137,   138,   139,   140,    -1,
      -1,    -1,    -1,    -1,    -1,   147,   148,   149,    -1,   151,
     152,   153,    -1,    -1,    -1,   157,    -1,    -1,   160,   161,
      -1,    -1,    -1,   165,   166,   167,   168,   169,   170,   171,
     172,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   180,   181,
     182,    -1,    -1,    -1,   186,    -1,   188,   189,    -1,    -1,
      -1,    -1,    -1,    -1,   196,    -1,   198,   199,   200,   201,
     202,   203,   204,   205,   206,   207,   208,    -1,    -1,   211,
     212,    -1,    -1,   215,   216,   217,   218,   219,   220,   221,
     222,   223,   224,   225,   226,   227,   228,   229,   230,   231,
     232,   233,   234,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,   245,   246,   247,   248,   249,   250,   251,
     252,   253,   254,   255,   256,   257,    -1,   259,   260,   261,
      -1,   263,   264,   265,   266,   267,   268,   269,   270,   271,
     272,   273,   274,   275,   276,   277,   278,   279,   280,   281,
     282,   283,   284,   285,   286,   287,   288,   289,   290,   291,
     292,   293,   294,   295,   296,   297,   298,   299,   300,   301,
     302,   303,   304,   305,   306,   307,   308,   309,    -1,    -1,
      16,    -1,   314,   315,   316,   317,   318,   319,   320,    25,
      26,    -1,    -1,    -1,    -1,    -1,    -1,    33,    34,    -1,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    -1,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    -1,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    -1,    95,
      96,    97,    98,    -1,    -1,   101,   102,   103,    -1,    -1,
     106,   107,    -1,    -1,    -1,    -1,   112,   113,   114,   115,
     116,   117,   118,   119,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   127,   128,   129,   130,    -1,   132,    -1,    -1,    -1,
     136,   137,   138,   139,   140,    -1,    -1,    -1,    -1,    -1,
      -1,   147,   148,   149,    -1,   151,   152,   153,    -1,    -1,
      -1,   157,    -1,    -1,   160,   161,    -1,    -1,    -1,   165,
     166,   167,   168,   169,   170,   171,   172,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   180,   181,   182,    -1,    -1,    -1,
     186,    -1,   188,   189,    -1,    -1,    -1,    -1,    -1,    -1,
     196,    -1,   198,   199,   200,   201,   202,   203,   204,   205,
     206,   207,   208,    -1,    -1,   211,   212,    -1,    -1,   215,
     216,   217,   218,   219,   220,   221,   222,   223,   224,   225,
     226,   227,   228,   229,   230,   231,   232,   233,   234,   235,
     236,   237,   238,   239,   240,   241,   242,   243,   244,   245,
     246,   247,   248,   249,   250,   251,   252,   253,   254,   255,
     256,   257,    -1,   259,   260,   261,    -1,   263,   264,   265,
     266,   267,   268,   269,   270,   271,   272,   273,   274,   275,
     276,   277,   278,   279,   280,   281,   282,   283,   284,   285,
     286,   287,   288,   289,   290,   291,   292,   293,   294,   295,
     296,   297,   298,   299,   300,   301,   302,   303,   304,   305,
     306,   307,   308,   309,    -1,    -1,    16,    -1,   314,   315,
     316,   317,   318,   319,   320,    25,    26,    -1,    -1,    -1,
      -1,    -1,    -1,    33,    34,    -1,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    -1,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    -1,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    -1,    95,    96,    97,    98,    -1,
      -1,   101,   102,   103,    -1,    -1,   106,   107,    -1,    -1,
      -1,    -1,   112,   113,   114,   115,   116,   117,   118,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   127,   128,   129,
     130,    -1,   132,    -1,    -1,    -1,   136,   137,   138,   139,
     140,    -1,    -1,    -1,    -1,    -1,    -1,   147,   148,   149,
      -1,   151,   152,   153,    -1,    -1,    -1,   157,    -1,    -1,
     160,   161,    -1,    -1,    -1,   165,   166,   167,   168,   169,
     170,   171,   172,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     180,   181,   182,    -1,    -1,    -1,   186,    -1,   188,   189,
      -1,    -1,    -1,    -1,    -1,    -1,   196,    -1,   198,   199,
     200,   201,   202,   203,   204,   205,   206,   207,   208,    -1,
      -1,   211,   212,    -1,    -1,   215,   216,   217,   218,   219,
     220,   221,   222,   223,   224,   225,   226,   227,   228,   229,
     230,   231,   232,   233,   234,   235,   236,   237,   238,   239,
     240,   241,   242,   243,   244,   245,   246,   247,   248,   249,
     250,   251,   252,   253,   254,   255,   256,   257,    -1,   259,
     260,   261,    -1,   263,   264,   265,   266,   267,   268,   269,
     270,   271,   272,   273,   274,   275,   276,   277,   278,   279,
     280,   281,   282,   283,   284,   285,   286,   287,   288,   289,
     290,   291,   292,   293,   294,   295,   296,   297,   298,   299,
     300,   301,   302,   303,   304,   305,   306,   307,   308,   309,
      -1,    -1,    16,    -1,   314,   315,   316,   317,   318,   319,
     320,    25,    26,    -1,    -1,    -1,    -1,    -1,    -1,    33,
      34,    -1,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    -1,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    -1,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      -1,    95,    96,    97,    98,    -1,    -1,   101,   102,   103,
      -1,    -1,   106,   107,    -1,    -1,    -1,    -1,   112,   113,
     114,   115,   116,   117,   118,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   127,   128,   129,   130,    -1,   132,    -1,
      -1,    -1,   136,   137,   138,   139,   140,    -1,    -1,    -1,
      -1,    -1,    -1,   147,   148,   149,    -1,   151,   152,   153,
      -1,    -1,    -1,   157,    -1,    -1,   160,   161,    -1,    -1,
      -1,   165,   166,   167,   168,   169,   170,   171,   172,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   180,   181,   182,    -1,
      -1,    -1,   186,    -1,   188,   189,    -1,    -1,    -1,    -1,
      -1,    -1,   196,    -1,   198,   199,   200,   201,   202,   203,
     204,   205,   206,   207,   208,    -1,    -1,   211,   212,    -1,
      -1,   215,   216,   217,   218,   219,   220,   221,   222,   223,
     224,   225,   226,   227,   228,   229,   230,   231,   232,   233,
     234,   235,   236,   237,   238,   239,   240,   241,   242,   243,
     244,   245,   246,   247,   248,   249,   250,   251,   252,   253,
     254,   255,   256,   257,    -1,   259,   260,   261,    -1,   263,
     264,   265,   266,   267,   268,   269,   270,   271,   272,   273,
     274,   275,   276,   277,   278,   279,   280,   281,   282,   283,
     284,   285,   286,   287,   288,   289,   290,   291,   292,   293,
     294,   295,   296,   297,   298,   299,   300,   301,   302,   303,
     304,   305,   306,   307,   308,   309,    -1,    -1,    16,    -1,
     314,   315,   316,   317,   318,   319,   320,    25,    26,    -1,
      -1,    -1,    -1,    -1,    -1,    33,    34,    -1,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    -1,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    -1,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    -1,    95,    96,    97,
      98,    -1,    -1,   101,   102,   103,    -1,    -1,   106,   107,
      -1,    -1,    -1,    -1,   112,   113,   114,   115,   116,   117,
     118,   119,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   127,
     128,   129,   130,    -1,   132,    -1,    -1,    -1,   136,   137,
     138,   139,   140,    -1,    -1,    -1,    -1,    -1,    -1,   147,
     148,   149,    -1,   151,   152,   153,    -1,    -1,    -1,    -1,
      -1,    -1,   160,   161,    -1,    -1,    -1,   165,   166,   167,
     168,   169,   170,   171,   172,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   180,   181,   182,    -1,    -1,    -1,   186,    -1,
     188,   189,    -1,    -1,    -1,    -1,    -1,    -1,   196,    -1,
     198,   199,   200,   201,   202,   203,   204,   205,   206,   207,
     208,    -1,    -1,   211,   212,    -1,    -1,   215,   216,   217,
     218,   219,   220,   221,   222,   223,   224,   225,   226,   227,
     228,   229,   230,   231,   232,   233,   234,   235,   236,   237,
     238,   239,   240,   241,   242,   243,   244,   245,   246,   247,
     248,   249,   250,   251,   252,   253,   254,   255,   256,   257,
      -1,   259,   260,   261,    -1,   263,   264,   265,   266,   267,
     268,   269,   270,   271,   272,   273,   274,   275,   276,   277,
     278,   279,   280,   281,   282,   283,   284,   285,   286,   287,
     288,   289,   290,   291,   292,   293,   294,   295,   296,   297,
     298,   299,   300,   301,   302,   303,   304,   305,   306,   307,
     308,   309,    -1,    -1,    16,    -1,   314,   315,   316,   317,
     318,   319,   320,    25,    26,    -1,    -1,    -1,    -1,    -1,
      -1,    33,    34,    -1,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    -1,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    -1,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    -1,    95,    96,    97,    98,    -1,    -1,   101,
     102,   103,    -1,    -1,   106,   107,    -1,    -1,    -1,    -1,
     112,   113,   114,   115,   116,   117,   118,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   127,   128,   129,   130,    -1,
     132,    -1,    -1,    -1,   136,   137,   138,   139,   140,    -1,
      -1,    -1,    -1,    -1,    -1,   147,   148,   149,    -1,   151,
     152,   153,    -1,    -1,    -1,    -1,    -1,    -1,   160,   161,
      -1,    -1,    -1,   165,   166,   167,   168,   169,   170,   171,
     172,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   180,   181,
     182,    -1,    -1,    -1,   186,    -1,   188,   189,    -1,    -1,
      -1,    -1,    -1,    -1,   196,    -1,   198,   199,   200,   201,
     202,   203,   204,   205,   206,   207,   208,    -1,    -1,   211,
     212,    -1,    -1,   215,   216,   217,   218,   219,   220,   221,
     222,   223,   224,   225,   226,   227,   228,   229,   230,   231,
     232,   233,   234,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,   245,   246,   247,   248,   249,   250,   251,
     252,   253,   254,   255,   256,   257,    -1,   259,   260,   261,
      -1,   263,   264,   265,   266,   267,   268,   269,   270,   271,
     272,   273,   274,   275,   276,   277,   278,   279,   280,   281,
     282,   283,   284,   285,   286,   287,   288,   289,   290,   291,
     292,   293,   294,   295,   296,   297,   298,   299,   300,   301,
     302,   303,   304,   305,   306,   307,   308,   309,    -1,    -1,
      -1,    -1,   314,   315,   316,   317,   318,   319,   320,    21,
      22,    23,    24,    25,    26,    -1,    -1,    -1,    30,    -1,
      -1,    33,    34,    -1,    36,    37,    38,    39,    40,    41,
      42,    43,    -1,    45,    46,    47,    -1,    49,    50,    51,
      52,    -1,    54,    -1,    56,    57,    58,    59,    60,    61,
      -1,    -1,    64,    65,    66,    67,    68,    -1,    70,    71,
      72,    -1,    -1,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    -1,    95,    96,    97,    98,    -1,    -1,   101,
     102,   103,    -1,    -1,   106,   107,    -1,    -1,    -1,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,    -1,
      -1,    -1,    -1,   125,    -1,   127,   128,   129,   130,    -1,
     132,    -1,    -1,    -1,   136,   137,   138,   139,   140,    -1,
      -1,    -1,    -1,    -1,    -1,   147,   148,   149,   150,   151,
     152,    -1,   154,   155,    -1,   157,   158,    -1,   160,   161,
      -1,    -1,    -1,   165,   166,   167,   168,   169,   170,   171,
     172,    -1,   174,    -1,    -1,    -1,    -1,    -1,   180,   181,
     182,    -1,    -1,    -1,   186,    -1,   188,   189,    -1,    -1,
      -1,    -1,    -1,    -1,   196,    -1,   198,   199,   200,   201,
     202,   203,   204,   205,   206,   207,   208,    -1,    -1,    -1,
     212,   213,    -1,   215,   216,   217,   218,   219,   220,   221,
     222,   223,   224,    -1,   226,   227,   228,   229,   230,   231,
     232,   233,   234,   235,   236,   237,   238,   239,   240,   241,
     242,    -1,   244,   245,   246,   247,   248,   249,   250,   251,
     252,   253,   254,   255,   256,   257,    -1,   259,   260,   261,
      -1,   263,   264,   265,   266,   267,   268,   269,   270,   271,
     272,   273,   274,   275,   276,   277,   278,   279,   280,   281,
     282,   283,   284,   285,   286,   287,   288,   289,   290,   291,
     292,   293,   294,   295,   296,   297,   298,   299,   300,   301,
     302,   303,   304,   305,   306,   307,   308,   309,   310,    -1,
     312,    25,    26,   315,   316,    -1,    -1,    -1,    -1,    33,
      34,    -1,    36,    37,    38,    -1,    40,    41,    42,    43,
      -1,    -1,    46,    47,    -1,    49,    50,    51,    52,    -1,
      54,    -1,    56,    57,    58,    59,    -1,    61,    -1,    -1,
      64,    65,    66,    67,    68,    -1,    70,    -1,    72,    -1,
      -1,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      -1,    95,    96,    97,    -1,    -1,    -1,   101,   102,   103,
      -1,    -1,   106,   107,    -1,    -1,    -1,    -1,   112,   113,
     114,   115,   116,   117,   118,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   127,   128,   129,   130,    -1,   132,    -1,
      -1,    -1,   136,   137,   138,   139,   140,    -1,    -1,    -1,
      -1,    -1,    -1,   147,   148,   149,    -1,   151,   152,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   160,   161,    -1,    -1,
      -1,   165,   166,   167,   168,   169,   170,   171,   172,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   180,   181,   182,    -1,
      -1,    -1,   186,    -1,   188,   189,    -1,    -1,    -1,    -1,
      -1,    -1,   196,    -1,   198,   199,   200,   201,   202,   203,
     204,   205,   206,   207,   208,    -1,    -1,    -1,   212,    -1,
      -1,   215,   216,   217,   218,   219,   220,   221,   222,   223,
     224,    -1,   226,   227,   228,   229,   230,   231,   232,   233,
     234,   235,   236,   237,   238,   239,   240,   241,   242,    -1,
     244,   245,   246,   247,   248,   249,   250,   251,   252,   253,
     254,   255,   256,   257,    -1,   259,   260,   261,    -1,   263,
     264,   265,   266,   267,   268,   269,   270,   271,   272,   273,
     274,   275,   276,   277,   278,   279,   280,   281,   282,   283,
     284,   285,   286,   287,   288,   289,   290,   291,   292,   293,
     294,   295,   296,   297,   298,   299,   300,   301,   302,   303,
     304,   305,   306,   307,   308,   309,    -1,    -1,    -1,    -1,
      -1,   315,   316
  };

  const short
  xquery_parser::yystos_[] =
  {
       0,    11,    12,    16,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    30,    33,    34,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    95,    96,    97,    98,    99,   101,   102,   103,
     106,   107,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   125,   127,   128,   129,   130,   132,   136,
     137,   138,   139,   140,   147,   148,   149,   150,   151,   152,
     153,   154,   155,   157,   158,   159,   160,   161,   165,   166,
     167,   168,   169,   170,   171,   172,   174,   176,   177,   180,
     181,   182,   186,   188,   189,   190,   191,   192,   194,   196,
     198,   199,   200,   201,   202,   203,   204,   205,   206,   207,
     208,   211,   212,   213,   215,   216,   217,   218,   219,   220,
     221,   222,   223,   224,   225,   226,   227,   228,   229,   230,
     231,   232,   233,   234,   235,   236,   237,   238,   239,   240,
     241,   242,   243,   244,   245,   246,   247,   248,   249,   250,
     251,   252,   253,   254,   255,   256,   257,   259,   260,   261,
     263,   264,   265,   266,   267,   268,   269,   270,   271,   272,
     273,   274,   275,   276,   277,   278,   279,   280,   281,   282,
     283,   284,   285,   286,   287,   288,   289,   290,   291,   292,
     293,   294,   295,   296,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,   312,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   335,   337,
     338,   339,   340,   341,   342,   343,   344,   345,   346,   347,
     348,   349,   350,   351,   352,   353,   356,   357,   358,   359,
     360,   361,   365,   366,   367,   371,   373,   374,   376,   383,
     385,   388,   389,   390,   392,   393,   394,   395,   396,   398,
     399,   401,   402,   403,   404,   405,   406,   408,   409,   412,
     413,   414,   415,   416,   422,   424,   426,   427,   428,   433,
     452,   455,   459,   462,   463,   469,   470,   471,   472,   474,
     475,   477,   478,   479,   480,   481,   482,   483,   484,   485,
     487,   488,   489,   490,   493,   494,   495,   496,   497,   498,
     499,   500,   501,   502,   503,   504,   505,   506,   507,   508,
     509,   510,   513,   514,   515,   516,   517,   518,   519,   520,
     521,   523,   524,   525,   526,   527,   528,   542,   543,   545,
     546,   547,   548,   549,   550,   551,   552,   560,   561,   562,
     563,   564,   565,   566,   567,   568,   569,   570,   571,   574,
     579,   580,   581,   582,   583,   584,   587,   645,   646,   647,
     648,   650,   651,   652,   653,   654,   657,   658,   659,    33,
      34,    49,   215,   391,   392,   393,   391,   391,   392,   393,
     157,   157,    37,    38,    40,    41,    47,    57,    58,    75,
      80,   112,   113,   114,   161,   220,   261,   287,   295,   303,
     372,   373,   377,   378,   379,   161,   154,   157,   154,   157,
     154,   157,     1,   157,   380,   157,    25,    34,    61,   119,
     273,   432,   434,   435,   157,   154,   157,   157,   157,   119,
     154,   157,   157,   157,    77,   154,   221,   222,   123,   123,
     123,   154,   157,    39,    43,    45,    46,    47,    48,    51,
      60,    68,    71,    73,    74,    76,    91,    92,    98,   106,
     113,   115,   116,   136,   139,   140,   161,   169,   172,   180,
     181,   189,   202,   224,   226,   228,   229,   236,   238,   239,
     240,   243,   244,   315,   507,   658,   123,   119,   400,   123,
     123,    39,    44,    45,    53,    60,    62,    63,    71,    98,
     225,   657,   119,   123,   123,   184,   391,   393,   414,   649,
      48,    73,    74,   119,   154,   185,   244,   413,   415,   426,
     187,   413,   657,   154,   154,   123,   657,    18,    19,   657,
     123,   123,   123,   499,   154,    31,   214,   216,   225,   227,
     314,   225,   227,   314,   225,   314,   225,   230,   119,   241,
     241,   242,   157,   154,   395,   311,   413,   313,   413,   314,
     322,   337,   337,     0,   339,   340,    33,    49,   342,   359,
       1,   190,   336,   190,   336,   113,   374,   394,   413,   108,
     190,   108,   336,   190,    42,    46,    51,    70,   168,   171,
     186,   212,   407,   417,   418,   423,   424,   425,   440,   441,
     445,     3,    67,    72,   119,   419,   167,    93,   133,   141,
     142,   144,   152,   156,   158,   163,   179,   203,   204,   205,
     206,   207,   208,   491,   492,   249,   100,   159,   176,   199,
     118,   147,   160,   194,   201,   210,   137,   151,    50,   200,
     102,   103,   159,   176,   489,   193,   154,   496,   499,   191,
     192,   155,   511,   512,   507,   511,   507,   157,   511,   157,
     146,   157,   184,   184,   184,   184,   375,   514,   375,    30,
     656,   182,   196,   182,   196,   166,   182,   658,   657,   169,
     202,    47,   657,   153,   119,    45,    47,    80,   107,   168,
     657,   221,   222,   223,   246,   618,   657,   657,   304,   138,
     143,   113,   287,   295,   377,   657,   392,   187,   392,    45,
      63,   187,   570,   571,   413,   187,   194,   657,   429,   430,
     657,   119,   187,   381,   382,   154,   397,   413,     1,   161,
     656,   114,   161,   355,   656,   657,   119,   143,   108,   187,
     413,    30,   187,   657,   657,   657,   453,   454,   657,   392,
     187,   413,   413,   572,   657,   392,   154,   154,   413,   187,
     194,   657,   657,   143,   453,   184,   184,   122,   108,   184,
     157,   157,   157,   657,   154,   185,   418,   187,    35,   530,
     531,   532,   413,   413,     8,   175,    17,   413,   214,    30,
     414,   414,    39,    45,    60,    71,    98,   510,   658,   414,
     414,   414,   649,   414,   510,   414,   231,   585,   586,   657,
     190,   190,   414,   413,   393,   413,   245,   410,   411,   311,
     313,   414,   337,   190,   336,   190,   336,     3,   343,   359,
     389,     1,   343,   359,   389,    33,   360,   389,   360,   389,
     400,   336,   400,   414,   414,   119,   168,   170,   170,   414,
     394,   414,   284,   284,   429,   119,   436,   471,   472,   474,
     474,   474,   474,   473,   474,   474,   474,    71,   475,   479,
     479,   478,   480,   480,   480,   480,   481,   481,   482,   482,
     231,    95,    95,    95,   497,   391,   499,   499,   413,   512,
     145,   187,   414,   522,   187,   150,   187,   522,   108,   187,
     187,   108,   108,   380,    30,   658,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,   362,   363,   364,    95,
     138,   143,   368,   369,   370,   657,   161,   161,   362,   656,
     128,   133,    55,    57,   101,   251,   264,   266,   277,   278,
     279,   281,   283,   619,   620,   621,   622,   623,   624,   631,
     637,   638,   246,    95,   299,   657,   143,   414,   119,   657,
     657,   133,   184,   184,   187,   187,   184,   108,   187,   108,
     187,   108,    36,    95,    97,   148,   431,   432,   553,   657,
      95,   108,   187,   391,   187,   657,    97,    45,   657,   656,
      97,   143,   553,   657,   414,   435,   184,   187,   187,   187,
     187,   108,   188,   148,   553,   184,   187,   187,   154,   184,
     392,   392,   184,   108,   187,   108,   187,   143,   553,   414,
     188,   414,   414,   413,   413,   413,   657,   531,   532,   131,
     197,   184,   184,   184,   132,   190,    95,   218,   219,   233,
      95,   218,   219,   233,   233,   233,    95,    95,   237,   225,
     314,   108,   235,   143,   190,   187,   413,   184,   508,   591,
     411,   233,   359,    33,    33,   190,   336,   190,   114,   394,
     657,   170,   414,   446,   447,   119,   414,   442,   443,   657,
      56,   217,   254,   420,   421,   474,   154,   157,   259,   495,
     514,   592,   595,   596,   597,   598,   599,   603,   605,   607,
     608,    47,   153,   157,   211,   317,   318,   319,   320,   554,
     556,   558,   560,   575,   576,   577,   578,   655,   657,   554,
     486,   559,   657,   486,   184,   185,   108,   187,   187,   514,
     149,   165,   149,   165,   138,   397,   380,   363,   133,   556,
     370,   414,   553,   656,   656,   129,   130,   656,   277,   278,
     279,   283,   657,   263,   274,   263,   274,    30,   286,    97,
     114,   157,   625,   628,   619,    39,    44,    53,    60,    62,
      71,    98,   225,   317,   318,   319,   384,   560,   655,   227,
     299,   308,   414,   657,    95,   299,   656,   154,   572,   573,
     657,   572,   573,   119,   430,   128,   554,   119,   414,   148,
     432,   148,    36,   148,   431,   432,   553,   554,   382,    95,
     184,   198,   133,   354,   656,   161,   133,    97,   354,   414,
     143,   432,   154,   119,   414,   414,   148,   101,   456,   457,
     458,   460,   461,   101,   464,   465,   466,   467,   392,   184,
     184,   154,   572,   572,   414,   143,   190,   414,   122,   187,
     187,   187,    35,   532,   131,   197,     9,    10,   104,   124,
     126,   154,   195,   527,   529,   540,   541,   544,   154,    30,
     232,   234,   414,   414,   414,   232,   234,   414,   414,   414,
     414,   414,   414,   414,   414,   414,   510,   119,   414,   414,
      48,    73,    74,   244,   394,   415,   426,   245,   588,   589,
     154,   210,   395,   414,   190,   114,   389,   389,   389,   446,
      96,   107,   117,   128,   448,   449,   450,   451,   108,   657,
     107,   444,   108,   148,   553,   254,    54,    59,    78,   119,
     431,   437,   438,   439,   421,   413,   592,   599,   154,   285,
     476,   644,    97,   169,   252,   253,   255,   262,   272,   284,
     593,   594,   613,   614,   615,   616,   639,   642,   258,   260,
     600,   618,   267,   604,   640,   247,   248,   270,   609,   610,
     157,   157,   556,   157,   157,   157,   157,   157,   145,   176,
     194,   555,   145,   145,   414,   138,   397,   574,   369,   286,
      30,    97,   114,   157,   632,    30,   625,   555,   555,   497,
     287,   306,   553,   384,   227,   190,   391,   187,   187,   145,
     187,   187,   430,   148,   431,   657,   414,   148,   414,   128,
     414,   148,   432,   148,   554,   394,   414,   656,   108,   354,
     414,   143,   391,   454,   414,   414,   114,   457,   458,   101,
     418,   114,   458,   461,   119,   468,   554,   101,   114,   465,
     101,   114,   467,   184,   391,   187,   187,   414,   414,   198,
     464,   133,   195,   529,     7,   392,   657,   195,   540,   392,
     190,   233,   233,   233,   233,    97,   237,   237,   586,   418,
     157,   157,   157,   418,   591,   589,   508,   656,   129,   130,
     450,   451,   451,   447,   143,   553,   656,   443,   414,   148,
     119,   119,   414,   657,   439,    78,   184,   187,   592,   606,
     250,   217,   254,   268,   275,   643,    97,   256,   257,   641,
     250,   596,   643,   478,   613,   597,   148,   282,   601,   602,
     641,   286,   612,    79,   611,   187,   194,   554,   557,   187,
     187,   187,   187,   187,   187,   187,    30,   137,   201,   634,
     635,   636,    30,   633,   634,   271,   629,   108,   626,   170,
     657,   257,   497,   184,   414,   148,   414,   148,   431,   414,
     148,   414,   127,   127,    97,   656,   414,   184,   418,   418,
     414,   394,   414,   418,   418,   657,   210,   418,   119,   468,
     119,   418,   119,   468,   418,   184,   114,   532,   657,   195,
     184,   532,   657,   184,   414,   414,   414,   414,   316,   414,
     414,   414,   413,   413,   413,   154,   590,   451,   414,   143,
     414,   657,   657,   438,   414,   184,   480,    52,   130,   478,
     478,   269,   276,   286,   617,   617,   598,   154,   280,    95,
     187,   108,   187,   632,   632,   636,   108,   187,    30,   630,
     641,   627,   628,   187,   386,   387,   497,   119,   225,   307,
     287,   170,   414,   414,   148,   414,   394,   414,   354,   414,
     394,    95,   554,   394,   414,   657,   418,   657,   414,   657,
     418,   394,   119,    94,   183,   533,   532,   657,   197,   532,
     414,   187,   187,   187,   413,   444,   414,    54,   478,   478,
     199,   413,   554,   554,    95,    30,   265,   108,   108,   451,
     553,   657,   119,   225,   657,   386,   414,   468,    95,   418,
      95,   657,     5,   134,   536,   537,   539,   541,    29,   135,
     534,   535,   538,   541,   197,   532,   197,   198,   464,   184,
     444,   119,   478,   184,   554,   628,   387,   451,   305,   657,
     119,   225,   418,   468,   394,   414,   468,   418,    94,   134,
     539,   183,   135,   538,   197,   114,   657,   414,   305,   657,
     119,   394,   414,   418,   418,   119,   293,   305,   657,   657,
     306,   414,   306,   418,   497,   497,   199,   287,   657,   225,
     119,   657,   306,   497
  };

  const short
  xquery_parser::yyr1_[] =
  {
       0,   334,   335,   335,   335,   335,   336,   336,   336,   337,
     337,   337,   337,   338,   338,   339,   339,   339,   339,   339,
     339,   339,   339,   340,   340,   340,   340,   341,   342,   342,
     342,   343,   343,   343,   343,   343,   344,   344,   344,   344,
     344,   344,   344,   344,   345,   345,   346,   347,   348,   348,
     349,   349,   350,   350,   351,   351,   351,   351,   352,   352,
     352,   353,   353,   353,   353,   354,   354,   355,   355,   356,
     356,   356,   356,   357,   358,   358,   359,   359,   359,   360,
     360,   360,   360,   360,   360,   360,   360,   361,   361,   362,
     362,   363,   364,   364,   364,   364,   364,   364,   364,   364,
     364,   364,   365,   366,   367,   368,   368,   369,   369,   369,
     370,   371,   371,   371,   372,   372,   372,   372,   373,   373,
     374,   374,   374,   374,   375,   375,   376,   376,   377,   377,
     378,   378,   379,   379,   380,   380,   380,   380,   381,   381,
     382,   382,   383,   383,   383,   383,   384,   384,   384,   384,
     385,   385,   386,   386,   387,   387,   387,   387,   388,   388,
     388,   388,   389,   390,   390,   390,   391,   391,   391,   392,
     392,   393,   393,   393,   394,   394,   394,   394,   394,   394,
     394,   394,   394,   394,   394,   394,   394,   395,   395,   396,
     397,   398,   399,   399,   399,   400,   400,   400,   400,   401,
     402,   403,   404,   405,   405,   406,   407,   408,   409,   410,
     410,   411,   412,   413,   413,   413,   414,   414,   414,   414,
     414,   414,   415,   415,   415,   415,   415,   415,   415,   415,
     415,   415,   415,   415,   416,   417,   418,   419,   419,   420,
     420,   420,   421,   421,   422,   422,   423,   424,   424,   424,
     425,   425,   425,   425,   425,   426,   426,   427,   427,   427,
     428,   429,   429,   429,   430,   430,   430,   430,   430,   430,
     430,   430,   430,   430,   430,   430,   431,   432,   433,   434,
     434,   435,   435,   435,   435,   436,   436,   437,   437,   437,
     438,   438,   438,   439,   439,   439,   440,   441,   442,   442,
     443,   443,   443,   443,   443,   443,   444,   445,   445,   446,
     446,   447,   447,   448,   448,   448,   448,   448,   448,   448,
     449,   449,   450,   450,   451,   452,   452,   453,   453,   454,
     454,   455,   456,   456,   457,   458,   458,   459,   460,   460,
     461,   462,   462,   463,   463,   464,   464,   465,   465,   466,
     466,   467,   467,   468,   468,   469,   470,   470,   471,   471,
     472,   472,   472,   472,   472,   473,   472,   472,   472,   472,
     474,   474,   475,   475,   476,   476,   477,   477,   478,   478,
     478,   479,   479,   479,   479,   479,   480,   480,   480,   481,
     481,   481,   482,   482,   483,   483,   484,   484,   485,   485,
     486,   486,   487,   487,   488,   488,   488,   488,   489,   489,
     489,   490,   490,   491,   491,   491,   491,   491,   491,   492,
     492,   492,   493,   493,   493,   493,   494,   495,   495,   496,
     496,   496,   497,   497,   497,   497,   498,   499,   499,   499,
     500,   500,   501,   501,   501,   501,   502,   502,   503,   503,
     503,   503,   503,   503,   503,   504,   504,   505,   505,   506,
     506,   506,   506,   506,   507,   507,   508,   508,   509,   509,
     509,   509,   510,   510,   510,   510,   511,   511,   512,   513,
     513,   513,   513,   513,   513,   513,   513,   513,   513,   513,
     513,   513,   513,   514,   514,   515,   515,   515,   516,   517,
     517,   518,   519,   520,   521,   521,   522,   522,   522,   522,
     523,   523,   524,   525,   526,   526,   527,   527,   527,   528,
     528,   528,   528,   528,   528,   529,   529,   530,   530,   531,
     532,   532,   533,   533,   534,   534,   535,   535,   535,   535,
     536,   536,   537,   537,   537,   537,   538,   538,   539,   539,
     540,   540,   540,   540,   541,   541,   541,   541,   542,   542,
     543,   543,   544,   545,   545,   545,   545,   545,   545,   545,
     546,   547,   547,   548,   548,   549,   550,   551,   551,   552,
     552,   553,   554,   554,   554,   555,   555,   555,   556,   556,
     556,   556,   556,   556,   556,   557,   557,   558,   559,   560,
     560,   560,   560,   560,   560,   560,   560,   560,   560,   561,
     562,   563,   563,   563,   564,   565,   566,   567,   567,   567,
     568,   568,   568,   568,   568,   569,   570,   570,   570,   570,
     570,   570,   570,   571,   572,   573,   574,   575,   575,   576,
     577,   577,   578,   579,   579,   579,   580,   580,   580,   580,
     580,   580,   580,   580,   580,   580,   581,   581,   582,   582,
     583,   584,   585,   585,   586,   587,   588,   588,   589,   590,
     591,   591,   592,   593,   593,   594,   594,   595,   595,   596,
     596,   597,   597,   598,   598,   599,   600,   600,   601,   601,
     602,   603,   603,   603,   604,   604,   605,   606,   606,   607,
     608,   608,   609,   609,   610,   610,   610,   611,   611,   612,
     612,   613,   613,   613,   613,   613,   614,   615,   616,   617,
     617,   617,   618,   618,   619,   619,   619,   619,   619,   619,
     619,   619,   620,   620,   620,   620,   621,   621,   622,   623,
     623,   624,   624,   624,   625,   625,   626,   626,   627,   627,
     628,   629,   629,   630,   630,   631,   631,   631,   632,   632,
     633,   633,   634,   634,   635,   635,   636,   636,   637,   638,
     638,   639,   639,   639,   640,   641,   641,   641,   641,   642,
     642,   643,   643,   644,   645,   645,   646,   646,   647,   647,
     648,   649,   649,   650,   650,   650,   651,   652,   653,   654,
     655,   655,   655,   656,   657,   657,   657,   657,   657,   657,
     657,   657,   657,   657,   657,   657,   657,   657,   657,   657,
     657,   657,   657,   657,   657,   657,   657,   658,   658,   658,
     658,   658,   658,   658,   658,   658,   658,   658,   658,   658,
     658,   658,   658,   658,   658,   658,   658,   658,   658,   658,
     658,   658,   658,   658,   658,   658,   658,   658,   658,   658,
     658,   658,   658,   658,   658,   658,   658,   658,   658,   658,
     658,   658,   658,   658,   658,   658,   658,   658,   658,   658,
     658,   658,   658,   658,   658,   658,   658,   658,   658,   658,
     658,   658,   658,   658,   658,   658,   658,   658,   658,   658,
     658,   658,   658,   658,   658,   658,   658,   658,   658,   658,
     658,   658,   658,   658,   658,   658,   658,   658,   658,   658,
     658,   658,   658,   658,   658,   658,   658,   658,   658,   658,
     658,   658,   658,   658,   658,   658,   658,   658,   658,   658,
     658,   658,   658,   658,   658,   658,   658,   658,   658,   658,
     658,   658,   658,   658,   658,   658,   658,   658,   658,   658,
     658,   658,   658,   658,   658,   658,   658,   658,   658,   658,
     658,   658,   658,   658,   658,   658,   658,   658,   658,   658,
     658,   658,   658,   658,   658,   658,   658,   658,   658,   658,
     658,   658,   658,   658,   658,   658,   658,   658,   658,   658,
     658,   658,   658,   658,   658,   658,   658,   658,   658,   658,
     658,   658,   658,   658,   658,   658,   658,   658,   658,   658,
     658,   658,   658,   658,   658,   658,   658,   658,   659
  };

  const signed char
  xquery_parser::yyr2_[] =
  {
       0,     2,     1,     2,     2,     3,     1,     2,     2,     1,
       2,     1,     2,     4,     6,     3,     3,     5,     1,     3,
       3,     5,     5,     1,     3,     3,     5,     6,     1,     3,
       3,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     3,     3,     4,     3,     3,     3,
       3,     3,     5,     5,     5,     5,     5,     5,     1,     1,
       3,     3,     4,     5,     6,     1,     3,     3,     3,     3,
       6,     5,     8,     5,     5,     5,     1,     3,     3,     1,
       1,     1,     1,     1,     1,     1,     1,     4,     4,     1,
       2,     3,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     4,     3,     4,     3,     1,     1,     1,     2,
       2,     4,     3,     5,     3,     4,     4,     5,     1,     2,
       1,     4,     1,     4,     1,     3,     2,     3,     1,     1,
       4,     4,     5,     5,     2,     3,     4,     5,     1,     3,
       2,     3,     3,     5,     4,     6,     1,     2,     1,     2,
       8,     9,     1,     3,     1,     2,     2,     3,    11,    14,
      13,    22,     1,     1,     1,     0,     1,     1,     0,     1,
       2,     1,     2,     4,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     3,     2,     3,
       3,     2,     3,     2,     3,     2,     3,     4,     5,     5,
       2,     4,     5,     3,     3,     2,     2,     8,     3,     1,
       2,     3,     1,     1,     3,     3,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     2,     2,     1,     2,     2,     1,
       1,     2,     4,     3,     5,     4,     3,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     2,     3,     3,     2,
       1,     1,     4,     3,     3,     5,     4,     6,     4,     6,
       5,     7,     4,     5,     5,     6,     3,     3,     2,     1,
       3,     4,     5,     3,     6,     4,     5,     1,     2,     3,
       1,     2,     1,     6,     3,     3,     2,     3,     1,     3,
       4,     5,     6,     5,     1,     2,     2,     3,     4,     1,
       3,     1,     2,     1,     1,     1,     2,     2,     2,     3,
       1,     1,     2,     2,     2,     5,     5,     1,     4,     3,
       4,     8,     1,     2,     3,     2,     3,     8,     1,     2,
       3,     8,    10,     8,    10,     1,     2,     4,     7,     1,
       2,     4,     7,     1,     3,     8,     1,     3,     1,     3,
       1,     3,     3,     3,     3,     0,     4,     3,     3,     3,
       1,     5,     1,     3,     0,     1,     1,     3,     1,     3,
       3,     1,     3,     3,     3,     3,     1,     3,     3,     1,
       3,     3,     1,     4,     1,     4,     1,     4,     1,     4,
       1,     2,     1,     2,     1,     1,     2,     2,     1,     1,
       1,     1,     3,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     4,     5,     5,     6,     4,     1,     2,     3,
       2,     2,     1,     2,     2,     1,     1,     1,     3,     3,
       1,     1,     1,     2,     1,     2,     2,     1,     2,     2,
       2,     2,     2,     2,     2,     1,     2,     2,     1,     2,
       2,     2,     2,     2,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     2,     3,     4,     1,     2,     3,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     2,     2,
       3,     1,     4,     4,     3,     4,     1,     3,     1,     3,
       1,     1,     3,     3,     1,     1,     1,     1,     1,     4,
       8,     5,     9,     9,    10,     1,     2,     1,     2,     6,
       0,     1,     3,     3,     0,     1,     1,     1,     2,     2,
       0,     1,     1,     1,     2,     2,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     3,     3,     2,
       3,     3,     2,     1,     1,     1,     1,     1,     1,     1,
       4,     3,     7,     3,     7,     4,     4,     3,     7,     3,
       7,     2,     1,     2,     3,     1,     1,     1,     1,     1,
       3,     3,     1,     1,     1,     1,     3,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     3,
       1,     3,     4,     4,     3,     3,     3,     3,     4,     4,
       3,     4,     6,     4,     6,     4,     3,     4,     6,     6,
       4,     6,     6,     4,     1,     2,     1,     1,     1,     4,
       5,     6,     3,     3,     3,     3,     5,     7,     7,     5,
       5,     5,     7,     7,     5,     5,     3,     3,     5,     7,
       5,     7,     1,     4,     3,     5,     1,     2,     3,     3,
       1,     3,     2,     0,     1,     1,     2,     1,     3,     1,
       3,     1,     4,     1,     2,     3,     0,     1,     0,     1,
       4,     2,     3,     1,     0,     1,     4,     0,     1,     2,
       1,     3,     0,     1,     2,     2,     1,     0,     1,     0,
       1,     1,     1,     1,     1,     1,     1,     3,     3,     1,
       1,     1,     2,     3,     1,     1,     1,     1,     1,     1,
       1,     1,     2,     2,     1,     1,     2,     2,     3,     1,
       2,     2,     5,     2,     1,     1,     0,     2,     1,     3,
       4,     0,     2,     0,     2,     4,     4,     3,     2,     3,
       1,     3,     0,     1,     1,     2,     2,     2,     2,     1,
       2,     2,     2,     2,     3,     2,     3,     3,     4,     2,
       2,     1,     1,     3,     2,     3,     2,     3,     2,     3,
       3,     3,     5,     5,     5,     8,     5,     3,     5,     7,
       3,     3,     3,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1
  };


#if YYDEBUG || 1
  // YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
  // First, the terminals, then, starting at \a YYNTOKENS, nonterminals.
  const char*
  const xquery_parser::yytname_[] =
  {
  "\"'end of file'\"", "error", "\"invalid token\"", "\"'unrecognized'\"",
  "\"'error'\"", "\"'apos attribute content'\"", "\"'char literal'\"",
  "\"'char literal]]>'\"", "\"'char literal and pi end'\"",
  "\"'&#charref;'\"", "\"'element content'\"", "\"'pref:*'\"",
  "\"'ns:*'\"", "\"'&entity;'\"", "\"'comment literal'\"",
  "\"'pi <NCName {>'\"", "\"'NCName_sval'\"", "\"'pragma literal'\"",
  "\"'QName #)'\"", "\"'EQName #)'\"", "\"'*:QName'\"",
  "\"'element QName {'\"", "\"'attribute QName {'\"",
  "\"'processing-instruction NCName {'\"", "\"'namespace NCName {'\"",
  "\"'QName'\"", "\"'EQName'\"", "\"'%QName'\"", "\"'%EQName'\"",
  "\"'quote attribute content'\"", "\"'STRING'\"", "\"'XML comment'\"",
  "\"'URI'\"", "\"'declare'\"", "\"'module'\"", "\"'<blank>'\"",
  "\"'allowing'\"", "\"'base-uri'\"", "\"'boundary-space'\"",
  "\"'comment'\"", "\"'construction'\"", "\"'copy-namespaces'\"",
  "\"'count'\"", "\"'document'\"", "\"'document-node'\"", "\"'element'\"",
  "\"'for'\"", "\"'function'\"", "\"'if'\"", "\"'import'\"",
  "\"'instance'\"", "\"'let'\"", "\"'most'\"", "\"'namespace-node'\"",
  "\"'next'\"", "\"'no'\"", "\"'only'\"", "\"'option'\"", "\"'ordering'\"",
  "\"'previous'\"", "\"'processing-instruction'\"", "\"'schema'\"",
  "\"'schema-attribute'\"", "\"'schema-element'\"", "\"'sequential'\"",
  "\"'set'\"", "\"'simple'\"", "\"'sliding'\"", "\"'some'\"",
  "\"'space'\"", "\"'stable'\"", "\"'text'\"", "\"'tumbling'\"",
  "\"'switch'\"", "\"'typeswitch'\"", "\"'updating'\"", "\"'validate'\"",
  "\"'type'\"", "\"'when'\"", "\"'word'\"", "\"'decimal-format'\"",
  "\"'decimal-separator'\"", "\"'grouping-separator'\"", "\"'infinity'\"",
  "\"'minus-sign'\"", "\"'NaN'\"", "\"'percent'\"", "\"'per-mille'\"",
  "\"'zero-digit'\"", "\"'digit'\"", "\"'pattern-separator'\"",
  "\"'ancestor'\"", "\"'ancestor-or-self'\"", "\"'and'\"", "\"'''\"",
  "\"'as'\"", "\"'ascending'\"", "\"'at'\"", "\"'attribute'\"", "\"'@'\"",
  "\"'||'\"", "\"'case'\"", "\"'castable'\"", "\"'cast'\"",
  "\"'CDATA[['\"", "\"']]'\"", "\"'child'\"", "\"'collation'\"", "\"','\"",
  "\"'(:'\"", "\"':)'\"", "\"'DECIMAL'\"", "\"'context'\"",
  "\"'variable'\"", "\"'default'\"", "\"'descendant'\"",
  "\"'descendant-or-self'\"", "\"'descending'\"", "\"'div'\"", "\"'$'\"",
  "\"'.'\"", "\"'..'\"", "\"':'\"", "\"'::'\"", "\"'{{'\"", "\"'DOUBLE'\"",
  "\"'<double {>'\"", "\"'else'\"", "\"'empty'\"", "\"'greatest'\"",
  "\"'least'\"", "\"'/>'\"", "\"'encoding'\"", "\"'='\"", "\"''''\"",
  "\"'\\\"\\\"'\"", "\"'every'\"", "\"'except'\"", "\"'external'\"",
  "\"'following'\"", "\"'following-sibling'\"", "\"'follows'\"",
  "\"'>='\"", "\"':='\"", "\"'>'\"", "\"'?'\"", "\"'#'\"", "\"'idiv'\"",
  "\"'in'\"", "\"'inherit'\"", "\"'INTEGER'\"", "\"'intersect'\"",
  "\"'is'\"", "\"'item'\"", "\"'{'\"", "\"'['\"", "\"'<='\"", "\"'('\"",
  "\"'<'\"", "\"'-'\"", "\"'mod'\"", "\"'namespace'\"", "\"'nan'\"",
  "\"'!='\"", "\"'nodecomp'\"", "\"'<no inherit>'\"",
  "\"'<no preserve>'\"", "\"'or'\"", "\"'order'\"", "\"'ordered'\"",
  "\"'by'\"", "\"'group'\"", "\"'parent'\"", "\"'%'\"", "\"'<?'\"",
  "\"'?>'\"", "\"'+'\"", "\"'(#'\"", "\"'#)'\"", "\"'<<'\"",
  "\"'preceding'\"", "\"'preceding-sibling'\"", "\"'preserve'\"",
  "\"'\\\"'\"", "\"'}'\"", "\"']'\"", "\"'return'\"", "\"')'\"",
  "\"'satisfies'\"", "\"'self'\"", "\"';'\"", "\"'/'\"", "\"'//'\"",
  "\"'!'\"", "\"'*'\"", "\"'</ (start tag end)'\"", "\"'strip'\"",
  "\"'> (tag end)'\"", "\"'then'\"", "\"'to'\"", "\"'treat'\"",
  "\"'union'\"", "\"'unordered'\"", "\"'eq'\"", "\"'ge'\"", "\"'gt'\"",
  "\"'le'\"", "\"'lt'\"", "\"'ne'\"", "\"'VALUECOMP'\"", "\"'|'\"",
  "\"'empty-sequence'\"", "\"'where'\"", "\"'<!--'\"", "\"'-->'\"",
  "\"'xquery'\"", "\"'version'\"", "\"'start'\"", "\"'after'\"",
  "\"'before'\"", "\"'revalidation'\"", "\"'strict'\"", "\"'lax'\"",
  "\"'skip'\"", "\"'delete'\"", "\"'node'\"", "\"'insert'\"",
  "\"'nodes'\"", "\"'rename'\"", "\"'replace'\"", "\"'value'\"",
  "\"'of'\"", "\"'first'\"", "\"'into'\"", "\"'last'\"", "\"'modify'\"",
  "\"'copy'\"", "\"'with'\"", "\"'break'\"", "\"'continue'\"",
  "\"'exit'\"", "\"'loop'\"", "\"'returning'\"", "\"'while'\"",
  "\"'try'\"", "\"'catch'\"", "\"'using'\"", "\"'all'\"", "\"'any'\"",
  "\"'contains'\"", "\"'content'\"", "\"'diacritics'\"", "\"'different'\"",
  "\"'distance'\"", "\"'end'\"", "\"'entire'\"", "\"'exactly'\"",
  "\"'from'\"", "\"'ftand'\"", "\"'ftnot'\"", "\"'not'\"",
  "\"'ft-option'\"", "\"'ftor'\"", "\"'insensitive'\"", "\"'language'\"",
  "\"'levels'\"", "\"'lowercase'\"", "\"'occurs'\"", "\"'paragraph'\"",
  "\"'paragraphs'\"", "\"'phrase'\"", "\"'relationship'\"", "\"'same'\"",
  "\"'score'\"", "\"'sensitive'\"", "\"'sentence'\"", "\"'sentences'\"",
  "\"'stemming'\"", "\"'stop'\"", "\"'thesaurus'\"", "\"'times'\"",
  "\"'uppercase'\"", "\"'weight'\"", "\"'wildcards'\"", "\"'window'\"",
  "\"'without'\"", "\"'words'\"", "\"'collection'\"", "\"'const'\"",
  "\"'append-only'\"", "\"'queue'\"", "\"'mutable'\"", "\"'read-only'\"",
  "\"'unique'\"", "\"'non'\"", "\"'index'\"", "\"'manually'\"",
  "\"'automatically'\"", "\"'maintained'\"", "\"'on'\"", "\"'range'\"",
  "\"'equality'\"", "\"'general'\"", "\"'integrity'\"", "\"'constraint'\"",
  "\"'check'\"", "\"'key'\"", "\"'foreach'\"", "\"'foreign'\"",
  "\"'keys'\"", "\"'{|'\"", "\"'|}'\"", "\"'{['\"", "\"']}'\"",
  "\"'json'\"", "\"'append'\"", "\"'position'\"", "\"'object'\"",
  "\"'array'\"", "\"'json-item'\"", "\"'structured-item'\"",
  "\"'BOM_UTF8'\"", "\"'#!/shebang\"", "RANGE_REDUCE",
  "SEQUENCE_TYPE_REDUCE", "ADDITIVE_REDUCE", "UNION_REDUCE",
  "INTERSECT_EXCEPT_REDUCE", "QVARINDECLLIST_REDUCE", "UNARY_PREC",
  "STEP_REDUCE", "SIMPLEMAPEXPR_REDUCE", "MULTIPLICATIVE_REDUCE", "SELECT",
  "$accept", "Module", "ERROR", "ModuleWithoutBOM", "VersionDecl",
  "MainModule", "LibraryModule", "ModuleDecl", "SIND_DeclList",
  "SIND_Decl", "Setter", "BoundarySpaceDecl", "DefaultCollationDecl",
  "BaseURIDecl", "ConstructionDecl", "OrderingModeDecl", "EmptyOrderDecl",
  "CopyNamespacesDecl", "Import", "SchemaImport", "URILiteralList",
  "SchemaPrefix", "ModuleImport", "NamespaceDecl", "DefaultNamespaceDecl",
  "VFO_DeclList", "VFO_Decl", "DecimalFormatDecl",
  "DecimalFormatParamList", "DecimalFormatParam", "DecimalFormatParamName",
  "OptionDecl", "FTOptionDecl", "CtxItemDecl", "CtxItemDecl2",
  "CtxItemDecl3", "CtxItemDecl4", "VarDecl", "VarNameAndType",
  "AnnotationList", "Annotation", "AnnotationLiteralList", "FunctionDecl",
  "FunctionDecl2", "FunctionDeclSimple", "FunctionDeclUpdating",
  "FunctionSig", "ParamList", "Param", "CollectionDecl",
  "CollectionTypeDecl", "IndexDecl", "IndexKeyList", "IndexKeySpec",
  "IntegrityConstraintDecl", "QueryBody", "StatementsAndOptionalExprTop",
  "StatementsAndOptionalExpr", "StatementsAndExpr", "Statements",
  "Statement", "BlockStatement", "BlockExpr",
  "EnclosedStatementsAndOptionalExpr", "VarDeclStatement",
  "BlockVarDeclList", "BlockVarDecl", "AssignStatement", "ApplyStatement",
  "ExitStatement", "WhileStatement", "FlowCtlStatement", "FLWORStatement",
  "ReturnStatement", "IfStatement", "TryStatement", "CatchListStatement",
  "CatchStatement", "VoidStatement", "Expr", "ExprSingle", "ExprSimple",
  "FLWORExpr", "ReturnExpr", "ReturnOrSelect", "WindowType",
  "FLWORWinCondType", "FLWORWinCond", "WindowClause", "CountClause",
  "ForLetWinClause", "FLWORClause", "FLWORClauseList", "ForClause",
  "ForOrFrom", "VarInDeclList", "VarInDecl", "PositionalVar", "FTScoreVar",
  "LetClause", "VarGetsDeclList", "VarGetsDecl", "WindowVarDecl",
  "WindowVars", "WindowVars3", "WindowVars2", "WhereClause",
  "GroupByClause", "GroupSpecList", "GroupSpec", "GroupCollationSpec",
  "OrderByClause", "OrderSpecList", "OrderSpec", "OrderModifier",
  "OrderDirSpec", "OrderEmptySpec", "OrderCollationSpec", "QuantifiedExpr",
  "QVarInDeclList", "QVarInDecl", "SwitchExpr", "SwitchCaseClauseList",
  "SwitchCaseClause", "SwitchCaseOperandList", "SwitchStatement",
  "SwitchCaseStatementList", "SwitchCaseStatement", "TypeswitchExpr",
  "TypeswitchStatement", "CaseClauseList", "CaseClause",
  "CaseStatementList", "CaseStatement", "SequenceTypeList", "IfExpr",
  "OrExpr", "AndExpr", "ComparisonExpr", "$@1", "FTContainsExpr",
  "StringConcatExpr", "opt_FTIgnoreOption", "RangeExpr", "AdditiveExpr",
  "MultiplicativeExpr", "UnionExpr", "IntersectExceptExpr",
  "InstanceofExpr", "TreatExpr", "CastableExpr", "CastExpr", "SingleType",
  "UnaryExpr", "SignList", "ValueExpr", "SimpleMapExpr", "ValueComp",
  "NodeComp", "ValidateExpr", "ExtensionExpr", "Pragma_list", "Pragma",
  "PathExpr", "LeadingSlash", "RelativePathExpr", "StepExpr", "AxisStep",
  "ForwardStep", "ForwardAxis", "AbbrevForwardStep", "ReverseStep",
  "ReverseAxis", "NodeTest", "NameTest", "Wildcard", "PostfixExpr",
  "PredicateList", "Predicate", "PrimaryExpr", "Literal", "NumericLiteral",
  "VarRef", "ParenthesizedExpr", "ContextItemExpr", "OrderedExpr",
  "UnorderedExpr", "FunctionCall", "ArgList", "FunctionItemExpr",
  "LiteralFunctionItem", "InlineFunction", "Constructor",
  "DirectConstructor", "DirElemConstructor", "DirElemContentList",
  "DirAttributeList", "DirAttr", "OptionalBlank", "DirAttributeValue",
  "opt_QuoteAttrContentList", "QuoteAttrContentList",
  "opt_AposAttrContentList", "AposAttrContentList",
  "QuoteAttrValueContent", "AposAttrValueContent", "DirElemContent",
  "CommonContent", "DirCommentConstructor", "DirPIConstructor",
  "CDataSection", "ComputedConstructor", "CompDocConstructor",
  "CompElemConstructor", "CompAttrConstructor", "CompTextConstructor",
  "CompCommentConstructor", "CompPIConstructor",
  "CompNamespaceConstructor", "TypeDeclaration", "SequenceType",
  "OccurrenceIndicator", "ItemType", "TypeList", "GeneralizedAtomicType",
  "SimpleType", "KindTest", "AnyKindTest", "NODE_KIND_TEST",
  "DocumentTest", "NamespaceTest", "TextTest", "CommentTest", "PITest",
  "AttributeTest", "SchemaAttributeTest", "ElementTest",
  "SchemaElementTest", "TypeName", "TypeName_WITH_HOOK", "StringLiteral",
  "FunctionTest", "AnyFunctionTest", "TypedFunctionTest",
  "ParenthesizedItemType", "RevalidationDecl", "InsertExpr", "DeleteExpr",
  "ReplaceExpr", "RenameExpr", "TransformExpr", "VarNameList",
  "VarNameDecl", "TryExpr", "CatchListExpr", "CatchExpr", "BracedExpr",
  "NameTestList", "FTSelection", "opt_FTPosFilter_list",
  "FTPosFilter_list", "FTOr", "FTAnd", "FTMildNot", "FTUnaryNot",
  "FTPrimaryWithOptions", "opt_FTMatchOptions", "opt_FTWeight", "FTWeight",
  "FTPrimary", "opt_FTTimes", "FTExtensionSelection", "opt_FTSelection",
  "FTWords", "FTWordsValue", "opt_FTAnyallOption", "FTAnyallOption",
  "opt_word", "opt_words", "FTPosFilter", "FTOrder", "FTWindow",
  "FTDistance", "FTUnit", "FTMatchOptions", "FTMatchOption",
  "FTCaseOption", "FTDiacriticsOption", "FTExtensionOption",
  "FTStemOption", "FTThesaurusOption", "FTThesaurusID_or_default",
  "opt_FTThesaurus_list", "FTThesaurus_list", "FTThesaurusID",
  "opt_relationship", "opt_levels", "FTStopWordOption", "FTStopWords",
  "STRING_LITERAL_list", "opt_FTStopWordsInclExcl_list",
  "FTStopWordsInclExcl_list", "FTStopWordsInclExcl", "FTLanguageOption",
  "FTWildCardOption", "FTContent", "FTTimes", "FTRange", "FTScope",
  "FTBigUnit", "FTIgnoreOption", "JSONArrayConstructor",
  "JSONSimpleObjectUnion", "JSONAccumulatorObjectUnion",
  "JSONObjectConstructor", "JSONPairList", "JSONInsertExpr",
  "JSONAppendExpr", "JSONDeleteExpr", "JSONRenameExpr", "JSONReplaceExpr",
  "JSONTest", "URI_LITERAL", "QNAME", "FUNCTION_NAME", "EQNAME", YY_NULLPTR
  };
#endif


#if YYDEBUG
  const short
  xquery_parser::yyrline_[] =
  {
       0,   880,   880,   881,   885,   889,   895,   899,   903,   909,
     915,   923,   929,   938,   943,   949,   955,   961,   969,   974,
     984,   994,  1005,  1017,  1022,  1028,  1034,  1043,  1051,  1058,
    1064,  1075,  1076,  1077,  1078,  1079,  1082,  1083,  1084,  1085,
    1086,  1087,  1088,  1089,  1092,  1097,  1103,  1109,  1115,  1120,
    1126,  1131,  1137,  1142,  1148,  1153,  1158,  1163,  1169,  1171,
    1173,  1181,  1186,  1194,  1202,  1211,  1218,  1226,  1232,  1238,
    1244,  1251,  1259,  1270,  1277,  1284,  1292,  1299,  1305,  1316,
    1317,  1318,  1319,  1320,  1321,  1322,  1323,  1326,  1332,  1339,
    1346,  1354,  1362,  1363,  1364,  1365,  1366,  1367,  1368,  1369,
    1370,  1371,  1374,  1388,  1394,  1400,  1407,  1413,  1420,  1425,
    1431,  1437,  1449,  1461,  1474,  1479,  1487,  1495,  1504,  1509,
    1516,  1521,  1528,  1533,  1541,  1546,  1553,  1559,  1568,  1569,
    1572,  1586,  1599,  1614,  1627,  1632,  1637,  1642,  1648,  1655,
    1663,  1668,  1676,  1684,  1692,  1700,  1710,  1714,  1720,  1724,
    1731,  1740,  1751,  1757,  1764,  1769,  1776,  1783,  1792,  1802,
    1812,  1822,  1837,  1853,  1857,  1862,  1867,  1871,  1876,  1881,
    1886,  1894,  1901,  1908,  1921,  1922,  1923,  1924,  1925,  1926,
    1927,  1928,  1929,  1930,  1931,  1932,  1933,  1936,  1941,  1948,
    1968,  1974,  1980,  1987,  1994,  2003,  2012,  2021,  2030,  2041,
    2047,  2053,  2059,  2071,  2076,  2082,  2094,  2107,  2127,  2133,
    2140,  2149,  2156,  2162,  2167,  2179,  2191,  2192,  2193,  2194,
    2195,  2196,  2199,  2200,  2201,  2202,  2203,  2204,  2205,  2206,
    2207,  2208,  2209,  2210,  2213,  2225,  2231,  2236,  2241,  2247,
    2252,  2257,  2263,  2272,  2282,  2292,  2303,  2309,  2310,  2311,
    2314,  2315,  2316,  2317,  2318,  2321,  2328,  2336,  2340,  2348,
    2356,  2361,  2368,  2375,  2386,  2396,  2406,  2416,  2426,  2436,
    2446,  2456,  2466,  2475,  2485,  2495,  2507,  2513,  2519,  2525,
    2531,  2539,  2547,  2555,  2563,  2573,  2579,  2588,  2589,  2593,
    2600,  2604,  2609,  2612,  2616,  2620,  2626,  2632,  2638,  2644,
    2652,  2656,  2664,  2672,  2680,  2688,  2698,  2704,  2710,  2718,
    2724,  2732,  2736,  2744,  2750,  2756,  2762,  2771,  2780,  2789,
    2800,  2804,  2810,  2816,  2824,  2830,  2839,  2850,  2856,  2864,
    2868,  2877,  2883,  2889,  2897,  2903,  2909,  2917,  2923,  2929,
    2937,  2943,  2950,  2960,  2967,  2977,  2983,  2991,  2997,  3006,
    3012,  3020,  3026,  3035,  3041,  3049,  3055,  3059,  3065,  3070,
    3076,  3080,  3089,  3095,  3104,  3114,  3113,  3126,  3135,  3144,
    3155,  3159,  3170,  3174,  3180,  3183,  3189,  3193,  3199,  3203,
    3207,  3213,  3217,  3223,  3229,  3235,  3243,  3247,  3251,  3257,
    3261,  3267,  3275,  3279,  3287,  3291,  3299,  3303,  3311,  3315,
    3323,  3327,  3333,  3337,  3343,  3347,  3351,  3355,  3363,  3364,
    3365,  3368,  3373,  3379,  3383,  3387,  3391,  3395,  3399,  3405,
    3409,  3413,  3419,  3423,  3427,  3431,  3440,  3448,  3454,  3462,
    3466,  3470,  3476,  3480,  3488,  3496,  3516,  3522,  3532,  3536,
    3542,  3543,  3546,  3552,  3560,  3566,  3576,  3582,  3590,  3594,
    3598,  3602,  3606,  3612,  3618,  3624,  3628,  3634,  3638,  3647,
    3651,  3655,  3661,  3665,  3673,  3674,  3677,  3681,  3687,  3691,
    3695,  3699,  3705,  3709,  3713,  3717,  3723,  3729,  3737,  3743,
    3744,  3745,  3746,  3747,  3748,  3749,  3750,  3751,  3752,  3753,
    3754,  3755,  3756,  3759,  3763,  3769,  3776,  3783,  3792,  3798,
    3802,  3808,  3815,  3821,  3827,  3831,  3849,  3855,  3861,  3867,
    3875,  3879,  3885,  3891,  3901,  3902,  3905,  3906,  3907,  3910,
    3918,  3935,  3943,  3960,  3979,  4000,  4006,  4015,  4021,  4030,
    4037,  4038,  4040,  4045,  4052,  4055,  4061,  4067,  4073,  4080,
    4089,  4092,  4098,  4104,  4110,  4117,  4126,  4130,  4136,  4140,
    4146,  4150,  4154,  4159,  4166,  4170,  4174,  4178,  4184,  4189,
    4195,  4201,  4208,  4214,  4219,  4224,  4229,  4234,  4239,  4244,
    4247,  4253,  4257,  4263,  4267,  4273,  4279,  4285,  4289,  4295,
    4299,  4305,  4311,  4315,  4319,  4325,  4331,  4337,  4345,  4349,
    4353,  4357,  4361,  4365,  4369,  4375,  4381,  4389,  4395,  4401,
    4405,  4409,  4413,  4417,  4421,  4425,  4429,  4433,  4437,  4443,
    4449,  4455,  4459,  4463,  4469,  4474,  4480,  4486,  4490,  4495,
    4501,  4505,  4511,  4517,  4521,  4529,  4535,  4539,  4545,  4554,
    4563,  4569,  4575,  4583,  4589,  4595,  4601,  4607,  4611,  4617,
    4623,  4627,  4635,  4641,  4647,  4653,  4661,  4665,  4671,  4677,
    4681,  4687,  4691,  4697,  4703,  4710,  4718,  4723,  4729,  4735,
    4743,  4749,  4756,  4762,  4771,  4777,  4783,  4789,  4798,  4805,
    4811,  4817,  4826,  4833,  4837,  4843,  4848,  4855,  4859,  4865,
    4869,  4875,  4879,  4885,  4889,  4897,  4908,  4911,  4917,  4920,
    4926,  4932,  4940,  4944,  4950,  4953,  4959,  4969,  4972,  4978,
    4988,  4994,  5002,  5005,  5011,  5015,  5019,  5025,  5028,  5034,
    5037,  5043,  5047,  5051,  5055,  5059,  5065,  5071,  5081,  5091,
    5095,  5099,  5105,  5111,  5119,  5123,  5127,  5131,  5135,  5139,
    5143,  5147,  5153,  5157,  5161,  5165,  5171,  5177,  5185,  5193,
    5197,  5203,  5213,  5224,  5230,  5234,  5240,  5243,  5249,  5254,
    5261,  5269,  5272,  5278,  5282,  5288,  5297,  5304,  5312,  5316,
    5322,  5328,  5336,  5339,  5345,  5350,  5360,  5368,  5378,  5384,
    5388,  5394,  5398,  5402,  5408,  5414,  5418,  5422,  5426,  5432,
    5440,  5450,  5454,  5460,  5466,  5470,  5476,  5480,  5486,  5490,
    5496,  5503,  5509,  5518,  5524,  5534,  5540,  5546,  5564,  5589,
    5609,  5613,  5617,  5623,  5626,  5627,  5628,  5629,  5630,  5631,
    5632,  5633,  5634,  5635,  5636,  5637,  5638,  5639,  5640,  5641,
    5642,  5643,  5644,  5645,  5646,  5647,  5648,  5651,  5652,  5653,
    5654,  5655,  5656,  5657,  5658,  5659,  5660,  5661,  5662,  5663,
    5664,  5665,  5666,  5667,  5668,  5669,  5670,  5671,  5672,  5673,
    5674,  5675,  5676,  5677,  5678,  5679,  5680,  5681,  5682,  5683,
    5684,  5685,  5686,  5687,  5688,  5689,  5690,  5691,  5692,  5693,
    5694,  5695,  5696,  5697,  5698,  5699,  5700,  5701,  5702,  5703,
    5704,  5705,  5706,  5707,  5708,  5709,  5710,  5711,  5712,  5713,
    5714,  5715,  5716,  5717,  5718,  5719,  5720,  5721,  5722,  5723,
    5724,  5725,  5726,  5727,  5728,  5729,  5730,  5731,  5732,  5733,
    5734,  5735,  5736,  5737,  5738,  5739,  5740,  5741,  5742,  5743,
    5744,  5745,  5746,  5747,  5748,  5749,  5750,  5751,  5752,  5753,
    5754,  5755,  5756,  5757,  5758,  5759,  5760,  5761,  5762,  5763,
    5764,  5765,  5766,  5767,  5768,  5769,  5770,  5771,  5772,  5773,
    5774,  5775,  5776,  5777,  5778,  5779,  5780,  5781,  5782,  5783,
    5784,  5785,  5786,  5787,  5788,  5789,  5790,  5791,  5792,  5793,
    5794,  5795,  5796,  5797,  5798,  5799,  5800,  5801,  5802,  5803,
    5804,  5805,  5806,  5807,  5808,  5809,  5810,  5811,  5812,  5813,
    5814,  5815,  5816,  5817,  5818,  5819,  5820,  5821,  5822,  5823,
    5824,  5825,  5826,  5827,  5828,  5829,  5830,  5831,  5832,  5833,
    5834,  5835,  5836,  5837,  5838,  5839,  5840,  5841,  5842,  5843,
    5844,  5845,  5846,  5847,  5848,  5849,  5850,  5851,  5854
  };

  void
  xquery_parser::yy_stack_print_ () const
  {
    *yycdebug_ << "Stack now";
    for (stack_type::const_iterator
           i = yystack_.begin (),
           i_end = yystack_.end ();
         i != i_end; ++i)
      *yycdebug_ << ' ' << int (i->state);
    *yycdebug_ << '\n';
  }

  void
  xquery_parser::yy_reduce_print_ (int yyrule) const
  {
    int yylno = yyrline_[yyrule];
    int yynrhs = yyr2_[yyrule];
    // Print the symbols being reduced, and their result.
    *yycdebug_ << "Reducing stack by rule " << yyrule - 1
               << " (line " << yylno << "):\n";
    // The symbols being reduced.
    for (int yyi = 0; yyi < yynrhs; yyi++)
      YY_SYMBOL_PRINT ("   $" << yyi + 1 << " =",
                       yystack_[(yynrhs) - (yyi + 1)]);
  }
#endif // YYDEBUG

  xquery_parser::symbol_kind_type
  xquery_parser::yytranslate_ (int t) YY_NOEXCEPT
  {
    // YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to
    // TOKEN-NUM as returned by yylex.
    static
    const short
    translate_table[] =
    {
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133,   134,
     135,   136,   137,   138,   139,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,   151,   152,   153,   154,
     155,   156,   157,   158,   159,   160,   161,   162,   163,   164,
     165,   166,   167,   168,   169,   170,   171,   172,   173,   174,
     175,   176,   177,   178,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,   197,   198,   199,   200,   201,   202,   203,   204,
     205,   206,   207,   208,   209,   210,   211,   212,   213,   214,
     215,   216,   217,   218,   219,   220,   221,   222,   223,   224,
     225,   226,   227,   228,   229,   230,   231,   232,   233,   234,
     235,   236,   237,   238,   239,   240,   241,   242,   243,   244,
     245,   246,   247,   248,   249,   250,   251,   252,   253,   254,
     255,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   333
    };
    // Last valid token kind.
    const int code_max = 588;

    if (t <= 0)
      return symbol_kind::S_YYEOF;
    else if (t <= code_max)
      return static_cast <symbol_kind_type> (translate_table[t]);
    else
      return symbol_kind::S_YYUNDEF;
  }

} // zorba


namespace zorba {
static bool contains(const zstring& msg, const char* str1, const char* str2)
{
  zstring::size_type pos = msg.find(str1);
  if (pos == zstring::npos)
    return false;
  if (zstring(str2).size() == 0)
    return true;
  pos = msg.find(str2, pos);
  if (pos == zstring::npos)
    return false;
  return true;
}
void xquery_parser::error(zorba::xquery_parser::location_type const& loc, string const& msg)
{
  if (driver.parserError != NULL)
  {
    driver.set_expr(new ParseErrorNode(driver.parserError->loc, driver.parserError->err_code, driver.parserError->msg));
  }
  else
  {
    ParseErrorNode* prevErr = dynamic_cast<ParseErrorNode*>(driver.get_expr());
    if (prevErr != NULL)
    {
      if ((msg.find("unexpected expression") != string::npos &&
            ! contains(prevErr->msg, "expecting", ","))
          ||
          (msg.find("missing semicolon \";\" after") != string::npos &&
            ! contains(prevErr->msg, "expecting", ";")))
        return;
    }
    string message = msg;
    int pos;
    std::string unexpected_qname = "unexpected \"'QName'\"";
    if ((pos = message.find(unexpected_qname)) != -1)
      message = message.substr(0, pos) + "unexpected qualified name \"" + driver.symtab.get_last_qname() + "\"" + message.substr(pos+unexpected_qname.length());
    while ((pos = message.find("\"'")) != -1 || (pos = message.find("'\"")) != -1)
      message.replace(pos, 2, "\"");
    driver.set_expr(new ParseErrorNode(driver.createQueryLoc(loc), err::XPST0003, message));
  }
}
}
