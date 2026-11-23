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

  case 79: // VFO_DeclList: VFO_DeclList "';'" Import
    {
      (yylhs.value.node) = (yystack_[2].value.node); (yylhs.value.node) = (yystack_[0].value.node);
      error(yystack_[0].location, "syntax error, imports must come before variable, function, option and other declarations");
      release_hack( (yystack_[2].value.node) );
      release_hack( (yystack_[0].value.node) );
      YYERROR;
    }
    break;

  case 80: // VFO_Decl: DecimalFormatDecl
    { (yylhs.value.node) = (yystack_[0].value.node); }
    break;

  case 81: // VFO_Decl: OptionDecl
    { (yylhs.value.node) = (yystack_[0].value.node); }
    break;

  case 82: // VFO_Decl: CtxItemDecl
    { (yylhs.value.node) = (yystack_[0].value.node); }
    break;

  case 83: // VFO_Decl: VarDecl
    { (yylhs.value.node) = (yystack_[0].value.node); }
    break;

  case 84: // VFO_Decl: FunctionDecl
    { (yylhs.value.node) = (yystack_[0].value.node); }
    break;

  case 85: // VFO_Decl: CollectionDecl
    { (yylhs.value.node) = (yystack_[0].value.node); }
    break;

  case 86: // VFO_Decl: IndexDecl
    { (yylhs.value.node) = (yystack_[0].value.node); }
    break;

  case 87: // VFO_Decl: IntegrityConstraintDecl
    { (yylhs.value.node) = (yystack_[0].value.node); }
    break;

  case 88: // DecimalFormatDecl: "'declare'" "'default'" "'decimal-format'" DecimalFormatParamList
    {
      (yylhs.value.node) = new DecimalFormatNode(LOC(yylhs.location), (yystack_[0].value.vstrpair));
      delete (yystack_[0].value.vstrpair);
    }
    break;

  case 89: // DecimalFormatDecl: "'declare'" "'decimal-format'" QNAME DecimalFormatParamList
    {
      (yylhs.value.node) = new DecimalFormatNode(LOC(yylhs.location), static_cast<QName*>((yystack_[1].value.expr)), (yystack_[0].value.vstrpair));
      delete (yystack_[0].value.vstrpair);
    }
    break;

  case 90: // DecimalFormatParamList: DecimalFormatParam
    {
      (yylhs.value.vstrpair) = new vector<string_pair_t>();
      (yylhs.value.vstrpair)->push_back( *(yystack_[0].value.strpair) );
      delete (yystack_[0].value.strpair);
    }
    break;

  case 91: // DecimalFormatParamList: DecimalFormatParamList DecimalFormatParam
    {
      (yystack_[1].value.vstrpair)->push_back( *(yystack_[0].value.strpair) );
      delete (yystack_[0].value.strpair);
      (yylhs.value.vstrpair) = (yystack_[1].value.vstrpair);
    }
    break;

  case 92: // DecimalFormatParam: DecimalFormatParamName "'='" StringLiteral
    {
      StringLiteral *sl = static_cast<StringLiteral*>((yystack_[0].value.expr));
      (yylhs.value.strpair) = new string_pair_t( (yystack_[2].value.strval), sl->get_strval().str() );
      delete sl;
    }
    break;

  case 93: // DecimalFormatParamName: "'decimal-separator'"
                        { (yylhs.value.strval) = "decimal-separator"; }
    break;

  case 94: // DecimalFormatParamName: "'digit'"
            { (yylhs.value.strval) = "digit"; }
    break;

  case 95: // DecimalFormatParamName: "'grouping-separator'"
                         { (yylhs.value.strval) = "grouping-separator"; }
    break;

  case 96: // DecimalFormatParamName: "'infinity'"
                     { (yylhs.value.strval) = "infinity"; }
    break;

  case 97: // DecimalFormatParamName: "'minus-sign'"
                 { (yylhs.value.strval) = "minus-sign"; }
    break;

  case 98: // DecimalFormatParamName: "'NaN'"
          { (yylhs.value.strval) = "NaN"; }
    break;

  case 99: // DecimalFormatParamName: "'pattern-separator'"
                        { (yylhs.value.strval) = "pattern-separator"; }
    break;

  case 100: // DecimalFormatParamName: "'percent'"
              { (yylhs.value.strval) = "percent"; }
    break;

  case 101: // DecimalFormatParamName: "'per-mille'"
                { (yylhs.value.strval) = "per-mille"; }
    break;

  case 102: // DecimalFormatParamName: "'zero-digit'"
                 { (yylhs.value.strval) = "zero-digit"; }
    break;

  case 103: // OptionDecl: "'declare'" "'option'" QNAME "'STRING'"
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

  case 104: // FTOptionDecl: "'declare'" "'ft-option'" FTMatchOptions
    {
      (yylhs.value.node) = new FTOptionDecl( LOC(yylhs.location), dynamic_cast<FTMatchOptions*>((yystack_[0].value.node)) );
    }
    break;

  case 105: // CtxItemDecl: "'declare'" "'context'" "'item'" CtxItemDecl2
    {
      (yylhs.value.node) = (yystack_[0].value.node);
    }
    break;

  case 106: // CtxItemDecl2: "'as'" ItemType CtxItemDecl3
    {
      CtxItemDecl* d = dynamic_cast<CtxItemDecl*>((yystack_[0].value.node));
      d->theType = (yystack_[1].value.node);
      (yylhs.value.node) = d;
    }
    break;

  case 107: // CtxItemDecl2: CtxItemDecl3
    {
      (yylhs.value.node) = (yystack_[0].value.node);
    }
    break;

  case 108: // CtxItemDecl3: CtxItemDecl4
    {
      CtxItemDecl* d = dynamic_cast<CtxItemDecl*>((yystack_[0].value.node));
      d->theIsExternal = false;
      (yylhs.value.node) = d;
    }
    break;

  case 109: // CtxItemDecl3: "'external'"
    {
      (yylhs.value.node) = new CtxItemDecl(LOC(yylhs.location), NULL);
    }
    break;

  case 110: // CtxItemDecl3: "'external'" CtxItemDecl4
    {
      (yylhs.value.node) = (yystack_[0].value.node);
    }
    break;

  case 111: // CtxItemDecl4: "':='" ExprSingle
    {
      (yylhs.value.node) = new CtxItemDecl(LOC(yylhs.location), (yystack_[0].value.expr));
    }
    break;

  case 112: // VarDecl: "'declare'" VarNameAndType "':='" ExprSingle
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

  case 113: // VarDecl: "'declare'" VarNameAndType "'external'"
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

  case 114: // VarDecl: "'declare'" VarNameAndType "'external'" "':='" ExprSingle
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

  case 115: // VarNameAndType: "'variable'" "'$'" QNAME
    {
      (yylhs.value.varnametype) = new VarNameAndType(LOC(yylhs.location), static_cast<QName*>((yystack_[0].value.expr)), NULL, NULL);
    }
    break;

  case 116: // VarNameAndType: "'variable'" "'$'" QNAME TypeDeclaration
    {
      (yylhs.value.varnametype) = new VarNameAndType(LOC(yylhs.location),
                              static_cast<QName*>((yystack_[1].value.expr)),
                              dynamic_cast<SequenceTypeAST *>((yystack_[0].value.node)),
                              NULL);
    }
    break;

  case 117: // VarNameAndType: AnnotationList "'variable'" "'$'" QNAME
    {
      (yylhs.value.varnametype) = new VarNameAndType(LOC(yylhs.location),
                              static_cast<QName*>((yystack_[0].value.expr)),
                              NULL,
                              static_cast<AnnotationListParsenode*>((yystack_[3].value.node)));
    }
    break;

  case 118: // VarNameAndType: AnnotationList "'variable'" "'$'" QNAME TypeDeclaration
    {
      (yylhs.value.varnametype) = new VarNameAndType(LOC(yylhs.location),
                              static_cast<QName*>((yystack_[1].value.expr)),
                              dynamic_cast<SequenceTypeAST *>((yystack_[0].value.node)),
                              static_cast<AnnotationListParsenode*>((yystack_[4].value.node)));
    }
    break;

  case 119: // AnnotationList: Annotation
    {
      (yylhs.value.node) = new AnnotationListParsenode(LOC(yylhs.location), static_cast<AnnotationParsenode*>((yystack_[0].value.node)));
    }
    break;

  case 120: // AnnotationList: AnnotationList Annotation
    {
      static_cast<AnnotationListParsenode*>((yystack_[1].value.node))->push_back(static_cast<AnnotationParsenode*>((yystack_[0].value.node)));
      (yylhs.value.node) = (yystack_[1].value.node);
    }
    break;

  case 121: // Annotation: "'%QName'"
    {
      (yylhs.value.node) = new AnnotationParsenode(LOC(yylhs.location), new QName(LOC(yylhs.location), SYMTAB((yystack_[0].value.sval))), NULL);
    }
    break;

  case 122: // Annotation: "'%QName'" "'('" AnnotationLiteralList "')'"
    {
      (yylhs.value.node) = new AnnotationParsenode(LOC(yylhs.location),
                                   new QName(LOC(yylhs.location), SYMTAB((yystack_[3].value.sval))),
                                   static_cast<AnnotationLiteralListParsenode*>((yystack_[1].value.node)));
    }
    break;

  case 123: // Annotation: "'%EQName'"
    {
      (yylhs.value.node) = new AnnotationParsenode(LOC(yylhs.location), new QName(LOC(yylhs.location), SYMTAB((yystack_[0].value.sval)), true), NULL);
    }
    break;

  case 124: // Annotation: "'%EQName'" "'('" AnnotationLiteralList "')'"
    {
      (yylhs.value.node) = new AnnotationParsenode(LOC(yylhs.location),
                                   new QName(LOC(yylhs.location), SYMTAB((yystack_[3].value.sval)), true),
                                   static_cast<AnnotationLiteralListParsenode*>((yystack_[1].value.node)));
    }
    break;

  case 125: // AnnotationLiteralList: Literal
    {
      (yylhs.value.node) = new AnnotationLiteralListParsenode(LOC(yylhs.location), (yystack_[0].value.expr));
    }
    break;

  case 126: // AnnotationLiteralList: AnnotationLiteralList "','" Literal
    {
      static_cast<AnnotationLiteralListParsenode*>((yystack_[2].value.node))->push_back((yystack_[0].value.expr));
      (yylhs.value.node) = (yystack_[2].value.node);
    }
    break;

  case 127: // FunctionDecl: "'declare'" FunctionDecl2
    {
      static_cast<FunctionDecl*>((yystack_[0].value.node))->setComment( SYMTAB((yystack_[1].value.sval)) );
      (yylhs.value.node) = (yystack_[0].value.node);
    }
    break;

  case 128: // FunctionDecl: "'declare'" AnnotationList FunctionDecl2
    {
      FunctionDecl* fdecl = static_cast<FunctionDecl*>((yystack_[0].value.node));
      fdecl->setComment( SYMTAB((yystack_[2].value.sval)) );
      fdecl->set_annotations(static_cast<AnnotationListParsenode*>((yystack_[1].value.node)));
      (yylhs.value.node) = (yystack_[0].value.node);
    }
    break;

  case 129: // FunctionDecl2: FunctionDeclSimple
    { (yylhs.value.node) = (yystack_[0].value.node); }
    break;

  case 130: // FunctionDecl2: FunctionDeclUpdating
    { (yylhs.value.node) = (yystack_[0].value.node); }
    break;

  case 131: // FunctionDeclSimple: "'function'" FUNCTION_NAME FunctionSig EnclosedStatementsAndOptionalExpr
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

  case 132: // FunctionDeclSimple: "'function'" FUNCTION_NAME FunctionSig "'external'"
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

  case 133: // FunctionDeclUpdating: "'updating'" "'function'" FUNCTION_NAME FunctionSig EnclosedStatementsAndOptionalExpr
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

  case 134: // FunctionDeclUpdating: "'updating'" "'function'" FUNCTION_NAME FunctionSig "'external'"
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

  case 135: // FunctionSig: "'('" "')'"
    {
      (yylhs.value.fnsig) = new FunctionSig(NULL);
    }
    break;

  case 136: // FunctionSig: "'('" ParamList "')'"
    {
      (yylhs.value.fnsig) = new FunctionSig(dynamic_cast<ParamList*>((yystack_[1].value.node)));
    }
    break;

  case 137: // FunctionSig: "'('" "')'" "'as'" SequenceType
    {
      (yylhs.value.fnsig) = new FunctionSig(NULL, dynamic_cast<SequenceTypeAST*>((yystack_[0].value.node)));
    }
    break;

  case 138: // FunctionSig: "'('" ParamList "')'" "'as'" SequenceType
    {
      (yylhs.value.fnsig) = new FunctionSig(dynamic_cast<ParamList*>((yystack_[3].value.node)), dynamic_cast<SequenceTypeAST*>((yystack_[0].value.node)));
    }
    break;

  case 139: // ParamList: Param
    {
      ParamList *pl = new ParamList( LOC(yylhs.location) );
      pl->push_back( dynamic_cast<Param*>((yystack_[0].value.node)) );
      (yylhs.value.node) = pl;
    }
    break;

  case 140: // ParamList: ParamList "','" Param
    {
      if ( ParamList *pl = dynamic_cast<ParamList*>((yystack_[2].value.node)) )
        pl->push_back( dynamic_cast<Param*>((yystack_[0].value.node)) );
      (yylhs.value.node) = (yystack_[2].value.node);
    }
    break;

  case 141: // Param: "'$'" QNAME
    {
      (yylhs.value.node) = new Param(LOC(yylhs.location), static_cast<QName*>((yystack_[0].value.expr)), NULL);
    }
    break;

  case 142: // Param: "'$'" QNAME TypeDeclaration
    {
      (yylhs.value.node) = new Param(LOC(yylhs.location),
                     static_cast<QName*>((yystack_[1].value.expr)),
                     dynamic_cast<SequenceTypeAST *>((yystack_[0].value.node)));
    }
    break;

  case 143: // CollectionDecl: "'declare'" "'collection'" QNAME
    {
      (yylhs.value.node) = new CollectionDecl( LOC(yylhs.location),
                              static_cast<QName*>((yystack_[0].value.expr)),
                              NULL,
                              NULL);
      static_cast<CollectionDecl*>((yylhs.value.node))->setComment(SYMTAB((yystack_[2].value.sval)));
    }
    break;

  case 144: // CollectionDecl: "'declare'" "'collection'" QNAME "'as'" CollectionTypeDecl
    {
      (yylhs.value.node) = new CollectionDecl( LOC(yylhs.location),
                              static_cast<QName*>((yystack_[2].value.expr)),
                              0,
                              static_cast<SequenceTypeAST*>((yystack_[0].value.node)));
      static_cast<CollectionDecl*>((yylhs.value.node))->setComment(SYMTAB((yystack_[4].value.sval)));
    }
    break;

  case 145: // CollectionDecl: "'declare'" AnnotationList "'collection'" QNAME
    {
      (yylhs.value.node) = new CollectionDecl( LOC(yylhs.location),
                               static_cast<QName*>((yystack_[0].value.expr)),
                               static_cast<AnnotationListParsenode*>((yystack_[2].value.node)),
                               0);
      static_cast<CollectionDecl*>((yylhs.value.node))->setComment(SYMTAB((yystack_[3].value.sval)));
    }
    break;

  case 146: // CollectionDecl: "'declare'" AnnotationList "'collection'" QNAME "'as'" CollectionTypeDecl
    {
      (yylhs.value.node) = new CollectionDecl( LOC(yylhs.location),
                               static_cast<QName*>((yystack_[2].value.expr)),
                               static_cast<AnnotationListParsenode*>((yystack_[4].value.node)),
                               static_cast<SequenceTypeAST*>((yystack_[0].value.node)));
      static_cast<CollectionDecl*>((yylhs.value.node))->setComment(SYMTAB((yystack_[5].value.sval)));
    }
    break;

  case 147: // CollectionTypeDecl: KindTest
    {
      (yylhs.value.node) = static_cast<parsenode*>(new SequenceTypeAST(LOC(yylhs.location), (yystack_[0].value.node), NULL));
    }
    break;

  case 148: // CollectionTypeDecl: KindTest OccurrenceIndicator
    {
      (yylhs.value.node) = static_cast<parsenode*>(new SequenceTypeAST(LOC(yylhs.location),
                                                    (yystack_[1].value.node),
                                                    dynamic_cast<OccurrenceIndicator*>((yystack_[0].value.node))));
    }
    break;

  case 149: // CollectionTypeDecl: JSONTest
    {
      (yylhs.value.node) = static_cast<parsenode*>(new SequenceTypeAST(LOC(yylhs.location), (yystack_[0].value.node), NULL));
    }
    break;

  case 150: // CollectionTypeDecl: JSONTest OccurrenceIndicator
    {
      (yylhs.value.node) = static_cast<parsenode*>(new SequenceTypeAST(LOC(yylhs.location),
                                                    (yystack_[1].value.node),
                                                    dynamic_cast<OccurrenceIndicator*>((yystack_[0].value.node))));
    }
    break;

  case 151: // IndexDecl: "'declare'" "'index'" QNAME "'on'" "'nodes'" PathExpr "'by'" IndexKeyList
    {
      (yylhs.value.node) = new AST_IndexDecl(LOC(yylhs.location),
                             static_cast<QName*>((yystack_[5].value.expr)),
                             (yystack_[2].value.expr),
                             dynamic_cast<IndexKeyList*>((yystack_[0].value.node)),
                             NULL);
      static_cast<AST_IndexDecl*>((yylhs.value.node))->setComment( SYMTAB((yystack_[7].value.sval)) );
    }
    break;

  case 152: // IndexDecl: "'declare'" AnnotationList "'index'" QNAME "'on'" "'nodes'" PathExpr "'by'" IndexKeyList
    {
      (yylhs.value.node) = new AST_IndexDecl(LOC(yylhs.location),
                             static_cast<QName*>((yystack_[5].value.expr)),
                             (yystack_[2].value.expr),
                             dynamic_cast<IndexKeyList*>((yystack_[0].value.node)),
                             static_cast<AnnotationListParsenode*>((yystack_[7].value.node)));
      static_cast<AST_IndexDecl*>((yylhs.value.node))->setComment( SYMTAB((yystack_[8].value.sval)) );
    }
    break;

  case 153: // IndexKeyList: IndexKeySpec
    {
      IndexKeyList* keyList = new IndexKeyList(LOC(yylhs.location));
      keyList->addKeySpec(dynamic_cast<IndexKeySpec*>((yystack_[0].value.node)));
      (yylhs.value.node) = keyList;
    }
    break;

  case 154: // IndexKeyList: IndexKeyList "','" IndexKeySpec
    {
      dynamic_cast<IndexKeyList*>((yystack_[2].value.node))->addKeySpec(dynamic_cast<IndexKeySpec*>((yystack_[0].value.node)));
      (yylhs.value.node) = (yystack_[2].value.node);
    }
    break;

  case 155: // IndexKeySpec: PathExpr
    {
      (yylhs.value.node) = new IndexKeySpec(LOC(yylhs.location), (yystack_[0].value.expr), NULL, NULL);
    }
    break;

  case 156: // IndexKeySpec: PathExpr TypeDeclaration
    {
      (yylhs.value.node) = new IndexKeySpec(LOC(yylhs.location),
                            (yystack_[1].value.expr),
                            dynamic_cast<SequenceTypeAST*>((yystack_[0].value.node)),
                            NULL);
    }
    break;

  case 157: // IndexKeySpec: PathExpr OrderCollationSpec
    {
      (yylhs.value.node) = new IndexKeySpec(LOC(yylhs.location),
                            (yystack_[1].value.expr),
                            NULL,
                            dynamic_cast<OrderCollationSpec*>((yystack_[0].value.node)));
    }
    break;

  case 158: // IndexKeySpec: PathExpr TypeDeclaration OrderCollationSpec
    {
      (yylhs.value.node) = new IndexKeySpec(LOC(yylhs.location),
                            (yystack_[2].value.expr),
                            dynamic_cast<SequenceTypeAST*>((yystack_[1].value.node)),
                            dynamic_cast<OrderCollationSpec*>((yystack_[0].value.node)));
    }
    break;

  case 159: // IntegrityConstraintDecl: "'declare'" "'integrity'" "'constraint'" QNAME "'on'" "'collection'" QNAME "'$'" QNAME "'check'" ExprSingle
    {
      (yylhs.value.node) = new ICCollSimpleCheck(LOC(yylhs.location),
                                 static_cast<QName*>((yystack_[7].value.expr)),
                                 static_cast<QName*>((yystack_[4].value.expr)),
                                 static_cast<QName*>((yystack_[2].value.expr)),
                                 (yystack_[0].value.expr));
    }
    break;

  case 160: // IntegrityConstraintDecl: "'declare'" "'integrity'" "'constraint'" QNAME "'on'" "'collection'" QNAME "'node'" "'$'" QNAME "'check'" "'unique'" "'key'" PathExpr
    {
      (yylhs.value.node) = new ICCollUniqueKeyCheck(LOC(yylhs.location),
                                    static_cast<QName*>((yystack_[10].value.expr)),
                                    static_cast<QName*>((yystack_[7].value.expr)),
                                    static_cast<QName*>((yystack_[4].value.expr)),
                                    (yystack_[0].value.expr));
    }
    break;

  case 161: // IntegrityConstraintDecl: "'declare'" "'integrity'" "'constraint'" QNAME "'on'" "'collection'" QNAME "'foreach'" "'node'" "'$'" QNAME "'check'" ExprSingle
    {
      (yylhs.value.node) = new ICCollForeachNode(LOC(yylhs.location),
                                 static_cast<QName*>((yystack_[9].value.expr)),
                                 static_cast<QName*>((yystack_[6].value.expr)),
                                 static_cast<QName*>((yystack_[2].value.expr)),
                                 (yystack_[0].value.expr));
    }
    break;

  case 162: // IntegrityConstraintDecl: "'declare'" "'integrity'" "'constraint'" QNAME "'foreign'" "'key'" "'from'" "'collection'" QNAME "'node'" "'$'" QNAME "'key'" PathExpr "'to'" "'collection'" QNAME "'node'" "'$'" QNAME "'key'" PathExpr
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

  case 163: // QueryBody: StatementsAndOptionalExprTop
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

  case 164: // StatementsAndOptionalExprTop: StatementsAndExpr
    {
      (yylhs.value.expr) = (yystack_[0].value.expr);
    }
    break;

  case 165: // StatementsAndOptionalExprTop: Statements
    {
      (yylhs.value.expr) = (yystack_[0].value.expr);
    }
    break;

  case 166: // StatementsAndOptionalExprTop: %empty
    {
      (yylhs.value.expr) = NULL;
    }
    break;

  case 167: // StatementsAndOptionalExpr: StatementsAndExpr
    {
      (yylhs.value.expr) = (yystack_[0].value.expr);
    }
    break;

  case 168: // StatementsAndOptionalExpr: Statements
    {
      (yylhs.value.expr) = (yystack_[0].value.expr);
    }
    break;

  case 169: // StatementsAndOptionalExpr: %empty
    {
      (yylhs.value.expr) = NULL;
    }
    break;

  case 170: // StatementsAndExpr: Expr
    {
      (yylhs.value.expr) = (yystack_[0].value.expr);
    }
    break;

  case 171: // StatementsAndExpr: Statements Expr
    {
      BlockBody* blk = static_cast<BlockBody*>((yystack_[1].value.expr));
      blk->add((yystack_[0].value.expr));
      (yylhs.value.expr) = blk;
    }
    break;

  case 172: // Statements: Statement
    {
      BlockBody* blk = new BlockBody(LOC(yylhs.location));
      blk->add((yystack_[0].value.expr));
      (yylhs.value.expr) = blk;
    }
    break;

  case 173: // Statements: Statements Statement
    {
      BlockBody* blk = static_cast<BlockBody*>((yystack_[1].value.expr));
      blk->add((yystack_[0].value.expr));
      (yylhs.value.expr) = blk;
    }
    break;

  case 174: // Statements: Statements Expr ERROR Statement
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

  case 175: // Statement: BlockStatement
    { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 176: // Statement: VarDeclStatement
    { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 177: // Statement: AssignStatement
    { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 178: // Statement: ApplyStatement
    { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 179: // Statement: ExitStatement
    { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 180: // Statement: WhileStatement
    { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 181: // Statement: FlowCtlStatement
    { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 182: // Statement: FLWORStatement
    { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 183: // Statement: IfStatement
    { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 184: // Statement: TypeswitchStatement
    { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 185: // Statement: SwitchStatement
    { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 186: // Statement: TryStatement
    { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 187: // Statement: VoidStatement
    { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 188: // BlockStatement: "'{'" Statements "'}'"
    {
      (yylhs.value.expr) = (yystack_[1].value.expr);
    }
    break;

  case 189: // BlockStatement: "'{'" "'}'"
    {
      driver.addCommonLanguageWarning(yystack_[1].location, ZED(ZWST0009_EMPTY_BLOCK));
      (yylhs.value.expr) = new BlockBody(LOC(yylhs.location));
    }
    break;

  case 190: // BlockExpr: "'{'" StatementsAndOptionalExpr "'}'"
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

  case 191: // EnclosedStatementsAndOptionalExpr: "'{'" StatementsAndOptionalExpr "'}'"
    {
      (yylhs.value.expr) = (yystack_[1].value.expr);
    }
    break;

  case 192: // VarDeclStatement: BlockVarDeclList "';'"
    {
      (yylhs.value.expr) = (yystack_[1].value.expr);
    }
    break;

  case 193: // BlockVarDeclList: BlockVarDeclList "','" BlockVarDecl
    {
      VarDeclStmt* vdecl = static_cast<VarDeclStmt*>((yystack_[2].value.expr));
      vdecl->add((yystack_[0].value.node));
      (yylhs.value.expr) = vdecl;
    }
    break;

  case 194: // BlockVarDeclList: "'variable'" BlockVarDecl
    {
      VarDeclStmt* vdecl = new VarDeclStmt(LOC(yylhs.location), NULL);
      vdecl->add((yystack_[0].value.node));
      (yylhs.value.expr) = vdecl;
    }
    break;

  case 195: // BlockVarDeclList: AnnotationList "'variable'" BlockVarDecl
    {
      VarDeclStmt* vdecl = new VarDeclStmt(LOC(yylhs.location),
                                           static_cast<AnnotationListParsenode*>((yystack_[2].value.node)));
      vdecl->add((yystack_[0].value.node));
      (yylhs.value.expr) = vdecl;
    }
    break;

  case 196: // BlockVarDecl: "'$'" QNAME
    {
      LocalVarDecl* vd = new LocalVarDecl(LOC(yylhs.location),
                                          static_cast<QName*>((yystack_[0].value.expr)),
                                          NULL,
                                          NULL,
                                          NULL);
      (yylhs.value.node) = vd;
    }
    break;

  case 197: // BlockVarDecl: "'$'" QNAME TypeDeclaration
    {
      LocalVarDecl* vd = new LocalVarDecl(LOC(yylhs.location),
                                          static_cast<QName*>((yystack_[1].value.expr)),
                                          dynamic_cast<SequenceTypeAST*>((yystack_[0].value.node)),
                                          NULL,
                                          NULL);
      (yylhs.value.node) = vd;
    }
    break;

  case 198: // BlockVarDecl: "'$'" QNAME "':='" ExprSingle
    {
      LocalVarDecl* vd = new LocalVarDecl(LOC(yylhs.location),
                                          static_cast<QName*>((yystack_[2].value.expr)),
                                          NULL,
                                          (yystack_[0].value.expr),
                                          NULL);
      (yylhs.value.node) = vd;
    }
    break;

  case 199: // BlockVarDecl: "'$'" QNAME TypeDeclaration "':='" ExprSingle
    {
      LocalVarDecl* vd = new LocalVarDecl(LOC(yylhs.location),
                                          static_cast<QName*>((yystack_[3].value.expr)),
                                          dynamic_cast<SequenceTypeAST*>((yystack_[2].value.node)),
                                          (yystack_[0].value.expr),
                                          NULL);
      (yylhs.value.node) = vd;
    }
    break;

  case 200: // AssignStatement: "'$'" QNAME "':='" ExprSingle "';'"
    {
      (yylhs.value.expr) = new AssignExpr(LOC(yylhs.location), static_cast<QName*>((yystack_[3].value.expr)), (yystack_[1].value.expr));
    }
    break;

  case 201: // ApplyStatement: ExprSimple "';'"
    {
      (yylhs.value.expr) = new ApplyExpr(LOC(yylhs.location), (yystack_[1].value.expr));
    }
    break;

  case 202: // ExitStatement: "'exit'" "'returning'" ExprSingle "';'"
    {
      (yylhs.value.expr) = new ExitExpr(LOC(yylhs.location), (yystack_[1].value.expr));
    }
    break;

  case 203: // WhileStatement: "'while'" "'('" Expr "')'" Statement
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

  case 204: // FlowCtlStatement: "'break'" "'loop'" "';'"
    {
      (yylhs.value.expr) = new FlowCtlStatement(LOC(yylhs.location), FlowCtlStatement::BREAK);
    }
    break;

  case 205: // FlowCtlStatement: "'continue'" "'loop'" "';'"
    {
      (yylhs.value.expr) = new FlowCtlStatement( LOC(yylhs.location), FlowCtlStatement::CONTINUE );
    }
    break;

  case 206: // FLWORStatement: FLWORClauseList ReturnStatement
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

  case 207: // ReturnStatement: ReturnOrSelect Statement
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

  case 208: // IfStatement: "'if'" "'('" Expr "')'" "'then'" Statement "'else'" Statement
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

  case 209: // TryStatement: "'try'" BlockStatement CatchListStatement
    {
      (yylhs.value.expr) = new TryExpr(LOC(yylhs.location), (yystack_[1].value.expr), (yystack_[0].value.expr));
    }
    break;

  case 210: // CatchListStatement: CatchStatement
    {
      CatchListExpr* cle = new CatchListExpr( LOC(yylhs.location) );
      cle->push_back( static_cast<CatchExpr*>((yystack_[0].value.expr)) );
      (yylhs.value.expr) = cle;
    }
    break;

  case 211: // CatchListStatement: CatchListStatement CatchStatement
    {
      CatchListExpr *cle = dynamic_cast<CatchListExpr*>((yystack_[1].value.expr));
      if ( cle )
        cle->push_back( static_cast<CatchExpr*>((yystack_[0].value.expr)) );
      (yylhs.value.expr) = (yystack_[1].value.expr);
    }
    break;

  case 212: // CatchStatement: "'catch'" NameTestList BlockStatement
    {
       (yylhs.value.expr) = new CatchExpr(LOC(yylhs.location), *(yystack_[1].value.name_test_list), (yystack_[0].value.expr));
       delete (yystack_[1].value.name_test_list);
    }
    break;

  case 213: // VoidStatement: "';'"
    {
      (yylhs.value.expr) = new BlockBody(LOC(yylhs.location));
    }
    break;

  case 214: // Expr: ExprSingle
    {
      (yylhs.value.expr) = (yystack_[0].value.expr);
    }
    break;

  case 215: // Expr: Expr "','" ExprSingle
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

  case 216: // Expr: Expr ERROR ExprSingle
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

  case 217: // ExprSingle: FLWORExpr
        { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 218: // ExprSingle: SwitchExpr
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 219: // ExprSingle: TypeswitchExpr
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 220: // ExprSingle: IfExpr
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 221: // ExprSingle: TryExpr
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 222: // ExprSingle: ExprSimple
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 223: // ExprSimple: OrExpr
        { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 224: // ExprSimple: QuantifiedExpr
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 225: // ExprSimple: InsertExpr
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 226: // ExprSimple: DeleteExpr
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 227: // ExprSimple: RenameExpr
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 228: // ExprSimple: ReplaceExpr
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 229: // ExprSimple: TransformExpr
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 230: // ExprSimple: JSONDeleteExpr
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 231: // ExprSimple: JSONInsertExpr
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 232: // ExprSimple: JSONRenameExpr
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 233: // ExprSimple: JSONReplaceExpr
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 234: // ExprSimple: JSONAppendExpr
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 235: // FLWORExpr: FLWORClauseList ReturnExpr
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

  case 236: // ReturnExpr: ReturnOrSelect ExprSingle
    {
      (yylhs.value.expr) = new ReturnExpr( LOC(yylhs.location), (yystack_[0].value.expr) );
    }
    break;

  case 237: // ReturnOrSelect: "'return'"
    {
    }
    break;

  case 238: // WindowType: "'sliding'" "'window'"
    {
      (yylhs.value.strval) = parser::the_sliding;
    }
    break;

  case 239: // WindowType: "'tumbling'" "'window'"
    {
      (yylhs.value.strval) = parser::the_tumbling;
    }
    break;

  case 240: // FLWORWinCondType: "'start'"
    {
      (yylhs.value.strval) = parser::the_start;
    }
    break;

  case 241: // FLWORWinCondType: "'end'"
    {
      (yylhs.value.strval) = parser::the_end;
    }
    break;

  case 242: // FLWORWinCondType: "'only'" "'end'"
  {
    (yylhs.value.strval) = parser::the_only_end;
  }
    break;

  case 243: // FLWORWinCond: FLWORWinCondType WindowVars "'when'" ExprSingle
    {
      (yylhs.value.node) = new FLWORWinCond(LOC(yylhs.location),
                            dynamic_cast<WindowVars*>((yystack_[2].value.node)),
                            (yystack_[0].value.expr),
                            (yystack_[3].value.strval) == parser::the_start,
                            (yystack_[3].value.strval) == parser::the_only_end);
    }
    break;

  case 244: // FLWORWinCond: FLWORWinCondType "'when'" ExprSingle
    {
      (yylhs.value.node) = new FLWORWinCond(LOC(yylhs.location),
                            NULL,
                            (yystack_[0].value.expr),
                            (yystack_[2].value.strval) == parser::the_start,
                            (yystack_[2].value.strval) == parser::the_only_end);
    }
    break;

  case 245: // WindowClause: ForOrFrom WindowType WindowVarDecl FLWORWinCond FLWORWinCond
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

  case 246: // WindowClause: ForOrFrom WindowType WindowVarDecl FLWORWinCond
    {
      (yylhs.value.node) = new WindowClause (LOC (yylhs.location),
                             ((yystack_[2].value.strval) == parser::the_tumbling ?
                              WindowClause::tumbling_window :
                              WindowClause::sliding_window),
                             dynamic_cast<WindowVarDecl *> ((yystack_[1].value.node)),
                             dynamic_cast<FLWORWinCond *> ((yystack_[0].value.node)), NULL);
    }
    break;

  case 247: // CountClause: "'count'" "'$'" QNAME
    {
      (yylhs.value.node) = new CountClause(LOC(yylhs.location), static_cast<QName*>((yystack_[0].value.expr)));
    }
    break;

  case 248: // ForLetWinClause: ForClause
    { (yylhs.value.node) = (yystack_[0].value.node); }
    break;

  case 249: // ForLetWinClause: LetClause
    { (yylhs.value.node) = (yystack_[0].value.node); }
    break;

  case 250: // ForLetWinClause: WindowClause
    { (yylhs.value.node) = (yystack_[0].value.node); }
    break;

  case 251: // FLWORClause: ForLetWinClause
    { (yylhs.value.node) = (yystack_[0].value.node); }
    break;

  case 252: // FLWORClause: WhereClause
    { (yylhs.value.node) = (yystack_[0].value.node); }
    break;

  case 253: // FLWORClause: OrderByClause
    { (yylhs.value.node) = (yystack_[0].value.node); }
    break;

  case 254: // FLWORClause: GroupByClause
    { (yylhs.value.node) = (yystack_[0].value.node); }
    break;

  case 255: // FLWORClause: CountClause
    { (yylhs.value.node) = (yystack_[0].value.node); }
    break;

  case 256: // FLWORClauseList: ForLetWinClause
    {
      FLWORClauseList *fcl = new FLWORClauseList( LOC(yylhs.location) );
      fcl->push_back( dynamic_cast<FLWORClause*>((yystack_[0].value.node)) );
      (yylhs.value.node) = fcl;
    }
    break;

  case 257: // FLWORClauseList: FLWORClauseList FLWORClause
    {
      FLWORClauseList *fcl = dynamic_cast<FLWORClauseList*>((yystack_[1].value.node));
      fcl->push_back( dynamic_cast<FLWORClause*>((yystack_[0].value.node)) );
      (yylhs.value.node) = fcl;
    }
    break;

  case 258: // ForClause: ForOrFrom "'$'" VarInDeclList
    {
      (yylhs.value.node) = new ForClause(LOC(yylhs.location), dynamic_cast<VarInDeclList*>((yystack_[0].value.node)));
    }
    break;

  case 259: // ForClause: "'for'" error VarInDeclList
    {
      (yylhs.value.node) = (yystack_[0].value.node);
      error(yystack_[1].location, "syntax error, unexpected qualified name \""
          + static_cast<VarInDeclList*>((yystack_[0].value.node))->operator[](0)->get_var_name()->get_qname().str() + "\" (missing \"$\" sign?)");
      delete (yystack_[0].value.node);
      YYERROR;
    }
    break;

  case 260: // ForClause: ForOrFrom "'unrecognized'"
    {
      (yylhs.value.node) = NULL;
      error(yystack_[0].location, "");
      YYERROR;
    }
    break;

  case 261: // ForOrFrom: "'for'"
    {
    }
    break;

  case 262: // VarInDeclList: VarInDecl
    {
      VarInDeclList* vdl = new VarInDeclList( LOC(yylhs.location) );
      vdl->push_back( dynamic_cast<VarInDecl*>((yystack_[0].value.node)) );
      (yylhs.value.node) = vdl;
    }
    break;

  case 263: // VarInDeclList: VarInDeclList "','" "'$'" VarInDecl
    {
      if ( VarInDeclList* vdl = dynamic_cast<VarInDeclList*>((yystack_[3].value.node)) )
        vdl->push_back( dynamic_cast<VarInDecl*>((yystack_[0].value.node)) );
      (yylhs.value.node) = (yystack_[3].value.node);
    }
    break;

  case 264: // VarInDeclList: VarInDeclList "','" VarInDecl
    {
      (yylhs.value.node) = (yystack_[2].value.node);
      error(yystack_[0].location, "syntax error, unexpected QName \""
          + static_cast<VarInDecl*>((yystack_[0].value.node))->get_var_name()->get_qname().str() + "\" (missing \"$\" sign?)");
      delete (yystack_[2].value.node);
      release_hack( (yystack_[0].value.node) );
      YYERROR;
    }
    break;

  case 265: // VarInDecl: QNAME "'in'" ExprSingle
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

  case 266: // VarInDecl: QNAME "'allowing'" "'empty'" "'in'" ExprSingle
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

  case 267: // VarInDecl: QNAME TypeDeclaration "'in'" ExprSingle
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

  case 268: // VarInDecl: QNAME TypeDeclaration "'allowing'" "'empty'" "'in'" ExprSingle
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

  case 269: // VarInDecl: QNAME PositionalVar "'in'" ExprSingle
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

  case 270: // VarInDecl: QNAME "'allowing'" "'empty'" PositionalVar "'in'" ExprSingle
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

  case 271: // VarInDecl: QNAME TypeDeclaration PositionalVar "'in'" ExprSingle
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

  case 272: // VarInDecl: QNAME TypeDeclaration "'allowing'" "'empty'" PositionalVar "'in'" ExprSingle
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

  case 273: // VarInDecl: QNAME FTScoreVar "'in'" ExprSingle
    {
      (yylhs.value.node) = new VarInDecl(LOC(yylhs.location),
                         static_cast<QName*>((yystack_[3].value.expr)),
                         NULL,NULL,
                         dynamic_cast<FTScoreVar*>((yystack_[2].value.node)),
                         (yystack_[0].value.expr),
                         false);
    }
    break;

  case 274: // VarInDecl: QNAME TypeDeclaration FTScoreVar "'in'" ExprSingle
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

  case 275: // VarInDecl: QNAME PositionalVar FTScoreVar "'in'" ExprSingle
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

  case 276: // VarInDecl: QNAME TypeDeclaration PositionalVar FTScoreVar "'in'" ExprSingle
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

  case 277: // PositionalVar: "'at'" "'$'" QNAME
    {
      (yylhs.value.node) = new PositionalVar(LOC(yylhs.location), static_cast<QName*>((yystack_[0].value.expr)));
    }
    break;

  case 278: // FTScoreVar: "'score'" "'$'" QNAME
    {
      (yylhs.value.node) = new FTScoreVar(LOC(yylhs.location), static_cast<QName*>((yystack_[0].value.expr)));
    }
    break;

  case 279: // LetClause: "'let'" VarGetsDeclList
        {
            (yylhs.value.node) = new LetClause( LOC(yylhs.location), dynamic_cast<VarGetsDeclList*>((yystack_[0].value.node)) );
        }
    break;

  case 280: // VarGetsDeclList: VarGetsDecl
        {
            VarGetsDeclList *vgdl = new VarGetsDeclList( LOC(yylhs.location) );
            vgdl->push_back( dynamic_cast<VarGetsDecl*>((yystack_[0].value.node)) );
            (yylhs.value.node) = vgdl;
        }
    break;

  case 281: // VarGetsDeclList: VarGetsDeclList "','" VarGetsDecl
        {
            if( VarGetsDeclList *vgdl = dynamic_cast<VarGetsDeclList*>((yystack_[2].value.node)) )
                vgdl->push_back( dynamic_cast<VarGetsDecl*>((yystack_[0].value.node)) );
            (yylhs.value.node) = (yystack_[2].value.node);
        }
    break;

  case 282: // VarGetsDecl: "'$'" QNAME "':='" ExprSingle
    {
      (yylhs.value.node) = new VarGetsDecl(LOC (yylhs.location),
                           static_cast<QName*>((yystack_[2].value.expr)),
                           NULL,
                           NULL,
                           (yystack_[0].value.expr));
    }
    break;

  case 283: // VarGetsDecl: "'$'" QNAME TypeDeclaration "':='" ExprSingle
    {
      (yylhs.value.node) = new VarGetsDecl(LOC (yylhs.location),
                           static_cast<QName*>((yystack_[3].value.expr)),
                           dynamic_cast<SequenceTypeAST *>((yystack_[2].value.node)),
                           NULL,
                           (yystack_[0].value.expr));
    }
    break;

  case 284: // VarGetsDecl: FTScoreVar "':='" ExprSingle
    {
      (yylhs.value.node) = new VarGetsDecl(LOC (yylhs.location),
                           dynamic_cast<FTScoreVar*>((yystack_[2].value.node))->get_var_name(),
                           NULL,
                           dynamic_cast<FTScoreVar*>((yystack_[2].value.node)),
                           (yystack_[0].value.expr));
     }
    break;

  case 285: // VarGetsDecl: "'$'" QNAME TypeDeclaration FTScoreVar "':='" ExprSingle
    {
      (yylhs.value.node) = new VarGetsDecl(LOC (yylhs.location),
                           static_cast<QName*>((yystack_[4].value.expr)),
                           dynamic_cast<SequenceTypeAST *>((yystack_[3].value.node)),
                           dynamic_cast<FTScoreVar*>((yystack_[2].value.node)),
                           (yystack_[0].value.expr));
    }
    break;

  case 286: // WindowVarDecl: "'$'" QNAME "'in'" ExprSingle
    {
      (yylhs.value.node) = new WindowVarDecl(LOC (yylhs.location),
                             static_cast<QName*>((yystack_[2].value.expr)),
                             NULL, (yystack_[0].value.expr));
    }
    break;

  case 287: // WindowVarDecl: "'$'" QNAME TypeDeclaration "'in'" ExprSingle
    {
      (yylhs.value.node) = new WindowVarDecl(LOC (yylhs.location),
                             static_cast<QName*>((yystack_[3].value.expr)),
                             dynamic_cast<SequenceTypeAST *>((yystack_[2].value.node)),
                             (yystack_[0].value.expr));
    }
    break;

  case 288: // WindowVars: WindowVars3
    { (yylhs.value.node) = (yystack_[0].value.node); }
    break;

  case 289: // WindowVars: "'$'" QNAME
    {
      (yylhs.value.node) = new WindowVars(LOC(yylhs.location), NULL, static_cast<QName*>((yystack_[0].value.expr)), NULL, NULL);
    }
    break;

  case 290: // WindowVars: "'$'" QNAME WindowVars3
    {
      (yylhs.value.node) = (yystack_[0].value.node);
      dynamic_cast<WindowVars *>((yylhs.value.node))->set_curr(static_cast<QName*>((yystack_[1].value.expr)));
    }
    break;

  case 291: // WindowVars3: PositionalVar
    {
      (yylhs.value.node) = new WindowVars(LOC(yylhs.location), dynamic_cast<PositionalVar*>((yystack_[0].value.node)), NULL, NULL, NULL);
    }
    break;

  case 292: // WindowVars3: PositionalVar WindowVars2
    {
      (yylhs.value.node) = (yystack_[0].value.node);
      dynamic_cast<WindowVars *>((yylhs.value.node))->set_posvar(dynamic_cast<PositionalVar*>((yystack_[1].value.node)));
    }
    break;

  case 293: // WindowVars3: WindowVars2
    { (yylhs.value.node) = (yystack_[0].value.node); }
    break;

  case 294: // WindowVars2: "'previous'" "'$'" QNAME "'next'" "'$'" QNAME
    {
      (yylhs.value.node) = new WindowVars(LOC(yylhs.location), NULL, NULL, static_cast<QName*>((yystack_[3].value.expr)), static_cast<QName*>((yystack_[0].value.expr)));
    }
    break;

  case 295: // WindowVars2: "'next'" "'$'" QNAME
    {
      (yylhs.value.node) = new WindowVars(LOC(yylhs.location), NULL, NULL, NULL, static_cast<QName*>((yystack_[0].value.expr)));
    }
    break;

  case 296: // WindowVars2: "'previous'" "'$'" QNAME
    {
      (yylhs.value.node) = new WindowVars(LOC(yylhs.location), NULL, NULL, static_cast<QName*>((yystack_[0].value.expr)), NULL);
    }
    break;

  case 297: // WhereClause: "'where'" ExprSingle
    {
      (yylhs.value.node) = new WhereClause(LOC (yylhs.location), (yystack_[0].value.expr));
    }
    break;

  case 298: // GroupByClause: "'group'" "'by'" GroupSpecList
    {
      (yylhs.value.node) = new GroupByClause(LOC(yylhs.location), dynamic_cast<GroupSpecList*>((yystack_[0].value.node)));
    }
    break;

  case 299: // GroupSpecList: GroupSpec
    {
      GroupSpecList* gsl = new GroupSpecList(LOC(yylhs.location));
      gsl->push_back(static_cast<GroupSpec*>((yystack_[0].value.node)));
      (yylhs.value.node) = gsl;
    }
    break;

  case 300: // GroupSpecList: GroupSpecList "','" GroupSpec
    {
      GroupSpecList* gsl = static_cast<GroupSpecList*>((yystack_[2].value.node));
      gsl->push_back(static_cast<GroupSpec*>((yystack_[0].value.node)));
      (yylhs.value.node) = gsl;
    }
    break;

  case 301: // GroupSpec: "'$'" QNAME "':='" ExprSingle
    {
      (yylhs.value.node) = new GroupSpec(LOC(yylhs.location), static_cast<QName*>((yystack_[2].value.expr)), NULL, (yystack_[0].value.expr), NULL);
    }
    break;

  case 302: // GroupSpec: "'$'" QNAME TypeDeclaration "':='" ExprSingle
    {
      (yylhs.value.node) = new GroupSpec(LOC(yylhs.location),
                         static_cast<QName*>((yystack_[3].value.expr)),
                         static_cast<SequenceTypeAST*>((yystack_[2].value.node)),
                         (yystack_[0].value.expr),
                         NULL);
    }
    break;

  case 303: // GroupSpec: "'$'" QNAME TypeDeclaration "':='" ExprSingle GroupCollationSpec
    {
      (yylhs.value.node) = new GroupSpec(LOC(yylhs.location),
                         static_cast<QName*>((yystack_[4].value.expr)),
                         static_cast<SequenceTypeAST*>((yystack_[3].value.node)),
                         (yystack_[1].value.expr),
                         static_cast<GroupCollationSpec*>((yystack_[0].value.node)));
    }
    break;

  case 304: // GroupSpec: "'$'" QNAME "':='" ExprSingle GroupCollationSpec
    {
      (yylhs.value.node) = new GroupSpec(LOC(yylhs.location),
                         static_cast<QName*>((yystack_[3].value.expr)),
                         NULL,
                         (yystack_[1].value.expr),
                         static_cast<GroupCollationSpec*>((yystack_[0].value.node)));
    }
    break;

  case 305: // GroupSpec: ExprSingle
    {
      VarRef* varRef = dynamic_cast<VarRef*>((yystack_[0].value.expr));
      if (varRef != NULL)
        (yylhs.value.node) = new GroupSpec(LOC(yylhs.location), varRef, NULL, NULL, NULL);
      else
        (yylhs.value.node) = new GroupSpec(LOC(yylhs.location), NULL, NULL, (yystack_[0].value.expr), NULL);
    }
    break;

  case 306: // GroupSpec: ExprSingle GroupCollationSpec
    {
      VarRef* varRef = dynamic_cast<VarRef*>((yystack_[1].value.expr));
      if (varRef != NULL)
        (yylhs.value.node) = new GroupSpec(LOC(yylhs.location), varRef, NULL, NULL, static_cast<GroupCollationSpec*>((yystack_[0].value.node)));
      else
        (yylhs.value.node) = new GroupSpec(LOC(yylhs.location), NULL, NULL, (yystack_[1].value.expr), static_cast<GroupCollationSpec*>((yystack_[0].value.node)));
    }
    break;

  case 307: // GroupCollationSpec: "'collation'" URI_LITERAL
    {
      (yylhs.value.node) = new GroupCollationSpec( LOC(yylhs.location), SYMTAB((yystack_[0].value.sval)) );
    }
    break;

  case 308: // OrderByClause: "'order'" "'by'" OrderSpecList
        {
            (yylhs.value.node) = new OrderByClause(
                LOC(yylhs.location), dynamic_cast<OrderSpecList*>((yystack_[0].value.node))
            );
        }
    break;

  case 309: // OrderByClause: "'stable'" "'order'" "'by'" OrderSpecList
        {
            (yylhs.value.node) = new OrderByClause(
                LOC(yylhs.location), dynamic_cast<OrderSpecList*>((yystack_[0].value.node)), true
            );
        }
    break;

  case 310: // OrderSpecList: OrderSpec
        {
            OrderSpecList *osl = new OrderSpecList( LOC(yylhs.location) );
            osl->push_back( dynamic_cast<OrderSpec*>((yystack_[0].value.node)) );
            (yylhs.value.node) = osl;
        }
    break;

  case 311: // OrderSpecList: OrderSpecList "','" OrderSpec
        {
            if ( OrderSpecList* osl = dynamic_cast<OrderSpecList*>((yystack_[2].value.node)) )
                osl->push_back( dynamic_cast<OrderSpec*>((yystack_[0].value.node)) );
            (yylhs.value.node) = (yystack_[2].value.node);
        }
    break;

  case 312: // OrderSpec: ExprSingle
        {
            (yylhs.value.node) = new OrderSpec( LOC(yylhs.location), (yystack_[0].value.expr), NULL );
        }
    break;

  case 313: // OrderSpec: ExprSingle OrderModifier
        {
            (yylhs.value.node) = new OrderSpec(
                LOC(yylhs.location), (yystack_[1].value.expr), dynamic_cast<OrderModifierPN*>((yystack_[0].value.node))
            );
        }
    break;

  case 314: // OrderModifier: OrderDirSpec
        {
            (yylhs.value.node) = new OrderModifierPN(
                LOC(yylhs.location), dynamic_cast<OrderDirSpec*>((yystack_[0].value.node)), NULL, NULL
            );
        }
    break;

  case 315: // OrderModifier: OrderEmptySpec
        {
            (yylhs.value.node) = new OrderModifierPN(
                LOC(yylhs.location), NULL, dynamic_cast<OrderEmptySpec*>((yystack_[0].value.node)), NULL
            );
        }
    break;

  case 316: // OrderModifier: OrderCollationSpec
        {
            (yylhs.value.node) = new OrderModifierPN(
                LOC(yylhs.location), NULL, NULL, dynamic_cast<OrderCollationSpec*>((yystack_[0].value.node))
            );
        }
    break;

  case 317: // OrderModifier: OrderDirSpec OrderEmptySpec
        {
            (yylhs.value.node) = new OrderModifierPN(
                LOC(yylhs.location),
                dynamic_cast<OrderDirSpec*>((yystack_[1].value.node)),
                dynamic_cast<OrderEmptySpec*>((yystack_[0].value.node)),
                NULL
            );
        }
    break;

  case 318: // OrderModifier: OrderDirSpec OrderCollationSpec
        {
            (yylhs.value.node) = new OrderModifierPN(
                LOC(yylhs.location),
                dynamic_cast<OrderDirSpec*>((yystack_[1].value.node)),
                NULL,
                dynamic_cast<OrderCollationSpec*>((yystack_[0].value.node))
            );
        }
    break;

  case 319: // OrderModifier: OrderEmptySpec OrderCollationSpec
        {
            (yylhs.value.node) = new OrderModifierPN(
                LOC(yylhs.location),
                NULL,
                dynamic_cast<OrderEmptySpec*>((yystack_[1].value.node)),
                dynamic_cast<OrderCollationSpec*>((yystack_[0].value.node))
            );
        }
    break;

  case 320: // OrderModifier: OrderDirSpec OrderEmptySpec OrderCollationSpec
        {
            (yylhs.value.node) = new OrderModifierPN(
                LOC(yylhs.location),
                dynamic_cast<OrderDirSpec*>((yystack_[2].value.node)),
                dynamic_cast<OrderEmptySpec*>((yystack_[1].value.node)),
                dynamic_cast<OrderCollationSpec*>((yystack_[0].value.node))
            );
        }
    break;

  case 321: // OrderDirSpec: "'ascending'"
        {
            (yylhs.value.node) = new OrderDirSpec( LOC(yylhs.location), ParseConstants::dir_ascending );
        }
    break;

  case 322: // OrderDirSpec: "'descending'"
        {
            (yylhs.value.node) = new OrderDirSpec( LOC(yylhs.location), ParseConstants::dir_descending );
        }
    break;

  case 323: // OrderEmptySpec: "'empty'" "'greatest'"
        {
            (yylhs.value.node) = new OrderEmptySpec(
                LOC(yylhs.location), StaticContextConsts::empty_greatest
            );
        }
    break;

  case 324: // OrderEmptySpec: "'empty'" "'least'"
        {
            (yylhs.value.node) = new OrderEmptySpec(
                LOC(yylhs.location), StaticContextConsts::empty_least
            );
        }
    break;

  case 325: // OrderCollationSpec: "'collation'" URI_LITERAL
        {
            (yylhs.value.node) = new OrderCollationSpec( LOC(yylhs.location), SYMTAB((yystack_[0].value.sval)) );
        }
    break;

  case 326: // QuantifiedExpr: "'some'" "'$'" QVarInDeclList "'satisfies'" ExprSingle
        {
            (yylhs.value.expr) = new QuantifiedExpr(
                LOC(yylhs.location),
                ParseConstants::quant_some,
                dynamic_cast<QVarInDeclList*>((yystack_[2].value.node)),
                (yystack_[0].value.expr)
            );
        }
    break;

  case 327: // QuantifiedExpr: "'every'" "'$'" QVarInDeclList "'satisfies'" ExprSingle
        {
            (yylhs.value.expr) = new QuantifiedExpr(
                LOC(yylhs.location),
                ParseConstants::quant_every,
                dynamic_cast<QVarInDeclList*>((yystack_[2].value.node)),
                (yystack_[0].value.expr)
            );
        }
    break;

  case 328: // QVarInDeclList: QVarInDecl
    {
      QVarInDeclList *qvidl = new QVarInDeclList( LOC(yylhs.location) );
      qvidl->push_back( dynamic_cast<QVarInDecl*>((yystack_[0].value.node)) );
      (yylhs.value.node) = qvidl;
    }
    break;

  case 329: // QVarInDeclList: QVarInDeclList "','" "'$'" QVarInDecl
    {
      QVarInDeclList *qvidl = dynamic_cast<QVarInDeclList*>((yystack_[3].value.node));
      qvidl->push_back( dynamic_cast<QVarInDecl*>((yystack_[0].value.node)) );
      (yylhs.value.node) = (yystack_[3].value.node);
    }
    break;

  case 330: // QVarInDecl: QNAME "'in'" ExprSingle
    {
      (yylhs.value.node) = new QVarInDecl(LOC(yylhs.location), static_cast<QName*>((yystack_[2].value.expr)), (yystack_[0].value.expr));
    }
    break;

  case 331: // QVarInDecl: QNAME TypeDeclaration "'in'" ExprSingle
    {
      (yylhs.value.node) = new QVarInDecl(LOC(yylhs.location),
                          static_cast<QName*>((yystack_[3].value.expr)),
                          dynamic_cast<SequenceTypeAST *>((yystack_[2].value.node)),
                          (yystack_[0].value.expr));
    }
    break;

  case 332: // SwitchExpr: "'switch'" "'('" Expr "')'" SwitchCaseClauseList "'default'" ReturnOrSelect ExprSingle
    {
      (yylhs.value.expr) = new SwitchExpr(LOC(yylhs.location), (yystack_[5].value.expr), static_cast<SwitchCaseClauseList*>((yystack_[3].value.node)), (yystack_[0].value.expr));
    }
    break;

  case 333: // SwitchCaseClauseList: SwitchCaseClause
    {
      SwitchCaseClauseList* scc_list_p = new SwitchCaseClauseList(LOC(yylhs.location));
      scc_list_p->push_back(static_cast<SwitchCaseClause*>((yystack_[0].value.node)));
      (yylhs.value.node) = scc_list_p;
    }
    break;

  case 334: // SwitchCaseClauseList: SwitchCaseClauseList SwitchCaseClause
    {
      SwitchCaseClauseList* scc_list_p = static_cast<SwitchCaseClauseList*>((yystack_[1].value.node));
      scc_list_p->push_back(static_cast<SwitchCaseClause*>((yystack_[0].value.node)));
      (yylhs.value.node) = (yystack_[1].value.node);
    }
    break;

  case 335: // SwitchCaseClause: SwitchCaseOperandList ReturnOrSelect ExprSingle
    {
      (yylhs.value.node) = new SwitchCaseClause(LOC(yylhs.location), dynamic_cast<SwitchCaseOperandList*>((yystack_[2].value.node)), (yystack_[0].value.expr));
    }
    break;

  case 336: // SwitchCaseOperandList: "'case'" ExprSingle
    {
      SwitchCaseOperandList* sco_list_p = new SwitchCaseOperandList(LOC(yylhs.location));
      sco_list_p->push_back((yystack_[0].value.expr));
      (yylhs.value.node) = sco_list_p;
    }
    break;

  case 337: // SwitchCaseOperandList: SwitchCaseOperandList "'case'" ExprSingle
    {
      SwitchCaseOperandList* sco_list_p = static_cast<SwitchCaseOperandList*>((yystack_[2].value.node));
      sco_list_p->push_back((yystack_[0].value.expr));
      (yylhs.value.node) = (yystack_[2].value.node);
    }
    break;

  case 338: // SwitchStatement: "'switch'" "'('" Expr "')'" SwitchCaseStatementList "'default'" ReturnOrSelect Statement
    {
      (yylhs.value.expr) = new SwitchExpr(LOC(yylhs.location), (yystack_[5].value.expr), static_cast<SwitchCaseClauseList*>((yystack_[3].value.node)), (yystack_[0].value.expr));
    }
    break;

  case 339: // SwitchCaseStatementList: SwitchCaseStatement
    {
      SwitchCaseClauseList* scc_list_p = new SwitchCaseClauseList(LOC(yylhs.location));
      scc_list_p->push_back(static_cast<SwitchCaseClause*>((yystack_[0].value.node)));
      (yylhs.value.node) = scc_list_p;
    }
    break;

  case 340: // SwitchCaseStatementList: SwitchCaseStatementList SwitchCaseStatement
    {
      SwitchCaseClauseList* scc_list_p = static_cast<SwitchCaseClauseList*>((yystack_[1].value.node));
      scc_list_p->push_back(static_cast<SwitchCaseClause*>((yystack_[0].value.node)));
      (yylhs.value.node) = (yystack_[1].value.node);
    }
    break;

  case 341: // SwitchCaseStatement: SwitchCaseOperandList ReturnOrSelect Statement
    {
      (yylhs.value.node) = new SwitchCaseClause(LOC(yylhs.location), dynamic_cast<SwitchCaseOperandList*>((yystack_[2].value.node)), (yystack_[0].value.expr));
    }
    break;

  case 342: // TypeswitchExpr: "'typeswitch'" "'('" Expr "')'" CaseClauseList "'default'" ReturnOrSelect ExprSingle
    {
      (yylhs.value.expr) = new TypeswitchExpr(LOC(yylhs.location),
                              (yystack_[5].value.expr),
                              static_cast<CaseClauseList*>((yystack_[3].value.node)),
                              (yystack_[0].value.expr));
    }
    break;

  case 343: // TypeswitchExpr: "'typeswitch'" "'('" Expr "')'" CaseClauseList "'default'" "'$'" QNAME ReturnOrSelect ExprSingle
    {
      (yylhs.value.expr) = new TypeswitchExpr(LOC (yylhs.location),
                              (yystack_[7].value.expr),
                              static_cast<CaseClauseList*>((yystack_[5].value.node)),
                              static_cast<QName*>((yystack_[2].value.expr)),
                              (yystack_[0].value.expr));
    }
    break;

  case 344: // TypeswitchStatement: "'typeswitch'" "'('" Expr "')'" CaseStatementList "'default'" ReturnOrSelect Statement
    {
      (yylhs.value.expr) = new TypeswitchExpr(LOC(yylhs.location),
                              (yystack_[5].value.expr),
                              static_cast<CaseClauseList*>((yystack_[3].value.node)),
                              (yystack_[0].value.expr));
    }
    break;

  case 345: // TypeswitchStatement: "'typeswitch'" "'('" Expr "')'" CaseClauseList "'default'" "'$'" QNAME ReturnOrSelect Statement
    {
      (yylhs.value.expr) = new TypeswitchExpr(LOC (yylhs.location),
                              (yystack_[7].value.expr),
                              static_cast<CaseClauseList*>((yystack_[5].value.node)),
                              static_cast<QName*>((yystack_[2].value.expr)),
                              (yystack_[0].value.expr));
    }
    break;

  case 346: // CaseClauseList: CaseClause
    {
      CaseClauseList* cc_list_p = new CaseClauseList(LOC (yylhs.location));
      cc_list_p->push_back(static_cast<CaseClause*>((yystack_[0].value.node)));
      (yylhs.value.node) = cc_list_p;
    }
    break;

  case 347: // CaseClauseList: CaseClauseList CaseClause
    {
      CaseClauseList* cc_list_p = dynamic_cast<CaseClauseList*>((yystack_[1].value.node));
      cc_list_p->push_back(static_cast<CaseClause*>((yystack_[0].value.node)));
      (yylhs.value.node) = (yystack_[1].value.node);
    }
    break;

  case 348: // CaseClause: "'case'" SequenceTypeList ReturnOrSelect ExprSingle
    {
      (yylhs.value.node) = new CaseClause(LOC (yylhs.location),
                          static_cast<SequenceTypeList*>((yystack_[2].value.node)),
                          (yystack_[0].value.expr));
    }
    break;

  case 349: // CaseClause: "'case'" "'$'" QNAME "'as'" SequenceTypeList ReturnOrSelect ExprSingle
    {
      (yylhs.value.node) = new CaseClause(LOC (yylhs.location),
                          static_cast<QName*>((yystack_[4].value.expr)),
                          static_cast<SequenceTypeList*>((yystack_[2].value.node)),
                          (yystack_[0].value.expr));
     }
    break;

  case 350: // CaseStatementList: CaseStatement
    {
      CaseClauseList* cc_list_p = new CaseClauseList(LOC (yylhs.location));
      cc_list_p->push_back(static_cast<CaseClause*>((yystack_[0].value.node)));
      (yylhs.value.node) = cc_list_p;
    }
    break;

  case 351: // CaseStatementList: CaseStatementList CaseStatement
    {
      CaseClauseList* cc_list_p = static_cast<CaseClauseList*>((yystack_[1].value.node));
      cc_list_p->push_back(static_cast<CaseClause*>((yystack_[0].value.node)));
      (yylhs.value.node) = (yystack_[1].value.node);
    }
    break;

  case 352: // CaseStatement: "'case'" SequenceTypeList ReturnOrSelect Statement
    {
      (yylhs.value.node) = new CaseClause(LOC (yylhs.location),
                          static_cast<SequenceTypeList*>((yystack_[2].value.node)),
                          (yystack_[0].value.expr));
    }
    break;

  case 353: // CaseStatement: "'case'" "'$'" QNAME "'as'" SequenceTypeList ReturnOrSelect Statement
    {
      (yylhs.value.node) = new CaseClause(LOC (yylhs.location),
                          static_cast<QName*>((yystack_[4].value.expr)),
                          static_cast<SequenceTypeList*>((yystack_[2].value.node)),
                          (yystack_[0].value.expr));
     }
    break;

  case 354: // SequenceTypeList: SequenceType
    {
      SequenceTypeList* seqList = new SequenceTypeList(LOC(yylhs.location));
      seqList->push_back(static_cast<SequenceTypeAST*>((yystack_[0].value.node)));
      (yylhs.value.node) = seqList;
    }
    break;

  case 355: // SequenceTypeList: SequenceTypeList "'|'" SequenceType
    {
      SequenceTypeList* seqList = static_cast<SequenceTypeList*>((yystack_[2].value.node));
      seqList->push_back(static_cast<SequenceTypeAST*>((yystack_[0].value.node)));
      (yylhs.value.node) = (yystack_[2].value.node);
    }
    break;

  case 356: // IfExpr: "'if'" "'('" Expr "')'" "'then'" ExprSingle "'else'" ExprSingle
    {
      (yylhs.value.expr) = new IfExpr(LOC (yylhs.location), (yystack_[5].value.expr), (yystack_[2].value.expr), (yystack_[0].value.expr));
    }
    break;

  case 357: // OrExpr: AndExpr
    {
      (yylhs.value.expr) = (yystack_[0].value.expr);
    }
    break;

  case 358: // OrExpr: OrExpr "'or'" AndExpr
    {
      (yylhs.value.expr) = new OrExpr( LOC(yylhs.location), (yystack_[2].value.expr), (yystack_[0].value.expr) );
    }
    break;

  case 359: // AndExpr: ComparisonExpr
        {
            (yylhs.value.expr) = (yystack_[0].value.expr);
        }
    break;

  case 360: // AndExpr: AndExpr "'and'" ComparisonExpr
        {
            (yylhs.value.expr) = new AndExpr( LOC(yylhs.location), (yystack_[2].value.expr), (yystack_[0].value.expr) );
        }
    break;

  case 361: // ComparisonExpr: FTContainsExpr
        {
            (yylhs.value.expr) = (yystack_[0].value.expr);
        }
    break;

  case 362: // ComparisonExpr: FTContainsExpr ValueComp FTContainsExpr
        {
            (yylhs.value.expr) = new ComparisonExpr(
                LOC(yylhs.location),
                dynamic_cast<ValueComp*>((yystack_[1].value.node)),
                (yystack_[2].value.expr),
                (yystack_[0].value.expr)
            );
        }
    break;

  case 363: // ComparisonExpr: FTContainsExpr NodeComp FTContainsExpr
        {
            (yylhs.value.expr) = new ComparisonExpr(
                LOC(yylhs.location), dynamic_cast<NodeComp*>((yystack_[1].value.node)), (yystack_[2].value.expr), (yystack_[0].value.expr)
            );
        }
    break;

  case 364: // ComparisonExpr: FTContainsExpr "'='" FTContainsExpr
        {
            (yylhs.value.expr) = new ComparisonExpr(
                LOC(yylhs.location),
                new GeneralComp( LOC(yylhs.location), ParseConstants::op_eq ),
                (yystack_[2].value.expr),
                (yystack_[0].value.expr)
            );
        }
    break;

  case 365: // ComparisonExpr: FTContainsExpr "'!='" FTContainsExpr
        {
            (yylhs.value.expr) = new ComparisonExpr(
                LOC(yylhs.location),
                new GeneralComp( LOC(yylhs.location), ParseConstants::op_ne ),
                (yystack_[2].value.expr),
                (yystack_[0].value.expr)
            );
        }
    break;

  case 366: // $@1: %empty
        {
            driver.getXqueryLexer()->interpretAsLessThan();
        }
    break;

  case 367: // ComparisonExpr: FTContainsExpr "'<'" $@1 FTContainsExpr
        {
            (yylhs.value.expr) = new ComparisonExpr(
                LOC(yylhs.location),
                new GeneralComp( LOC(yylhs.location), ParseConstants::op_lt ),
                (yystack_[3].value.expr),
                (yystack_[0].value.expr)
            );
        }
    break;

  case 368: // ComparisonExpr: FTContainsExpr "'<='" FTContainsExpr
        {
            (yylhs.value.expr) = new ComparisonExpr(
                LOC(yylhs.location),
                new GeneralComp( LOC(yylhs.location), ParseConstants::op_le ),
                (yystack_[2].value.expr),
                (yystack_[0].value.expr)
            );
        }
    break;

  case 369: // ComparisonExpr: FTContainsExpr "'>'" FTContainsExpr
        {
            (yylhs.value.expr) = new ComparisonExpr(
                LOC(yylhs.location),
                new GeneralComp( LOC(yylhs.location), ParseConstants::op_gt ),
                (yystack_[2].value.expr),
                (yystack_[0].value.expr)
            );
        }
    break;

  case 370: // ComparisonExpr: FTContainsExpr "'>='" FTContainsExpr
        {
            (yylhs.value.expr) = new ComparisonExpr(
                LOC(yylhs.location),
                new GeneralComp( LOC(yylhs.location), ParseConstants::op_ge ),
                (yystack_[2].value.expr),
                (yystack_[0].value.expr)
            );
        }
    break;

  case 371: // FTContainsExpr: StringConcatExpr
        {
            (yylhs.value.expr) = (yystack_[0].value.expr);
        }
    break;

  case 372: // FTContainsExpr: StringConcatExpr "'contains'" "'text'" FTSelection opt_FTIgnoreOption
        {
            (yylhs.value.expr) = new FTContainsExpr(
                LOC(yylhs.location),
                (yystack_[4].value.expr),
                dynamic_cast<FTSelection*>((yystack_[1].value.node)),
                dynamic_cast<FTIgnoreOption*>((yystack_[0].value.node))
            );
        }
    break;

  case 373: // StringConcatExpr: RangeExpr
        {
          (yylhs.value.expr) = (yystack_[0].value.expr);
        }
    break;

  case 374: // StringConcatExpr: RangeExpr "'||'" StringConcatExpr
        {
          (yylhs.value.expr) = new StringConcatExpr(LOC(yylhs.location), (yystack_[2].value.expr), (yystack_[0].value.expr));
        }
    break;

  case 375: // opt_FTIgnoreOption: %empty
        {
            (yylhs.value.node) = NULL;
        }
    break;

  case 376: // opt_FTIgnoreOption: FTIgnoreOption
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 377: // RangeExpr: AdditiveExpr
        {
            (yylhs.value.expr) = (yystack_[0].value.expr);
        }
    break;

  case 378: // RangeExpr: AdditiveExpr "'to'" AdditiveExpr
        {
            (yylhs.value.expr) = new RangeExpr( LOC(yylhs.location), (yystack_[2].value.expr), (yystack_[0].value.expr) );
        }
    break;

  case 379: // AdditiveExpr: MultiplicativeExpr
        {
            (yylhs.value.expr) = (yystack_[0].value.expr);
        }
    break;

  case 380: // AdditiveExpr: AdditiveExpr "'+'" MultiplicativeExpr
        {
            (yylhs.value.expr) = new AdditiveExpr( LOC(yylhs.location), ParseConstants::op_plus, (yystack_[2].value.expr), (yystack_[0].value.expr) );
        }
    break;

  case 381: // AdditiveExpr: AdditiveExpr "'-'" MultiplicativeExpr
        {
            (yylhs.value.expr) = new AdditiveExpr( LOC(yylhs.location), ParseConstants::op_minus, (yystack_[2].value.expr), (yystack_[0].value.expr) );
        }
    break;

  case 382: // MultiplicativeExpr: UnionExpr
        {
            (yylhs.value.expr) = (yystack_[0].value.expr);
        }
    break;

  case 383: // MultiplicativeExpr: MultiplicativeExpr "'*'" UnionExpr
        {
            (yylhs.value.expr) = new MultiplicativeExpr(
                LOC(yylhs.location), ParseConstants::op_mul, (yystack_[2].value.expr), (yystack_[0].value.expr)
            );
        }
    break;

  case 384: // MultiplicativeExpr: MultiplicativeExpr "'div'" UnionExpr
        {
            (yylhs.value.expr) = new MultiplicativeExpr(
                LOC(yylhs.location), ParseConstants::op_div, (yystack_[2].value.expr), (yystack_[0].value.expr)
            );
        }
    break;

  case 385: // MultiplicativeExpr: MultiplicativeExpr "'idiv'" UnionExpr
        {
            (yylhs.value.expr) = new MultiplicativeExpr(
                LOC(yylhs.location), ParseConstants::op_idiv, (yystack_[2].value.expr), (yystack_[0].value.expr)
            );
        }
    break;

  case 386: // MultiplicativeExpr: MultiplicativeExpr "'mod'" UnionExpr
        {
            (yylhs.value.expr) = new MultiplicativeExpr(
                LOC(yylhs.location), ParseConstants::op_mod, (yystack_[2].value.expr), (yystack_[0].value.expr)
            );
        }
    break;

  case 387: // UnionExpr: IntersectExceptExpr
        {
            (yylhs.value.expr) = (yystack_[0].value.expr);
        }
    break;

  case 388: // UnionExpr: UnionExpr "'union'" IntersectExceptExpr
        {
            (yylhs.value.expr) = new UnionExpr( LOC(yylhs.location), (yystack_[2].value.expr), (yystack_[0].value.expr) );
        }
    break;

  case 389: // UnionExpr: UnionExpr "'|'" IntersectExceptExpr
        {
            (yylhs.value.expr) = new UnionExpr( LOC(yylhs.location), (yystack_[2].value.expr), (yystack_[0].value.expr) );
        }
    break;

  case 390: // IntersectExceptExpr: InstanceofExpr
        {
            (yylhs.value.expr) = (yystack_[0].value.expr);
        }
    break;

  case 391: // IntersectExceptExpr: IntersectExceptExpr "'intersect'" InstanceofExpr
        {
            (yylhs.value.expr) = new IntersectExceptExpr(
                LOC(yylhs.location), ParseConstants::op_intersect, (yystack_[2].value.expr), (yystack_[0].value.expr)
            );
        }
    break;

  case 392: // IntersectExceptExpr: IntersectExceptExpr "'except'" InstanceofExpr
        {
            (yylhs.value.expr) = new IntersectExceptExpr(
                LOC(yylhs.location), ParseConstants::op_except, (yystack_[2].value.expr), (yystack_[0].value.expr)
            );
        }
    break;

  case 393: // InstanceofExpr: TreatExpr
        {
            (yylhs.value.expr) = (yystack_[0].value.expr);
        }
    break;

  case 394: // InstanceofExpr: TreatExpr "'instance'" "'of'" SequenceType
        {
            (yylhs.value.expr) = new InstanceofExpr(
                LOC(yylhs.location), (yystack_[3].value.expr), dynamic_cast<SequenceTypeAST*>((yystack_[0].value.node))
            );
        }
    break;

  case 395: // TreatExpr: CastableExpr
        {
            (yylhs.value.expr) = (yystack_[0].value.expr);
        }
    break;

  case 396: // TreatExpr: CastableExpr "'treat'" "'as'" SequenceType
        {
            (yylhs.value.expr) = new TreatExpr(
                LOC(yylhs.location), (yystack_[3].value.expr), dynamic_cast<SequenceTypeAST*>((yystack_[0].value.node))
            );
        }
    break;

  case 397: // CastableExpr: CastExpr
        {
            (yylhs.value.expr) = (yystack_[0].value.expr);
        }
    break;

  case 398: // CastableExpr: CastExpr "'castable'" "'as'" SingleType
        {
            (yylhs.value.expr) = new CastableExpr(
                LOC(yylhs.location), (yystack_[3].value.expr), dynamic_cast<SingleType*>((yystack_[0].value.node))
            );
        }
    break;

  case 399: // CastExpr: UnaryExpr
        {
            (yylhs.value.expr) = (yystack_[0].value.expr);
        }
    break;

  case 400: // CastExpr: UnaryExpr "'cast'" "'as'" SingleType
        {
            (yylhs.value.expr) = new CastExpr(
                LOC(yylhs.location), (yystack_[3].value.expr), dynamic_cast<SingleType*>((yystack_[0].value.node))
            );
        }
    break;

  case 401: // SingleType: SimpleType
    {
      (yylhs.value.node) = new SingleType(LOC(yylhs.location), dynamic_cast<SimpleType*>((yystack_[0].value.node)), false);
    }
    break;

  case 402: // SingleType: SimpleType "'?'"
    {
      (yylhs.value.node) = new SingleType(LOC(yylhs.location), dynamic_cast<SimpleType*>((yystack_[1].value.node)), true);
    }
    break;

  case 403: // UnaryExpr: ValueExpr
        {
            (yylhs.value.expr) = (yystack_[0].value.expr);
        }
    break;

  case 404: // UnaryExpr: SignList ValueExpr
        {
            (yylhs.value.expr) = new UnaryExpr( LOC(yylhs.location), dynamic_cast<SignList*>((yystack_[1].value.node)), (yystack_[0].value.expr) );
        }
    break;

  case 405: // SignList: "'+'"
        {
            (yylhs.value.node) = new SignList( LOC(yylhs.location), true );
        }
    break;

  case 406: // SignList: "'-'"
        {
            (yylhs.value.node) = new SignList( LOC(yylhs.location), false );
        }
    break;

  case 407: // SignList: SignList "'+'"
        {
            (yylhs.value.node) = (yystack_[1].value.node);
        }
    break;

  case 408: // SignList: SignList "'-'"
        {
            if ( SignList *sl = dynamic_cast<SignList*>((yystack_[1].value.node)) )
                sl->negate();
            (yylhs.value.node) = (yystack_[1].value.node);
        }
    break;

  case 409: // ValueExpr: ValidateExpr
        { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 410: // ValueExpr: SimpleMapExpr
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 411: // ValueExpr: ExtensionExpr
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 412: // SimpleMapExpr: PathExpr
      {
        (yylhs.value.expr) = (yystack_[0].value.expr);
      }
    break;

  case 413: // SimpleMapExpr: SimpleMapExpr "'!'" PathExpr
      {
        (yylhs.value.expr) = new SimpleMapExpr(LOC(yylhs.location), (yystack_[2].value.expr), (yystack_[0].value.expr));
      }
    break;

  case 414: // ValueComp: "'eq'"
        {
            (yylhs.value.node) = new ValueComp( LOC(yylhs.location), ParseConstants::op_val_eq );
        }
    break;

  case 415: // ValueComp: "'ne'"
        {
            (yylhs.value.node) = new ValueComp( LOC(yylhs.location), ParseConstants::op_val_ne );
        }
    break;

  case 416: // ValueComp: "'lt'"
        {
            (yylhs.value.node) = new ValueComp( LOC(yylhs.location), ParseConstants::op_val_lt );
        }
    break;

  case 417: // ValueComp: "'le'"
        {
            (yylhs.value.node) = new ValueComp( LOC(yylhs.location), ParseConstants::op_val_le );
        }
    break;

  case 418: // ValueComp: "'gt'"
        {
            (yylhs.value.node) = new ValueComp( LOC(yylhs.location), ParseConstants::op_val_gt );
        }
    break;

  case 419: // ValueComp: "'ge'"
        {
            (yylhs.value.node) = new ValueComp( LOC(yylhs.location), ParseConstants::op_val_ge );
        }
    break;

  case 420: // NodeComp: "'is'"
        {
            (yylhs.value.node) = new NodeComp( LOC(yylhs.location), ParseConstants::op_is );
        }
    break;

  case 421: // NodeComp: "'<<'"
        {
            (yylhs.value.node) = new NodeComp( LOC(yylhs.location), ParseConstants::op_precedes );
        }
    break;

  case 422: // NodeComp: "'follows'"
        {
            (yylhs.value.node) = new NodeComp( LOC(yylhs.location), ParseConstants::op_follows );
        }
    break;

  case 423: // ValidateExpr: "'validate'" "'{'" StatementsAndExpr "'}'"
        {
            (yylhs.value.expr) = new ValidateExpr( LOC(yylhs.location), "strict", (yystack_[1].value.expr) );
        }
    break;

  case 424: // ValidateExpr: "'validate'" "'lax'" "'{'" StatementsAndExpr "'}'"
        {
            (yylhs.value.expr) = new ValidateExpr( LOC(yylhs.location), "lax", (yystack_[1].value.expr) );
        }
    break;

  case 425: // ValidateExpr: "'validate'" "'strict'" "'{'" StatementsAndExpr "'}'"
        {
            (yylhs.value.expr) = new ValidateExpr( LOC(yylhs.location), "strict", (yystack_[1].value.expr) );
        }
    break;

  case 426: // ValidateExpr: "'validate'" "'type'" TypeName "'{'" StatementsAndExpr "'}'"
        {
            (yylhs.value.expr) = new ValidateExpr(
                LOC(yylhs.location), dynamic_cast<TypeName*>((yystack_[3].value.node))->get_name(), (yystack_[1].value.expr)
            );
            delete (yystack_[3].value.node);
        }
    break;

  case 427: // ExtensionExpr: Pragma_list "'{'" StatementsAndOptionalExpr "'}'"
        {
            (yylhs.value.expr) = new ExtensionExpr(
                LOC(yylhs.location), dynamic_cast<PragmaList*>((yystack_[3].value.node)), (yystack_[1].value.expr)
            );
        }
    break;

  case 428: // Pragma_list: Pragma
        {
            PragmaList *pl = new PragmaList( LOC(yylhs.location) );
            pl->push_back( dynamic_cast<Pragma*>((yystack_[0].value.node)) );
            (yylhs.value.node) = pl;
        }
    break;

  case 429: // Pragma_list: Pragma_list Pragma
        {
            if ( PragmaList *pl = dynamic_cast<PragmaList*>((yystack_[1].value.node)) )
                pl->push_back( dynamic_cast<Pragma*>((yystack_[0].value.node)) );
            (yylhs.value.node) = (yystack_[1].value.node);
        }
    break;

  case 430: // Pragma: "'(#'" QNAME "'pragma literal'"
        {
            (yylhs.value.node) = new Pragma( LOC(yylhs.location), static_cast<QName*>((yystack_[1].value.expr)), SYMTAB((yystack_[0].value.sval)) );
        }
    break;

  case 431: // Pragma: "'(#'" "'QName #)'"
        {
            (yylhs.value.node) = new Pragma( LOC(yylhs.location), new QName( LOC(yylhs.location), SYMTAB((yystack_[0].value.sval)) ), "" );
        }
    break;

  case 432: // Pragma: "'(#'" "'EQName #)'"
        {
            (yylhs.value.node) = new Pragma( LOC(yylhs.location), new QName( LOC(yylhs.location), SYMTAB((yystack_[0].value.sval)), true ), "" );
        }
    break;

  case 433: // PathExpr: LeadingSlash
    {
      (yylhs.value.expr) = new PathExpr(LOC(yylhs.location), ParseConstants::path_leading_lone_slash, NULL);
    }
    break;

  case 434: // PathExpr: LeadingSlash RelativePathExpr
    {
      RelativePathExpr* rpe;
      rpe = new RelativePathExpr(LOC(yylhs.location), ParseConstants::st_slash, NULL, (yystack_[0].value.expr), false);
      (yylhs.value.expr) = new PathExpr(LOC(yylhs.location),
                        ParseConstants::path_leading_slash,
                        rpe);
    }
    break;

  case 435: // PathExpr: "'//'" RelativePathExpr
    {
      RelativePathExpr* rpe;
      rpe = new RelativePathExpr(LOC(yylhs.location), ParseConstants::st_slashslash, NULL, (yystack_[0].value.expr), false);
      (yylhs.value.expr) = new PathExpr(LOC(yylhs.location),
                        ParseConstants::path_leading_slashslash,
                        rpe);
    }
    break;

  case 436: // PathExpr: RelativePathExpr
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

  case 437: // LeadingSlash: "'/'"
    {
      (yylhs.value.expr) = NULL;
    }
    break;

  case 438: // RelativePathExpr: StepExpr
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

  case 439: // RelativePathExpr: StepExpr "'/'" RelativePathExpr
    {
      (yylhs.value.expr) = new RelativePathExpr(LOC(yylhs.location), ParseConstants::st_slash, (yystack_[2].value.expr), (yystack_[0].value.expr), false);
    }
    break;

  case 440: // RelativePathExpr: StepExpr "'//'" RelativePathExpr
    {
      (yylhs.value.expr) = new RelativePathExpr(LOC(yylhs.location), ParseConstants::st_slashslash, (yystack_[2].value.expr), (yystack_[0].value.expr), false);
    }
    break;

  case 441: // StepExpr: AxisStep
    { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 442: // StepExpr: PostfixExpr
    { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 443: // AxisStep: ForwardStep
        {
            (yylhs.value.expr) = new AxisStep(
                LOC(yylhs.location), dynamic_cast<ForwardStep*>((yystack_[0].value.node)), NULL
            );
        }
    break;

  case 444: // AxisStep: ForwardStep PredicateList
        {
            (yylhs.value.expr) = new AxisStep(
                LOC(yylhs.location),
                dynamic_cast<ForwardStep*>((yystack_[1].value.node)),
                dynamic_cast<PredicateList*>((yystack_[0].value.node))
            );
        }
    break;

  case 445: // AxisStep: ReverseStep
        {
            (yylhs.value.expr) = new AxisStep(
                LOC(yylhs.location), dynamic_cast<ReverseStep*>((yystack_[0].value.node)), NULL
            );
        }
    break;

  case 446: // AxisStep: ReverseStep PredicateList
        {
            (yylhs.value.expr) = new AxisStep(
                LOC(yylhs.location),
                dynamic_cast<ReverseStep*>((yystack_[1].value.node)),
                dynamic_cast<PredicateList*>((yystack_[0].value.node))
            );
        }
    break;

  case 447: // ForwardStep: ForwardAxis NodeTest
        {
            (yylhs.value.node) = new ForwardStep(
                LOC(yylhs.location), dynamic_cast<ForwardAxis*>((yystack_[1].value.node)), (yystack_[0].value.node)
            );
        }
    break;

  case 448: // ForwardStep: AbbrevForwardStep
        {
            (yylhs.value.node) = new ForwardStep(
                LOC(yylhs.location), dynamic_cast<AbbrevForwardStep*>((yystack_[0].value.node))
            );
        }
    break;

  case 449: // ForwardAxis: "'child'" "'::'"
        {
            (yylhs.value.node) = new ForwardAxis( LOC(yylhs.location), ParseConstants::axis_child );
        }
    break;

  case 450: // ForwardAxis: "'descendant'" "'::'"
        {
            (yylhs.value.node) = new ForwardAxis( LOC(yylhs.location), ParseConstants::axis_descendant);
        }
    break;

  case 451: // ForwardAxis: "'attribute'" "'::'"
        {
            (yylhs.value.node) = new ForwardAxis( LOC(yylhs.location), ParseConstants::axis_attribute );
        }
    break;

  case 452: // ForwardAxis: "'self'" "'::'"
        {
            (yylhs.value.node) = new ForwardAxis( LOC(yylhs.location), ParseConstants::axis_self );
        }
    break;

  case 453: // ForwardAxis: "'descendant-or-self'" "'::'"
        {
            (yylhs.value.node) = new ForwardAxis(
                LOC(yylhs.location), ParseConstants::axis_descendant_or_self
            );
        }
    break;

  case 454: // ForwardAxis: "'following-sibling'" "'::'"
        {
            (yylhs.value.node) = new ForwardAxis(
                LOC(yylhs.location), ParseConstants::axis_following_sibling
            );
        }
    break;

  case 455: // ForwardAxis: "'following'" "'::'"
        {
            (yylhs.value.node) = new ForwardAxis( LOC(yylhs.location), ParseConstants::axis_following );
        }
    break;

  case 456: // AbbrevForwardStep: NodeTest
        {
            (yylhs.value.node) = new AbbrevForwardStep( LOC(yylhs.location), (yystack_[0].value.node), false );
        }
    break;

  case 457: // AbbrevForwardStep: "'@'" NodeTest
        {
            (yylhs.value.node) = new AbbrevForwardStep( LOC(yylhs.location), (yystack_[0].value.node), true );
        }
    break;

  case 458: // ReverseStep: ReverseAxis NodeTest
        {
            (yylhs.value.node) = new ReverseStep( LOC(yylhs.location), dynamic_cast<ReverseAxis*>((yystack_[1].value.node)), (yystack_[0].value.node) );
        }
    break;

  case 459: // ReverseStep: "'..'"
        {
            ReverseAxis *ra = new ReverseAxis(
                LOC(yylhs.location), ParseConstants::axis_parent
            );
            (yylhs.value.node) = new ReverseStep( LOC(yylhs.location), ra, NULL );
        }
    break;

  case 460: // ReverseAxis: "'parent'" "'::'"
        {
            (yylhs.value.node) = new ReverseAxis( LOC(yylhs.location), ParseConstants::axis_parent );
        }
    break;

  case 461: // ReverseAxis: "'ancestor'" "'::'"
        {
            (yylhs.value.node) = new ReverseAxis( LOC(yylhs.location), ParseConstants::axis_ancestor );
        }
    break;

  case 462: // ReverseAxis: "'preceding-sibling'" "'::'"
        {
            (yylhs.value.node) = new ReverseAxis(
                LOC(yylhs.location), ParseConstants::axis_preceding_sibling
            );
        }
    break;

  case 463: // ReverseAxis: "'preceding'" "'::'"
        {
            (yylhs.value.node) = new ReverseAxis( LOC(yylhs.location), ParseConstants::axis_preceding );
        }
    break;

  case 464: // ReverseAxis: "'ancestor-or-self'" "'::'"
        {
            (yylhs.value.node) = new ReverseAxis(
                LOC(yylhs.location), ParseConstants::axis_ancestor_or_self
            );
        }
    break;

  case 465: // NodeTest: KindTest
        { (yylhs.value.node) = (yystack_[0].value.node); }
    break;

  case 466: // NodeTest: NameTest
      { (yylhs.value.node) = (yystack_[0].value.node); }
    break;

  case 467: // NameTest: QNAME
        {
            (yylhs.value.node) = new NameTest( LOC(yylhs.location), static_cast<QName*>((yystack_[0].value.expr)) );
        }
    break;

  case 468: // NameTest: Wildcard
        {
            (yylhs.value.node) = new NameTest( LOC(yylhs.location), dynamic_cast<Wildcard*>((yystack_[0].value.node)) );
        }
    break;

  case 469: // Wildcard: "'*'"
    {
      (yylhs.value.node) = new Wildcard(LOC(yylhs.location), "", "", ParseConstants::wild_all, false);
    }
    break;

  case 470: // Wildcard: "'pref:*'"
    {
      (yylhs.value.node) = new Wildcard(LOC(yylhs.location), SYMTAB((yystack_[0].value.sval)), "", ParseConstants::wild_elem, false);
    }
    break;

  case 471: // Wildcard: "'ns:*'"
    {
      (yylhs.value.node) = new Wildcard(LOC(yylhs.location), SYMTAB((yystack_[0].value.sval)), "", ParseConstants::wild_elem, true);
    }
    break;

  case 472: // Wildcard: "'*:QName'"
    {
      (yylhs.value.node) = new Wildcard(LOC(yylhs.location), "", SYMTAB((yystack_[0].value.sval)), ParseConstants::wild_prefix, false);
    }
    break;

  case 473: // PostfixExpr: PrimaryExpr
     {
       (yylhs.value.expr) = (yystack_[0].value.expr);
     }
    break;

  case 474: // PostfixExpr: PostfixExpr PredicateList
     {
       (yylhs.value.expr) = new FilterExpr(LOC(yylhs.location), (yystack_[1].value.expr), dynamic_cast<PredicateList*>((yystack_[0].value.node)));
     }
    break;

  case 475: // PostfixExpr: PostfixExpr "'('" "')'"
     {
       (yylhs.value.expr) = new DynamicFunctionInvocation(LOC(yylhs.location), (yystack_[2].value.expr), false);
     }
    break;

  case 476: // PostfixExpr: PostfixExpr "'('" ArgList "')'"
     {
       (yylhs.value.expr) = new DynamicFunctionInvocation(LOC(yylhs.location), (yystack_[3].value.expr), dynamic_cast<ArgList*>((yystack_[1].value.node)), false);
     }
    break;

  case 477: // PredicateList: Predicate
    {
      PredicateList* pl = new PredicateList( LOC(yylhs.location) );
      pl->push_back(dynamic_cast<exprnode*>((yystack_[0].value.expr)));
      (yylhs.value.node) = pl;
    }
    break;

  case 478: // PredicateList: PredicateList Predicate
    {
      if (PredicateList* pl = dynamic_cast<PredicateList*>((yystack_[1].value.node)))
        pl->push_back(dynamic_cast<exprnode*>((yystack_[0].value.expr)));
      (yylhs.value.node) = (yystack_[1].value.node);
    }
    break;

  case 479: // Predicate: "'['" Expr "']'"
    {
      (yylhs.value.expr) = (yystack_[1].value.expr);
    }
    break;

  case 480: // PrimaryExpr: Literal
        { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 481: // PrimaryExpr: VarRef
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 482: // PrimaryExpr: ParenthesizedExpr
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 483: // PrimaryExpr: ContextItemExpr
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 484: // PrimaryExpr: FunctionCall
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 485: // PrimaryExpr: Constructor
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 486: // PrimaryExpr: OrderedExpr
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 487: // PrimaryExpr: UnorderedExpr
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 488: // PrimaryExpr: FunctionItemExpr
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 489: // PrimaryExpr: BlockExpr
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 490: // PrimaryExpr: JSONObjectConstructor
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 491: // PrimaryExpr: JSONArrayConstructor
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 492: // PrimaryExpr: JSONSimpleObjectUnion
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 493: // PrimaryExpr: JSONAccumulatorObjectUnion
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 494: // Literal: NumericLiteral
        {
            (yylhs.value.expr) = (yystack_[0].value.expr);
        }
    break;

  case 495: // Literal: StringLiteral
        {
            (yylhs.value.expr) = (yystack_[0].value.expr);
        }
    break;

  case 496: // NumericLiteral: "'DECIMAL'"
        {
            (yylhs.value.expr) = NumericLiteral::new_literal(
                LOC(yylhs.location), ParseConstants::num_decimal, *(yystack_[0].value.decval)
            );
            delete (yystack_[0].value.decval);
        }
    break;

  case 497: // NumericLiteral: "'INTEGER'"
        {
            (yylhs.value.expr) = NumericLiteral::new_literal(
                LOC(yylhs.location), ParseConstants::num_integer, *(yystack_[0].value.ival)
            );
            delete (yystack_[0].value.ival);
        }
    break;

  case 498: // NumericLiteral: "'DOUBLE'"
        {
            (yylhs.value.expr) = NumericLiteral::new_literal(
                LOC(yylhs.location), ParseConstants::num_double, *(yystack_[0].value.dval)
            );
            delete (yystack_[0].value.dval);
        }
    break;

  case 499: // VarRef: "'$'" QNAME
        {
            (yylhs.value.expr) = new VarRef(LOC(yylhs.location), static_cast<QName*>((yystack_[0].value.expr)));
        }
    break;

  case 500: // ParenthesizedExpr: "'('" "')'"
        {
            (yylhs.value.expr) = new ParenthesizedExpr( LOC(yylhs.location), NULL);
        }
    break;

  case 501: // ParenthesizedExpr: "'('" Expr "')'"
        {
            (yylhs.value.expr) = new ParenthesizedExpr( LOC(yylhs.location), (yystack_[1].value.expr) );
        }
    break;

  case 502: // ContextItemExpr: "'.'"
        {
            driver.addCommonLanguageWarning(yystack_[0].location, ZED(ZWST0009_CONTEXT_ITEM_EXPR));
            (yylhs.value.expr) = new ContextItemExpr( LOC(yylhs.location) );
        }
    break;

  case 503: // OrderedExpr: "'ordered'" "'{'" Expr "'}'"
        {
            (yylhs.value.expr) = new OrderedExpr( LOC(yylhs.location), (yystack_[1].value.expr) );
        }
    break;

  case 504: // UnorderedExpr: "'unordered'" "'{'" Expr "'}'"
        {
            (yylhs.value.expr) = new UnorderedExpr( LOC(yylhs.location), (yystack_[1].value.expr) );
        }
    break;

  case 505: // FunctionCall: FUNCTION_NAME "'('" "')'"
    {
      (yylhs.value.expr) = new FunctionCall(LOC(yylhs.location), static_cast<QName*>((yystack_[2].value.expr)), NULL);
    }
    break;

  case 506: // FunctionCall: FUNCTION_NAME "'('" ArgList "')'"
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

  case 507: // ArgList: "'?'"
    {
      ArgList* al = new ArgList(LOC(yylhs.location));
      al->push_back(new ArgumentPlaceholder(LOC(yylhs.location)));
      (yylhs.value.node) = al;
    }
    break;

  case 508: // ArgList: ArgList "','" "'?'"
    {
      if (ArgList* al = dynamic_cast<ArgList*>((yystack_[2].value.node)))
        al->push_back(new ArgumentPlaceholder(LOC(yylhs.location)));
      (yylhs.value.node) = (yystack_[2].value.node);
    }
    break;

  case 509: // ArgList: ExprSingle
    {
      ArgList *al = new ArgList( LOC(yylhs.location) );
      al->push_back( (yystack_[0].value.expr) );
      (yylhs.value.node) = al;
    }
    break;

  case 510: // ArgList: ArgList "','" ExprSingle
    {
      if ( ArgList *al = dynamic_cast<ArgList*>((yystack_[2].value.node)) )
        al->push_back( (yystack_[0].value.expr) );
      (yylhs.value.node) = (yystack_[2].value.node);
    }
    break;

  case 511: // FunctionItemExpr: LiteralFunctionItem
    {
      (yylhs.value.expr) = (yystack_[0].value.expr);
    }
    break;

  case 512: // FunctionItemExpr: InlineFunction
    {
      (yylhs.value.expr) = (yystack_[0].value.expr);
    }
    break;

  case 513: // LiteralFunctionItem: FUNCTION_NAME "'#'" "'INTEGER'"
    {
      (yylhs.value.expr) = new LiteralFunctionItem(LOC (yylhs.location), dynamic_cast<QName*>((yystack_[2].value.expr)), (yystack_[0].value.ival));
    }
    break;

  case 514: // InlineFunction: "'function'" FunctionSig EnclosedStatementsAndOptionalExpr
    {
      (yylhs.value.expr) = new InlineFunction(LOC(yylhs.location),
                              (yystack_[1].value.fnsig)->theParams.getp(),
                              (yystack_[1].value.fnsig)->theReturnType.getp(),
                              (yystack_[0].value.expr));
      delete (yystack_[1].value.fnsig);
    }
    break;

  case 515: // InlineFunction: AnnotationList "'function'" FunctionSig EnclosedStatementsAndOptionalExpr
    {
      InlineFunction* f = new InlineFunction(LOC(yylhs.location),
                                             (yystack_[1].value.fnsig)->theParams.getp(),
                                             (yystack_[1].value.fnsig)->theReturnType.getp(),
                                             (yystack_[0].value.expr));
      f->setAnnotations(static_cast<AnnotationListParsenode*>((yystack_[3].value.node)));
      (yylhs.value.expr) = f;
      delete (yystack_[1].value.fnsig);
    }
    break;

  case 516: // Constructor: DirectConstructor
    { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 517: // Constructor: ComputedConstructor
  { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 518: // DirectConstructor: DirElemConstructor
    { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 519: // DirectConstructor: DirCommentConstructor
    { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 520: // DirectConstructor: DirPIConstructor
    { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 521: // DirElemConstructor: "'<'" QNAME OptionalBlank "'/>'"
    {
      (yylhs.value.expr) = new DirElemConstructor(LOC(yylhs.location),
                                  static_cast<QName*>((yystack_[2].value.expr)),
                                  NULL,
                                  NULL,
                                  NULL);
    }
    break;

  case 522: // DirElemConstructor: "'<'" QNAME OptionalBlank "'> (tag end)'" "'</ (start tag end)'" QNAME OptionalBlank "'> (tag end)'"
    {
      if (static_cast<QName*>((yystack_[6].value.expr))->get_qname() != static_cast<QName*>((yystack_[2].value.expr))->get_qname())
      {
        driver.parserErr("end tag </" +
                         static_cast<QName*>((yystack_[2].value.expr))->get_qname().str() +
                         "> does not match start tag <" +
                         static_cast<QName*>((yystack_[6].value.expr))->get_qname().str() + ">",
                         yystack_[3].location, err::XQST0118);
        error(yystack_[3].location, "");
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

  case 523: // DirElemConstructor: "'<'" QNAME DirAttributeList OptionalBlank "'/>'"
    {
      (yylhs.value.expr) = new DirElemConstructor(LOC(yylhs.location),
                                  static_cast<QName*>((yystack_[3].value.expr)),
                                  NULL,
                                  dynamic_cast<DirAttributeList*>((yystack_[2].value.node)),
                                  NULL);
    }
    break;

  case 524: // DirElemConstructor: "'<'" QNAME DirAttributeList OptionalBlank "'> (tag end)'" "'</ (start tag end)'" QNAME OptionalBlank "'> (tag end)'"
    {
      if (static_cast<QName*>((yystack_[7].value.expr))->get_qname() != static_cast<QName*>((yystack_[2].value.expr))->get_qname())
      {
        driver.parserErr("end tag </" +
                         static_cast<QName*>((yystack_[2].value.expr))->get_qname().str() +
                         "> does not match start tag <" +
                         static_cast<QName*>((yystack_[7].value.expr))->get_qname().str() + ">",
                         yystack_[4].location, err::XQST0118);
        error(yystack_[4].location, "");
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

  case 525: // DirElemConstructor: "'<'" QNAME OptionalBlank "'> (tag end)'" DirElemContentList "'</ (start tag end)'" QNAME OptionalBlank "'> (tag end)'"
    {
      if (static_cast<QName*>((yystack_[7].value.expr))->get_qname() != static_cast<QName*>((yystack_[2].value.expr))->get_qname())
      {
        driver.parserErr("end tag </" +
                         static_cast<QName*>((yystack_[2].value.expr))->get_qname().str() +
                         "> does not match start tag <" +
                         static_cast<QName*>((yystack_[7].value.expr))->get_qname().str() + ">",
                         yystack_[4].location, err::XQST0118);
        error(yystack_[4].location, "");
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

  case 526: // DirElemConstructor: "'<'" QNAME DirAttributeList OptionalBlank "'> (tag end)'" DirElemContentList "'</ (start tag end)'" QNAME OptionalBlank "'> (tag end)'"
    {
      if (static_cast<QName*>((yystack_[8].value.expr))->get_qname() != static_cast<QName*>((yystack_[2].value.expr))->get_qname())
      {
        driver.parserErr("end tag </" +
                         static_cast<QName*>((yystack_[2].value.expr))->get_qname().str() +
                         "> does not match start tag <" +
                         static_cast<QName*>((yystack_[8].value.expr))->get_qname().str() + ">",
                         yystack_[5].location, err::XQST0118);
        error(yystack_[5].location, "");
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

  case 527: // DirElemContentList: DirElemContent
    {
      DirElemContentList *decl = new DirElemContentList( LOC(yylhs.location) );
      decl->push_back( dynamic_cast<DirElemContent*>((yystack_[0].value.expr)) );
      (yylhs.value.node) = decl;
    }
    break;

  case 528: // DirElemContentList: DirElemContentList DirElemContent
    {
      DirElemContentList *decl = dynamic_cast<DirElemContentList*>((yystack_[1].value.node));
      if ( decl )
        decl->push_back( dynamic_cast<DirElemContent*>((yystack_[0].value.expr)) );
      (yylhs.value.node) = (yystack_[1].value.node);
    }
    break;

  case 529: // DirAttributeList: DirAttr
    {
      DirAttributeList *dal = new DirAttributeList( LOC(yylhs.location) );
      dal->push_back( dynamic_cast<DirAttr*>((yystack_[0].value.node)) );
      (yylhs.value.node) = dal;
    }
    break;

  case 530: // DirAttributeList: DirAttributeList DirAttr
    {
      DirAttributeList *dal = dynamic_cast<DirAttributeList*>((yystack_[1].value.node));
      if ( dal )
        dal->push_back( dynamic_cast<DirAttr*>((yystack_[0].value.node)) );
      (yylhs.value.node) = (yystack_[1].value.node);
    }
    break;

  case 531: // DirAttr: "'<blank>'" QNAME OptionalBlank "'='" OptionalBlank DirAttributeValue
    {
      (yylhs.value.node) = new DirAttr(LOC(yylhs.location),
                       static_cast<QName*>((yystack_[4].value.expr)),
                       dynamic_cast<DirAttributeValue*>((yystack_[0].value.node)));
    }
    break;

  case 534: // DirAttributeValue: "'\"'" opt_QuoteAttrContentList "'\"'"
    {
      (yylhs.value.node) = new DirAttributeValue(LOC(yylhs.location),
                                 dynamic_cast<QuoteAttrContentList*>((yystack_[1].value.node)));
    }
    break;

  case 535: // DirAttributeValue: "'''" opt_AposAttrContentList "'''"
    {
      (yylhs.value.node) = new DirAttributeValue( LOC(yylhs.location),
                                  dynamic_cast<AposAttrContentList*>((yystack_[1].value.node)));
    }
    break;

  case 536: // opt_QuoteAttrContentList: %empty
    {
      (yylhs.value.node) = new QuoteAttrContentList( LOC(yylhs.location) );
    }
    break;

  case 537: // opt_QuoteAttrContentList: QuoteAttrContentList
    {
      (yylhs.value.node) = (yystack_[0].value.node);
    }
    break;

  case 538: // QuoteAttrContentList: "'\"\"'"
    {
      QuoteAttrContentList *qacl = new QuoteAttrContentList( LOC(yylhs.location) );
      qacl->push_back( new QuoteAttrValueContent( LOC(yylhs.location), "\"" ) );
      (yylhs.value.node) = qacl;
    }
    break;

  case 539: // QuoteAttrContentList: QuoteAttrValueContent
    {
      QuoteAttrContentList *qacl = new QuoteAttrContentList( LOC(yylhs.location) );
      qacl->push_back( dynamic_cast<QuoteAttrValueContent*>((yystack_[0].value.node)) );
      (yylhs.value.node) = qacl;
    }
    break;

  case 540: // QuoteAttrContentList: QuoteAttrContentList "'\"\"'"
    {
      QuoteAttrContentList* qacl = dynamic_cast<QuoteAttrContentList*>((yystack_[1].value.node));
      if ( qacl )
        qacl->push_back( new QuoteAttrValueContent( LOC(yylhs.location), "\"" ) );
      (yylhs.value.node) = (yystack_[1].value.node);
    }
    break;

  case 541: // QuoteAttrContentList: QuoteAttrContentList QuoteAttrValueContent
    {
      QuoteAttrContentList *qacl = dynamic_cast<QuoteAttrContentList*>((yystack_[1].value.node));
      if ( qacl )
        qacl->push_back( dynamic_cast<QuoteAttrValueContent*>((yystack_[0].value.node)) );
      (yylhs.value.node) = (yystack_[1].value.node);
    }
    break;

  case 542: // opt_AposAttrContentList: %empty
    {
      (yylhs.value.node) = new AposAttrContentList( LOC(yylhs.location) );
    }
    break;

  case 543: // opt_AposAttrContentList: AposAttrContentList
    {
      (yylhs.value.node) = (yystack_[0].value.node);
    }
    break;

  case 544: // AposAttrContentList: "''''"
    {
      AposAttrContentList *aacl = new AposAttrContentList( LOC(yylhs.location) );
      aacl->push_back( new AposAttrValueContent( LOC(yylhs.location),"'") );
      (yylhs.value.node) = aacl;
    }
    break;

  case 545: // AposAttrContentList: AposAttrValueContent
    {
      AposAttrContentList *aacl = new AposAttrContentList( LOC(yylhs.location) );
      aacl->push_back( dynamic_cast<AposAttrValueContent*>((yystack_[0].value.node)) );
      (yylhs.value.node) = aacl;
    }
    break;

  case 546: // AposAttrContentList: AposAttrContentList "''''"
    {
      AposAttrContentList *aacl = dynamic_cast<AposAttrContentList*>((yystack_[1].value.node));
      if (aacl)
        aacl->push_back( new AposAttrValueContent( LOC(yylhs.location),"'") );
      (yylhs.value.node) = (yystack_[1].value.node);
    }
    break;

  case 547: // AposAttrContentList: AposAttrContentList AposAttrValueContent
    {
      AposAttrContentList *aacl = dynamic_cast<AposAttrContentList*>((yystack_[1].value.node));
      if ( aacl )
        aacl->push_back( dynamic_cast<AposAttrValueContent*>((yystack_[0].value.node)) );
      (yylhs.value.node) = (yystack_[1].value.node);
    }
    break;

  case 548: // QuoteAttrValueContent: "'quote attribute content'"
    {
      (yylhs.value.node) = new QuoteAttrValueContent( LOC(yylhs.location), SYMTAB((yystack_[0].value.sval)) );
    }
    break;

  case 549: // QuoteAttrValueContent: CommonContent
    {
      (yylhs.value.node) = new QuoteAttrValueContent(LOC(yylhs.location), dynamic_cast<CommonContent*>((yystack_[0].value.expr)));
    }
    break;

  case 550: // AposAttrValueContent: "'apos attribute content'"
    {
      (yylhs.value.node) = new AposAttrValueContent( LOC(yylhs.location), SYMTAB((yystack_[0].value.sval)) );
    }
    break;

  case 551: // AposAttrValueContent: CommonContent
    {
      (yylhs.value.node) = new AposAttrValueContent(LOC(yylhs.location), dynamic_cast<CommonContent*>((yystack_[0].value.expr)));
    }
    break;

  case 552: // DirElemContent: DirectConstructor
    {
      (yylhs.value.expr) = new DirElemContent( LOC(yylhs.location), (yystack_[0].value.expr) );
    }
    break;

  case 553: // DirElemContent: "'element content'"
    {
      (yylhs.value.expr) = new DirElemContent( LOC(yylhs.location), SYMTAB((yystack_[0].value.sval)) );
    }
    break;

  case 554: // DirElemContent: CDataSection
    {
      rchandle<CDataSection> cdata_h = dynamic_cast<CDataSection*>((yystack_[0].value.expr));
      (yylhs.value.expr) = new DirElemContent( LOC(yylhs.location), cdata_h );
    }
    break;

  case 555: // DirElemContent: CommonContent
    {
      rchandle<CommonContent> cont_h = dynamic_cast<CommonContent*>((yystack_[0].value.expr));
      (yylhs.value.expr) = new DirElemContent( LOC(yylhs.location), cont_h );
    }
    break;

  case 556: // CommonContent: "'&#charref;'"
    {
      (yylhs.value.expr) = new CommonContent(LOC(yylhs.location), ParseConstants::cont_charref, SYMTAB((yystack_[0].value.sval)));
    }
    break;

  case 557: // CommonContent: "'{{'"
    {
      (yylhs.value.expr) = new CommonContent(LOC(yylhs.location), ParseConstants::cont_escape_lbrace);
    }
    break;

  case 558: // CommonContent: "'<double {>'"
    {
      (yylhs.value.expr) = new CommonContent(LOC(yylhs.location), ParseConstants::cont_escape_rbrace);
    }
    break;

  case 559: // CommonContent: "'{'" StatementsAndExpr "'}'"
    {
      (yylhs.value.expr) = new CommonContent(LOC(yylhs.location), new EnclosedExpr(LOC(yylhs.location), (yystack_[1].value.expr)));
    }
    break;

  case 560: // DirCommentConstructor: "'<!--'" "'XML comment'" "'-->'"
    {
      (yylhs.value.expr) = new DirCommentConstructor( LOC(yylhs.location), SYMTAB((yystack_[1].value.sval)) );
    }
    break;

  case 561: // DirCommentConstructor: "'<!--'" "'-->'"
    {
      (yylhs.value.expr) = new DirCommentConstructor( LOC(yylhs.location), "" );
    }
    break;

  case 562: // DirPIConstructor: "'<?'" QNAME "'?>'"
    {
      ERROR_IF_QNAME_NOT_NCNAME((yystack_[1].value.expr), yystack_[1].location);
      (yylhs.value.expr) = new DirPIConstructor( LOC(yylhs.location), static_cast<QName*>((yystack_[1].value.expr)) );
    }
    break;

  case 563: // DirPIConstructor: "'<?'" QNAME "'char literal and pi end'"
    {
      ERROR_IF_QNAME_NOT_NCNAME((yystack_[1].value.expr), yystack_[1].location);
      (yylhs.value.expr) = new DirPIConstructor( LOC(yylhs.location), static_cast<QName*>((yystack_[1].value.expr)), SYMTAB((yystack_[0].value.sval)) );
    }
    break;

  case 564: // CDataSection: "'CDATA[['" "'char literal]]>'"
    {
      (yylhs.value.expr) = new CDataSection( LOC(yylhs.location),SYMTAB((yystack_[0].value.sval)) );
    }
    break;

  case 565: // ComputedConstructor: CompDocConstructor
    {
      (yylhs.value.expr) = (yystack_[0].value.expr);
    }
    break;

  case 566: // ComputedConstructor: CompElemConstructor
    {
      (yylhs.value.expr) = (yystack_[0].value.expr);
    }
    break;

  case 567: // ComputedConstructor: CompAttrConstructor
    {
      (yylhs.value.expr) = (yystack_[0].value.expr);
    }
    break;

  case 568: // ComputedConstructor: CompTextConstructor
    {
      (yylhs.value.expr) = (yystack_[0].value.expr);
    }
    break;

  case 569: // ComputedConstructor: CompCommentConstructor
    {
      (yylhs.value.expr) = (yystack_[0].value.expr);
    }
    break;

  case 570: // ComputedConstructor: CompPIConstructor
    {
      (yylhs.value.expr) = (yystack_[0].value.expr);
    }
    break;

  case 571: // ComputedConstructor: CompNamespaceConstructor
    { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 572: // CompDocConstructor: "'document'" "'{'" StatementsAndExpr "'}'"
    {
      (yylhs.value.expr) = new CompDocConstructor( LOC(yylhs.location), (yystack_[1].value.expr) );
    }
    break;

  case 573: // CompElemConstructor: "'element QName {'" StatementsAndOptionalExpr "'}'"
    {
      (yylhs.value.expr) = new CompElemConstructor(LOC(yylhs.location), new QName(LOC(yylhs.location), SYMTAB((yystack_[2].value.sval))), (yystack_[1].value.expr));
    }
    break;

  case 574: // CompElemConstructor: "'element EQName {'" StatementsAndOptionalExpr "'}'"
    {
      (yylhs.value.expr) = new CompElemConstructor(LOC(yylhs.location), new QName(LOC(yylhs.location), SYMTAB((yystack_[2].value.sval)), true), (yystack_[1].value.expr));
    }
    break;

  case 575: // CompElemConstructor: "'element'" "'{'" Expr "'}'" "'{'" StatementsAndOptionalExpr "'}'"
    {
      (yylhs.value.expr) = new CompElemConstructor( LOC(yylhs.location), (yystack_[4].value.expr), (yystack_[1].value.expr) );
    }
    break;

  case 576: // CompAttrConstructor: "'attribute QName {'" StatementsAndOptionalExpr "'}'"
    {
      (yylhs.value.expr) = new CompAttrConstructor( LOC(yylhs.location), new QName(LOC(yylhs.location), SYMTAB((yystack_[2].value.sval))), (yystack_[1].value.expr) );
    }
    break;

  case 577: // CompAttrConstructor: "'attribute EQName {'" StatementsAndOptionalExpr "'}'"
    {
      (yylhs.value.expr) = new CompAttrConstructor( LOC(yylhs.location), new QName(LOC(yylhs.location), SYMTAB((yystack_[2].value.sval)), true), (yystack_[1].value.expr) );
    }
    break;

  case 578: // CompAttrConstructor: "'attribute'" "'{'" Expr "'}'" "'{'" StatementsAndOptionalExpr "'}'"
    {
      (yylhs.value.expr) = new CompAttrConstructor( LOC(yylhs.location), (yystack_[4].value.expr), (yystack_[1].value.expr) );
    }
    break;

  case 579: // CompTextConstructor: "'text'" "'{'" StatementsAndExpr "'}'"
    {
      (yylhs.value.expr) = new CompTextConstructor( LOC(yylhs.location), (yystack_[1].value.expr) );
    }
    break;

  case 580: // CompCommentConstructor: "'comment'" "'{'" StatementsAndExpr "'}'"
    {
      (yylhs.value.expr) = new CompCommentConstructor( LOC(yylhs.location), (yystack_[1].value.expr) );
    }
    break;

  case 581: // CompPIConstructor: "'processing-instruction NCName {'" StatementsAndOptionalExpr "'}'"
    {
      (yylhs.value.expr) = new CompPIConstructor(LOC(yylhs.location), SYMTAB((yystack_[2].value.sval)), (yystack_[1].value.expr));
    }
    break;

  case 582: // CompPIConstructor: "'processing-instruction'" "'{'" Expr "'}'" "'{'" StatementsAndOptionalExpr "'}'"
    {
      (yylhs.value.expr) = new CompPIConstructor(LOC(yylhs.location), (yystack_[4].value.expr), (yystack_[1].value.expr));
    }
    break;

  case 583: // CompNamespaceConstructor: "'namespace NCName {'" StatementsAndExpr "'}'"
    {
      (yylhs.value.expr) = new CompNamespaceConstructor(LOC(yylhs.location), SYMTAB((yystack_[2].value.sval)), (yystack_[1].value.expr));
    }
    break;

  case 584: // CompNamespaceConstructor: "'namespace'" "'{'" Expr "'}'" "'{'" StatementsAndExpr "'}'"
    {
      (yylhs.value.expr) = new CompNamespaceConstructor(LOC(yylhs.location), (yystack_[4].value.expr), (yystack_[1].value.expr));
    }
    break;

  case 585: // TypeDeclaration: "'as'" SequenceType
    {
      (yylhs.value.node) = (yystack_[0].value.node);
    }
    break;

  case 586: // SequenceType: ItemType
        {
            (yylhs.value.node) = new SequenceTypeAST( LOC(yylhs.location), (yystack_[0].value.node), NULL );
        }
    break;

  case 587: // SequenceType: ItemType OccurrenceIndicator
        {
            (yylhs.value.node) = new SequenceTypeAST(LOC(yylhs.location), (yystack_[1].value.node), dynamic_cast<OccurrenceIndicator*>((yystack_[0].value.node)));
        }
    break;

  case 588: // SequenceType: "'empty-sequence'" "'('" "')'"
        {
            (yylhs.value.node) = new SequenceTypeAST( LOC(yylhs.location), NULL, NULL );
        }
    break;

  case 589: // OccurrenceIndicator: "'?'"
        {
            (yylhs.value.node) = new OccurrenceIndicator(
                LOC(yylhs.location), ParseConstants::occurs_optionally
            );
        }
    break;

  case 590: // OccurrenceIndicator: "'*'"
        {
            (yylhs.value.node) = new OccurrenceIndicator(
                LOC(yylhs.location), ParseConstants::occurs_zero_or_more
            );
        }
    break;

  case 591: // OccurrenceIndicator: "'+'"
        {
            (yylhs.value.node) = new OccurrenceIndicator(
                LOC(yylhs.location), ParseConstants::occurs_one_or_more
            );
        }
    break;

  case 592: // ItemType: GeneralizedAtomicType
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 593: // ItemType: KindTest
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 594: // ItemType: "'item'" "'('" "')'"
        {
            (yylhs.value.node) = new ItemType( LOC(yylhs.location), true );
        }
    break;

  case 595: // ItemType: "'structured-item'" "'('" "')'"
        {
            (yylhs.value.node) = new StructuredItemType(LOC(yylhs.location));
        }
    break;

  case 596: // ItemType: FunctionTest
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 597: // ItemType: ParenthesizedItemType
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 598: // ItemType: JSONTest
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 599: // TypeList: SequenceType
        {
          TypeList* aTypeList = new TypeList(LOC (yylhs.location));
          aTypeList->push_back(dynamic_cast<SequenceTypeAST *>((yystack_[0].value.node)));
          (yylhs.value.node) = aTypeList;
        }
    break;

  case 600: // TypeList: TypeList "','" SequenceType
        {
          TypeList* aTypeList = dynamic_cast<TypeList *>((yystack_[2].value.node));
          aTypeList->push_back(dynamic_cast<SequenceTypeAST *>((yystack_[0].value.node)));
          (yylhs.value.node) = (yystack_[2].value.node);
        }
    break;

  case 601: // GeneralizedAtomicType: QNAME
    {
      (yylhs.value.node) = new GeneralizedAtomicType( LOC(yylhs.location), static_cast<QName*>((yystack_[0].value.expr)) );
    }
    break;

  case 602: // SimpleType: QNAME
    {
      (yylhs.value.node) = new SimpleType( LOC(yylhs.location), static_cast<QName*>((yystack_[0].value.expr)) );
    }
    break;

  case 603: // KindTest: DocumentTest
    {
      (yylhs.value.node) = (yystack_[0].value.node);
    }
    break;

  case 604: // KindTest: ElementTest
    {
      (yylhs.value.node) = (yystack_[0].value.node);
    }
    break;

  case 605: // KindTest: AttributeTest
    {
      (yylhs.value.node) = (yystack_[0].value.node);
    }
    break;

  case 606: // KindTest: SchemaElementTest
    {
      (yylhs.value.node) = (yystack_[0].value.node);
    }
    break;

  case 607: // KindTest: SchemaAttributeTest
    {
      (yylhs.value.node) = (yystack_[0].value.node);
    }
    break;

  case 608: // KindTest: PITest
    {
      (yylhs.value.node) = (yystack_[0].value.node);
    }
    break;

  case 609: // KindTest: CommentTest
    {
      (yylhs.value.node) = (yystack_[0].value.node);
    }
    break;

  case 610: // KindTest: TextTest
    {
      (yylhs.value.node) = (yystack_[0].value.node);
    }
    break;

  case 611: // KindTest: NamespaceTest
    {
      (yylhs.value.node) = (yystack_[0].value.node);
    }
    break;

  case 612: // KindTest: AnyKindTest
    {
      (yylhs.value.node) = (yystack_[0].value.node);
    }
    break;

  case 613: // AnyKindTest: NODE_KIND_TEST "'('" "')'"
    {
      (yylhs.value.node) = new AnyKindTest( LOC(yylhs.location) );
    }
    break;

  case 614: // NODE_KIND_TEST: "'node'"
    {
      (yylhs.value.expr) = NULL;
    }
    break;

  case 615: // DocumentTest: "'document-node'" "'('" "')'"
    {
      (yylhs.value.node) = new DocumentTest(LOC(yylhs.location));
    }
    break;

  case 616: // DocumentTest: "'document-node'" "'('" ElementTest "')'"
    {
      (yylhs.value.node) = new DocumentTest(LOC(yylhs.location), dynamic_cast<ElementTest*>((yystack_[1].value.node)));
    }
    break;

  case 617: // DocumentTest: "'document-node'" "'('" SchemaElementTest "')'"
    {
      (yylhs.value.node) = new DocumentTest(LOC(yylhs.location), dynamic_cast<SchemaElementTest*>((yystack_[1].value.node)));
    }
    break;

  case 618: // NamespaceTest: "'namespace-node'" "'('" "')'"
    {
      (yylhs.value.node) = new NamespaceTest(LOC(yylhs.location));
    }
    break;

  case 619: // TextTest: "'text'" "'('" "')'"
    {
      (yylhs.value.node) = new TextTest(LOC(yylhs.location));
    }
    break;

  case 620: // CommentTest: "'comment'" "'('" "')'"
    {
      (yylhs.value.node) = new CommentTest(LOC(yylhs.location));
    }
    break;

  case 621: // PITest: "'processing-instruction'" "'('" "')'"
        {
            (yylhs.value.node) = new PITest( LOC(yylhs.location), "" );
        }
    break;

  case 622: // PITest: "'processing-instruction'" "'('" QNAME "')'"
        {
            ERROR_IF_QNAME_NOT_NCNAME((yystack_[1].value.expr), yystack_[1].location);
            (yylhs.value.node) = new PITest( LOC(yylhs.location), static_cast<QName*>((yystack_[1].value.expr)) );
        }
    break;

  case 623: // PITest: "'processing-instruction'" "'('" "'STRING'" "')'"
        {
            (yylhs.value.node) = new PITest( LOC(yylhs.location), SYMTAB((yystack_[1].value.sval)) );
        }
    break;

  case 624: // AttributeTest: "'attribute'" "'('" "')'"
        {
            (yylhs.value.node) = new AttributeTest( LOC(yylhs.location), NULL, NULL );
        }
    break;

  case 625: // AttributeTest: "'attribute'" "'('" QNAME "')'"
        {
            (yylhs.value.node) = new AttributeTest(
                LOC(yylhs.location), static_cast<QName*>((yystack_[1].value.expr)), NULL
            );
        }
    break;

  case 626: // AttributeTest: "'attribute'" "'('" QNAME "','" TypeName "')'"
        {
            (yylhs.value.node) = new AttributeTest(
                LOC(yylhs.location), static_cast<QName*>((yystack_[3].value.expr)), dynamic_cast<TypeName*>((yystack_[1].value.node))
            );
        }
    break;

  case 627: // AttributeTest: "'attribute'" "'('" "'*'" "')'"
        {
            (yylhs.value.node) = new AttributeTest( LOC(yylhs.location), NULL, NULL );
        }
    break;

  case 628: // AttributeTest: "'attribute'" "'('" "'*'" "','" TypeName "')'"
        {
            (yylhs.value.node) = new AttributeTest(
                LOC(yylhs.location), NULL, dynamic_cast<TypeName*>((yystack_[1].value.node))
            );
        }
    break;

  case 629: // SchemaAttributeTest: "'schema-attribute'" "'('" QNAME "')'"
        {
            (yylhs.value.node) = new SchemaAttributeTest( LOC(yylhs.location), static_cast<QName*>((yystack_[1].value.expr)) );
        }
    break;

  case 630: // ElementTest: "'element'" "'('" "')'"
        {
            (yylhs.value.node) = new ElementTest( LOC(yylhs.location), NULL, NULL, true );
        }
    break;

  case 631: // ElementTest: "'element'" "'('" QNAME "')'"
        {
            (yylhs.value.node) = new ElementTest(
                LOC(yylhs.location), static_cast<QName*>((yystack_[1].value.expr)), NULL, true
            );
        }
    break;

  case 632: // ElementTest: "'element'" "'('" QNAME "','" TypeName "')'"
        {
            (yylhs.value.node) = new ElementTest(
                LOC(yylhs.location),
                static_cast<QName*>((yystack_[3].value.expr)),
                dynamic_cast<TypeName*>((yystack_[1].value.node)),
                false
            );
        }
    break;

  case 633: // ElementTest: "'element'" "'('" QNAME "','" TypeName_WITH_HOOK "')'"
        {
            (yylhs.value.node) = new ElementTest(
                LOC(yylhs.location),
                static_cast<QName*>((yystack_[3].value.expr)),
                dynamic_cast<TypeName*>((yystack_[1].value.node)),
                true
            );
        }
    break;

  case 634: // ElementTest: "'element'" "'('" "'*'" "')'"
        {
            (yylhs.value.node) = new ElementTest(
                LOC(yylhs.location), NULL, NULL, true
            );
        }
    break;

  case 635: // ElementTest: "'element'" "'('" "'*'" "','" TypeName "')'"
        {
            (yylhs.value.node) = new ElementTest(
                LOC(yylhs.location), NULL, dynamic_cast<TypeName*>((yystack_[1].value.node)), false
            );
        }
    break;

  case 636: // ElementTest: "'element'" "'('" "'*'" "','" TypeName_WITH_HOOK "')'"
        {
            (yylhs.value.node) = new ElementTest(
                LOC(yylhs.location), NULL, dynamic_cast<TypeName*>((yystack_[1].value.node)), true
            );
        }
    break;

  case 637: // SchemaElementTest: "'schema-element'" "'('" QNAME "')'"
        {
            (yylhs.value.node) = new SchemaElementTest( LOC(yylhs.location), static_cast<QName*>((yystack_[1].value.expr)) );
        }
    break;

  case 638: // TypeName: QNAME
        {
            (yylhs.value.node) = new TypeName( LOC(yylhs.location), static_cast<QName*>((yystack_[0].value.expr)) );
        }
    break;

  case 639: // TypeName_WITH_HOOK: QNAME "'?'"
        {
            (yylhs.value.node) = new TypeName( LOC(yylhs.location), static_cast<QName*>((yystack_[1].value.expr)), true );
        }
    break;

  case 640: // StringLiteral: "'STRING'"
        {
            (yylhs.value.expr) = new StringLiteral( LOC(yylhs.location), SYMTAB((yystack_[0].value.sval)) );
        }
    break;

  case 641: // FunctionTest: AnyFunctionTest
    {
      (yylhs.value.node) = (yystack_[0].value.node);
    }
    break;

  case 642: // FunctionTest: TypedFunctionTest
    {
      (yylhs.value.node) = (yystack_[0].value.node);
    }
    break;

  case 643: // FunctionTest: AnnotationList AnyFunctionTest
    {
      static_cast<AnyFunctionTest*>((yystack_[0].value.node))->
      setAnnotations(static_cast<AnnotationListParsenode*>((yystack_[1].value.node)));
      (yylhs.value.node) = (yystack_[0].value.node);
    }
    break;

  case 644: // FunctionTest: AnnotationList TypedFunctionTest
    {
      static_cast<TypedFunctionTest*>((yystack_[0].value.node))->
      setAnnotations(static_cast<AnnotationListParsenode*>((yystack_[1].value.node)));
      (yylhs.value.node) = (yystack_[0].value.node);
    }
    break;

  case 645: // AnyFunctionTest: "'function'" "'('" "'*'" "')'"
    {
      (yylhs.value.node) = new AnyFunctionTest(LOC(yylhs.location));
    }
    break;

  case 646: // TypedFunctionTest: "'function'" "'('" "')'" "'as'" SequenceType
        {
          (yylhs.value.node) = new TypedFunctionTest(LOC (yylhs.location), dynamic_cast<SequenceTypeAST *>((yystack_[0].value.node)));
        }
    break;

  case 647: // TypedFunctionTest: "'function'" "'('" TypeList "')'" "'as'" SequenceType
        {
          (yylhs.value.node) = new TypedFunctionTest(LOC (yylhs.location),
              dynamic_cast<TypeList *>((yystack_[3].value.node)),
              dynamic_cast<SequenceTypeAST *>((yystack_[0].value.node)));
        }
    break;

  case 648: // ParenthesizedItemType: "'('" ItemType "')'"
        {
           (yylhs.value.node) = (yystack_[1].value.node);
        }
    break;

  case 649: // RevalidationDecl: "'declare'" "'revalidation'" "'strict'"
        {
            (yylhs.value.node) = new RevalidationDecl(
                LOC(yylhs.location), StaticContextConsts::strict_validation
            );
        }
    break;

  case 650: // RevalidationDecl: "'declare'" "'revalidation'" "'lax'"
        {
            (yylhs.value.node) = new RevalidationDecl(
                LOC(yylhs.location), StaticContextConsts::lax_validation
            );
        }
    break;

  case 651: // RevalidationDecl: "'declare'" "'revalidation'" "'skip'"
        {
            (yylhs.value.node) = new RevalidationDecl(
                LOC(yylhs.location), StaticContextConsts::skip_validation
            );
        }
    break;

  case 652: // InsertExpr: "'insert'" "'node'" ExprSingle "'into'" ExprSingle
        {
            (yylhs.value.expr) = new InsertExpr( LOC(yylhs.location), store::UpdateConsts::INTO, (yystack_[2].value.expr), (yystack_[0].value.expr) );
        }
    break;

  case 653: // InsertExpr: "'insert'" "'node'" ExprSingle "'as'" "'first'" "'into'" ExprSingle
        {
            (yylhs.value.expr) = new InsertExpr(
                LOC(yylhs.location), store::UpdateConsts::AS_FIRST_INTO, (yystack_[4].value.expr), (yystack_[0].value.expr)
            );
        }
    break;

  case 654: // InsertExpr: "'insert'" "'node'" ExprSingle "'as'" "'last'" "'into'" ExprSingle
        {
            (yylhs.value.expr) = new InsertExpr(
                LOC(yylhs.location), store::UpdateConsts::AS_LAST_INTO, (yystack_[4].value.expr), (yystack_[0].value.expr)
            );
        }
    break;

  case 655: // InsertExpr: "'insert'" "'node'" ExprSingle "'after'" ExprSingle
        {
            (yylhs.value.expr) = new InsertExpr( LOC(yylhs.location), store::UpdateConsts::AFTER, (yystack_[2].value.expr), (yystack_[0].value.expr) );
        }
    break;

  case 656: // InsertExpr: "'insert'" "'node'" ExprSingle "'before'" ExprSingle
        {
            (yylhs.value.expr) = new InsertExpr(
                LOC (yylhs.location), store::UpdateConsts::BEFORE, (yystack_[2].value.expr), (yystack_[0].value.expr)
            );
        }
    break;

  case 657: // InsertExpr: "'insert'" "'nodes'" ExprSingle "'into'" ExprSingle
        {
            (yylhs.value.expr) = new InsertExpr( LOC(yylhs.location), store::UpdateConsts::INTO, (yystack_[2].value.expr), (yystack_[0].value.expr) );
        }
    break;

  case 658: // InsertExpr: "'insert'" "'nodes'" ExprSingle "'as'" "'first'" "'into'" ExprSingle
        {
            (yylhs.value.expr) = new InsertExpr(
                LOC(yylhs.location), store::UpdateConsts::AS_FIRST_INTO, (yystack_[4].value.expr), (yystack_[0].value.expr)
            );
        }
    break;

  case 659: // InsertExpr: "'insert'" "'nodes'" ExprSingle "'as'" "'last'" "'into'" ExprSingle
        {
            (yylhs.value.expr) = new InsertExpr(
                  LOC(yylhs.location), store::UpdateConsts::AS_LAST_INTO, (yystack_[4].value.expr), (yystack_[0].value.expr)
            );
        }
    break;

  case 660: // InsertExpr: "'insert'" "'nodes'" ExprSingle "'after'" ExprSingle
        {
            (yylhs.value.expr) = new InsertExpr(
                LOC (yylhs.location),
                store::UpdateConsts::AFTER, (yystack_[2].value.expr), (yystack_[0].value.expr)
            );
        }
    break;

  case 661: // InsertExpr: "'insert'" "'nodes'" ExprSingle "'before'" ExprSingle
        {
            (yylhs.value.expr) = new InsertExpr(
                LOC (yylhs.location), store::UpdateConsts::BEFORE, (yystack_[2].value.expr), (yystack_[0].value.expr)
            );
        }
    break;

  case 662: // DeleteExpr: "'delete'" "'node'" ExprSingle
        {
            (yylhs.value.expr) = new DeleteExpr( LOC (yylhs.location), (yystack_[0].value.expr) );
        }
    break;

  case 663: // DeleteExpr: "'delete'" "'nodes'" ExprSingle
        {
            (yylhs.value.expr) = new DeleteExpr( LOC (yylhs.location), (yystack_[0].value.expr) );
        }
    break;

  case 664: // ReplaceExpr: "'replace'" "'node'" ExprSingle "'with'" ExprSingle
        {
            (yylhs.value.expr) = new ReplaceExpr(
                LOC(yylhs.location), store::UpdateConsts::NODE, (yystack_[2].value.expr), (yystack_[0].value.expr)
            );
        }
    break;

  case 665: // ReplaceExpr: "'replace'" "'value'" "'of'" "'node'" ExprSingle "'with'" ExprSingle
        {
            (yylhs.value.expr) = new ReplaceExpr(
                LOC(yylhs.location), store::UpdateConsts::VALUE_OF_NODE, (yystack_[2].value.expr), (yystack_[0].value.expr)
            );
        }
    break;

  case 666: // RenameExpr: "'rename'" "'node'" ExprSingle "'as'" ExprSingle
        {
            (yylhs.value.expr) = new RenameExpr( LOC (yylhs.location), (yystack_[2].value.expr), (yystack_[0].value.expr) );
        }
    break;

  case 667: // TransformExpr: "'copy'" "'$'" VarNameList "'modify'" ExprSingle ReturnOrSelect ExprSingle
    {
      CopyVarList *cvl = dynamic_cast<CopyVarList*>((yystack_[4].value.expr));
      (yylhs.value.expr) = new TransformExpr( LOC(yylhs.location), cvl, (yystack_[2].value.expr), (yystack_[0].value.expr) );
    }
    break;

  case 668: // VarNameList: VarNameDecl
    {
      CopyVarList* lList = new CopyVarList(LOC(yylhs.location));
      lList->push_back (dynamic_cast<VarBinding*> ((yystack_[0].value.expr)));
      (yylhs.value.expr) = lList;
    }
    break;

  case 669: // VarNameList: VarNameList "','" "'$'" VarNameDecl
    {
      CopyVarList* lList = dynamic_cast<CopyVarList*>((yystack_[3].value.expr));
      VarBinding* lBinding = dynamic_cast<VarBinding*>((yystack_[0].value.expr));
      lList->push_back(lBinding);
      (yylhs.value.expr) = lList;
    }
    break;

  case 670: // VarNameDecl: QNAME "':='" ExprSingle
    {
       (yylhs.value.expr) = new VarBinding(LOC(yylhs.location), static_cast<QName*>((yystack_[2].value.expr)), (yystack_[0].value.expr));
    }
    break;

  case 671: // TryExpr: "'try'" "'{'" Expr "'}'" CatchListExpr
        {
            (yylhs.value.expr) = new TryExpr( LOC(yylhs.location), (yystack_[2].value.expr), (yystack_[0].value.expr) );
        }
    break;

  case 672: // CatchListExpr: CatchExpr
        {
            CatchListExpr *cle = new CatchListExpr( LOC(yylhs.location) );
            cle->push_back( static_cast<CatchExpr*>((yystack_[0].value.expr)) );
            (yylhs.value.expr) = cle;
        }
    break;

  case 673: // CatchListExpr: CatchListExpr CatchExpr
        {
            CatchListExpr *cle = dynamic_cast<CatchListExpr*>((yystack_[1].value.expr));
            if ( cle )
                cle->push_back( static_cast<CatchExpr*>((yystack_[0].value.expr)) );
            (yylhs.value.expr) = (yystack_[1].value.expr);
        }
    break;

  case 674: // CatchExpr: "'catch'" NameTestList BracedExpr
    {
       (yylhs.value.expr) = new CatchExpr(LOC(yylhs.location), *(yystack_[1].value.name_test_list), (yystack_[0].value.expr));
       delete (yystack_[1].value.name_test_list);
    }
    break;

  case 675: // BracedExpr: "'{'" Expr "'}'"
    {
      (yylhs.value.expr) = (yystack_[1].value.expr);
    }
    break;

  case 676: // NameTestList: NameTest
        {
            CatchExpr::NameTestList *ntl = new CatchExpr::NameTestList;
            ntl->push_back( static_cast<NameTest*>((yystack_[0].value.node)) );
            (yylhs.value.name_test_list) = ntl;
        }
    break;

  case 677: // NameTestList: NameTestList "'|'" NameTest
        {
            CatchExpr::NameTestList *ntl =
                static_cast<CatchExpr::NameTestList*>((yystack_[2].value.name_test_list));
            ntl->push_back( static_cast<NameTest*>((yystack_[0].value.node)) );
            (yylhs.value.name_test_list) = ntl;
        }
    break;

  case 678: // FTSelection: FTOr opt_FTPosFilter_list
        {
            (yylhs.value.node) = new FTSelection( LOC(yylhs.location), (yystack_[1].value.node), (yystack_[0].value.pos_filter_list) );
            delete (yystack_[0].value.pos_filter_list);
        }
    break;

  case 679: // opt_FTPosFilter_list: %empty
        {
            (yylhs.value.pos_filter_list) = NULL;
        }
    break;

  case 680: // opt_FTPosFilter_list: FTPosFilter_list
        {
            (yylhs.value.pos_filter_list) = (yystack_[0].value.pos_filter_list);
        }
    break;

  case 681: // FTPosFilter_list: FTPosFilter
        {
            (yylhs.value.pos_filter_list) = new FTSelection::pos_filter_list_t;
            (yylhs.value.pos_filter_list)->push_back( dynamic_cast<FTPosFilter*>((yystack_[0].value.node)) );
        }
    break;

  case 682: // FTPosFilter_list: FTPosFilter_list FTPosFilter
        {
            (yystack_[1].value.pos_filter_list)->push_back( dynamic_cast<FTPosFilter*>((yystack_[0].value.node)) );
            (yylhs.value.pos_filter_list) = (yystack_[1].value.pos_filter_list);
        }
    break;

  case 683: // FTOr: FTAnd
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 684: // FTOr: FTOr "'ftor'" FTAnd
        {
            (yylhs.value.node) = new FTOr( LOC(yylhs.location), (yystack_[2].value.node), (yystack_[0].value.node) );
        }
    break;

  case 685: // FTAnd: FTMildNot
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 686: // FTAnd: FTAnd "'ftand'" FTMildNot
        {
            (yylhs.value.node) = new FTAnd( LOC(yylhs.location), (yystack_[2].value.node), (yystack_[0].value.node) );
        }
    break;

  case 687: // FTMildNot: FTUnaryNot
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 688: // FTMildNot: FTMildNot "'not'" "'in'" FTUnaryNot
        {
            (yylhs.value.node) = new FTMildNot( LOC(yylhs.location), (yystack_[3].value.node), (yystack_[0].value.node) );
        }
    break;

  case 689: // FTUnaryNot: FTPrimaryWithOptions
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 690: // FTUnaryNot: "'ftnot'" FTPrimaryWithOptions
        {
            (yylhs.value.node) = new FTUnaryNot(
                LOC(yylhs.location), dynamic_cast<FTPrimaryWithOptions*>((yystack_[0].value.node))
            );
        }
    break;

  case 691: // FTPrimaryWithOptions: FTPrimary opt_FTMatchOptions opt_FTWeight
        {
            (yylhs.value.node) = new FTPrimaryWithOptions(
                LOC(yylhs.location),
                dynamic_cast<FTPrimary*>((yystack_[2].value.node)),
                dynamic_cast<FTMatchOptions*>((yystack_[1].value.node)),
                dynamic_cast<FTWeight*>((yystack_[0].value.node))
            );
        }
    break;

  case 692: // opt_FTMatchOptions: %empty
        {
            (yylhs.value.node) = NULL;
        }
    break;

  case 693: // opt_FTMatchOptions: FTMatchOptions
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 694: // opt_FTWeight: %empty
        {
            (yylhs.value.node) = NULL;
        }
    break;

  case 695: // opt_FTWeight: FTWeight
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 696: // FTWeight: "'weight'" "'{'" Expr "'}'"
        {
            (yylhs.value.node) = new FTWeight( LOC(yylhs.location), dynamic_cast<exprnode*>((yystack_[1].value.expr)) );
        }
    break;

  case 697: // FTPrimary: FTWords opt_FTTimes
        {
            (yylhs.value.node) = new FTWordsTimes(
                LOC(yylhs.location),
                dynamic_cast<FTWords*>((yystack_[1].value.node)),
                dynamic_cast<FTTimes*>((yystack_[0].value.node))
            );
        }
    break;

  case 698: // FTPrimary: "'('" FTSelection "')'"
        {
            (yylhs.value.node) = (yystack_[1].value.node);
        }
    break;

  case 699: // FTPrimary: FTExtensionSelection
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 700: // opt_FTTimes: %empty
        {
            (yylhs.value.node) = NULL;
        }
    break;

  case 701: // opt_FTTimes: FTTimes
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 702: // FTExtensionSelection: Pragma_list "'{'" opt_FTSelection "'}'"
        {
            (yylhs.value.node) = new FTExtensionSelection(
                LOC(yylhs.location),
                dynamic_cast<PragmaList*>((yystack_[3].value.node)),
                dynamic_cast<FTSelection*>((yystack_[1].value.node))
            );
        }
    break;

  case 703: // opt_FTSelection: %empty
        {
            (yylhs.value.node) = NULL;
        }
    break;

  case 704: // opt_FTSelection: FTSelection
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 705: // FTWords: FTWordsValue opt_FTAnyallOption
        {
            (yylhs.value.node) = new FTWords(
                LOC(yylhs.location),
                dynamic_cast<FTWordsValue*>((yystack_[1].value.node)),
                dynamic_cast<FTAnyallOption*>((yystack_[0].value.node))
            );
        }
    break;

  case 706: // FTWordsValue: StringLiteral
        {
            (yylhs.value.node) = new FTWordsValue(
                LOC(yylhs.location), static_cast<StringLiteral*>((yystack_[0].value.expr)), NULL
            );
        }
    break;

  case 707: // FTWordsValue: "'{'" Expr "'}'"
        {
            (yylhs.value.node) = new FTWordsValue(
                LOC(yylhs.location), NULL, dynamic_cast<exprnode*>((yystack_[1].value.expr))
            );
        }
    break;

  case 708: // opt_FTAnyallOption: %empty
        {
            (yylhs.value.node) = new FTAnyallOption( LOC(yylhs.location), ft_anyall_mode::any );
        }
    break;

  case 709: // opt_FTAnyallOption: FTAnyallOption
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 710: // FTAnyallOption: "'any'" opt_word
        {
            (yylhs.value.node) = new FTAnyallOption( LOC(yylhs.location), (yystack_[0].value.ft_anyall_value) );
        }
    break;

  case 711: // FTAnyallOption: "'all'" opt_words
        {
            (yylhs.value.node) = new FTAnyallOption( LOC(yylhs.location), (yystack_[0].value.ft_anyall_value) );
        }
    break;

  case 712: // FTAnyallOption: "'phrase'"
        {
            (yylhs.value.node) = new FTAnyallOption( LOC(yylhs.location), ft_anyall_mode::phrase );
        }
    break;

  case 713: // opt_word: %empty
        {
            (yylhs.value.ft_anyall_value) = ft_anyall_mode::any;
        }
    break;

  case 714: // opt_word: "'word'"
        {
            (yylhs.value.ft_anyall_value) = ft_anyall_mode::any_word;
        }
    break;

  case 715: // opt_words: %empty
        {
            (yylhs.value.ft_anyall_value) = ft_anyall_mode::all;
        }
    break;

  case 716: // opt_words: "'words'"
        {
            (yylhs.value.ft_anyall_value) = ft_anyall_mode::all_words;
        }
    break;

  case 717: // FTPosFilter: FTOrder
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 718: // FTPosFilter: FTWindow
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 719: // FTPosFilter: FTDistance
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 720: // FTPosFilter: FTScope
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 721: // FTPosFilter: FTContent
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 722: // FTOrder: "'ordered'"
        {
            (yylhs.value.node) = new FTOrder( LOC(yylhs.location) );
        }
    break;

  case 723: // FTWindow: "'window'" AdditiveExpr FTUnit
        {
            (yylhs.value.node) = new FTWindow(
                LOC(yylhs.location),
                (yystack_[1].value.expr),
                static_cast<FTUnit*>((yystack_[0].value.node))
            );
        }
    break;

  case 724: // FTDistance: "'distance'" FTRange FTUnit
        {
            (yylhs.value.node) = new FTDistance(
                LOC(yylhs.location),
                dynamic_cast<FTRange*>((yystack_[1].value.node)),
                dynamic_cast<FTUnit*>((yystack_[0].value.node))
            );
        }
    break;

  case 725: // FTUnit: "'words'"
        {
            (yylhs.value.node) = new FTUnit( LOC(yylhs.location), ft_unit::words );
        }
    break;

  case 726: // FTUnit: "'sentences'"
        {
            (yylhs.value.node) = new FTUnit( LOC(yylhs.location), ft_unit::sentences );
        }
    break;

  case 727: // FTUnit: "'paragraphs'"
        {
            (yylhs.value.node) = new FTUnit( LOC(yylhs.location), ft_unit::paragraphs );
        }
    break;

  case 728: // FTMatchOptions: "'using'" FTMatchOption
        {
            FTMatchOptions *mo = new FTMatchOptions( LOC(yylhs.location) );
            mo->push_back( dynamic_cast<FTMatchOption*>((yystack_[0].value.node)) );
            (yylhs.value.node) = mo;
        }
    break;

  case 729: // FTMatchOptions: FTMatchOptions "'using'" FTMatchOption
        {
            FTMatchOptions *mo = dynamic_cast<FTMatchOptions*>((yystack_[2].value.node));
            mo->push_back( dynamic_cast<FTMatchOption*>((yystack_[0].value.node)) );
            (yylhs.value.node) = (yystack_[2].value.node);
        }
    break;

  case 730: // FTMatchOption: FTCaseOption
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 731: // FTMatchOption: FTDiacriticsOption
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 732: // FTMatchOption: FTExtensionOption
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 733: // FTMatchOption: FTLanguageOption
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 734: // FTMatchOption: FTStemOption
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 735: // FTMatchOption: FTStopWordOption
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 736: // FTMatchOption: FTThesaurusOption
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 737: // FTMatchOption: FTWildCardOption
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 738: // FTCaseOption: "'case'" "'sensitive'"
        {
            (yylhs.value.node) = new FTCaseOption( LOC(yylhs.location), ft_case_mode::sensitive );
        }
    break;

  case 739: // FTCaseOption: "'case'" "'insensitive'"
        {
            (yylhs.value.node) = new FTCaseOption( LOC(yylhs.location), ft_case_mode::insensitive );
        }
    break;

  case 740: // FTCaseOption: "'lowercase'"
        {
            (yylhs.value.node) = new FTCaseOption( LOC(yylhs.location), ft_case_mode::lower );
        }
    break;

  case 741: // FTCaseOption: "'uppercase'"
        {
            (yylhs.value.node) = new FTCaseOption( LOC(yylhs.location), ft_case_mode::upper );
        }
    break;

  case 742: // FTDiacriticsOption: "'diacritics'" "'sensitive'"
        {
            (yylhs.value.node) = new FTDiacriticsOption(
                LOC(yylhs.location), ft_diacritics_mode::sensitive
            );
        }
    break;

  case 743: // FTDiacriticsOption: "'diacritics'" "'insensitive'"
        {
            (yylhs.value.node) = new FTDiacriticsOption(
                LOC(yylhs.location), ft_diacritics_mode::insensitive
            );
        }
    break;

  case 744: // FTExtensionOption: "'option'" QNAME "'STRING'"
        {
            (yylhs.value.node) = new FTExtensionOption(
                LOC(yylhs.location), static_cast<QName*>((yystack_[1].value.expr)), SYMTAB((yystack_[0].value.sval))
            );
        }
    break;

  case 745: // FTStemOption: "'stemming'"
        {
            (yylhs.value.node) = new FTStemOption( LOC(yylhs.location), ft_stem_mode::stemming );
        }
    break;

  case 746: // FTStemOption: "'no'" "'stemming'"
        {
            (yylhs.value.node) = new FTStemOption( LOC(yylhs.location), ft_stem_mode::no_stemming );
        }
    break;

  case 747: // FTThesaurusOption: "'thesaurus'" FTThesaurusID_or_default
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

  case 748: // FTThesaurusOption: "'thesaurus'" "'('" FTThesaurusID_or_default opt_FTThesaurus_list "')'"
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

  case 749: // FTThesaurusOption: "'no'" "'thesaurus'"
        {
            (yylhs.value.node) = new FTThesaurusOption( LOC(yylhs.location), NULL, false, true );
        }
    break;

  case 750: // FTThesaurusID_or_default: FTThesaurusID
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 751: // FTThesaurusID_or_default: "'default'"
        {
            (yylhs.value.node) = NULL;
        }
    break;

  case 752: // opt_FTThesaurus_list: %empty
        {
            (yylhs.value.thesaurus_id_list) = NULL;
        }
    break;

  case 753: // opt_FTThesaurus_list: "','" FTThesaurus_list
        {
            (yylhs.value.thesaurus_id_list) = (yystack_[0].value.thesaurus_id_list);
        }
    break;

  case 754: // FTThesaurus_list: FTThesaurusID
        {
            (yylhs.value.thesaurus_id_list) = new FTThesaurusOption::thesaurus_id_list_t;
            (yylhs.value.thesaurus_id_list)->push_back( dynamic_cast<FTThesaurusID*>((yystack_[0].value.node)) );
        }
    break;

  case 755: // FTThesaurus_list: FTThesaurus_list "','" FTThesaurusID
        {
            (yystack_[2].value.thesaurus_id_list)->push_back( dynamic_cast<FTThesaurusID*>((yystack_[0].value.node)) );
            (yylhs.value.thesaurus_id_list) = (yystack_[2].value.thesaurus_id_list);
        }
    break;

  case 756: // FTThesaurusID: "'at'" "'STRING'" opt_relationship opt_levels
        {
            (yylhs.value.node) = new FTThesaurusID(
                LOC(yylhs.location), SYMTAB((yystack_[2].value.sval)), SYMTAB((yystack_[1].value.sval)), dynamic_cast<FTRange*>((yystack_[0].value.node))
            );
        }
    break;

  case 757: // opt_relationship: %empty
        {
            (yylhs.value.sval) = 0;
        }
    break;

  case 758: // opt_relationship: "'relationship'" "'STRING'"
        {
            (yylhs.value.sval) = (yystack_[0].value.sval);
        }
    break;

  case 759: // opt_levels: %empty
        {
            (yylhs.value.node) = NULL;
        }
    break;

  case 760: // opt_levels: FTRange "'levels'"
        {
            (yylhs.value.node) = (yystack_[1].value.node);
        }
    break;

  case 761: // FTStopWordOption: "'stop'" "'words'" FTStopWords opt_FTStopWordsInclExcl_list
        {
            (yylhs.value.node) = new FTStopWordOption(
                LOC(yylhs.location),
                dynamic_cast<FTStopWords*>((yystack_[1].value.node)), (yystack_[0].value.incl_excl_list),
                ft_stop_words_mode::with
            );
            delete (yystack_[0].value.incl_excl_list);
        }
    break;

  case 762: // FTStopWordOption: "'stop'" "'words'" "'default'" opt_FTStopWordsInclExcl_list
        {
            (yylhs.value.node) = new FTStopWordOption(
                LOC(yylhs.location), NULL, (yystack_[0].value.incl_excl_list), ft_stop_words_mode::with_default
            );
            delete (yystack_[0].value.incl_excl_list);
        }
    break;

  case 763: // FTStopWordOption: "'no'" "'stop'" "'words'"
        {
            (yylhs.value.node) = new FTStopWordOption(
                LOC(yylhs.location), NULL, NULL, ft_stop_words_mode::without
            );
        }
    break;

  case 764: // FTStopWords: "'at'" "'STRING'"
        {
            (yylhs.value.node) = new FTStopWords( LOC(yylhs.location), SYMTAB((yystack_[0].value.sval)), NULL );
        }
    break;

  case 765: // FTStopWords: "'('" STRING_LITERAL_list "')'"
        {
            (yylhs.value.node) = new FTStopWords( LOC(yylhs.location), "", (yystack_[1].value.strlist) );
        }
    break;

  case 766: // STRING_LITERAL_list: "'STRING'"
        {
            string_list_t *sl = new string_list_t;
            sl->push_back( SYMTAB((yystack_[0].value.sval)) );
            (yylhs.value.strlist) = sl;
        }
    break;

  case 767: // STRING_LITERAL_list: STRING_LITERAL_list "','" "'STRING'"
        {
            if ( (yystack_[2].value.strlist) )
                (yystack_[2].value.strlist)->push_back( SYMTAB((yystack_[0].value.sval)) );
            (yylhs.value.strlist) = (yystack_[2].value.strlist);
        }
    break;

  case 768: // opt_FTStopWordsInclExcl_list: %empty
        {
            (yylhs.value.incl_excl_list) = NULL;
        }
    break;

  case 769: // opt_FTStopWordsInclExcl_list: FTStopWordsInclExcl_list
        {
            (yylhs.value.incl_excl_list) = (yystack_[0].value.incl_excl_list);
        }
    break;

  case 770: // FTStopWordsInclExcl_list: FTStopWordsInclExcl
        {
            (yylhs.value.incl_excl_list) = new FTStopWordOption::incl_excl_list_t;
            (yylhs.value.incl_excl_list)->push_back( dynamic_cast<FTStopWordsInclExcl*>((yystack_[0].value.node)) );
        }
    break;

  case 771: // FTStopWordsInclExcl_list: FTStopWordsInclExcl_list FTStopWordsInclExcl
        {
            FTStopWordOption::incl_excl_list_t *iel = (yystack_[1].value.incl_excl_list);
            if ( !iel )
                iel = new FTStopWordOption::incl_excl_list_t;
            iel->push_back( dynamic_cast<FTStopWordsInclExcl*>((yystack_[0].value.node)) );
            (yylhs.value.incl_excl_list) = iel;
        }
    break;

  case 772: // FTStopWordsInclExcl: "'union'" FTStopWords
        {
            (yylhs.value.node) = new FTStopWordsInclExcl(
                LOC(yylhs.location),
                dynamic_cast<FTStopWords*>((yystack_[0].value.node)),
                ft_stop_words_unex::union_
            );
        }
    break;

  case 773: // FTStopWordsInclExcl: "'except'" FTStopWords
        {
            (yylhs.value.node) = new FTStopWordsInclExcl(
                LOC(yylhs.location),
                dynamic_cast<FTStopWords*>((yystack_[0].value.node)),
                ft_stop_words_unex::except
            );
        }
    break;

  case 774: // FTLanguageOption: "'language'" "'STRING'"
        {
            (yylhs.value.node) = new FTLanguageOption( LOC(yylhs.location), SYMTAB((yystack_[0].value.sval)) );
        }
    break;

  case 775: // FTWildCardOption: "'wildcards'"
        {
            (yylhs.value.node) = new FTWildCardOption( LOC(yylhs.location), ft_wild_card_mode::with );
        }
    break;

  case 776: // FTWildCardOption: "'no'" "'wildcards'"
        {
            (yylhs.value.node) = new FTWildCardOption( LOC(yylhs.location), ft_wild_card_mode::without );
        }
    break;

  case 777: // FTContent: "'at'" "'start'"
        {
            (yylhs.value.node) = new FTContent( LOC(yylhs.location), ft_content_mode::at_start );
        }
    break;

  case 778: // FTContent: "'at'" "'end'"
        {
            (yylhs.value.node) = new FTContent( LOC(yylhs.location), ft_content_mode::at_end );
        }
    break;

  case 779: // FTContent: "'entire'" "'content'"
        {
            (yylhs.value.node) = new FTContent( LOC(yylhs.location), ft_content_mode::entire );
        }
    break;

  case 780: // FTTimes: "'occurs'" FTRange "'times'"
        {
            (yylhs.value.node) = new FTTimes( LOC(yylhs.location), dynamic_cast<FTRange*>((yystack_[1].value.node)) );
        }
    break;

  case 781: // FTRange: "'exactly'" AdditiveExpr
        {
            (yylhs.value.node) = new FTRange( LOC(yylhs.location), ft_range_mode::exactly, (yystack_[0].value.expr) );
        }
    break;

  case 782: // FTRange: "'at'" "'least'" AdditiveExpr
        {
            (yylhs.value.node) = new FTRange( LOC(yylhs.location), ft_range_mode::at_least, (yystack_[0].value.expr) );
        }
    break;

  case 783: // FTRange: "'at'" "'most'" AdditiveExpr
        {
            (yylhs.value.node) = new FTRange( LOC(yylhs.location), ft_range_mode::at_most, (yystack_[0].value.expr) );
        }
    break;

  case 784: // FTRange: "'from'" AdditiveExpr "'to'" AdditiveExpr
        {
            (yylhs.value.node) = new FTRange( LOC(yylhs.location), ft_range_mode::from_to, (yystack_[2].value.expr), (yystack_[0].value.expr) );
        }
    break;

  case 785: // FTScope: "'same'" FTBigUnit
        {
            (yylhs.value.node) = new FTScope(
                LOC(yylhs.location),
                ft_scope::same,
                dynamic_cast<FTBigUnit*>((yystack_[0].value.node))
            );
        }
    break;

  case 786: // FTScope: "'different'" FTBigUnit
        {
            (yylhs.value.node) = new FTScope(
                LOC(yylhs.location),
                ft_scope::different,
                dynamic_cast<FTBigUnit*>((yystack_[0].value.node))
            );
        }
    break;

  case 787: // FTBigUnit: "'sentence'"
        {
            (yylhs.value.node) = new FTBigUnit( LOC(yylhs.location), ft_big_unit::sentence );
        }
    break;

  case 788: // FTBigUnit: "'paragraph'"
        {
            (yylhs.value.node) = new FTBigUnit( LOC(yylhs.location), ft_big_unit::paragraph );
        }
    break;

  case 789: // FTIgnoreOption: "'without'" "'content'" UnionExpr
        {
            (yylhs.value.node) = new FTIgnoreOption( LOC(yylhs.location), (yystack_[0].value.expr) );
        }
    break;

  case 790: // JSONArrayConstructor: "'['" "']'"
        {
          (yylhs.value.expr) = new JSONArrayConstructor( LOC(yylhs.location), NULL );
        }
    break;

  case 791: // JSONArrayConstructor: "'['" Expr "']'"
        {
          (yylhs.value.expr) = new JSONArrayConstructor( LOC(yylhs.location), (yystack_[1].value.expr) );
        }
    break;

  case 792: // JSONSimpleObjectUnion: "'{|'" "'|}'"
        {
          (yylhs.value.expr) = new JSONObjectConstructor(LOC(yylhs.location), NULL, false);
        }
    break;

  case 793: // JSONSimpleObjectUnion: "'{|'" Expr "'|}'"
        {
          (yylhs.value.expr) = new JSONObjectConstructor(LOC(yylhs.location), (yystack_[1].value.expr), false);
        }
    break;

  case 794: // JSONAccumulatorObjectUnion: "'{['" "']}'"
        {
          (yylhs.value.expr) = new JSONObjectConstructor(LOC(yylhs.location), NULL, true);
        }
    break;

  case 795: // JSONAccumulatorObjectUnion: "'{['" Expr "']}'"
        {
          (yylhs.value.expr) = new JSONObjectConstructor(LOC(yylhs.location), (yystack_[1].value.expr), true);
        }
    break;

  case 796: // JSONObjectConstructor: "'{'" JSONPairList "'}'"
        {
          (yylhs.value.expr) = new JSONDirectObjectConstructor(LOC(yylhs.location),
                                               dynamic_cast<JSONPairList*>((yystack_[1].value.node)));
        }
    break;

  case 797: // JSONPairList: ExprSingle "':'" ExprSingle
        {
          JSONPairList* jpl = new JSONPairList(LOC(yylhs.location));
          jpl->push_back(new JSONPairConstructor(LOC(yylhs.location), (yystack_[2].value.expr), (yystack_[0].value.expr)));
          (yylhs.value.node) = jpl;
        }
    break;

  case 798: // JSONPairList: JSONPairList "','" ExprSingle "':'" ExprSingle
        {
          JSONPairList* jpl = dynamic_cast<JSONPairList*>((yystack_[4].value.node));
          assert(jpl);
          jpl->push_back(new JSONPairConstructor(LOC(yylhs.location), (yystack_[2].value.expr), (yystack_[0].value.expr)));
          (yylhs.value.node) = jpl;
        }
    break;

  case 799: // JSONInsertExpr: "'insert'" "'json'" ExprSingle "'into'" ExprSingle
        {
          (yylhs.value.expr) = new JSONObjectInsertExpr(LOC(yylhs.location),
                                        (yystack_[2].value.expr),
                                        (yystack_[0].value.expr));
        }
    break;

  case 800: // JSONInsertExpr: "'insert'" "'json'" JSONPairList "'into'" ExprSingle
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

  case 801: // JSONInsertExpr: "'insert'" "'json'" ExprSingle "'into'" ExprSingle "'at'" "'position'" ExprSingle
        {
          (yylhs.value.expr) = new JSONArrayInsertExpr(LOC(yylhs.location), (yystack_[5].value.expr), (yystack_[3].value.expr), (yystack_[0].value.expr));
        }
    break;

  case 802: // JSONAppendExpr: "'append'" "'json'" ExprSingle "'into'" ExprSingle
        {
          (yylhs.value.expr) = new JSONArrayAppendExpr(LOC(yylhs.location), (yystack_[2].value.expr), (yystack_[0].value.expr));
        }
    break;

  case 803: // JSONDeleteExpr: "'delete'" "'json'" PostfixExpr
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

  case 804: // JSONRenameExpr: "'rename'" "'json'" PostfixExpr "'as'" ExprSingle
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

  case 805: // JSONReplaceExpr: "'replace'" "'value'" "'of'" "'json'" PostfixExpr "'with'" ExprSingle
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

  case 806: // JSONTest: "'json-item'" "'('" "')'"
        {
          (yylhs.value.node) = new JSON_Test(LOC(yylhs.location), store::StoreConsts::jsonItem);
        }
    break;

  case 807: // JSONTest: "'array'" "'('" "')'"
        {
          (yylhs.value.node) = new JSON_Test(LOC(yylhs.location), store::StoreConsts::jsonArray);
        }
    break;

  case 808: // JSONTest: "'object'" "'('" "')'"
        {
          (yylhs.value.node) = new JSON_Test(LOC(yylhs.location), store::StoreConsts::jsonObject);
        }
    break;

  case 809: // URI_LITERAL: "'STRING'"
        { (yylhs.value.sval) = (yystack_[0].value.sval); }
    break;

  case 810: // QNAME: FUNCTION_NAME
        { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 811: // QNAME: "'NCName_sval'"
                  { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB((yystack_[0].value.sval))); }
    break;

  case 812: // QNAME: "'attribute'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("attribute"))); }
    break;

  case 813: // QNAME: "'comment'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("comment"))); }
    break;

  case 814: // QNAME: "'document-node'"
                    { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("document-node"))); }
    break;

  case 815: // QNAME: "'namespace-node'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("namespace-node"))); }
    break;

  case 816: // QNAME: "'element'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("element"))); }
    break;

  case 817: // QNAME: "'item'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("item"))); }
    break;

  case 818: // QNAME: "'if'"
         { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("if"))); }
    break;

  case 819: // QNAME: "'node'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("node"))); }
    break;

  case 820: // QNAME: "'processing-instruction'"
                             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("processing-instruction"))); }
    break;

  case 821: // QNAME: "'schema-attribute'"
                       { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("schema-attribute"))); }
    break;

  case 822: // QNAME: "'schema-element'"
                     { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("schema-element"))); }
    break;

  case 823: // QNAME: "'text'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("text"))); }
    break;

  case 824: // QNAME: "'typeswitch'"
                 { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("typeswitch"))); }
    break;

  case 825: // QNAME: "'switch'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("switch"))); }
    break;

  case 826: // QNAME: "'empty-sequence'"
                     { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("empty-sequence"))); }
    break;

  case 827: // QNAME: "'while'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("while"))); }
    break;

  case 828: // QNAME: "'json'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("json"))); }
    break;

  case 829: // QNAME: "'array'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("array"))); }
    break;

  case 830: // QNAME: "'object'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("object"))); }
    break;

  case 831: // QNAME: "'json-item'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("json-item"))); }
    break;

  case 832: // QNAME: "'structured-item'"
                      { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("structured-item"))); }
    break;

  case 833: // FUNCTION_NAME: EQNAME
        { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 834: // FUNCTION_NAME: "'QName'"
                 { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB((yystack_[0].value.sval))); }
    break;

  case 835: // FUNCTION_NAME: "'xquery'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("xquery"))); }
    break;

  case 836: // FUNCTION_NAME: "'empty'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("empty"))); }
    break;

  case 837: // FUNCTION_NAME: "'boundary-space'"
                     { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("boundary-space"))); }
    break;

  case 838: // FUNCTION_NAME: "'ft-option'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("ft-option"))); }
    break;

  case 839: // FUNCTION_NAME: "'base-uri'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("base-uri"))); }
    break;

  case 840: // FUNCTION_NAME: "'lax'"
          { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("lax"))); }
    break;

  case 841: // FUNCTION_NAME: "'strict'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("strict"))); }
    break;

  case 842: // FUNCTION_NAME: "'idiv'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("idiv"))); }
    break;

  case 843: // FUNCTION_NAME: "'document'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("document"))); }
    break;

  case 844: // FUNCTION_NAME: "'ftnot'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("ftnot"))); }
    break;

  case 845: // FUNCTION_NAME: "'not'"
          { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("not"))); }
    break;

  case 846: // FUNCTION_NAME: "'sensitive'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("sensitive"))); }
    break;

  case 847: // FUNCTION_NAME: "'insensitive'"
                  { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("insensitive"))); }
    break;

  case 848: // FUNCTION_NAME: "'diacritics'"
                 { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("diacritics"))); }
    break;

  case 849: // FUNCTION_NAME: "'without'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("without"))); }
    break;

  case 850: // FUNCTION_NAME: "'stemming'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("stemming"))); }
    break;

  case 851: // FUNCTION_NAME: "'thesaurus'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("thesaurus"))); }
    break;

  case 852: // FUNCTION_NAME: "'stop'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("stop"))); }
    break;

  case 853: // FUNCTION_NAME: "'wildcards'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("wildcards"))); }
    break;

  case 854: // FUNCTION_NAME: "'entire'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("entire"))); }
    break;

  case 855: // FUNCTION_NAME: "'content'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("content"))); }
    break;

  case 856: // FUNCTION_NAME: "'word'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("word"))); }
    break;

  case 857: // FUNCTION_NAME: "'start'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("start"))); }
    break;

  case 858: // FUNCTION_NAME: "'end'"
          { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("end"))); }
    break;

  case 859: // FUNCTION_NAME: "'most'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("most"))); }
    break;

  case 860: // FUNCTION_NAME: "'skip'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("skip"))); }
    break;

  case 861: // FUNCTION_NAME: "'copy'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("copy"))); }
    break;

  case 862: // FUNCTION_NAME: "'general'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("general"))); }
    break;

  case 863: // FUNCTION_NAME: "'value'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("value"))); }
    break;

  case 864: // FUNCTION_NAME: "'eq'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("eq"))); }
    break;

  case 865: // FUNCTION_NAME: "'ne'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("ne"))); }
    break;

  case 866: // FUNCTION_NAME: "'lt'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("lt"))); }
    break;

  case 867: // FUNCTION_NAME: "'le'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("le"))); }
    break;

  case 868: // FUNCTION_NAME: "'gt'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("gt"))); }
    break;

  case 869: // FUNCTION_NAME: "'ge'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("ge"))); }
    break;

  case 870: // FUNCTION_NAME: "'at'"
         { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("at"))); }
    break;

  case 871: // FUNCTION_NAME: "'context'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("context"))); }
    break;

  case 872: // FUNCTION_NAME: "'variable'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("variable"))); }
    break;

  case 873: // FUNCTION_NAME: "'return'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("return"))); }
    break;

  case 874: // FUNCTION_NAME: "'for'"
          { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("for"))); }
    break;

  case 875: // FUNCTION_NAME: "'from'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("from"))); }
    break;

  case 876: // FUNCTION_NAME: "'allowing'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("allowing"))); }
    break;

  case 877: // FUNCTION_NAME: "'sliding'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("sliding"))); }
    break;

  case 878: // FUNCTION_NAME: "'tumbling'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("tumbling"))); }
    break;

  case 879: // FUNCTION_NAME: "'previous'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("previous"))); }
    break;

  case 880: // FUNCTION_NAME: "'next'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("next"))); }
    break;

  case 881: // FUNCTION_NAME: "'only'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("only"))); }
    break;

  case 882: // FUNCTION_NAME: "'when'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("when"))); }
    break;

  case 883: // FUNCTION_NAME: "'count'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("count"))); }
    break;

  case 884: // FUNCTION_NAME: "'in'"
          { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("in"))); }
    break;

  case 885: // FUNCTION_NAME: "'let'"
          { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("let"))); }
    break;

  case 886: // FUNCTION_NAME: "'where'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("where"))); }
    break;

  case 887: // FUNCTION_NAME: "'by'"
         { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("by"))); }
    break;

  case 888: // FUNCTION_NAME: "'group'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("group"))); }
    break;

  case 889: // FUNCTION_NAME: "'order'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("order"))); }
    break;

  case 890: // FUNCTION_NAME: "'stable'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("stable"))); }
    break;

  case 891: // FUNCTION_NAME: "'ascending'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("ascending"))); }
    break;

  case 892: // FUNCTION_NAME: "'descending'"
                 { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("descending"))); }
    break;

  case 893: // FUNCTION_NAME: "'greatest'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("greatest"))); }
    break;

  case 894: // FUNCTION_NAME: "'least'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("least"))); }
    break;

  case 895: // FUNCTION_NAME: "'collation'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("collation"))); }
    break;

  case 896: // FUNCTION_NAME: "'some'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("some"))); }
    break;

  case 897: // FUNCTION_NAME: "'every'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("every"))); }
    break;

  case 898: // FUNCTION_NAME: "'satisfies'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("satisfies"))); }
    break;

  case 899: // FUNCTION_NAME: "'case'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("case"))); }
    break;

  case 900: // FUNCTION_NAME: "'as'"
         { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("as"))); }
    break;

  case 901: // FUNCTION_NAME: "'then'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("then"))); }
    break;

  case 902: // FUNCTION_NAME: "'else'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("else"))); }
    break;

  case 903: // FUNCTION_NAME: "'or'"
         { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("or"))); }
    break;

  case 904: // FUNCTION_NAME: "'and'"
          { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("and"))); }
    break;

  case 905: // FUNCTION_NAME: "'instance'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("instance"))); }
    break;

  case 906: // FUNCTION_NAME: "'of'"
         { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("of"))); }
    break;

  case 907: // FUNCTION_NAME: "'castable'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("castable"))); }
    break;

  case 908: // FUNCTION_NAME: "'to'"
         { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("to"))); }
    break;

  case 909: // FUNCTION_NAME: "'div'"
          { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("div"))); }
    break;

  case 910: // FUNCTION_NAME: "'mod'"
          { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("mod"))); }
    break;

  case 911: // FUNCTION_NAME: "'union'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("union"))); }
    break;

  case 912: // FUNCTION_NAME: "'intersect'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("intersect"))); }
    break;

  case 913: // FUNCTION_NAME: "'except'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("except"))); }
    break;

  case 914: // FUNCTION_NAME: "'validate'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("validate"))); }
    break;

  case 915: // FUNCTION_NAME: "'type'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("type"))); }
    break;

  case 916: // FUNCTION_NAME: "'cast'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("cast"))); }
    break;

  case 917: // FUNCTION_NAME: "'treat'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("treat"))); }
    break;

  case 918: // FUNCTION_NAME: "'is'"
         { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("is"))); }
    break;

  case 919: // FUNCTION_NAME: "'preserve'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("preserve"))); }
    break;

  case 920: // FUNCTION_NAME: "'strip'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("strip"))); }
    break;

  case 921: // FUNCTION_NAME: "'namespace'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("namespace"))); }
    break;

  case 922: // FUNCTION_NAME: "'external'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("external"))); }
    break;

  case 923: // FUNCTION_NAME: "'encoding'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("encoding"))); }
    break;

  case 924: // FUNCTION_NAME: "'<no preserve>'"
                  { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("no-preserve"))); }
    break;

  case 925: // FUNCTION_NAME: "'inherit'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("inherit"))); }
    break;

  case 926: // FUNCTION_NAME: "'<no inherit>'"
                 { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("no-inherit"))); }
    break;

  case 927: // FUNCTION_NAME: "'declare'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("declare"))); }
    break;

  case 928: // FUNCTION_NAME: "'construction'"
                   { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("construction"))); }
    break;

  case 929: // FUNCTION_NAME: "'ordering'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("ordering"))); }
    break;

  case 930: // FUNCTION_NAME: "'default'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("default"))); }
    break;

  case 931: // FUNCTION_NAME: "'copy-namespaces'"
                      { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("copy-namespaces"))); }
    break;

  case 932: // FUNCTION_NAME: "'option'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("option"))); }
    break;

  case 933: // FUNCTION_NAME: "'version'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("version"))); }
    break;

  case 934: // FUNCTION_NAME: "'import'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("import"))); }
    break;

  case 935: // FUNCTION_NAME: "'schema'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("schema"))); }
    break;

  case 936: // FUNCTION_NAME: "'module'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("module"))); }
    break;

  case 937: // FUNCTION_NAME: "'function'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("function"))); }
    break;

  case 938: // FUNCTION_NAME: "'score'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("score"))); }
    break;

  case 939: // FUNCTION_NAME: "'contains'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("contains"))); }
    break;

  case 940: // FUNCTION_NAME: "'weight'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("weight"))); }
    break;

  case 941: // FUNCTION_NAME: "'window'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("window"))); }
    break;

  case 942: // FUNCTION_NAME: "'distance'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("distance"))); }
    break;

  case 943: // FUNCTION_NAME: "'occurs'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("occurs"))); }
    break;

  case 944: // FUNCTION_NAME: "'times'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("times"))); }
    break;

  case 945: // FUNCTION_NAME: "'same'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("same"))); }
    break;

  case 946: // FUNCTION_NAME: "'different'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("different"))); }
    break;

  case 947: // FUNCTION_NAME: "'lowercase'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("lowercase"))); }
    break;

  case 948: // FUNCTION_NAME: "'uppercase'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("uppercase"))); }
    break;

  case 949: // FUNCTION_NAME: "'relationship'"
                   { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("relationship"))); }
    break;

  case 950: // FUNCTION_NAME: "'levels'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("levels"))); }
    break;

  case 951: // FUNCTION_NAME: "'language'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("language"))); }
    break;

  case 952: // FUNCTION_NAME: "'any'"
          { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("any"))); }
    break;

  case 953: // FUNCTION_NAME: "'all'"
          { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("all"))); }
    break;

  case 954: // FUNCTION_NAME: "'exactly'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("exactly"))); }
    break;

  case 955: // FUNCTION_NAME: "'words'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("words"))); }
    break;

  case 956: // FUNCTION_NAME: "'sentences'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("sentences"))); }
    break;

  case 957: // FUNCTION_NAME: "'sentence'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("sentence"))); }
    break;

  case 958: // FUNCTION_NAME: "'phrase'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("phrase"))); }
    break;

  case 959: // FUNCTION_NAME: "'paragraph'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("paragraph"))); }
    break;

  case 960: // FUNCTION_NAME: "'paragraphs'"
                 { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("paragraphs"))); }
    break;

  case 961: // FUNCTION_NAME: "'modify'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("modify"))); }
    break;

  case 962: // FUNCTION_NAME: "'first'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("first"))); }
    break;

  case 963: // FUNCTION_NAME: "'append'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("append"))); }
    break;

  case 964: // FUNCTION_NAME: "'insert'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("insert"))); }
    break;

  case 965: // FUNCTION_NAME: "'replace'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("replace"))); }
    break;

  case 966: // FUNCTION_NAME: "'rename'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("rename"))); }
    break;

  case 967: // FUNCTION_NAME: "'delete'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("delete"))); }
    break;

  case 968: // FUNCTION_NAME: "'before'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("before"))); }
    break;

  case 969: // FUNCTION_NAME: "'after'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("after"))); }
    break;

  case 970: // FUNCTION_NAME: "'revalidation'"
                   { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("revalidation"))); }
    break;

  case 971: // FUNCTION_NAME: "'with'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("with"))); }
    break;

  case 972: // FUNCTION_NAME: "'nodes'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("nodes"))); }
    break;

  case 973: // FUNCTION_NAME: "'last'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("last"))); }
    break;

  case 974: // FUNCTION_NAME: "'into'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("into"))); }
    break;

  case 975: // FUNCTION_NAME: "'simple'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("simple"))); }
    break;

  case 976: // FUNCTION_NAME: "'sequential'"
                 { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("sequential"))); }
    break;

  case 977: // FUNCTION_NAME: "'updating'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("updating"))); }
    break;

  case 978: // FUNCTION_NAME: "'ordered'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("ordered"))); }
    break;

  case 979: // FUNCTION_NAME: "'unordered'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("unordered"))); }
    break;

  case 980: // FUNCTION_NAME: "'returning'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("returning"))); }
    break;

  case 981: // FUNCTION_NAME: "'exit'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("exit"))); }
    break;

  case 982: // FUNCTION_NAME: "'loop'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("loop"))); }
    break;

  case 983: // FUNCTION_NAME: "'break'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("break"))); }
    break;

  case 984: // FUNCTION_NAME: "'continue'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("continue"))); }
    break;

  case 985: // FUNCTION_NAME: "'try'"
          { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("try"))); }
    break;

  case 986: // FUNCTION_NAME: "'catch'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("catch"))); }
    break;

  case 987: // FUNCTION_NAME: "'using'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("using"))); }
    break;

  case 988: // FUNCTION_NAME: "'set'"
          { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("set"))); }
    break;

  case 989: // FUNCTION_NAME: "'index'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("index"))); }
    break;

  case 990: // FUNCTION_NAME: "'unique'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("unique"))); }
    break;

  case 991: // FUNCTION_NAME: "'non'"
          { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("non"))); }
    break;

  case 992: // FUNCTION_NAME: "'on'"
         { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("on"))); }
    break;

  case 993: // FUNCTION_NAME: "'range'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("range"))); }
    break;

  case 994: // FUNCTION_NAME: "'equality'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("equality"))); }
    break;

  case 995: // FUNCTION_NAME: "'manually'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("manually"))); }
    break;

  case 996: // FUNCTION_NAME: "'automatically'"
                    { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("automatically"))); }
    break;

  case 997: // FUNCTION_NAME: "'maintained'"
                 { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("maintained"))); }
    break;

  case 998: // FUNCTION_NAME: "'decimal-format'"
                     { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("decimal-format"))); }
    break;

  case 999: // FUNCTION_NAME: "'decimal-separator'"
                        { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("decimal-separator"))); }
    break;

  case 1000: // FUNCTION_NAME: "'grouping-separator'"
                         { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("grouping-separator"))); }
    break;

  case 1001: // FUNCTION_NAME: "'infinity'"
                     { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("infinity"))); }
    break;

  case 1002: // FUNCTION_NAME: "'minus-sign'"
                 { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("minus-sign"))); }
    break;

  case 1003: // FUNCTION_NAME: "'NaN'"
          { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("NaN"))); }
    break;

  case 1004: // FUNCTION_NAME: "'percent'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("percent"))); }
    break;

  case 1005: // FUNCTION_NAME: "'per-mille'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("per-mille"))); }
    break;

  case 1006: // FUNCTION_NAME: "'zero-digit'"
                 { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("zero-digit"))); }
    break;

  case 1007: // FUNCTION_NAME: "'digit'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("digit"))); }
    break;

  case 1008: // FUNCTION_NAME: "'pattern-separator'"
                        { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("pattern-separator"))); }
    break;

  case 1009: // FUNCTION_NAME: "'collection'"
                 { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("collection"))); }
    break;

  case 1010: // FUNCTION_NAME: "'const'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("const"))); }
    break;

  case 1011: // FUNCTION_NAME: "'append-only'"
                  { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("append-only"))); }
    break;

  case 1012: // FUNCTION_NAME: "'queue'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("queue"))); }
    break;

  case 1013: // FUNCTION_NAME: "'mutable'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("mutable"))); }
    break;

  case 1014: // FUNCTION_NAME: "'read-only'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("read-only"))); }
    break;

  case 1015: // FUNCTION_NAME: "'integrity'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("integrity"))); }
    break;

  case 1016: // FUNCTION_NAME: "'constraint'"
                 { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("constraint"))); }
    break;

  case 1017: // FUNCTION_NAME: "'check'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("check"))); }
    break;

  case 1018: // FUNCTION_NAME: "'key'"
          { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("key"))); }
    break;

  case 1019: // FUNCTION_NAME: "'foreach'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("foreach"))); }
    break;

  case 1020: // FUNCTION_NAME: "'foreign'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("foreign"))); }
    break;

  case 1021: // FUNCTION_NAME: "'keys'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("keys"))); }
    break;

  case 1022: // FUNCTION_NAME: "'ancestor'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("ancestor"))); }
    break;

  case 1023: // FUNCTION_NAME: "'child'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("child"))); }
    break;

  case 1024: // FUNCTION_NAME: "'descendant'"
                 { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("descendant"))); }
    break;

  case 1025: // FUNCTION_NAME: "'parent'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("parent"))); }
    break;

  case 1026: // FUNCTION_NAME: "'preceding'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("preceding"))); }
    break;

  case 1027: // FUNCTION_NAME: "'self'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("self"))); }
    break;

  case 1028: // FUNCTION_NAME: "'following'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("following"))); }
    break;

  case 1029: // FUNCTION_NAME: "'ancestor-or-self'"
                       { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("ancestor-or-self"))); }
    break;

  case 1030: // FUNCTION_NAME: "'descendant-or-self'"
                         { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("descendant-or-self"))); }
    break;

  case 1031: // FUNCTION_NAME: "'following-sibling'"
                        { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("following-sibling"))); }
    break;

  case 1032: // FUNCTION_NAME: "'preceding-sibling'"
                        { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("preceding-sibling"))); }
    break;

  case 1033: // FUNCTION_NAME: "'position'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("position"))); }
    break;

  case 1034: // EQNAME: "'EQName'"
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


  const short xquery_parser::yypact_ninf_ = -1469;

  const short xquery_parser::yytable_ninf_ = -875;

  const short
  xquery_parser::yypact_[] =
  {
    5453, -1469, -1469, -1469, -1469,  7641,  7641,  7641,  7641,  7641,
    7641, -1469, -1469,    29,    39, -1469,   658,   359, -1469, -1469,
   -1469,   839, -1469, -1469, -1469,   416,   639,   850,  5171,   665,
     736,   828, -1469,    15, -1469,   791, -1469, -1469, -1469, -1469,
   -1469,   856, -1469,   795,   801, -1469, -1469, -1469, -1469,   599,
   -1469,   858, -1469,   807,   813, -1469,   189, -1469, -1469, -1469,
   -1469, -1469, -1469, -1469, -1469, -1469, -1469, -1469, -1469, -1469,
   -1469,   879,   883, -1469, -1469, -1469, -1469,   757, 13257, -1469,
   -1469, -1469,   891, -1469, -1469, -1469,   861, -1469,   898,   901,
   -1469, -1469, 17121, -1469, -1469, -1469, -1469, -1469, -1469, -1469,
   -1469,   918, -1469, -1469,   922,   929, -1469, -1469, -1469, -1469,
   -1469, -1469, -1469,  6081,  7953,  8265, 17121, -1469, -1469,   900,
   -1469, -1469, -1469, -1469,   902, -1469, -1469,   932, 17121, -1469,
   15641,   934,   937, -1469, -1469, -1469,   938, -1469, -1469, 12633,
   -1469, -1469, -1469, -1469, -1469, -1469,   904, -1469, -1469, -1469,
   -1469, -1469, -1469, -1469, -1469,   100,   846, -1469, -1469, -1469,
   -1469, -1469, -1469, -1469, -1469,   -84,   907,   -72, -1469,   -87,
     741, -1469, -1469, -1469, -1469, -1469, -1469,   949, -1469,   837,
     841,   827, -1469, -1469,   923,   925, -1469, -1469, -1469, -1469,
   -1469, -1469, -1469, -1469, -1469, -1469, -1469, -1469, -1469, -1469,
   -1469, -1469, -1469, -1469, -1469, -1469, -1469, -1469, -1469, -1469,
   -1469, -1469, -1469, -1469, -1469, -1469, -1469, -1469, -1469, -1469,
   -1469, -1469, -1469, -1469, -1469, -1469, -1469, -1469, -1469, -1469,
   -1469, -1469, -1469, -1469, -1469, -1469, -1469, -1469, -1469, -1469,
   -1469, -1469, -1469, -1469, -1469, -1469, -1469, -1469, -1469,  8577,
    8889, -1469,   770, -1469, -1469, -1469, -1469, -1469,  5767,  9201,
    1087, -1469,  9513, -1469, -1469,   161,   109, -1469, -1469, -1469,
   -1469, -1469, -1469, -1469, -1469, -1469, -1469, -1469, -1469, -1469,
   -1469,   111, -1469, -1469, -1469, -1469, -1469, -1469,   738, -1469,
   -1469, -1469, -1469, -1469, -1469, -1469, -1469,  7641, -1469, -1469,
   -1469, -1469,   274, -1469, -1469, -1469, -1469, -1469, -1469, -1469,
   -1469, -1469,   216, -1469,   896, -1469, -1469, -1469,   611, -1469,
     674, -1469, -1469, -1469, -1469, -1469, -1469, -1469,   921,   999,
   -1469,   867,   844,   994,   685,   537,   714,   295, -1469,  1045,
     897,   996,   993, 11073, -1469,   906, -1469, -1469,    35, -1469,
   -1469, 12945, -1469,   835, -1469,   946, 13257, -1469,   946, 13257,
   -1469, -1469, -1469,   763, -1469, -1469, -1469, -1469, -1469, -1469,
   -1469, -1469, -1469, -1469, -1469, -1469, -1469, -1469, -1469, -1469,
   -1469, -1469, -1469, -1469, -1469, -1469, -1469, -1469, -1469, -1469,
   -1469,   945, -1469, -1469, -1469, -1469, -1469, -1469, -1469, -1469,
   -1469, -1469, -1469, -1469, -1469, -1469, -1469, -1469, -1469, -1469,
   -1469, -1469, -1469, -1469, -1469, -1469, -1469, -1469, -1469,   482,
   -1469, -1469, -1469, -1469, -1469,   919, -1469,  7641,   920,   924,
     926,   927,   928,  7641,   603,   603,  1075,   497,   721,   716,
   17711, 17121,    11,  1059, 17121,   954,   995,   703, 17121,   776,
     869, 17121, 17121,   812,   845,   112, -1469, -1469, -1469, 17121,
    7641,   931,  7641,   208, 11385, 15937, 17121,    31,   965, 11385,
    1121,   145,   391, 17121,  1002,   983,  1019, -1469,   941, 11385,
   16233, 17121, 17121, 17121,  7641,   942, 11385, 11385, 17121,  7641,
     977,   978, -1469, -1469, -1469, 11385, 16529,   976, -1469,   979,
   -1469, -1469, -1469, -1469,   980, -1469,   981, -1469, -1469, -1469,
   -1469, -1469,   982, -1469, -1469, -1469, -1469, -1469, -1469, -1469,
   -1469, -1469, -1469, -1469, -1469, -1469, -1469, -1469, -1469, -1469,
   -1469, -1469, -1469, -1469, -1469, -1469, -1469, -1469, -1469, -1469,
   -1469, 17121, -1469, -1469, -1469, -1469, -1469, -1469, -1469, -1469,
   -1469, -1469, -1469, -1469, -1469,   991, 17121, -1469, -1469, -1469,
     951,  6393,  1021,   266,   987,   988,   989, 17121,  7641, -1469,
     997,   822,   220, -1469,   611, -1469,   124,  1113, 11385, 11385,
   -1469,   182, -1469, -1469,  1134, -1469, -1469, -1469, -1469, 11385,
     936, -1469,  1122, 11385, 11385, 17423, 11385, 11385, 11385, 11385,
   17423, 11385,   930, 17121,   964,   966, 11385, 11385,  6081,   915,
   -1469,    77, -1469,    73, 11385,  9201, -1469, -1469, -1469, -1469,
   -1469,   658,   828,   113,   114,  1162,  9825,  3901, 10137,  4224,
     665,   861, -1469, -1469,   251,   861, -1469, 11385,  4848, -1469,
    1047,   544,    15,  1000,  1011,  1012, -1469, 11385, -1469, -1469,
    7641, -1469, -1469, -1469, -1469, -1469, -1469, -1469,   880,   885,
   17121,  1048, 11697, 11697, 11697, -1469, 11697, 11697, -1469, 11697,
   -1469, 11697, -1469, -1469, -1469, -1469, -1469, -1469, -1469, 11697,
   11697,  1112, 11697, 11697, 11697, 11697, 11697, 11697, 11697, 11697,
   11697, 11697, 11697, 11697,   953,  1091,  1092,  1093, -1469, -1469,
   -1469, 12321,  7641, -1469, -1469, 12633, 12633, 11385,   946, -1469,
   -1469,   946, -1469,  6705,   946,   998,  1039,  7017, -1469, -1469,
   -1469, -1469, -1469, -1469,    12, -1469,   293, -1469, -1469, -1469,
   -1469, -1469, -1469,  1082,  1083,   665,  1163, -1469, -1469, 17711,
    1090,    79, 17121,  1031,  1033,  1090,  1075,  1067,  1063, -1469,
   -1469, -1469,   443,   952,  1102,   905, 17121,  1056, 11385,  1081,
   17121, 17121, -1469,  1068,  1018, -1469,  1022,   979,   801, -1469,
    1016,  1024,   199, -1469,   298,   310,  1097, -1469,    52, 17121,
    1114,   335, -1469,  7641, -1469,   128, -1469, 17121,  1111,  1168,
   17121,  1075,  1117,   467, 17121, 11385,    15, -1469,   242,  1030,
   -1469,  1034,  1035,  1036,   353, -1469,    76,  1040, -1469,   131,
     159,  1064, -1469,  1042,  7641,  7641,   270, -1469,   337,   445,
     561, 11385,   426, -1469, -1469, 11385, 11385, -1469, 11385, 11385,
   11385, -1469, 11385, -1469, 11385, -1469, 17121,  1113, -1469,   281,
     348,   355, -1469, -1469, -1469,   366, -1469,   431, -1469, -1469,
    1066,  1073,  1074,  1076,  1077,   763,   482,   294,   328,    10,
     101,  1137,   634,   992,   -81,    40, -1469,  1094, -1469, -1469,
    1043,   162,  6393,   371, 13569,   915, -1469, -1469, -1469,  1001,
   -1469,   161,   753,   857,   775, -1469, -1469,   115, -1469, -1469,
   -1469,   119, -1469,    89, -1469, -1469, -1469, -1469, -1469,   965,
   -1469,  4536, -1469, -1469, -1469, 17121,  1065, 11385, 12009, -1469,
   -1469, -1469, -1469, -1469,  1097, 17121,    65,   999, -1469, -1469,
   -1469, -1469, -1469, 11697, -1469, -1469, -1469,    74, -1469,   537,
     537,   271,   714,   714,   714,   714,   295,   295, -1469, -1469,
   15049, 15049, 17121, 17121, -1469,  1052, -1469, -1469,   253, -1469,
   -1469, -1469, -1469,   481, -1469, -1469, -1469,   493,   603, -1469,
   -1469,   747,   772,   790, -1469,   665, -1469, -1469, -1469, -1469,
   -1469, -1469, -1469, -1469, -1469, -1469,  1090, -1469,  1105, 15345,
    1096, 11385, -1469, -1469, -1469,  1145,  1075,  1075,  1090, -1469,
     899,  1075,   695, 17121,   578,   686,  1211, -1469, -1469,   956,
     697, -1469, -1469, -1469, -1469, -1469, -1469, -1469, -1469, -1469,
   -1469, -1469,   443,    62,  1017,   657, 11385, -1469, 17121,  1148,
     947,  1075, -1469, -1469, -1469, -1469,  1095, 17121, -1469, 17121,
   -1469, 16825,  1120, 15049,  1126, 11385,    75,  1103,    59,  1145,
   15049,  1133,  1158,  1070,  1057,  1123,  1075,  1098,  1124,  1161,
    1075, 11385,     8, -1469, -1469, -1469,  1106, -1469, -1469, -1469,
   -1469,  1142, 11385, 11385,  1115, -1469,  1164,  1165,  7641, -1469,
    1078,  1084,  1116, 17121, -1469, 17121, -1469, 11385,  1129,  1085,
   11385, -1469,  1151,   164,   168,   171,  1232, -1469,   305, -1469,
     315,  1125, -1469, -1469,  1244, -1469,   771, 11385, 11385, 11385,
     788, 11385, 11385, 11385, 11385, 11385, 11385, 11385, 11385, 11385,
   17423,  1157, 11385, 11385, -1469, 10449,   138,  1032, -1469,   522,
   -1469, 11385,   120,   207,    89, 10137,  4224, 10137,  1198, -1469,
   -1469, -1469, 11385,   829,  1172, -1469, 17121,  1176,  1177, -1469,
     319,  1037, -1469, -1469,   690,    65, -1469, 11385,    74,   510,
     626, -1469,  1004,   200,  1026,  1027, -1469, -1469,   869, -1469,
    1025,   181,  1138,  1139, 15345,  1140,  1143,  1146,  1147,  1149,
     838, -1469,   682, -1469, -1469, -1469, -1469, -1469, -1469, -1469,
   -1469, -1469, -1469,  1141, -1469, -1469, -1469, -1469, 10761, -1469,
   -1469, -1469, -1469, -1469, -1469, -1469, -1469, -1469,   797, -1469,
    1264,   849, -1469, -1469, -1469, -1469, -1469, -1469, -1469, -1469,
   -1469,  1023, -1469, -1469,  1272, -1469, -1469, -1469, -1469, -1469,
     717,  1277, -1469,   743, -1469, -1469, -1469,   976,   639,   791,
     980,   795,   981,   982, -1469,  1143,  1146,  1147, -1469,   682,
     682, 12321,  1028,  1006, -1469,  1145,    62,  1086,  1118,  7641,
    1127,  1132,  1178,  1135,  1144, 17121, -1469,    68, -1469, 17121,
   -1469, 11385,  1179, 11385,  1182, 11385,   197,  1181, -1469, -1469,
   -1469, 15049, -1469,  7641,  1075,  1216, -1469, -1469, -1469,  1075,
    1216, -1469, 11385,  1183,  7641, 17121, -1469, -1469, 11385, 11385,
      90, -1469,    25,   589, -1469, 14161,   719, -1469,   774, -1469,
    1150, -1469, -1469,  7641,  1153,  1154, -1469, 11385, -1469, -1469,
   11385,  1155,  1164,  1229, -1469,  1199, -1469,   565, -1469, -1469,
    1331, -1469, -1469,  7641, 17121, -1469,   755, -1469, -1469, -1469,
    7641,  1152,  1104,  1110, -1469, -1469, -1469,  1136,  1156, -1469,
   -1469, -1469,  1250, -1469, -1469, -1469, -1469,  1119,   517, 17121,
    1171, -1469,  1191,  1192,  1201,  1205, -1469,   896,   611, 13569,
    1032, -1469,  7329, 13569, -1469, -1469,   857,   524, -1469, -1469,
   -1469,  1172, -1469,  1075, -1469,   903, -1469,   693,  1259, -1469,
   11385,   661,  1075, -1469, 12009, 11385,  1220, -1469,  1251,  1254,
   11385, 17121,   935,  1296, -1469, -1469, -1469,   373,  1188, -1469,
      74,  1128, -1469, -1469,   552, -1469,   694,   140,  1130,    74,
     694, 11697, -1469,   399, -1469, -1469, -1469, -1469, -1469, -1469,
      74,  1228,  1099,   952,   140, -1469, -1469,  1100,  1303, -1469,
   -1469, -1469, 13865,  1196,  1197,  1200,  1203,  1204,  1206,  1208,
    1138, -1469, -1469, -1469, -1469, -1469, -1469, -1469, -1469, -1469,
   -1469, -1469, -1469, -1469, -1469, -1469,  1355,   411,  1358,   411,
    1160,  1284, -1469, -1469,  1226, 17121,  1159, -1469, -1469, 12321,
   -1469,  1213, -1469, -1469, -1469, -1469, -1469, -1469, 11385,  1252,
   -1469, -1469, 11385, -1469,   369, -1469, 11385,  1256, 11385, -1469,
    1271,  1279,  1302,  1075,  1216, -1469, 11385,  1223, -1469, -1469,
   -1469,  1171, -1469,    25, 11385,  7641,  1171,    25, -1469, 17121,
      -2, -1469, 14457,   554, -1469, 14753,  1171, -1469, -1469,  1224,
   -1469, -1469, -1469, -1469, 11385,   796,  1232, 17121,   768, -1469,
    1227,  1232, 17121, -1469,  1230, -1469, 11385, 11385, 11385, 11385,
    1101, 11385, 11385, -1469, 11385, 11385, 11385, 11385, 10449,   557,
   -1469, -1469, -1469, -1469, -1469,  1259, -1469, -1469, -1469, 11385,
    1267, -1469, -1469, -1469, 11385, 17121, 17121, -1469,   789, -1469,
   11385, -1469, -1469, -1469,  1231, 11697, -1469, -1469, -1469, -1469,
   -1469,   125, 11697, 11697,   654, -1469,  1026, -1469,   461, -1469,
    1027,    74,  1265, -1469, -1469,  1166, -1469, -1469, -1469, -1469,
    1323,  1234, -1469,   521, -1469, -1469, -1469, -1469, -1469, -1469,
   -1469, -1469,   489,   489, -1469,   411, -1469, -1469,   526, -1469,
    1392,   140,  1326,  1237, 12321,    53,  1167,  1255, -1469, -1469,
   11385, -1469, 11385,  1280, -1469, 11385, -1469, 10449, 11385,  1075,
   -1469, -1469, -1469, 11385, 11385, -1469, -1469, -1469, 10449, 10449,
    1334, 15049,  7641, 17121,    -2, 17121, 11385, 17121,    -2, 10449,
   -1469,   559,   300,  1232, 17121, -1469,  1235,  1232, -1469, -1469,
   -1469, -1469, -1469, 11385, -1469, -1469, -1469,   177,   184,   230,
   11385, -1469, -1469,  1176, 11385, -1469, -1469,  1379, -1469, -1469,
   -1469,   714, 11697, 11697,   271,   701, -1469, -1469, -1469, -1469,
   -1469, -1469, 11385, -1469, 15049, -1469, 15049,  1340, -1469, -1469,
   -1469,  1406, -1469, -1469, -1469,  1173,  1329, -1469, -1469,  1332,
   -1469,   848, 17121,  1320,  1225, 17121, 12321, -1469, -1469, 11385,
   -1469, -1469, -1469,  1216, -1469, -1469, 15049, -1469, -1469, -1469,
    1350, 11385,  1171, -1469,  1353, 10449, -1469, 17121,   430,   464,
   -1469,  1258,  1232, -1469,  1261, -1469,  1262,  1164,  1165,   421,
   -1469,  1176,  1333,   271,   271, 11697,   423, -1469, -1469, 15049,
   -1469, -1469,  1326, 12321, -1469,  1259,  1174, 17121,  1337,  1236,
    1332, -1469,    -2, 15049,  7641, 15049,  1171, -1469, -1469,  1359,
     601, -1469, -1469, -1469, -1469,  1276,   681, -1469, -1469, -1469,
    1266, -1469, 10449,   840, -1469, -1469, 17121,   271, -1469, -1469,
   -1469, -1469, -1469, 11385,  1175, 17121,  1343,  7641,    -2, -1469,
   -1469,    -2, 11385, -1469, -1469, -1469, -1469, -1469, -1469, -1469,
    1345, -1469, -1469,  1180,  1184, 17121, -1469, -1469, 11385, 10449,
   17121,  1186, 11385,  1187,  1171, 12321, -1469, 12321, 10449, -1469,
    1268,  1185, 17121,  1240,  1347, 17121,  1190, 12321, -1469
  };

  const short
  xquery_parser::yydefact_[] =
  {
     166,   470,   471,   811,   472,   169,   169,   169,   169,   169,
       0,   834,  1034,   121,   123,   640,   927,   936,   876,   839,
     837,   813,   928,   931,   883,   843,   814,   816,     0,   937,
     818,   934,   905,   885,   859,   815,   880,   881,   932,   929,
     879,   820,   935,   821,   822,   976,   988,   975,   877,   896,
     890,   823,   878,   825,   824,   977,   914,   915,   882,   856,
     998,   999,  1000,  1001,  1002,  1003,  1004,  1005,  1006,  1007,
    1008,  1022,  1029,   904,   900,   891,   870,   812,     0,   899,
     907,   916,  1023,   895,   496,   871,   872,   930,  1024,  1030,
     892,   909,     0,   502,   459,   498,   902,   836,   893,   894,
     923,   897,   913,   922,  1028,  1031,   842,   884,   925,   497,
     912,   918,   817,     0,     0,     0,     0,   406,   910,   921,
     926,   924,   903,   889,   978,   887,   888,  1025,     0,   405,
       0,  1026,  1032,   919,   873,   898,  1027,   213,   437,     0,
     469,   920,   901,   908,   917,   911,   979,   864,   869,   868,
     867,   866,   865,   826,   886,     0,   835,   933,   857,   969,
     968,   970,   841,   840,   860,   967,   819,   964,   972,   966,
     965,   863,   906,   962,   974,   973,   961,   861,   971,   983,
     984,   981,   982,   980,   827,   985,   986,   987,   953,   952,
     939,   855,   848,   946,   942,   858,   854,   954,   875,   844,
     845,   838,   847,   951,   950,   947,   943,   959,   960,   958,
     949,   945,   938,   846,   957,   956,   850,   852,   851,   944,
     948,   940,   853,   941,   849,   955,  1009,  1010,  1011,  1012,
    1013,  1014,   990,   991,   989,   995,   996,   997,   992,   993,
     994,   862,  1015,  1016,  1017,  1018,  1019,  1020,  1021,     0,
       0,   828,   963,  1033,   830,   829,   831,   832,   166,   166,
       0,     2,   166,     9,    11,    23,     0,    28,    31,    36,
      37,    38,    39,    40,    41,    42,    32,    58,    59,    33,
      34,     0,    76,    80,    81,    35,    82,    83,     0,   119,
      84,    85,    86,    87,    18,   163,   164,   165,   172,   175,
     489,   176,     0,   177,   178,   179,   180,   181,   182,   183,
     186,   187,     0,   214,   222,   217,   250,   256,     0,   248,
       0,   249,   224,   218,   185,   219,   184,   220,   223,   357,
     359,   361,   371,   373,   377,   379,   382,   387,   390,   393,
     395,   397,   399,     0,   403,   410,   409,   411,     0,   428,
     412,   433,   436,   438,   441,   443,     0,   448,   445,     0,
     456,   466,   468,   442,   473,   480,   494,   481,   482,   483,
     486,   487,   484,   488,   511,   512,   485,   516,   518,   519,
     520,   517,   565,   566,   567,   568,   569,   570,   571,   465,
     612,     0,   603,   611,   610,   609,   608,   605,   607,   604,
     606,   495,    43,   225,   226,   228,   227,   229,   221,   491,
     492,   493,   490,   231,   234,   230,   232,   233,   467,   810,
     833,   927,   936,   934,   835,     0,   167,   168,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   127,   129,   130,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   279,   280,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   461,   464,   451,     0,     0,   813,   843,   816,
     874,   937,   818,   885,   820,   896,   823,   825,   824,   914,
    1022,  1029,   812,  1023,   872,  1024,  1030,   897,  1028,  1031,
     921,   978,  1025,  1026,  1032,  1027,   979,   967,   964,   966,
     965,   861,   983,   984,   981,   827,   985,   963,   457,   810,
     449,     0,   194,   450,   453,   813,   814,   816,   815,   820,
     821,   822,   823,   812,   819,   499,     0,   455,   454,   189,
       0,     0,   214,     0,   818,   825,   824,     0,   169,   790,
     985,     0,     0,   222,     0,   500,     0,   532,     0,     0,
     460,     0,   431,   432,     0,   463,   462,   452,   435,     0,
       0,   561,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     792,     0,   794,     0,     0,   166,     3,     4,     1,    10,
      12,     0,     0,     0,     0,     6,   166,     0,   166,     0,
       0,     0,   120,   173,     0,     0,   192,     0,     0,   201,
       0,     0,     0,     0,     0,     0,   237,     0,   206,   235,
       0,   255,   251,   257,   252,   254,   253,   260,     0,     0,
       0,     0,     0,     0,     0,   422,     0,     0,   420,     0,
     366,     0,   421,   414,   419,   418,   417,   416,   415,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   408,   407,
     404,     0,   169,   429,   434,     0,     0,     0,   444,   477,
     447,   446,   458,     0,   474,     0,     0,     0,   573,   576,
     574,   577,   581,   583,     0,   125,     0,   809,    47,    44,
      45,    48,    49,     0,     0,     0,     0,    50,    51,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   649,
     650,   651,     0,   104,   143,     0,     0,   113,     0,     0,
       0,     0,   128,     0,     0,   620,     0,     0,     0,   615,
       0,     0,     0,   630,     0,     0,   259,   262,     0,     0,
     135,     0,   139,   169,   514,     0,    60,     0,    69,     0,
       0,     0,    61,     0,     0,     0,     0,   618,     0,     0,
     621,     0,     0,     0,     0,   328,     0,     0,   619,     0,
       0,     0,   638,     0,     0,     0,     0,   624,     0,     0,
     196,     0,     0,   190,   188,     0,     0,   796,     0,     0,
       0,   499,     0,   791,     0,   501,   533,   532,   529,     0,
       0,     0,   563,   562,   430,     0,   560,     0,   662,   663,
       0,     0,     0,     0,     0,   803,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   668,     0,   204,   205,
       0,     0,     0,     0,     0,   209,   210,   793,   795,     0,
       5,    24,     0,    25,     0,     7,    29,     0,    15,     8,
      30,     0,    19,   927,    79,    77,    16,    78,    20,     0,
     195,     0,   193,   215,   216,     0,     0,     0,     0,   297,
     207,   236,   238,   239,   258,     0,     0,   358,   360,   364,
     370,   369,   368,     0,   365,   362,   363,     0,   374,   381,
     380,   378,   384,   385,   386,   383,   388,   389,   392,   391,
       0,     0,     0,     0,   413,     0,   439,   440,     0,   478,
     507,   475,   509,     0,   613,   513,   505,     0,     0,   122,
     124,     0,     0,     0,   103,     0,    93,    95,    96,    97,
      98,   100,   101,   102,    94,    99,    89,    90,     0,     0,
     109,     0,   105,   107,   108,   115,     0,     0,    88,    46,
       0,     0,     0,     0,     0,     0,     0,   740,   745,     0,
       0,   741,   775,   728,   730,   731,   732,   734,   736,   735,
     733,   737,     0,     0,     0,     0,     0,   112,     0,   145,
       0,     0,   580,   572,   616,   617,     0,     0,   634,     0,
     631,     0,     0,     0,     0,     0,     0,     0,     0,   141,
       0,     0,   136,     0,     0,     0,     0,     0,     0,    62,
       0,     0,     0,   278,   284,   281,     0,   623,   622,   629,
     637,     0,     0,     0,     0,   579,     0,     0,     0,   423,
       0,     0,     0,     0,   627,     0,   625,     0,   197,     0,
       0,   797,     0,     0,     0,     0,   532,   530,     0,   521,
       0,     0,   503,   504,     0,    13,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   202,     0,     0,     0,   676,     0,
     211,     0,     0,     0,     0,   166,     0,   166,     0,   515,
     174,   247,     0,   312,   308,   310,     0,   305,   298,   299,
       0,     0,   240,   241,     0,   246,   367,     0,     0,     0,
       0,   706,   375,   679,   683,   685,   687,   689,   692,   699,
     700,   708,   937,   817,     0,   826,   830,   829,   831,   832,
       0,   394,   586,   592,   593,   596,   641,   642,   597,   598,
     601,   396,   398,   401,   602,   400,   427,   479,     0,   476,
     506,   126,    56,    57,    54,    55,   132,   131,     0,    91,
       0,     0,   110,   111,   116,    74,    75,    52,    53,    73,
     746,     0,   749,   776,     0,   739,   738,   743,   742,   774,
       0,     0,   751,     0,   747,   750,   729,     0,     0,     0,
       0,     0,     0,     0,   614,     0,     0,     0,   144,   147,
     149,     0,     0,     0,   114,   117,     0,     0,     0,   169,
       0,     0,   638,     0,     0,     0,   264,     0,   585,     0,
     265,     0,     0,     0,     0,     0,     0,     0,   142,   137,
     140,     0,   191,     0,     0,    71,    65,    68,    67,     0,
      63,   282,     0,     0,   169,     0,   326,   330,     0,     0,
       0,   333,     0,     0,   339,     0,     0,   346,     0,   350,
       0,   425,   424,   169,     0,     0,   198,     0,   200,   327,
       0,     0,     0,     0,   533,     0,   523,     0,   556,   553,
       0,   557,   558,     0,     0,   552,     0,   527,   555,   554,
       0,     0,     0,     0,   655,   656,   652,     0,     0,   660,
     661,   657,   799,   800,   666,   804,   664,     0,     0,     0,
       0,   670,   818,   825,   824,   985,   203,     0,     0,     0,
     671,   672,     0,     0,   212,   802,    26,     0,    17,    21,
      22,   309,   321,     0,   322,     0,   313,   314,   315,   316,
       0,   499,     0,   306,     0,     0,     0,   242,     0,     0,
       0,     0,   291,     0,   288,   293,   245,     0,     0,   690,
     703,     0,   372,   376,     0,   722,     0,     0,     0,     0,
       0,     0,   678,   680,   681,   717,   718,   719,   721,   720,
       0,     0,   694,   693,     0,   697,   701,   715,   713,   712,
     705,   709,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   643,   644,   589,   591,   590,   587,   402,   508,   510,
     134,   133,    92,   106,   763,   744,     0,   768,     0,   768,
     757,   752,   148,   150,     0,     0,     0,   118,   146,     0,
      27,     0,   635,   636,   639,   632,   633,   263,     0,     0,
     277,   269,     0,   273,     0,   267,     0,     0,     0,   138,
       0,     0,    70,     0,    64,   283,     0,     0,   329,   331,
     336,     0,   334,     0,     0,     0,     0,     0,   340,     0,
       0,   354,     0,     0,   347,     0,     0,   351,   426,     0,
     628,   626,   199,   798,     0,     0,   532,     0,     0,   564,
       0,   532,     0,   528,     0,    14,     0,     0,     0,     0,
       0,     0,     0,   669,     0,     0,     0,     0,     0,     0,
     673,   677,   325,   323,   324,   317,   318,   319,   311,     0,
       0,   307,   300,   286,     0,     0,     0,   244,   289,   292,
       0,   707,   698,   704,     0,     0,   777,   778,   788,   787,
     786,     0,     0,     0,     0,   779,   684,   785,     0,   682,
     686,     0,     0,   691,   695,     0,   716,   711,   714,   710,
       0,     0,   599,     0,   594,   648,   588,   808,   807,   806,
     595,   764,     0,     0,   762,   769,   770,   766,     0,   761,
       0,   759,     0,     0,     0,     0,     0,     0,   575,   266,
       0,   275,     0,     0,   271,     0,   274,     0,     0,     0,
      66,   285,   582,     0,     0,   337,   341,   335,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     578,     0,     0,   532,     0,   559,     0,   532,   584,   653,
     654,   658,   659,     0,   665,   805,   667,     0,     0,     0,
       0,   674,   320,   301,     0,   287,   295,   296,   290,   243,
     702,   789,     0,     0,   781,     0,   727,   726,   725,   724,
     723,   688,     0,   780,     0,   645,     0,     0,   773,   772,
     771,     0,   765,   758,   756,     0,   753,   754,   748,   151,
     153,   155,     0,     0,     0,     0,     0,   270,   268,     0,
     276,   208,   356,    72,   332,   338,     0,   355,   352,   348,
       0,     0,     0,   342,     0,     0,   344,     0,   542,   536,
     531,     0,   532,   522,     0,   801,     0,     0,     0,     0,
     304,   302,     0,   783,   782,     0,     0,   646,   600,     0,
     767,   760,     0,     0,   157,   156,     0,     0,     0,     0,
     152,   272,     0,     0,     0,     0,     0,   550,   544,     0,
     543,   545,   551,   548,   538,     0,   537,   539,   549,   524,
       0,   525,     0,     0,   675,   303,     0,   784,   696,   647,
     755,   154,   158,     0,     0,     0,     0,     0,     0,   345,
     343,     0,     0,   535,   546,   547,   534,   540,   541,   526,
       0,   294,   159,     0,     0,     0,   353,   349,     0,     0,
       0,     0,     0,     0,     0,     0,   161,     0,     0,   160,
       0,     0,     0,     0,     0,     0,     0,     0,   162
  };

  const short
  xquery_parser::yypgoto_[] =
  {
   -1469, -1469,  -219,  -173, -1469,  1209,  1210, -1469,  1214,  -538,
   -1469, -1469, -1469, -1469, -1469, -1469, -1469, -1469,  -565, -1469,
   -1015, -1469, -1469, -1469, -1469,  -234,  -546, -1469,   725,   -27,
   -1469, -1469, -1469, -1469, -1469,   272,   500, -1469, -1469,     0,
    -285,  1049, -1469,  1041, -1469, -1469,  -576, -1469,   442, -1469,
     239, -1469,  -240,  -286, -1469,  -527, -1469,    64,    51,    43,
    -296,  -183, -1469,  -819, -1469, -1469,   362, -1469, -1469, -1469,
   -1469, -1469, -1469, -1469, -1469, -1469, -1469,   614, -1469,   -69,
    2078,    17, -1469, -1469,  -289, -1469, -1469,   345, -1469, -1469,
    -314, -1469,    34, -1469, -1469,   832,  -936,  -740,  -697, -1469,
   -1469,   704, -1469, -1469,   -67,   107, -1469, -1469, -1469,   118,
   -1224, -1469,   375,   123, -1469, -1469,   132, -1285, -1469,   955,
     223, -1469, -1469,   224, -1053, -1469, -1469,   217, -1469, -1469,
   -1280, -1259, -1469,   215, -1408, -1469, -1469,   854,   855, -1469,
    -472,   842, -1469, -1469,  -673,   354,  -668,   352,   357, -1469,
   -1469, -1469,   574, -1469, -1469,  1193, -1469, -1469, -1469, -1469,
   -1469,  -816,  -320,  -690, -1469,  -109, -1469, -1469, -1469, -1469,
   -1469, -1469, -1469,     9,  -838, -1469,  -516,   282,   273, -1469,
    -369, -1469, -1469, -1469, -1469, -1469, -1469, -1469,   803, -1469,
   -1469, -1469, -1469, -1035, -1469,   206, -1469,   688,  -823, -1469,
   -1469, -1469, -1469, -1469,  -270,  -263, -1228,  -921, -1469, -1469,
   -1469, -1469, -1469, -1469, -1469, -1469, -1469, -1469, -1469,  -729,
    -865,  -194,  -795, -1469, -1469, -1469,  -718, -1469, -1469, -1469,
   -1469, -1469, -1469, -1469, -1469, -1469,  1071,  1072,  -237,   499,
    -912, -1469,   361,   363, -1469, -1469, -1469, -1469, -1469, -1469,
   -1469, -1469,   188, -1469, -1469,   178, -1469,   183, -1054, -1469,
   -1469, -1469,   135,   121,   -51,   396, -1469, -1469, -1469, -1469,
   -1469, -1469, -1469, -1469, -1469, -1469, -1469, -1469, -1469,   133,
   -1469, -1469, -1469,   -41,   392,   539, -1469, -1469, -1469, -1469,
   -1469,   326, -1469, -1469, -1468, -1469, -1469, -1469,  -560, -1469,
      94, -1469,   -61, -1469, -1469, -1469, -1469, -1319, -1469,   146,
   -1469, -1469, -1469, -1469, -1469,   959, -1469, -1469, -1469, -1469,
   -1469,  -917,  -404,  1569,   963, -1469
  };

  const short
  xquery_parser::yydefgoto_[] =
  {
       0,   260,   638,   261,   262,   263,   264,   265,   266,   267,
     268,   269,   270,   271,   272,   273,   274,   275,   276,   277,
    1275,   791,   278,   279,   280,   281,   282,   283,   976,   977,
     978,   284,   285,   286,   982,   983,   984,   287,   454,   571,
     289,   724,   290,   456,   457,   458,   468,   781,   782,   291,
    1238,   292,  1719,  1720,   293,   294,   295,   560,   296,   297,
     298,   299,   300,   784,   301,   302,   542,   303,   304,   305,
     306,   307,   308,   648,   309,   310,   875,   876,   311,   312,
     313,   573,   315,   649,  1656,   661,  1144,  1145,   316,   651,
     317,   653,   574,   319,   320,   776,   777,  1392,   475,   321,
     476,   477,   916,  1393,  1394,  1395,   654,   655,  1138,  1139,
    1383,   656,  1134,  1135,  1376,  1377,  1378,  1379,   322,   804,
     805,   323,  1290,  1291,  1503,   324,  1293,  1294,   325,   326,
    1296,  1297,  1298,  1299,  1510,   327,   328,   329,   330,   923,
     331,   332,  1402,   333,   334,   335,   336,   337,   338,   339,
     340,   341,  1182,   342,   343,   344,   345,   679,   680,   346,
     347,   348,   349,   350,   351,   352,   353,   354,   355,   356,
     357,   358,   359,   360,   361,   362,   363,   714,   709,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   953,   373,
     374,   375,   376,   377,   378,  1326,   837,   838,   839,  1750,
    1795,  1796,  1789,  1790,  1797,  1791,  1327,  1328,   379,   380,
    1329,   381,   382,   383,   384,   385,   386,   387,   388,  1038,
    1511,  1446,  1172,  1603,  1173,  1183,   389,   390,   391,   392,
     393,   394,   395,   396,   397,   398,   399,   400,   811,  1251,
     401,  1175,  1176,  1177,  1178,   402,   403,   404,   405,   406,
     407,   865,   866,   408,  1360,  1361,  1681,  1119,  1152,  1412,
    1413,  1153,  1154,  1155,  1156,  1157,  1422,  1593,  1594,  1158,
    1425,  1159,  1574,  1160,  1161,  1430,  1431,  1599,  1597,  1414,
    1415,  1416,  1417,  1699,   753,  1003,  1004,  1005,  1006,  1007,
    1008,  1224,  1623,  1716,  1225,  1621,  1714,  1009,  1459,  1618,
    1614,  1615,  1616,  1010,  1011,  1418,  1426,  1584,  1419,  1580,
    1403,   409,   410,   411,   412,   563,   413,   414,   415,   416,
     417,  1179,  1276,   418,   419,   420
  };

  const short
  xquery_parser::yytable_[] =
  {
     288,   633,   609,   632,   652,   288,   288,   288,   288,   288,
     288,   944,   931,  1292,  1088,  1151,   455,   314,   932,   933,
     934,   935,   314,   314,   314,   314,   314,   314,   703,   650,
     588,   624,   728,  1525,   318,  1280,  1118,  1514,  1036,   318,
     318,   318,   318,   318,   318,   572,   576,   627,   427,   427,
     427,   427,   427,   433,   899,  1325,   426,   426,   426,   426,
     426,   432,   629,   894,  1052,   725,   725,   788,   792,   425,
     428,   429,   430,   431,   625,  1171,  1181,  1064,   625,   855,
    1129,  1037,   895,   897,   862,   616,   617,   538,   886,   890,
    1032,  1078,  1556,  1557,  1398,  1256,  1240,  1264,  1533,   888,
     892,   896,   898,  1227,  1654,  1595,    15,  1658,  1228,   767,
     625,  1150,   625,   288,   625,   625,   625,  1229,    13,    14,
     625,   625,   958,  1141,  1230,   625,  1231,   768,  1504,   625,
     314,   633,   625,   590,   825,  1232,   473,   633,   440,   625,
     599,    13,    14,   593,  1197,   594,  1109,   318,   441,  1033,
    1111,  1034,   779,  1282,  1717,   596,   561,   597,  1034,   963,
     625,   440,  1233,   625,   426,   625,   443,  1034,  1258,   625,
     632,   444,   625,  1033,  1722,  1269,   979,   727,   625,  1692,
     611,   613,   737,   637,  1201,   625,   646,   637,   434,   443,
     842,   702,   919,  1289,   920,   921,   621,   922,   435,   924,
     625,   959,  1035,   445,   446,  1128,  1501,   925,   926,  1265,
    1651,   826,   622,   646,   130,   738,  -170,   625,  1478,   980,
     780,   625,  1174,  1174,   981,  1261,  1063,   759,   634,   600,
    1147,   625,   595,  1148,   637,  1110,  1151,  1151,   637,  1581,
    1507,   637,   704,   625,   598,  1104,   436,   437,   637,   438,
     439,  -171,   625,   130,   625,   767,  1204,  1693,   288,   288,
     652,  1174,   288,  1315,  1494,   633,  1514,   442,   488,   637,
    1682,   625,   637,   768,   637,   314,   314,  1112,   637,   314,
    1723,   637,  1325,   474,  1142,   834,   632,   637,  1452,  1234,
     474,  1325,   318,   318,   637,  1239,   318,   288,  1266,  1404,
    1533,   626,  1715,   628,  1810,   881,   883,  1125,   787,   637,
    1268,  1127,  1366,   835,   314,  1174,   591,  1044,   894,  1477,
    1066,  1143,  1174,  1367,  1318,  1319,   637,   474,  1782,  1240,
     637,   318,  1150,  1150,   474,  1149,  1105,   895,   897,  1262,
     637,  1267,   989,   886,   890,   489,  1573,  1486,  1067,   625,
     474,  1115,   637,  1311,   910,  1283,   625,  1312,   634,   843,
    1313,   637,  1724,   637,   634,   710,  1756,   625,   712,  1434,
     448,  1405,   625,  1757,   625,  1818,   826,  1821,   451,  1451,
     637,  1235,  1236,  1237,   635,  1026,   452,  1049,   878,  1198,
     877,  1096,   887,   891,   453,   772,  1748,   769,  1582,  1583,
     785,   760,  -170,   958,   882,   884,  1489,   833,  1027,   761,
     798,  1386,   490,   491,  1089,   901,  1033,   809,   810,  1758,
    1029,  1320,   625,   727,   625,  1100,   816,   288,  1056,   449,
    1427,  1428,   683,   288,   692,  1787,  1774,  -171,  1316,  1318,
    1187,  1321,   880,  1322,   314,  1041,  1174,  1073,   693,   684,
     314,  1146,   827,  1429,  1406,  1407,  1072,  1408,   637,  1760,
     288,   318,   288,  1061,  1409,   637,   636,   318,  1034,  1385,
     450,  1323,   474,  1318,  1410,   116,   637,   314,  1803,   314,
    1090,   637,   960,   637,   288,  1749,  1411,  1028,  1151,   288,
    1812,   128,   634,  1325,   318,  1793,   318,  1151,  1404,  1030,
     992,   314,   993,   433,  1317,   433,   314,   789,  1151,   840,
     841,   764,  1324,   766,  1097,  1098,  1467,  1479,   318,  1632,
     845,  1118,   459,   318,  1042,  1551,  1074,   433,  1239,  1099,
     155,   637,   433,   637,  1091,   807,  1061,  1805,   871,   873,
     813,  1092,    15,  1062,  1514,   466,   994,  -261,  1101,  1102,
    1612,  1464,  1093,  1174,   790,  1075,  1321,  1117,  1322,  1571,
     894,   288,   894,  1103,  1033,  1094,  1788,  1602,   288,  1487,
    1405,   743,   462,   744,  1318,  1319,   633,  1174,   314,   895,
     897,   895,  1205,  1206,  1150,   314,  1323,  1209,  1456,  1191,
    1321,  1188,  1322,  1150,  1348,   318,   946,   947,  1368,  1369,
    1370,  1794,   318,  1188,  1150,  1130,  1787,  1804,   288,  1808,
    1318,   427,  1051,  -261,  1613,   288,  1080,  1248,  -261,   426,
    1323,   455,   683,  1095,  1733,   314,   288,   288,   288,   288,
     716,  1706,   314,   746,  1076,    15,  1711,   708,   948,   684,
     711,   717,   318,   314,   314,   314,   314,  1122,  1458,   318,
     288,   872,  1560,  1406,  1407,   640,  1408,   686,  1033,   641,
     318,   318,   318,   318,   642,  -261,  1147,   314,  1126,  1148,
    1189,  1320,   884,  1410,   707,  1655,   713,   657,  1362,  1151,
    1747,   729,  1190,   643,   318,  1411,   687,    13,    14,   130,
    1318,  1321,  1289,  1322,   747,   730,   995,   436,   437,   688,
     438,   439,   288,  1662,  1507,  1506,  1077,   440,  1666,   996,
    1707,   997,  1793,  1680,  1174,  1712,    84,   441,   442,   314,
     483,  1323,   998,   999,  1000,   116,  1001,  1321,  1002,  1322,
      95,  1107,  1696,   689,  1363,   443,   318,  1824,  1588,  1697,
     444,   128,   646,   658,  1633,   427,  1388,   646,   659,  1698,
     743,  1389,   744,   426,   889,   109,  1542,  1323,  1033,  1083,
    1084,  1085,  1527,   873,  1318,  1319,   945,    13,    14,  1363,
    1390,  1576,   445,   446,   447,  1150,   889,  1318,  1319,  1627,
     155,   644,  1400,   288,   645,   745,  1737,   630,  1123,  1034,
    1250,   707,  1253,   713,  1174,   660,  1221,  1174,   463,   646,
     314,   894,  1373,  1116,   622,   130,  1559,  1321,  1577,  1322,
    1124,  1391,   746,  1222,   288,   288,  1456,   318,  1827,  1356,
     895,   448,  1512,  1375,   467,   647,   427,  1792,  1798,  1443,
     703,   314,   314,  1457,   426,  1513,  1304,  1323,  1305,  1767,
    1751,  1768,  1221,  1215,  1754,  1388,   683,  1043,   318,   318,
    1389,    13,    14,   631,  1216,   470,  1223,   433,   433,  1222,
    1444,  1320,   683,   684,   471,  1070,  1071,    13,    14,  1792,
    1492,   630,   288,   747,  1320,  1798,  1458,  1515,  1445,   684,
     449,  1321,   494,  1322,   733,   632,   685,  1440,  1034,   314,
    1516,   472,  1124,   455,  1321,   469,  1322,   901,  1192,  1512,
     734,   288,  1765,   884,  1809,   731,   318,  1691,   622,  1694,
    1695,  1323,  1661,   495,  1193,   116,   496,   690,   314,   732,
     707,   450,   713,  1194,  1323,  1696,   691,  1372,   116,  1800,
    1196,   128,  1697,  1174,  1721,   318,  1364,  1450,  1373,  1195,
    1170,  1170,  1698,  1512,   128,  1033,   783,   451,  1374,  1199,
     478,  1217,  1532,   783,   481,   452,  1830,  1373,  1242,  1375,
     482,  1199,  1218,   453,  1578,  1664,   486,  1243,   601,  1552,
     155,  1579,   487,   602,  1210,  1211,  1212,  1490,  1561,  1170,
    1213,   949,   541,   155,   949,   757,  1174,   949,  1174,   980,
     758,  1388,  1775,   900,   981,   460,  1389,   902,   461,   749,
     750,   751,   664,  1505,   492,  1332,   464,  1333,   493,   465,
     665,   666,   479,   667,   484,   480,   540,   485,  1174,  1763,
    1764,   668,  1337,   543,  1338,   669,   544,   670,   705,   706,
    1207,  1208,   671,  1170,  1553,  1554,  1721,   929,   930,   556,
    1170,   539,   936,   937,   652,  1462,  1463,   557,   672,   938,
     939,  1174,  1708,  1709,   558,   539,   578,   580,   579,   585,
     589,  1544,   586,   587,   592,  1174,  -614,  1174,   288,  1548,
     603,   606,   673,   674,   675,   676,   677,   678,  1397,   539,
     604,   608,   607,  1721,   605,   314,   614,   618,   639,  1640,
     662,   539,  1807,   539,   663,   681,   682,   694,   697,   695,
     696,   701,   318,   707,   715,   718,   719,   727,   739,   741,
     720,   433,   721,   722,   723,   288,   742,   752,   756,  1300,
     765,   783,   786,   794,   455,   288,   288,   288,   795,   796,
     797,   808,  1357,   814,   815,   461,   821,   823,   465,   480,
     485,   496,   314,   314,   314,   825,   828,   829,   830,  1358,
     836,   844,   846,   832,   847,  1849,   868,  1850,   869,   318,
     318,   318,   874,   864,  1170,   885,   912,  1858,   905,   915,
     906,   913,   609,   966,   967,   968,   969,   970,   971,   972,
     973,   974,   975,   907,   908,   927,   940,   954,   941,   942,
     943,   955,   961,   962,   986,   964,   987,   990,   991,  1013,
    1012,  1016,  1018,  1021,  1022,  1024,  1014,  1031,  1023,  1646,
    1046,  1040,  1643,  1025,  1644,  1047,  1050,  1648,  1649,  1057,
    1068,  1652,   460,  1058,  1059,  1060,  1065,  1659,  1069,   464,
     479,  1108,   484,   495,  1106,  1114,  1121,  1132,  1186,  1113,
    1200,   981,  1033,  1219,  1220,  1246,  1241,  1259,  1247,   288,
    1257,  1249,   910,  1263,   779,  1271,  1272,  1273,  1274,  1278,
    1279,  1277,  1284,  1285,  1301,  1288,   314,  1289,  1295,  1314,
    1302,  1170,  1303,   288,  1307,  1310,  1331,  1308,  1349,  1359,
     745,  1330,  1380,   318,   288,  1382,  1420,  1384,  1447,  1421,
     314,  1401,   427,  1387,  1424,  1170,    15,  1432,  1433,  1435,
     426,   314,  1436,   288,  1455,  1437,  1438,   318,  1439,  1460,
    1470,  1454,  1484,  1471,  1466,  1469,  1472,  1465,   318,   539,
     314,  1473,   539,   288,  1475,  1474,  1493,   427,  1496,  1482,
     288,  1488,  1512,  1476,  1526,   426,  1518,   318,  1529,  1536,
     314,  1731,  1520,  1521,  1535,  1537,   427,   314,  1497,  1540,
    1545,  1546,  1735,  1646,   426,  1524,  1738,   318,  1541,   646,
    1547,  1362,   288,  1746,   318,  1741,   433,  1519,  1373,  1745,
    1564,  1538,  1565,   433,  1530,  1566,  1570,  1572,  1591,  1357,
    1575,  1534,  1585,  1592,  1598,  1604,  1605,  1611,  1596,  1606,
    1617,  1539,  1607,  1608,  1622,  1609,  1358,  1610,  1624,  1628,
    1637,  1639,  1630,   735,   539,   872,  1635,   539,  1638,  1642,
    1660,   539,  1684,  1665,   539,   539,  1668,  1690,  1626,  1673,
    1704,  1702,   539,  1705,  1713,  1221,  1718,  1726,   539,   539,
    1729,  1736,  1170,  1620,  1753,  1762,   539,  1769,  1770,  1772,
    1771,  1777,  1773,   539,   539,   539,   539,  1783,  1703,  1738,
    1785,   539,  1778,  1784,  1806,  1823,  1725,  1799,  1815,   539,
    1801,  1826,  1802,  1816,  1835,  1829,  1840,  1854,  1855,  1851,
     988,   619,   620,  1453,  1852,  1841,  1677,  1678,  1679,   623,
    1202,  1813,  1833,  1270,   726,  1468,  1780,  1811,  1819,  1120,
    1396,  1842,   914,  1817,  1845,  1847,   762,  1822,  1857,  1569,
    1055,  1688,  1562,  1558,   539,   288,  1490,  1371,  1498,  1555,
    1508,   822,  1170,  1517,  1502,  1170,   917,  1185,   918,   539,
     957,  1836,   314,  1528,   928,  1087,  1828,  1825,  1254,  1838,
     539,  1441,  1839,  1442,   770,   771,   700,  1543,  1550,   318,
    1701,  1590,  1549,  1836,  1586,  1399,  1589,  1700,   288,  1461,
    1423,  1226,  1819,  1619,  1710,  1848,  1587,   860,   856,     0,
       0,     0,     0,   856,     0,  1357,   539,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1358,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1759,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   539,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1766,     0,     0,     0,   288,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   288,   288,
       0,  1170,   288,     0,  1357,     0,     0,     0,     0,   288,
       0,   555,     0,     0,     0,  1357,  1357,     0,     0,   314,
       0,  1358,     0,     0,     0,     0,  1357,     0,     0,     0,
       0,     0,  1358,  1358,     0,   577,   318,     0,     0,     0,
       0,     0,     0,  1358,     0,     0,     0,   581,     0,   584,
       0,     0,   965,     0,  1170,   539,  1170,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   539,
       0,     0,     0,   539,   539,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1170,     0,     0,     0,
       0,     0,   539,     0,     0,   288,     0,     0,     0,     0,
     539,     0,     0,   539,     0,     0,     0,   539,     0,     0,
       0,     0,  1357,     0,     0,     0,     0,     0,     0,  1170,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1358,
       0,     0,     0,  1170,   288,  1170,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   539,
       0,   314,   288,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   288,   318,  1357,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   314,     0,  1358,   539,     0,   288,
       0,     0,     0,     0,     0,     0,     0,     0,   288,     0,
       0,   318,     0,     0,     0,     0,  1357,     0,     0,     0,
       0,     0,     0,     0,     0,  1357,     0,     0,   539,     0,
       0,     0,     0,  1358,     0,     0,     0,     0,   539,     0,
       0,     0,  1358,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   539,   539,   539,   539,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   539,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   539,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   539,     0,     0,     0,     0,     0,     0,     0,     0,
     539,     0,   539,     0,   539,     0,   539,     0,     0,     0,
       0,     0,     0,   539,     0,     0,     0,     0,     0,     0,
     736,     0,     0,   740,     0,     0,     0,   748,     0,     0,
     754,   755,     0,     0,     0,     0,     0,     0,   763,     0,
       0,     0,     0,     0,   775,   778,   539,     0,   539,     0,
       0,     0,   793,     0,     0,     0,     0,     0,     0,   801,
     802,   803,   806,     0,     0,     0,     0,   812,     0,     0,
       0,     0,     0,     0,     0,   819,     0,     0,     0,     0,
       0,     0,     0,   856,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   539,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     820,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   806,     0,   539,     0,     0,
       0,     0,     0,     0,     0,     0,   831,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   867,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   562,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   539,     0,
       0,     0,   539,     0,     0,     0,     0,     0,     0,   778,
       0,     0,     0,     0,   539,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   539,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   539,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   539,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   985,   539,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   539,     0,     0,  1015,   539,     0,     0,  1019,
    1020,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1039,     0,
       0,     0,     0,     0,   539,     0,  1045,     0,     0,  1048,
       0,     0,     0,  1053,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   539,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1086,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   539,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   539,     0,  1131,   539,     0,     0,   539,     0,
       0,     0,     0,     0,  1140,     0,     0,     0,     0,     0,
     539,     0,     0,     0,     0,   539,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1180,
    1180,  1184,  1184,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   539,   539,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1180,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1214,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1245,     0,     0,
       0,     0,     0,     0,     0,     0,  1252,     0,  1252,     0,
     778,     0,  1180,     0,     0,     0,     0,     0,     0,  1180,
       0,     0,     0,     0,   539,     0,   539,     0,   539,     0,
     539,     0,     0,     0,     0,     0,     0,   539,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   812,     0,   812,     0,   562,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   539,     0,   539,
       0,   848,   849,     0,   857,   858,   859,   861,     0,   863,
       0,     0,     0,     0,   870,   539,     0,     0,   539,     0,
       0,     0,   879,     0,     0,     0,     0,     0,     0,   539,
       0,     0,     0,     0,     0,  1381,     0,     0,     0,     0,
     539,     0,     0,     0,     0,   903,   904,     0,     0,     0,
       0,     0,     0,     0,     0,   909,     0,     0,   911,     0,
       0,     0,   539,  1180,     0,     0,     0,     0,     0,     0,
     539,     0,     0,     0,     0,     0,   539,     0,   539,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   539,
       0,     0,     0,     0,     0,     0,     0,     0,   539,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   952,     0,     0,     0,   952,     0,     0,   539,     0,
       0,     0,     0,   539,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   539,     0,     0,   539,     0,
       0,     0,     0,     0,   778,     0,     0,     0,  1480,     0,
       0,     0,     0,     0,     0,     0,  1017,     0,     0,     0,
    1180,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   806,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1180,     0,     0,     0,     0,     0,
       0,     0,     0,  1054,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1531,     0,     0,     0,     0,     0,  1079,
       0,     0,     0,  1081,  1082,     0,     0,     0,     0,     0,
       0,     0,   911,     0,     0,     0,     0,     0,   867,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1568,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   904,
       0,     0,     0,     0,     0,  1133,  1137,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1180,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1625,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1203,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1650,     0,
       0,  1180,     0,     0,  1180,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1244,     0,  1663,     0,     0,     0,
       0,  1667,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1260,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1281,
       0,     0,     0,     0,  1686,  1687,     0,     0,     0,     0,
    1286,  1287,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1306,     0,     0,  1309,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1334,  1335,  1336,     0,  1339,
    1340,  1341,  1342,  1343,  1344,  1345,  1346,  1347,     0,     0,
    1350,  1351,     0,     0,     0,     0,     0,     0,     0,  1365,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1133,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1180,     0,  1740,     0,  1742,     0,  1744,     0,     0,     0,
       0,     0,     0,  1752,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1449,     0,     0,     0,
       0,     0,     0,  1180,     0,  1180,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1776,     0,     0,  1779,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1180,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1786,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1180,  1481,
       0,  1483,     0,  1485,     0,     0,  1814,     0,     0,     0,
       0,  1491,  1180,     0,  1180,     0,     0,     0,     0,     0,
    1495,     0,     0,     0,     0,     0,  1499,  1500,     0,     0,
       0,     0,     0,     0,     0,  1831,     0,     0,     0,     0,
       0,     0,     0,     0,  1834,  1522,     0,     0,  1523,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1843,     0,     0,     0,     0,  1844,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1853,     0,     0,  1856,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1133,     0,
       0,     0,  1137,  1563,     0,     0,     0,     0,  1567,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1629,     0,     0,     0,
    1631,     0,     0,     0,  1634,     0,  1636,     0,     0,     0,
       0,     0,     0,     0,  1641,     0,     0,     0,     0,     0,
       0,     0,  1645,  1647,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1491,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1669,  1670,  1671,  1672,     0,  1674,
    1675,     0,  1676,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1683,     0,     0,
       0,     0,  1685,     0,     0,     0,     0,     0,  1689,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1727,     0,
    1728,     0,     0,  1730,     0,     0,  1732,     0,     0,     0,
       0,  1734,  1647,     0,     0,     0,     0,     0,     0,     0,
    1739,     0,     0,     0,  1743,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1755,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1761,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1781,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1739,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1820,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1832,     0,     0,     0,  1837,     0,     0,     0,     0,
    1820,  -166,   889,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     1,     2,     0,     0,  1837,     3,     0,     0,
    1846,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,     0,    15,     0,     0,    16,   422,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,     0,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,     0,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,     0,    74,    75,
      76,    77,    78,     0,    79,    80,    81,     0,     0,    82,
      83,     0,     0,     0,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,     0,     0,     0,    95,     0,
      96,    97,    98,    99,     0,   100,     0,     0,     0,   101,
     102,   103,   104,   105,     0,     0,     0,     0,     0,     0,
     106,   107,   108,   109,   110,   111,   112,   113,   114,     0,
     115,   116,   117,   118,   119,     0,     0,     0,   120,   121,
     122,   123,   124,   125,   126,   127,     0,   128,     0,   129,
     130,     0,     0,   131,   132,   133,     0,     0,     0,   134,
       0,   135,   136,   137,   138,   139,     0,   140,     0,   141,
       0,   142,   143,   144,   145,   146,   147,   148,   149,   150,
     151,   152,     0,     0,   153,   154,   155,     0,   424,   157,
     158,   159,   160,   161,   162,   163,   164,   165,   166,   167,
     168,   169,   170,   171,   172,   173,   174,   175,   176,   177,
     178,   179,   180,   181,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   197,
     198,     0,   199,   200,   201,     0,   202,   203,   204,   205,
     206,   207,   208,   209,   210,   211,   212,   213,   214,   215,
     216,   217,   218,   219,   220,   221,   222,   223,   224,   225,
     226,   227,   228,   229,   230,   231,   232,   233,   234,   235,
     236,   237,   238,   239,   240,   241,   242,   243,   244,   245,
     246,   247,   248,   249,     0,   250,     0,   251,   252,   253,
     254,   255,   256,   257,  -166,   889,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     1,     2,     0,     0,     0,
       3,     0,     0,     0,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,     0,    15,     0,     0,   893,
     422,     0,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,   423,    32,    33,    34,    35,
      36,     0,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,     0,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
       0,    74,    75,    76,    77,    78,     0,    79,    80,    81,
       0,     0,    82,    83,     0,     0,     0,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,     0,     0,
       0,    95,     0,    96,    97,    98,    99,     0,   100,     0,
       0,     0,   101,   102,   103,   104,   105,     0,     0,     0,
       0,     0,     0,   106,   107,   108,   109,   110,   111,   112,
     113,   114,     0,   115,   116,   117,   118,   119,     0,     0,
       0,   120,   121,   122,   123,   124,   125,   126,   127,     0,
     128,     0,   129,   130,     0,     0,   131,   132,   133,     0,
       0,     0,   134,     0,   135,   136,   137,   138,   139,     0,
     140,     0,   141,     0,   142,   143,   144,   145,   146,   147,
     148,   149,   150,   151,   152,     0,     0,   153,   154,   155,
       0,   424,   157,   158,   159,   160,   161,   162,   163,   164,
     165,   166,   167,   168,   169,   170,   171,   172,   173,   174,
     175,   176,   177,   178,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,   197,   198,     0,   199,   200,   201,     0,   202,
     203,   204,   205,   206,   207,   208,   209,   210,   211,   212,
     213,   214,   215,   216,   217,   218,   219,   220,   221,   222,
     223,   224,   225,   226,   227,   228,   229,   230,   231,   232,
     233,   234,   235,   236,   237,   238,   239,   240,   241,   242,
     243,   244,   245,   246,   247,   248,   249,   889,   250,     0,
     251,   252,   253,   254,   255,   256,   257,     1,     2,     0,
       0,     0,     3,     0,     0,     0,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,     0,    15,     0,
       0,   421,   422,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,   423,    32,    33,
      34,    35,    36,     0,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,     0,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,     0,    74,    75,    76,    77,    78,     0,    79,
      80,    81,     0,     0,    82,    83,     0,     0,     0,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
       0,     0,     0,    95,     0,    96,    97,    98,    99,     0,
     100,     0,     0,     0,   101,   102,   103,   104,   105,     0,
       0,     0,     0,     0,     0,   106,   107,   108,   109,   110,
     111,   112,   113,   114,     0,   115,   116,   117,   118,   119,
       0,     0,     0,   120,   121,   122,   123,   124,   125,   126,
     127,     0,   128,     0,   129,   130,     0,     0,   131,   132,
     133,     0,     0,     0,   134,     0,   135,   136,   137,   138,
     139,     0,   140,     0,   141,     0,   142,   143,   144,   145,
     146,   147,   148,   149,   150,   151,   152,     0,     0,   153,
     154,   155,     0,   424,   157,   158,   159,   160,   161,   162,
     163,   164,   165,   166,   167,   168,   169,   170,   171,   172,
     173,   174,   175,   176,   177,   178,   179,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,   197,   198,     0,   199,   200,   201,
       0,   202,   203,   204,   205,   206,   207,   208,   209,   210,
     211,   212,   213,   214,   215,   216,   217,   218,   219,   220,
     221,   222,   223,   224,   225,   226,   227,   228,   229,   230,
     231,   232,   233,   234,   235,   236,   237,   238,   239,   240,
     241,   242,   243,   244,   245,   246,   247,   248,   249,   889,
     250,     0,   251,   252,   253,   254,   255,   256,   257,     1,
       2,     0,     0,     0,     3,     0,     0,     0,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,     0,
      15,     0,     0,   421,   422,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,   564,   423,
      32,    33,    34,    35,    36,     0,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,     0,
      50,    51,    52,   565,   566,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,     0,    74,    75,    76,    77,    78,
       0,    79,    80,    81,     0,     0,    82,    83,     0,     0,
       0,    84,    85,   514,    87,    88,    89,    90,    91,   567,
      93,    94,     0,     0,     0,    95,     0,    96,    97,    98,
      99,     0,   100,     0,     0,     0,   101,   102,   103,   104,
     105,     0,     0,     0,     0,     0,     0,   106,   107,   108,
     109,   110,   111,   112,   568,   114,     0,   115,   116,   117,
     118,   119,     0,     0,     0,   120,   121,   122,   123,   124,
     125,   126,   127,     0,   128,     0,   129,   130,     0,     0,
     131,   132,   133,     0,     0,     0,   134,     0,   135,   136,
       0,   138,   139,     0,   140,     0,   141,     0,   142,   143,
     144,   145,   146,   147,   148,   149,   150,   151,   152,     0,
       0,   153,   154,   155,     0,   424,   157,   158,   159,   160,
     161,   162,   163,   164,   165,   166,   167,   168,   169,   170,
     171,   172,   173,   174,   175,   176,   177,   178,   532,   533,
     534,   182,   183,   535,   570,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,   197,   198,     0,   199,
     200,   201,     0,   202,   203,   204,   205,   206,   207,   208,
     209,   210,   211,   212,   213,   214,   215,   216,   217,   218,
     219,   220,   221,   222,   223,   224,   225,   226,   227,   228,
     229,   230,   231,   232,   233,   234,   235,   236,   237,   238,
     239,   240,   241,   242,   243,   244,   245,   246,   247,   248,
     249,     0,   250,     0,   251,   252,   253,   254,   255,   256,
     257,  -874,   466,     0,  -261,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  -874,     0,     0,     0,  -874,
       0,     0,     0,  -874,  -874,     0,     0,     0,     0,  -874,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    -261,     0,     0,  -874,     0,  -261,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  -874,     0,  -874,  -874,
    -874,     0,     0,  -874,  -874,  -874,  -874,     0,     0,     0,
    -874,  -874,     0,     0,     0,     0,     0,  -874,     0,     0,
    -874,  -874,  -261,     0,     0,  -874,     0,     0,     0,     0,
    -874,  -874,     0,     0,     0,     0,  -874,     0,     0,     0,
    -874,     0,     0,     0,  -874,  -874,     0,  -874,     0,  -874,
    -874,     0,     0,     0,  -874,  -874,     0,     0,  -874,  -874,
    -874,  -874,  -874,  -874,     0,     0,  -874,     0,     0,     0,
    -874,  -874,     0,     0,  -874,     0,     0,     0,     0,  -874,
       0,     0,  -874,     0,     0,     0,     0,  -874,  -874,  -874,
    -874,  -874,     0,  -874,  -874,  -874,  -874,  -874,     0,     0,
       0,     0,  -874,  -874,  -874,     0,  -874,  -874,  -874,  -874,
    -874,  -874,     0,  -874,     0,  -874,     0,     0,     0,     0,
    -874,  -874,  -874,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  -874,     0,  -874,     0,
    -874,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  -874,     0,     0,     0,     0,  -874,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     1,     2,     0,     0,     0,     3,
       0,     0,     0,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,  -874,    15,  -874,     0,    16,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
       0,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,     0,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,     0,
      74,    75,    76,    77,    78,     0,    79,    80,    81,     0,
       0,    82,    83,     0,     0,     0,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,     0,     0,     0,
      95,     0,    96,    97,    98,    99,     0,   100,     0,     0,
       0,   101,   102,   103,   104,   105,     0,     0,     0,     0,
       0,     0,   106,   107,   108,   109,   110,   111,   112,   113,
     114,     0,   115,   116,   117,   118,   119,     0,     0,     0,
     120,   121,   122,   123,   124,   125,   126,   127,     0,   128,
       0,   129,   130,     0,     0,   131,   132,   133,     0,     0,
       0,   134,     0,   135,   136,   137,   138,   139,     0,   140,
       0,   141,     0,   142,   143,   144,   145,   146,   147,   148,
     149,   150,   151,   152,     0,     0,   153,   154,   155,     0,
     156,   157,   158,   159,   160,   161,   162,   163,   164,   165,
     166,   167,   168,   169,   170,   171,   172,   173,   174,   175,
     176,   177,   178,   179,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   197,   198,     0,   199,   200,   201,     0,   202,   203,
     204,   205,   206,   207,   208,   209,   210,   211,   212,   213,
     214,   215,   216,   217,   218,   219,   220,   221,   222,   223,
     224,   225,   226,   227,   228,   229,   230,   231,   232,   233,
     234,   235,   236,   237,   238,   239,   240,   241,   242,   243,
     244,   245,   246,   247,   248,   249,     0,   250,     0,   251,
     252,   253,   254,   255,   256,   257,   258,   259,     1,     2,
       0,     0,     0,     3,     0,     0,     0,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,     0,    15,
       0,     0,    16,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,     0,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,     0,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,     0,    74,    75,    76,    77,    78,     0,
      79,    80,    81,     0,     0,    82,    83,     0,     0,     0,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,     0,     0,     0,    95,     0,    96,    97,    98,    99,
       0,   100,     0,     0,     0,   101,   102,   103,   104,   105,
       0,     0,     0,     0,     0,     0,   106,   107,   108,   109,
     110,   111,   112,   113,   114,     0,   115,   116,   117,   118,
     119,     0,     0,     0,   120,   121,   122,   123,   124,   125,
     126,   127,     0,   128,     0,   129,   130,     0,     0,   131,
     132,   133,     0,     0,     0,   134,     0,   135,   136,   137,
     138,   139,     0,   140,     0,   141,     0,   142,   143,   144,
     145,   146,   147,   148,   149,   150,   151,   152,     0,     0,
     153,   154,   155,     0,   156,   157,   158,   159,   160,   161,
     162,   163,   164,   165,   166,   167,   168,   169,   170,   171,
     172,   173,   174,   175,   176,   177,   178,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,   197,   198,     0,   199,   200,
     201,     0,   202,   203,   204,   205,   206,   207,   208,   209,
     210,   211,   212,   213,   214,   215,   216,   217,   218,   219,
     220,   221,   222,   223,   224,   225,   226,   227,   228,   229,
     230,   231,   232,   233,   234,   235,   236,   237,   238,   239,
     240,   241,   242,   243,   244,   245,   246,   247,   248,   249,
       0,   250,     0,   251,   252,   253,   254,   255,   256,   257,
       0,   615,     1,     2,     0,     0,     0,     3,     0,     0,
       0,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,     0,    15,     0,     0,   421,   422,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,   423,    32,    33,    34,    35,    36,     0,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,     0,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,     0,    74,    75,
      76,    77,    78,     0,    79,    80,    81,     0,     0,    82,
      83,     0,     0,     0,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,     0,     0,     0,    95,     0,
      96,    97,    98,    99,     0,   100,     0,     0,     0,   101,
     102,   103,   104,   105,     0,     0,     0,     0,     0,     0,
     106,   107,   108,   109,   110,   111,   112,   113,   114,     0,
     115,   116,   117,   118,   119,     0,     0,     0,   120,   121,
     122,   123,   124,   125,   126,   127,     0,   128,     0,   129,
     130,     0,     0,   131,   132,   133,     0,   559,     0,   134,
       0,   135,   136,   137,   138,   139,     0,   140,     0,   141,
       0,   142,   143,   144,   145,   146,   147,   148,   149,   150,
     151,   152,     0,     0,   153,   154,   155,     0,   424,   157,
     158,   159,   160,   161,   162,   163,   164,   165,   166,   167,
     168,   169,   170,   171,   172,   173,   174,   175,   176,   177,
     178,   179,   180,   181,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   197,
     198,     0,   199,   200,   201,     0,   202,   203,   204,   205,
     206,   207,   208,   209,   210,   211,   212,   213,   214,   215,
     216,   217,   218,   219,   220,   221,   222,   223,   224,   225,
     226,   227,   228,   229,   230,   231,   232,   233,   234,   235,
     236,   237,   238,   239,   240,   241,   242,   243,   244,   245,
     246,   247,   248,   249,     0,   250,     0,   251,   252,   253,
     254,   255,   256,   257,     1,     2,     0,     0,     0,     3,
       0,     0,     0,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,     0,    15,     0,     0,   421,   422,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,   423,    32,    33,    34,    35,    36,
       0,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,     0,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,     0,
      74,    75,    76,    77,    78,     0,    79,    80,    81,     0,
       0,    82,    83,     0,     0,     0,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,     0,     0,     0,
      95,     0,    96,    97,    98,    99,     0,   100,     0,     0,
       0,   101,   102,   103,   104,   105,     0,     0,     0,     0,
       0,     0,   106,   107,   108,   109,   110,   111,   112,   113,
     114,     0,   115,   116,   117,   118,   119,     0,     0,     0,
     120,   121,   122,   123,   124,   125,   126,   127,     0,   128,
       0,   129,   130,     0,     0,   131,   132,   133,     0,   824,
       0,   134,     0,   135,   136,   137,   138,   139,     0,   140,
       0,   141,     0,   142,   143,   144,   145,   146,   147,   148,
     149,   150,   151,   152,     0,     0,   153,   154,   155,     0,
     424,   157,   158,   159,   160,   161,   162,   163,   164,   165,
     166,   167,   168,   169,   170,   171,   172,   173,   174,   175,
     176,   177,   178,   179,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   197,   198,     0,   199,   200,   201,     0,   202,   203,
     204,   205,   206,   207,   208,   209,   210,   211,   212,   213,
     214,   215,   216,   217,   218,   219,   220,   221,   222,   223,
     224,   225,   226,   227,   228,   229,   230,   231,   232,   233,
     234,   235,   236,   237,   238,   239,   240,   241,   242,   243,
     244,   245,   246,   247,   248,   249,     0,   250,     0,   251,
     252,   253,   254,   255,   256,   257,     1,     2,     0,     0,
       0,     3,     0,     0,     0,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,     0,    15,     0,     0,
     421,   422,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,   564,   423,    32,    33,    34,
      35,    36,     0,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,     0,    50,    51,    52,
     565,   566,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,     0,    74,    75,    76,    77,    78,     0,    79,    80,
      81,     0,     0,    82,    83,     0,     0,     0,    84,    85,
     514,    87,    88,    89,    90,    91,   567,    93,    94,     0,
       0,     0,    95,     0,    96,    97,    98,    99,     0,   100,
       0,     0,     0,   101,   102,   103,   104,   105,     0,     0,
       0,     0,   950,     0,   106,   107,   108,   109,   110,   111,
     112,   568,   114,     0,   115,   116,   117,   118,   119,     0,
       0,     0,   120,   121,   122,   123,   124,   125,   126,   127,
       0,   128,     0,   129,   130,     0,     0,   131,   132,   133,
       0,     0,     0,   134,   951,   135,   136,     0,   138,   139,
       0,   140,     0,   141,     0,   142,   143,   144,   145,   146,
     147,   148,   149,   150,   151,   152,     0,     0,   153,   154,
     155,     0,   424,   157,   158,   159,   160,   161,   162,   163,
     164,   165,   166,   167,   168,   169,   170,   171,   172,   173,
     174,   175,   176,   177,   178,   532,   533,   534,   182,   183,
     535,   570,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,   197,   198,     0,   199,   200,   201,     0,
     202,   203,   204,   205,   206,   207,   208,   209,   210,   211,
     212,   213,   214,   215,   216,   217,   218,   219,   220,   221,
     222,   223,   224,   225,   226,   227,   228,   229,   230,   231,
     232,   233,   234,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,   245,   246,   247,   248,   249,     0,   250,
       0,   251,   252,   253,   254,   255,   256,   257,     1,     2,
       0,     0,     0,     3,     0,     0,     0,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,     0,    15,
       0,     0,   421,   422,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,   564,   423,    32,
      33,    34,    35,    36,     0,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,     0,    50,
      51,    52,   565,   566,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,     0,    74,    75,    76,    77,    78,     0,
      79,    80,    81,     0,     0,    82,    83,     0,     0,     0,
      84,    85,   514,    87,    88,    89,    90,    91,   567,    93,
      94,     0,     0,     0,    95,     0,    96,    97,    98,    99,
       0,   100,     0,     0,     0,   101,   102,   103,   104,   105,
       0,     0,     0,     0,   950,     0,   106,   107,   108,   109,
     110,   111,   112,   568,   114,     0,   115,   116,   117,   118,
     119,     0,     0,     0,   120,   121,   122,   123,   124,   125,
     126,   127,     0,   128,     0,   129,   130,     0,     0,   131,
     132,   133,     0,     0,     0,   134,   956,   135,   136,     0,
     138,   139,     0,   140,     0,   141,     0,   142,   143,   144,
     145,   146,   147,   148,   149,   150,   151,   152,     0,     0,
     153,   154,   155,     0,   424,   157,   158,   159,   160,   161,
     162,   163,   164,   165,   166,   167,   168,   169,   170,   171,
     172,   173,   174,   175,   176,   177,   178,   532,   533,   534,
     182,   183,   535,   570,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,   197,   198,     0,   199,   200,
     201,     0,   202,   203,   204,   205,   206,   207,   208,   209,
     210,   211,   212,   213,   214,   215,   216,   217,   218,   219,
     220,   221,   222,   223,   224,   225,   226,   227,   228,   229,
     230,   231,   232,   233,   234,   235,   236,   237,   238,   239,
     240,   241,   242,   243,   244,   245,   246,   247,   248,   249,
       0,   250,     0,   251,   252,   253,   254,   255,   256,   257,
       1,     2,     0,     0,     0,     3,     0,     0,     0,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
       0,    15,     0,     0,   421,   422,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,  1352,
     423,    32,    33,    34,    35,    36,     0,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
       0,    50,    51,    52,  1353,  1354,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,     0,    74,    75,    76,    77,
      78,     0,    79,    80,    81,     0,     0,    82,    83,     0,
       0,     0,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,     0,     0,     0,    95,     0,    96,    97,
      98,    99,     0,   100,     0,     0,     0,   101,   102,   103,
     104,   105,     0,     0,     0,     0,     0,     0,   106,   107,
     108,   109,   110,   111,   112,   113,   114,     0,   115,   116,
     117,   118,   119,     0,     0,     0,   120,   121,   122,   123,
     124,   125,   126,   127,     0,   128,     0,   129,   130,     0,
       0,   131,   132,   133,     0,   559,     0,   134,     0,   135,
     136,   137,   138,   139,     0,   140,     0,   141,     0,   142,
     143,   144,   145,   146,   147,   148,   149,   150,   151,   152,
       0,     0,   153,   154,   155,     0,   424,   157,   158,   159,
     160,   161,   162,   163,   164,   165,   166,   167,   168,   169,
     170,   171,   172,   173,   174,   175,   176,   177,   178,   179,
     180,   181,   182,   183,   184,  1355,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,   197,   198,     0,
     199,   200,   201,     0,   202,   203,   204,   205,   206,   207,
     208,   209,   210,   211,   212,   213,   214,   215,   216,   217,
     218,   219,   220,   221,   222,   223,   224,   225,   226,   227,
     228,   229,   230,   231,   232,   233,   234,   235,   236,   237,
     238,   239,   240,   241,   242,   243,   244,   245,   246,   247,
     248,   249,     0,   250,     0,   251,   252,   253,   254,   255,
     256,   257,     1,     2,     0,     0,     0,     3,     0,     0,
       0,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,     0,    15,     0,     0,   421,   422,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,   423,    32,    33,    34,    35,    36,     0,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,     0,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,     0,    74,    75,
      76,    77,    78,     0,    79,    80,    81,     0,     0,    82,
      83,     0,     0,     0,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,     0,     0,     0,    95,     0,
      96,    97,    98,    99,     0,   100,     0,     0,     0,   101,
     102,   103,   104,   105,     0,     0,     0,     0,     0,     0,
     106,   107,   108,   109,   110,   111,   112,   113,   114,     0,
     115,   116,   117,   118,   119,     0,     0,     0,   120,   121,
     122,   123,   124,   125,   126,   127,     0,   128,     0,   129,
     130,     0,     0,   131,   132,   133,     0,     0,     0,   134,
       0,   135,   136,   137,   138,   139,     0,   140,     0,   141,
       0,   142,   143,   144,   145,   146,   147,   148,   149,   150,
     151,   152,     0,     0,   153,   154,   155,     0,   424,   157,
     158,   159,   160,   161,   162,   163,   164,   165,   166,   167,
     168,   169,   170,   171,   172,   173,   174,   175,   176,   177,
     178,   179,   180,   181,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   197,
     198,     0,   199,   200,   201,     0,   202,   203,   204,   205,
     206,   207,   208,   209,   210,   211,   212,   213,   214,   215,
     216,   217,   218,   219,   220,   221,   222,   223,   224,   225,
     226,   227,   228,   229,   230,   231,   232,   233,   234,   235,
     236,   237,   238,   239,   240,   241,   242,   243,   244,   245,
     246,   247,   248,   249,     0,   250,     0,   251,   252,   253,
     254,   255,   256,   257,     1,     2,     0,     0,     0,     3,
       0,     0,     0,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,     0,    15,     0,     0,   421,   422,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,   564,   423,    32,    33,    34,    35,    36,
       0,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,     0,    50,    51,    52,   565,   566,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,     0,
      74,    75,    76,    77,    78,     0,    79,    80,    81,     0,
       0,    82,    83,     0,     0,     0,    84,    85,   514,    87,
      88,    89,    90,    91,   567,    93,    94,     0,     0,     0,
      95,     0,    96,    97,    98,    99,     0,   100,     0,     0,
       0,   101,   102,   103,   104,   105,     0,     0,     0,     0,
       0,     0,   106,   107,   108,   109,   110,   111,   112,   568,
     114,     0,   115,   116,   117,   118,   119,     0,     0,     0,
     120,   121,   122,   123,   124,   125,   126,   127,     0,   128,
       0,   129,   130,     0,     0,   131,   132,   133,     0,     0,
     569,   134,     0,   135,   136,     0,   138,   139,     0,   140,
       0,   141,     0,   142,   143,   144,   145,   146,   147,   148,
     149,   150,   151,   152,     0,     0,   153,   154,   155,     0,
     424,   157,   158,   159,   160,   161,   162,   163,   164,   165,
     166,   167,   168,   169,   170,   171,   172,   173,   174,   175,
     176,   177,   178,   532,   533,   534,   182,   183,   535,   570,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   197,   198,     0,   199,   200,   201,     0,   202,   203,
     204,   205,   206,   207,   208,   209,   210,   211,   212,   213,
     214,   215,   216,   217,   218,   219,   220,   221,   222,   223,
     224,   225,   226,   227,   228,   229,   230,   231,   232,   233,
     234,   235,   236,   237,   238,   239,   240,   241,   242,   243,
     244,   245,   246,   247,   248,   249,     0,   250,     0,   251,
     252,   253,   254,   255,   256,   257,     1,     2,     0,     0,
       0,     3,     0,     0,     0,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,     0,    15,     0,     0,
     421,   422,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,   564,   423,    32,    33,    34,
      35,    36,     0,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,     0,    50,    51,    52,
     565,   566,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,     0,    74,    75,    76,    77,    78,     0,    79,    80,
      81,     0,     0,    82,    83,     0,     0,     0,    84,    85,
     514,    87,    88,    89,    90,    91,   567,    93,    94,     0,
       0,     0,    95,     0,    96,    97,    98,    99,     0,   100,
       0,     0,     0,   101,   102,   103,   104,   105,     0,     0,
       0,     0,     0,     0,   106,   107,   108,   109,   110,   111,
     112,   568,   114,     0,   115,   116,   117,   118,   119,     0,
       0,     0,   120,   121,   122,   123,   124,   125,   126,   127,
       0,   128,     0,   129,   130,     0,     0,   131,   132,   133,
       0,     0,     0,   134,   575,   135,   136,     0,   138,   139,
       0,   140,     0,   141,     0,   142,   143,   144,   145,   146,
     147,   148,   149,   150,   151,   152,     0,     0,   153,   154,
     155,     0,   424,   157,   158,   159,   160,   161,   162,   163,
     164,   165,   166,   167,   168,   169,   170,   171,   172,   173,
     174,   175,   176,   177,   178,   532,   533,   534,   182,   183,
     535,   570,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,   197,   198,     0,   199,   200,   201,     0,
     202,   203,   204,   205,   206,   207,   208,   209,   210,   211,
     212,   213,   214,   215,   216,   217,   218,   219,   220,   221,
     222,   223,   224,   225,   226,   227,   228,   229,   230,   231,
     232,   233,   234,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,   245,   246,   247,   248,   249,     0,   250,
       0,   251,   252,   253,   254,   255,   256,   257,     1,     2,
       0,     0,     0,     3,     0,     0,     0,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,     0,    15,
       0,     0,   421,   422,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,   564,   423,    32,
      33,    34,    35,    36,     0,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,     0,    50,
      51,    52,   565,   566,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,     0,    74,    75,    76,    77,    78,     0,
      79,    80,    81,     0,     0,    82,    83,     0,     0,     0,
      84,    85,   514,    87,    88,    89,    90,    91,   567,    93,
      94,     0,     0,     0,    95,     0,    96,    97,    98,    99,
       0,   100,     0,     0,     0,   101,   102,   103,   104,   105,
       0,     0,     0,     0,     0,     0,   106,   107,   108,   109,
     110,   111,   112,   568,   114,     0,   115,   116,   117,   118,
     119,     0,     0,     0,   120,   121,   122,   123,   124,   125,
     126,   127,     0,   128,     0,   129,   130,     0,     0,   131,
     132,   133,     0,     0,     0,   134,     0,   135,   136,     0,
     138,   139,     0,   140,     0,   141,     0,   142,   143,   144,
     145,   146,   147,   148,   149,   150,   151,   152,     0,     0,
     153,   154,   155,     0,   424,   157,   158,   159,   160,   161,
     162,   163,   164,   165,   166,   167,   168,   169,   170,   171,
     172,   173,   174,   175,   176,   177,   178,   532,   533,   534,
     182,   183,   535,   570,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,   197,   198,     0,   199,   200,
     201,     0,   202,   203,   204,   205,   206,   207,   208,   209,
     210,   211,   212,   213,   214,   215,   216,   217,   218,   219,
     220,   221,   222,   223,   224,   225,   226,   227,   228,   229,
     230,   231,   232,   233,   234,   235,   236,   237,   238,   239,
     240,   241,   242,   243,   244,   245,   246,   247,   248,   249,
     610,   250,     0,   251,   252,   253,   254,   255,   256,   257,
       1,     2,     0,     0,     0,     3,     0,     0,     0,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
       0,    15,     0,     0,   421,   422,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,   564,
     423,    32,    33,    34,    35,    36,     0,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
       0,    50,    51,    52,   565,   566,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,     0,    74,    75,    76,    77,
      78,     0,    79,    80,    81,     0,     0,    82,    83,     0,
       0,     0,    84,    85,   514,    87,    88,    89,    90,    91,
     567,    93,    94,     0,     0,     0,    95,     0,    96,    97,
      98,    99,     0,   100,     0,     0,     0,   101,   102,   103,
     104,   105,     0,     0,     0,     0,     0,     0,   106,   107,
     108,   109,   110,   111,   112,   568,   114,     0,   115,   116,
     117,   118,   119,     0,     0,     0,   120,   121,   122,   123,
     124,   125,   126,   127,     0,   128,     0,   129,   130,     0,
       0,   131,   132,   133,     0,     0,     0,   134,     0,   135,
     136,     0,   138,   139,     0,   140,     0,   141,     0,   142,
     143,   144,   145,   146,   147,   148,   149,   150,   151,   152,
       0,     0,   153,   154,   155,     0,   424,   157,   158,   159,
     160,   161,   162,   163,   164,   165,   166,   167,   168,   169,
     170,   171,   172,   173,   174,   175,   176,   177,   178,   532,
     533,   534,   182,   183,   535,   570,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,   197,   198,     0,
     199,   200,   201,     0,   202,   203,   204,   205,   206,   207,
     208,   209,   210,   211,   212,   213,   214,   215,   216,   217,
     218,   219,   220,   221,   222,   223,   224,   225,   226,   227,
     228,   229,   230,   231,   232,   233,   234,   235,   236,   237,
     238,   239,   240,   241,   242,   243,   244,   245,   246,   247,
     248,   249,     0,   250,   612,   251,   252,   253,   254,   255,
     256,   257,     1,     2,     0,     0,     0,     3,     0,     0,
       0,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,     0,    15,     0,     0,    16,    17,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,     0,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,     0,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,     0,    74,    75,
      76,    77,    78,     0,    79,    80,    81,     0,     0,    82,
      83,     0,     0,     0,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,     0,     0,     0,    95,     0,
      96,    97,    98,    99,     0,   100,     0,     0,     0,   101,
     102,   103,   104,   105,     0,     0,     0,     0,     0,     0,
     106,   107,   108,   109,   110,   111,   112,   113,   114,     0,
     115,   116,   117,   118,   119,     0,     0,     0,   120,   121,
     122,   123,   124,   125,   126,   127,     0,   128,     0,   129,
     130,     0,     0,   131,   132,   133,     0,     0,     0,   134,
       0,   135,   136,   137,   138,   139,     0,   140,     0,   141,
       0,   142,   143,   144,   145,   146,   147,   148,   149,   150,
     151,   152,     0,     0,   153,   154,   155,     0,   156,   157,
     158,   159,   160,   161,   162,   163,   164,   165,   166,   167,
     168,   169,   170,   171,   172,   173,   174,   175,   176,   177,
     178,   179,   180,   181,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   197,
     198,     0,   199,   200,   201,     0,   202,   203,   204,   205,
     206,   207,   208,   209,   210,   211,   212,   213,   214,   215,
     216,   217,   218,   219,   220,   221,   222,   223,   224,   225,
     226,   227,   228,   229,   230,   231,   232,   233,   234,   235,
     236,   237,   238,   239,   240,   241,   242,   243,   244,   245,
     246,   247,   248,   249,     0,   250,     0,   251,   252,   253,
     254,   255,   256,   257,     1,     2,     0,     0,     0,     3,
       0,     0,     0,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,     0,    15,     0,     0,    16,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
       0,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,     0,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,     0,
      74,    75,    76,    77,    78,     0,    79,    80,    81,     0,
       0,    82,    83,     0,     0,     0,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,     0,     0,     0,
      95,     0,    96,    97,    98,    99,     0,   100,     0,     0,
       0,   101,   102,   103,   104,   105,     0,     0,     0,     0,
       0,     0,   106,   107,   108,   109,   110,   111,   112,   113,
     114,     0,   115,   116,   117,   118,   119,     0,     0,     0,
     120,   121,   122,   123,   124,   125,   126,   127,     0,   128,
       0,   129,   130,     0,     0,   131,   132,   133,     0,     0,
       0,   134,     0,   135,   136,   137,   138,   139,     0,   140,
       0,   141,     0,   142,   143,   144,   145,   146,   147,   148,
     149,   150,   151,   152,     0,     0,   153,   154,   155,     0,
     424,   157,   158,   159,   160,   161,   162,   163,   164,   165,
     166,   167,   168,   169,   170,   171,   172,   173,   174,   175,
     176,   177,   178,   179,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   197,   198,     0,   199,   200,   201,     0,   202,   203,
     204,   205,   206,   207,   208,   209,   210,   211,   212,   213,
     214,   215,   216,   217,   218,   219,   220,   221,   222,   223,
     224,   225,   226,   227,   228,   229,   230,   231,   232,   233,
     234,   235,   236,   237,   238,   239,   240,   241,   242,   243,
     244,   245,   246,   247,   248,   249,     0,   250,     0,   251,
     252,   253,   254,   255,   256,   257,     1,     2,     0,     0,
       0,     3,     0,     0,     0,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,     0,    15,     0,     0,
      16,   422,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,     0,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,     0,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,     0,    74,    75,    76,    77,    78,     0,    79,    80,
      81,     0,     0,    82,    83,     0,     0,     0,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,     0,
       0,     0,    95,     0,    96,    97,    98,    99,     0,   100,
       0,     0,     0,   101,   102,   103,   104,   105,     0,     0,
       0,     0,     0,     0,   106,   107,   108,   109,   110,   111,
     112,   113,   114,     0,   115,   116,   117,   118,   119,     0,
       0,     0,   120,   121,   122,   123,   124,   125,   126,   127,
       0,   128,     0,   129,   130,     0,     0,   131,   132,   133,
       0,     0,     0,   134,     0,   135,   136,   137,   138,   139,
       0,   140,     0,   141,     0,   142,   143,   144,   145,   146,
     147,   148,   149,   150,   151,   152,     0,     0,   153,   154,
     155,     0,   424,   157,   158,   159,   160,   161,   162,   163,
     164,   165,   166,   167,   168,   169,   170,   171,   172,   173,
     174,   175,   176,   177,   178,   179,   180,   181,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,   197,   198,     0,   199,   200,   201,     0,
     202,   203,   204,   205,   206,   207,   208,   209,   210,   211,
     212,   213,   214,   215,   216,   217,   218,   219,   220,   221,
     222,   223,   224,   225,   226,   227,   228,   229,   230,   231,
     232,   233,   234,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,   245,   246,   247,   248,   249,     0,   250,
       0,   251,   252,   253,   254,   255,   256,   257,     1,     2,
       0,     0,     0,     3,     0,     0,     0,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,     0,    15,
       0,     0,   893,   422,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,     0,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,     0,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,     0,    74,    75,    76,    77,    78,     0,
      79,    80,    81,     0,     0,    82,    83,     0,     0,     0,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,     0,     0,     0,    95,     0,    96,    97,    98,    99,
       0,   100,     0,     0,     0,   101,   102,   103,   104,   105,
       0,     0,     0,     0,     0,     0,   106,   107,   108,   109,
     110,   111,   112,   113,   114,     0,   115,   116,   117,   118,
     119,     0,     0,     0,   120,   121,   122,   123,   124,   125,
     126,   127,     0,   128,     0,   129,   130,     0,     0,   131,
     132,   133,     0,     0,     0,   134,     0,   135,   136,   137,
     138,   139,     0,   140,     0,   141,     0,   142,   143,   144,
     145,   146,   147,   148,   149,   150,   151,   152,     0,     0,
     153,   154,   155,     0,   424,   157,   158,   159,   160,   161,
     162,   163,   164,   165,   166,   167,   168,   169,   170,   171,
     172,   173,   174,   175,   176,   177,   178,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,   197,   198,     0,   199,   200,
     201,     0,   202,   203,   204,   205,   206,   207,   208,   209,
     210,   211,   212,   213,   214,   215,   216,   217,   218,   219,
     220,   221,   222,   223,   224,   225,   226,   227,   228,   229,
     230,   231,   232,   233,   234,   235,   236,   237,   238,   239,
     240,   241,   242,   243,   244,   245,   246,   247,   248,   249,
       0,   250,     0,   251,   252,   253,   254,   255,   256,   257,
       1,     2,     0,     0,     0,     3,     0,     0,     0,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
       0,    15,     0,     0,   421,   422,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,  1352,
     423,    32,    33,    34,    35,    36,     0,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
       0,    50,    51,    52,  1353,  1354,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,     0,    74,    75,    76,    77,
      78,     0,    79,    80,    81,     0,     0,    82,    83,     0,
       0,     0,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,     0,     0,     0,    95,     0,    96,    97,
      98,    99,     0,   100,     0,     0,     0,   101,   102,   103,
     104,   105,     0,     0,     0,     0,     0,     0,   106,   107,
     108,   109,   110,   111,   112,   113,   114,     0,   115,   116,
     117,   118,   119,     0,     0,     0,   120,   121,   122,   123,
     124,   125,   126,   127,     0,   128,     0,   129,   130,     0,
       0,   131,   132,   133,     0,     0,     0,   134,     0,   135,
     136,   137,   138,   139,     0,   140,     0,   141,     0,   142,
     143,   144,   145,   146,   147,   148,   149,   150,   151,   152,
       0,     0,   153,   154,   155,     0,   424,   157,   158,   159,
     160,   161,   162,   163,   164,   165,   166,   167,   168,   169,
     170,   171,   172,   173,   174,   175,   176,   177,   178,   179,
     180,   181,   182,   183,   184,  1355,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,   197,   198,     0,
     199,   200,   201,     0,   202,   203,   204,   205,   206,   207,
     208,   209,   210,   211,   212,   213,   214,   215,   216,   217,
     218,   219,   220,   221,   222,   223,   224,   225,   226,   227,
     228,   229,   230,   231,   232,   233,   234,   235,   236,   237,
     238,   239,   240,   241,   242,   243,   244,   245,   246,   247,
     248,   249,     0,   250,     0,   251,   252,   253,   254,   255,
     256,   257,     1,     2,     0,     0,     0,     3,     0,     0,
       0,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,     0,    15,     0,     0,   421,   422,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,   564,   423,    32,    33,    34,    35,    36,     0,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,     0,    50,    51,    52,   565,   566,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,     0,    74,    75,
      76,    77,    78,     0,    79,    80,    81,     0,     0,    82,
      83,     0,     0,     0,    84,    85,   514,    87,    88,    89,
      90,    91,   567,    93,    94,     0,     0,     0,    95,     0,
      96,    97,    98,    99,     0,   100,     0,     0,     0,   101,
     102,   103,   104,   105,     0,     0,     0,     0,  1448,     0,
     106,   107,   108,   109,   110,   111,   112,   568,   114,     0,
     115,   116,   117,   118,   119,     0,     0,     0,   120,   121,
     122,   123,   124,   125,   126,   127,     0,   128,     0,   129,
     130,     0,     0,   131,   132,   133,     0,     0,     0,   134,
       0,   135,   136,     0,   138,   139,     0,   140,     0,   141,
       0,   142,   143,   144,   145,   146,   147,   148,   149,   150,
     151,   152,     0,     0,   153,   154,   155,     0,   424,   157,
     158,   159,   160,   161,   162,   163,   164,   165,   166,   167,
     168,   169,   170,   171,   172,   173,   174,   175,   176,   177,
     178,   532,   533,   534,   182,   183,   535,   570,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   197,
     198,     0,   199,   200,   201,     0,   202,   203,   204,   205,
     206,   207,   208,   209,   210,   211,   212,   213,   214,   215,
     216,   217,   218,   219,   220,   221,   222,   223,   224,   225,
     226,   227,   228,   229,   230,   231,   232,   233,   234,   235,
     236,   237,   238,   239,   240,   241,   242,   243,   244,   245,
     246,   247,   248,   249,     0,   250,     0,   251,   252,   253,
     254,   255,   256,   257,     1,     2,     0,     0,     0,     3,
       0,     0,     0,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,     0,    15,     0,     0,   421,   422,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,   500,    29,   502,   423,    32,   503,    34,    35,    36,
       0,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,   505,     0,    50,    51,    52,   507,   508,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,     0,
      74,    75,    76,    77,    78,     0,    79,    80,    81,     0,
       0,    82,    83,     0,     0,     0,    84,    85,   514,    87,
      88,    89,    90,    91,   567,    93,    94,     0,     0,     0,
      95,     0,    96,    97,    98,    99,     0,   100,     0,     0,
       0,   517,   102,   103,   104,   105,     0,     0,     0,     0,
       0,     0,   106,   107,   108,   109,   110,   111,   112,   568,
     114,     0,   115,   116,   698,   118,   119,     0,     0,     0,
     120,   121,   122,   123,   124,   125,   126,   127,     0,   128,
       0,   699,   130,     0,     0,   131,   132,   133,     0,     0,
       0,   134,     0,   135,   136,     0,   138,   139,     0,   140,
       0,   141,     0,   142,   143,   144,   145,   146,   147,   148,
     149,   150,   151,   152,     0,     0,   153,   154,   155,     0,
     424,   157,   158,   159,   160,   161,   162,   163,   164,   527,
     166,   528,   168,   529,   530,   171,   172,   173,   174,   175,
     176,   531,   178,   532,   533,   534,   182,   183,   535,   536,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   197,   198,     0,   199,   200,   201,     0,   202,   203,
     204,   205,   206,   207,   208,   209,   210,   211,   212,   213,
     214,   215,   216,   217,   218,   219,   220,   221,   222,   223,
     224,   225,   226,   227,   228,   229,   230,   231,   232,   233,
     234,   235,   236,   237,   238,   239,   240,   241,   242,   243,
     244,   245,   246,   247,   248,   249,     0,   250,     0,   251,
     537,   253,   254,   255,   256,   257,     1,     2,     0,     0,
       0,     3,     0,     0,     0,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,     0,    15,     0,     0,
     421,   422,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,   564,   423,    32,    33,    34,
      35,    36,     0,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,     0,    50,    51,    52,
     565,   566,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,     0,    74,    75,    76,    77,    78,     0,    79,    80,
      81,     0,     0,    82,    83,     0,     0,     0,    84,    85,
     514,    87,    88,    89,    90,    91,   567,    93,    94,     0,
       0,     0,    95,     0,    96,    97,    98,    99,     0,   100,
       0,     0,     0,   101,   102,   103,   104,   105,     0,     0,
       0,     0,     0,     0,   106,   107,   108,   109,   110,   111,
     112,   568,   114,     0,   115,   116,   117,   118,   119,     0,
       0,     0,   120,   121,   122,   123,   124,   125,   126,   127,
       0,   128,     0,   129,   130,     0,     0,   131,   132,   133,
       0,     0,     0,   134,     0,   135,   136,     0,   138,   139,
       0,   140,     0,   141,     0,   142,   143,   144,   145,   146,
     147,   148,   149,   150,   151,   152,     0,     0,   153,   154,
     155,     0,   424,   157,   158,   159,   160,   161,   162,   163,
     164,   165,   166,   167,   168,   169,   170,   171,   172,   173,
     174,   175,   176,   177,   178,   532,   533,   534,   182,   183,
     535,   570,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,   197,   198,     0,   199,   200,   201,     0,
     202,   203,   204,   205,   206,   207,   208,   209,   210,   211,
     212,   213,   214,   215,   216,   217,   218,   219,   220,   221,
     222,   223,   224,   225,   226,   227,   228,   229,   230,   231,
     232,   233,   234,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,   245,   246,   247,   248,   249,     0,   250,
       0,   251,   252,   253,   254,   255,   256,   257,     1,     2,
       0,     0,     0,     3,     0,     0,     0,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,     0,    15,
       0,     0,   421,   422,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,   500,    29,   502,   423,    32,
     503,    34,    35,    36,     0,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,   505,     0,    50,
      51,    52,   507,   508,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,     0,    74,    75,    76,    77,    78,     0,
      79,    80,    81,     0,     0,    82,    83,     0,     0,     0,
      84,    85,   514,    87,    88,    89,    90,    91,   567,    93,
      94,     0,     0,     0,    95,     0,    96,    97,    98,    99,
       0,   100,     0,     0,     0,   517,   102,   103,   104,   105,
       0,     0,     0,     0,     0,     0,   106,   107,   108,   109,
     110,   111,   112,   568,   114,     0,   115,   116,   117,   118,
     119,     0,     0,     0,   120,   121,   122,   123,   124,   125,
     126,   127,     0,   128,     0,   129,   130,     0,     0,   131,
     132,   133,     0,     0,     0,   134,     0,   135,   136,     0,
     138,   139,     0,   140,     0,   141,     0,   142,   143,   144,
     145,   146,   147,   148,   149,   150,   151,   152,     0,     0,
     153,   154,   155,     0,   424,   157,   158,   159,   160,   161,
     162,   163,   164,   527,   166,   528,   168,   529,   530,   171,
     172,   173,   174,   175,   176,   531,   178,   532,   533,   534,
     182,   183,   535,   536,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,   197,   198,     0,   199,   200,
     201,     0,   202,   203,   204,   205,   206,   207,   208,   209,
     210,   211,   212,   213,   214,   215,   216,   217,   218,   219,
     220,   221,   222,   223,   224,   225,   226,   227,   228,   229,
     230,   231,   232,   233,   234,   235,   236,   237,   238,   239,
     240,   241,   242,   243,   244,   245,   246,   247,   248,   249,
       0,   250,     0,   251,   537,   253,   254,   255,   256,   257,
       1,     2,     0,     0,     0,     3,     0,     0,     0,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
       0,    15,     0,     0,   421,   422,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,   564,
     423,    32,    33,    34,    35,    36,     0,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
       0,    50,    51,    52,   565,   566,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,     0,    74,    75,    76,    77,
      78,     0,    79,    80,    81,     0,     0,    82,    83,     0,
       0,     0,    84,    85,   514,    87,    88,    89,    90,    91,
    1136,    93,    94,     0,     0,     0,    95,     0,    96,    97,
      98,    99,     0,   100,     0,     0,     0,   101,   102,   103,
     104,   105,     0,     0,     0,     0,     0,     0,   106,   107,
     108,   109,   110,   111,   112,   568,   114,     0,   115,   116,
     117,   118,   119,     0,     0,     0,   120,   121,   122,   123,
     124,   125,   126,   127,     0,   128,     0,   129,   130,     0,
       0,   131,   132,   133,     0,     0,     0,   134,     0,   135,
     136,     0,   138,   139,     0,   140,     0,   141,     0,   142,
     143,   144,   145,   146,   147,   148,   149,   150,   151,   152,
       0,     0,   153,   154,   155,     0,   424,   157,   158,   159,
     160,   161,   162,   163,   164,   165,   166,   167,   168,   169,
     170,   171,   172,   173,   174,   175,   176,   177,   178,   532,
     533,   534,   182,   183,   535,   570,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,   197,   198,     0,
     199,   200,   201,     0,   202,   203,   204,   205,   206,   207,
     208,   209,   210,   211,   212,   213,   214,   215,   216,   217,
     218,   219,   220,   221,   222,   223,   224,   225,   226,   227,
     228,   229,   230,   231,   232,   233,   234,   235,   236,   237,
     238,   239,   240,   241,   242,   243,   244,   245,   246,   247,
     248,   249,     0,   250,     0,   251,   252,   253,   254,   255,
     256,   257,     1,     2,     0,     0,     0,     3,     0,     0,
       0,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,     0,    15,     0,     0,   421,   422,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,   500,
      29,   502,   423,    32,   503,    34,    35,    36,     0,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,   505,     0,    50,    51,    52,   507,   508,    55,   509,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,     0,    74,    75,
      76,    77,    78,     0,    79,    80,    81,     0,     0,    82,
      83,     0,     0,     0,    84,    85,   514,    87,    88,    89,
      90,    91,   567,    93,    94,     0,     0,     0,    95,     0,
      96,    97,    98,    99,     0,   100,     0,     0,     0,   517,
     102,   103,   104,   105,     0,     0,     0,     0,     0,     0,
     106,   107,   108,   109,   110,   111,   112,   568,   114,     0,
     115,   116,     0,   118,   119,     0,     0,     0,   120,   121,
     122,   123,   124,   125,   126,   127,     0,   128,     0,     0,
       0,     0,     0,   131,   132,   133,     0,     0,     0,   134,
       0,   135,   136,     0,   138,   139,     0,   140,     0,   141,
       0,   142,   143,   144,   145,   146,   147,   148,   149,   150,
     151,   152,     0,     0,   153,   154,   155,     0,   424,   157,
     158,   159,   160,   161,   162,   163,   164,   527,   166,   528,
     168,   529,   530,   171,   172,   173,   174,   175,   176,   531,
     178,   532,   533,   534,   182,   183,   535,   536,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   197,
     198,     0,   199,   200,   201,     0,   202,   203,   204,   205,
     206,   207,   208,   209,   210,   211,   212,   213,   214,   215,
     216,   217,   218,   219,   220,   221,   222,   223,   224,   225,
     226,   227,   228,   229,   230,   231,   232,   233,   234,   235,
     236,   237,   238,   239,   240,   241,   242,   243,   244,   245,
     246,   247,   248,   249,     0,   250,     0,   251,   537,   253,
     254,   255,   256,   257,     1,     2,     0,     0,     0,     3,
       0,     0,     0,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,     0,    15,     0,     0,   421,   422,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,   500,    29,   502,   423,    32,   503,    34,    35,    36,
       0,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,   505,     0,    50,    51,    52,   507,   508,
      55,   509,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,     0,
      74,    75,    76,    77,    78,     0,    79,    80,    81,     0,
       0,    82,    83,     0,     0,     0,    84,    85,   514,    87,
      88,    89,    90,    91,   567,    93,    94,     0,     0,     0,
      95,     0,    96,    97,    98,    99,     0,   100,     0,     0,
       0,   517,   102,   103,   104,   105,     0,     0,     0,     0,
       0,     0,   106,   107,   108,   109,   110,   111,   112,   568,
     114,     0,   115,   116,     0,   118,   119,     0,     0,     0,
     120,   121,   122,   123,   124,   125,   126,   127,     0,   128,
       0,     0,     0,     0,     0,   131,   132,   133,     0,     0,
       0,   134,     0,   135,   136,     0,     0,     0,     0,   140,
       0,   141,     0,   142,   143,   144,   145,   146,   147,   148,
     149,   150,   151,   152,     0,     0,   153,   154,   155,     0,
     424,   157,   158,   159,   160,   161,   162,   163,   164,   527,
     166,   528,   168,   529,   530,   171,   172,   173,   174,   175,
     176,   531,   178,   532,   533,   534,   182,   183,   535,   536,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   197,   198,     0,   199,   200,   201,     0,   202,   203,
     204,   205,   206,   207,   208,   209,   210,   211,   212,   213,
     214,   215,   216,   217,   218,   219,   220,   221,   222,   223,
     224,   225,   226,   227,   228,   229,   230,   231,   232,   233,
     234,   235,   236,   237,   238,   239,   240,   241,   242,   243,
     244,   245,   246,   247,   248,   249,     0,   250,     0,   251,
     537,   253,   254,   255,   256,   257,     1,     2,     0,     0,
       0,     3,     0,     0,     0,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,     0,    15,     0,     0,
     421,   422,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,   500,    29,   502,   423,    32,   503,    34,
      35,    36,     0,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,   505,     0,    50,    51,    52,
     507,   508,    55,   509,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,     0,    74,    75,    76,    77,    78,     0,    79,    80,
      81,     0,     0,    82,    83,     0,     0,     0,    84,    85,
     514,    87,    88,    89,    90,    91,   567,    93,    94,     0,
       0,     0,    95,     0,    96,    97,    98,    99,     0,   100,
       0,     0,     0,   517,     0,   103,   104,   105,     0,     0,
       0,     0,     0,     0,   106,   107,   108,   109,     0,   111,
     112,   568,   114,     0,   115,   116,     0,   118,   119,     0,
       0,     0,   120,   121,   122,   123,   124,   125,   126,   127,
       0,   128,     0,     0,     0,     0,     0,   131,   132,   133,
       0,     0,     0,   134,     0,   135,   136,     0,     0,     0,
       0,   140,     0,   141,     0,   142,     0,   144,     0,   146,
     147,   148,   149,   150,   151,   152,     0,     0,   153,   154,
     155,     0,   424,   157,   158,   159,   160,   161,   162,   163,
     164,   527,   166,   528,   168,   529,   530,   171,   172,   173,
     174,   175,   176,   531,   178,   532,   533,   534,   182,   183,
     535,   536,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,   197,   198,     0,   199,   200,   201,     0,
     202,   203,   204,   205,   206,   207,   208,   209,   210,   211,
     212,   213,   214,   215,   216,   217,   218,   219,   220,   221,
     222,   223,   224,   225,   226,   227,   228,   229,   230,   231,
     232,   233,   234,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,   245,   246,   247,   248,   249,     0,   250,
       0,   251,   537,   253,   254,   255,   256,   257,     1,     2,
       0,     0,     0,     3,     0,     0,     0,     4,     0,     0,
       0,     0,     0,     0,    11,    12,     0,     0,     0,     0,
       0,     0,   421,   422,     0,    18,    19,    20,   497,    22,
      23,    24,   498,    26,   499,   500,   501,   502,   423,    32,
     503,    34,    35,    36,     0,    37,    38,    39,    40,   504,
      42,    43,    44,    45,    46,    47,    48,   505,     0,    50,
     506,    52,   507,   508,    55,   509,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
     510,   511,    73,     0,    74,    75,    76,   512,     0,     0,
      79,    80,    81,     0,     0,   513,    83,     0,     0,     0,
       0,    85,   514,    87,   515,   516,    90,    91,     0,     0,
       0,     0,     0,     0,     0,     0,    96,    97,    98,    99,
       0,   100,     0,     0,     0,   517,   102,   103,   518,   519,
       0,     0,     0,     0,     0,     0,   106,   107,   108,     0,
     110,   111,   112,     0,     0,     0,     0,     0,     0,   118,
     520,     0,     0,     0,   120,   121,   122,   123,   521,   125,
     126,   522,     0,     0,     0,     0,     0,     0,     0,   523,
     524,   133,     0,     0,     0,   134,     0,   135,   525,     0,
       0,     0,     0,   140,     0,   141,     0,   142,   143,   144,
     145,   526,   147,   148,   149,   150,   151,   152,     0,     0,
     153,   154,     0,     0,   424,   157,   158,   159,   160,   161,
     162,   163,   164,   527,   166,   528,   168,   529,   530,   171,
     172,   173,   174,   175,   176,   531,   178,   532,   533,   534,
     182,   183,   535,   536,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,   197,   198,     0,   199,   200,
     201,     0,   202,   203,   204,   205,   206,   207,   208,   209,
     210,   211,   212,   213,   214,   215,   216,   217,   218,   219,
     220,   221,   222,   223,   224,   225,   226,   227,   228,   229,
     230,   231,   232,   233,   234,   235,   236,   237,   238,   239,
     240,   241,   242,   243,   244,   245,   246,   247,   248,     0,
       0,     0,     0,   251,   537,   253,   254,   255,   256,   257,
       1,     2,     0,     0,     0,     3,     0,     0,     0,     4,
       0,     0,     0,     0,     0,     0,    11,    12,     0,     0,
       0,     0,     0,     0,   421,   422,     0,    18,    19,    20,
     545,    22,    23,    24,   498,   546,   547,   500,   501,   502,
     423,    32,   503,    34,   548,    36,     0,    37,    38,    39,
      40,   549,    42,   550,   551,    45,    46,    47,    48,   505,
       0,    50,   552,    52,   507,   508,    55,   509,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,   510,   511,    73,     0,    74,    75,    76,   553,
       0,     0,    79,    80,    81,     0,     0,   513,    83,     0,
       0,     0,     0,    85,   514,    87,   515,   516,    90,    91,
       0,     0,     0,     0,     0,     0,     0,     0,    96,    97,
      98,    99,     0,   100,     0,     0,     0,   517,   102,   103,
     518,   519,     0,     0,     0,     0,     0,     0,   106,   107,
     108,     0,   110,   111,   112,     0,     0,     0,     0,     0,
       0,   118,   520,     0,     0,     0,   120,   121,   122,   123,
     521,   125,   126,   522,     0,     0,     0,     0,     0,     0,
       0,   523,   524,   133,     0,     0,     0,   134,     0,   135,
     525,     0,     0,     0,     0,   140,     0,   141,     0,   142,
     143,   144,   145,   526,   147,   148,   149,   150,   151,   152,
       0,     0,   153,   154,     0,     0,   424,   157,   158,   159,
     160,   161,   162,   163,   164,   527,   554,   528,   168,   529,
     530,   171,   172,   173,   174,   175,   176,   531,   178,   532,
     533,   534,   182,   183,   535,   536,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,   197,   198,     0,
     199,   200,   201,     0,   202,   203,   204,   205,   206,   207,
     208,   209,   210,   211,   212,   213,   214,   215,   216,   217,
     218,   219,   220,   221,   222,   223,   224,   225,   226,   227,
     228,   229,   230,   231,   232,   233,   234,   235,   236,   237,
     238,   239,   240,   241,   242,   243,   244,   245,   246,   247,
     248,     3,     0,     0,     0,   251,   537,   253,   254,   255,
     256,   257,    11,    12,    13,    14,     0,     0,     0,     0,
     421,   422,     0,    18,    19,    20,   497,    22,    23,    24,
     498,    26,   499,   500,  1162,   502,   423,    32,   503,    34,
      35,    36,     0,    37,    38,    39,    40,   504,    42,    43,
      44,    45,    46,    47,    48,   505,     0,    50,   506,    52,
     507,   508,    55,   509,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,   510,   511,
      73,     0,    74,    75,    76,   512,     0,     0,    79,    80,
      81,     0,     0,   513,    83,     0,     0,     0,     0,    85,
     514,    87,   515,   516,    90,    91,     0,     0,     0,     0,
       0,     0,     0,     0,    96,    97,    98,    99,     0,   100,
       0,     0,     0,   517,   102,   103,   518,   519,     0,     0,
       0,     0,     0,     0,   106,   107,   108,     0,   110,   111,
    1163,     0,     0,     0,  1164,     0,     0,   118,   520,     0,
       0,     0,   120,   121,   122,   123,   521,   125,   126,   522,
       0,     0,     0,     0,     0,     0,     0,   523,   524,   133,
       0,     0,     0,   134,  1600,   135,   525,     0,     0,     0,
       0,  1601,     0,   141,     0,   142,   143,   144,   145,   526,
     147,   148,   149,   150,   151,   152,     0,     0,  1165,   154,
       0,     0,   424,   157,   158,   159,   160,   161,   162,   163,
     164,   527,   166,   528,   168,   529,   530,   171,   172,   173,
     174,   175,   176,   531,   178,   532,   533,   534,   182,   183,
     535,   536,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,   197,   198,     0,   199,   200,   201,     0,
     202,   203,   204,   205,   206,   207,   208,   209,   210,   211,
     212,   213,   214,   215,   216,   217,   218,   219,   220,   221,
     222,   223,   224,   225,   226,   227,   228,   229,   230,   231,
     232,   233,   234,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,   245,   246,   247,   248,     3,     0,     0,
       0,   251,   537,   253,  1166,  1167,  1168,  1169,    11,    12,
      13,    14,     0,     0,     0,     0,   421,   422,     0,    18,
      19,    20,   497,    22,    23,    24,   498,    26,   499,   500,
    1162,   502,   423,    32,   503,    34,    35,    36,     0,    37,
      38,    39,    40,   504,    42,    43,    44,    45,    46,    47,
      48,   505,     0,    50,   506,    52,   507,   508,    55,   509,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,   510,   511,    73,     0,    74,    75,
      76,   512,     0,     0,    79,    80,    81,     0,     0,   513,
      83,     0,     0,     0,     0,    85,   514,    87,   515,   516,
      90,    91,  1509,     0,     0,     0,     0,     0,     0,     0,
      96,    97,    98,    99,     0,   100,     0,     0,     0,   517,
     102,   103,   518,   519,     0,     0,     0,     0,     0,     0,
     106,   107,   108,     0,   110,   111,  1163,     0,     0,     0,
    1164,     0,     0,   118,   520,     0,     0,     0,   120,   121,
     122,   123,   521,   125,   126,   522,     0,     0,     0,     0,
       0,     0,     0,   523,   524,   133,     0,     0,     0,   134,
       0,   135,   525,     0,     0,     0,     0,     0,     0,   141,
       0,   142,   143,   144,   145,   526,   147,   148,   149,   150,
     151,   152,     0,     0,  1165,   154,     0,     0,   424,   157,
     158,   159,   160,   161,   162,   163,   164,   527,   166,   528,
     168,   529,   530,   171,   172,   173,   174,   175,   176,   531,
     178,   532,   533,   534,   182,   183,   535,   536,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   197,
     198,     0,   199,   200,   201,     0,   202,   203,   204,   205,
     206,   207,   208,   209,   210,   211,   212,   213,   214,   215,
     216,   217,   218,   219,   220,   221,   222,   223,   224,   225,
     226,   227,   228,   229,   230,   231,   232,   233,   234,   235,
     236,   237,   238,   239,   240,   241,   242,   243,   244,   245,
     246,   247,   248,     3,     0,     0,     0,   251,   537,   253,
    1166,  1167,  1168,  1169,    11,    12,    13,    14,     0,     0,
       0,     0,   421,   422,     0,    18,    19,    20,   497,    22,
      23,    24,   498,    26,   499,   500,  1162,   502,   423,    32,
     503,    34,    35,    36,     0,    37,    38,    39,    40,   504,
      42,    43,    44,    45,    46,    47,    48,   505,     0,    50,
     506,    52,   507,   508,    55,   509,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
     510,   511,    73,     0,    74,    75,    76,   512,     0,     0,
      79,    80,    81,     0,     0,   513,    83,     0,     0,     0,
       0,    85,   514,    87,   515,   516,    90,    91,  1653,     0,
       0,     0,     0,     0,     0,     0,    96,    97,    98,    99,
       0,   100,     0,     0,     0,   517,   102,   103,   518,   519,
       0,     0,     0,     0,     0,     0,   106,   107,   108,     0,
     110,   111,  1163,     0,     0,     0,  1164,     0,     0,   118,
     520,     0,     0,     0,   120,   121,   122,   123,   521,   125,
     126,   522,     0,     0,     0,     0,     0,     0,     0,   523,
     524,   133,     0,     0,     0,   134,     0,   135,   525,     0,
       0,     0,     0,     0,     0,   141,     0,   142,   143,   144,
     145,   526,   147,   148,   149,   150,   151,   152,     0,     0,
    1165,   154,     0,     0,   424,   157,   158,   159,   160,   161,
     162,   163,   164,   527,   166,   528,   168,   529,   530,   171,
     172,   173,   174,   175,   176,   531,   178,   532,   533,   534,
     182,   183,   535,   536,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,   197,   198,     0,   199,   200,
     201,     0,   202,   203,   204,   205,   206,   207,   208,   209,
     210,   211,   212,   213,   214,   215,   216,   217,   218,   219,
     220,   221,   222,   223,   224,   225,   226,   227,   228,   229,
     230,   231,   232,   233,   234,   235,   236,   237,   238,   239,
     240,   241,   242,   243,   244,   245,   246,   247,   248,     3,
       0,     0,     0,   251,   537,   253,  1166,  1167,  1168,  1169,
      11,    12,    13,    14,     0,     0,     0,     0,   421,   422,
       0,    18,    19,    20,   497,    22,    23,    24,   498,    26,
     499,   500,  1162,   502,   423,    32,   503,    34,    35,    36,
       0,    37,    38,    39,    40,   504,    42,    43,    44,    45,
      46,    47,    48,   505,     0,    50,   506,    52,   507,   508,
      55,   509,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,   510,   511,    73,     0,
      74,    75,    76,   512,     0,     0,    79,    80,    81,     0,
       0,   513,    83,     0,     0,     0,     0,    85,   514,    87,
     515,   516,    90,    91,  1657,     0,     0,     0,     0,     0,
       0,     0,    96,    97,    98,    99,     0,   100,     0,     0,
       0,   517,   102,   103,   518,   519,     0,     0,     0,     0,
       0,     0,   106,   107,   108,     0,   110,   111,  1163,     0,
       0,     0,  1164,     0,     0,   118,   520,     0,     0,     0,
     120,   121,   122,   123,   521,   125,   126,   522,     0,     0,
       0,     0,     0,     0,     0,   523,   524,   133,     0,     0,
       0,   134,     0,   135,   525,     0,     0,     0,     0,     0,
       0,   141,     0,   142,   143,   144,   145,   526,   147,   148,
     149,   150,   151,   152,     0,     0,  1165,   154,     0,     0,
     424,   157,   158,   159,   160,   161,   162,   163,   164,   527,
     166,   528,   168,   529,   530,   171,   172,   173,   174,   175,
     176,   531,   178,   532,   533,   534,   182,   183,   535,   536,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   197,   198,     0,   199,   200,   201,     0,   202,   203,
     204,   205,   206,   207,   208,   209,   210,   211,   212,   213,
     214,   215,   216,   217,   218,   219,   220,   221,   222,   223,
     224,   225,   226,   227,   228,   229,   230,   231,   232,   233,
     234,   235,   236,   237,   238,   239,   240,   241,   242,   243,
     244,   245,   246,   247,   248,     3,     0,     0,     0,   251,
     537,   253,  1166,  1167,  1168,  1169,    11,    12,    13,    14,
       0,     0,     0,     0,   421,   422,     0,    18,    19,    20,
     497,    22,    23,    24,   498,    26,   499,   500,  1162,   502,
     423,    32,   503,    34,    35,    36,     0,    37,    38,    39,
      40,   504,    42,    43,    44,    45,    46,    47,    48,   505,
       0,    50,   506,    52,   507,   508,    55,   509,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,   510,   511,    73,     0,    74,    75,    76,   512,
       0,     0,    79,    80,    81,     0,     0,   513,    83,     0,
       0,     0,     0,    85,   514,    87,   515,   516,    90,    91,
       0,     0,     0,     0,     0,     0,     0,     0,    96,    97,
      98,    99,     0,   100,     0,     0,     0,   517,   102,   103,
     518,   519,     0,     0,     0,     0,     0,     0,   106,   107,
     108,     0,   110,   111,  1163,     0,     0,     0,  1164,     0,
       0,   118,   520,     0,     0,     0,   120,   121,   122,   123,
     521,   125,   126,   522,     0,     0,     0,     0,     0,     0,
       0,   523,   524,   133,     0,     0,     0,   134,     0,   135,
     525,     0,     0,     0,     0,     0,     0,   141,     0,   142,
     143,   144,   145,   526,   147,   148,   149,   150,   151,   152,
       0,     0,  1165,   154,     0,     0,   424,   157,   158,   159,
     160,   161,   162,   163,   164,   527,   166,   528,   168,   529,
     530,   171,   172,   173,   174,   175,   176,   531,   178,   532,
     533,   534,   182,   183,   535,   536,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,   197,   198,     0,
     199,   200,   201,     0,   202,   203,   204,   205,   206,   207,
     208,   209,   210,   211,   212,   213,   214,   215,   216,   217,
     218,   219,   220,   221,   222,   223,   224,   225,   226,   227,
     228,   229,   230,   231,   232,   233,   234,   235,   236,   237,
     238,   239,   240,   241,   242,   243,   244,   245,   246,   247,
     248,     3,     0,     0,     0,   251,   537,   253,  1166,  1167,
    1168,  1169,    11,    12,    13,    14,     0,     0,     0,     0,
     421,   422,     0,    18,    19,    20,   497,    22,    23,    24,
     498,    26,   499,   500,  1162,   502,   423,    32,   503,    34,
      35,    36,     0,    37,    38,    39,    40,   504,    42,    43,
      44,    45,    46,    47,    48,   505,     0,    50,   506,    52,
     507,   508,    55,   509,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,   510,   511,
      73,     0,    74,    75,    76,   512,     0,     0,    79,    80,
      81,     0,     0,   513,    83,     0,     0,     0,     0,    85,
     514,    87,   515,   516,    90,    91,     0,     0,     0,     0,
       0,     0,     0,     0,    96,    97,    98,    99,     0,   100,
       0,     0,     0,   517,   102,   103,   518,   519,     0,     0,
       0,     0,     0,     0,   106,   107,   108,     0,   110,   111,
    1163,     0,     0,     0,  1164,     0,     0,   118,   520,     0,
       0,     0,   120,   121,   122,   123,   521,   125,   126,   522,
       0,     0,     0,     0,     0,     0,     0,   523,   524,   133,
       0,     0,     0,   134,     0,   135,   525,     0,     0,     0,
       0,     0,     0,   141,     0,   142,   143,   144,   145,   526,
     147,   148,   149,   150,   151,   152,     0,     0,   153,   154,
       0,     0,   424,   157,   158,   159,   160,   161,   162,   163,
     164,   527,   166,   528,   168,   529,   530,   171,   172,   173,
     174,   175,   176,   531,   178,   532,   533,   534,   182,   183,
     535,   536,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,   197,   198,     0,   199,   200,   201,     0,
     202,   203,   204,   205,   206,   207,   208,   209,   210,   211,
     212,   213,   214,   215,   216,   217,   218,   219,   220,   221,
     222,   223,   224,   225,   226,   227,   228,   229,   230,   231,
     232,   233,   234,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,   245,   246,   247,   248,     3,     0,   582,
     583,   251,   537,   253,  1166,  1167,  1168,  1169,    11,    12,
       0,     0,     0,     0,     0,     0,   421,   422,     0,    18,
      19,    20,   545,    22,    23,    24,   498,   546,   547,   500,
     501,   502,   423,    32,   503,    34,   548,    36,     0,    37,
      38,    39,    40,   549,    42,   550,   551,    45,    46,    47,
      48,   505,     0,    50,   552,    52,   507,   508,    55,   509,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,   510,   511,    73,     0,    74,    75,
      76,   553,     0,     0,    79,    80,    81,     0,     0,   513,
      83,     0,     0,     0,     0,    85,   514,    87,   515,   516,
      90,    91,     0,     0,     0,     0,     0,     0,     0,     0,
      96,    97,    98,    99,     0,   100,     0,     0,     0,   517,
     102,   103,   518,   519,     0,     0,     0,     0,     0,     0,
     106,   107,   108,     0,   110,   111,   112,     0,     0,     0,
       0,     0,     0,   118,   520,     0,     0,     0,   120,   121,
     122,   123,   521,   125,   126,   522,     0,     0,     0,     0,
       0,     0,     0,   523,   524,   133,     0,     0,     0,   134,
       0,   135,   525,     0,     0,     0,     0,     0,     0,   141,
       0,   142,   143,   144,   145,   526,   147,   148,   149,   150,
     151,   152,     0,     0,   153,   154,     0,     0,   424,   157,
     158,   159,   160,   161,   162,   163,   164,   527,   554,   528,
     168,   529,   530,   171,   172,   173,   174,   175,   176,   531,
     178,   532,   533,   534,   182,   183,   535,   536,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   197,
     198,     0,   199,   200,   201,     0,   202,   203,   204,   205,
     206,   207,   208,   209,   210,   211,   212,   213,   214,   215,
     216,   217,   218,   219,   220,   221,   222,   223,   224,   225,
     226,   227,   228,   229,   230,   231,   232,   233,   234,   235,
     236,   237,   238,   239,   240,   241,   242,   243,   244,   245,
     246,   247,   248,     3,     0,     0,     0,   251,   537,   253,
     254,   255,   256,   257,    11,    12,     0,     0,     0,     0,
       0,     0,   421,   422,     0,    18,    19,    20,   545,    22,
      23,    24,   498,   546,   547,   500,   501,   502,   423,    32,
     503,    34,   548,    36,     0,    37,    38,    39,    40,   549,
      42,   550,   551,    45,    46,    47,    48,   505,     0,    50,
     552,    52,   507,   508,    55,   509,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
     510,   511,    73,     0,    74,    75,    76,   553,     0,     0,
      79,    80,    81,     0,     0,   513,    83,     0,     0,     0,
       0,    85,   514,    87,   515,   516,    90,    91,     0,     0,
       0,     0,     0,     0,     0,     0,    96,    97,    98,    99,
       0,   100,     0,     0,     0,   517,   102,   103,   518,   519,
       0,     0,     0,     0,     0,     0,   106,   107,   108,     0,
     110,   111,   112,     0,     0,     0,     0,     0,     0,   118,
     520,     0,     0,     0,   120,   121,   122,   123,   521,   125,
     126,   522,     0,     0,     0,     0,     0,     0,     0,   523,
     524,   133,     0,     0,     0,   134,   773,   135,   525,     0,
       0,     0,     0,   774,     0,   141,     0,   142,   143,   144,
     145,   526,   147,   148,   149,   150,   151,   152,     0,     0,
     153,   154,     0,     0,   424,   157,   158,   159,   160,   161,
     162,   163,   164,   527,   554,   528,   168,   529,   530,   171,
     172,   173,   174,   175,   176,   531,   178,   532,   533,   534,
     182,   183,   535,   536,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,   197,   198,     0,   199,   200,
     201,     0,   202,   203,   204,   205,   206,   207,   208,   209,
     210,   211,   212,   213,   214,   215,   216,   217,   218,   219,
     220,   221,   222,   223,   224,   225,   226,   227,   228,   229,
     230,   231,   232,   233,   234,   235,   236,   237,   238,   239,
     240,   241,   242,   243,   244,   245,   246,   247,   248,     3,
       0,     0,     0,   251,   537,   253,   254,   255,   256,   257,
      11,    12,     0,     0,     0,   799,     0,     0,   421,   422,
       0,    18,    19,    20,   545,    22,    23,    24,   498,   546,
     547,   500,   501,   502,   423,    32,   503,    34,   548,    36,
       0,    37,    38,    39,    40,   549,    42,   550,   551,    45,
      46,    47,    48,   505,     0,    50,   552,    52,   507,   508,
      55,   509,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,   510,   511,    73,     0,
      74,    75,    76,   553,     0,     0,    79,    80,    81,     0,
       0,   513,    83,     0,     0,     0,     0,    85,   514,    87,
     515,   516,    90,    91,     0,     0,     0,     0,     0,     0,
       0,     0,    96,    97,    98,    99,     0,   100,     0,     0,
       0,   517,   102,   103,   518,   519,     0,     0,     0,     0,
       0,     0,   106,   107,   108,     0,   110,   111,   112,     0,
       0,     0,     0,     0,     0,   118,   520,     0,     0,     0,
     120,   121,   122,   123,   521,   125,   126,   522,     0,     0,
       0,     0,     0,     0,     0,   523,   524,   133,     0,     0,
       0,   134,   800,   135,   525,     0,     0,     0,     0,     0,
       0,   141,     0,   142,   143,   144,   145,   526,   147,   148,
     149,   150,   151,   152,     0,     0,   153,   154,     0,     0,
     424,   157,   158,   159,   160,   161,   162,   163,   164,   527,
     554,   528,   168,   529,   530,   171,   172,   173,   174,   175,
     176,   531,   178,   532,   533,   534,   182,   183,   535,   536,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   197,   198,     0,   199,   200,   201,     0,   202,   203,
     204,   205,   206,   207,   208,   209,   210,   211,   212,   213,
     214,   215,   216,   217,   218,   219,   220,   221,   222,   223,
     224,   225,   226,   227,   228,   229,   230,   231,   232,   233,
     234,   235,   236,   237,   238,   239,   240,   241,   242,   243,
     244,   245,   246,   247,   248,     3,     0,     0,     0,   251,
     537,   253,   254,   255,   256,   257,    11,    12,     0,     0,
       0,     0,     0,     0,   421,   422,     0,    18,    19,    20,
     545,    22,    23,    24,   498,   546,   547,   500,   501,   502,
     423,    32,   503,    34,   548,    36,     0,    37,    38,    39,
      40,   549,    42,   550,   551,    45,    46,    47,    48,   505,
       0,    50,   552,    52,   507,   508,    55,   509,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,   510,   511,    73,     0,    74,    75,    76,   553,
       0,     0,    79,    80,    81,     0,     0,   513,    83,     0,
       0,     0,     0,    85,   514,    87,   515,   516,    90,    91,
       0,     0,     0,     0,     0,     0,     0,     0,    96,    97,
      98,    99,     0,   100,     0,     0,     0,   517,   102,   103,
     518,   519,     0,     0,     0,     0,     0,     0,   106,   107,
     108,     0,   110,   111,   112,     0,     0,     0,     0,     0,
       0,   118,   520,     0,     0,     0,   120,   121,   122,   123,
     521,   125,   126,   522,     0,     0,     0,     0,     0,     0,
       0,   523,   524,   133,     0,     0,     0,   134,   817,   135,
     525,     0,     0,     0,     0,   818,     0,   141,     0,   142,
     143,   144,   145,   526,   147,   148,   149,   150,   151,   152,
       0,     0,   153,   154,     0,     0,   424,   157,   158,   159,
     160,   161,   162,   163,   164,   527,   554,   528,   168,   529,
     530,   171,   172,   173,   174,   175,   176,   531,   178,   532,
     533,   534,   182,   183,   535,   536,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,   197,   198,     0,
     199,   200,   201,     0,   202,   203,   204,   205,   206,   207,
     208,   209,   210,   211,   212,   213,   214,   215,   216,   217,
     218,   219,   220,   221,   222,   223,   224,   225,   226,   227,
     228,   229,   230,   231,   232,   233,   234,   235,   236,   237,
     238,   239,   240,   241,   242,   243,   244,   245,   246,   247,
     248,     3,     0,     0,     0,   251,   537,   253,   254,   255,
     256,   257,    11,    12,     0,     0,     0,     0,     0,     0,
     421,   422,     0,    18,    19,    20,   545,    22,    23,    24,
     498,   546,   547,   500,   501,   502,   423,    32,   503,    34,
     548,    36,     0,    37,    38,    39,    40,   549,    42,   550,
     551,    45,    46,    47,    48,   505,     0,    50,   552,    52,
     507,   508,    55,   509,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,   510,   511,
      73,     0,    74,    75,    76,   553,     0,     0,    79,    80,
      81,     0,     0,   513,    83,     0,     0,     0,     0,    85,
     514,    87,   515,   516,    90,    91,  1255,     0,     0,     0,
       0,     0,     0,     0,    96,    97,    98,    99,     0,   100,
       0,     0,     0,   517,   102,   103,   518,   519,     0,     0,
       0,     0,     0,     0,   106,   107,   108,     0,   110,   111,
     112,     0,     0,     0,     0,     0,     0,   118,   520,     0,
       0,     0,   120,   121,   122,   123,   521,   125,   126,   522,
       0,     0,     0,     0,     0,     0,     0,   523,   524,   133,
       0,     0,     0,   134,     0,   135,   525,     0,     0,     0,
       0,     0,     0,   141,     0,   142,   143,   144,   145,   526,
     147,   148,   149,   150,   151,   152,     0,     0,   153,   154,
       0,     0,   424,   157,   158,   159,   160,   161,   162,   163,
     164,   527,   554,   528,   168,   529,   530,   171,   172,   173,
     174,   175,   176,   531,   178,   532,   533,   534,   182,   183,
     535,   536,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,   197,   198,     0,   199,   200,   201,     0,
     202,   203,   204,   205,   206,   207,   208,   209,   210,   211,
     212,   213,   214,   215,   216,   217,   218,   219,   220,   221,
     222,   223,   224,   225,   226,   227,   228,   229,   230,   231,
     232,   233,   234,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,   245,   246,   247,   248,     3,     0,     0,
       0,   251,   537,   253,   254,   255,   256,   257,    11,    12,
       0,     0,     0,     0,     0,     0,   421,   422,     0,    18,
      19,    20,   545,    22,    23,    24,   498,   546,   547,   500,
     501,   502,   423,    32,   503,    34,   548,    36,     0,    37,
      38,    39,    40,   549,    42,   550,   551,    45,    46,    47,
      48,   505,     0,    50,   552,    52,   507,   508,    55,   509,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,   510,   511,    73,     0,    74,    75,
      76,   553,     0,     0,    79,    80,    81,     0,     0,   513,
      83,     0,     0,     0,     0,    85,   514,    87,   515,   516,
      90,    91,     0,     0,     0,     0,     0,     0,     0,     0,
      96,    97,    98,    99,     0,   100,     0,     0,     0,   517,
     102,   103,   518,   519,     0,     0,     0,     0,     0,     0,
     106,   107,   108,     0,   110,   111,   112,     0,     0,     0,
       0,     0,     0,   118,   520,     0,     0,     0,   120,   121,
     122,   123,   521,   125,   126,   522,     0,     0,     0,     0,
       0,     0,     0,   523,   524,   133,     0,     0,     0,   134,
       0,   135,   525,     0,     0,     0,     0,     0,     0,   141,
       0,   142,   143,   144,   145,   526,   147,   148,   149,   150,
     151,   152,     0,     0,   153,   154,     0,     0,   424,   157,
     158,   159,   160,   161,   162,   163,   164,   527,   554,   528,
     168,   529,   530,   171,   172,   173,   174,   175,   176,   531,
     178,   532,   533,   534,   182,   183,   535,   536,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   197,
     198,     0,   199,   200,   201,     0,   202,   203,   204,   205,
     206,   207,   208,   209,   210,   211,   212,   213,   214,   215,
     216,   217,   218,   219,   220,   221,   222,   223,   224,   225,
     226,   227,   228,   229,   230,   231,   232,   233,   234,   235,
     236,   237,   238,   239,   240,   241,   242,   243,   244,   245,
     246,   247,   248,     0,     0,     0,     0,   251,   537,   253,
     254,   255,   256,   257,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,     0,    15,     0,     0,   421,   422,
       0,    18,    19,    20,   850,    22,    23,    24,    25,     0,
     851,   500,    29,     0,   423,    32,   503,    34,     0,    36,
       0,    37,    38,    39,    40,   852,    42,     0,     0,    45,
      46,    47,    48,   505,     0,    50,   853,    52,     0,     0,
      55,   509,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,   510,   511,    73,     0,
      74,    75,    76,   854,     0,     0,    79,    80,    81,     0,
       0,   513,    83,     0,     0,     0,    84,    85,   514,    87,
     515,   516,    90,    91,   567,    93,     0,     0,     0,     0,
      95,     0,    96,    97,    98,    99,     0,   100,     0,     0,
       0,   517,   102,   103,   518,   519,     0,     0,     0,     0,
       0,     0,   106,   107,   108,   109,   110,   111,     0,   568,
     114,     0,   115,   116,     0,   118,   119,     0,     0,     0,
     120,   121,   122,   123,   124,   125,   126,   522,     0,   128,
       0,     0,     0,     0,     0,   523,   524,   133,     0,     0,
       0,   134,     0,   135,   525,     0,     0,     0,     0,     0,
       0,   141,     0,   142,   143,   144,   145,   146,   147,   148,
     149,   150,   151,   152,     0,     0,     0,   154,   155,     0,
     424,   157,   158,   159,   160,   161,   162,   163,   164,   527,
       0,   528,   168,   529,   530,   171,   172,   173,   174,   175,
     176,   531,   178,   532,   533,   534,   182,   183,     0,   536,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   197,   198,     0,   199,   200,   201,     0,   202,   203,
     204,   205,   206,   207,   208,   209,   210,   211,   212,   213,
     214,   215,   216,   217,   218,   219,   220,   221,   222,   223,
     224,   225,   226,   227,   228,   229,   230,   231,   232,   233,
     234,   235,   236,   237,   238,   239,   240,   241,   242,   243,
     244,   245,   246,   247,   248,   249,     0,   250,    11,    12,
     537,   253,     0,     0,     0,     0,   421,   422,     0,    18,
      19,    20,     0,    22,    23,    24,   498,     0,     0,   500,
     501,     0,   423,    32,   503,    34,     0,    36,     0,    37,
      38,    39,    40,     0,    42,     0,     0,    45,    46,    47,
      48,   505,     0,    50,     0,    52,     0,     0,    55,   509,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,   510,   511,    73,     0,    74,    75,
      76,     0,     0,     0,    79,    80,    81,     0,     0,   513,
      83,     0,     0,     0,     0,    85,   514,    87,   515,   516,
      90,    91,     0,     0,     0,     0,     0,     0,     0,     0,
      96,    97,    98,    99,     0,   100,     0,     0,     0,   517,
     102,   103,   518,   519,     0,     0,     0,     0,     0,     0,
     106,   107,   108,     0,   110,   111,     0,     0,     0,     0,
       0,     0,     0,   118,   520,     0,     0,     0,   120,   121,
     122,   123,   521,   125,   126,   522,     0,     0,     0,     0,
       0,     0,     0,   523,   524,   133,     0,     0,     0,   134,
       0,   135,   525,     0,     0,     0,     0,     0,     0,   141,
       0,   142,   143,   144,   145,   526,   147,   148,   149,   150,
     151,   152,     0,     0,     0,   154,     0,     0,   424,   157,
     158,   159,   160,   161,   162,   163,   164,   527,     0,   528,
     168,   529,   530,   171,   172,   173,   174,   175,   176,   531,
     178,   532,   533,   534,   182,   183,     0,   536,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   197,
     198,     0,   199,   200,   201,     0,   202,   203,   204,   205,
     206,   207,   208,   209,   210,   211,   212,   213,   214,   215,
     216,   217,   218,   219,   220,   221,   222,   223,   224,   225,
     226,   227,   228,   229,   230,   231,   232,   233,   234,   235,
     236,   237,   238,   239,   240,   241,   242,   243,   244,   245,
     246,   247,   248,     0,     0,     0,     0,     0,   537,   253
  };

  const short
  xquery_parser::yycheck_[] =
  {
       0,   297,   185,   288,   318,     5,     6,     7,     8,     9,
      10,   701,   685,  1066,   837,   927,    16,     0,   686,   687,
     688,   689,     5,     6,     7,     8,     9,    10,   348,   318,
     139,   265,   436,  1313,     0,  1050,   874,  1296,   778,     5,
       6,     7,     8,     9,    10,   114,   115,   266,     5,     6,
       7,     8,     9,    10,   630,  1090,     5,     6,     7,     8,
       9,    10,   281,   628,   793,   434,   435,   471,   472,     5,
       6,     7,     8,     9,     1,   940,   941,   806,     1,   595,
     899,   778,   628,   629,   600,   258,   259,    78,   626,   627,
      38,   820,  1377,  1378,  1148,  1031,  1013,    38,  1326,   626,
     627,   628,   629,    41,  1512,  1424,    32,  1515,    46,    47,
       1,   927,     1,   113,     1,     1,     1,    55,    29,    30,
       1,     1,   110,    58,    62,     1,    64,    65,   103,     1,
     113,   427,     1,    33,   124,    73,   121,   433,    49,     1,
     227,    29,    30,   227,   963,   229,   227,   113,    59,    97,
     110,    99,   121,   145,  1622,   227,   113,   229,    99,   735,
       1,    49,   100,     1,   113,     1,    77,    99,  1033,     1,
     455,    82,     1,    97,   121,  1040,    97,    32,     1,    54,
     249,   250,   171,   110,   979,     1,   188,   110,   159,    77,
       8,   156,   664,   103,   666,   667,    35,   669,   159,   671,
       1,   189,   150,   114,   115,   116,   116,   679,   680,   150,
     212,   110,    51,   188,   179,   204,     0,     1,   150,   140,
     189,     1,   940,   941,   145,   150,   150,   115,   297,   316,
     156,     1,   316,   159,   110,   316,  1148,  1149,   110,    99,
    1293,   110,   351,     1,   316,   235,    39,    40,   110,    42,
      43,     0,     1,   179,     1,    47,   985,   132,   258,   259,
     574,   979,   262,  1086,  1279,   561,  1525,    60,    79,   110,
    1555,     1,   110,    65,   110,   258,   259,   237,   110,   262,
     227,   110,  1317,   275,   219,   574,   571,   110,  1200,   227,
     275,  1326,   258,   259,   110,  1013,   262,   297,  1038,    99,
    1528,   192,  1621,   192,  1772,   192,   192,   192,   163,   110,
    1039,   192,   192,   189,   297,  1033,   216,   189,   883,  1255,
     189,   256,  1040,   116,     9,    10,   110,   275,  1736,  1246,
     110,   297,  1148,  1149,   275,   261,   235,   883,   884,  1036,
     110,  1038,   746,   881,   882,   156,  1400,   150,   189,     1,
     275,   189,   110,   189,   650,  1052,     1,   189,   427,   177,
     189,   110,   309,   110,   433,   356,   189,     1,   359,  1164,
     163,   171,     1,   189,     1,  1783,   110,  1785,   289,  1198,
     110,   319,   320,   321,   110,   186,   297,   791,   315,   965,
     313,    97,   626,   627,   305,   464,    96,   189,   258,   259,
     469,   289,   186,   110,   623,   624,  1271,   187,   110,   297,
     479,  1140,   223,   224,   133,   634,    97,   486,   487,   189,
     110,   106,     1,    32,     1,    97,   495,   427,   186,   222,
     249,   250,   161,   433,   139,     5,  1721,   186,   133,     9,
     187,   126,   615,   128,   427,   110,  1164,   110,   153,   178,
     433,   923,   186,   272,   254,   255,   186,   257,   110,  1683,
     460,   427,   462,   110,   264,   110,   192,   433,    99,   150,
     263,   156,   275,     9,   274,   160,   110,   460,  1758,   462,
     199,   110,   189,   110,   484,   185,   286,   189,  1400,   489,
    1775,   176,   561,  1528,   460,    31,   462,  1409,    99,   189,
      57,   484,    59,   460,   199,   462,   489,   116,  1420,   578,
     579,   460,   197,   462,   220,   221,  1245,  1257,   484,   150,
     589,  1359,   163,   489,   189,  1363,   189,   484,  1246,   235,
     215,   110,   489,   110,   186,   484,   110,  1761,   607,   608,
     489,   186,    32,   190,  1803,     1,   103,     3,   220,   221,
     139,  1241,   186,  1271,   163,   110,   126,   186,   128,   186,
    1125,   561,  1127,   235,    97,   134,   136,  1432,   568,  1266,
     171,    47,   156,    49,     9,    10,   872,  1295,   561,  1125,
    1126,  1127,   986,   987,  1400,   568,   156,   991,    99,   958,
     126,   110,   128,  1409,  1110,   561,   705,   706,  1125,  1126,
    1127,   137,   568,   110,  1420,   901,     5,   186,   608,   186,
       9,   568,   145,    69,   203,   615,   190,  1021,    74,   568,
     156,   621,   161,   192,  1639,   608,   626,   627,   628,   629,
     148,   110,   615,   109,   189,    32,   110,   355,   707,   178,
     358,   159,   608,   626,   627,   628,   629,   881,   159,   615,
     650,   608,  1381,   254,   255,    44,   257,   120,    97,    48,
     626,   627,   628,   629,    53,   121,   156,   650,   887,   159,
     189,   106,   891,   274,   157,   121,   159,     3,   156,  1591,
     121,   184,   189,    72,   650,   286,   149,    29,    30,   179,
       9,   126,   103,   128,   170,   198,   253,    39,    40,   162,
      42,    43,   702,  1526,  1757,   116,   145,    49,  1531,   266,
     189,   268,    31,   156,  1432,   189,   113,    59,    60,   702,
     121,   156,   279,   280,   281,   160,   283,   126,   285,   128,
     127,    97,   271,   196,   212,    77,   702,   136,  1411,   278,
      82,   176,   188,    69,  1484,   702,    56,   188,    74,   288,
      47,    61,    49,   702,     1,   152,   239,   156,    97,   828,
     829,   830,   197,   832,     9,    10,   702,    29,    30,   212,
      80,   219,   114,   115,   116,  1591,     1,     9,    10,  1469,
     215,   170,   156,   783,   173,    82,  1651,    49,    35,    99,
    1027,   157,  1029,   159,  1512,   121,    99,  1515,   159,   188,
     783,  1366,   109,   872,    51,   179,   145,   126,   256,   128,
      35,   121,   109,   116,   814,   815,    99,   783,   137,  1115,
    1366,   163,   103,   130,   159,   214,   783,  1748,  1749,   147,
    1150,   814,   815,   116,   783,   116,  1073,   156,  1075,  1704,
    1663,  1706,    99,   265,  1667,    56,   161,   783,   814,   815,
      61,    29,    30,   115,   276,    27,   159,   814,   815,   116,
     178,   106,   161,   178,    36,   814,   815,    29,    30,  1790,
    1274,    49,   872,   170,   106,  1796,   159,   103,   196,   178,
     222,   126,   125,   128,   168,  1170,   201,    49,    99,   872,
     116,    63,    35,   893,   126,   159,   128,  1116,   151,   103,
     184,   901,   201,  1122,  1769,   184,   872,  1575,    51,  1582,
    1583,   156,   116,   156,   167,   160,   159,   203,   901,   198,
     157,   263,   159,   151,   156,   271,   212,    98,   160,  1752,
     140,   176,   278,  1651,  1624,   901,  1119,   140,   109,   167,
     940,   941,   288,   103,   176,    97,   156,   289,   119,   976,
     159,   265,   197,   156,   159,   297,   116,   109,   301,   130,
     159,   988,   276,   305,   270,   197,   159,   310,   227,  1373,
     215,   277,   159,   232,   279,   280,   281,  1273,  1382,   979,
     285,   708,   121,   215,   711,   140,  1704,   714,  1706,   140,
     145,    56,  1721,   631,   145,   156,    61,   635,   159,   223,
     224,   225,   135,  1292,   125,   234,   156,   236,   125,   159,
     143,   144,   156,   146,   156,   159,   125,   159,  1736,  1692,
    1693,   154,   234,   125,   236,   158,   125,   160,   193,   194,
     131,   132,   165,  1033,   131,   132,  1726,   683,   684,   121,
    1040,    78,   690,   691,  1358,  1239,  1240,   125,   181,   692,
     693,  1769,  1612,  1613,   125,    92,   156,   125,   156,   125,
     156,  1350,   125,   125,   218,  1783,   159,  1785,  1068,  1358,
     121,   244,   205,   206,   207,   208,   209,   210,  1147,   116,
     243,   156,   159,  1773,   243,  1068,   316,     0,   192,  1493,
     169,   128,  1765,   130,    95,   251,   102,    52,   105,   202,
     104,   195,  1068,   157,   159,   186,   186,    32,    49,   155,
     186,  1068,   186,   186,   186,  1115,   121,   248,   306,  1068,
     189,   156,     1,   121,  1124,  1125,  1126,  1127,   145,   110,
     189,   189,  1115,   156,   156,   159,   145,   186,   159,   159,
     159,   159,  1125,  1126,  1127,   124,   159,   159,   159,  1115,
      37,    17,   216,   156,    32,  1845,   192,  1847,   192,  1125,
    1126,  1127,   247,   233,  1164,     3,   286,  1857,   121,   121,
     170,   286,  1355,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,   172,   172,    73,   233,   189,    97,    97,
      97,   152,   110,   110,   163,    32,   163,   130,   135,    97,
     248,   145,   121,   135,   186,   189,   301,   110,   186,  1505,
      99,    97,  1501,   189,  1503,    47,    99,  1506,  1507,   189,
     156,  1510,   156,   189,   189,   189,   186,  1516,   186,   156,
     156,   239,   156,   156,    97,   192,   235,   172,   186,   145,
     135,   145,    97,    32,   288,    97,   229,   121,   301,  1249,
     130,   156,  1548,   150,   121,    97,   186,   200,   135,   135,
      99,   163,   156,   121,   186,   150,  1249,   103,   103,    37,
     186,  1271,   156,  1273,   145,   124,    32,   192,   121,   247,
      82,   156,   110,  1249,  1284,   109,   260,   110,   147,   262,
    1273,   287,  1249,   256,   269,  1295,    32,   159,   159,   159,
    1249,  1284,   159,  1303,    32,   159,   159,  1273,   159,    32,
     192,   288,   130,  1249,   308,   229,   189,   289,  1284,   356,
    1303,   189,   359,  1323,   189,   147,   110,  1284,   145,   150,
    1330,   150,   103,   189,   135,  1284,   186,  1303,     7,   235,
    1323,  1637,   189,   189,   192,   235,  1303,  1330,  1284,    99,
     159,   159,  1648,  1649,  1303,   200,  1652,  1323,   239,   188,
     159,   156,  1362,  1659,  1330,  1654,  1323,  1303,   109,  1658,
     150,   235,   121,  1330,  1323,   121,    80,   189,   150,  1362,
     252,  1330,   252,   284,    81,   189,   189,    32,   288,   189,
      32,   235,   189,   189,   110,   189,  1362,   189,   172,   186,
     129,    99,   150,   440,   441,  1362,   150,   444,   129,   186,
     186,   448,   145,   186,   451,   452,   186,   186,   259,   318,
      97,   156,   459,   189,    32,    99,   189,   172,   465,   466,
     150,    97,  1432,   273,   199,    56,   473,    97,    32,   110,
     267,   121,   110,   480,   481,   482,   483,    97,   282,  1745,
      97,   488,   227,  1742,   121,    96,   289,   199,   121,   496,
     199,   185,   200,   227,   121,   199,   121,   227,   121,   201,
     745,   262,   262,  1201,   289,   295,  1545,  1546,  1547,   265,
     980,   307,   307,  1041,   435,  1246,  1726,  1773,  1784,   875,
    1145,   307,   660,  1782,   308,   308,   455,  1786,   308,  1392,
     796,  1568,  1384,  1380,   541,  1505,  1802,  1132,  1285,  1377,
    1293,   556,  1512,  1298,  1290,  1515,   662,   943,   663,   556,
     717,  1817,  1505,  1317,   682,   837,  1796,  1790,  1029,  1818,
     567,  1170,  1821,  1170,   463,   463,   343,  1349,  1360,  1505,
    1591,  1420,  1359,  1839,  1409,  1149,  1413,  1588,  1548,  1223,
    1158,  1012,  1848,  1459,  1615,  1844,  1410,   598,   595,    -1,
      -1,    -1,    -1,   600,    -1,  1548,   603,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1548,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1680,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   660,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1702,    -1,    -1,    -1,  1637,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1648,  1649,
      -1,  1651,  1652,    -1,  1637,    -1,    -1,    -1,    -1,  1659,
      -1,    92,    -1,    -1,    -1,  1648,  1649,    -1,    -1,  1652,
      -1,  1637,    -1,    -1,    -1,    -1,  1659,    -1,    -1,    -1,
      -1,    -1,  1648,  1649,    -1,   116,  1652,    -1,    -1,    -1,
      -1,    -1,    -1,  1659,    -1,    -1,    -1,   128,    -1,   130,
      -1,    -1,   739,    -1,  1704,   742,  1706,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   756,
      -1,    -1,    -1,   760,   761,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1736,    -1,    -1,    -1,
      -1,    -1,   779,    -1,    -1,  1745,    -1,    -1,    -1,    -1,
     787,    -1,    -1,   790,    -1,    -1,    -1,   794,    -1,    -1,
      -1,    -1,  1745,    -1,    -1,    -1,    -1,    -1,    -1,  1769,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1745,
      -1,    -1,    -1,  1783,  1784,  1785,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   836,
      -1,  1784,  1802,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1817,  1784,  1802,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1817,    -1,  1802,   874,    -1,  1839,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1848,    -1,
      -1,  1817,    -1,    -1,    -1,    -1,  1839,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1848,    -1,    -1,   905,    -1,
      -1,    -1,    -1,  1839,    -1,    -1,    -1,    -1,   915,    -1,
      -1,    -1,  1848,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   940,   941,   942,   943,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   979,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   993,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1018,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1027,    -1,  1029,    -1,  1031,    -1,  1033,    -1,    -1,    -1,
      -1,    -1,    -1,  1040,    -1,    -1,    -1,    -1,    -1,    -1,
     441,    -1,    -1,   444,    -1,    -1,    -1,   448,    -1,    -1,
     451,   452,    -1,    -1,    -1,    -1,    -1,    -1,   459,    -1,
      -1,    -1,    -1,    -1,   465,   466,  1073,    -1,  1075,    -1,
      -1,    -1,   473,    -1,    -1,    -1,    -1,    -1,    -1,   480,
     481,   482,   483,    -1,    -1,    -1,    -1,   488,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   496,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1110,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1136,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     541,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   556,    -1,  1164,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   567,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   603,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   113,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1255,    -1,
      -1,    -1,  1259,    -1,    -1,    -1,    -1,    -1,    -1,   660,
      -1,    -1,    -1,    -1,  1271,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1285,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1295,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1324,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   742,  1349,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1359,    -1,    -1,   756,  1363,    -1,    -1,   760,
     761,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   779,    -1,
      -1,    -1,    -1,    -1,  1391,    -1,   787,    -1,    -1,   790,
      -1,    -1,    -1,   794,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1432,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   836,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1465,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1509,    -1,   905,  1512,    -1,    -1,  1515,    -1,
      -1,    -1,    -1,    -1,   915,    -1,    -1,    -1,    -1,    -1,
    1527,    -1,    -1,    -1,    -1,  1532,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   940,
     941,   942,   943,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1565,  1566,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   979,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   993,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1018,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1027,    -1,  1029,    -1,
    1031,    -1,  1033,    -1,    -1,    -1,    -1,    -1,    -1,  1040,
      -1,    -1,    -1,    -1,  1651,    -1,  1653,    -1,  1655,    -1,
    1657,    -1,    -1,    -1,    -1,    -1,    -1,  1664,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1073,    -1,  1075,    -1,   568,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1704,    -1,  1706,
      -1,   593,   594,    -1,   596,   597,   598,   599,    -1,   601,
      -1,    -1,    -1,    -1,   606,  1722,    -1,    -1,  1725,    -1,
      -1,    -1,   614,    -1,    -1,    -1,    -1,    -1,    -1,  1736,
      -1,    -1,    -1,    -1,    -1,  1136,    -1,    -1,    -1,    -1,
    1747,    -1,    -1,    -1,    -1,   637,   638,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   647,    -1,    -1,   650,    -1,
      -1,    -1,  1769,  1164,    -1,    -1,    -1,    -1,    -1,    -1,
    1777,    -1,    -1,    -1,    -1,    -1,  1783,    -1,  1785,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1806,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1815,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   713,    -1,    -1,    -1,   717,    -1,    -1,  1835,    -1,
      -1,    -1,    -1,  1840,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1852,    -1,    -1,  1855,    -1,
      -1,    -1,    -1,    -1,  1255,    -1,    -1,    -1,  1259,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   758,    -1,    -1,    -1,
    1271,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1285,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1295,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   795,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1324,    -1,    -1,    -1,    -1,    -1,   821,
      -1,    -1,    -1,   825,   826,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   834,    -1,    -1,    -1,    -1,    -1,  1349,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1391,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   901,
      -1,    -1,    -1,    -1,    -1,   907,   908,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1432,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1465,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   981,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1509,    -1,
      -1,  1512,    -1,    -1,  1515,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1016,    -1,  1527,    -1,    -1,    -1,
      -1,  1532,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1035,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1051,
      -1,    -1,    -1,    -1,  1565,  1566,    -1,    -1,    -1,    -1,
    1062,  1063,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1077,    -1,    -1,  1080,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1097,  1098,  1099,    -1,  1101,
    1102,  1103,  1104,  1105,  1106,  1107,  1108,  1109,    -1,    -1,
    1112,  1113,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1121,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1132,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1651,    -1,  1653,    -1,  1655,    -1,  1657,    -1,    -1,    -1,
      -1,    -1,    -1,  1664,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1188,    -1,    -1,    -1,
      -1,    -1,    -1,  1704,    -1,  1706,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1722,    -1,    -1,  1725,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1736,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1747,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1769,  1261,
      -1,  1263,    -1,  1265,    -1,    -1,  1777,    -1,    -1,    -1,
      -1,  1273,  1783,    -1,  1785,    -1,    -1,    -1,    -1,    -1,
    1282,    -1,    -1,    -1,    -1,    -1,  1288,  1289,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1806,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1815,  1307,    -1,    -1,  1310,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1835,    -1,    -1,    -1,    -1,  1840,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1852,    -1,    -1,  1855,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1380,    -1,
      -1,    -1,  1384,  1385,    -1,    -1,    -1,    -1,  1390,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1478,    -1,    -1,    -1,
    1482,    -1,    -1,    -1,  1486,    -1,  1488,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1496,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1504,  1505,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1524,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1536,  1537,  1538,  1539,    -1,  1541,
    1542,    -1,  1544,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1559,    -1,    -1,
      -1,    -1,  1564,    -1,    -1,    -1,    -1,    -1,  1570,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1630,    -1,
    1632,    -1,    -1,  1635,    -1,    -1,  1638,    -1,    -1,    -1,
      -1,  1643,  1644,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1652,    -1,    -1,    -1,  1656,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1673,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1684,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1729,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1741,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1784,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1813,    -1,    -1,    -1,  1817,    -1,    -1,    -1,    -1,
    1822,     0,     1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    11,    12,    -1,    -1,  1838,    16,    -1,    -1,
    1842,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    -1,    32,    -1,    -1,    35,    36,    -1,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    -1,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    -1,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    -1,    97,    98,
      99,   100,   101,    -1,   103,   104,   105,    -1,    -1,   108,
     109,    -1,    -1,    -1,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,    -1,    -1,    -1,   127,    -1,
     129,   130,   131,   132,    -1,   134,    -1,    -1,    -1,   138,
     139,   140,   141,   142,    -1,    -1,    -1,    -1,    -1,    -1,
     149,   150,   151,   152,   153,   154,   155,   156,   157,    -1,
     159,   160,   161,   162,   163,    -1,    -1,    -1,   167,   168,
     169,   170,   171,   172,   173,   174,    -1,   176,    -1,   178,
     179,    -1,    -1,   182,   183,   184,    -1,    -1,    -1,   188,
      -1,   190,   191,   192,   193,   194,    -1,   196,    -1,   198,
      -1,   200,   201,   202,   203,   204,   205,   206,   207,   208,
     209,   210,    -1,    -1,   213,   214,   215,    -1,   217,   218,
     219,   220,   221,   222,   223,   224,   225,   226,   227,   228,
     229,   230,   231,   232,   233,   234,   235,   236,   237,   238,
     239,   240,   241,   242,   243,   244,   245,   246,   247,   248,
     249,   250,   251,   252,   253,   254,   255,   256,   257,   258,
     259,    -1,   261,   262,   263,    -1,   265,   266,   267,   268,
     269,   270,   271,   272,   273,   274,   275,   276,   277,   278,
     279,   280,   281,   282,   283,   284,   285,   286,   287,   288,
     289,   290,   291,   292,   293,   294,   295,   296,   297,   298,
     299,   300,   301,   302,   303,   304,   305,   306,   307,   308,
     309,   310,   311,   312,    -1,   314,    -1,   316,   317,   318,
     319,   320,   321,   322,     0,     1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    11,    12,    -1,    -1,    -1,
      16,    -1,    -1,    -1,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    -1,    32,    -1,    -1,    35,
      36,    -1,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    -1,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    -1,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    95,
      -1,    97,    98,    99,   100,   101,    -1,   103,   104,   105,
      -1,    -1,   108,   109,    -1,    -1,    -1,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,    -1,    -1,
      -1,   127,    -1,   129,   130,   131,   132,    -1,   134,    -1,
      -1,    -1,   138,   139,   140,   141,   142,    -1,    -1,    -1,
      -1,    -1,    -1,   149,   150,   151,   152,   153,   154,   155,
     156,   157,    -1,   159,   160,   161,   162,   163,    -1,    -1,
      -1,   167,   168,   169,   170,   171,   172,   173,   174,    -1,
     176,    -1,   178,   179,    -1,    -1,   182,   183,   184,    -1,
      -1,    -1,   188,    -1,   190,   191,   192,   193,   194,    -1,
     196,    -1,   198,    -1,   200,   201,   202,   203,   204,   205,
     206,   207,   208,   209,   210,    -1,    -1,   213,   214,   215,
      -1,   217,   218,   219,   220,   221,   222,   223,   224,   225,
     226,   227,   228,   229,   230,   231,   232,   233,   234,   235,
     236,   237,   238,   239,   240,   241,   242,   243,   244,   245,
     246,   247,   248,   249,   250,   251,   252,   253,   254,   255,
     256,   257,   258,   259,    -1,   261,   262,   263,    -1,   265,
     266,   267,   268,   269,   270,   271,   272,   273,   274,   275,
     276,   277,   278,   279,   280,   281,   282,   283,   284,   285,
     286,   287,   288,   289,   290,   291,   292,   293,   294,   295,
     296,   297,   298,   299,   300,   301,   302,   303,   304,   305,
     306,   307,   308,   309,   310,   311,   312,     1,   314,    -1,
     316,   317,   318,   319,   320,   321,   322,    11,    12,    -1,
      -1,    -1,    16,    -1,    -1,    -1,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    -1,    32,    -1,
      -1,    35,    36,    -1,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    -1,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    -1,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    -1,    97,    98,    99,   100,   101,    -1,   103,
     104,   105,    -1,    -1,   108,   109,    -1,    -1,    -1,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
      -1,    -1,    -1,   127,    -1,   129,   130,   131,   132,    -1,
     134,    -1,    -1,    -1,   138,   139,   140,   141,   142,    -1,
      -1,    -1,    -1,    -1,    -1,   149,   150,   151,   152,   153,
     154,   155,   156,   157,    -1,   159,   160,   161,   162,   163,
      -1,    -1,    -1,   167,   168,   169,   170,   171,   172,   173,
     174,    -1,   176,    -1,   178,   179,    -1,    -1,   182,   183,
     184,    -1,    -1,    -1,   188,    -1,   190,   191,   192,   193,
     194,    -1,   196,    -1,   198,    -1,   200,   201,   202,   203,
     204,   205,   206,   207,   208,   209,   210,    -1,    -1,   213,
     214,   215,    -1,   217,   218,   219,   220,   221,   222,   223,
     224,   225,   226,   227,   228,   229,   230,   231,   232,   233,
     234,   235,   236,   237,   238,   239,   240,   241,   242,   243,
     244,   245,   246,   247,   248,   249,   250,   251,   252,   253,
     254,   255,   256,   257,   258,   259,    -1,   261,   262,   263,
      -1,   265,   266,   267,   268,   269,   270,   271,   272,   273,
     274,   275,   276,   277,   278,   279,   280,   281,   282,   283,
     284,   285,   286,   287,   288,   289,   290,   291,   292,   293,
     294,   295,   296,   297,   298,   299,   300,   301,   302,   303,
     304,   305,   306,   307,   308,   309,   310,   311,   312,     1,
     314,    -1,   316,   317,   318,   319,   320,   321,   322,    11,
      12,    -1,    -1,    -1,    16,    -1,    -1,    -1,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    -1,
      32,    -1,    -1,    35,    36,    -1,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    -1,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    -1,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    -1,    97,    98,    99,   100,   101,
      -1,   103,   104,   105,    -1,    -1,   108,   109,    -1,    -1,
      -1,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,    -1,    -1,    -1,   127,    -1,   129,   130,   131,
     132,    -1,   134,    -1,    -1,    -1,   138,   139,   140,   141,
     142,    -1,    -1,    -1,    -1,    -1,    -1,   149,   150,   151,
     152,   153,   154,   155,   156,   157,    -1,   159,   160,   161,
     162,   163,    -1,    -1,    -1,   167,   168,   169,   170,   171,
     172,   173,   174,    -1,   176,    -1,   178,   179,    -1,    -1,
     182,   183,   184,    -1,    -1,    -1,   188,    -1,   190,   191,
      -1,   193,   194,    -1,   196,    -1,   198,    -1,   200,   201,
     202,   203,   204,   205,   206,   207,   208,   209,   210,    -1,
      -1,   213,   214,   215,    -1,   217,   218,   219,   220,   221,
     222,   223,   224,   225,   226,   227,   228,   229,   230,   231,
     232,   233,   234,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,   245,   246,   247,   248,   249,   250,   251,
     252,   253,   254,   255,   256,   257,   258,   259,    -1,   261,
     262,   263,    -1,   265,   266,   267,   268,   269,   270,   271,
     272,   273,   274,   275,   276,   277,   278,   279,   280,   281,
     282,   283,   284,   285,   286,   287,   288,   289,   290,   291,
     292,   293,   294,   295,   296,   297,   298,   299,   300,   301,
     302,   303,   304,   305,   306,   307,   308,   309,   310,   311,
     312,    -1,   314,    -1,   316,   317,   318,   319,   320,   321,
     322,     0,     1,    -1,     3,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    44,    -1,    -1,    -1,    48,
      -1,    -1,    -1,    52,    53,    -1,    -1,    -1,    -1,    58,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    -1,    -1,    72,    -1,    74,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    95,    -1,    97,    98,
      99,    -1,    -1,   102,   103,   104,   105,    -1,    -1,    -1,
     109,   110,    -1,    -1,    -1,    -1,    -1,   116,    -1,    -1,
     119,   120,   121,    -1,    -1,   124,    -1,    -1,    -1,    -1,
     129,   130,    -1,    -1,    -1,    -1,   135,    -1,    -1,    -1,
     139,    -1,    -1,    -1,   143,   144,    -1,   146,    -1,   148,
     149,    -1,    -1,    -1,   153,   154,    -1,    -1,   157,   158,
     159,   160,   161,   162,    -1,    -1,   165,    -1,    -1,    -1,
     169,   170,    -1,    -1,   173,    -1,    -1,    -1,    -1,   178,
      -1,    -1,   181,    -1,    -1,    -1,    -1,   186,   187,   188,
     189,   190,    -1,   192,   193,   194,   195,   196,    -1,    -1,
      -1,    -1,   201,   202,   203,    -1,   205,   206,   207,   208,
     209,   210,    -1,   212,    -1,   214,    -1,    -1,    -1,    -1,
     219,   220,   221,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   235,    -1,   237,    -1,
     239,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   251,    -1,    -1,    -1,    -1,   256,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    11,    12,    -1,    -1,    -1,    16,
      -1,    -1,    -1,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,   313,    32,   315,    -1,    35,    36,
      -1,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      -1,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    -1,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    -1,
      97,    98,    99,   100,   101,    -1,   103,   104,   105,    -1,
      -1,   108,   109,    -1,    -1,    -1,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,    -1,    -1,    -1,
     127,    -1,   129,   130,   131,   132,    -1,   134,    -1,    -1,
      -1,   138,   139,   140,   141,   142,    -1,    -1,    -1,    -1,
      -1,    -1,   149,   150,   151,   152,   153,   154,   155,   156,
     157,    -1,   159,   160,   161,   162,   163,    -1,    -1,    -1,
     167,   168,   169,   170,   171,   172,   173,   174,    -1,   176,
      -1,   178,   179,    -1,    -1,   182,   183,   184,    -1,    -1,
      -1,   188,    -1,   190,   191,   192,   193,   194,    -1,   196,
      -1,   198,    -1,   200,   201,   202,   203,   204,   205,   206,
     207,   208,   209,   210,    -1,    -1,   213,   214,   215,    -1,
     217,   218,   219,   220,   221,   222,   223,   224,   225,   226,
     227,   228,   229,   230,   231,   232,   233,   234,   235,   236,
     237,   238,   239,   240,   241,   242,   243,   244,   245,   246,
     247,   248,   249,   250,   251,   252,   253,   254,   255,   256,
     257,   258,   259,    -1,   261,   262,   263,    -1,   265,   266,
     267,   268,   269,   270,   271,   272,   273,   274,   275,   276,
     277,   278,   279,   280,   281,   282,   283,   284,   285,   286,
     287,   288,   289,   290,   291,   292,   293,   294,   295,   296,
     297,   298,   299,   300,   301,   302,   303,   304,   305,   306,
     307,   308,   309,   310,   311,   312,    -1,   314,    -1,   316,
     317,   318,   319,   320,   321,   322,   323,   324,    11,    12,
      -1,    -1,    -1,    16,    -1,    -1,    -1,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    -1,    32,
      -1,    -1,    35,    36,    -1,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    -1,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    -1,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    -1,    97,    98,    99,   100,   101,    -1,
     103,   104,   105,    -1,    -1,   108,   109,    -1,    -1,    -1,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,    -1,    -1,    -1,   127,    -1,   129,   130,   131,   132,
      -1,   134,    -1,    -1,    -1,   138,   139,   140,   141,   142,
      -1,    -1,    -1,    -1,    -1,    -1,   149,   150,   151,   152,
     153,   154,   155,   156,   157,    -1,   159,   160,   161,   162,
     163,    -1,    -1,    -1,   167,   168,   169,   170,   171,   172,
     173,   174,    -1,   176,    -1,   178,   179,    -1,    -1,   182,
     183,   184,    -1,    -1,    -1,   188,    -1,   190,   191,   192,
     193,   194,    -1,   196,    -1,   198,    -1,   200,   201,   202,
     203,   204,   205,   206,   207,   208,   209,   210,    -1,    -1,
     213,   214,   215,    -1,   217,   218,   219,   220,   221,   222,
     223,   224,   225,   226,   227,   228,   229,   230,   231,   232,
     233,   234,   235,   236,   237,   238,   239,   240,   241,   242,
     243,   244,   245,   246,   247,   248,   249,   250,   251,   252,
     253,   254,   255,   256,   257,   258,   259,    -1,   261,   262,
     263,    -1,   265,   266,   267,   268,   269,   270,   271,   272,
     273,   274,   275,   276,   277,   278,   279,   280,   281,   282,
     283,   284,   285,   286,   287,   288,   289,   290,   291,   292,
     293,   294,   295,   296,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,   311,   312,
      -1,   314,    -1,   316,   317,   318,   319,   320,   321,   322,
      -1,   324,    11,    12,    -1,    -1,    -1,    16,    -1,    -1,
      -1,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    -1,    32,    -1,    -1,    35,    36,    -1,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    -1,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    -1,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    -1,    97,    98,
      99,   100,   101,    -1,   103,   104,   105,    -1,    -1,   108,
     109,    -1,    -1,    -1,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,    -1,    -1,    -1,   127,    -1,
     129,   130,   131,   132,    -1,   134,    -1,    -1,    -1,   138,
     139,   140,   141,   142,    -1,    -1,    -1,    -1,    -1,    -1,
     149,   150,   151,   152,   153,   154,   155,   156,   157,    -1,
     159,   160,   161,   162,   163,    -1,    -1,    -1,   167,   168,
     169,   170,   171,   172,   173,   174,    -1,   176,    -1,   178,
     179,    -1,    -1,   182,   183,   184,    -1,   186,    -1,   188,
      -1,   190,   191,   192,   193,   194,    -1,   196,    -1,   198,
      -1,   200,   201,   202,   203,   204,   205,   206,   207,   208,
     209,   210,    -1,    -1,   213,   214,   215,    -1,   217,   218,
     219,   220,   221,   222,   223,   224,   225,   226,   227,   228,
     229,   230,   231,   232,   233,   234,   235,   236,   237,   238,
     239,   240,   241,   242,   243,   244,   245,   246,   247,   248,
     249,   250,   251,   252,   253,   254,   255,   256,   257,   258,
     259,    -1,   261,   262,   263,    -1,   265,   266,   267,   268,
     269,   270,   271,   272,   273,   274,   275,   276,   277,   278,
     279,   280,   281,   282,   283,   284,   285,   286,   287,   288,
     289,   290,   291,   292,   293,   294,   295,   296,   297,   298,
     299,   300,   301,   302,   303,   304,   305,   306,   307,   308,
     309,   310,   311,   312,    -1,   314,    -1,   316,   317,   318,
     319,   320,   321,   322,    11,    12,    -1,    -1,    -1,    16,
      -1,    -1,    -1,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    -1,    32,    -1,    -1,    35,    36,
      -1,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      -1,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    -1,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    -1,
      97,    98,    99,   100,   101,    -1,   103,   104,   105,    -1,
      -1,   108,   109,    -1,    -1,    -1,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,    -1,    -1,    -1,
     127,    -1,   129,   130,   131,   132,    -1,   134,    -1,    -1,
      -1,   138,   139,   140,   141,   142,    -1,    -1,    -1,    -1,
      -1,    -1,   149,   150,   151,   152,   153,   154,   155,   156,
     157,    -1,   159,   160,   161,   162,   163,    -1,    -1,    -1,
     167,   168,   169,   170,   171,   172,   173,   174,    -1,   176,
      -1,   178,   179,    -1,    -1,   182,   183,   184,    -1,   186,
      -1,   188,    -1,   190,   191,   192,   193,   194,    -1,   196,
      -1,   198,    -1,   200,   201,   202,   203,   204,   205,   206,
     207,   208,   209,   210,    -1,    -1,   213,   214,   215,    -1,
     217,   218,   219,   220,   221,   222,   223,   224,   225,   226,
     227,   228,   229,   230,   231,   232,   233,   234,   235,   236,
     237,   238,   239,   240,   241,   242,   243,   244,   245,   246,
     247,   248,   249,   250,   251,   252,   253,   254,   255,   256,
     257,   258,   259,    -1,   261,   262,   263,    -1,   265,   266,
     267,   268,   269,   270,   271,   272,   273,   274,   275,   276,
     277,   278,   279,   280,   281,   282,   283,   284,   285,   286,
     287,   288,   289,   290,   291,   292,   293,   294,   295,   296,
     297,   298,   299,   300,   301,   302,   303,   304,   305,   306,
     307,   308,   309,   310,   311,   312,    -1,   314,    -1,   316,
     317,   318,   319,   320,   321,   322,    11,    12,    -1,    -1,
      -1,    16,    -1,    -1,    -1,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    -1,    32,    -1,    -1,
      35,    36,    -1,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    -1,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    -1,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    -1,    97,    98,    99,   100,   101,    -1,   103,   104,
     105,    -1,    -1,   108,   109,    -1,    -1,    -1,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,    -1,
      -1,    -1,   127,    -1,   129,   130,   131,   132,    -1,   134,
      -1,    -1,    -1,   138,   139,   140,   141,   142,    -1,    -1,
      -1,    -1,   147,    -1,   149,   150,   151,   152,   153,   154,
     155,   156,   157,    -1,   159,   160,   161,   162,   163,    -1,
      -1,    -1,   167,   168,   169,   170,   171,   172,   173,   174,
      -1,   176,    -1,   178,   179,    -1,    -1,   182,   183,   184,
      -1,    -1,    -1,   188,   189,   190,   191,    -1,   193,   194,
      -1,   196,    -1,   198,    -1,   200,   201,   202,   203,   204,
     205,   206,   207,   208,   209,   210,    -1,    -1,   213,   214,
     215,    -1,   217,   218,   219,   220,   221,   222,   223,   224,
     225,   226,   227,   228,   229,   230,   231,   232,   233,   234,
     235,   236,   237,   238,   239,   240,   241,   242,   243,   244,
     245,   246,   247,   248,   249,   250,   251,   252,   253,   254,
     255,   256,   257,   258,   259,    -1,   261,   262,   263,    -1,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,    -1,   314,
      -1,   316,   317,   318,   319,   320,   321,   322,    11,    12,
      -1,    -1,    -1,    16,    -1,    -1,    -1,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    -1,    32,
      -1,    -1,    35,    36,    -1,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    -1,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    -1,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    -1,    97,    98,    99,   100,   101,    -1,
     103,   104,   105,    -1,    -1,   108,   109,    -1,    -1,    -1,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,    -1,    -1,    -1,   127,    -1,   129,   130,   131,   132,
      -1,   134,    -1,    -1,    -1,   138,   139,   140,   141,   142,
      -1,    -1,    -1,    -1,   147,    -1,   149,   150,   151,   152,
     153,   154,   155,   156,   157,    -1,   159,   160,   161,   162,
     163,    -1,    -1,    -1,   167,   168,   169,   170,   171,   172,
     173,   174,    -1,   176,    -1,   178,   179,    -1,    -1,   182,
     183,   184,    -1,    -1,    -1,   188,   189,   190,   191,    -1,
     193,   194,    -1,   196,    -1,   198,    -1,   200,   201,   202,
     203,   204,   205,   206,   207,   208,   209,   210,    -1,    -1,
     213,   214,   215,    -1,   217,   218,   219,   220,   221,   222,
     223,   224,   225,   226,   227,   228,   229,   230,   231,   232,
     233,   234,   235,   236,   237,   238,   239,   240,   241,   242,
     243,   244,   245,   246,   247,   248,   249,   250,   251,   252,
     253,   254,   255,   256,   257,   258,   259,    -1,   261,   262,
     263,    -1,   265,   266,   267,   268,   269,   270,   271,   272,
     273,   274,   275,   276,   277,   278,   279,   280,   281,   282,
     283,   284,   285,   286,   287,   288,   289,   290,   291,   292,
     293,   294,   295,   296,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,   311,   312,
      -1,   314,    -1,   316,   317,   318,   319,   320,   321,   322,
      11,    12,    -1,    -1,    -1,    16,    -1,    -1,    -1,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      -1,    32,    -1,    -1,    35,    36,    -1,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    -1,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      -1,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    -1,    97,    98,    99,   100,
     101,    -1,   103,   104,   105,    -1,    -1,   108,   109,    -1,
      -1,    -1,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,    -1,    -1,    -1,   127,    -1,   129,   130,
     131,   132,    -1,   134,    -1,    -1,    -1,   138,   139,   140,
     141,   142,    -1,    -1,    -1,    -1,    -1,    -1,   149,   150,
     151,   152,   153,   154,   155,   156,   157,    -1,   159,   160,
     161,   162,   163,    -1,    -1,    -1,   167,   168,   169,   170,
     171,   172,   173,   174,    -1,   176,    -1,   178,   179,    -1,
      -1,   182,   183,   184,    -1,   186,    -1,   188,    -1,   190,
     191,   192,   193,   194,    -1,   196,    -1,   198,    -1,   200,
     201,   202,   203,   204,   205,   206,   207,   208,   209,   210,
      -1,    -1,   213,   214,   215,    -1,   217,   218,   219,   220,
     221,   222,   223,   224,   225,   226,   227,   228,   229,   230,
     231,   232,   233,   234,   235,   236,   237,   238,   239,   240,
     241,   242,   243,   244,   245,   246,   247,   248,   249,   250,
     251,   252,   253,   254,   255,   256,   257,   258,   259,    -1,
     261,   262,   263,    -1,   265,   266,   267,   268,   269,   270,
     271,   272,   273,   274,   275,   276,   277,   278,   279,   280,
     281,   282,   283,   284,   285,   286,   287,   288,   289,   290,
     291,   292,   293,   294,   295,   296,   297,   298,   299,   300,
     301,   302,   303,   304,   305,   306,   307,   308,   309,   310,
     311,   312,    -1,   314,    -1,   316,   317,   318,   319,   320,
     321,   322,    11,    12,    -1,    -1,    -1,    16,    -1,    -1,
      -1,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    -1,    32,    -1,    -1,    35,    36,    -1,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    -1,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    -1,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    -1,    97,    98,
      99,   100,   101,    -1,   103,   104,   105,    -1,    -1,   108,
     109,    -1,    -1,    -1,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,    -1,    -1,    -1,   127,    -1,
     129,   130,   131,   132,    -1,   134,    -1,    -1,    -1,   138,
     139,   140,   141,   142,    -1,    -1,    -1,    -1,    -1,    -1,
     149,   150,   151,   152,   153,   154,   155,   156,   157,    -1,
     159,   160,   161,   162,   163,    -1,    -1,    -1,   167,   168,
     169,   170,   171,   172,   173,   174,    -1,   176,    -1,   178,
     179,    -1,    -1,   182,   183,   184,    -1,    -1,    -1,   188,
      -1,   190,   191,   192,   193,   194,    -1,   196,    -1,   198,
      -1,   200,   201,   202,   203,   204,   205,   206,   207,   208,
     209,   210,    -1,    -1,   213,   214,   215,    -1,   217,   218,
     219,   220,   221,   222,   223,   224,   225,   226,   227,   228,
     229,   230,   231,   232,   233,   234,   235,   236,   237,   238,
     239,   240,   241,   242,   243,   244,   245,   246,   247,   248,
     249,   250,   251,   252,   253,   254,   255,   256,   257,   258,
     259,    -1,   261,   262,   263,    -1,   265,   266,   267,   268,
     269,   270,   271,   272,   273,   274,   275,   276,   277,   278,
     279,   280,   281,   282,   283,   284,   285,   286,   287,   288,
     289,   290,   291,   292,   293,   294,   295,   296,   297,   298,
     299,   300,   301,   302,   303,   304,   305,   306,   307,   308,
     309,   310,   311,   312,    -1,   314,    -1,   316,   317,   318,
     319,   320,   321,   322,    11,    12,    -1,    -1,    -1,    16,
      -1,    -1,    -1,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    -1,    32,    -1,    -1,    35,    36,
      -1,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      -1,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    -1,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    -1,
      97,    98,    99,   100,   101,    -1,   103,   104,   105,    -1,
      -1,   108,   109,    -1,    -1,    -1,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,    -1,    -1,    -1,
     127,    -1,   129,   130,   131,   132,    -1,   134,    -1,    -1,
      -1,   138,   139,   140,   141,   142,    -1,    -1,    -1,    -1,
      -1,    -1,   149,   150,   151,   152,   153,   154,   155,   156,
     157,    -1,   159,   160,   161,   162,   163,    -1,    -1,    -1,
     167,   168,   169,   170,   171,   172,   173,   174,    -1,   176,
      -1,   178,   179,    -1,    -1,   182,   183,   184,    -1,    -1,
     187,   188,    -1,   190,   191,    -1,   193,   194,    -1,   196,
      -1,   198,    -1,   200,   201,   202,   203,   204,   205,   206,
     207,   208,   209,   210,    -1,    -1,   213,   214,   215,    -1,
     217,   218,   219,   220,   221,   222,   223,   224,   225,   226,
     227,   228,   229,   230,   231,   232,   233,   234,   235,   236,
     237,   238,   239,   240,   241,   242,   243,   244,   245,   246,
     247,   248,   249,   250,   251,   252,   253,   254,   255,   256,
     257,   258,   259,    -1,   261,   262,   263,    -1,   265,   266,
     267,   268,   269,   270,   271,   272,   273,   274,   275,   276,
     277,   278,   279,   280,   281,   282,   283,   284,   285,   286,
     287,   288,   289,   290,   291,   292,   293,   294,   295,   296,
     297,   298,   299,   300,   301,   302,   303,   304,   305,   306,
     307,   308,   309,   310,   311,   312,    -1,   314,    -1,   316,
     317,   318,   319,   320,   321,   322,    11,    12,    -1,    -1,
      -1,    16,    -1,    -1,    -1,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    -1,    32,    -1,    -1,
      35,    36,    -1,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    -1,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    -1,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    -1,    97,    98,    99,   100,   101,    -1,   103,   104,
     105,    -1,    -1,   108,   109,    -1,    -1,    -1,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,    -1,
      -1,    -1,   127,    -1,   129,   130,   131,   132,    -1,   134,
      -1,    -1,    -1,   138,   139,   140,   141,   142,    -1,    -1,
      -1,    -1,    -1,    -1,   149,   150,   151,   152,   153,   154,
     155,   156,   157,    -1,   159,   160,   161,   162,   163,    -1,
      -1,    -1,   167,   168,   169,   170,   171,   172,   173,   174,
      -1,   176,    -1,   178,   179,    -1,    -1,   182,   183,   184,
      -1,    -1,    -1,   188,   189,   190,   191,    -1,   193,   194,
      -1,   196,    -1,   198,    -1,   200,   201,   202,   203,   204,
     205,   206,   207,   208,   209,   210,    -1,    -1,   213,   214,
     215,    -1,   217,   218,   219,   220,   221,   222,   223,   224,
     225,   226,   227,   228,   229,   230,   231,   232,   233,   234,
     235,   236,   237,   238,   239,   240,   241,   242,   243,   244,
     245,   246,   247,   248,   249,   250,   251,   252,   253,   254,
     255,   256,   257,   258,   259,    -1,   261,   262,   263,    -1,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,    -1,   314,
      -1,   316,   317,   318,   319,   320,   321,   322,    11,    12,
      -1,    -1,    -1,    16,    -1,    -1,    -1,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    -1,    32,
      -1,    -1,    35,    36,    -1,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    -1,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    -1,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    -1,    97,    98,    99,   100,   101,    -1,
     103,   104,   105,    -1,    -1,   108,   109,    -1,    -1,    -1,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,    -1,    -1,    -1,   127,    -1,   129,   130,   131,   132,
      -1,   134,    -1,    -1,    -1,   138,   139,   140,   141,   142,
      -1,    -1,    -1,    -1,    -1,    -1,   149,   150,   151,   152,
     153,   154,   155,   156,   157,    -1,   159,   160,   161,   162,
     163,    -1,    -1,    -1,   167,   168,   169,   170,   171,   172,
     173,   174,    -1,   176,    -1,   178,   179,    -1,    -1,   182,
     183,   184,    -1,    -1,    -1,   188,    -1,   190,   191,    -1,
     193,   194,    -1,   196,    -1,   198,    -1,   200,   201,   202,
     203,   204,   205,   206,   207,   208,   209,   210,    -1,    -1,
     213,   214,   215,    -1,   217,   218,   219,   220,   221,   222,
     223,   224,   225,   226,   227,   228,   229,   230,   231,   232,
     233,   234,   235,   236,   237,   238,   239,   240,   241,   242,
     243,   244,   245,   246,   247,   248,   249,   250,   251,   252,
     253,   254,   255,   256,   257,   258,   259,    -1,   261,   262,
     263,    -1,   265,   266,   267,   268,   269,   270,   271,   272,
     273,   274,   275,   276,   277,   278,   279,   280,   281,   282,
     283,   284,   285,   286,   287,   288,   289,   290,   291,   292,
     293,   294,   295,   296,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,   311,   312,
     313,   314,    -1,   316,   317,   318,   319,   320,   321,   322,
      11,    12,    -1,    -1,    -1,    16,    -1,    -1,    -1,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      -1,    32,    -1,    -1,    35,    36,    -1,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    -1,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      -1,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    -1,    97,    98,    99,   100,
     101,    -1,   103,   104,   105,    -1,    -1,   108,   109,    -1,
      -1,    -1,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,    -1,    -1,    -1,   127,    -1,   129,   130,
     131,   132,    -1,   134,    -1,    -1,    -1,   138,   139,   140,
     141,   142,    -1,    -1,    -1,    -1,    -1,    -1,   149,   150,
     151,   152,   153,   154,   155,   156,   157,    -1,   159,   160,
     161,   162,   163,    -1,    -1,    -1,   167,   168,   169,   170,
     171,   172,   173,   174,    -1,   176,    -1,   178,   179,    -1,
      -1,   182,   183,   184,    -1,    -1,    -1,   188,    -1,   190,
     191,    -1,   193,   194,    -1,   196,    -1,   198,    -1,   200,
     201,   202,   203,   204,   205,   206,   207,   208,   209,   210,
      -1,    -1,   213,   214,   215,    -1,   217,   218,   219,   220,
     221,   222,   223,   224,   225,   226,   227,   228,   229,   230,
     231,   232,   233,   234,   235,   236,   237,   238,   239,   240,
     241,   242,   243,   244,   245,   246,   247,   248,   249,   250,
     251,   252,   253,   254,   255,   256,   257,   258,   259,    -1,
     261,   262,   263,    -1,   265,   266,   267,   268,   269,   270,
     271,   272,   273,   274,   275,   276,   277,   278,   279,   280,
     281,   282,   283,   284,   285,   286,   287,   288,   289,   290,
     291,   292,   293,   294,   295,   296,   297,   298,   299,   300,
     301,   302,   303,   304,   305,   306,   307,   308,   309,   310,
     311,   312,    -1,   314,   315,   316,   317,   318,   319,   320,
     321,   322,    11,    12,    -1,    -1,    -1,    16,    -1,    -1,
      -1,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    -1,    32,    -1,    -1,    35,    36,    -1,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    -1,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    -1,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    -1,    97,    98,
      99,   100,   101,    -1,   103,   104,   105,    -1,    -1,   108,
     109,    -1,    -1,    -1,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,    -1,    -1,    -1,   127,    -1,
     129,   130,   131,   132,    -1,   134,    -1,    -1,    -1,   138,
     139,   140,   141,   142,    -1,    -1,    -1,    -1,    -1,    -1,
     149,   150,   151,   152,   153,   154,   155,   156,   157,    -1,
     159,   160,   161,   162,   163,    -1,    -1,    -1,   167,   168,
     169,   170,   171,   172,   173,   174,    -1,   176,    -1,   178,
     179,    -1,    -1,   182,   183,   184,    -1,    -1,    -1,   188,
      -1,   190,   191,   192,   193,   194,    -1,   196,    -1,   198,
      -1,   200,   201,   202,   203,   204,   205,   206,   207,   208,
     209,   210,    -1,    -1,   213,   214,   215,    -1,   217,   218,
     219,   220,   221,   222,   223,   224,   225,   226,   227,   228,
     229,   230,   231,   232,   233,   234,   235,   236,   237,   238,
     239,   240,   241,   242,   243,   244,   245,   246,   247,   248,
     249,   250,   251,   252,   253,   254,   255,   256,   257,   258,
     259,    -1,   261,   262,   263,    -1,   265,   266,   267,   268,
     269,   270,   271,   272,   273,   274,   275,   276,   277,   278,
     279,   280,   281,   282,   283,   284,   285,   286,   287,   288,
     289,   290,   291,   292,   293,   294,   295,   296,   297,   298,
     299,   300,   301,   302,   303,   304,   305,   306,   307,   308,
     309,   310,   311,   312,    -1,   314,    -1,   316,   317,   318,
     319,   320,   321,   322,    11,    12,    -1,    -1,    -1,    16,
      -1,    -1,    -1,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    -1,    32,    -1,    -1,    35,    36,
      -1,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      -1,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    -1,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    -1,
      97,    98,    99,   100,   101,    -1,   103,   104,   105,    -1,
      -1,   108,   109,    -1,    -1,    -1,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,    -1,    -1,    -1,
     127,    -1,   129,   130,   131,   132,    -1,   134,    -1,    -1,
      -1,   138,   139,   140,   141,   142,    -1,    -1,    -1,    -1,
      -1,    -1,   149,   150,   151,   152,   153,   154,   155,   156,
     157,    -1,   159,   160,   161,   162,   163,    -1,    -1,    -1,
     167,   168,   169,   170,   171,   172,   173,   174,    -1,   176,
      -1,   178,   179,    -1,    -1,   182,   183,   184,    -1,    -1,
      -1,   188,    -1,   190,   191,   192,   193,   194,    -1,   196,
      -1,   198,    -1,   200,   201,   202,   203,   204,   205,   206,
     207,   208,   209,   210,    -1,    -1,   213,   214,   215,    -1,
     217,   218,   219,   220,   221,   222,   223,   224,   225,   226,
     227,   228,   229,   230,   231,   232,   233,   234,   235,   236,
     237,   238,   239,   240,   241,   242,   243,   244,   245,   246,
     247,   248,   249,   250,   251,   252,   253,   254,   255,   256,
     257,   258,   259,    -1,   261,   262,   263,    -1,   265,   266,
     267,   268,   269,   270,   271,   272,   273,   274,   275,   276,
     277,   278,   279,   280,   281,   282,   283,   284,   285,   286,
     287,   288,   289,   290,   291,   292,   293,   294,   295,   296,
     297,   298,   299,   300,   301,   302,   303,   304,   305,   306,
     307,   308,   309,   310,   311,   312,    -1,   314,    -1,   316,
     317,   318,   319,   320,   321,   322,    11,    12,    -1,    -1,
      -1,    16,    -1,    -1,    -1,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    -1,    32,    -1,    -1,
      35,    36,    -1,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    -1,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    -1,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    -1,    97,    98,    99,   100,   101,    -1,   103,   104,
     105,    -1,    -1,   108,   109,    -1,    -1,    -1,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,    -1,
      -1,    -1,   127,    -1,   129,   130,   131,   132,    -1,   134,
      -1,    -1,    -1,   138,   139,   140,   141,   142,    -1,    -1,
      -1,    -1,    -1,    -1,   149,   150,   151,   152,   153,   154,
     155,   156,   157,    -1,   159,   160,   161,   162,   163,    -1,
      -1,    -1,   167,   168,   169,   170,   171,   172,   173,   174,
      -1,   176,    -1,   178,   179,    -1,    -1,   182,   183,   184,
      -1,    -1,    -1,   188,    -1,   190,   191,   192,   193,   194,
      -1,   196,    -1,   198,    -1,   200,   201,   202,   203,   204,
     205,   206,   207,   208,   209,   210,    -1,    -1,   213,   214,
     215,    -1,   217,   218,   219,   220,   221,   222,   223,   224,
     225,   226,   227,   228,   229,   230,   231,   232,   233,   234,
     235,   236,   237,   238,   239,   240,   241,   242,   243,   244,
     245,   246,   247,   248,   249,   250,   251,   252,   253,   254,
     255,   256,   257,   258,   259,    -1,   261,   262,   263,    -1,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,    -1,   314,
      -1,   316,   317,   318,   319,   320,   321,   322,    11,    12,
      -1,    -1,    -1,    16,    -1,    -1,    -1,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    -1,    32,
      -1,    -1,    35,    36,    -1,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    -1,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    -1,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    -1,    97,    98,    99,   100,   101,    -1,
     103,   104,   105,    -1,    -1,   108,   109,    -1,    -1,    -1,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,    -1,    -1,    -1,   127,    -1,   129,   130,   131,   132,
      -1,   134,    -1,    -1,    -1,   138,   139,   140,   141,   142,
      -1,    -1,    -1,    -1,    -1,    -1,   149,   150,   151,   152,
     153,   154,   155,   156,   157,    -1,   159,   160,   161,   162,
     163,    -1,    -1,    -1,   167,   168,   169,   170,   171,   172,
     173,   174,    -1,   176,    -1,   178,   179,    -1,    -1,   182,
     183,   184,    -1,    -1,    -1,   188,    -1,   190,   191,   192,
     193,   194,    -1,   196,    -1,   198,    -1,   200,   201,   202,
     203,   204,   205,   206,   207,   208,   209,   210,    -1,    -1,
     213,   214,   215,    -1,   217,   218,   219,   220,   221,   222,
     223,   224,   225,   226,   227,   228,   229,   230,   231,   232,
     233,   234,   235,   236,   237,   238,   239,   240,   241,   242,
     243,   244,   245,   246,   247,   248,   249,   250,   251,   252,
     253,   254,   255,   256,   257,   258,   259,    -1,   261,   262,
     263,    -1,   265,   266,   267,   268,   269,   270,   271,   272,
     273,   274,   275,   276,   277,   278,   279,   280,   281,   282,
     283,   284,   285,   286,   287,   288,   289,   290,   291,   292,
     293,   294,   295,   296,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,   311,   312,
      -1,   314,    -1,   316,   317,   318,   319,   320,   321,   322,
      11,    12,    -1,    -1,    -1,    16,    -1,    -1,    -1,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      -1,    32,    -1,    -1,    35,    36,    -1,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    -1,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      -1,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    -1,    97,    98,    99,   100,
     101,    -1,   103,   104,   105,    -1,    -1,   108,   109,    -1,
      -1,    -1,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,    -1,    -1,    -1,   127,    -1,   129,   130,
     131,   132,    -1,   134,    -1,    -1,    -1,   138,   139,   140,
     141,   142,    -1,    -1,    -1,    -1,    -1,    -1,   149,   150,
     151,   152,   153,   154,   155,   156,   157,    -1,   159,   160,
     161,   162,   163,    -1,    -1,    -1,   167,   168,   169,   170,
     171,   172,   173,   174,    -1,   176,    -1,   178,   179,    -1,
      -1,   182,   183,   184,    -1,    -1,    -1,   188,    -1,   190,
     191,   192,   193,   194,    -1,   196,    -1,   198,    -1,   200,
     201,   202,   203,   204,   205,   206,   207,   208,   209,   210,
      -1,    -1,   213,   214,   215,    -1,   217,   218,   219,   220,
     221,   222,   223,   224,   225,   226,   227,   228,   229,   230,
     231,   232,   233,   234,   235,   236,   237,   238,   239,   240,
     241,   242,   243,   244,   245,   246,   247,   248,   249,   250,
     251,   252,   253,   254,   255,   256,   257,   258,   259,    -1,
     261,   262,   263,    -1,   265,   266,   267,   268,   269,   270,
     271,   272,   273,   274,   275,   276,   277,   278,   279,   280,
     281,   282,   283,   284,   285,   286,   287,   288,   289,   290,
     291,   292,   293,   294,   295,   296,   297,   298,   299,   300,
     301,   302,   303,   304,   305,   306,   307,   308,   309,   310,
     311,   312,    -1,   314,    -1,   316,   317,   318,   319,   320,
     321,   322,    11,    12,    -1,    -1,    -1,    16,    -1,    -1,
      -1,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    -1,    32,    -1,    -1,    35,    36,    -1,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    -1,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    -1,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    -1,    97,    98,
      99,   100,   101,    -1,   103,   104,   105,    -1,    -1,   108,
     109,    -1,    -1,    -1,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,    -1,    -1,    -1,   127,    -1,
     129,   130,   131,   132,    -1,   134,    -1,    -1,    -1,   138,
     139,   140,   141,   142,    -1,    -1,    -1,    -1,   147,    -1,
     149,   150,   151,   152,   153,   154,   155,   156,   157,    -1,
     159,   160,   161,   162,   163,    -1,    -1,    -1,   167,   168,
     169,   170,   171,   172,   173,   174,    -1,   176,    -1,   178,
     179,    -1,    -1,   182,   183,   184,    -1,    -1,    -1,   188,
      -1,   190,   191,    -1,   193,   194,    -1,   196,    -1,   198,
      -1,   200,   201,   202,   203,   204,   205,   206,   207,   208,
     209,   210,    -1,    -1,   213,   214,   215,    -1,   217,   218,
     219,   220,   221,   222,   223,   224,   225,   226,   227,   228,
     229,   230,   231,   232,   233,   234,   235,   236,   237,   238,
     239,   240,   241,   242,   243,   244,   245,   246,   247,   248,
     249,   250,   251,   252,   253,   254,   255,   256,   257,   258,
     259,    -1,   261,   262,   263,    -1,   265,   266,   267,   268,
     269,   270,   271,   272,   273,   274,   275,   276,   277,   278,
     279,   280,   281,   282,   283,   284,   285,   286,   287,   288,
     289,   290,   291,   292,   293,   294,   295,   296,   297,   298,
     299,   300,   301,   302,   303,   304,   305,   306,   307,   308,
     309,   310,   311,   312,    -1,   314,    -1,   316,   317,   318,
     319,   320,   321,   322,    11,    12,    -1,    -1,    -1,    16,
      -1,    -1,    -1,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    -1,    32,    -1,    -1,    35,    36,
      -1,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      -1,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    -1,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    -1,
      97,    98,    99,   100,   101,    -1,   103,   104,   105,    -1,
      -1,   108,   109,    -1,    -1,    -1,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,    -1,    -1,    -1,
     127,    -1,   129,   130,   131,   132,    -1,   134,    -1,    -1,
      -1,   138,   139,   140,   141,   142,    -1,    -1,    -1,    -1,
      -1,    -1,   149,   150,   151,   152,   153,   154,   155,   156,
     157,    -1,   159,   160,   161,   162,   163,    -1,    -1,    -1,
     167,   168,   169,   170,   171,   172,   173,   174,    -1,   176,
      -1,   178,   179,    -1,    -1,   182,   183,   184,    -1,    -1,
      -1,   188,    -1,   190,   191,    -1,   193,   194,    -1,   196,
      -1,   198,    -1,   200,   201,   202,   203,   204,   205,   206,
     207,   208,   209,   210,    -1,    -1,   213,   214,   215,    -1,
     217,   218,   219,   220,   221,   222,   223,   224,   225,   226,
     227,   228,   229,   230,   231,   232,   233,   234,   235,   236,
     237,   238,   239,   240,   241,   242,   243,   244,   245,   246,
     247,   248,   249,   250,   251,   252,   253,   254,   255,   256,
     257,   258,   259,    -1,   261,   262,   263,    -1,   265,   266,
     267,   268,   269,   270,   271,   272,   273,   274,   275,   276,
     277,   278,   279,   280,   281,   282,   283,   284,   285,   286,
     287,   288,   289,   290,   291,   292,   293,   294,   295,   296,
     297,   298,   299,   300,   301,   302,   303,   304,   305,   306,
     307,   308,   309,   310,   311,   312,    -1,   314,    -1,   316,
     317,   318,   319,   320,   321,   322,    11,    12,    -1,    -1,
      -1,    16,    -1,    -1,    -1,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    -1,    32,    -1,    -1,
      35,    36,    -1,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    -1,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    -1,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    -1,    97,    98,    99,   100,   101,    -1,   103,   104,
     105,    -1,    -1,   108,   109,    -1,    -1,    -1,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,    -1,
      -1,    -1,   127,    -1,   129,   130,   131,   132,    -1,   134,
      -1,    -1,    -1,   138,   139,   140,   141,   142,    -1,    -1,
      -1,    -1,    -1,    -1,   149,   150,   151,   152,   153,   154,
     155,   156,   157,    -1,   159,   160,   161,   162,   163,    -1,
      -1,    -1,   167,   168,   169,   170,   171,   172,   173,   174,
      -1,   176,    -1,   178,   179,    -1,    -1,   182,   183,   184,
      -1,    -1,    -1,   188,    -1,   190,   191,    -1,   193,   194,
      -1,   196,    -1,   198,    -1,   200,   201,   202,   203,   204,
     205,   206,   207,   208,   209,   210,    -1,    -1,   213,   214,
     215,    -1,   217,   218,   219,   220,   221,   222,   223,   224,
     225,   226,   227,   228,   229,   230,   231,   232,   233,   234,
     235,   236,   237,   238,   239,   240,   241,   242,   243,   244,
     245,   246,   247,   248,   249,   250,   251,   252,   253,   254,
     255,   256,   257,   258,   259,    -1,   261,   262,   263,    -1,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,    -1,   314,
      -1,   316,   317,   318,   319,   320,   321,   322,    11,    12,
      -1,    -1,    -1,    16,    -1,    -1,    -1,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    -1,    32,
      -1,    -1,    35,    36,    -1,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    -1,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    -1,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    -1,    97,    98,    99,   100,   101,    -1,
     103,   104,   105,    -1,    -1,   108,   109,    -1,    -1,    -1,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,    -1,    -1,    -1,   127,    -1,   129,   130,   131,   132,
      -1,   134,    -1,    -1,    -1,   138,   139,   140,   141,   142,
      -1,    -1,    -1,    -1,    -1,    -1,   149,   150,   151,   152,
     153,   154,   155,   156,   157,    -1,   159,   160,   161,   162,
     163,    -1,    -1,    -1,   167,   168,   169,   170,   171,   172,
     173,   174,    -1,   176,    -1,   178,   179,    -1,    -1,   182,
     183,   184,    -1,    -1,    -1,   188,    -1,   190,   191,    -1,
     193,   194,    -1,   196,    -1,   198,    -1,   200,   201,   202,
     203,   204,   205,   206,   207,   208,   209,   210,    -1,    -1,
     213,   214,   215,    -1,   217,   218,   219,   220,   221,   222,
     223,   224,   225,   226,   227,   228,   229,   230,   231,   232,
     233,   234,   235,   236,   237,   238,   239,   240,   241,   242,
     243,   244,   245,   246,   247,   248,   249,   250,   251,   252,
     253,   254,   255,   256,   257,   258,   259,    -1,   261,   262,
     263,    -1,   265,   266,   267,   268,   269,   270,   271,   272,
     273,   274,   275,   276,   277,   278,   279,   280,   281,   282,
     283,   284,   285,   286,   287,   288,   289,   290,   291,   292,
     293,   294,   295,   296,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,   311,   312,
      -1,   314,    -1,   316,   317,   318,   319,   320,   321,   322,
      11,    12,    -1,    -1,    -1,    16,    -1,    -1,    -1,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      -1,    32,    -1,    -1,    35,    36,    -1,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    -1,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      -1,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    -1,    97,    98,    99,   100,
     101,    -1,   103,   104,   105,    -1,    -1,   108,   109,    -1,
      -1,    -1,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,    -1,    -1,    -1,   127,    -1,   129,   130,
     131,   132,    -1,   134,    -1,    -1,    -1,   138,   139,   140,
     141,   142,    -1,    -1,    -1,    -1,    -1,    -1,   149,   150,
     151,   152,   153,   154,   155,   156,   157,    -1,   159,   160,
     161,   162,   163,    -1,    -1,    -1,   167,   168,   169,   170,
     171,   172,   173,   174,    -1,   176,    -1,   178,   179,    -1,
      -1,   182,   183,   184,    -1,    -1,    -1,   188,    -1,   190,
     191,    -1,   193,   194,    -1,   196,    -1,   198,    -1,   200,
     201,   202,   203,   204,   205,   206,   207,   208,   209,   210,
      -1,    -1,   213,   214,   215,    -1,   217,   218,   219,   220,
     221,   222,   223,   224,   225,   226,   227,   228,   229,   230,
     231,   232,   233,   234,   235,   236,   237,   238,   239,   240,
     241,   242,   243,   244,   245,   246,   247,   248,   249,   250,
     251,   252,   253,   254,   255,   256,   257,   258,   259,    -1,
     261,   262,   263,    -1,   265,   266,   267,   268,   269,   270,
     271,   272,   273,   274,   275,   276,   277,   278,   279,   280,
     281,   282,   283,   284,   285,   286,   287,   288,   289,   290,
     291,   292,   293,   294,   295,   296,   297,   298,   299,   300,
     301,   302,   303,   304,   305,   306,   307,   308,   309,   310,
     311,   312,    -1,   314,    -1,   316,   317,   318,   319,   320,
     321,   322,    11,    12,    -1,    -1,    -1,    16,    -1,    -1,
      -1,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    -1,    32,    -1,    -1,    35,    36,    -1,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    -1,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    -1,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    -1,    97,    98,
      99,   100,   101,    -1,   103,   104,   105,    -1,    -1,   108,
     109,    -1,    -1,    -1,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,    -1,    -1,    -1,   127,    -1,
     129,   130,   131,   132,    -1,   134,    -1,    -1,    -1,   138,
     139,   140,   141,   142,    -1,    -1,    -1,    -1,    -1,    -1,
     149,   150,   151,   152,   153,   154,   155,   156,   157,    -1,
     159,   160,    -1,   162,   163,    -1,    -1,    -1,   167,   168,
     169,   170,   171,   172,   173,   174,    -1,   176,    -1,    -1,
      -1,    -1,    -1,   182,   183,   184,    -1,    -1,    -1,   188,
      -1,   190,   191,    -1,   193,   194,    -1,   196,    -1,   198,
      -1,   200,   201,   202,   203,   204,   205,   206,   207,   208,
     209,   210,    -1,    -1,   213,   214,   215,    -1,   217,   218,
     219,   220,   221,   222,   223,   224,   225,   226,   227,   228,
     229,   230,   231,   232,   233,   234,   235,   236,   237,   238,
     239,   240,   241,   242,   243,   244,   245,   246,   247,   248,
     249,   250,   251,   252,   253,   254,   255,   256,   257,   258,
     259,    -1,   261,   262,   263,    -1,   265,   266,   267,   268,
     269,   270,   271,   272,   273,   274,   275,   276,   277,   278,
     279,   280,   281,   282,   283,   284,   285,   286,   287,   288,
     289,   290,   291,   292,   293,   294,   295,   296,   297,   298,
     299,   300,   301,   302,   303,   304,   305,   306,   307,   308,
     309,   310,   311,   312,    -1,   314,    -1,   316,   317,   318,
     319,   320,   321,   322,    11,    12,    -1,    -1,    -1,    16,
      -1,    -1,    -1,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    -1,    32,    -1,    -1,    35,    36,
      -1,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      -1,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    -1,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    -1,
      97,    98,    99,   100,   101,    -1,   103,   104,   105,    -1,
      -1,   108,   109,    -1,    -1,    -1,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,    -1,    -1,    -1,
     127,    -1,   129,   130,   131,   132,    -1,   134,    -1,    -1,
      -1,   138,   139,   140,   141,   142,    -1,    -1,    -1,    -1,
      -1,    -1,   149,   150,   151,   152,   153,   154,   155,   156,
     157,    -1,   159,   160,    -1,   162,   163,    -1,    -1,    -1,
     167,   168,   169,   170,   171,   172,   173,   174,    -1,   176,
      -1,    -1,    -1,    -1,    -1,   182,   183,   184,    -1,    -1,
      -1,   188,    -1,   190,   191,    -1,    -1,    -1,    -1,   196,
      -1,   198,    -1,   200,   201,   202,   203,   204,   205,   206,
     207,   208,   209,   210,    -1,    -1,   213,   214,   215,    -1,
     217,   218,   219,   220,   221,   222,   223,   224,   225,   226,
     227,   228,   229,   230,   231,   232,   233,   234,   235,   236,
     237,   238,   239,   240,   241,   242,   243,   244,   245,   246,
     247,   248,   249,   250,   251,   252,   253,   254,   255,   256,
     257,   258,   259,    -1,   261,   262,   263,    -1,   265,   266,
     267,   268,   269,   270,   271,   272,   273,   274,   275,   276,
     277,   278,   279,   280,   281,   282,   283,   284,   285,   286,
     287,   288,   289,   290,   291,   292,   293,   294,   295,   296,
     297,   298,   299,   300,   301,   302,   303,   304,   305,   306,
     307,   308,   309,   310,   311,   312,    -1,   314,    -1,   316,
     317,   318,   319,   320,   321,   322,    11,    12,    -1,    -1,
      -1,    16,    -1,    -1,    -1,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    -1,    32,    -1,    -1,
      35,    36,    -1,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    -1,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    -1,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    -1,    97,    98,    99,   100,   101,    -1,   103,   104,
     105,    -1,    -1,   108,   109,    -1,    -1,    -1,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,    -1,
      -1,    -1,   127,    -1,   129,   130,   131,   132,    -1,   134,
      -1,    -1,    -1,   138,    -1,   140,   141,   142,    -1,    -1,
      -1,    -1,    -1,    -1,   149,   150,   151,   152,    -1,   154,
     155,   156,   157,    -1,   159,   160,    -1,   162,   163,    -1,
      -1,    -1,   167,   168,   169,   170,   171,   172,   173,   174,
      -1,   176,    -1,    -1,    -1,    -1,    -1,   182,   183,   184,
      -1,    -1,    -1,   188,    -1,   190,   191,    -1,    -1,    -1,
      -1,   196,    -1,   198,    -1,   200,    -1,   202,    -1,   204,
     205,   206,   207,   208,   209,   210,    -1,    -1,   213,   214,
     215,    -1,   217,   218,   219,   220,   221,   222,   223,   224,
     225,   226,   227,   228,   229,   230,   231,   232,   233,   234,
     235,   236,   237,   238,   239,   240,   241,   242,   243,   244,
     245,   246,   247,   248,   249,   250,   251,   252,   253,   254,
     255,   256,   257,   258,   259,    -1,   261,   262,   263,    -1,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,    -1,   314,
      -1,   316,   317,   318,   319,   320,   321,   322,    11,    12,
      -1,    -1,    -1,    16,    -1,    -1,    -1,    20,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    28,    -1,    -1,    -1,    -1,
      -1,    -1,    35,    36,    -1,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    -1,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    -1,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    -1,    97,    98,    99,   100,    -1,    -1,
     103,   104,   105,    -1,    -1,   108,   109,    -1,    -1,    -1,
      -1,   114,   115,   116,   117,   118,   119,   120,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   129,   130,   131,   132,
      -1,   134,    -1,    -1,    -1,   138,   139,   140,   141,   142,
      -1,    -1,    -1,    -1,    -1,    -1,   149,   150,   151,    -1,
     153,   154,   155,    -1,    -1,    -1,    -1,    -1,    -1,   162,
     163,    -1,    -1,    -1,   167,   168,   169,   170,   171,   172,
     173,   174,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   182,
     183,   184,    -1,    -1,    -1,   188,    -1,   190,   191,    -1,
      -1,    -1,    -1,   196,    -1,   198,    -1,   200,   201,   202,
     203,   204,   205,   206,   207,   208,   209,   210,    -1,    -1,
     213,   214,    -1,    -1,   217,   218,   219,   220,   221,   222,
     223,   224,   225,   226,   227,   228,   229,   230,   231,   232,
     233,   234,   235,   236,   237,   238,   239,   240,   241,   242,
     243,   244,   245,   246,   247,   248,   249,   250,   251,   252,
     253,   254,   255,   256,   257,   258,   259,    -1,   261,   262,
     263,    -1,   265,   266,   267,   268,   269,   270,   271,   272,
     273,   274,   275,   276,   277,   278,   279,   280,   281,   282,
     283,   284,   285,   286,   287,   288,   289,   290,   291,   292,
     293,   294,   295,   296,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,   311,    -1,
      -1,    -1,    -1,   316,   317,   318,   319,   320,   321,   322,
      11,    12,    -1,    -1,    -1,    16,    -1,    -1,    -1,    20,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    -1,    -1,
      -1,    -1,    -1,    -1,    35,    36,    -1,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    -1,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      -1,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    -1,    97,    98,    99,   100,
      -1,    -1,   103,   104,   105,    -1,    -1,   108,   109,    -1,
      -1,    -1,    -1,   114,   115,   116,   117,   118,   119,   120,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   129,   130,
     131,   132,    -1,   134,    -1,    -1,    -1,   138,   139,   140,
     141,   142,    -1,    -1,    -1,    -1,    -1,    -1,   149,   150,
     151,    -1,   153,   154,   155,    -1,    -1,    -1,    -1,    -1,
      -1,   162,   163,    -1,    -1,    -1,   167,   168,   169,   170,
     171,   172,   173,   174,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   182,   183,   184,    -1,    -1,    -1,   188,    -1,   190,
     191,    -1,    -1,    -1,    -1,   196,    -1,   198,    -1,   200,
     201,   202,   203,   204,   205,   206,   207,   208,   209,   210,
      -1,    -1,   213,   214,    -1,    -1,   217,   218,   219,   220,
     221,   222,   223,   224,   225,   226,   227,   228,   229,   230,
     231,   232,   233,   234,   235,   236,   237,   238,   239,   240,
     241,   242,   243,   244,   245,   246,   247,   248,   249,   250,
     251,   252,   253,   254,   255,   256,   257,   258,   259,    -1,
     261,   262,   263,    -1,   265,   266,   267,   268,   269,   270,
     271,   272,   273,   274,   275,   276,   277,   278,   279,   280,
     281,   282,   283,   284,   285,   286,   287,   288,   289,   290,
     291,   292,   293,   294,   295,   296,   297,   298,   299,   300,
     301,   302,   303,   304,   305,   306,   307,   308,   309,   310,
     311,    16,    -1,    -1,    -1,   316,   317,   318,   319,   320,
     321,   322,    27,    28,    29,    30,    -1,    -1,    -1,    -1,
      35,    36,    -1,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    -1,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    -1,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    -1,    97,    98,    99,   100,    -1,    -1,   103,   104,
     105,    -1,    -1,   108,   109,    -1,    -1,    -1,    -1,   114,
     115,   116,   117,   118,   119,   120,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   129,   130,   131,   132,    -1,   134,
      -1,    -1,    -1,   138,   139,   140,   141,   142,    -1,    -1,
      -1,    -1,    -1,    -1,   149,   150,   151,    -1,   153,   154,
     155,    -1,    -1,    -1,   159,    -1,    -1,   162,   163,    -1,
      -1,    -1,   167,   168,   169,   170,   171,   172,   173,   174,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   182,   183,   184,
      -1,    -1,    -1,   188,   189,   190,   191,    -1,    -1,    -1,
      -1,   196,    -1,   198,    -1,   200,   201,   202,   203,   204,
     205,   206,   207,   208,   209,   210,    -1,    -1,   213,   214,
      -1,    -1,   217,   218,   219,   220,   221,   222,   223,   224,
     225,   226,   227,   228,   229,   230,   231,   232,   233,   234,
     235,   236,   237,   238,   239,   240,   241,   242,   243,   244,
     245,   246,   247,   248,   249,   250,   251,   252,   253,   254,
     255,   256,   257,   258,   259,    -1,   261,   262,   263,    -1,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,    16,    -1,    -1,
      -1,   316,   317,   318,   319,   320,   321,   322,    27,    28,
      29,    30,    -1,    -1,    -1,    -1,    35,    36,    -1,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    -1,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    -1,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    -1,    97,    98,
      99,   100,    -1,    -1,   103,   104,   105,    -1,    -1,   108,
     109,    -1,    -1,    -1,    -1,   114,   115,   116,   117,   118,
     119,   120,   121,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     129,   130,   131,   132,    -1,   134,    -1,    -1,    -1,   138,
     139,   140,   141,   142,    -1,    -1,    -1,    -1,    -1,    -1,
     149,   150,   151,    -1,   153,   154,   155,    -1,    -1,    -1,
     159,    -1,    -1,   162,   163,    -1,    -1,    -1,   167,   168,
     169,   170,   171,   172,   173,   174,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   182,   183,   184,    -1,    -1,    -1,   188,
      -1,   190,   191,    -1,    -1,    -1,    -1,    -1,    -1,   198,
      -1,   200,   201,   202,   203,   204,   205,   206,   207,   208,
     209,   210,    -1,    -1,   213,   214,    -1,    -1,   217,   218,
     219,   220,   221,   222,   223,   224,   225,   226,   227,   228,
     229,   230,   231,   232,   233,   234,   235,   236,   237,   238,
     239,   240,   241,   242,   243,   244,   245,   246,   247,   248,
     249,   250,   251,   252,   253,   254,   255,   256,   257,   258,
     259,    -1,   261,   262,   263,    -1,   265,   266,   267,   268,
     269,   270,   271,   272,   273,   274,   275,   276,   277,   278,
     279,   280,   281,   282,   283,   284,   285,   286,   287,   288,
     289,   290,   291,   292,   293,   294,   295,   296,   297,   298,
     299,   300,   301,   302,   303,   304,   305,   306,   307,   308,
     309,   310,   311,    16,    -1,    -1,    -1,   316,   317,   318,
     319,   320,   321,   322,    27,    28,    29,    30,    -1,    -1,
      -1,    -1,    35,    36,    -1,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    -1,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    -1,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    -1,    97,    98,    99,   100,    -1,    -1,
     103,   104,   105,    -1,    -1,   108,   109,    -1,    -1,    -1,
      -1,   114,   115,   116,   117,   118,   119,   120,   121,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   129,   130,   131,   132,
      -1,   134,    -1,    -1,    -1,   138,   139,   140,   141,   142,
      -1,    -1,    -1,    -1,    -1,    -1,   149,   150,   151,    -1,
     153,   154,   155,    -1,    -1,    -1,   159,    -1,    -1,   162,
     163,    -1,    -1,    -1,   167,   168,   169,   170,   171,   172,
     173,   174,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   182,
     183,   184,    -1,    -1,    -1,   188,    -1,   190,   191,    -1,
      -1,    -1,    -1,    -1,    -1,   198,    -1,   200,   201,   202,
     203,   204,   205,   206,   207,   208,   209,   210,    -1,    -1,
     213,   214,    -1,    -1,   217,   218,   219,   220,   221,   222,
     223,   224,   225,   226,   227,   228,   229,   230,   231,   232,
     233,   234,   235,   236,   237,   238,   239,   240,   241,   242,
     243,   244,   245,   246,   247,   248,   249,   250,   251,   252,
     253,   254,   255,   256,   257,   258,   259,    -1,   261,   262,
     263,    -1,   265,   266,   267,   268,   269,   270,   271,   272,
     273,   274,   275,   276,   277,   278,   279,   280,   281,   282,
     283,   284,   285,   286,   287,   288,   289,   290,   291,   292,
     293,   294,   295,   296,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,   311,    16,
      -1,    -1,    -1,   316,   317,   318,   319,   320,   321,   322,
      27,    28,    29,    30,    -1,    -1,    -1,    -1,    35,    36,
      -1,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      -1,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    -1,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    -1,
      97,    98,    99,   100,    -1,    -1,   103,   104,   105,    -1,
      -1,   108,   109,    -1,    -1,    -1,    -1,   114,   115,   116,
     117,   118,   119,   120,   121,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   129,   130,   131,   132,    -1,   134,    -1,    -1,
      -1,   138,   139,   140,   141,   142,    -1,    -1,    -1,    -1,
      -1,    -1,   149,   150,   151,    -1,   153,   154,   155,    -1,
      -1,    -1,   159,    -1,    -1,   162,   163,    -1,    -1,    -1,
     167,   168,   169,   170,   171,   172,   173,   174,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   182,   183,   184,    -1,    -1,
      -1,   188,    -1,   190,   191,    -1,    -1,    -1,    -1,    -1,
      -1,   198,    -1,   200,   201,   202,   203,   204,   205,   206,
     207,   208,   209,   210,    -1,    -1,   213,   214,    -1,    -1,
     217,   218,   219,   220,   221,   222,   223,   224,   225,   226,
     227,   228,   229,   230,   231,   232,   233,   234,   235,   236,
     237,   238,   239,   240,   241,   242,   243,   244,   245,   246,
     247,   248,   249,   250,   251,   252,   253,   254,   255,   256,
     257,   258,   259,    -1,   261,   262,   263,    -1,   265,   266,
     267,   268,   269,   270,   271,   272,   273,   274,   275,   276,
     277,   278,   279,   280,   281,   282,   283,   284,   285,   286,
     287,   288,   289,   290,   291,   292,   293,   294,   295,   296,
     297,   298,   299,   300,   301,   302,   303,   304,   305,   306,
     307,   308,   309,   310,   311,    16,    -1,    -1,    -1,   316,
     317,   318,   319,   320,   321,   322,    27,    28,    29,    30,
      -1,    -1,    -1,    -1,    35,    36,    -1,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    -1,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      -1,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    -1,    97,    98,    99,   100,
      -1,    -1,   103,   104,   105,    -1,    -1,   108,   109,    -1,
      -1,    -1,    -1,   114,   115,   116,   117,   118,   119,   120,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   129,   130,
     131,   132,    -1,   134,    -1,    -1,    -1,   138,   139,   140,
     141,   142,    -1,    -1,    -1,    -1,    -1,    -1,   149,   150,
     151,    -1,   153,   154,   155,    -1,    -1,    -1,   159,    -1,
      -1,   162,   163,    -1,    -1,    -1,   167,   168,   169,   170,
     171,   172,   173,   174,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   182,   183,   184,    -1,    -1,    -1,   188,    -1,   190,
     191,    -1,    -1,    -1,    -1,    -1,    -1,   198,    -1,   200,
     201,   202,   203,   204,   205,   206,   207,   208,   209,   210,
      -1,    -1,   213,   214,    -1,    -1,   217,   218,   219,   220,
     221,   222,   223,   224,   225,   226,   227,   228,   229,   230,
     231,   232,   233,   234,   235,   236,   237,   238,   239,   240,
     241,   242,   243,   244,   245,   246,   247,   248,   249,   250,
     251,   252,   253,   254,   255,   256,   257,   258,   259,    -1,
     261,   262,   263,    -1,   265,   266,   267,   268,   269,   270,
     271,   272,   273,   274,   275,   276,   277,   278,   279,   280,
     281,   282,   283,   284,   285,   286,   287,   288,   289,   290,
     291,   292,   293,   294,   295,   296,   297,   298,   299,   300,
     301,   302,   303,   304,   305,   306,   307,   308,   309,   310,
     311,    16,    -1,    -1,    -1,   316,   317,   318,   319,   320,
     321,   322,    27,    28,    29,    30,    -1,    -1,    -1,    -1,
      35,    36,    -1,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    -1,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    -1,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    -1,    97,    98,    99,   100,    -1,    -1,   103,   104,
     105,    -1,    -1,   108,   109,    -1,    -1,    -1,    -1,   114,
     115,   116,   117,   118,   119,   120,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   129,   130,   131,   132,    -1,   134,
      -1,    -1,    -1,   138,   139,   140,   141,   142,    -1,    -1,
      -1,    -1,    -1,    -1,   149,   150,   151,    -1,   153,   154,
     155,    -1,    -1,    -1,   159,    -1,    -1,   162,   163,    -1,
      -1,    -1,   167,   168,   169,   170,   171,   172,   173,   174,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   182,   183,   184,
      -1,    -1,    -1,   188,    -1,   190,   191,    -1,    -1,    -1,
      -1,    -1,    -1,   198,    -1,   200,   201,   202,   203,   204,
     205,   206,   207,   208,   209,   210,    -1,    -1,   213,   214,
      -1,    -1,   217,   218,   219,   220,   221,   222,   223,   224,
     225,   226,   227,   228,   229,   230,   231,   232,   233,   234,
     235,   236,   237,   238,   239,   240,   241,   242,   243,   244,
     245,   246,   247,   248,   249,   250,   251,   252,   253,   254,
     255,   256,   257,   258,   259,    -1,   261,   262,   263,    -1,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,    16,    -1,    18,
      19,   316,   317,   318,   319,   320,   321,   322,    27,    28,
      -1,    -1,    -1,    -1,    -1,    -1,    35,    36,    -1,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    -1,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    -1,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    -1,    97,    98,
      99,   100,    -1,    -1,   103,   104,   105,    -1,    -1,   108,
     109,    -1,    -1,    -1,    -1,   114,   115,   116,   117,   118,
     119,   120,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     129,   130,   131,   132,    -1,   134,    -1,    -1,    -1,   138,
     139,   140,   141,   142,    -1,    -1,    -1,    -1,    -1,    -1,
     149,   150,   151,    -1,   153,   154,   155,    -1,    -1,    -1,
      -1,    -1,    -1,   162,   163,    -1,    -1,    -1,   167,   168,
     169,   170,   171,   172,   173,   174,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   182,   183,   184,    -1,    -1,    -1,   188,
      -1,   190,   191,    -1,    -1,    -1,    -1,    -1,    -1,   198,
      -1,   200,   201,   202,   203,   204,   205,   206,   207,   208,
     209,   210,    -1,    -1,   213,   214,    -1,    -1,   217,   218,
     219,   220,   221,   222,   223,   224,   225,   226,   227,   228,
     229,   230,   231,   232,   233,   234,   235,   236,   237,   238,
     239,   240,   241,   242,   243,   244,   245,   246,   247,   248,
     249,   250,   251,   252,   253,   254,   255,   256,   257,   258,
     259,    -1,   261,   262,   263,    -1,   265,   266,   267,   268,
     269,   270,   271,   272,   273,   274,   275,   276,   277,   278,
     279,   280,   281,   282,   283,   284,   285,   286,   287,   288,
     289,   290,   291,   292,   293,   294,   295,   296,   297,   298,
     299,   300,   301,   302,   303,   304,   305,   306,   307,   308,
     309,   310,   311,    16,    -1,    -1,    -1,   316,   317,   318,
     319,   320,   321,   322,    27,    28,    -1,    -1,    -1,    -1,
      -1,    -1,    35,    36,    -1,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    -1,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    -1,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    -1,    97,    98,    99,   100,    -1,    -1,
     103,   104,   105,    -1,    -1,   108,   109,    -1,    -1,    -1,
      -1,   114,   115,   116,   117,   118,   119,   120,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   129,   130,   131,   132,
      -1,   134,    -1,    -1,    -1,   138,   139,   140,   141,   142,
      -1,    -1,    -1,    -1,    -1,    -1,   149,   150,   151,    -1,
     153,   154,   155,    -1,    -1,    -1,    -1,    -1,    -1,   162,
     163,    -1,    -1,    -1,   167,   168,   169,   170,   171,   172,
     173,   174,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   182,
     183,   184,    -1,    -1,    -1,   188,   189,   190,   191,    -1,
      -1,    -1,    -1,   196,    -1,   198,    -1,   200,   201,   202,
     203,   204,   205,   206,   207,   208,   209,   210,    -1,    -1,
     213,   214,    -1,    -1,   217,   218,   219,   220,   221,   222,
     223,   224,   225,   226,   227,   228,   229,   230,   231,   232,
     233,   234,   235,   236,   237,   238,   239,   240,   241,   242,
     243,   244,   245,   246,   247,   248,   249,   250,   251,   252,
     253,   254,   255,   256,   257,   258,   259,    -1,   261,   262,
     263,    -1,   265,   266,   267,   268,   269,   270,   271,   272,
     273,   274,   275,   276,   277,   278,   279,   280,   281,   282,
     283,   284,   285,   286,   287,   288,   289,   290,   291,   292,
     293,   294,   295,   296,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,   311,    16,
      -1,    -1,    -1,   316,   317,   318,   319,   320,   321,   322,
      27,    28,    -1,    -1,    -1,    32,    -1,    -1,    35,    36,
      -1,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      -1,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    -1,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    -1,
      97,    98,    99,   100,    -1,    -1,   103,   104,   105,    -1,
      -1,   108,   109,    -1,    -1,    -1,    -1,   114,   115,   116,
     117,   118,   119,   120,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   129,   130,   131,   132,    -1,   134,    -1,    -1,
      -1,   138,   139,   140,   141,   142,    -1,    -1,    -1,    -1,
      -1,    -1,   149,   150,   151,    -1,   153,   154,   155,    -1,
      -1,    -1,    -1,    -1,    -1,   162,   163,    -1,    -1,    -1,
     167,   168,   169,   170,   171,   172,   173,   174,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   182,   183,   184,    -1,    -1,
      -1,   188,   189,   190,   191,    -1,    -1,    -1,    -1,    -1,
      -1,   198,    -1,   200,   201,   202,   203,   204,   205,   206,
     207,   208,   209,   210,    -1,    -1,   213,   214,    -1,    -1,
     217,   218,   219,   220,   221,   222,   223,   224,   225,   226,
     227,   228,   229,   230,   231,   232,   233,   234,   235,   236,
     237,   238,   239,   240,   241,   242,   243,   244,   245,   246,
     247,   248,   249,   250,   251,   252,   253,   254,   255,   256,
     257,   258,   259,    -1,   261,   262,   263,    -1,   265,   266,
     267,   268,   269,   270,   271,   272,   273,   274,   275,   276,
     277,   278,   279,   280,   281,   282,   283,   284,   285,   286,
     287,   288,   289,   290,   291,   292,   293,   294,   295,   296,
     297,   298,   299,   300,   301,   302,   303,   304,   305,   306,
     307,   308,   309,   310,   311,    16,    -1,    -1,    -1,   316,
     317,   318,   319,   320,   321,   322,    27,    28,    -1,    -1,
      -1,    -1,    -1,    -1,    35,    36,    -1,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    -1,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      -1,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    -1,    97,    98,    99,   100,
      -1,    -1,   103,   104,   105,    -1,    -1,   108,   109,    -1,
      -1,    -1,    -1,   114,   115,   116,   117,   118,   119,   120,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   129,   130,
     131,   132,    -1,   134,    -1,    -1,    -1,   138,   139,   140,
     141,   142,    -1,    -1,    -1,    -1,    -1,    -1,   149,   150,
     151,    -1,   153,   154,   155,    -1,    -1,    -1,    -1,    -1,
      -1,   162,   163,    -1,    -1,    -1,   167,   168,   169,   170,
     171,   172,   173,   174,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   182,   183,   184,    -1,    -1,    -1,   188,   189,   190,
     191,    -1,    -1,    -1,    -1,   196,    -1,   198,    -1,   200,
     201,   202,   203,   204,   205,   206,   207,   208,   209,   210,
      -1,    -1,   213,   214,    -1,    -1,   217,   218,   219,   220,
     221,   222,   223,   224,   225,   226,   227,   228,   229,   230,
     231,   232,   233,   234,   235,   236,   237,   238,   239,   240,
     241,   242,   243,   244,   245,   246,   247,   248,   249,   250,
     251,   252,   253,   254,   255,   256,   257,   258,   259,    -1,
     261,   262,   263,    -1,   265,   266,   267,   268,   269,   270,
     271,   272,   273,   274,   275,   276,   277,   278,   279,   280,
     281,   282,   283,   284,   285,   286,   287,   288,   289,   290,
     291,   292,   293,   294,   295,   296,   297,   298,   299,   300,
     301,   302,   303,   304,   305,   306,   307,   308,   309,   310,
     311,    16,    -1,    -1,    -1,   316,   317,   318,   319,   320,
     321,   322,    27,    28,    -1,    -1,    -1,    -1,    -1,    -1,
      35,    36,    -1,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    -1,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    -1,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    -1,    97,    98,    99,   100,    -1,    -1,   103,   104,
     105,    -1,    -1,   108,   109,    -1,    -1,    -1,    -1,   114,
     115,   116,   117,   118,   119,   120,   121,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   129,   130,   131,   132,    -1,   134,
      -1,    -1,    -1,   138,   139,   140,   141,   142,    -1,    -1,
      -1,    -1,    -1,    -1,   149,   150,   151,    -1,   153,   154,
     155,    -1,    -1,    -1,    -1,    -1,    -1,   162,   163,    -1,
      -1,    -1,   167,   168,   169,   170,   171,   172,   173,   174,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   182,   183,   184,
      -1,    -1,    -1,   188,    -1,   190,   191,    -1,    -1,    -1,
      -1,    -1,    -1,   198,    -1,   200,   201,   202,   203,   204,
     205,   206,   207,   208,   209,   210,    -1,    -1,   213,   214,
      -1,    -1,   217,   218,   219,   220,   221,   222,   223,   224,
     225,   226,   227,   228,   229,   230,   231,   232,   233,   234,
     235,   236,   237,   238,   239,   240,   241,   242,   243,   244,
     245,   246,   247,   248,   249,   250,   251,   252,   253,   254,
     255,   256,   257,   258,   259,    -1,   261,   262,   263,    -1,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,    16,    -1,    -1,
      -1,   316,   317,   318,   319,   320,   321,   322,    27,    28,
      -1,    -1,    -1,    -1,    -1,    -1,    35,    36,    -1,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    -1,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    -1,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    -1,    97,    98,
      99,   100,    -1,    -1,   103,   104,   105,    -1,    -1,   108,
     109,    -1,    -1,    -1,    -1,   114,   115,   116,   117,   118,
     119,   120,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     129,   130,   131,   132,    -1,   134,    -1,    -1,    -1,   138,
     139,   140,   141,   142,    -1,    -1,    -1,    -1,    -1,    -1,
     149,   150,   151,    -1,   153,   154,   155,    -1,    -1,    -1,
      -1,    -1,    -1,   162,   163,    -1,    -1,    -1,   167,   168,
     169,   170,   171,   172,   173,   174,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   182,   183,   184,    -1,    -1,    -1,   188,
      -1,   190,   191,    -1,    -1,    -1,    -1,    -1,    -1,   198,
      -1,   200,   201,   202,   203,   204,   205,   206,   207,   208,
     209,   210,    -1,    -1,   213,   214,    -1,    -1,   217,   218,
     219,   220,   221,   222,   223,   224,   225,   226,   227,   228,
     229,   230,   231,   232,   233,   234,   235,   236,   237,   238,
     239,   240,   241,   242,   243,   244,   245,   246,   247,   248,
     249,   250,   251,   252,   253,   254,   255,   256,   257,   258,
     259,    -1,   261,   262,   263,    -1,   265,   266,   267,   268,
     269,   270,   271,   272,   273,   274,   275,   276,   277,   278,
     279,   280,   281,   282,   283,   284,   285,   286,   287,   288,
     289,   290,   291,   292,   293,   294,   295,   296,   297,   298,
     299,   300,   301,   302,   303,   304,   305,   306,   307,   308,
     309,   310,   311,    -1,    -1,    -1,    -1,   316,   317,   318,
     319,   320,   321,   322,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    -1,    32,    -1,    -1,    35,    36,
      -1,    38,    39,    40,    41,    42,    43,    44,    45,    -1,
      47,    48,    49,    -1,    51,    52,    53,    54,    -1,    56,
      -1,    58,    59,    60,    61,    62,    63,    -1,    -1,    66,
      67,    68,    69,    70,    -1,    72,    73,    74,    -1,    -1,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    -1,
      97,    98,    99,   100,    -1,    -1,   103,   104,   105,    -1,
      -1,   108,   109,    -1,    -1,    -1,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,    -1,    -1,    -1,    -1,
     127,    -1,   129,   130,   131,   132,    -1,   134,    -1,    -1,
      -1,   138,   139,   140,   141,   142,    -1,    -1,    -1,    -1,
      -1,    -1,   149,   150,   151,   152,   153,   154,    -1,   156,
     157,    -1,   159,   160,    -1,   162,   163,    -1,    -1,    -1,
     167,   168,   169,   170,   171,   172,   173,   174,    -1,   176,
      -1,    -1,    -1,    -1,    -1,   182,   183,   184,    -1,    -1,
      -1,   188,    -1,   190,   191,    -1,    -1,    -1,    -1,    -1,
      -1,   198,    -1,   200,   201,   202,   203,   204,   205,   206,
     207,   208,   209,   210,    -1,    -1,    -1,   214,   215,    -1,
     217,   218,   219,   220,   221,   222,   223,   224,   225,   226,
      -1,   228,   229,   230,   231,   232,   233,   234,   235,   236,
     237,   238,   239,   240,   241,   242,   243,   244,    -1,   246,
     247,   248,   249,   250,   251,   252,   253,   254,   255,   256,
     257,   258,   259,    -1,   261,   262,   263,    -1,   265,   266,
     267,   268,   269,   270,   271,   272,   273,   274,   275,   276,
     277,   278,   279,   280,   281,   282,   283,   284,   285,   286,
     287,   288,   289,   290,   291,   292,   293,   294,   295,   296,
     297,   298,   299,   300,   301,   302,   303,   304,   305,   306,
     307,   308,   309,   310,   311,   312,    -1,   314,    27,    28,
     317,   318,    -1,    -1,    -1,    -1,    35,    36,    -1,    38,
      39,    40,    -1,    42,    43,    44,    45,    -1,    -1,    48,
      49,    -1,    51,    52,    53,    54,    -1,    56,    -1,    58,
      59,    60,    61,    -1,    63,    -1,    -1,    66,    67,    68,
      69,    70,    -1,    72,    -1,    74,    -1,    -1,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    -1,    97,    98,
      99,    -1,    -1,    -1,   103,   104,   105,    -1,    -1,   108,
     109,    -1,    -1,    -1,    -1,   114,   115,   116,   117,   118,
     119,   120,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     129,   130,   131,   132,    -1,   134,    -1,    -1,    -1,   138,
     139,   140,   141,   142,    -1,    -1,    -1,    -1,    -1,    -1,
     149,   150,   151,    -1,   153,   154,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   162,   163,    -1,    -1,    -1,   167,   168,
     169,   170,   171,   172,   173,   174,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   182,   183,   184,    -1,    -1,    -1,   188,
      -1,   190,   191,    -1,    -1,    -1,    -1,    -1,    -1,   198,
      -1,   200,   201,   202,   203,   204,   205,   206,   207,   208,
     209,   210,    -1,    -1,    -1,   214,    -1,    -1,   217,   218,
     219,   220,   221,   222,   223,   224,   225,   226,    -1,   228,
     229,   230,   231,   232,   233,   234,   235,   236,   237,   238,
     239,   240,   241,   242,   243,   244,    -1,   246,   247,   248,
     249,   250,   251,   252,   253,   254,   255,   256,   257,   258,
     259,    -1,   261,   262,   263,    -1,   265,   266,   267,   268,
     269,   270,   271,   272,   273,   274,   275,   276,   277,   278,
     279,   280,   281,   282,   283,   284,   285,   286,   287,   288,
     289,   290,   291,   292,   293,   294,   295,   296,   297,   298,
     299,   300,   301,   302,   303,   304,   305,   306,   307,   308,
     309,   310,   311,    -1,    -1,    -1,    -1,    -1,   317,   318
  };

  const short
  xquery_parser::yystos_[] =
  {
       0,    11,    12,    16,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    32,    35,    36,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    97,    98,    99,   100,   101,   103,
     104,   105,   108,   109,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   127,   129,   130,   131,   132,
     134,   138,   139,   140,   141,   142,   149,   150,   151,   152,
     153,   154,   155,   156,   157,   159,   160,   161,   162,   163,
     167,   168,   169,   170,   171,   172,   173,   174,   176,   178,
     179,   182,   183,   184,   188,   190,   191,   192,   193,   194,
     196,   198,   200,   201,   202,   203,   204,   205,   206,   207,
     208,   209,   210,   213,   214,   215,   217,   218,   219,   220,
     221,   222,   223,   224,   225,   226,   227,   228,   229,   230,
     231,   232,   233,   234,   235,   236,   237,   238,   239,   240,
     241,   242,   243,   244,   245,   246,   247,   248,   249,   250,
     251,   252,   253,   254,   255,   256,   257,   258,   259,   261,
     262,   263,   265,   266,   267,   268,   269,   270,   271,   272,
     273,   274,   275,   276,   277,   278,   279,   280,   281,   282,
     283,   284,   285,   286,   287,   288,   289,   290,   291,   292,
     293,   294,   295,   296,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,   311,   312,
     314,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     337,   339,   340,   341,   342,   343,   344,   345,   346,   347,
     348,   349,   350,   351,   352,   353,   354,   355,   358,   359,
     360,   361,   362,   363,   367,   368,   369,   373,   375,   376,
     378,   385,   387,   390,   391,   392,   394,   395,   396,   397,
     398,   400,   401,   403,   404,   405,   406,   407,   408,   410,
     411,   414,   415,   416,   417,   418,   424,   426,   428,   429,
     430,   435,   454,   457,   461,   464,   465,   471,   472,   473,
     474,   476,   477,   479,   480,   481,   482,   483,   484,   485,
     486,   487,   489,   490,   491,   492,   495,   496,   497,   498,
     499,   500,   501,   502,   503,   504,   505,   506,   507,   508,
     509,   510,   511,   512,   515,   516,   517,   518,   519,   520,
     521,   522,   523,   525,   526,   527,   528,   529,   530,   544,
     545,   547,   548,   549,   550,   551,   552,   553,   554,   562,
     563,   564,   565,   566,   567,   568,   569,   570,   571,   572,
     573,   576,   581,   582,   583,   584,   585,   586,   589,   647,
     648,   649,   650,   652,   653,   654,   655,   656,   659,   660,
     661,    35,    36,    51,   217,   393,   394,   395,   393,   393,
     393,   393,   394,   395,   159,   159,    39,    40,    42,    43,
      49,    59,    60,    77,    82,   114,   115,   116,   163,   222,
     263,   289,   297,   305,   374,   375,   379,   380,   381,   163,
     156,   159,   156,   159,   156,   159,     1,   159,   382,   159,
      27,    36,    63,   121,   275,   434,   436,   437,   159,   156,
     159,   159,   159,   121,   156,   159,   159,   159,    79,   156,
     223,   224,   125,   125,   125,   156,   159,    41,    45,    47,
      48,    49,    50,    53,    62,    70,    73,    75,    76,    78,
      93,    94,   100,   108,   115,   117,   118,   138,   141,   142,
     163,   171,   174,   182,   183,   191,   204,   226,   228,   230,
     231,   238,   240,   241,   242,   245,   246,   317,   509,   660,
     125,   121,   402,   125,   125,    41,    46,    47,    55,    62,
      64,    65,    73,   100,   227,   659,   121,   125,   125,   186,
     393,   395,   416,   651,    50,    75,    76,   121,   156,   187,
     246,   375,   415,   417,   428,   189,   415,   659,   156,   156,
     125,   659,    18,    19,   659,   125,   125,   125,   501,   156,
      33,   216,   218,   227,   229,   316,   227,   229,   316,   227,
     316,   227,   232,   121,   243,   243,   244,   159,   156,   397,
     313,   415,   315,   415,   316,   324,   339,   339,     0,   341,
     342,    35,    51,   344,   361,     1,   192,   338,   192,   338,
      49,   115,   376,   396,   415,   110,   192,   110,   338,   192,
      44,    48,    53,    72,   170,   173,   188,   214,   409,   419,
     420,   425,   426,   427,   442,   443,   447,     3,    69,    74,
     121,   421,   169,    95,   135,   143,   144,   146,   154,   158,
     160,   165,   181,   205,   206,   207,   208,   209,   210,   493,
     494,   251,   102,   161,   178,   201,   120,   149,   162,   196,
     203,   212,   139,   153,    52,   202,   104,   105,   161,   178,
     491,   195,   156,   498,   501,   193,   194,   157,   513,   514,
     509,   513,   509,   159,   513,   159,   148,   159,   186,   186,
     186,   186,   186,   186,   377,   516,   377,    32,   658,   184,
     198,   184,   198,   168,   184,   660,   659,   171,   204,    49,
     659,   155,   121,    47,    49,    82,   109,   170,   659,   223,
     224,   225,   248,   620,   659,   659,   306,   140,   145,   115,
     289,   297,   379,   659,   394,   189,   394,    47,    65,   189,
     572,   573,   415,   189,   196,   659,   431,   432,   659,   121,
     189,   383,   384,   156,   399,   415,     1,   163,   658,   116,
     163,   357,   658,   659,   121,   145,   110,   189,   415,    32,
     189,   659,   659,   659,   455,   456,   659,   394,   189,   415,
     415,   574,   659,   394,   156,   156,   415,   189,   196,   659,
     659,   145,   455,   186,   186,   124,   110,   186,   159,   159,
     159,   659,   156,   187,   420,   189,    37,   532,   533,   534,
     415,   415,     8,   177,    17,   415,   216,    32,   416,   416,
      41,    47,    62,    73,   100,   512,   660,   416,   416,   416,
     651,   416,   512,   416,   233,   587,   588,   659,   192,   192,
     416,   415,   395,   415,   247,   412,   413,   313,   315,   416,
     339,   192,   338,   192,   338,     3,   345,   361,   391,     1,
     345,   361,   391,    35,   354,   362,   391,   362,   391,   382,
     402,   338,   402,   416,   416,   121,   170,   172,   172,   416,
     396,   416,   286,   286,   431,   121,   438,   473,   474,   476,
     476,   476,   476,   475,   476,   476,   476,    73,   477,   481,
     481,   480,   482,   482,   482,   482,   483,   483,   484,   484,
     233,    97,    97,    97,   499,   393,   501,   501,   415,   514,
     147,   189,   416,   524,   189,   152,   189,   524,   110,   189,
     189,   110,   110,   382,    32,   660,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,   364,   365,   366,    97,
     140,   145,   370,   371,   372,   659,   163,   163,   364,   658,
     130,   135,    57,    59,   103,   253,   266,   268,   279,   280,
     281,   283,   285,   621,   622,   623,   624,   625,   626,   633,
     639,   640,   248,    97,   301,   659,   145,   416,   121,   659,
     659,   135,   186,   186,   189,   189,   186,   110,   189,   110,
     189,   110,    38,    97,    99,   150,   433,   434,   555,   659,
      97,   110,   189,   393,   189,   659,    99,    47,   659,   658,
      99,   145,   555,   659,   416,   437,   186,   189,   189,   189,
     189,   110,   190,   150,   555,   186,   189,   189,   156,   186,
     394,   394,   186,   110,   189,   110,   189,   145,   555,   416,
     190,   416,   416,   415,   415,   415,   659,   533,   534,   133,
     199,   186,   186,   186,   134,   192,    97,   220,   221,   235,
      97,   220,   221,   235,   235,   235,    97,    97,   239,   227,
     316,   110,   237,   145,   192,   189,   415,   186,   510,   593,
     413,   235,   361,    35,    35,   192,   338,   192,   116,   399,
     396,   659,   172,   416,   448,   449,   121,   416,   444,   445,
     659,    58,   219,   256,   422,   423,   476,   156,   159,   261,
     497,   576,   594,   597,   598,   599,   600,   601,   605,   607,
     609,   610,    49,   155,   159,   213,   319,   320,   321,   322,
     375,   556,   558,   560,   562,   577,   578,   579,   580,   657,
     659,   556,   488,   561,   659,   488,   186,   187,   110,   189,
     189,   516,   151,   167,   151,   167,   140,   399,   382,   365,
     135,   558,   372,   416,   555,   658,   658,   131,   132,   658,
     279,   280,   281,   285,   659,   265,   276,   265,   276,    32,
     288,    99,   116,   159,   627,   630,   621,    41,    46,    55,
      62,    64,    73,   100,   227,   319,   320,   321,   386,   562,
     657,   229,   301,   310,   416,   659,    97,   301,   658,   156,
     574,   575,   659,   574,   575,   121,   432,   130,   556,   121,
     416,   150,   434,   150,    38,   150,   433,   434,   555,   556,
     384,    97,   186,   200,   135,   356,   658,   163,   135,    99,
     356,   416,   145,   434,   156,   121,   416,   416,   150,   103,
     458,   459,   460,   462,   463,   103,   466,   467,   468,   469,
     394,   186,   186,   156,   574,   574,   416,   145,   192,   416,
     124,   189,   189,   189,    37,   534,   133,   199,     9,    10,
     106,   126,   128,   156,   197,   529,   531,   542,   543,   546,
     156,    32,   234,   236,   416,   416,   416,   234,   236,   416,
     416,   416,   416,   416,   416,   416,   416,   416,   512,   121,
     416,   416,    50,    75,    76,   246,   396,   417,   428,   247,
     590,   591,   156,   212,   397,   416,   192,   116,   391,   391,
     391,   448,    98,   109,   119,   130,   450,   451,   452,   453,
     110,   659,   109,   446,   110,   150,   555,   256,    56,    61,
      80,   121,   433,   439,   440,   441,   423,   415,   594,   601,
     156,   287,   478,   646,    99,   171,   254,   255,   257,   264,
     274,   286,   595,   596,   615,   616,   617,   618,   641,   644,
     260,   262,   602,   620,   269,   606,   642,   249,   250,   272,
     611,   612,   159,   159,   558,   159,   159,   159,   159,   159,
      49,   578,   579,   147,   178,   196,   557,   147,   147,   416,
     140,   399,   576,   371,   288,    32,    99,   116,   159,   634,
      32,   627,   557,   557,   499,   289,   308,   555,   386,   229,
     192,   393,   189,   189,   147,   189,   189,   432,   150,   433,
     659,   416,   150,   416,   130,   416,   150,   434,   150,   556,
     396,   416,   658,   110,   356,   416,   145,   393,   456,   416,
     416,   116,   459,   460,   103,   420,   116,   460,   463,   121,
     470,   556,   103,   116,   467,   103,   116,   469,   186,   393,
     189,   189,   416,   416,   200,   466,   135,   197,   531,     7,
     394,   659,   197,   542,   394,   192,   235,   235,   235,   235,
      99,   239,   239,   588,   420,   159,   159,   159,   420,   593,
     591,   510,   658,   131,   132,   452,   453,   453,   449,   145,
     555,   658,   445,   416,   150,   121,   121,   416,   659,   441,
      80,   186,   189,   594,   608,   252,   219,   256,   270,   277,
     645,    99,   258,   259,   643,   252,   598,   645,   480,   615,
     599,   150,   284,   603,   604,   643,   288,   614,    81,   613,
     189,   196,   556,   559,   189,   189,   189,   189,   189,   189,
     189,    32,   139,   203,   636,   637,   638,    32,   635,   636,
     273,   631,   110,   628,   172,   659,   259,   499,   186,   416,
     150,   416,   150,   433,   416,   150,   416,   129,   129,    99,
     658,   416,   186,   420,   420,   416,   396,   416,   420,   420,
     659,   212,   420,   121,   470,   121,   420,   121,   470,   420,
     186,   116,   534,   659,   197,   186,   534,   659,   186,   416,
     416,   416,   416,   318,   416,   416,   416,   415,   415,   415,
     156,   592,   453,   416,   145,   416,   659,   659,   440,   416,
     186,   482,    54,   132,   480,   480,   271,   278,   288,   619,
     619,   600,   156,   282,    97,   189,   110,   189,   634,   634,
     638,   110,   189,    32,   632,   643,   629,   630,   189,   388,
     389,   499,   121,   227,   309,   289,   172,   416,   416,   150,
     416,   396,   416,   356,   416,   396,    97,   556,   396,   416,
     659,   420,   659,   416,   659,   420,   396,   121,    96,   185,
     535,   534,   659,   199,   534,   416,   189,   189,   189,   415,
     446,   416,    56,   480,   480,   201,   415,   556,   556,    97,
      32,   267,   110,   110,   453,   555,   659,   121,   227,   659,
     388,   416,   470,    97,   420,    97,   659,     5,   136,   538,
     539,   541,   543,    31,   137,   536,   537,   540,   543,   199,
     534,   199,   200,   466,   186,   446,   121,   480,   186,   556,
     630,   389,   453,   307,   659,   121,   227,   420,   470,   396,
     416,   470,   420,    96,   136,   541,   185,   137,   540,   199,
     116,   659,   416,   307,   659,   121,   396,   416,   420,   420,
     121,   295,   307,   659,   659,   308,   416,   308,   420,   499,
     499,   201,   289,   659,   227,   121,   659,   308,   499
  };

  const short
  xquery_parser::yyr1_[] =
  {
       0,   336,   337,   337,   337,   337,   338,   338,   338,   339,
     339,   339,   339,   340,   340,   341,   341,   341,   341,   341,
     341,   341,   341,   342,   342,   342,   342,   343,   344,   344,
     344,   345,   345,   345,   345,   345,   346,   346,   346,   346,
     346,   346,   346,   346,   347,   347,   348,   349,   350,   350,
     351,   351,   352,   352,   353,   353,   353,   353,   354,   354,
     354,   355,   355,   355,   355,   356,   356,   357,   357,   358,
     358,   358,   358,   359,   360,   360,   361,   361,   361,   361,
     362,   362,   362,   362,   362,   362,   362,   362,   363,   363,
     364,   364,   365,   366,   366,   366,   366,   366,   366,   366,
     366,   366,   366,   367,   368,   369,   370,   370,   371,   371,
     371,   372,   373,   373,   373,   374,   374,   374,   374,   375,
     375,   376,   376,   376,   376,   377,   377,   378,   378,   379,
     379,   380,   380,   381,   381,   382,   382,   382,   382,   383,
     383,   384,   384,   385,   385,   385,   385,   386,   386,   386,
     386,   387,   387,   388,   388,   389,   389,   389,   389,   390,
     390,   390,   390,   391,   392,   392,   392,   393,   393,   393,
     394,   394,   395,   395,   395,   396,   396,   396,   396,   396,
     396,   396,   396,   396,   396,   396,   396,   396,   397,   397,
     398,   399,   400,   401,   401,   401,   402,   402,   402,   402,
     403,   404,   405,   406,   407,   407,   408,   409,   410,   411,
     412,   412,   413,   414,   415,   415,   415,   416,   416,   416,
     416,   416,   416,   417,   417,   417,   417,   417,   417,   417,
     417,   417,   417,   417,   417,   418,   419,   420,   421,   421,
     422,   422,   422,   423,   423,   424,   424,   425,   426,   426,
     426,   427,   427,   427,   427,   427,   428,   428,   429,   429,
     429,   430,   431,   431,   431,   432,   432,   432,   432,   432,
     432,   432,   432,   432,   432,   432,   432,   433,   434,   435,
     436,   436,   437,   437,   437,   437,   438,   438,   439,   439,
     439,   440,   440,   440,   441,   441,   441,   442,   443,   444,
     444,   445,   445,   445,   445,   445,   445,   446,   447,   447,
     448,   448,   449,   449,   450,   450,   450,   450,   450,   450,
     450,   451,   451,   452,   452,   453,   454,   454,   455,   455,
     456,   456,   457,   458,   458,   459,   460,   460,   461,   462,
     462,   463,   464,   464,   465,   465,   466,   466,   467,   467,
     468,   468,   469,   469,   470,   470,   471,   472,   472,   473,
     473,   474,   474,   474,   474,   474,   475,   474,   474,   474,
     474,   476,   476,   477,   477,   478,   478,   479,   479,   480,
     480,   480,   481,   481,   481,   481,   481,   482,   482,   482,
     483,   483,   483,   484,   484,   485,   485,   486,   486,   487,
     487,   488,   488,   489,   489,   490,   490,   490,   490,   491,
     491,   491,   492,   492,   493,   493,   493,   493,   493,   493,
     494,   494,   494,   495,   495,   495,   495,   496,   497,   497,
     498,   498,   498,   499,   499,   499,   499,   500,   501,   501,
     501,   502,   502,   503,   503,   503,   503,   504,   504,   505,
     505,   505,   505,   505,   505,   505,   506,   506,   507,   507,
     508,   508,   508,   508,   508,   509,   509,   510,   510,   511,
     511,   511,   511,   512,   512,   512,   512,   513,   513,   514,
     515,   515,   515,   515,   515,   515,   515,   515,   515,   515,
     515,   515,   515,   515,   516,   516,   517,   517,   517,   518,
     519,   519,   520,   521,   522,   523,   523,   524,   524,   524,
     524,   525,   525,   526,   527,   527,   528,   528,   529,   529,
     529,   530,   530,   530,   530,   530,   530,   531,   531,   532,
     532,   533,   534,   534,   535,   535,   536,   536,   537,   537,
     537,   537,   538,   538,   539,   539,   539,   539,   540,   540,
     541,   541,   542,   542,   542,   542,   543,   543,   543,   543,
     544,   544,   545,   545,   546,   547,   547,   547,   547,   547,
     547,   547,   548,   549,   549,   549,   550,   550,   550,   551,
     552,   553,   553,   554,   554,   555,   556,   556,   556,   557,
     557,   557,   558,   558,   558,   558,   558,   558,   558,   559,
     559,   560,   561,   562,   562,   562,   562,   562,   562,   562,
     562,   562,   562,   563,   564,   565,   565,   565,   566,   567,
     568,   569,   569,   569,   570,   570,   570,   570,   570,   571,
     572,   572,   572,   572,   572,   572,   572,   573,   574,   575,
     576,   577,   577,   577,   577,   578,   579,   579,   580,   581,
     581,   581,   582,   582,   582,   582,   582,   582,   582,   582,
     582,   582,   583,   583,   584,   584,   585,   586,   587,   587,
     588,   589,   590,   590,   591,   592,   593,   593,   594,   595,
     595,   596,   596,   597,   597,   598,   598,   599,   599,   600,
     600,   601,   602,   602,   603,   603,   604,   605,   605,   605,
     606,   606,   607,   608,   608,   609,   610,   610,   611,   611,
     612,   612,   612,   613,   613,   614,   614,   615,   615,   615,
     615,   615,   616,   617,   618,   619,   619,   619,   620,   620,
     621,   621,   621,   621,   621,   621,   621,   621,   622,   622,
     622,   622,   623,   623,   624,   625,   625,   626,   626,   626,
     627,   627,   628,   628,   629,   629,   630,   631,   631,   632,
     632,   633,   633,   633,   634,   634,   635,   635,   636,   636,
     637,   637,   638,   638,   639,   640,   640,   641,   641,   641,
     642,   643,   643,   643,   643,   644,   644,   645,   645,   646,
     647,   647,   648,   648,   649,   649,   650,   651,   651,   652,
     652,   652,   653,   654,   655,   656,   657,   657,   657,   658,
     659,   659,   659,   659,   659,   659,   659,   659,   659,   659,
     659,   659,   659,   659,   659,   659,   659,   659,   659,   659,
     659,   659,   659,   660,   660,   660,   660,   660,   660,   660,
     660,   660,   660,   660,   660,   660,   660,   660,   660,   660,
     660,   660,   660,   660,   660,   660,   660,   660,   660,   660,
     660,   660,   660,   660,   660,   660,   660,   660,   660,   660,
     660,   660,   660,   660,   660,   660,   660,   660,   660,   660,
     660,   660,   660,   660,   660,   660,   660,   660,   660,   660,
     660,   660,   660,   660,   660,   660,   660,   660,   660,   660,
     660,   660,   660,   660,   660,   660,   660,   660,   660,   660,
     660,   660,   660,   660,   660,   660,   660,   660,   660,   660,
     660,   660,   660,   660,   660,   660,   660,   660,   660,   660,
     660,   660,   660,   660,   660,   660,   660,   660,   660,   660,
     660,   660,   660,   660,   660,   660,   660,   660,   660,   660,
     660,   660,   660,   660,   660,   660,   660,   660,   660,   660,
     660,   660,   660,   660,   660,   660,   660,   660,   660,   660,
     660,   660,   660,   660,   660,   660,   660,   660,   660,   660,
     660,   660,   660,   660,   660,   660,   660,   660,   660,   660,
     660,   660,   660,   660,   660,   660,   660,   660,   660,   660,
     660,   660,   660,   660,   660,   660,   660,   660,   660,   660,
     660,   660,   660,   660,   660,   660,   660,   660,   660,   660,
     660,   660,   660,   660,   660,   660,   660,   660,   660,   660,
     660,   660,   660,   660,   661
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
       6,     5,     8,     5,     5,     5,     1,     3,     3,     3,
       1,     1,     1,     1,     1,     1,     1,     1,     4,     4,
       1,     2,     3,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     4,     3,     4,     3,     1,     1,     1,
       2,     2,     4,     3,     5,     3,     4,     4,     5,     1,
       2,     1,     4,     1,     4,     1,     3,     2,     3,     1,
       1,     4,     4,     5,     5,     2,     3,     4,     5,     1,
       3,     2,     3,     3,     5,     4,     6,     1,     2,     1,
       2,     8,     9,     1,     3,     1,     2,     2,     3,    11,
      14,    13,    22,     1,     1,     1,     0,     1,     1,     0,
       1,     2,     1,     2,     4,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     3,     2,
       3,     3,     2,     3,     2,     3,     2,     3,     4,     5,
       5,     2,     4,     5,     3,     3,     2,     2,     8,     3,
       1,     2,     3,     1,     1,     3,     3,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     2,     2,     1,     2,     2,
       1,     1,     2,     4,     3,     5,     4,     3,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     2,     3,     3,
       2,     1,     1,     4,     3,     3,     5,     4,     6,     4,
       6,     5,     7,     4,     5,     5,     6,     3,     3,     2,
       1,     3,     4,     5,     3,     6,     4,     5,     1,     2,
       3,     1,     2,     1,     6,     3,     3,     2,     3,     1,
       3,     4,     5,     6,     5,     1,     2,     2,     3,     4,
       1,     3,     1,     2,     1,     1,     1,     2,     2,     2,
       3,     1,     1,     2,     2,     2,     5,     5,     1,     4,
       3,     4,     8,     1,     2,     3,     2,     3,     8,     1,
       2,     3,     8,    10,     8,    10,     1,     2,     4,     7,
       1,     2,     4,     7,     1,     3,     8,     1,     3,     1,
       3,     1,     3,     3,     3,     3,     0,     4,     3,     3,
       3,     1,     5,     1,     3,     0,     1,     1,     3,     1,
       3,     3,     1,     3,     3,     3,     3,     1,     3,     3,
       1,     3,     3,     1,     4,     1,     4,     1,     4,     1,
       4,     1,     2,     1,     2,     1,     1,     2,     2,     1,
       1,     1,     1,     3,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     4,     5,     5,     6,     4,     1,     2,
       3,     2,     2,     1,     2,     2,     1,     1,     1,     3,
       3,     1,     1,     1,     2,     1,     2,     2,     1,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     2,     1,
       2,     2,     2,     2,     2,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     2,     3,     4,     1,     2,     3,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     2,
       2,     3,     1,     4,     4,     3,     4,     1,     3,     1,
       3,     1,     1,     3,     3,     4,     1,     1,     1,     1,
       1,     4,     8,     5,     9,     9,    10,     1,     2,     1,
       2,     6,     0,     1,     3,     3,     0,     1,     1,     1,
       2,     2,     0,     1,     1,     1,     2,     2,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     3,
       3,     2,     3,     3,     2,     1,     1,     1,     1,     1,
       1,     1,     4,     3,     3,     7,     3,     3,     7,     4,
       4,     3,     7,     3,     7,     2,     1,     2,     3,     1,
       1,     1,     1,     1,     3,     3,     1,     1,     1,     1,
       3,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     3,     1,     3,     4,     4,     3,     3,
       3,     3,     4,     4,     3,     4,     6,     4,     6,     4,
       3,     4,     6,     6,     4,     6,     6,     4,     1,     2,
       1,     1,     1,     2,     2,     4,     5,     6,     3,     3,
       3,     3,     5,     7,     7,     5,     5,     5,     7,     7,
       5,     5,     3,     3,     5,     7,     5,     7,     1,     4,
       3,     5,     1,     2,     3,     3,     1,     3,     2,     0,
       1,     1,     2,     1,     3,     1,     3,     1,     4,     1,
       2,     3,     0,     1,     0,     1,     4,     2,     3,     1,
       0,     1,     4,     0,     1,     2,     1,     3,     0,     1,
       2,     2,     1,     0,     1,     0,     1,     1,     1,     1,
       1,     1,     1,     3,     3,     1,     1,     1,     2,     3,
       1,     1,     1,     1,     1,     1,     1,     1,     2,     2,
       1,     1,     2,     2,     3,     1,     2,     2,     5,     2,
       1,     1,     0,     2,     1,     3,     4,     0,     2,     0,
       2,     4,     4,     3,     2,     3,     1,     3,     0,     1,
       1,     2,     2,     2,     2,     1,     2,     2,     2,     2,
       3,     2,     3,     3,     4,     2,     2,     1,     1,     3,
       2,     3,     2,     3,     2,     3,     3,     3,     5,     5,
       5,     8,     5,     3,     5,     7,     3,     3,     3,     1,
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
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1
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
  "\"'element EQName {'\"", "\"'attribute EQName {'\"",
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
       0,   878,   878,   879,   883,   887,   893,   897,   901,   907,
     913,   921,   927,   936,   941,   947,   953,   959,   967,   972,
     982,   992,  1003,  1015,  1020,  1026,  1032,  1041,  1049,  1056,
    1062,  1073,  1074,  1075,  1076,  1077,  1080,  1081,  1082,  1083,
    1084,  1085,  1086,  1087,  1090,  1095,  1101,  1107,  1113,  1118,
    1124,  1129,  1135,  1140,  1146,  1151,  1156,  1161,  1167,  1169,
    1171,  1179,  1184,  1192,  1200,  1209,  1216,  1224,  1230,  1236,
    1242,  1249,  1257,  1268,  1275,  1282,  1290,  1297,  1303,  1313,
    1323,  1324,  1325,  1326,  1327,  1328,  1329,  1330,  1333,  1339,
    1346,  1353,  1361,  1369,  1370,  1371,  1372,  1373,  1374,  1375,
    1376,  1377,  1378,  1381,  1395,  1401,  1407,  1414,  1420,  1427,
    1432,  1438,  1444,  1456,  1468,  1481,  1486,  1494,  1502,  1511,
    1516,  1523,  1528,  1535,  1540,  1548,  1553,  1560,  1566,  1575,
    1576,  1579,  1593,  1606,  1621,  1634,  1639,  1644,  1649,  1655,
    1662,  1670,  1675,  1683,  1691,  1699,  1707,  1717,  1721,  1727,
    1731,  1738,  1747,  1758,  1764,  1771,  1776,  1783,  1790,  1799,
    1809,  1819,  1829,  1844,  1860,  1864,  1869,  1874,  1878,  1883,
    1888,  1893,  1901,  1908,  1915,  1928,  1929,  1930,  1931,  1932,
    1933,  1934,  1935,  1936,  1937,  1938,  1939,  1940,  1943,  1948,
    1955,  1975,  1981,  1987,  1994,  2001,  2010,  2019,  2028,  2037,
    2048,  2054,  2060,  2066,  2078,  2083,  2089,  2101,  2114,  2134,
    2140,  2147,  2156,  2163,  2169,  2174,  2186,  2198,  2199,  2200,
    2201,  2202,  2203,  2206,  2207,  2208,  2209,  2210,  2211,  2212,
    2213,  2214,  2215,  2216,  2217,  2220,  2232,  2238,  2243,  2248,
    2254,  2259,  2264,  2270,  2279,  2289,  2299,  2310,  2316,  2317,
    2318,  2321,  2322,  2323,  2324,  2325,  2328,  2335,  2343,  2347,
    2355,  2363,  2368,  2375,  2382,  2393,  2403,  2413,  2423,  2433,
    2443,  2453,  2463,  2473,  2482,  2492,  2502,  2514,  2520,  2526,
    2532,  2538,  2546,  2554,  2562,  2570,  2580,  2586,  2595,  2596,
    2600,  2607,  2611,  2616,  2619,  2623,  2627,  2633,  2639,  2645,
    2651,  2659,  2663,  2671,  2679,  2687,  2695,  2705,  2711,  2717,
    2725,  2731,  2739,  2743,  2751,  2757,  2763,  2769,  2778,  2787,
    2796,  2807,  2811,  2817,  2823,  2831,  2837,  2846,  2857,  2863,
    2871,  2875,  2884,  2890,  2896,  2904,  2910,  2916,  2924,  2930,
    2936,  2944,  2950,  2957,  2967,  2974,  2984,  2990,  2998,  3004,
    3013,  3019,  3027,  3033,  3042,  3048,  3056,  3062,  3066,  3072,
    3077,  3083,  3087,  3096,  3102,  3111,  3121,  3120,  3133,  3142,
    3151,  3162,  3166,  3177,  3181,  3187,  3190,  3196,  3200,  3206,
    3210,  3214,  3220,  3224,  3230,  3236,  3242,  3250,  3254,  3258,
    3264,  3268,  3274,  3282,  3286,  3294,  3298,  3306,  3310,  3318,
    3322,  3330,  3334,  3340,  3344,  3350,  3354,  3358,  3362,  3370,
    3371,  3372,  3375,  3380,  3386,  3390,  3394,  3398,  3402,  3406,
    3412,  3416,  3420,  3426,  3430,  3434,  3438,  3447,  3455,  3461,
    3469,  3473,  3477,  3483,  3487,  3495,  3503,  3523,  3529,  3539,
    3543,  3549,  3550,  3553,  3559,  3567,  3573,  3583,  3589,  3597,
    3601,  3605,  3609,  3613,  3619,  3625,  3631,  3635,  3641,  3645,
    3654,  3658,  3662,  3668,  3672,  3680,  3681,  3684,  3688,  3694,
    3698,  3702,  3706,  3712,  3716,  3720,  3724,  3730,  3736,  3744,
    3750,  3751,  3752,  3753,  3754,  3755,  3756,  3757,  3758,  3759,
    3760,  3761,  3762,  3763,  3766,  3770,  3776,  3783,  3790,  3799,
    3805,  3809,  3815,  3822,  3828,  3834,  3838,  3856,  3862,  3868,
    3874,  3882,  3886,  3892,  3898,  3907,  3919,  3920,  3923,  3924,
    3925,  3928,  3936,  3955,  3963,  3982,  4003,  4026,  4032,  4041,
    4047,  4056,  4063,  4064,  4066,  4071,  4078,  4081,  4087,  4093,
    4099,  4106,  4115,  4118,  4124,  4130,  4136,  4143,  4152,  4156,
    4162,  4166,  4172,  4176,  4180,  4185,  4192,  4196,  4200,  4204,
    4210,  4215,  4221,  4227,  4234,  4240,  4245,  4250,  4255,  4260,
    4265,  4270,  4273,  4279,  4283,  4287,  4293,  4297,  4301,  4307,
    4313,  4319,  4323,  4329,  4333,  4339,  4345,  4349,  4353,  4359,
    4365,  4371,  4379,  4383,  4387,  4391,  4395,  4399,  4403,  4409,
    4415,  4423,  4429,  4435,  4439,  4443,  4447,  4451,  4455,  4459,
    4463,  4467,  4471,  4477,  4483,  4489,  4493,  4497,  4503,  4508,
    4514,  4520,  4524,  4529,  4535,  4539,  4545,  4551,  4555,  4563,
    4569,  4573,  4579,  4588,  4597,  4603,  4609,  4617,  4623,  4629,
    4635,  4641,  4645,  4649,  4655,  4663,  4669,  4673,  4681,  4687,
    4693,  4699,  4707,  4711,  4717,  4723,  4727,  4733,  4737,  4743,
    4749,  4756,  4764,  4769,  4775,  4781,  4789,  4795,  4802,  4808,
    4817,  4823,  4829,  4835,  4844,  4851,  4857,  4863,  4872,  4879,
    4883,  4889,  4894,  4901,  4905,  4911,  4915,  4921,  4925,  4931,
    4935,  4943,  4954,  4957,  4963,  4966,  4972,  4978,  4986,  4990,
    4996,  4999,  5005,  5015,  5018,  5024,  5034,  5040,  5048,  5051,
    5057,  5061,  5065,  5071,  5074,  5080,  5083,  5089,  5093,  5097,
    5101,  5105,  5111,  5117,  5127,  5137,  5141,  5145,  5151,  5157,
    5165,  5169,  5173,  5177,  5181,  5185,  5189,  5193,  5199,  5203,
    5207,  5211,  5217,  5223,  5231,  5239,  5243,  5249,  5259,  5270,
    5276,  5280,  5286,  5289,  5295,  5300,  5307,  5315,  5318,  5324,
    5328,  5334,  5343,  5350,  5358,  5362,  5368,  5374,  5382,  5385,
    5391,  5396,  5406,  5414,  5424,  5430,  5434,  5440,  5444,  5448,
    5454,  5460,  5464,  5468,  5472,  5478,  5486,  5496,  5500,  5506,
    5512,  5516,  5522,  5526,  5532,  5536,  5542,  5549,  5555,  5564,
    5570,  5580,  5586,  5592,  5610,  5635,  5655,  5659,  5663,  5669,
    5672,  5673,  5674,  5675,  5676,  5677,  5678,  5679,  5680,  5681,
    5682,  5683,  5684,  5685,  5686,  5687,  5688,  5689,  5690,  5691,
    5692,  5693,  5694,  5697,  5698,  5699,  5700,  5701,  5702,  5703,
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
    5844,  5845,  5846,  5847,  5848,  5849,  5850,  5851,  5852,  5853,
    5854,  5855,  5856,  5857,  5858,  5859,  5860,  5861,  5862,  5863,
    5864,  5865,  5866,  5867,  5868,  5869,  5870,  5871,  5872,  5873,
    5874,  5875,  5876,  5877,  5878,  5879,  5880,  5881,  5882,  5883,
    5884,  5885,  5886,  5887,  5888,  5889,  5890,  5891,  5892,  5893,
    5894,  5895,  5896,  5897,  5900
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
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335
    };
    // Last valid token kind.
    const int code_max = 590;

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
