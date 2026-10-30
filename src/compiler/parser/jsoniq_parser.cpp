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

#define JSONIQ_PARSER


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




#include "jsoniq_parser.hpp"

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
  else if (lObjectLookup != NULL)
  {
    *aTargetExpr = lObjectLookup->get_object_expr();
    lObjectLookup->release_object_expr();
    *aSelectorExpr = lObjectLookup->get_selector_expr();
    lObjectLookup->release_selector_expr();
    return true;
  }
  else if (!allowArray && lFilterExpr != NULL)
  {
    *anError = "An object lookup is expected.";
    return false;
  }
  else if (allowArray && lFilterExpr != NULL)
  {
    rchandle<exprnode> lPrimary = lFilterExpr->get_primary();
    rchandle<PredicateList> lPredicateList = lFilterExpr->get_pred_list();
    ulong lSize = lPredicateList->size();
    if (lSize < 1)
    {
      *anError = "An object or array lookup with exactly one argument is expected. No argument was found.";
      return false;
    }
    rchandle<JSONArrayConstructor> lConstructor =
        dynamic_cast<JSONArrayConstructor*>(lPredicateList->operator[](lSize - 1).getp());
    if (lConstructor == NULL)
    {
      *anError = "An object or array lookup is expected.";
      return false;
    }
    *aSelectorExpr = lConstructor->get_expr();
    if (*aSelectorExpr == NULL)
    {
      *anError = "An object or array lookup with exactly one argument is expected. No argument was found.";
      return false;
    }
    lConstructor->set_expr(NULL);
    *aTargetExpr = lPrimary;
    if (lSize > 1)
    {
      rchandle<PredicateList> lNewPredicateList =
          new PredicateList(lPredicateList->get_location());
      for (ulong i = 0; i < lSize - 1; ++i)
      {
        lNewPredicateList->push_back(lPredicateList->operator[](i));
      }
      *aTargetExpr = new FilterExpr(
          lFilterExpr->get_location(),
          lPrimary,
          lNewPredicateList);
    }
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
  jsoniq_parser::jsoniq_parser (xquery_driver& driver_yyarg)
#if YYDEBUG
    : yydebug_ (false),
      yycdebug_ (&std::cerr),
#else
    :
#endif
      driver (driver_yyarg)
  {}

  jsoniq_parser::~jsoniq_parser ()
  {}

  jsoniq_parser::syntax_error::~syntax_error () YY_NOEXCEPT YY_NOTHROW
  {}

  /*---------.
  | symbol.  |
  `---------*/

  // basic_symbol.
  template <typename Base>
  jsoniq_parser::basic_symbol<Base>::basic_symbol (const basic_symbol& that)
    : Base (that)
    , value (that.value)
    , location (that.location)
  {}


  /// Constructor for valueless symbols.
  template <typename Base>
  jsoniq_parser::basic_symbol<Base>::basic_symbol (typename Base::kind_type t, YY_MOVE_REF (location_type) l)
    : Base (t)
    , value ()
    , location (l)
  {}

  template <typename Base>
  jsoniq_parser::basic_symbol<Base>::basic_symbol (typename Base::kind_type t, YY_RVREF (value_type) v, YY_RVREF (location_type) l)
    : Base (t)
    , value (YY_MOVE (v))
    , location (YY_MOVE (l))
  {}


  template <typename Base>
  jsoniq_parser::symbol_kind_type
  jsoniq_parser::basic_symbol<Base>::type_get () const YY_NOEXCEPT
  {
    return this->kind ();
  }


  template <typename Base>
  bool
  jsoniq_parser::basic_symbol<Base>::empty () const YY_NOEXCEPT
  {
    return this->kind () == symbol_kind::S_YYEMPTY;
  }

  template <typename Base>
  void
  jsoniq_parser::basic_symbol<Base>::move (basic_symbol& s)
  {
    super_type::move (s);
    value = YY_MOVE (s.value);
    location = YY_MOVE (s.location);
  }

  // by_kind.
  jsoniq_parser::by_kind::by_kind () YY_NOEXCEPT
    : kind_ (symbol_kind::S_YYEMPTY)
  {}

#if 201103L <= YY_CPLUSPLUS
  jsoniq_parser::by_kind::by_kind (by_kind&& that) YY_NOEXCEPT
    : kind_ (that.kind_)
  {
    that.clear ();
  }
#endif

  jsoniq_parser::by_kind::by_kind (const by_kind& that) YY_NOEXCEPT
    : kind_ (that.kind_)
  {}

  jsoniq_parser::by_kind::by_kind (token_kind_type t) YY_NOEXCEPT
    : kind_ (yytranslate_ (t))
  {}



  void
  jsoniq_parser::by_kind::clear () YY_NOEXCEPT
  {
    kind_ = symbol_kind::S_YYEMPTY;
  }

  void
  jsoniq_parser::by_kind::move (by_kind& that)
  {
    kind_ = that.kind_;
    that.clear ();
  }

  jsoniq_parser::symbol_kind_type
  jsoniq_parser::by_kind::kind () const YY_NOEXCEPT
  {
    return kind_;
  }


  jsoniq_parser::symbol_kind_type
  jsoniq_parser::by_kind::type_get () const YY_NOEXCEPT
  {
    return this->kind ();
  }



  // by_state.
  jsoniq_parser::by_state::by_state () YY_NOEXCEPT
    : state (empty_state)
  {}

  jsoniq_parser::by_state::by_state (const by_state& that) YY_NOEXCEPT
    : state (that.state)
  {}

  void
  jsoniq_parser::by_state::clear () YY_NOEXCEPT
  {
    state = empty_state;
  }

  void
  jsoniq_parser::by_state::move (by_state& that)
  {
    state = that.state;
    that.clear ();
  }

  jsoniq_parser::by_state::by_state (state_type s) YY_NOEXCEPT
    : state (s)
  {}

  jsoniq_parser::symbol_kind_type
  jsoniq_parser::by_state::kind () const YY_NOEXCEPT
  {
    if (state == empty_state)
      return symbol_kind::S_YYEMPTY;
    else
      return YY_CAST (symbol_kind_type, yystos_[+state]);
  }

  jsoniq_parser::stack_symbol_type::stack_symbol_type ()
  {}

  jsoniq_parser::stack_symbol_type::stack_symbol_type (YY_RVREF (stack_symbol_type) that)
    : super_type (YY_MOVE (that.state), YY_MOVE (that.value), YY_MOVE (that.location))
  {
#if 201103L <= YY_CPLUSPLUS
    // that is emptied.
    that.state = empty_state;
#endif
  }

  jsoniq_parser::stack_symbol_type::stack_symbol_type (state_type s, YY_MOVE_REF (symbol_type) that)
    : super_type (s, YY_MOVE (that.value), YY_MOVE (that.location))
  {
    // that is emptied.
    that.kind_ = symbol_kind::S_YYEMPTY;
  }

#if YY_CPLUSPLUS < 201103L
  jsoniq_parser::stack_symbol_type&
  jsoniq_parser::stack_symbol_type::operator= (const stack_symbol_type& that)
  {
    state = that.state;
    value = that.value;
    location = that.location;
    return *this;
  }

  jsoniq_parser::stack_symbol_type&
  jsoniq_parser::stack_symbol_type::operator= (stack_symbol_type& that)
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
  jsoniq_parser::yy_destroy_ (const char* yymsg, basic_symbol<Base>& yysym) const
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

      case symbol_kind::S_NotExpr: // NotExpr
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
  jsoniq_parser::yy_print_ (std::ostream& yyo, const basic_symbol<Base>& yysym) const
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
  jsoniq_parser::yypush_ (const char* m, YY_MOVE_REF (stack_symbol_type) sym)
  {
    if (m)
      YY_SYMBOL_PRINT (m, sym);
    yystack_.push (YY_MOVE (sym));
  }

  void
  jsoniq_parser::yypush_ (const char* m, state_type s, YY_MOVE_REF (symbol_type) sym)
  {
#if 201103L <= YY_CPLUSPLUS
    yypush_ (m, stack_symbol_type (s, std::move (sym)));
#else
    stack_symbol_type ss (s, sym);
    yypush_ (m, ss);
#endif
  }

  void
  jsoniq_parser::yypop_ (int n) YY_NOEXCEPT
  {
    yystack_.pop (n);
  }

#if YYDEBUG
  std::ostream&
  jsoniq_parser::debug_stream () const
  {
    return *yycdebug_;
  }

  void
  jsoniq_parser::set_debug_stream (std::ostream& o)
  {
    yycdebug_ = &o;
  }


  jsoniq_parser::debug_level_type
  jsoniq_parser::debug_level () const
  {
    return yydebug_;
  }

  void
  jsoniq_parser::set_debug_level (debug_level_type l)
  {
    yydebug_ = l;
  }
#endif // YYDEBUG

  jsoniq_parser::state_type
  jsoniq_parser::yy_lr_goto_state_ (state_type yystate, int yysym)
  {
    int yyr = yypgoto_[yysym - YYNTOKENS] + yystate;
    if (0 <= yyr && yyr <= yylast_ && yycheck_[yyr] == yystate)
      return yytable_[yyr];
    else
      return yydefgoto_[yysym - YYNTOKENS];
  }

  bool
  jsoniq_parser::yy_pact_value_is_default_ (int yyvalue) YY_NOEXCEPT
  {
    return yyvalue == yypact_ninf_;
  }

  bool
  jsoniq_parser::yy_table_value_is_error_ (int yyvalue) YY_NOEXCEPT
  {
    return yyvalue == yytable_ninf_;
  }

  int
  jsoniq_parser::operator() ()
  {
    return parse ();
  }

  int
  jsoniq_parser::parse ()
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

  case 13: // VersionDecl: "'jsoniq'" "'version'" "'STRING'" "';'"
    {
      (yylhs.value.node) = new VersionDecl( LOC(yylhs.location), SYMTAB((yystack_[1].value.sval)), "utf-8", VersionDecl::jsoniq );
    }
    break;

  case 14: // VersionDecl: "'jsoniq'" "'version'" "'STRING'" "'encoding'" "'STRING'" "';'"
    {
      (yylhs.value.node) = new VersionDecl( LOC(yylhs.location), SYMTAB((yystack_[3].value.sval)), SYMTAB((yystack_[1].value.sval)), VersionDecl::jsoniq );
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

  case 188: // BlockExpr: "'{'" StatementsAndOptionalExpr "'}'"
    {
      BlockBody* block = dynamic_cast<BlockBody*>((yystack_[1].value.expr));
      if ((yystack_[1].value.expr) == NULL || (block != NULL && block->isEmpty()))
      {
        driver.addCommonLanguageWarning(yystack_[2].location, ZED(ZWST0009_EMPTY_OBJECT));
        (yylhs.value.expr) = new JSONDirectObjectConstructor(LOC(yylhs.location));
      }
      else
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

  case 189: // EnclosedStatementsAndOptionalExpr: "'{'" StatementsAndOptionalExpr "'}'"
    {
      (yylhs.value.expr) = (yystack_[1].value.expr);
    }
    break;

  case 190: // VarDeclStatement: BlockVarDeclList "';'"
    {
      (yylhs.value.expr) = (yystack_[1].value.expr);
    }
    break;

  case 191: // BlockVarDeclList: BlockVarDeclList "','" BlockVarDecl
    {
      VarDeclStmt* vdecl = static_cast<VarDeclStmt*>((yystack_[2].value.expr));
      vdecl->add((yystack_[0].value.node));
      (yylhs.value.expr) = vdecl;
    }
    break;

  case 192: // BlockVarDeclList: "'variable'" BlockVarDecl
    {
      VarDeclStmt* vdecl = new VarDeclStmt(LOC(yylhs.location), NULL);
      vdecl->add((yystack_[0].value.node));
      (yylhs.value.expr) = vdecl;
    }
    break;

  case 193: // BlockVarDeclList: AnnotationList "'variable'" BlockVarDecl
    {
      VarDeclStmt* vdecl = new VarDeclStmt(LOC(yylhs.location),
                                           static_cast<AnnotationListParsenode*>((yystack_[2].value.node)));
      vdecl->add((yystack_[0].value.node));
      (yylhs.value.expr) = vdecl;
    }
    break;

  case 194: // BlockVarDecl: "'$'" QNAME
    {
      LocalVarDecl* vd = new LocalVarDecl(LOC(yylhs.location),
                                          static_cast<QName*>((yystack_[0].value.expr)),
                                          NULL,
                                          NULL,
                                          NULL);
      (yylhs.value.node) = vd;
    }
    break;

  case 195: // BlockVarDecl: "'$'" QNAME TypeDeclaration
    {
      LocalVarDecl* vd = new LocalVarDecl(LOC(yylhs.location),
                                          static_cast<QName*>((yystack_[1].value.expr)),
                                          dynamic_cast<SequenceTypeAST*>((yystack_[0].value.node)),
                                          NULL,
                                          NULL);
      (yylhs.value.node) = vd;
    }
    break;

  case 196: // BlockVarDecl: "'$'" QNAME "':='" ExprSingle
    {
      LocalVarDecl* vd = new LocalVarDecl(LOC(yylhs.location),
                                          static_cast<QName*>((yystack_[2].value.expr)),
                                          NULL,
                                          (yystack_[0].value.expr),
                                          NULL);
      (yylhs.value.node) = vd;
    }
    break;

  case 197: // BlockVarDecl: "'$'" QNAME TypeDeclaration "':='" ExprSingle
    {
      LocalVarDecl* vd = new LocalVarDecl(LOC(yylhs.location),
                                          static_cast<QName*>((yystack_[3].value.expr)),
                                          dynamic_cast<SequenceTypeAST*>((yystack_[2].value.node)),
                                          (yystack_[0].value.expr),
                                          NULL);
      (yylhs.value.node) = vd;
    }
    break;

  case 198: // AssignStatement: "'$'" QNAME "':='" ExprSingle "';'"
    {
      (yylhs.value.expr) = new AssignExpr(LOC(yylhs.location), static_cast<QName*>((yystack_[3].value.expr)), (yystack_[1].value.expr));
    }
    break;

  case 199: // ApplyStatement: ExprSimple "';'"
    {
      (yylhs.value.expr) = new ApplyExpr(LOC(yylhs.location), (yystack_[1].value.expr));
    }
    break;

  case 200: // ExitStatement: "'exit'" "'returning'" ExprSingle "';'"
    {
      (yylhs.value.expr) = new ExitExpr(LOC(yylhs.location), (yystack_[1].value.expr));
    }
    break;

  case 201: // WhileStatement: "'while'" "'('" Expr "')'" Statement
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

  case 202: // FlowCtlStatement: "'break'" "'loop'" "';'"
    {
      (yylhs.value.expr) = new FlowCtlStatement(LOC(yylhs.location), FlowCtlStatement::BREAK);
    }
    break;

  case 203: // FlowCtlStatement: "'continue'" "'loop'" "';'"
    {
      (yylhs.value.expr) = new FlowCtlStatement( LOC(yylhs.location), FlowCtlStatement::CONTINUE );
    }
    break;

  case 204: // FLWORStatement: FLWORClauseList ReturnStatement
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

  case 205: // ReturnStatement: ReturnOrSelect Statement
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

  case 206: // IfStatement: "'if'" "'('" Expr "')'" "'then'" Statement "'else'" Statement
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

  case 207: // TryStatement: "'try'" BlockStatement CatchListStatement
    {
      (yylhs.value.expr) = new TryExpr(LOC(yylhs.location), (yystack_[1].value.expr), (yystack_[0].value.expr));
    }
    break;

  case 208: // CatchListStatement: CatchStatement
    {
      CatchListExpr* cle = new CatchListExpr( LOC(yylhs.location) );
      cle->push_back( static_cast<CatchExpr*>((yystack_[0].value.expr)) );
      (yylhs.value.expr) = cle;
    }
    break;

  case 209: // CatchListStatement: CatchListStatement CatchStatement
    {
      CatchListExpr *cle = dynamic_cast<CatchListExpr*>((yystack_[1].value.expr));
      if ( cle )
        cle->push_back( static_cast<CatchExpr*>((yystack_[0].value.expr)) );
      (yylhs.value.expr) = (yystack_[1].value.expr);
    }
    break;

  case 210: // CatchStatement: "'catch'" NameTestList BlockStatement
    {
       (yylhs.value.expr) = new CatchExpr(LOC(yylhs.location), *(yystack_[1].value.name_test_list), (yystack_[0].value.expr));
       delete (yystack_[1].value.name_test_list);
    }
    break;

  case 211: // VoidStatement: "';'"
    {
      (yylhs.value.expr) = new BlockBody(LOC(yylhs.location));
    }
    break;

  case 212: // Expr: ExprSingle
    {
      (yylhs.value.expr) = (yystack_[0].value.expr);
    }
    break;

  case 213: // Expr: Expr "','" ExprSingle
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

  case 214: // Expr: Expr ERROR ExprSingle
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

  case 215: // ExprSingle: FLWORExpr
        { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 216: // ExprSingle: SwitchExpr
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 217: // ExprSingle: TypeswitchExpr
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 218: // ExprSingle: IfExpr
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 219: // ExprSingle: TryExpr
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 220: // ExprSingle: ExprSimple
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 221: // ExprSimple: OrExpr
        { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 222: // ExprSimple: QuantifiedExpr
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 223: // ExprSimple: InsertExpr
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 224: // ExprSimple: DeleteExpr
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 225: // ExprSimple: RenameExpr
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 226: // ExprSimple: ReplaceExpr
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 227: // ExprSimple: TransformExpr
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 228: // ExprSimple: JSONDeleteExpr
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 229: // ExprSimple: JSONInsertExpr
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 230: // ExprSimple: JSONRenameExpr
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 231: // ExprSimple: JSONReplaceExpr
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 232: // ExprSimple: JSONAppendExpr
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 233: // FLWORExpr: FLWORClauseList ReturnExpr
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

  case 234: // ReturnExpr: ReturnOrSelect ExprSingle
    {
      (yylhs.value.expr) = new ReturnExpr( LOC(yylhs.location), (yystack_[0].value.expr) );
    }
    break;

  case 236: // ReturnOrSelect: "'select'"
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

  case 259: // ForClause: "'from'" error VarInDeclList
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

  case 262: // ForOrFrom: "'from'"
    {
    }
    break;

  case 263: // VarInDeclList: VarInDecl
    {
      VarInDeclList* vdl = new VarInDeclList( LOC(yylhs.location) );
      vdl->push_back( dynamic_cast<VarInDecl*>((yystack_[0].value.node)) );
      (yylhs.value.node) = vdl;
    }
    break;

  case 264: // VarInDeclList: VarInDeclList "','" "'$'" VarInDecl
    {
      if ( VarInDeclList* vdl = dynamic_cast<VarInDeclList*>((yystack_[3].value.node)) )
        vdl->push_back( dynamic_cast<VarInDecl*>((yystack_[0].value.node)) );
      (yylhs.value.node) = (yystack_[3].value.node);
    }
    break;

  case 265: // VarInDeclList: VarInDeclList "','" VarInDecl
    {
      (yylhs.value.node) = (yystack_[2].value.node);
      error(yystack_[0].location, "syntax error, unexpected QName \""
          + static_cast<VarInDecl*>((yystack_[0].value.node))->get_var_name()->get_qname().str() + "\" (missing \"$\" sign?)");
      delete (yystack_[2].value.node);
      release_hack( (yystack_[0].value.node) );
      YYERROR;
    }
    break;

  case 266: // VarInDecl: QNAME "'in'" ExprSingle
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

  case 267: // VarInDecl: QNAME "'allowing'" "'empty'" "'in'" ExprSingle
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

  case 268: // VarInDecl: QNAME TypeDeclaration "'in'" ExprSingle
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

  case 269: // VarInDecl: QNAME TypeDeclaration "'allowing'" "'empty'" "'in'" ExprSingle
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

  case 270: // VarInDecl: QNAME PositionalVar "'in'" ExprSingle
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

  case 271: // VarInDecl: QNAME "'allowing'" "'empty'" PositionalVar "'in'" ExprSingle
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

  case 272: // VarInDecl: QNAME TypeDeclaration PositionalVar "'in'" ExprSingle
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

  case 273: // VarInDecl: QNAME TypeDeclaration "'allowing'" "'empty'" PositionalVar "'in'" ExprSingle
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

  case 274: // VarInDecl: QNAME FTScoreVar "'in'" ExprSingle
    {
      (yylhs.value.node) = new VarInDecl(LOC(yylhs.location),
                         static_cast<QName*>((yystack_[3].value.expr)),
                         NULL,NULL,
                         dynamic_cast<FTScoreVar*>((yystack_[2].value.node)),
                         (yystack_[0].value.expr),
                         false);
    }
    break;

  case 275: // VarInDecl: QNAME TypeDeclaration FTScoreVar "'in'" ExprSingle
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

  case 276: // VarInDecl: QNAME PositionalVar FTScoreVar "'in'" ExprSingle
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

  case 277: // VarInDecl: QNAME TypeDeclaration PositionalVar FTScoreVar "'in'" ExprSingle
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

  case 278: // PositionalVar: "'at'" "'$'" QNAME
    {
      (yylhs.value.node) = new PositionalVar(LOC(yylhs.location), static_cast<QName*>((yystack_[0].value.expr)));
    }
    break;

  case 279: // FTScoreVar: "'score'" "'$'" QNAME
    {
      (yylhs.value.node) = new FTScoreVar(LOC(yylhs.location), static_cast<QName*>((yystack_[0].value.expr)));
    }
    break;

  case 280: // LetClause: "'let'" VarGetsDeclList
        {
            (yylhs.value.node) = new LetClause( LOC(yylhs.location), dynamic_cast<VarGetsDeclList*>((yystack_[0].value.node)) );
        }
    break;

  case 281: // VarGetsDeclList: VarGetsDecl
        {
            VarGetsDeclList *vgdl = new VarGetsDeclList( LOC(yylhs.location) );
            vgdl->push_back( dynamic_cast<VarGetsDecl*>((yystack_[0].value.node)) );
            (yylhs.value.node) = vgdl;
        }
    break;

  case 282: // VarGetsDeclList: VarGetsDeclList "','" VarGetsDecl
        {
            if( VarGetsDeclList *vgdl = dynamic_cast<VarGetsDeclList*>((yystack_[2].value.node)) )
                vgdl->push_back( dynamic_cast<VarGetsDecl*>((yystack_[0].value.node)) );
            (yylhs.value.node) = (yystack_[2].value.node);
        }
    break;

  case 283: // VarGetsDecl: "'$'" QNAME "':='" ExprSingle
    {
      (yylhs.value.node) = new VarGetsDecl(LOC (yylhs.location),
                           static_cast<QName*>((yystack_[2].value.expr)),
                           NULL,
                           NULL,
                           (yystack_[0].value.expr));
    }
    break;

  case 284: // VarGetsDecl: "'$'" QNAME TypeDeclaration "':='" ExprSingle
    {
      (yylhs.value.node) = new VarGetsDecl(LOC (yylhs.location),
                           static_cast<QName*>((yystack_[3].value.expr)),
                           dynamic_cast<SequenceTypeAST *>((yystack_[2].value.node)),
                           NULL,
                           (yystack_[0].value.expr));
    }
    break;

  case 285: // VarGetsDecl: FTScoreVar "':='" ExprSingle
    {
      (yylhs.value.node) = new VarGetsDecl(LOC (yylhs.location),
                           dynamic_cast<FTScoreVar*>((yystack_[2].value.node))->get_var_name(),
                           NULL,
                           dynamic_cast<FTScoreVar*>((yystack_[2].value.node)),
                           (yystack_[0].value.expr));
     }
    break;

  case 286: // VarGetsDecl: "'$'" QNAME TypeDeclaration FTScoreVar "':='" ExprSingle
    {
      (yylhs.value.node) = new VarGetsDecl(LOC (yylhs.location),
                           static_cast<QName*>((yystack_[4].value.expr)),
                           dynamic_cast<SequenceTypeAST *>((yystack_[3].value.node)),
                           dynamic_cast<FTScoreVar*>((yystack_[2].value.node)),
                           (yystack_[0].value.expr));
    }
    break;

  case 287: // WindowVarDecl: "'$'" QNAME "'in'" ExprSingle
    {
      (yylhs.value.node) = new WindowVarDecl(LOC (yylhs.location),
                             static_cast<QName*>((yystack_[2].value.expr)),
                             NULL, (yystack_[0].value.expr));
    }
    break;

  case 288: // WindowVarDecl: "'$'" QNAME TypeDeclaration "'in'" ExprSingle
    {
      (yylhs.value.node) = new WindowVarDecl(LOC (yylhs.location),
                             static_cast<QName*>((yystack_[3].value.expr)),
                             dynamic_cast<SequenceTypeAST *>((yystack_[2].value.node)),
                             (yystack_[0].value.expr));
    }
    break;

  case 289: // WindowVars: WindowVars3
    { (yylhs.value.node) = (yystack_[0].value.node); }
    break;

  case 290: // WindowVars: "'$'" QNAME
    {
      (yylhs.value.node) = new WindowVars(LOC(yylhs.location), NULL, static_cast<QName*>((yystack_[0].value.expr)), NULL, NULL);
    }
    break;

  case 291: // WindowVars: "'$'" QNAME WindowVars3
    {
      (yylhs.value.node) = (yystack_[0].value.node);
      dynamic_cast<WindowVars *>((yylhs.value.node))->set_curr(static_cast<QName*>((yystack_[1].value.expr)));
    }
    break;

  case 292: // WindowVars3: PositionalVar
    {
      (yylhs.value.node) = new WindowVars(LOC(yylhs.location), dynamic_cast<PositionalVar*>((yystack_[0].value.node)), NULL, NULL, NULL);
    }
    break;

  case 293: // WindowVars3: PositionalVar WindowVars2
    {
      (yylhs.value.node) = (yystack_[0].value.node);
      dynamic_cast<WindowVars *>((yylhs.value.node))->set_posvar(dynamic_cast<PositionalVar*>((yystack_[1].value.node)));
    }
    break;

  case 294: // WindowVars3: WindowVars2
    { (yylhs.value.node) = (yystack_[0].value.node); }
    break;

  case 295: // WindowVars2: "'previous'" "'$'" QNAME "'next'" "'$'" QNAME
    {
      (yylhs.value.node) = new WindowVars(LOC(yylhs.location), NULL, NULL, static_cast<QName*>((yystack_[3].value.expr)), static_cast<QName*>((yystack_[0].value.expr)));
    }
    break;

  case 296: // WindowVars2: "'next'" "'$'" QNAME
    {
      (yylhs.value.node) = new WindowVars(LOC(yylhs.location), NULL, NULL, NULL, static_cast<QName*>((yystack_[0].value.expr)));
    }
    break;

  case 297: // WindowVars2: "'previous'" "'$'" QNAME
    {
      (yylhs.value.node) = new WindowVars(LOC(yylhs.location), NULL, NULL, static_cast<QName*>((yystack_[0].value.expr)), NULL);
    }
    break;

  case 298: // WhereClause: "'where'" ExprSingle
    {
      (yylhs.value.node) = new WhereClause(LOC (yylhs.location), (yystack_[0].value.expr));
    }
    break;

  case 299: // GroupByClause: "'group'" "'by'" GroupSpecList
    {
      (yylhs.value.node) = new GroupByClause(LOC(yylhs.location), dynamic_cast<GroupSpecList*>((yystack_[0].value.node)));
    }
    break;

  case 300: // GroupSpecList: GroupSpec
    {
      GroupSpecList* gsl = new GroupSpecList(LOC(yylhs.location));
      gsl->push_back(static_cast<GroupSpec*>((yystack_[0].value.node)));
      (yylhs.value.node) = gsl;
    }
    break;

  case 301: // GroupSpecList: GroupSpecList "','" GroupSpec
    {
      GroupSpecList* gsl = static_cast<GroupSpecList*>((yystack_[2].value.node));
      gsl->push_back(static_cast<GroupSpec*>((yystack_[0].value.node)));
      (yylhs.value.node) = gsl;
    }
    break;

  case 302: // GroupSpec: "'$'" QNAME "':='" ExprSingle
    {
      (yylhs.value.node) = new GroupSpec(LOC(yylhs.location), static_cast<QName*>((yystack_[2].value.expr)), NULL, (yystack_[0].value.expr), NULL);
    }
    break;

  case 303: // GroupSpec: "'$'" QNAME TypeDeclaration "':='" ExprSingle
    {
      (yylhs.value.node) = new GroupSpec(LOC(yylhs.location),
                         static_cast<QName*>((yystack_[3].value.expr)),
                         static_cast<SequenceTypeAST*>((yystack_[2].value.node)),
                         (yystack_[0].value.expr),
                         NULL);
    }
    break;

  case 304: // GroupSpec: "'$'" QNAME TypeDeclaration "':='" ExprSingle GroupCollationSpec
    {
      (yylhs.value.node) = new GroupSpec(LOC(yylhs.location),
                         static_cast<QName*>((yystack_[4].value.expr)),
                         static_cast<SequenceTypeAST*>((yystack_[3].value.node)),
                         (yystack_[1].value.expr),
                         static_cast<GroupCollationSpec*>((yystack_[0].value.node)));
    }
    break;

  case 305: // GroupSpec: "'$'" QNAME "':='" ExprSingle GroupCollationSpec
    {
      (yylhs.value.node) = new GroupSpec(LOC(yylhs.location),
                         static_cast<QName*>((yystack_[3].value.expr)),
                         NULL,
                         (yystack_[1].value.expr),
                         static_cast<GroupCollationSpec*>((yystack_[0].value.node)));
    }
    break;

  case 306: // GroupSpec: ExprSingle
    {
      VarRef* varRef = dynamic_cast<VarRef*>((yystack_[0].value.expr));
      if (varRef != NULL)
        (yylhs.value.node) = new GroupSpec(LOC(yylhs.location), varRef, NULL, NULL, NULL);
      else
        (yylhs.value.node) = new GroupSpec(LOC(yylhs.location), NULL, NULL, (yystack_[0].value.expr), NULL);
    }
    break;

  case 307: // GroupSpec: ExprSingle GroupCollationSpec
    {
      VarRef* varRef = dynamic_cast<VarRef*>((yystack_[1].value.expr));
      if (varRef != NULL)
        (yylhs.value.node) = new GroupSpec(LOC(yylhs.location), varRef, NULL, NULL, static_cast<GroupCollationSpec*>((yystack_[0].value.node)));
      else
        (yylhs.value.node) = new GroupSpec(LOC(yylhs.location), NULL, NULL, (yystack_[1].value.expr), static_cast<GroupCollationSpec*>((yystack_[0].value.node)));
    }
    break;

  case 308: // GroupCollationSpec: "'collation'" URI_LITERAL
    {
      (yylhs.value.node) = new GroupCollationSpec( LOC(yylhs.location), SYMTAB((yystack_[0].value.sval)) );
    }
    break;

  case 309: // OrderByClause: "'order'" "'by'" OrderSpecList
        {
            (yylhs.value.node) = new OrderByClause(
                LOC(yylhs.location), dynamic_cast<OrderSpecList*>((yystack_[0].value.node))
            );
        }
    break;

  case 310: // OrderByClause: "'stable'" "'order'" "'by'" OrderSpecList
        {
            (yylhs.value.node) = new OrderByClause(
                LOC(yylhs.location), dynamic_cast<OrderSpecList*>((yystack_[0].value.node)), true
            );
        }
    break;

  case 311: // OrderSpecList: OrderSpec
        {
            OrderSpecList *osl = new OrderSpecList( LOC(yylhs.location) );
            osl->push_back( dynamic_cast<OrderSpec*>((yystack_[0].value.node)) );
            (yylhs.value.node) = osl;
        }
    break;

  case 312: // OrderSpecList: OrderSpecList "','" OrderSpec
        {
            if ( OrderSpecList* osl = dynamic_cast<OrderSpecList*>((yystack_[2].value.node)) )
                osl->push_back( dynamic_cast<OrderSpec*>((yystack_[0].value.node)) );
            (yylhs.value.node) = (yystack_[2].value.node);
        }
    break;

  case 313: // OrderSpec: ExprSingle
        {
            (yylhs.value.node) = new OrderSpec( LOC(yylhs.location), (yystack_[0].value.expr), NULL );
        }
    break;

  case 314: // OrderSpec: ExprSingle OrderModifier
        {
            (yylhs.value.node) = new OrderSpec(
                LOC(yylhs.location), (yystack_[1].value.expr), dynamic_cast<OrderModifierPN*>((yystack_[0].value.node))
            );
        }
    break;

  case 315: // OrderModifier: OrderDirSpec
        {
            (yylhs.value.node) = new OrderModifierPN(
                LOC(yylhs.location), dynamic_cast<OrderDirSpec*>((yystack_[0].value.node)), NULL, NULL
            );
        }
    break;

  case 316: // OrderModifier: OrderEmptySpec
        {
            (yylhs.value.node) = new OrderModifierPN(
                LOC(yylhs.location), NULL, dynamic_cast<OrderEmptySpec*>((yystack_[0].value.node)), NULL
            );
        }
    break;

  case 317: // OrderModifier: OrderCollationSpec
        {
            (yylhs.value.node) = new OrderModifierPN(
                LOC(yylhs.location), NULL, NULL, dynamic_cast<OrderCollationSpec*>((yystack_[0].value.node))
            );
        }
    break;

  case 318: // OrderModifier: OrderDirSpec OrderEmptySpec
        {
            (yylhs.value.node) = new OrderModifierPN(
                LOC(yylhs.location),
                dynamic_cast<OrderDirSpec*>((yystack_[1].value.node)),
                dynamic_cast<OrderEmptySpec*>((yystack_[0].value.node)),
                NULL
            );
        }
    break;

  case 319: // OrderModifier: OrderDirSpec OrderCollationSpec
        {
            (yylhs.value.node) = new OrderModifierPN(
                LOC(yylhs.location),
                dynamic_cast<OrderDirSpec*>((yystack_[1].value.node)),
                NULL,
                dynamic_cast<OrderCollationSpec*>((yystack_[0].value.node))
            );
        }
    break;

  case 320: // OrderModifier: OrderEmptySpec OrderCollationSpec
        {
            (yylhs.value.node) = new OrderModifierPN(
                LOC(yylhs.location),
                NULL,
                dynamic_cast<OrderEmptySpec*>((yystack_[1].value.node)),
                dynamic_cast<OrderCollationSpec*>((yystack_[0].value.node))
            );
        }
    break;

  case 321: // OrderModifier: OrderDirSpec OrderEmptySpec OrderCollationSpec
        {
            (yylhs.value.node) = new OrderModifierPN(
                LOC(yylhs.location),
                dynamic_cast<OrderDirSpec*>((yystack_[2].value.node)),
                dynamic_cast<OrderEmptySpec*>((yystack_[1].value.node)),
                dynamic_cast<OrderCollationSpec*>((yystack_[0].value.node))
            );
        }
    break;

  case 322: // OrderDirSpec: "'ascending'"
        {
            (yylhs.value.node) = new OrderDirSpec( LOC(yylhs.location), ParseConstants::dir_ascending );
        }
    break;

  case 323: // OrderDirSpec: "'descending'"
        {
            (yylhs.value.node) = new OrderDirSpec( LOC(yylhs.location), ParseConstants::dir_descending );
        }
    break;

  case 324: // OrderEmptySpec: "'empty'" "'greatest'"
        {
            (yylhs.value.node) = new OrderEmptySpec(
                LOC(yylhs.location), StaticContextConsts::empty_greatest
            );
        }
    break;

  case 325: // OrderEmptySpec: "'empty'" "'least'"
        {
            (yylhs.value.node) = new OrderEmptySpec(
                LOC(yylhs.location), StaticContextConsts::empty_least
            );
        }
    break;

  case 326: // OrderCollationSpec: "'collation'" URI_LITERAL
        {
            (yylhs.value.node) = new OrderCollationSpec( LOC(yylhs.location), SYMTAB((yystack_[0].value.sval)) );
        }
    break;

  case 327: // QuantifiedExpr: "'some'" "'$'" QVarInDeclList "'satisfies'" ExprSingle
        {
            (yylhs.value.expr) = new QuantifiedExpr(
                LOC(yylhs.location),
                ParseConstants::quant_some,
                dynamic_cast<QVarInDeclList*>((yystack_[2].value.node)),
                (yystack_[0].value.expr)
            );
        }
    break;

  case 328: // QuantifiedExpr: "'every'" "'$'" QVarInDeclList "'satisfies'" ExprSingle
        {
            (yylhs.value.expr) = new QuantifiedExpr(
                LOC(yylhs.location),
                ParseConstants::quant_every,
                dynamic_cast<QVarInDeclList*>((yystack_[2].value.node)),
                (yystack_[0].value.expr)
            );
        }
    break;

  case 329: // QVarInDeclList: QVarInDecl
    {
      QVarInDeclList *qvidl = new QVarInDeclList( LOC(yylhs.location) );
      qvidl->push_back( dynamic_cast<QVarInDecl*>((yystack_[0].value.node)) );
      (yylhs.value.node) = qvidl;
    }
    break;

  case 330: // QVarInDeclList: QVarInDeclList "','" "'$'" QVarInDecl
    {
      QVarInDeclList *qvidl = dynamic_cast<QVarInDeclList*>((yystack_[3].value.node));
      qvidl->push_back( dynamic_cast<QVarInDecl*>((yystack_[0].value.node)) );
      (yylhs.value.node) = (yystack_[3].value.node);
    }
    break;

  case 331: // QVarInDecl: QNAME "'in'" ExprSingle
    {
      (yylhs.value.node) = new QVarInDecl(LOC(yylhs.location), static_cast<QName*>((yystack_[2].value.expr)), (yystack_[0].value.expr));
    }
    break;

  case 332: // QVarInDecl: QNAME TypeDeclaration "'in'" ExprSingle
    {
      (yylhs.value.node) = new QVarInDecl(LOC(yylhs.location),
                          static_cast<QName*>((yystack_[3].value.expr)),
                          dynamic_cast<SequenceTypeAST *>((yystack_[2].value.node)),
                          (yystack_[0].value.expr));
    }
    break;

  case 333: // SwitchExpr: "'switch'" "'('" Expr "')'" SwitchCaseClauseList "'default'" ReturnOrSelect ExprSingle
    {
      (yylhs.value.expr) = new SwitchExpr(LOC(yylhs.location), (yystack_[5].value.expr), static_cast<SwitchCaseClauseList*>((yystack_[3].value.node)), (yystack_[0].value.expr));
    }
    break;

  case 334: // SwitchCaseClauseList: SwitchCaseClause
    {
      SwitchCaseClauseList* scc_list_p = new SwitchCaseClauseList(LOC(yylhs.location));
      scc_list_p->push_back(static_cast<SwitchCaseClause*>((yystack_[0].value.node)));
      (yylhs.value.node) = scc_list_p;
    }
    break;

  case 335: // SwitchCaseClauseList: SwitchCaseClauseList SwitchCaseClause
    {
      SwitchCaseClauseList* scc_list_p = static_cast<SwitchCaseClauseList*>((yystack_[1].value.node));
      scc_list_p->push_back(static_cast<SwitchCaseClause*>((yystack_[0].value.node)));
      (yylhs.value.node) = (yystack_[1].value.node);
    }
    break;

  case 336: // SwitchCaseClause: SwitchCaseOperandList ReturnOrSelect ExprSingle
    {
      (yylhs.value.node) = new SwitchCaseClause(LOC(yylhs.location), dynamic_cast<SwitchCaseOperandList*>((yystack_[2].value.node)), (yystack_[0].value.expr));
    }
    break;

  case 337: // SwitchCaseOperandList: "'case'" ExprSingle
    {
      SwitchCaseOperandList* sco_list_p = new SwitchCaseOperandList(LOC(yylhs.location));
      sco_list_p->push_back((yystack_[0].value.expr));
      (yylhs.value.node) = sco_list_p;
    }
    break;

  case 338: // SwitchCaseOperandList: SwitchCaseOperandList "'case'" ExprSingle
    {
      SwitchCaseOperandList* sco_list_p = static_cast<SwitchCaseOperandList*>((yystack_[2].value.node));
      sco_list_p->push_back((yystack_[0].value.expr));
      (yylhs.value.node) = (yystack_[2].value.node);
    }
    break;

  case 339: // SwitchStatement: "'switch'" "'('" Expr "')'" SwitchCaseStatementList "'default'" ReturnOrSelect Statement
    {
      (yylhs.value.expr) = new SwitchExpr(LOC(yylhs.location), (yystack_[5].value.expr), static_cast<SwitchCaseClauseList*>((yystack_[3].value.node)), (yystack_[0].value.expr));
    }
    break;

  case 340: // SwitchCaseStatementList: SwitchCaseStatement
    {
      SwitchCaseClauseList* scc_list_p = new SwitchCaseClauseList(LOC(yylhs.location));
      scc_list_p->push_back(static_cast<SwitchCaseClause*>((yystack_[0].value.node)));
      (yylhs.value.node) = scc_list_p;
    }
    break;

  case 341: // SwitchCaseStatementList: SwitchCaseStatementList SwitchCaseStatement
    {
      SwitchCaseClauseList* scc_list_p = static_cast<SwitchCaseClauseList*>((yystack_[1].value.node));
      scc_list_p->push_back(static_cast<SwitchCaseClause*>((yystack_[0].value.node)));
      (yylhs.value.node) = (yystack_[1].value.node);
    }
    break;

  case 342: // SwitchCaseStatement: SwitchCaseOperandList ReturnOrSelect Statement
    {
      (yylhs.value.node) = new SwitchCaseClause(LOC(yylhs.location), dynamic_cast<SwitchCaseOperandList*>((yystack_[2].value.node)), (yystack_[0].value.expr));
    }
    break;

  case 343: // TypeswitchExpr: "'typeswitch'" "'('" Expr "')'" CaseClauseList "'default'" ReturnOrSelect ExprSingle
    {
      (yylhs.value.expr) = new TypeswitchExpr(LOC(yylhs.location),
                              (yystack_[5].value.expr),
                              static_cast<CaseClauseList*>((yystack_[3].value.node)),
                              (yystack_[0].value.expr));
    }
    break;

  case 344: // TypeswitchExpr: "'typeswitch'" "'('" Expr "')'" CaseClauseList "'default'" "'$'" QNAME ReturnOrSelect ExprSingle
    {
      (yylhs.value.expr) = new TypeswitchExpr(LOC (yylhs.location),
                              (yystack_[7].value.expr),
                              static_cast<CaseClauseList*>((yystack_[5].value.node)),
                              static_cast<QName*>((yystack_[2].value.expr)),
                              (yystack_[0].value.expr));
    }
    break;

  case 345: // TypeswitchStatement: "'typeswitch'" "'('" Expr "')'" CaseStatementList "'default'" ReturnOrSelect Statement
    {
      (yylhs.value.expr) = new TypeswitchExpr(LOC(yylhs.location),
                              (yystack_[5].value.expr),
                              static_cast<CaseClauseList*>((yystack_[3].value.node)),
                              (yystack_[0].value.expr));
    }
    break;

  case 346: // TypeswitchStatement: "'typeswitch'" "'('" Expr "')'" CaseClauseList "'default'" "'$'" QNAME ReturnOrSelect Statement
    {
      (yylhs.value.expr) = new TypeswitchExpr(LOC (yylhs.location),
                              (yystack_[7].value.expr),
                              static_cast<CaseClauseList*>((yystack_[5].value.node)),
                              static_cast<QName*>((yystack_[2].value.expr)),
                              (yystack_[0].value.expr));
    }
    break;

  case 347: // CaseClauseList: CaseClause
    {
      CaseClauseList* cc_list_p = new CaseClauseList(LOC (yylhs.location));
      cc_list_p->push_back(static_cast<CaseClause*>((yystack_[0].value.node)));
      (yylhs.value.node) = cc_list_p;
    }
    break;

  case 348: // CaseClauseList: CaseClauseList CaseClause
    {
      CaseClauseList* cc_list_p = dynamic_cast<CaseClauseList*>((yystack_[1].value.node));
      cc_list_p->push_back(static_cast<CaseClause*>((yystack_[0].value.node)));
      (yylhs.value.node) = (yystack_[1].value.node);
    }
    break;

  case 349: // CaseClause: "'case'" SequenceTypeList ReturnOrSelect ExprSingle
    {
      (yylhs.value.node) = new CaseClause(LOC (yylhs.location),
                          static_cast<SequenceTypeList*>((yystack_[2].value.node)),
                          (yystack_[0].value.expr));
    }
    break;

  case 350: // CaseClause: "'case'" "'$'" QNAME "'as'" SequenceTypeList ReturnOrSelect ExprSingle
    {
      (yylhs.value.node) = new CaseClause(LOC (yylhs.location),
                          static_cast<QName*>((yystack_[4].value.expr)),
                          static_cast<SequenceTypeList*>((yystack_[2].value.node)),
                          (yystack_[0].value.expr));
     }
    break;

  case 351: // CaseStatementList: CaseStatement
    {
      CaseClauseList* cc_list_p = new CaseClauseList(LOC (yylhs.location));
      cc_list_p->push_back(static_cast<CaseClause*>((yystack_[0].value.node)));
      (yylhs.value.node) = cc_list_p;
    }
    break;

  case 352: // CaseStatementList: CaseStatementList CaseStatement
    {
      CaseClauseList* cc_list_p = static_cast<CaseClauseList*>((yystack_[1].value.node));
      cc_list_p->push_back(static_cast<CaseClause*>((yystack_[0].value.node)));
      (yylhs.value.node) = (yystack_[1].value.node);
    }
    break;

  case 353: // CaseStatement: "'case'" SequenceTypeList ReturnOrSelect Statement
    {
      (yylhs.value.node) = new CaseClause(LOC (yylhs.location),
                          static_cast<SequenceTypeList*>((yystack_[2].value.node)),
                          (yystack_[0].value.expr));
    }
    break;

  case 354: // CaseStatement: "'case'" "'$'" QNAME "'as'" SequenceTypeList ReturnOrSelect Statement
    {
      (yylhs.value.node) = new CaseClause(LOC (yylhs.location),
                          static_cast<QName*>((yystack_[4].value.expr)),
                          static_cast<SequenceTypeList*>((yystack_[2].value.node)),
                          (yystack_[0].value.expr));
     }
    break;

  case 355: // SequenceTypeList: SequenceType
    {
      SequenceTypeList* seqList = new SequenceTypeList(LOC(yylhs.location));
      seqList->push_back(static_cast<SequenceTypeAST*>((yystack_[0].value.node)));
      (yylhs.value.node) = seqList;
    }
    break;

  case 356: // SequenceTypeList: SequenceTypeList "'|'" SequenceType
    {
      SequenceTypeList* seqList = static_cast<SequenceTypeList*>((yystack_[2].value.node));
      seqList->push_back(static_cast<SequenceTypeAST*>((yystack_[0].value.node)));
      (yylhs.value.node) = (yystack_[2].value.node);
    }
    break;

  case 357: // IfExpr: "'if'" "'('" Expr "')'" "'then'" ExprSingle "'else'" ExprSingle
    {
      (yylhs.value.expr) = new IfExpr(LOC (yylhs.location), (yystack_[5].value.expr), (yystack_[2].value.expr), (yystack_[0].value.expr));
    }
    break;

  case 358: // OrExpr: AndExpr
    {
      (yylhs.value.expr) = (yystack_[0].value.expr);
    }
    break;

  case 359: // OrExpr: OrExpr "'or'" AndExpr
    {
      (yylhs.value.expr) = new OrExpr( LOC(yylhs.location), (yystack_[2].value.expr), (yystack_[0].value.expr) );
    }
    break;

  case 360: // AndExpr: NotExpr
        {
            (yylhs.value.expr) = (yystack_[0].value.expr);
        }
    break;

  case 361: // AndExpr: AndExpr "'and'" NotExpr
        {
            (yylhs.value.expr) = new AndExpr( LOC(yylhs.location), (yystack_[2].value.expr), (yystack_[0].value.expr) );
        }
    break;

  case 362: // NotExpr: ComparisonExpr
        {
          (yylhs.value.expr) = (yystack_[0].value.expr);
        }
    break;

  case 363: // NotExpr: "'not'" NotExpr
        {
          (yylhs.value.expr) = new ComparisonExpr(
              LOC(yylhs.location),
              new ValueComp( LOC(yylhs.location), ParseConstants::op_val_not ),
              (yystack_[0].value.expr),
              NULL
          );
        }
    break;

  case 364: // ComparisonExpr: FTContainsExpr
        {
            (yylhs.value.expr) = (yystack_[0].value.expr);
        }
    break;

  case 365: // ComparisonExpr: FTContainsExpr ValueComp FTContainsExpr
        {
            (yylhs.value.expr) = new ComparisonExpr(
                LOC(yylhs.location),
                dynamic_cast<ValueComp*>((yystack_[1].value.node)),
                (yystack_[2].value.expr),
                (yystack_[0].value.expr)
            );
        }
    break;

  case 366: // ComparisonExpr: FTContainsExpr NodeComp FTContainsExpr
        {
            (yylhs.value.expr) = new ComparisonExpr(
                LOC(yylhs.location), dynamic_cast<NodeComp*>((yystack_[1].value.node)), (yystack_[2].value.expr), (yystack_[0].value.expr)
            );
        }
    break;

  case 367: // ComparisonExpr: FTContainsExpr "'='" FTContainsExpr
        {
            (yylhs.value.expr) = new ComparisonExpr(
                LOC(yylhs.location),
                new GeneralComp( LOC(yylhs.location), ParseConstants::op_eq ),
                (yystack_[2].value.expr),
                (yystack_[0].value.expr)
            );
        }
    break;

  case 368: // ComparisonExpr: FTContainsExpr "'!='" FTContainsExpr
        {
            (yylhs.value.expr) = new ComparisonExpr(
                LOC(yylhs.location),
                new GeneralComp( LOC(yylhs.location), ParseConstants::op_ne ),
                (yystack_[2].value.expr),
                (yystack_[0].value.expr)
            );
        }
    break;

  case 369: // $@1: %empty
        {
            driver.getJsoniqLexer()->interpretAsLessThan();
        }
    break;

  case 370: // ComparisonExpr: FTContainsExpr "'<'" $@1 FTContainsExpr
        {
            (yylhs.value.expr) = new ComparisonExpr(
                LOC(yylhs.location),
                new GeneralComp( LOC(yylhs.location), ParseConstants::op_lt ),
                (yystack_[3].value.expr),
                (yystack_[0].value.expr)
            );
        }
    break;

  case 371: // ComparisonExpr: FTContainsExpr "'<='" FTContainsExpr
        {
            (yylhs.value.expr) = new ComparisonExpr(
                LOC(yylhs.location),
                new GeneralComp( LOC(yylhs.location), ParseConstants::op_le ),
                (yystack_[2].value.expr),
                (yystack_[0].value.expr)
            );
        }
    break;

  case 372: // ComparisonExpr: FTContainsExpr "'>'" FTContainsExpr
        {
            (yylhs.value.expr) = new ComparisonExpr(
                LOC(yylhs.location),
                new GeneralComp( LOC(yylhs.location), ParseConstants::op_gt ),
                (yystack_[2].value.expr),
                (yystack_[0].value.expr)
            );
        }
    break;

  case 373: // ComparisonExpr: FTContainsExpr "'>='" FTContainsExpr
        {
            (yylhs.value.expr) = new ComparisonExpr(
                LOC(yylhs.location),
                new GeneralComp( LOC(yylhs.location), ParseConstants::op_ge ),
                (yystack_[2].value.expr),
                (yystack_[0].value.expr)
            );
        }
    break;

  case 374: // FTContainsExpr: StringConcatExpr
        {
            (yylhs.value.expr) = (yystack_[0].value.expr);
        }
    break;

  case 375: // FTContainsExpr: StringConcatExpr "'contains'" "'text'" FTSelection opt_FTIgnoreOption
        {
            (yylhs.value.expr) = new FTContainsExpr(
                LOC(yylhs.location),
                (yystack_[4].value.expr),
                dynamic_cast<FTSelection*>((yystack_[1].value.node)),
                dynamic_cast<FTIgnoreOption*>((yystack_[0].value.node))
            );
        }
    break;

  case 376: // StringConcatExpr: RangeExpr
        {
          (yylhs.value.expr) = (yystack_[0].value.expr);
        }
    break;

  case 377: // StringConcatExpr: RangeExpr "'||'" StringConcatExpr
        {
          (yylhs.value.expr) = new StringConcatExpr(LOC(yylhs.location), (yystack_[2].value.expr), (yystack_[0].value.expr));
        }
    break;

  case 378: // opt_FTIgnoreOption: %empty
        {
            (yylhs.value.node) = NULL;
        }
    break;

  case 379: // opt_FTIgnoreOption: FTIgnoreOption
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 380: // RangeExpr: AdditiveExpr
        {
            (yylhs.value.expr) = (yystack_[0].value.expr);
        }
    break;

  case 381: // RangeExpr: AdditiveExpr "'to'" AdditiveExpr
        {
            (yylhs.value.expr) = new RangeExpr( LOC(yylhs.location), (yystack_[2].value.expr), (yystack_[0].value.expr) );
        }
    break;

  case 382: // AdditiveExpr: MultiplicativeExpr
        {
            (yylhs.value.expr) = (yystack_[0].value.expr);
        }
    break;

  case 383: // AdditiveExpr: AdditiveExpr "'+'" MultiplicativeExpr
        {
            (yylhs.value.expr) = new AdditiveExpr( LOC(yylhs.location), ParseConstants::op_plus, (yystack_[2].value.expr), (yystack_[0].value.expr) );
        }
    break;

  case 384: // AdditiveExpr: AdditiveExpr "'-'" MultiplicativeExpr
        {
            (yylhs.value.expr) = new AdditiveExpr( LOC(yylhs.location), ParseConstants::op_minus, (yystack_[2].value.expr), (yystack_[0].value.expr) );
        }
    break;

  case 385: // MultiplicativeExpr: UnionExpr
        {
            (yylhs.value.expr) = (yystack_[0].value.expr);
        }
    break;

  case 386: // MultiplicativeExpr: MultiplicativeExpr "'*'" UnionExpr
        {
            (yylhs.value.expr) = new MultiplicativeExpr(
                LOC(yylhs.location), ParseConstants::op_mul, (yystack_[2].value.expr), (yystack_[0].value.expr)
            );
        }
    break;

  case 387: // MultiplicativeExpr: MultiplicativeExpr "'div'" UnionExpr
        {
            (yylhs.value.expr) = new MultiplicativeExpr(
                LOC(yylhs.location), ParseConstants::op_div, (yystack_[2].value.expr), (yystack_[0].value.expr)
            );
        }
    break;

  case 388: // MultiplicativeExpr: MultiplicativeExpr "'idiv'" UnionExpr
        {
            (yylhs.value.expr) = new MultiplicativeExpr(
                LOC(yylhs.location), ParseConstants::op_idiv, (yystack_[2].value.expr), (yystack_[0].value.expr)
            );
        }
    break;

  case 389: // MultiplicativeExpr: MultiplicativeExpr "'mod'" UnionExpr
        {
            (yylhs.value.expr) = new MultiplicativeExpr(
                LOC(yylhs.location), ParseConstants::op_mod, (yystack_[2].value.expr), (yystack_[0].value.expr)
            );
        }
    break;

  case 390: // UnionExpr: IntersectExceptExpr
        {
            (yylhs.value.expr) = (yystack_[0].value.expr);
        }
    break;

  case 391: // UnionExpr: UnionExpr "'union'" IntersectExceptExpr
        {
            (yylhs.value.expr) = new UnionExpr( LOC(yylhs.location), (yystack_[2].value.expr), (yystack_[0].value.expr) );
        }
    break;

  case 392: // UnionExpr: UnionExpr "'|'" IntersectExceptExpr
        {
            (yylhs.value.expr) = new UnionExpr( LOC(yylhs.location), (yystack_[2].value.expr), (yystack_[0].value.expr) );
        }
    break;

  case 393: // IntersectExceptExpr: InstanceofExpr
        {
            (yylhs.value.expr) = (yystack_[0].value.expr);
        }
    break;

  case 394: // IntersectExceptExpr: IntersectExceptExpr "'intersect'" InstanceofExpr
        {
            (yylhs.value.expr) = new IntersectExceptExpr(
                LOC(yylhs.location), ParseConstants::op_intersect, (yystack_[2].value.expr), (yystack_[0].value.expr)
            );
        }
    break;

  case 395: // IntersectExceptExpr: IntersectExceptExpr "'except'" InstanceofExpr
        {
            (yylhs.value.expr) = new IntersectExceptExpr(
                LOC(yylhs.location), ParseConstants::op_except, (yystack_[2].value.expr), (yystack_[0].value.expr)
            );
        }
    break;

  case 396: // InstanceofExpr: TreatExpr
        {
            (yylhs.value.expr) = (yystack_[0].value.expr);
        }
    break;

  case 397: // InstanceofExpr: TreatExpr "'instance'" "'of'" SequenceType
        {
            (yylhs.value.expr) = new InstanceofExpr(
                LOC(yylhs.location), (yystack_[3].value.expr), dynamic_cast<SequenceTypeAST*>((yystack_[0].value.node))
            );
        }
    break;

  case 398: // TreatExpr: CastableExpr
        {
            (yylhs.value.expr) = (yystack_[0].value.expr);
        }
    break;

  case 399: // TreatExpr: CastableExpr "'treat'" "'as'" SequenceType
        {
            (yylhs.value.expr) = new TreatExpr(
                LOC(yylhs.location), (yystack_[3].value.expr), dynamic_cast<SequenceTypeAST*>((yystack_[0].value.node))
            );
        }
    break;

  case 400: // CastableExpr: CastExpr
        {
            (yylhs.value.expr) = (yystack_[0].value.expr);
        }
    break;

  case 401: // CastableExpr: CastExpr "'castable'" "'as'" SingleType
        {
            (yylhs.value.expr) = new CastableExpr(
                LOC(yylhs.location), (yystack_[3].value.expr), dynamic_cast<SingleType*>((yystack_[0].value.node))
            );
        }
    break;

  case 402: // CastExpr: UnaryExpr
        {
            (yylhs.value.expr) = (yystack_[0].value.expr);
        }
    break;

  case 403: // CastExpr: UnaryExpr "'cast'" "'as'" SingleType
        {
            (yylhs.value.expr) = new CastExpr(
                LOC(yylhs.location), (yystack_[3].value.expr), dynamic_cast<SingleType*>((yystack_[0].value.node))
            );
        }
    break;

  case 404: // SingleType: SimpleType
    {
      (yylhs.value.node) = new SingleType(LOC(yylhs.location), dynamic_cast<SimpleType*>((yystack_[0].value.node)), false);
    }
    break;

  case 405: // SingleType: SimpleType "'?'"
    {
      (yylhs.value.node) = new SingleType(LOC(yylhs.location), dynamic_cast<SimpleType*>((yystack_[1].value.node)), true);
    }
    break;

  case 406: // UnaryExpr: ValueExpr
        {
            (yylhs.value.expr) = (yystack_[0].value.expr);
        }
    break;

  case 407: // UnaryExpr: SignList ValueExpr
        {
            (yylhs.value.expr) = new UnaryExpr( LOC(yylhs.location), dynamic_cast<SignList*>((yystack_[1].value.node)), (yystack_[0].value.expr) );
        }
    break;

  case 408: // SignList: "'+'"
        {
            (yylhs.value.node) = new SignList( LOC(yylhs.location), true );
        }
    break;

  case 409: // SignList: "'-'"
        {
            (yylhs.value.node) = new SignList( LOC(yylhs.location), false );
        }
    break;

  case 410: // SignList: SignList "'+'"
        {
            (yylhs.value.node) = (yystack_[1].value.node);
        }
    break;

  case 411: // SignList: SignList "'-'"
        {
            if ( SignList *sl = dynamic_cast<SignList*>((yystack_[1].value.node)) )
                sl->negate();
            (yylhs.value.node) = (yystack_[1].value.node);
        }
    break;

  case 412: // ValueExpr: ValidateExpr
        { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 413: // ValueExpr: SimpleMapExpr
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 414: // ValueExpr: ExtensionExpr
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 415: // SimpleMapExpr: PathExpr
      {
        (yylhs.value.expr) = (yystack_[0].value.expr);
      }
    break;

  case 416: // SimpleMapExpr: SimpleMapExpr "'!'" PathExpr
      {
        (yylhs.value.expr) = new SimpleMapExpr(LOC(yylhs.location), (yystack_[2].value.expr), (yystack_[0].value.expr));
      }
    break;

  case 417: // ValueComp: "'eq'"
        {
            (yylhs.value.node) = new ValueComp( LOC(yylhs.location), ParseConstants::op_val_eq );
        }
    break;

  case 418: // ValueComp: "'ne'"
        {
            (yylhs.value.node) = new ValueComp( LOC(yylhs.location), ParseConstants::op_val_ne );
        }
    break;

  case 419: // ValueComp: "'lt'"
        {
            (yylhs.value.node) = new ValueComp( LOC(yylhs.location), ParseConstants::op_val_lt );
        }
    break;

  case 420: // ValueComp: "'le'"
        {
            (yylhs.value.node) = new ValueComp( LOC(yylhs.location), ParseConstants::op_val_le );
        }
    break;

  case 421: // ValueComp: "'gt'"
        {
            (yylhs.value.node) = new ValueComp( LOC(yylhs.location), ParseConstants::op_val_gt );
        }
    break;

  case 422: // ValueComp: "'ge'"
        {
            (yylhs.value.node) = new ValueComp( LOC(yylhs.location), ParseConstants::op_val_ge );
        }
    break;

  case 423: // NodeComp: "'is'"
        {
            (yylhs.value.node) = new NodeComp( LOC(yylhs.location), ParseConstants::op_is );
        }
    break;

  case 424: // NodeComp: "'<<'"
        {
            (yylhs.value.node) = new NodeComp( LOC(yylhs.location), ParseConstants::op_precedes );
        }
    break;

  case 425: // NodeComp: "'follows'"
        {
            (yylhs.value.node) = new NodeComp( LOC(yylhs.location), ParseConstants::op_follows );
        }
    break;

  case 426: // ValidateExpr: "'validate'" "'{'" StatementsAndExpr "'}'"
        {
            (yylhs.value.expr) = new ValidateExpr( LOC(yylhs.location), "strict", (yystack_[1].value.expr) );
        }
    break;

  case 427: // ValidateExpr: "'validate'" "'lax'" "'{'" StatementsAndExpr "'}'"
        {
            (yylhs.value.expr) = new ValidateExpr( LOC(yylhs.location), "lax", (yystack_[1].value.expr) );
        }
    break;

  case 428: // ValidateExpr: "'validate'" "'strict'" "'{'" StatementsAndExpr "'}'"
        {
            (yylhs.value.expr) = new ValidateExpr( LOC(yylhs.location), "strict", (yystack_[1].value.expr) );
        }
    break;

  case 429: // ValidateExpr: "'validate'" "'type'" TypeName "'{'" StatementsAndExpr "'}'"
        {
            (yylhs.value.expr) = new ValidateExpr(
                LOC(yylhs.location), dynamic_cast<TypeName*>((yystack_[3].value.node))->get_name(), (yystack_[1].value.expr)
            );
            delete (yystack_[3].value.node);
        }
    break;

  case 430: // ExtensionExpr: Pragma_list "'{'" StatementsAndOptionalExpr "'}'"
        {
            (yylhs.value.expr) = new ExtensionExpr(
                LOC(yylhs.location), dynamic_cast<PragmaList*>((yystack_[3].value.node)), (yystack_[1].value.expr)
            );
        }
    break;

  case 431: // Pragma_list: Pragma
        {
            PragmaList *pl = new PragmaList( LOC(yylhs.location) );
            pl->push_back( dynamic_cast<Pragma*>((yystack_[0].value.node)) );
            (yylhs.value.node) = pl;
        }
    break;

  case 432: // Pragma_list: Pragma_list Pragma
        {
            if ( PragmaList *pl = dynamic_cast<PragmaList*>((yystack_[1].value.node)) )
                pl->push_back( dynamic_cast<Pragma*>((yystack_[0].value.node)) );
            (yylhs.value.node) = (yystack_[1].value.node);
        }
    break;

  case 433: // Pragma: "'(#'" QNAME "'pragma literal'"
        {
            (yylhs.value.node) = new Pragma( LOC(yylhs.location), static_cast<QName*>((yystack_[1].value.expr)), SYMTAB((yystack_[0].value.sval)) );
        }
    break;

  case 434: // Pragma: "'(#'" "'QName #)'"
        {
            (yylhs.value.node) = new Pragma( LOC(yylhs.location), new QName( LOC(yylhs.location), SYMTAB((yystack_[0].value.sval)) ), "" );
        }
    break;

  case 435: // Pragma: "'(#'" "'EQName #)'"
        {
            (yylhs.value.node) = new Pragma( LOC(yylhs.location), new QName( LOC(yylhs.location), SYMTAB((yystack_[0].value.sval)), true ), "" );
        }
    break;

  case 436: // PathExpr: LeadingSlash
    {
      (yylhs.value.expr) = new PathExpr(LOC(yylhs.location), ParseConstants::path_leading_lone_slash, NULL);
    }
    break;

  case 437: // PathExpr: LeadingSlash RelativePathExpr
    {
      RelativePathExpr* rpe;
      rpe = new RelativePathExpr(LOC(yylhs.location), ParseConstants::st_slash, NULL, (yystack_[0].value.expr), false);
      (yylhs.value.expr) = new PathExpr(LOC(yylhs.location),
                        ParseConstants::path_leading_slash,
                        rpe);
    }
    break;

  case 438: // PathExpr: "'//'" RelativePathExpr
    {
      RelativePathExpr* rpe;
      rpe = new RelativePathExpr(LOC(yylhs.location), ParseConstants::st_slashslash, NULL, (yystack_[0].value.expr), false);
      (yylhs.value.expr) = new PathExpr(LOC(yylhs.location),
                        ParseConstants::path_leading_slashslash,
                        rpe);
    }
    break;

  case 439: // PathExpr: RelativePathExpr
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
        switch (rpe->is_jsoniq_literal())
        {
        case 0:
          break;
        case 1:
          driver.addCommonLanguageWarning(yystack_[0].location, ZED(ZWST0009_TRUE_FALSE_NULL_KEYWORDS));
          (yylhs.value.expr) = new NullLiteral(LOC(yylhs.location));
          break;
        case 2:
          driver.addCommonLanguageWarning(yystack_[0].location, ZED(ZWST0009_TRUE_FALSE_NULL_KEYWORDS));
          (yylhs.value.expr) = new BooleanLiteral(LOC(yylhs.location), false);
          break;
        case 3:
          driver.addCommonLanguageWarning(yystack_[0].location, ZED(ZWST0009_TRUE_FALSE_NULL_KEYWORDS));
          (yylhs.value.expr) = new BooleanLiteral(LOC(yylhs.location), true);
          break;
        }
        if ((yylhs.value.expr) != NULL)
          release_hack( rpe );
      }
      if ((yylhs.value.expr) == NULL)
        (yylhs.value.expr) = (rpe ?
              new PathExpr( LOC(yylhs.location), ParseConstants::path_relative, (yystack_[0].value.expr)) :
              (yystack_[0].value.expr));
    }
    break;

  case 440: // LeadingSlash: "'/'"
    {
      (yylhs.value.expr) = NULL;
    }
    break;

  case 441: // RelativePathExpr: StepExpr
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

  case 442: // RelativePathExpr: StepExpr "'/'" RelativePathExpr
    {
      (yylhs.value.expr) = new RelativePathExpr(LOC(yylhs.location), ParseConstants::st_slash, (yystack_[2].value.expr), (yystack_[0].value.expr), false);
    }
    break;

  case 443: // RelativePathExpr: StepExpr "'//'" RelativePathExpr
    {
      (yylhs.value.expr) = new RelativePathExpr(LOC(yylhs.location), ParseConstants::st_slashslash, (yystack_[2].value.expr), (yystack_[0].value.expr), false);
    }
    break;

  case 444: // StepExpr: AxisStep
    { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 445: // StepExpr: PostfixExpr
    { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 446: // AxisStep: ForwardStep
        {
            (yylhs.value.expr) = new AxisStep(
                LOC(yylhs.location), dynamic_cast<ForwardStep*>((yystack_[0].value.node)), NULL
            );
        }
    break;

  case 447: // AxisStep: ForwardStep PredicateList
        {
            (yylhs.value.expr) = new AxisStep(
                LOC(yylhs.location),
                dynamic_cast<ForwardStep*>((yystack_[1].value.node)),
                dynamic_cast<PredicateList*>((yystack_[0].value.node))
            );
        }
    break;

  case 448: // AxisStep: ReverseStep
        {
            (yylhs.value.expr) = new AxisStep(
                LOC(yylhs.location), dynamic_cast<ReverseStep*>((yystack_[0].value.node)), NULL
            );
        }
    break;

  case 449: // AxisStep: ReverseStep PredicateList
        {
            (yylhs.value.expr) = new AxisStep(
                LOC(yylhs.location),
                dynamic_cast<ReverseStep*>((yystack_[1].value.node)),
                dynamic_cast<PredicateList*>((yystack_[0].value.node))
            );
        }
    break;

  case 450: // ForwardStep: ForwardAxis NodeTest
        {
            (yylhs.value.node) = new ForwardStep(
                LOC(yylhs.location), dynamic_cast<ForwardAxis*>((yystack_[1].value.node)), (yystack_[0].value.node)
            );
        }
    break;

  case 451: // ForwardStep: AbbrevForwardStep
        {
            (yylhs.value.node) = new ForwardStep(
                LOC(yylhs.location), dynamic_cast<AbbrevForwardStep*>((yystack_[0].value.node))
            );
        }
    break;

  case 452: // ForwardAxis: "'child'" "'::'"
        {
            (yylhs.value.node) = new ForwardAxis( LOC(yylhs.location), ParseConstants::axis_child );
        }
    break;

  case 453: // ForwardAxis: "'descendant'" "'::'"
        {
            (yylhs.value.node) = new ForwardAxis( LOC(yylhs.location), ParseConstants::axis_descendant);
        }
    break;

  case 454: // ForwardAxis: "'attribute'" "'::'"
        {
            (yylhs.value.node) = new ForwardAxis( LOC(yylhs.location), ParseConstants::axis_attribute );
        }
    break;

  case 455: // ForwardAxis: "'self'" "'::'"
        {
            (yylhs.value.node) = new ForwardAxis( LOC(yylhs.location), ParseConstants::axis_self );
        }
    break;

  case 456: // ForwardAxis: "'descendant-or-self'" "'::'"
        {
            (yylhs.value.node) = new ForwardAxis(
                LOC(yylhs.location), ParseConstants::axis_descendant_or_self
            );
        }
    break;

  case 457: // ForwardAxis: "'following-sibling'" "'::'"
        {
            (yylhs.value.node) = new ForwardAxis(
                LOC(yylhs.location), ParseConstants::axis_following_sibling
            );
        }
    break;

  case 458: // ForwardAxis: "'following'" "'::'"
        {
            (yylhs.value.node) = new ForwardAxis( LOC(yylhs.location), ParseConstants::axis_following );
        }
    break;

  case 459: // AbbrevForwardStep: NodeTest
        {
            (yylhs.value.node) = new AbbrevForwardStep( LOC(yylhs.location), (yystack_[0].value.node), false );
        }
    break;

  case 460: // AbbrevForwardStep: "'@'" NodeTest
        {
            (yylhs.value.node) = new AbbrevForwardStep( LOC(yylhs.location), (yystack_[0].value.node), true );
        }
    break;

  case 461: // ReverseStep: ReverseAxis NodeTest
        {
            (yylhs.value.node) = new ReverseStep( LOC(yylhs.location), dynamic_cast<ReverseAxis*>((yystack_[1].value.node)), (yystack_[0].value.node) );
        }
    break;

  case 462: // ReverseStep: "'..'"
        {
            ReverseAxis *ra = new ReverseAxis(
                LOC(yylhs.location), ParseConstants::axis_parent
            );
            (yylhs.value.node) = new ReverseStep( LOC(yylhs.location), ra, NULL );
        }
    break;

  case 463: // ReverseAxis: "'parent'" "'::'"
        {
            (yylhs.value.node) = new ReverseAxis( LOC(yylhs.location), ParseConstants::axis_parent );
        }
    break;

  case 464: // ReverseAxis: "'ancestor'" "'::'"
        {
            (yylhs.value.node) = new ReverseAxis( LOC(yylhs.location), ParseConstants::axis_ancestor );
        }
    break;

  case 465: // ReverseAxis: "'preceding-sibling'" "'::'"
        {
            (yylhs.value.node) = new ReverseAxis(
                LOC(yylhs.location), ParseConstants::axis_preceding_sibling
            );
        }
    break;

  case 466: // ReverseAxis: "'preceding'" "'::'"
        {
            (yylhs.value.node) = new ReverseAxis( LOC(yylhs.location), ParseConstants::axis_preceding );
        }
    break;

  case 467: // ReverseAxis: "'ancestor-or-self'" "'::'"
        {
            (yylhs.value.node) = new ReverseAxis(
                LOC(yylhs.location), ParseConstants::axis_ancestor_or_self
            );
        }
    break;

  case 468: // NodeTest: KindTest
        { (yylhs.value.node) = (yystack_[0].value.node); }
    break;

  case 469: // NodeTest: NameTest
      { (yylhs.value.node) = (yystack_[0].value.node); }
    break;

  case 470: // NameTest: QNAME
        {
            (yylhs.value.node) = new NameTest( LOC(yylhs.location), static_cast<QName*>((yystack_[0].value.expr)) );
        }
    break;

  case 471: // NameTest: Wildcard
        {
            (yylhs.value.node) = new NameTest( LOC(yylhs.location), dynamic_cast<Wildcard*>((yystack_[0].value.node)) );
        }
    break;

  case 472: // Wildcard: "'*'"
    {
      (yylhs.value.node) = new Wildcard(LOC(yylhs.location), "", "", ParseConstants::wild_all, false);
    }
    break;

  case 473: // Wildcard: "'pref:*'"
    {
      (yylhs.value.node) = new Wildcard(LOC(yylhs.location), SYMTAB((yystack_[0].value.sval)), "", ParseConstants::wild_elem, false);
    }
    break;

  case 474: // Wildcard: "'ns:*'"
    {
      (yylhs.value.node) = new Wildcard(LOC(yylhs.location), SYMTAB((yystack_[0].value.sval)), "", ParseConstants::wild_elem, true);
    }
    break;

  case 475: // Wildcard: "'*:QName'"
    {
      (yylhs.value.node) = new Wildcard(LOC(yylhs.location), "", SYMTAB((yystack_[0].value.sval)), ParseConstants::wild_prefix, false);
    }
    break;

  case 476: // PostfixExpr: PrimaryExpr
     {
       (yylhs.value.expr) = (yystack_[0].value.expr);
     }
    break;

  case 477: // PostfixExpr: PostfixExpr PredicateList
     {
       (yylhs.value.expr) = new FilterExpr(LOC(yylhs.location), (yystack_[1].value.expr), dynamic_cast<PredicateList*>((yystack_[0].value.node)));
     }
    break;

  case 478: // PostfixExpr: PostfixExpr "'('" "')'"
     {
       (yylhs.value.expr) = new DynamicFunctionInvocation(LOC(yylhs.location), (yystack_[2].value.expr), false);
     }
    break;

  case 479: // PostfixExpr: PostfixExpr "'('" ArgList "')'"
     {
       (yylhs.value.expr) = new DynamicFunctionInvocation(LOC(yylhs.location), (yystack_[3].value.expr), dynamic_cast<ArgList*>((yystack_[1].value.node)), false);
     }
    break;

  case 480: // PostfixExpr: PostfixExpr "'['" "']'"
    {
      (yylhs.value.expr) = new JSONArrayUnboxing(LOC(yylhs.location), (yystack_[2].value.expr));
    }
    break;

  case 481: // PostfixExpr: PostfixExpr "'.'" QNAME
     {
       ERROR_IF_QNAME_NOT_NCNAME((yystack_[0].value.expr), yystack_[0].location, (yystack_[2].value.expr));
       StringLiteral* sl = new StringLiteral( LOC(yylhs.location), static_cast<QName*>((yystack_[0].value.expr)));
       (yylhs.value.expr) = new JSONObjectLookup(LOC(yylhs.location), LOC(yystack_[1].location), (yystack_[2].value.expr), sl);
     }
    break;

  case 482: // PostfixExpr: PostfixExpr "'.'" "'('" "')'"
     {
       (yylhs.value.expr) = new JSONObjectLookup(LOC(yylhs.location), LOC(yystack_[2].location),
                                 (yystack_[3].value.expr),
                                 new ParenthesizedExpr(LOC(yylhs.location), NULL));
     }
    break;

  case 483: // PostfixExpr: PostfixExpr "'.'" "'('" Expr "')'"
     {
       (yylhs.value.expr) = new JSONObjectLookup(LOC(yylhs.location), LOC(yystack_[3].location),
                                 (yystack_[4].value.expr),
                                 new ParenthesizedExpr(LOC(yylhs.location), (yystack_[1].value.expr)));
     }
    break;

  case 484: // PostfixExpr: PostfixExpr "'.'" VarRef
     {
        (yylhs.value.expr) = new JSONObjectLookup(LOC(yylhs.location), LOC(yystack_[1].location), (yystack_[2].value.expr), (yystack_[0].value.expr));
     }
    break;

  case 485: // PostfixExpr: PostfixExpr "'.'" ContextItemExpr
     {
        (yylhs.value.expr) = new JSONObjectLookup(LOC(yylhs.location), LOC(yystack_[1].location), (yystack_[2].value.expr), (yystack_[0].value.expr));
     }
    break;

  case 486: // PostfixExpr: PostfixExpr "'.'" StringLiteral
     {
       (yylhs.value.expr) = new JSONObjectLookup(LOC(yylhs.location), LOC(yystack_[1].location), (yystack_[2].value.expr), (yystack_[0].value.expr));
     }
    break;

  case 487: // PredicateList: Predicate
    {
      PredicateList* pl = new PredicateList( LOC(yylhs.location) );
      pl->push_back(dynamic_cast<exprnode*>((yystack_[0].value.expr)));
      (yylhs.value.node) = pl;
    }
    break;

  case 488: // PredicateList: PredicateList Predicate
    {
      if (PredicateList* pl = dynamic_cast<PredicateList*>((yystack_[1].value.node)))
        pl->push_back(dynamic_cast<exprnode*>((yystack_[0].value.expr)));
      (yylhs.value.node) = (yystack_[1].value.node);
    }
    break;

  case 489: // Predicate: "'['" Expr "']'"
    {
      (yylhs.value.expr) = (yystack_[1].value.expr);
    }
    break;

  case 490: // PrimaryExpr: Literal
        { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 491: // PrimaryExpr: VarRef
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 492: // PrimaryExpr: ParenthesizedExpr
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 493: // PrimaryExpr: ContextItemExpr
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 494: // PrimaryExpr: FunctionCall
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 495: // PrimaryExpr: Constructor
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 496: // PrimaryExpr: OrderedExpr
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 497: // PrimaryExpr: UnorderedExpr
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 498: // PrimaryExpr: FunctionItemExpr
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 499: // PrimaryExpr: BlockExpr
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 500: // PrimaryExpr: JSONObjectConstructor
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 501: // PrimaryExpr: JSONArrayConstructor
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 502: // PrimaryExpr: JSONSimpleObjectUnion
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 503: // PrimaryExpr: JSONAccumulatorObjectUnion
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 504: // Literal: NumericLiteral
        {
            (yylhs.value.expr) = (yystack_[0].value.expr);
        }
    break;

  case 505: // Literal: StringLiteral
        {
            (yylhs.value.expr) = (yystack_[0].value.expr);
        }
    break;

  case 506: // NumericLiteral: "'DECIMAL'"
        {
            (yylhs.value.expr) = NumericLiteral::new_literal(
                LOC(yylhs.location), ParseConstants::num_decimal, *(yystack_[0].value.decval)
            );
            delete (yystack_[0].value.decval);
        }
    break;

  case 507: // NumericLiteral: "'INTEGER'"
        {
            (yylhs.value.expr) = NumericLiteral::new_literal(
                LOC(yylhs.location), ParseConstants::num_integer, *(yystack_[0].value.ival)
            );
            delete (yystack_[0].value.ival);
        }
    break;

  case 508: // NumericLiteral: "'DOUBLE'"
        {
            (yylhs.value.expr) = NumericLiteral::new_literal(
                LOC(yylhs.location), ParseConstants::num_double, *(yystack_[0].value.dval)
            );
            delete (yystack_[0].value.dval);
        }
    break;

  case 509: // VarRef: "'$'" QNAME
        {
            (yylhs.value.expr) = new VarRef(LOC(yylhs.location), static_cast<QName*>((yystack_[0].value.expr)));
        }
    break;

  case 510: // ParenthesizedExpr: "'('" "')'"
        {
            (yylhs.value.expr) = new ParenthesizedExpr( LOC(yylhs.location), NULL);
        }
    break;

  case 511: // ParenthesizedExpr: "'('" Expr "')'"
        {
            (yylhs.value.expr) = new ParenthesizedExpr( LOC(yylhs.location), (yystack_[1].value.expr) );
        }
    break;

  case 512: // ContextItemExpr: "'$_'"
        {
            driver.addCommonLanguageWarning(yystack_[0].location, ZED(ZWST0009_CONTEXT_ITEM_EXPR));
            (yylhs.value.expr) = new ContextItemExpr( LOC(yylhs.location) );
        }
    break;

  case 513: // OrderedExpr: "'ordered'" "'{'" Expr "'}'"
        {
            (yylhs.value.expr) = new OrderedExpr( LOC(yylhs.location), (yystack_[1].value.expr) );
        }
    break;

  case 514: // UnorderedExpr: "'unordered'" "'{'" Expr "'}'"
        {
            (yylhs.value.expr) = new UnorderedExpr( LOC(yylhs.location), (yystack_[1].value.expr) );
        }
    break;

  case 515: // FunctionCall: FUNCTION_NAME "'('" "')'"
    {
      (yylhs.value.expr) = new FunctionCall(LOC(yylhs.location), static_cast<QName*>((yystack_[2].value.expr)), NULL);
    }
    break;

  case 516: // FunctionCall: FUNCTION_NAME "'('" ArgList "')'"
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

  case 517: // ArgList: "'?'"
    {
      ArgList* al = new ArgList(LOC(yylhs.location));
      al->push_back(new ArgumentPlaceholder(LOC(yylhs.location)));
      (yylhs.value.node) = al;
    }
    break;

  case 518: // ArgList: ArgList "','" "'?'"
    {
      if (ArgList* al = dynamic_cast<ArgList*>((yystack_[2].value.node)))
        al->push_back(new ArgumentPlaceholder(LOC(yylhs.location)));
      (yylhs.value.node) = (yystack_[2].value.node);
    }
    break;

  case 519: // ArgList: ExprSingle
    {
      ArgList *al = new ArgList( LOC(yylhs.location) );
      al->push_back( (yystack_[0].value.expr) );
      (yylhs.value.node) = al;
    }
    break;

  case 520: // ArgList: ArgList "','" ExprSingle
    {
      if ( ArgList *al = dynamic_cast<ArgList*>((yystack_[2].value.node)) )
        al->push_back( (yystack_[0].value.expr) );
      (yylhs.value.node) = (yystack_[2].value.node);
    }
    break;

  case 521: // FunctionItemExpr: LiteralFunctionItem
    {
      (yylhs.value.expr) = (yystack_[0].value.expr);
    }
    break;

  case 522: // FunctionItemExpr: InlineFunction
    {
      (yylhs.value.expr) = (yystack_[0].value.expr);
    }
    break;

  case 523: // LiteralFunctionItem: FUNCTION_NAME "'#'" "'INTEGER'"
    {
      (yylhs.value.expr) = new LiteralFunctionItem(LOC (yylhs.location), dynamic_cast<QName*>((yystack_[2].value.expr)), (yystack_[0].value.ival));
    }
    break;

  case 524: // InlineFunction: "'function'" FunctionSig EnclosedStatementsAndOptionalExpr
    {
      (yylhs.value.expr) = new InlineFunction(LOC(yylhs.location),
                              (yystack_[1].value.fnsig)->theParams.getp(),
                              (yystack_[1].value.fnsig)->theReturnType.getp(),
                              (yystack_[0].value.expr));
      delete (yystack_[1].value.fnsig);
    }
    break;

  case 525: // Constructor: DirectConstructor
    { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 526: // Constructor: ComputedConstructor
  { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 527: // DirectConstructor: DirElemConstructor
    { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 528: // DirectConstructor: DirCommentConstructor
    { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 529: // DirectConstructor: DirPIConstructor
    { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 530: // DirElemConstructor: "'<'" QNAME OptionalBlank "'/>'"
    {
      (yylhs.value.expr) = new DirElemConstructor(LOC(yylhs.location),
                                  static_cast<QName*>((yystack_[2].value.expr)),
                                  NULL,
                                  NULL,
                                  NULL);
    }
    break;

  case 531: // DirElemConstructor: "'<'" QNAME OptionalBlank "'> (tag end)'" "'</ (start tag end)'" QNAME OptionalBlank "'> (tag end)'"
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

  case 532: // DirElemConstructor: "'<'" QNAME DirAttributeList OptionalBlank "'/>'"
    {
      (yylhs.value.expr) = new DirElemConstructor(LOC(yylhs.location),
                                  static_cast<QName*>((yystack_[3].value.expr)),
                                  NULL,
                                  dynamic_cast<DirAttributeList*>((yystack_[2].value.node)),
                                  NULL);
    }
    break;

  case 533: // DirElemConstructor: "'<'" QNAME DirAttributeList OptionalBlank "'> (tag end)'" "'</ (start tag end)'" QNAME OptionalBlank "'> (tag end)'"
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

  case 534: // DirElemConstructor: "'<'" QNAME OptionalBlank "'> (tag end)'" DirElemContentList "'</ (start tag end)'" QNAME OptionalBlank "'> (tag end)'"
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

  case 535: // DirElemConstructor: "'<'" QNAME DirAttributeList OptionalBlank "'> (tag end)'" DirElemContentList "'</ (start tag end)'" QNAME OptionalBlank "'> (tag end)'"
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

  case 536: // DirElemContentList: DirElemContent
    {
      DirElemContentList *decl = new DirElemContentList( LOC(yylhs.location) );
      decl->push_back( dynamic_cast<DirElemContent*>((yystack_[0].value.expr)) );
      (yylhs.value.node) = decl;
    }
    break;

  case 537: // DirElemContentList: DirElemContentList DirElemContent
    {
      DirElemContentList *decl = dynamic_cast<DirElemContentList*>((yystack_[1].value.node));
      if ( decl )
        decl->push_back( dynamic_cast<DirElemContent*>((yystack_[0].value.expr)) );
      (yylhs.value.node) = (yystack_[1].value.node);
    }
    break;

  case 538: // DirAttributeList: DirAttr
    {
      DirAttributeList *dal = new DirAttributeList( LOC(yylhs.location) );
      dal->push_back( dynamic_cast<DirAttr*>((yystack_[0].value.node)) );
      (yylhs.value.node) = dal;
    }
    break;

  case 539: // DirAttributeList: DirAttributeList DirAttr
    {
      DirAttributeList *dal = dynamic_cast<DirAttributeList*>((yystack_[1].value.node));
      if ( dal )
        dal->push_back( dynamic_cast<DirAttr*>((yystack_[0].value.node)) );
      (yylhs.value.node) = (yystack_[1].value.node);
    }
    break;

  case 540: // DirAttr: "'<blank>'" QNAME OptionalBlank "'='" OptionalBlank DirAttributeValue
    {
      (yylhs.value.node) = new DirAttr(LOC(yylhs.location),
                       static_cast<QName*>((yystack_[4].value.expr)),
                       dynamic_cast<DirAttributeValue*>((yystack_[0].value.node)));
    }
    break;

  case 543: // DirAttributeValue: "'\"'" opt_QuoteAttrContentList "'\"'"
    {
      (yylhs.value.node) = new DirAttributeValue(LOC(yylhs.location),
                                 dynamic_cast<QuoteAttrContentList*>((yystack_[1].value.node)));
    }
    break;

  case 544: // DirAttributeValue: "'''" opt_AposAttrContentList "'''"
    {
      (yylhs.value.node) = new DirAttributeValue( LOC(yylhs.location),
                                  dynamic_cast<AposAttrContentList*>((yystack_[1].value.node)));
    }
    break;

  case 545: // opt_QuoteAttrContentList: %empty
    {
      (yylhs.value.node) = new QuoteAttrContentList( LOC(yylhs.location) );
    }
    break;

  case 546: // opt_QuoteAttrContentList: QuoteAttrContentList
    {
      (yylhs.value.node) = (yystack_[0].value.node);
    }
    break;

  case 547: // QuoteAttrContentList: "'\"\"'"
    {
      QuoteAttrContentList *qacl = new QuoteAttrContentList( LOC(yylhs.location) );
      qacl->push_back( new QuoteAttrValueContent( LOC(yylhs.location), "\"" ) );
      (yylhs.value.node) = qacl;
    }
    break;

  case 548: // QuoteAttrContentList: QuoteAttrValueContent
    {
      QuoteAttrContentList *qacl = new QuoteAttrContentList( LOC(yylhs.location) );
      qacl->push_back( dynamic_cast<QuoteAttrValueContent*>((yystack_[0].value.node)) );
      (yylhs.value.node) = qacl;
    }
    break;

  case 549: // QuoteAttrContentList: QuoteAttrContentList "'\"\"'"
    {
      QuoteAttrContentList* qacl = dynamic_cast<QuoteAttrContentList*>((yystack_[1].value.node));
      if ( qacl )
        qacl->push_back( new QuoteAttrValueContent( LOC(yylhs.location), "\"" ) );
      (yylhs.value.node) = (yystack_[1].value.node);
    }
    break;

  case 550: // QuoteAttrContentList: QuoteAttrContentList QuoteAttrValueContent
    {
      QuoteAttrContentList *qacl = dynamic_cast<QuoteAttrContentList*>((yystack_[1].value.node));
      if ( qacl )
        qacl->push_back( dynamic_cast<QuoteAttrValueContent*>((yystack_[0].value.node)) );
      (yylhs.value.node) = (yystack_[1].value.node);
    }
    break;

  case 551: // opt_AposAttrContentList: %empty
    {
      (yylhs.value.node) = new AposAttrContentList( LOC(yylhs.location) );
    }
    break;

  case 552: // opt_AposAttrContentList: AposAttrContentList
    {
      (yylhs.value.node) = (yystack_[0].value.node);
    }
    break;

  case 553: // AposAttrContentList: "''''"
    {
      AposAttrContentList *aacl = new AposAttrContentList( LOC(yylhs.location) );
      aacl->push_back( new AposAttrValueContent( LOC(yylhs.location),"'") );
      (yylhs.value.node) = aacl;
    }
    break;

  case 554: // AposAttrContentList: AposAttrValueContent
    {
      AposAttrContentList *aacl = new AposAttrContentList( LOC(yylhs.location) );
      aacl->push_back( dynamic_cast<AposAttrValueContent*>((yystack_[0].value.node)) );
      (yylhs.value.node) = aacl;
    }
    break;

  case 555: // AposAttrContentList: AposAttrContentList "''''"
    {
      AposAttrContentList *aacl = dynamic_cast<AposAttrContentList*>((yystack_[1].value.node));
      if (aacl)
        aacl->push_back( new AposAttrValueContent( LOC(yylhs.location),"'") );
      (yylhs.value.node) = (yystack_[1].value.node);
    }
    break;

  case 556: // AposAttrContentList: AposAttrContentList AposAttrValueContent
    {
      AposAttrContentList *aacl = dynamic_cast<AposAttrContentList*>((yystack_[1].value.node));
      if ( aacl )
        aacl->push_back( dynamic_cast<AposAttrValueContent*>((yystack_[0].value.node)) );
      (yylhs.value.node) = (yystack_[1].value.node);
    }
    break;

  case 557: // QuoteAttrValueContent: "'quote attribute content'"
    {
      (yylhs.value.node) = new QuoteAttrValueContent( LOC(yylhs.location), SYMTAB((yystack_[0].value.sval)) );
    }
    break;

  case 558: // QuoteAttrValueContent: CommonContent
    {
      (yylhs.value.node) = new QuoteAttrValueContent(LOC(yylhs.location), dynamic_cast<CommonContent*>((yystack_[0].value.expr)));
    }
    break;

  case 559: // AposAttrValueContent: "'apos attribute content'"
    {
      (yylhs.value.node) = new AposAttrValueContent( LOC(yylhs.location), SYMTAB((yystack_[0].value.sval)) );
    }
    break;

  case 560: // AposAttrValueContent: CommonContent
    {
      (yylhs.value.node) = new AposAttrValueContent(LOC(yylhs.location), dynamic_cast<CommonContent*>((yystack_[0].value.expr)));
    }
    break;

  case 561: // DirElemContent: DirectConstructor
    {
      (yylhs.value.expr) = new DirElemContent( LOC(yylhs.location), (yystack_[0].value.expr) );
    }
    break;

  case 562: // DirElemContent: "'element content'"
    {
      (yylhs.value.expr) = new DirElemContent( LOC(yylhs.location), SYMTAB((yystack_[0].value.sval)) );
    }
    break;

  case 563: // DirElemContent: CDataSection
    {
      rchandle<CDataSection> cdata_h = dynamic_cast<CDataSection*>((yystack_[0].value.expr));
      (yylhs.value.expr) = new DirElemContent( LOC(yylhs.location), cdata_h );
    }
    break;

  case 564: // DirElemContent: CommonContent
    {
      rchandle<CommonContent> cont_h = dynamic_cast<CommonContent*>((yystack_[0].value.expr));
      (yylhs.value.expr) = new DirElemContent( LOC(yylhs.location), cont_h );
    }
    break;

  case 565: // CommonContent: "'&#charref;'"
    {
      (yylhs.value.expr) = new CommonContent(LOC(yylhs.location), ParseConstants::cont_charref, SYMTAB((yystack_[0].value.sval)));
    }
    break;

  case 566: // CommonContent: "'{{'"
    {
      (yylhs.value.expr) = new CommonContent(LOC(yylhs.location), ParseConstants::cont_escape_lbrace);
    }
    break;

  case 567: // CommonContent: "'<double {>'"
    {
      (yylhs.value.expr) = new CommonContent(LOC(yylhs.location), ParseConstants::cont_escape_rbrace);
    }
    break;

  case 568: // CommonContent: "'{'" StatementsAndExpr "'}'"
    {
      (yylhs.value.expr) = new CommonContent(LOC(yylhs.location), new EnclosedExpr(LOC(yylhs.location), (yystack_[1].value.expr)));
    }
    break;

  case 569: // DirCommentConstructor: "'<!--'" "'XML comment'" "'-->'"
    {
      (yylhs.value.expr) = new DirCommentConstructor( LOC(yylhs.location), SYMTAB((yystack_[1].value.sval)) );
    }
    break;

  case 570: // DirCommentConstructor: "'<!--'" "'-->'"
    {
      (yylhs.value.expr) = new DirCommentConstructor( LOC(yylhs.location), "" );
    }
    break;

  case 571: // DirPIConstructor: "'<?'" QNAME "'?>'"
    {
      ERROR_IF_QNAME_NOT_NCNAME((yystack_[1].value.expr), yystack_[1].location);
      (yylhs.value.expr) = new DirPIConstructor( LOC(yylhs.location), static_cast<QName*>((yystack_[1].value.expr)) );
    }
    break;

  case 572: // DirPIConstructor: "'<?'" QNAME "'char literal and pi end'"
    {
      ERROR_IF_QNAME_NOT_NCNAME((yystack_[1].value.expr), yystack_[1].location);
      (yylhs.value.expr) = new DirPIConstructor( LOC(yylhs.location), static_cast<QName*>((yystack_[1].value.expr)), SYMTAB((yystack_[0].value.sval)) );
    }
    break;

  case 573: // CDataSection: "'CDATA[['" "'char literal]]>'"
    {
      (yylhs.value.expr) = new CDataSection( LOC(yylhs.location),SYMTAB((yystack_[0].value.sval)) );
    }
    break;

  case 574: // ComputedConstructor: CompDocConstructor
    {
      (yylhs.value.expr) = (yystack_[0].value.expr);
    }
    break;

  case 575: // ComputedConstructor: CompElemConstructor
    {
      (yylhs.value.expr) = (yystack_[0].value.expr);
    }
    break;

  case 576: // ComputedConstructor: CompAttrConstructor
    {
      (yylhs.value.expr) = (yystack_[0].value.expr);
    }
    break;

  case 577: // ComputedConstructor: CompTextConstructor
    {
      (yylhs.value.expr) = (yystack_[0].value.expr);
    }
    break;

  case 578: // ComputedConstructor: CompCommentConstructor
    {
      (yylhs.value.expr) = (yystack_[0].value.expr);
    }
    break;

  case 579: // ComputedConstructor: CompPIConstructor
    {
      (yylhs.value.expr) = (yystack_[0].value.expr);
    }
    break;

  case 580: // ComputedConstructor: CompNamespaceConstructor
    { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 581: // CompDocConstructor: "'document'" "'{'" StatementsAndExpr "'}'"
    {
      (yylhs.value.expr) = new CompDocConstructor( LOC(yylhs.location), (yystack_[1].value.expr) );
    }
    break;

  case 582: // CompElemConstructor: "'element QName {'" StatementsAndOptionalExpr "'}'"
    {
      (yylhs.value.expr) = new CompElemConstructor(LOC(yylhs.location), new QName(LOC(yylhs.location), SYMTAB((yystack_[2].value.sval))), (yystack_[1].value.expr));
    }
    break;

  case 583: // CompElemConstructor: "'element'" "'{'" Expr "'}'" "'{'" StatementsAndOptionalExpr "'}'"
    {
      (yylhs.value.expr) = new CompElemConstructor( LOC(yylhs.location), (yystack_[4].value.expr), (yystack_[1].value.expr) );
    }
    break;

  case 584: // CompAttrConstructor: "'attribute QName {'" StatementsAndOptionalExpr "'}'"
    {
      (yylhs.value.expr) = new CompAttrConstructor( LOC(yylhs.location), new QName(LOC(yylhs.location), SYMTAB((yystack_[2].value.sval))), (yystack_[1].value.expr) );
    }
    break;

  case 585: // CompAttrConstructor: "'attribute'" "'{'" Expr "'}'" "'{'" StatementsAndOptionalExpr "'}'"
    {
      (yylhs.value.expr) = new CompAttrConstructor( LOC(yylhs.location), (yystack_[4].value.expr), (yystack_[1].value.expr) );
    }
    break;

  case 586: // CompTextConstructor: "'text'" "'{'" StatementsAndExpr "'}'"
    {
      (yylhs.value.expr) = new CompTextConstructor( LOC(yylhs.location), (yystack_[1].value.expr) );
    }
    break;

  case 587: // CompCommentConstructor: "'comment'" "'{'" StatementsAndExpr "'}'"
    {
      (yylhs.value.expr) = new CompCommentConstructor( LOC(yylhs.location), (yystack_[1].value.expr) );
    }
    break;

  case 588: // CompPIConstructor: "'processing-instruction NCName {'" StatementsAndOptionalExpr "'}'"
    {
      (yylhs.value.expr) = new CompPIConstructor(LOC(yylhs.location), SYMTAB((yystack_[2].value.sval)), (yystack_[1].value.expr));
    }
    break;

  case 589: // CompPIConstructor: "'processing-instruction'" "'{'" Expr "'}'" "'{'" StatementsAndOptionalExpr "'}'"
    {
      (yylhs.value.expr) = new CompPIConstructor(LOC(yylhs.location), (yystack_[4].value.expr), (yystack_[1].value.expr));
    }
    break;

  case 590: // CompNamespaceConstructor: "'namespace NCName {'" StatementsAndExpr "'}'"
    {
      (yylhs.value.expr) = new CompNamespaceConstructor(LOC(yylhs.location), SYMTAB((yystack_[2].value.sval)), (yystack_[1].value.expr));
    }
    break;

  case 591: // CompNamespaceConstructor: "'namespace'" "'{'" Expr "'}'" "'{'" StatementsAndExpr "'}'"
    {
      (yylhs.value.expr) = new CompNamespaceConstructor(LOC(yylhs.location), (yystack_[4].value.expr), (yystack_[1].value.expr));
    }
    break;

  case 592: // TypeDeclaration: "'as'" SequenceType
    {
      (yylhs.value.node) = (yystack_[0].value.node);
    }
    break;

  case 593: // SequenceType: ItemType
        {
            (yylhs.value.node) = new SequenceTypeAST( LOC(yylhs.location), (yystack_[0].value.node), NULL );
        }
    break;

  case 594: // SequenceType: ItemType OccurrenceIndicator
        {
            (yylhs.value.node) = new SequenceTypeAST(LOC(yylhs.location), (yystack_[1].value.node), dynamic_cast<OccurrenceIndicator*>((yystack_[0].value.node)));
        }
    break;

  case 595: // SequenceType: "'empty-sequence'" "'('" "')'"
        {
            (yylhs.value.node) = new SequenceTypeAST( LOC(yylhs.location), NULL, NULL );
        }
    break;

  case 596: // SequenceType: "'('" "')'"
        {
            driver.addCommonLanguageWarning(yystack_[1].location, ZED(ZWST0009_JSONIQ_EMPTY_SEQUENCE));
            (yylhs.value.node) = new SequenceTypeAST( LOC(yylhs.location), NULL, NULL );
        }
    break;

  case 597: // OccurrenceIndicator: "'?'"
        {
            (yylhs.value.node) = new OccurrenceIndicator(
                LOC(yylhs.location), ParseConstants::occurs_optionally
            );
        }
    break;

  case 598: // OccurrenceIndicator: "'*'"
        {
            (yylhs.value.node) = new OccurrenceIndicator(
                LOC(yylhs.location), ParseConstants::occurs_zero_or_more
            );
        }
    break;

  case 599: // OccurrenceIndicator: "'+'"
        {
            (yylhs.value.node) = new OccurrenceIndicator(
                LOC(yylhs.location), ParseConstants::occurs_one_or_more
            );
        }
    break;

  case 600: // ItemType: GeneralizedAtomicType
        {
            GeneralizedAtomicType* gat = static_cast<GeneralizedAtomicType*>((yystack_[0].value.node));
            QName* q = gat->get_qname();
            if (q->get_qname() == "item")
            {
              driver.addCommonLanguageWarning(yystack_[0].location, ZED(ZWST0009_JSONIQ_TYPE_KEYWORDS));
              (yylhs.value.node) = new ItemType( LOC(yylhs.location), true );
              delete gat;
            }
            else if (q->get_qname() == "array")
            {
              driver.addCommonLanguageWarning(yystack_[0].location, ZED(ZWST0009_JSONIQ_TYPE_KEYWORDS));
              (yylhs.value.node) = new JSON_Test(LOC(yylhs.location), store::StoreConsts::jsonArray);
              delete gat;
            }
            else if (q->get_qname() == "object")
            {
              driver.addCommonLanguageWarning(yystack_[0].location, ZED(ZWST0009_JSONIQ_TYPE_KEYWORDS));
              (yylhs.value.node) = new JSON_Test(LOC(yylhs.location), store::StoreConsts::jsonObject);
              delete gat;
            }
            else if (q->get_qname() == "json-item")
            {
              driver.addCommonLanguageWarning(yystack_[0].location, ZED(ZWST0009_JSONIQ_TYPE_KEYWORDS));
              (yylhs.value.node) = new JSON_Test(LOC(yylhs.location), store::StoreConsts::jsonItem);
              delete gat;
            }
            else if (q->get_qname() == "structured-item")
            {
              driver.addCommonLanguageWarning(yystack_[0].location, ZED(ZWST0009_JSONIQ_TYPE_KEYWORDS));
              (yylhs.value.node) = new StructuredItemType(LOC(yylhs.location));
              delete gat;
            }
            else
            {
              (yylhs.value.node) = (yystack_[0].value.node);
            }
        }
    break;

  case 601: // ItemType: KindTest
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 602: // ItemType: "'item'" "'('" "')'"
        {
            (yylhs.value.node) = new ItemType( LOC(yylhs.location), true );
        }
    break;

  case 603: // ItemType: "'structured-item'" "'('" "')'"
        {
            (yylhs.value.node) = new StructuredItemType(LOC(yylhs.location));
        }
    break;

  case 604: // ItemType: FunctionTest
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 605: // ItemType: ParenthesizedItemType
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 606: // ItemType: JSONTest
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 607: // TypeList: SequenceType
        {
          TypeList* aTypeList = new TypeList(LOC (yylhs.location));
          aTypeList->push_back(dynamic_cast<SequenceTypeAST *>((yystack_[0].value.node)));
          (yylhs.value.node) = aTypeList;
        }
    break;

  case 608: // TypeList: TypeList "','" SequenceType
        {
          TypeList* aTypeList = dynamic_cast<TypeList *>((yystack_[2].value.node));
          aTypeList->push_back(dynamic_cast<SequenceTypeAST *>((yystack_[0].value.node)));
          (yylhs.value.node) = (yystack_[2].value.node);
        }
    break;

  case 609: // GeneralizedAtomicType: QNAME
    {
      (yylhs.value.node) = new GeneralizedAtomicType( LOC(yylhs.location), static_cast<QName*>((yystack_[0].value.expr)) );
    }
    break;

  case 610: // SimpleType: QNAME
    {
      (yylhs.value.node) = new SimpleType( LOC(yylhs.location), static_cast<QName*>((yystack_[0].value.expr)) );
    }
    break;

  case 611: // KindTest: DocumentTest
    {
      (yylhs.value.node) = (yystack_[0].value.node);
    }
    break;

  case 612: // KindTest: ElementTest
    {
      (yylhs.value.node) = (yystack_[0].value.node);
    }
    break;

  case 613: // KindTest: AttributeTest
    {
      (yylhs.value.node) = (yystack_[0].value.node);
    }
    break;

  case 614: // KindTest: SchemaElementTest
    {
      (yylhs.value.node) = (yystack_[0].value.node);
    }
    break;

  case 615: // KindTest: SchemaAttributeTest
    {
      (yylhs.value.node) = (yystack_[0].value.node);
    }
    break;

  case 616: // KindTest: PITest
    {
      (yylhs.value.node) = (yystack_[0].value.node);
    }
    break;

  case 617: // KindTest: CommentTest
    {
      (yylhs.value.node) = (yystack_[0].value.node);
    }
    break;

  case 618: // KindTest: TextTest
    {
      (yylhs.value.node) = (yystack_[0].value.node);
    }
    break;

  case 619: // KindTest: NamespaceTest
    {
      (yylhs.value.node) = (yystack_[0].value.node);
    }
    break;

  case 620: // KindTest: AnyKindTest
    {
      (yylhs.value.node) = (yystack_[0].value.node);
    }
    break;

  case 621: // AnyKindTest: NODE_KIND_TEST "'('" "')'"
    {
      (yylhs.value.node) = new AnyKindTest( LOC(yylhs.location) );
    }
    break;

  case 622: // NODE_KIND_TEST: "'node'"
    {
      (yylhs.value.expr) = NULL;
    }
    break;

  case 623: // DocumentTest: "'document-node'" "'('" "')'"
    {
      (yylhs.value.node) = new DocumentTest(LOC(yylhs.location));
    }
    break;

  case 624: // DocumentTest: "'document-node'" "'('" ElementTest "')'"
    {
      (yylhs.value.node) = new DocumentTest(LOC(yylhs.location), dynamic_cast<ElementTest*>((yystack_[1].value.node)));
    }
    break;

  case 625: // DocumentTest: "'document-node'" "'('" SchemaElementTest "')'"
    {
      (yylhs.value.node) = new DocumentTest(LOC(yylhs.location), dynamic_cast<SchemaElementTest*>((yystack_[1].value.node)));
    }
    break;

  case 626: // NamespaceTest: "'namespace-node'" "'('" "')'"
    {
      (yylhs.value.node) = new NamespaceTest(LOC(yylhs.location));
    }
    break;

  case 627: // TextTest: "'text'" "'('" "')'"
    {
      (yylhs.value.node) = new TextTest(LOC(yylhs.location));
    }
    break;

  case 628: // CommentTest: "'comment'" "'('" "')'"
    {
      (yylhs.value.node) = new CommentTest(LOC(yylhs.location));
    }
    break;

  case 629: // PITest: "'processing-instruction'" "'('" "')'"
        {
            (yylhs.value.node) = new PITest( LOC(yylhs.location), "" );
        }
    break;

  case 630: // PITest: "'processing-instruction'" "'('" QNAME "')'"
        {
            ERROR_IF_QNAME_NOT_NCNAME((yystack_[1].value.expr), yystack_[1].location);
            (yylhs.value.node) = new PITest( LOC(yylhs.location), static_cast<QName*>((yystack_[1].value.expr)) );
        }
    break;

  case 631: // PITest: "'processing-instruction'" "'('" "'STRING'" "')'"
        {
            (yylhs.value.node) = new PITest( LOC(yylhs.location), SYMTAB((yystack_[1].value.sval)) );
        }
    break;

  case 632: // AttributeTest: "'attribute'" "'('" "')'"
        {
            (yylhs.value.node) = new AttributeTest( LOC(yylhs.location), NULL, NULL );
        }
    break;

  case 633: // AttributeTest: "'attribute'" "'('" QNAME "')'"
        {
            (yylhs.value.node) = new AttributeTest(
                LOC(yylhs.location), static_cast<QName*>((yystack_[1].value.expr)), NULL
            );
        }
    break;

  case 634: // AttributeTest: "'attribute'" "'('" QNAME "','" TypeName "')'"
        {
            (yylhs.value.node) = new AttributeTest(
                LOC(yylhs.location), static_cast<QName*>((yystack_[3].value.expr)), dynamic_cast<TypeName*>((yystack_[1].value.node))
            );
        }
    break;

  case 635: // AttributeTest: "'attribute'" "'('" "'*'" "')'"
        {
            (yylhs.value.node) = new AttributeTest( LOC(yylhs.location), NULL, NULL );
        }
    break;

  case 636: // AttributeTest: "'attribute'" "'('" "'*'" "','" TypeName "')'"
        {
            (yylhs.value.node) = new AttributeTest(
                LOC(yylhs.location), NULL, dynamic_cast<TypeName*>((yystack_[1].value.node))
            );
        }
    break;

  case 637: // SchemaAttributeTest: "'schema-attribute'" "'('" QNAME "')'"
        {
            (yylhs.value.node) = new SchemaAttributeTest( LOC(yylhs.location), static_cast<QName*>((yystack_[1].value.expr)) );
        }
    break;

  case 638: // ElementTest: "'element'" "'('" "')'"
        {
            (yylhs.value.node) = new ElementTest( LOC(yylhs.location), NULL, NULL, true );
        }
    break;

  case 639: // ElementTest: "'element'" "'('" QNAME "')'"
        {
            (yylhs.value.node) = new ElementTest(
                LOC(yylhs.location), static_cast<QName*>((yystack_[1].value.expr)), NULL, true
            );
        }
    break;

  case 640: // ElementTest: "'element'" "'('" QNAME "','" TypeName "')'"
        {
            (yylhs.value.node) = new ElementTest(
                LOC(yylhs.location),
                static_cast<QName*>((yystack_[3].value.expr)),
                dynamic_cast<TypeName*>((yystack_[1].value.node)),
                false
            );
        }
    break;

  case 641: // ElementTest: "'element'" "'('" QNAME "','" TypeName_WITH_HOOK "')'"
        {
            (yylhs.value.node) = new ElementTest(
                LOC(yylhs.location),
                static_cast<QName*>((yystack_[3].value.expr)),
                dynamic_cast<TypeName*>((yystack_[1].value.node)),
                true
            );
        }
    break;

  case 642: // ElementTest: "'element'" "'('" "'*'" "')'"
        {
            (yylhs.value.node) = new ElementTest(
                LOC(yylhs.location), NULL, NULL, true
            );
        }
    break;

  case 643: // ElementTest: "'element'" "'('" "'*'" "','" TypeName "')'"
        {
            (yylhs.value.node) = new ElementTest(
                LOC(yylhs.location), NULL, dynamic_cast<TypeName*>((yystack_[1].value.node)), false
            );
        }
    break;

  case 644: // ElementTest: "'element'" "'('" "'*'" "','" TypeName_WITH_HOOK "')'"
        {
            (yylhs.value.node) = new ElementTest(
                LOC(yylhs.location), NULL, dynamic_cast<TypeName*>((yystack_[1].value.node)), true
            );
        }
    break;

  case 645: // SchemaElementTest: "'schema-element'" "'('" QNAME "')'"
        {
            (yylhs.value.node) = new SchemaElementTest( LOC(yylhs.location), static_cast<QName*>((yystack_[1].value.expr)) );
        }
    break;

  case 646: // TypeName: QNAME
        {
            (yylhs.value.node) = new TypeName( LOC(yylhs.location), static_cast<QName*>((yystack_[0].value.expr)) );
        }
    break;

  case 647: // TypeName_WITH_HOOK: QNAME "'?'"
        {
            (yylhs.value.node) = new TypeName( LOC(yylhs.location), static_cast<QName*>((yystack_[1].value.expr)), true );
        }
    break;

  case 648: // StringLiteral: "'STRING'"
        {
            (yylhs.value.expr) = new StringLiteral( LOC(yylhs.location), SYMTAB((yystack_[0].value.sval)) );
        }
    break;

  case 649: // FunctionTest: AnyFunctionTest
    {
      (yylhs.value.node) = (yystack_[0].value.node);
    }
    break;

  case 650: // FunctionTest: TypedFunctionTest
    {
      (yylhs.value.node) = (yystack_[0].value.node);
    }
    break;

  case 651: // AnyFunctionTest: "'function'" "'('" "'*'" "')'"
    {
      (yylhs.value.node) = new AnyFunctionTest(LOC(yylhs.location));
    }
    break;

  case 652: // TypedFunctionTest: "'function'" "'('" "')'" "'as'" SequenceType
        {
          (yylhs.value.node) = new TypedFunctionTest(LOC (yylhs.location), dynamic_cast<SequenceTypeAST *>((yystack_[0].value.node)));
        }
    break;

  case 653: // TypedFunctionTest: "'function'" "'('" TypeList "')'" "'as'" SequenceType
        {
          (yylhs.value.node) = new TypedFunctionTest(LOC (yylhs.location),
              dynamic_cast<TypeList *>((yystack_[3].value.node)),
              dynamic_cast<SequenceTypeAST *>((yystack_[0].value.node)));
        }
    break;

  case 654: // ParenthesizedItemType: "'('" ItemType "')'"
        {
           (yylhs.value.node) = (yystack_[1].value.node);
        }
    break;

  case 655: // RevalidationDecl: "'declare'" "'revalidation'" "'strict'"
        {
            (yylhs.value.node) = new RevalidationDecl(
                LOC(yylhs.location), StaticContextConsts::strict_validation
            );
        }
    break;

  case 656: // RevalidationDecl: "'declare'" "'revalidation'" "'lax'"
        {
            (yylhs.value.node) = new RevalidationDecl(
                LOC(yylhs.location), StaticContextConsts::lax_validation
            );
        }
    break;

  case 657: // RevalidationDecl: "'declare'" "'revalidation'" "'skip'"
        {
            (yylhs.value.node) = new RevalidationDecl(
                LOC(yylhs.location), StaticContextConsts::skip_validation
            );
        }
    break;

  case 658: // InsertExpr: "'insert'" "'node'" ExprSingle "'into'" ExprSingle
        {
            (yylhs.value.expr) = new InsertExpr( LOC(yylhs.location), store::UpdateConsts::INTO, (yystack_[2].value.expr), (yystack_[0].value.expr) );
        }
    break;

  case 659: // InsertExpr: "'insert'" "'node'" ExprSingle "'as'" "'first'" "'into'" ExprSingle
        {
            (yylhs.value.expr) = new InsertExpr(
                LOC(yylhs.location), store::UpdateConsts::AS_FIRST_INTO, (yystack_[4].value.expr), (yystack_[0].value.expr)
            );
        }
    break;

  case 660: // InsertExpr: "'insert'" "'node'" ExprSingle "'as'" "'last'" "'into'" ExprSingle
        {
            (yylhs.value.expr) = new InsertExpr(
                LOC(yylhs.location), store::UpdateConsts::AS_LAST_INTO, (yystack_[4].value.expr), (yystack_[0].value.expr)
            );
        }
    break;

  case 661: // InsertExpr: "'insert'" "'node'" ExprSingle "'after'" ExprSingle
        {
            (yylhs.value.expr) = new InsertExpr( LOC(yylhs.location), store::UpdateConsts::AFTER, (yystack_[2].value.expr), (yystack_[0].value.expr) );
        }
    break;

  case 662: // InsertExpr: "'insert'" "'node'" ExprSingle "'before'" ExprSingle
        {
            (yylhs.value.expr) = new InsertExpr(
                LOC (yylhs.location), store::UpdateConsts::BEFORE, (yystack_[2].value.expr), (yystack_[0].value.expr)
            );
        }
    break;

  case 663: // InsertExpr: "'insert'" "'nodes'" ExprSingle "'into'" ExprSingle
        {
            (yylhs.value.expr) = new InsertExpr( LOC(yylhs.location), store::UpdateConsts::INTO, (yystack_[2].value.expr), (yystack_[0].value.expr) );
        }
    break;

  case 664: // InsertExpr: "'insert'" "'nodes'" ExprSingle "'as'" "'first'" "'into'" ExprSingle
        {
            (yylhs.value.expr) = new InsertExpr(
                LOC(yylhs.location), store::UpdateConsts::AS_FIRST_INTO, (yystack_[4].value.expr), (yystack_[0].value.expr)
            );
        }
    break;

  case 665: // InsertExpr: "'insert'" "'nodes'" ExprSingle "'as'" "'last'" "'into'" ExprSingle
        {
            (yylhs.value.expr) = new InsertExpr(
                  LOC(yylhs.location), store::UpdateConsts::AS_LAST_INTO, (yystack_[4].value.expr), (yystack_[0].value.expr)
            );
        }
    break;

  case 666: // InsertExpr: "'insert'" "'nodes'" ExprSingle "'after'" ExprSingle
        {
            (yylhs.value.expr) = new InsertExpr(
                LOC (yylhs.location),
                store::UpdateConsts::AFTER, (yystack_[2].value.expr), (yystack_[0].value.expr)
            );
        }
    break;

  case 667: // InsertExpr: "'insert'" "'nodes'" ExprSingle "'before'" ExprSingle
        {
            (yylhs.value.expr) = new InsertExpr(
                LOC (yylhs.location), store::UpdateConsts::BEFORE, (yystack_[2].value.expr), (yystack_[0].value.expr)
            );
        }
    break;

  case 668: // DeleteExpr: "'delete'" "'node'" ExprSingle
        {
            (yylhs.value.expr) = new DeleteExpr( LOC (yylhs.location), (yystack_[0].value.expr) );
        }
    break;

  case 669: // DeleteExpr: "'delete'" "'nodes'" ExprSingle
        {
            (yylhs.value.expr) = new DeleteExpr( LOC (yylhs.location), (yystack_[0].value.expr) );
        }
    break;

  case 670: // ReplaceExpr: "'replace'" "'node'" ExprSingle "'with'" ExprSingle
        {
            (yylhs.value.expr) = new ReplaceExpr(
                LOC(yylhs.location), store::UpdateConsts::NODE, (yystack_[2].value.expr), (yystack_[0].value.expr)
            );
        }
    break;

  case 671: // ReplaceExpr: "'replace'" "'value'" "'of'" "'node'" ExprSingle "'with'" ExprSingle
        {
            (yylhs.value.expr) = new ReplaceExpr(
                LOC(yylhs.location), store::UpdateConsts::VALUE_OF_NODE, (yystack_[2].value.expr), (yystack_[0].value.expr)
            );
        }
    break;

  case 672: // RenameExpr: "'rename'" "'node'" ExprSingle "'as'" ExprSingle
        {
            (yylhs.value.expr) = new RenameExpr( LOC (yylhs.location), (yystack_[2].value.expr), (yystack_[0].value.expr) );
        }
    break;

  case 673: // TransformExpr: "'copy'" "'$'" VarNameList "'modify'" ExprSingle ReturnOrSelect ExprSingle
    {
      CopyVarList *cvl = dynamic_cast<CopyVarList*>((yystack_[4].value.expr));
      (yylhs.value.expr) = new TransformExpr( LOC(yylhs.location), cvl, (yystack_[2].value.expr), (yystack_[0].value.expr) );
    }
    break;

  case 674: // VarNameList: VarNameDecl
    {
      CopyVarList* lList = new CopyVarList(LOC(yylhs.location));
      lList->push_back (dynamic_cast<VarBinding*> ((yystack_[0].value.expr)));
      (yylhs.value.expr) = lList;
    }
    break;

  case 675: // VarNameList: VarNameList "','" "'$'" VarNameDecl
    {
      CopyVarList* lList = dynamic_cast<CopyVarList*>((yystack_[3].value.expr));
      VarBinding* lBinding = dynamic_cast<VarBinding*>((yystack_[0].value.expr));
      lList->push_back(lBinding);
      (yylhs.value.expr) = lList;
    }
    break;

  case 676: // VarNameDecl: QNAME "':='" ExprSingle
    {
       (yylhs.value.expr) = new VarBinding(LOC(yylhs.location), static_cast<QName*>((yystack_[2].value.expr)), (yystack_[0].value.expr));
    }
    break;

  case 677: // TryExpr: "'try'" "'{'" Expr "'}'" CatchListExpr
        {
            (yylhs.value.expr) = new TryExpr( LOC(yylhs.location), (yystack_[2].value.expr), (yystack_[0].value.expr) );
        }
    break;

  case 678: // CatchListExpr: CatchExpr
        {
            CatchListExpr *cle = new CatchListExpr( LOC(yylhs.location) );
            cle->push_back( static_cast<CatchExpr*>((yystack_[0].value.expr)) );
            (yylhs.value.expr) = cle;
        }
    break;

  case 679: // CatchListExpr: CatchListExpr CatchExpr
        {
            CatchListExpr *cle = dynamic_cast<CatchListExpr*>((yystack_[1].value.expr));
            if ( cle )
                cle->push_back( static_cast<CatchExpr*>((yystack_[0].value.expr)) );
            (yylhs.value.expr) = (yystack_[1].value.expr);
        }
    break;

  case 680: // CatchExpr: "'catch'" NameTestList BracedExpr
    {
       (yylhs.value.expr) = new CatchExpr(LOC(yylhs.location), *(yystack_[1].value.name_test_list), (yystack_[0].value.expr));
       delete (yystack_[1].value.name_test_list);
    }
    break;

  case 681: // BracedExpr: "'{'" Expr "'}'"
    {
      (yylhs.value.expr) = (yystack_[1].value.expr);
    }
    break;

  case 682: // NameTestList: NameTest
        {
            CatchExpr::NameTestList *ntl = new CatchExpr::NameTestList;
            ntl->push_back( static_cast<NameTest*>((yystack_[0].value.node)) );
            (yylhs.value.name_test_list) = ntl;
        }
    break;

  case 683: // NameTestList: NameTestList "'|'" NameTest
        {
            CatchExpr::NameTestList *ntl =
                static_cast<CatchExpr::NameTestList*>((yystack_[2].value.name_test_list));
            ntl->push_back( static_cast<NameTest*>((yystack_[0].value.node)) );
            (yylhs.value.name_test_list) = ntl;
        }
    break;

  case 684: // FTSelection: FTOr opt_FTPosFilter_list
        {
            (yylhs.value.node) = new FTSelection( LOC(yylhs.location), (yystack_[1].value.node), (yystack_[0].value.pos_filter_list) );
            delete (yystack_[0].value.pos_filter_list);
        }
    break;

  case 685: // opt_FTPosFilter_list: %empty
        {
            (yylhs.value.pos_filter_list) = NULL;
        }
    break;

  case 686: // opt_FTPosFilter_list: FTPosFilter_list
        {
            (yylhs.value.pos_filter_list) = (yystack_[0].value.pos_filter_list);
        }
    break;

  case 687: // FTPosFilter_list: FTPosFilter
        {
            (yylhs.value.pos_filter_list) = new FTSelection::pos_filter_list_t;
            (yylhs.value.pos_filter_list)->push_back( dynamic_cast<FTPosFilter*>((yystack_[0].value.node)) );
        }
    break;

  case 688: // FTPosFilter_list: FTPosFilter_list FTPosFilter
        {
            (yystack_[1].value.pos_filter_list)->push_back( dynamic_cast<FTPosFilter*>((yystack_[0].value.node)) );
            (yylhs.value.pos_filter_list) = (yystack_[1].value.pos_filter_list);
        }
    break;

  case 689: // FTOr: FTAnd
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 690: // FTOr: FTOr "'ftor'" FTAnd
        {
            (yylhs.value.node) = new FTOr( LOC(yylhs.location), (yystack_[2].value.node), (yystack_[0].value.node) );
        }
    break;

  case 691: // FTAnd: FTMildNot
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 692: // FTAnd: FTAnd "'ftand'" FTMildNot
        {
            (yylhs.value.node) = new FTAnd( LOC(yylhs.location), (yystack_[2].value.node), (yystack_[0].value.node) );
        }
    break;

  case 693: // FTMildNot: FTUnaryNot
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 694: // FTMildNot: FTMildNot "'not'" "'in'" FTUnaryNot
        {
            (yylhs.value.node) = new FTMildNot( LOC(yylhs.location), (yystack_[3].value.node), (yystack_[0].value.node) );
        }
    break;

  case 695: // FTUnaryNot: FTPrimaryWithOptions
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 696: // FTUnaryNot: "'ftnot'" FTPrimaryWithOptions
        {
            (yylhs.value.node) = new FTUnaryNot(
                LOC(yylhs.location), dynamic_cast<FTPrimaryWithOptions*>((yystack_[0].value.node))
            );
        }
    break;

  case 697: // FTPrimaryWithOptions: FTPrimary opt_FTMatchOptions opt_FTWeight
        {
            (yylhs.value.node) = new FTPrimaryWithOptions(
                LOC(yylhs.location),
                dynamic_cast<FTPrimary*>((yystack_[2].value.node)),
                dynamic_cast<FTMatchOptions*>((yystack_[1].value.node)),
                dynamic_cast<FTWeight*>((yystack_[0].value.node))
            );
        }
    break;

  case 698: // opt_FTMatchOptions: %empty
        {
            (yylhs.value.node) = NULL;
        }
    break;

  case 699: // opt_FTMatchOptions: FTMatchOptions
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 700: // opt_FTWeight: %empty
        {
            (yylhs.value.node) = NULL;
        }
    break;

  case 701: // opt_FTWeight: FTWeight
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 702: // FTWeight: "'weight'" "'{'" Expr "'}'"
        {
            (yylhs.value.node) = new FTWeight( LOC(yylhs.location), dynamic_cast<exprnode*>((yystack_[1].value.expr)) );
        }
    break;

  case 703: // FTPrimary: FTWords opt_FTTimes
        {
            (yylhs.value.node) = new FTWordsTimes(
                LOC(yylhs.location),
                dynamic_cast<FTWords*>((yystack_[1].value.node)),
                dynamic_cast<FTTimes*>((yystack_[0].value.node))
            );
        }
    break;

  case 704: // FTPrimary: "'('" FTSelection "')'"
        {
            (yylhs.value.node) = (yystack_[1].value.node);
        }
    break;

  case 705: // FTPrimary: FTExtensionSelection
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 706: // opt_FTTimes: %empty
        {
            (yylhs.value.node) = NULL;
        }
    break;

  case 707: // opt_FTTimes: FTTimes
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 708: // FTExtensionSelection: Pragma_list "'{'" opt_FTSelection "'}'"
        {
            (yylhs.value.node) = new FTExtensionSelection(
                LOC(yylhs.location),
                dynamic_cast<PragmaList*>((yystack_[3].value.node)),
                dynamic_cast<FTSelection*>((yystack_[1].value.node))
            );
        }
    break;

  case 709: // opt_FTSelection: %empty
        {
            (yylhs.value.node) = NULL;
        }
    break;

  case 710: // opt_FTSelection: FTSelection
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 711: // FTWords: FTWordsValue opt_FTAnyallOption
        {
            (yylhs.value.node) = new FTWords(
                LOC(yylhs.location),
                dynamic_cast<FTWordsValue*>((yystack_[1].value.node)),
                dynamic_cast<FTAnyallOption*>((yystack_[0].value.node))
            );
        }
    break;

  case 712: // FTWordsValue: StringLiteral
        {
            (yylhs.value.node) = new FTWordsValue(
                LOC(yylhs.location), static_cast<StringLiteral*>((yystack_[0].value.expr)), NULL
            );
        }
    break;

  case 713: // FTWordsValue: "'{'" Expr "'}'"
        {
            (yylhs.value.node) = new FTWordsValue(
                LOC(yylhs.location), NULL, dynamic_cast<exprnode*>((yystack_[1].value.expr))
            );
        }
    break;

  case 714: // opt_FTAnyallOption: %empty
        {
            (yylhs.value.node) = new FTAnyallOption( LOC(yylhs.location), ft_anyall_mode::any );
        }
    break;

  case 715: // opt_FTAnyallOption: FTAnyallOption
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 716: // FTAnyallOption: "'any'" opt_word
        {
            (yylhs.value.node) = new FTAnyallOption( LOC(yylhs.location), (yystack_[0].value.ft_anyall_value) );
        }
    break;

  case 717: // FTAnyallOption: "'all'" opt_words
        {
            (yylhs.value.node) = new FTAnyallOption( LOC(yylhs.location), (yystack_[0].value.ft_anyall_value) );
        }
    break;

  case 718: // FTAnyallOption: "'phrase'"
        {
            (yylhs.value.node) = new FTAnyallOption( LOC(yylhs.location), ft_anyall_mode::phrase );
        }
    break;

  case 719: // opt_word: %empty
        {
            (yylhs.value.ft_anyall_value) = ft_anyall_mode::any;
        }
    break;

  case 720: // opt_word: "'word'"
        {
            (yylhs.value.ft_anyall_value) = ft_anyall_mode::any_word;
        }
    break;

  case 721: // opt_words: %empty
        {
            (yylhs.value.ft_anyall_value) = ft_anyall_mode::all;
        }
    break;

  case 722: // opt_words: "'words'"
        {
            (yylhs.value.ft_anyall_value) = ft_anyall_mode::all_words;
        }
    break;

  case 723: // FTPosFilter: FTOrder
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 724: // FTPosFilter: FTWindow
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 725: // FTPosFilter: FTDistance
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 726: // FTPosFilter: FTScope
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 727: // FTPosFilter: FTContent
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 728: // FTOrder: "'ordered'"
        {
            (yylhs.value.node) = new FTOrder( LOC(yylhs.location) );
        }
    break;

  case 729: // FTWindow: "'window'" AdditiveExpr FTUnit
        {
            (yylhs.value.node) = new FTWindow(
                LOC(yylhs.location),
                static_cast<AdditiveExpr*>((yystack_[1].value.expr)),
                static_cast<FTUnit*>((yystack_[0].value.node))
            );
        }
    break;

  case 730: // FTDistance: "'distance'" FTRange FTUnit
        {
            (yylhs.value.node) = new FTDistance(
                LOC(yylhs.location),
                dynamic_cast<FTRange*>((yystack_[1].value.node)),
                dynamic_cast<FTUnit*>((yystack_[0].value.node))
            );
        }
    break;

  case 731: // FTUnit: "'words'"
        {
            (yylhs.value.node) = new FTUnit( LOC(yylhs.location), ft_unit::words );
        }
    break;

  case 732: // FTUnit: "'sentences'"
        {
            (yylhs.value.node) = new FTUnit( LOC(yylhs.location), ft_unit::sentences );
        }
    break;

  case 733: // FTUnit: "'paragraphs'"
        {
            (yylhs.value.node) = new FTUnit( LOC(yylhs.location), ft_unit::paragraphs );
        }
    break;

  case 734: // FTMatchOptions: "'using'" FTMatchOption
        {
            FTMatchOptions *mo = new FTMatchOptions( LOC(yylhs.location) );
            mo->push_back( dynamic_cast<FTMatchOption*>((yystack_[0].value.node)) );
            (yylhs.value.node) = mo;
        }
    break;

  case 735: // FTMatchOptions: FTMatchOptions "'using'" FTMatchOption
        {
            FTMatchOptions *mo = dynamic_cast<FTMatchOptions*>((yystack_[2].value.node));
            mo->push_back( dynamic_cast<FTMatchOption*>((yystack_[0].value.node)) );
            (yylhs.value.node) = (yystack_[2].value.node);
        }
    break;

  case 736: // FTMatchOption: FTCaseOption
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 737: // FTMatchOption: FTDiacriticsOption
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 738: // FTMatchOption: FTExtensionOption
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 739: // FTMatchOption: FTLanguageOption
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 740: // FTMatchOption: FTStemOption
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 741: // FTMatchOption: FTStopWordOption
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 742: // FTMatchOption: FTThesaurusOption
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 743: // FTMatchOption: FTWildCardOption
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 744: // FTCaseOption: "'case'" "'sensitive'"
        {
            (yylhs.value.node) = new FTCaseOption( LOC(yylhs.location), ft_case_mode::sensitive );
        }
    break;

  case 745: // FTCaseOption: "'case'" "'insensitive'"
        {
            (yylhs.value.node) = new FTCaseOption( LOC(yylhs.location), ft_case_mode::insensitive );
        }
    break;

  case 746: // FTCaseOption: "'lowercase'"
        {
            (yylhs.value.node) = new FTCaseOption( LOC(yylhs.location), ft_case_mode::lower );
        }
    break;

  case 747: // FTCaseOption: "'uppercase'"
        {
            (yylhs.value.node) = new FTCaseOption( LOC(yylhs.location), ft_case_mode::upper );
        }
    break;

  case 748: // FTDiacriticsOption: "'diacritics'" "'sensitive'"
        {
            (yylhs.value.node) = new FTDiacriticsOption(
                LOC(yylhs.location), ft_diacritics_mode::sensitive
            );
        }
    break;

  case 749: // FTDiacriticsOption: "'diacritics'" "'insensitive'"
        {
            (yylhs.value.node) = new FTDiacriticsOption(
                LOC(yylhs.location), ft_diacritics_mode::insensitive
            );
        }
    break;

  case 750: // FTExtensionOption: "'option'" QNAME "'STRING'"
        {
            (yylhs.value.node) = new FTExtensionOption(
                LOC(yylhs.location), static_cast<QName*>((yystack_[1].value.expr)), SYMTAB((yystack_[0].value.sval))
            );
        }
    break;

  case 751: // FTStemOption: "'stemming'"
        {
            (yylhs.value.node) = new FTStemOption( LOC(yylhs.location), ft_stem_mode::stemming );
        }
    break;

  case 752: // FTStemOption: "'no'" "'stemming'"
        {
            (yylhs.value.node) = new FTStemOption( LOC(yylhs.location), ft_stem_mode::no_stemming );
        }
    break;

  case 753: // FTThesaurusOption: "'thesaurus'" FTThesaurusID_or_default
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

  case 754: // FTThesaurusOption: "'thesaurus'" "'('" FTThesaurusID_or_default opt_FTThesaurus_list "')'"
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

  case 755: // FTThesaurusOption: "'no'" "'thesaurus'"
        {
            (yylhs.value.node) = new FTThesaurusOption( LOC(yylhs.location), NULL, false, true );
        }
    break;

  case 756: // FTThesaurusID_or_default: FTThesaurusID
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 757: // FTThesaurusID_or_default: "'default'"
        {
            (yylhs.value.node) = NULL;
        }
    break;

  case 758: // opt_FTThesaurus_list: %empty
        {
            (yylhs.value.thesaurus_id_list) = NULL;
        }
    break;

  case 759: // opt_FTThesaurus_list: "','" FTThesaurus_list
        {
            (yylhs.value.thesaurus_id_list) = (yystack_[0].value.thesaurus_id_list);
        }
    break;

  case 760: // FTThesaurus_list: FTThesaurusID
        {
            (yylhs.value.thesaurus_id_list) = new FTThesaurusOption::thesaurus_id_list_t;
            (yylhs.value.thesaurus_id_list)->push_back( dynamic_cast<FTThesaurusID*>((yystack_[0].value.node)) );
        }
    break;

  case 761: // FTThesaurus_list: FTThesaurus_list "','" FTThesaurusID
        {
            (yystack_[2].value.thesaurus_id_list)->push_back( dynamic_cast<FTThesaurusID*>((yystack_[0].value.node)) );
            (yylhs.value.thesaurus_id_list) = (yystack_[2].value.thesaurus_id_list);
        }
    break;

  case 762: // FTThesaurusID: "'at'" "'STRING'" opt_relationship opt_levels
        {
            (yylhs.value.node) = new FTThesaurusID(
                LOC(yylhs.location), SYMTAB((yystack_[2].value.sval)), SYMTAB((yystack_[1].value.sval)), dynamic_cast<FTRange*>((yystack_[0].value.node))
            );
        }
    break;

  case 763: // opt_relationship: %empty
        {
            (yylhs.value.sval) = 0;
        }
    break;

  case 764: // opt_relationship: "'relationship'" "'STRING'"
        {
            (yylhs.value.sval) = (yystack_[0].value.sval);
        }
    break;

  case 765: // opt_levels: %empty
        {
            (yylhs.value.node) = NULL;
        }
    break;

  case 766: // opt_levels: FTRange "'levels'"
        {
            (yylhs.value.node) = (yystack_[1].value.node);
        }
    break;

  case 767: // FTStopWordOption: "'stop'" "'words'" FTStopWords opt_FTStopWordsInclExcl_list
        {
            (yylhs.value.node) = new FTStopWordOption(
                LOC(yylhs.location),
                dynamic_cast<FTStopWords*>((yystack_[1].value.node)), (yystack_[0].value.incl_excl_list),
                ft_stop_words_mode::with
            );
            delete (yystack_[0].value.incl_excl_list);
        }
    break;

  case 768: // FTStopWordOption: "'stop'" "'words'" "'default'" opt_FTStopWordsInclExcl_list
        {
            (yylhs.value.node) = new FTStopWordOption(
                LOC(yylhs.location), NULL, (yystack_[0].value.incl_excl_list), ft_stop_words_mode::with_default
            );
            delete (yystack_[0].value.incl_excl_list);
        }
    break;

  case 769: // FTStopWordOption: "'no'" "'stop'" "'words'"
        {
            (yylhs.value.node) = new FTStopWordOption(
                LOC(yylhs.location), NULL, NULL, ft_stop_words_mode::without
            );
        }
    break;

  case 770: // FTStopWords: "'at'" "'STRING'"
        {
            (yylhs.value.node) = new FTStopWords( LOC(yylhs.location), SYMTAB((yystack_[0].value.sval)), NULL );
        }
    break;

  case 771: // FTStopWords: "'('" STRING_LITERAL_list "')'"
        {
            (yylhs.value.node) = new FTStopWords( LOC(yylhs.location), "", (yystack_[1].value.strlist) );
        }
    break;

  case 772: // STRING_LITERAL_list: "'STRING'"
        {
            string_list_t *sl = new string_list_t;
            sl->push_back( SYMTAB((yystack_[0].value.sval)) );
            (yylhs.value.strlist) = sl;
        }
    break;

  case 773: // STRING_LITERAL_list: STRING_LITERAL_list "','" "'STRING'"
        {
            if ( (yystack_[2].value.strlist) )
                (yystack_[2].value.strlist)->push_back( SYMTAB((yystack_[0].value.sval)) );
            (yylhs.value.strlist) = (yystack_[2].value.strlist);
        }
    break;

  case 774: // opt_FTStopWordsInclExcl_list: %empty
        {
            (yylhs.value.incl_excl_list) = NULL;
        }
    break;

  case 775: // opt_FTStopWordsInclExcl_list: FTStopWordsInclExcl_list
        {
            (yylhs.value.incl_excl_list) = (yystack_[0].value.incl_excl_list);
        }
    break;

  case 776: // FTStopWordsInclExcl_list: FTStopWordsInclExcl
        {
            (yylhs.value.incl_excl_list) = new FTStopWordOption::incl_excl_list_t;
            (yylhs.value.incl_excl_list)->push_back( dynamic_cast<FTStopWordsInclExcl*>((yystack_[0].value.node)) );
        }
    break;

  case 777: // FTStopWordsInclExcl_list: FTStopWordsInclExcl_list FTStopWordsInclExcl
        {
            FTStopWordOption::incl_excl_list_t *iel = (yystack_[1].value.incl_excl_list);
            if ( !iel )
                iel = new FTStopWordOption::incl_excl_list_t;
            iel->push_back( dynamic_cast<FTStopWordsInclExcl*>((yystack_[0].value.node)) );
            (yylhs.value.incl_excl_list) = iel;
        }
    break;

  case 778: // FTStopWordsInclExcl: "'union'" FTStopWords
        {
            (yylhs.value.node) = new FTStopWordsInclExcl(
                LOC(yylhs.location),
                dynamic_cast<FTStopWords*>((yystack_[0].value.node)),
                ft_stop_words_unex::union_
            );
        }
    break;

  case 779: // FTStopWordsInclExcl: "'except'" FTStopWords
        {
            (yylhs.value.node) = new FTStopWordsInclExcl(
                LOC(yylhs.location),
                dynamic_cast<FTStopWords*>((yystack_[0].value.node)),
                ft_stop_words_unex::except
            );
        }
    break;

  case 780: // FTLanguageOption: "'language'" "'STRING'"
        {
            (yylhs.value.node) = new FTLanguageOption( LOC(yylhs.location), SYMTAB((yystack_[0].value.sval)) );
        }
    break;

  case 781: // FTWildCardOption: "'wildcards'"
        {
            (yylhs.value.node) = new FTWildCardOption( LOC(yylhs.location), ft_wild_card_mode::with );
        }
    break;

  case 782: // FTWildCardOption: "'no'" "'wildcards'"
        {
            (yylhs.value.node) = new FTWildCardOption( LOC(yylhs.location), ft_wild_card_mode::without );
        }
    break;

  case 783: // FTContent: "'at'" "'start'"
        {
            (yylhs.value.node) = new FTContent( LOC(yylhs.location), ft_content_mode::at_start );
        }
    break;

  case 784: // FTContent: "'at'" "'end'"
        {
            (yylhs.value.node) = new FTContent( LOC(yylhs.location), ft_content_mode::at_end );
        }
    break;

  case 785: // FTContent: "'entire'" "'content'"
        {
            (yylhs.value.node) = new FTContent( LOC(yylhs.location), ft_content_mode::entire );
        }
    break;

  case 786: // FTTimes: "'occurs'" FTRange "'times'"
        {
            (yylhs.value.node) = new FTTimes( LOC(yylhs.location), dynamic_cast<FTRange*>((yystack_[1].value.node)) );
        }
    break;

  case 787: // FTRange: "'exactly'" AdditiveExpr
        {
            (yylhs.value.node) = new FTRange( LOC(yylhs.location), ft_range_mode::exactly, (yystack_[0].value.expr) );
        }
    break;

  case 788: // FTRange: "'at'" "'least'" AdditiveExpr
        {
            (yylhs.value.node) = new FTRange( LOC(yylhs.location), ft_range_mode::at_least, (yystack_[0].value.expr) );
        }
    break;

  case 789: // FTRange: "'at'" "'most'" AdditiveExpr
        {
            (yylhs.value.node) = new FTRange( LOC(yylhs.location), ft_range_mode::at_most, (yystack_[0].value.expr) );
        }
    break;

  case 790: // FTRange: "'from'" AdditiveExpr "'to'" AdditiveExpr
        {
            (yylhs.value.node) = new FTRange( LOC(yylhs.location), ft_range_mode::from_to, (yystack_[2].value.expr), (yystack_[0].value.expr) );
        }
    break;

  case 791: // FTScope: "'same'" FTBigUnit
        {
            (yylhs.value.node) = new FTScope(
                LOC(yylhs.location),
                ft_scope::same,
                dynamic_cast<FTBigUnit*>((yystack_[0].value.node))
            );
        }
    break;

  case 792: // FTScope: "'different'" FTBigUnit
        {
            (yylhs.value.node) = new FTScope(
                LOC(yylhs.location),
                ft_scope::different,
                dynamic_cast<FTBigUnit*>((yystack_[0].value.node))
            );
        }
    break;

  case 793: // FTBigUnit: "'sentence'"
        {
            (yylhs.value.node) = new FTBigUnit( LOC(yylhs.location), ft_big_unit::sentence );
        }
    break;

  case 794: // FTBigUnit: "'paragraph'"
        {
            (yylhs.value.node) = new FTBigUnit( LOC(yylhs.location), ft_big_unit::paragraph );
        }
    break;

  case 795: // FTIgnoreOption: "'without'" "'content'" UnionExpr
        {
            (yylhs.value.node) = new FTIgnoreOption( LOC(yylhs.location), static_cast<UnionExpr*>((yystack_[0].value.expr)) );
        }
    break;

  case 796: // JSONArrayConstructor: "'['" "']'"
        {
          (yylhs.value.expr) = new JSONArrayConstructor( LOC(yylhs.location), NULL );
        }
    break;

  case 797: // JSONArrayConstructor: "'['" Expr "']'"
        {
          (yylhs.value.expr) = new JSONArrayConstructor( LOC(yylhs.location), (yystack_[1].value.expr) );
        }
    break;

  case 798: // JSONSimpleObjectUnion: "'{|'" "'|}'"
        {
          (yylhs.value.expr) = new JSONObjectConstructor(LOC(yylhs.location), NULL, false);
        }
    break;

  case 799: // JSONSimpleObjectUnion: "'{|'" Expr "'|}'"
        {
          (yylhs.value.expr) = new JSONObjectConstructor(LOC(yylhs.location), (yystack_[1].value.expr), false);
        }
    break;

  case 800: // JSONAccumulatorObjectUnion: "'{['" "']}'"
        {
          (yylhs.value.expr) = new JSONObjectConstructor(LOC(yylhs.location), NULL, true);
        }
    break;

  case 801: // JSONAccumulatorObjectUnion: "'{['" Expr "']}'"
        {
          (yylhs.value.expr) = new JSONObjectConstructor(LOC(yylhs.location), (yystack_[1].value.expr), true);
        }
    break;

  case 802: // JSONObjectConstructor: "'{'" JSONPairList "'}'"
        {
          (yylhs.value.expr) = new JSONDirectObjectConstructor(LOC(yylhs.location),
                                               dynamic_cast<JSONPairList*>((yystack_[1].value.node)));
        }
    break;

  case 803: // JSONPairList: QNAME "':'" ExprSingle
        {
          ERROR_IF_QNAME_NOT_NCNAME((yystack_[2].value.expr), yystack_[2].location, (yystack_[0].value.expr));
          driver.addCommonLanguageWarning(yystack_[2].location, ZED(ZWST0009_OBJECT_KEY_NOT_QUOTED));
          StringLiteral* sl = new StringLiteral( LOC(yylhs.location), static_cast<QName*>((yystack_[2].value.expr)) );
          JSONPairList* jpl = new JSONPairList(LOC(yylhs.location));
          jpl->push_back(new JSONPairConstructor(LOC(yylhs.location), sl, (yystack_[0].value.expr)));
          (yylhs.value.node) = jpl;
        }
    break;

  case 804: // JSONPairList: ExprSingle "':'" ExprSingle
        {
          JSONPairList* jpl = new JSONPairList(LOC(yylhs.location));
          jpl->push_back(new JSONPairConstructor(LOC(yylhs.location), (yystack_[2].value.expr), (yystack_[0].value.expr)));
          (yylhs.value.node) = jpl;
        }
    break;

  case 805: // JSONPairList: JSONPairList "','" ExprSingle "':'" ExprSingle
        {
          JSONPairList* jpl = dynamic_cast<JSONPairList*>((yystack_[4].value.node));
          assert(jpl);
          jpl->push_back(new JSONPairConstructor(LOC(yylhs.location), (yystack_[2].value.expr), (yystack_[0].value.expr)));
          (yylhs.value.node) = jpl;
        }
    break;

  case 806: // JSONPairList: JSONPairList "','" QNAME "':'" ExprSingle
        {
          ERROR_IF_QNAME_NOT_NCNAME((yystack_[2].value.expr), yystack_[2].location, (yystack_[4].value.node), (yystack_[0].value.expr));
          JSONPairList* jpl = dynamic_cast<JSONPairList*>((yystack_[4].value.node));
          assert(jpl);
          StringLiteral* sl = new StringLiteral( LOC(yylhs.location), static_cast<QName*>((yystack_[2].value.expr)) );
          jpl->push_back(new JSONPairConstructor(LOC(yylhs.location), sl, (yystack_[0].value.expr)));
          (yylhs.value.node) = jpl;
        }
    break;

  case 807: // JSONInsertExpr: "'insert'" "'json'" ExprSingle "'into'" ExprSingle
        {
          (yylhs.value.expr) = new JSONObjectInsertExpr(LOC(yylhs.location),
                                        (yystack_[2].value.expr),
                                        (yystack_[0].value.expr));
        }
    break;

  case 808: // JSONInsertExpr: "'insert'" "'json'" JSONPairList "'into'" ExprSingle
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

  case 809: // JSONInsertExpr: "'insert'" "'json'" ExprSingle "'into'" ExprSingle "'at'" "'position'" ExprSingle
        {
          (yylhs.value.expr) = new JSONArrayInsertExpr(LOC(yylhs.location), (yystack_[5].value.expr), (yystack_[3].value.expr), (yystack_[0].value.expr));
        }
    break;

  case 810: // JSONAppendExpr: "'append'" "'json'" ExprSingle "'into'" ExprSingle
        {
          (yylhs.value.expr) = new JSONArrayAppendExpr(LOC(yylhs.location), (yystack_[2].value.expr), (yystack_[0].value.expr));
        }
    break;

  case 811: // JSONDeleteExpr: "'delete'" "'json'" PostfixExpr
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

  case 812: // JSONRenameExpr: "'rename'" "'json'" PostfixExpr "'as'" ExprSingle
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

  case 813: // JSONReplaceExpr: "'replace'" "'value'" "'of'" "'json'" PostfixExpr "'with'" ExprSingle
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

  case 814: // JSONTest: "'json-item'" "'('" "')'"
        {
          (yylhs.value.node) = new JSON_Test(LOC(yylhs.location), store::StoreConsts::jsonItem);
        }
    break;

  case 815: // JSONTest: "'array'" "'('" "')'"
        {
          (yylhs.value.node) = new JSON_Test(LOC(yylhs.location), store::StoreConsts::jsonArray);
        }
    break;

  case 816: // JSONTest: "'object'" "'('" "')'"
        {
          (yylhs.value.node) = new JSON_Test(LOC(yylhs.location), store::StoreConsts::jsonObject);
        }
    break;

  case 817: // URI_LITERAL: "'STRING'"
        { (yylhs.value.sval) = (yystack_[0].value.sval); }
    break;

  case 818: // QNAME: FUNCTION_NAME
        { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 819: // QNAME: "'NCName_sval'"
                  { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB((yystack_[0].value.sval))); }
    break;

  case 820: // QNAME: "'attribute'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("attribute"))); }
    break;

  case 821: // QNAME: "'comment'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("comment"))); }
    break;

  case 822: // QNAME: "'document-node'"
                    { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("document-node"))); }
    break;

  case 823: // QNAME: "'namespace-node'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("namespace-node"))); }
    break;

  case 824: // QNAME: "'element'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("element"))); }
    break;

  case 825: // QNAME: "'item'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("item"))); }
    break;

  case 826: // QNAME: "'if'"
         { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("if"))); }
    break;

  case 827: // QNAME: "'node'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("node"))); }
    break;

  case 828: // QNAME: "'processing-instruction'"
                             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("processing-instruction"))); }
    break;

  case 829: // QNAME: "'schema-attribute'"
                       { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("schema-attribute"))); }
    break;

  case 830: // QNAME: "'schema-element'"
                     { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("schema-element"))); }
    break;

  case 831: // QNAME: "'text'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("text"))); }
    break;

  case 832: // QNAME: "'typeswitch'"
                 { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("typeswitch"))); }
    break;

  case 833: // QNAME: "'switch'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("switch"))); }
    break;

  case 834: // QNAME: "'empty-sequence'"
                     { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("empty-sequence"))); }
    break;

  case 835: // QNAME: "'while'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("while"))); }
    break;

  case 836: // QNAME: "'json'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("json"))); }
    break;

  case 837: // QNAME: "'array'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("array"))); }
    break;

  case 838: // QNAME: "'object'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("object"))); }
    break;

  case 839: // QNAME: "'json-item'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("json-item"))); }
    break;

  case 840: // QNAME: "'structured-item'"
                      { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("structured-item"))); }
    break;

  case 841: // FUNCTION_NAME: EQNAME
        { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 842: // FUNCTION_NAME: "'QName'"
                 { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB((yystack_[0].value.sval))); }
    break;

  case 843: // FUNCTION_NAME: "'xquery'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("xquery"))); }
    break;

  case 844: // FUNCTION_NAME: "'empty'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("empty"))); }
    break;

  case 845: // FUNCTION_NAME: "'boundary-space'"
                     { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("boundary-space"))); }
    break;

  case 846: // FUNCTION_NAME: "'ft-option'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("ft-option"))); }
    break;

  case 847: // FUNCTION_NAME: "'base-uri'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("base-uri"))); }
    break;

  case 848: // FUNCTION_NAME: "'lax'"
          { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("lax"))); }
    break;

  case 849: // FUNCTION_NAME: "'strict'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("strict"))); }
    break;

  case 850: // FUNCTION_NAME: "'idiv'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("idiv"))); }
    break;

  case 851: // FUNCTION_NAME: "'document'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("document"))); }
    break;

  case 852: // FUNCTION_NAME: "'ftnot'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("ftnot"))); }
    break;

  case 853: // FUNCTION_NAME: "'not'"
          { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("not"))); }
    break;

  case 854: // FUNCTION_NAME: "'sensitive'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("sensitive"))); }
    break;

  case 855: // FUNCTION_NAME: "'insensitive'"
                  { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("insensitive"))); }
    break;

  case 856: // FUNCTION_NAME: "'diacritics'"
                 { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("diacritics"))); }
    break;

  case 857: // FUNCTION_NAME: "'without'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("without"))); }
    break;

  case 858: // FUNCTION_NAME: "'stemming'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("stemming"))); }
    break;

  case 859: // FUNCTION_NAME: "'thesaurus'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("thesaurus"))); }
    break;

  case 860: // FUNCTION_NAME: "'stop'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("stop"))); }
    break;

  case 861: // FUNCTION_NAME: "'wildcards'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("wildcards"))); }
    break;

  case 862: // FUNCTION_NAME: "'entire'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("entire"))); }
    break;

  case 863: // FUNCTION_NAME: "'content'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("content"))); }
    break;

  case 864: // FUNCTION_NAME: "'word'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("word"))); }
    break;

  case 865: // FUNCTION_NAME: "'start'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("start"))); }
    break;

  case 866: // FUNCTION_NAME: "'end'"
          { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("end"))); }
    break;

  case 867: // FUNCTION_NAME: "'most'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("most"))); }
    break;

  case 868: // FUNCTION_NAME: "'skip'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("skip"))); }
    break;

  case 869: // FUNCTION_NAME: "'copy'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("copy"))); }
    break;

  case 870: // FUNCTION_NAME: "'general'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("general"))); }
    break;

  case 871: // FUNCTION_NAME: "'value'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("value"))); }
    break;

  case 872: // FUNCTION_NAME: "'eq'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("eq"))); }
    break;

  case 873: // FUNCTION_NAME: "'ne'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("ne"))); }
    break;

  case 874: // FUNCTION_NAME: "'lt'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("lt"))); }
    break;

  case 875: // FUNCTION_NAME: "'le'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("le"))); }
    break;

  case 876: // FUNCTION_NAME: "'gt'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("gt"))); }
    break;

  case 877: // FUNCTION_NAME: "'ge'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("ge"))); }
    break;

  case 878: // FUNCTION_NAME: "'at'"
         { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("at"))); }
    break;

  case 879: // FUNCTION_NAME: "'context'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("context"))); }
    break;

  case 880: // FUNCTION_NAME: "'variable'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("variable"))); }
    break;

  case 881: // FUNCTION_NAME: "'return'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("return"))); }
    break;

  case 882: // FUNCTION_NAME: "'for'"
          { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("for"))); }
    break;

  case 883: // FUNCTION_NAME: "'from'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("from"))); }
    break;

  case 884: // FUNCTION_NAME: "'allowing'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("allowing"))); }
    break;

  case 885: // FUNCTION_NAME: "'sliding'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("sliding"))); }
    break;

  case 886: // FUNCTION_NAME: "'tumbling'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("tumbling"))); }
    break;

  case 887: // FUNCTION_NAME: "'previous'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("previous"))); }
    break;

  case 888: // FUNCTION_NAME: "'next'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("next"))); }
    break;

  case 889: // FUNCTION_NAME: "'only'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("only"))); }
    break;

  case 890: // FUNCTION_NAME: "'when'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("when"))); }
    break;

  case 891: // FUNCTION_NAME: "'count'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("count"))); }
    break;

  case 892: // FUNCTION_NAME: "'in'"
          { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("in"))); }
    break;

  case 893: // FUNCTION_NAME: "'let'"
          { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("let"))); }
    break;

  case 894: // FUNCTION_NAME: "'where'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("where"))); }
    break;

  case 895: // FUNCTION_NAME: "'by'"
         { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("by"))); }
    break;

  case 896: // FUNCTION_NAME: "'group'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("group"))); }
    break;

  case 897: // FUNCTION_NAME: "'order'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("order"))); }
    break;

  case 898: // FUNCTION_NAME: "'stable'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("stable"))); }
    break;

  case 899: // FUNCTION_NAME: "'ascending'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("ascending"))); }
    break;

  case 900: // FUNCTION_NAME: "'descending'"
                 { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("descending"))); }
    break;

  case 901: // FUNCTION_NAME: "'greatest'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("greatest"))); }
    break;

  case 902: // FUNCTION_NAME: "'least'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("least"))); }
    break;

  case 903: // FUNCTION_NAME: "'collation'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("collation"))); }
    break;

  case 904: // FUNCTION_NAME: "'some'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("some"))); }
    break;

  case 905: // FUNCTION_NAME: "'every'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("every"))); }
    break;

  case 906: // FUNCTION_NAME: "'satisfies'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("satisfies"))); }
    break;

  case 907: // FUNCTION_NAME: "'case'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("case"))); }
    break;

  case 908: // FUNCTION_NAME: "'as'"
         { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("as"))); }
    break;

  case 909: // FUNCTION_NAME: "'then'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("then"))); }
    break;

  case 910: // FUNCTION_NAME: "'else'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("else"))); }
    break;

  case 911: // FUNCTION_NAME: "'or'"
         { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("or"))); }
    break;

  case 912: // FUNCTION_NAME: "'and'"
          { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("and"))); }
    break;

  case 913: // FUNCTION_NAME: "'instance'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("instance"))); }
    break;

  case 914: // FUNCTION_NAME: "'of'"
         { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("of"))); }
    break;

  case 915: // FUNCTION_NAME: "'castable'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("castable"))); }
    break;

  case 916: // FUNCTION_NAME: "'to'"
         { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("to"))); }
    break;

  case 917: // FUNCTION_NAME: "'div'"
          { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("div"))); }
    break;

  case 918: // FUNCTION_NAME: "'mod'"
          { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("mod"))); }
    break;

  case 919: // FUNCTION_NAME: "'union'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("union"))); }
    break;

  case 920: // FUNCTION_NAME: "'intersect'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("intersect"))); }
    break;

  case 921: // FUNCTION_NAME: "'except'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("except"))); }
    break;

  case 922: // FUNCTION_NAME: "'validate'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("validate"))); }
    break;

  case 923: // FUNCTION_NAME: "'type'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("type"))); }
    break;

  case 924: // FUNCTION_NAME: "'cast'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("cast"))); }
    break;

  case 925: // FUNCTION_NAME: "'treat'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("treat"))); }
    break;

  case 926: // FUNCTION_NAME: "'is'"
         { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("is"))); }
    break;

  case 927: // FUNCTION_NAME: "'preserve'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("preserve"))); }
    break;

  case 928: // FUNCTION_NAME: "'strip'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("strip"))); }
    break;

  case 929: // FUNCTION_NAME: "'namespace'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("namespace"))); }
    break;

  case 930: // FUNCTION_NAME: "'external'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("external"))); }
    break;

  case 931: // FUNCTION_NAME: "'encoding'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("encoding"))); }
    break;

  case 932: // FUNCTION_NAME: "'<no preserve>'"
                  { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("no-preserve"))); }
    break;

  case 933: // FUNCTION_NAME: "'inherit'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("inherit"))); }
    break;

  case 934: // FUNCTION_NAME: "'<no inherit>'"
                 { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("no-inherit"))); }
    break;

  case 935: // FUNCTION_NAME: "'declare'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("declare"))); }
    break;

  case 936: // FUNCTION_NAME: "'construction'"
                   { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("construction"))); }
    break;

  case 937: // FUNCTION_NAME: "'ordering'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("ordering"))); }
    break;

  case 938: // FUNCTION_NAME: "'default'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("default"))); }
    break;

  case 939: // FUNCTION_NAME: "'copy-namespaces'"
                      { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("copy-namespaces"))); }
    break;

  case 940: // FUNCTION_NAME: "'option'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("option"))); }
    break;

  case 941: // FUNCTION_NAME: "'version'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("version"))); }
    break;

  case 942: // FUNCTION_NAME: "'import'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("import"))); }
    break;

  case 943: // FUNCTION_NAME: "'schema'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("schema"))); }
    break;

  case 944: // FUNCTION_NAME: "'module'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("module"))); }
    break;

  case 945: // FUNCTION_NAME: "'function'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("function"))); }
    break;

  case 946: // FUNCTION_NAME: "'score'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("score"))); }
    break;

  case 947: // FUNCTION_NAME: "'contains'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("contains"))); }
    break;

  case 948: // FUNCTION_NAME: "'weight'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("weight"))); }
    break;

  case 949: // FUNCTION_NAME: "'window'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("window"))); }
    break;

  case 950: // FUNCTION_NAME: "'distance'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("distance"))); }
    break;

  case 951: // FUNCTION_NAME: "'occurs'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("occurs"))); }
    break;

  case 952: // FUNCTION_NAME: "'times'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("times"))); }
    break;

  case 953: // FUNCTION_NAME: "'same'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("same"))); }
    break;

  case 954: // FUNCTION_NAME: "'different'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("different"))); }
    break;

  case 955: // FUNCTION_NAME: "'lowercase'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("lowercase"))); }
    break;

  case 956: // FUNCTION_NAME: "'uppercase'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("uppercase"))); }
    break;

  case 957: // FUNCTION_NAME: "'relationship'"
                   { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("relationship"))); }
    break;

  case 958: // FUNCTION_NAME: "'levels'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("levels"))); }
    break;

  case 959: // FUNCTION_NAME: "'language'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("language"))); }
    break;

  case 960: // FUNCTION_NAME: "'any'"
          { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("any"))); }
    break;

  case 961: // FUNCTION_NAME: "'all'"
          { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("all"))); }
    break;

  case 962: // FUNCTION_NAME: "'exactly'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("exactly"))); }
    break;

  case 963: // FUNCTION_NAME: "'words'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("words"))); }
    break;

  case 964: // FUNCTION_NAME: "'sentences'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("sentences"))); }
    break;

  case 965: // FUNCTION_NAME: "'sentence'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("sentence"))); }
    break;

  case 966: // FUNCTION_NAME: "'phrase'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("phrase"))); }
    break;

  case 967: // FUNCTION_NAME: "'paragraph'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("paragraph"))); }
    break;

  case 968: // FUNCTION_NAME: "'paragraphs'"
                 { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("paragraphs"))); }
    break;

  case 969: // FUNCTION_NAME: "'modify'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("modify"))); }
    break;

  case 970: // FUNCTION_NAME: "'first'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("first"))); }
    break;

  case 971: // FUNCTION_NAME: "'append'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("append"))); }
    break;

  case 972: // FUNCTION_NAME: "'insert'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("insert"))); }
    break;

  case 973: // FUNCTION_NAME: "'replace'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("replace"))); }
    break;

  case 974: // FUNCTION_NAME: "'rename'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("rename"))); }
    break;

  case 975: // FUNCTION_NAME: "'delete'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("delete"))); }
    break;

  case 976: // FUNCTION_NAME: "'before'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("before"))); }
    break;

  case 977: // FUNCTION_NAME: "'after'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("after"))); }
    break;

  case 978: // FUNCTION_NAME: "'revalidation'"
                   { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("revalidation"))); }
    break;

  case 979: // FUNCTION_NAME: "'with'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("with"))); }
    break;

  case 980: // FUNCTION_NAME: "'nodes'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("nodes"))); }
    break;

  case 981: // FUNCTION_NAME: "'last'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("last"))); }
    break;

  case 982: // FUNCTION_NAME: "'into'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("into"))); }
    break;

  case 983: // FUNCTION_NAME: "'simple'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("simple"))); }
    break;

  case 984: // FUNCTION_NAME: "'sequential'"
                 { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("sequential"))); }
    break;

  case 985: // FUNCTION_NAME: "'updating'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("updating"))); }
    break;

  case 986: // FUNCTION_NAME: "'ordered'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("ordered"))); }
    break;

  case 987: // FUNCTION_NAME: "'unordered'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("unordered"))); }
    break;

  case 988: // FUNCTION_NAME: "'returning'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("returning"))); }
    break;

  case 989: // FUNCTION_NAME: "'exit'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("exit"))); }
    break;

  case 990: // FUNCTION_NAME: "'loop'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("loop"))); }
    break;

  case 991: // FUNCTION_NAME: "'break'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("break"))); }
    break;

  case 992: // FUNCTION_NAME: "'continue'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("continue"))); }
    break;

  case 993: // FUNCTION_NAME: "'try'"
          { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("try"))); }
    break;

  case 994: // FUNCTION_NAME: "'catch'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("catch"))); }
    break;

  case 995: // FUNCTION_NAME: "'using'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("using"))); }
    break;

  case 996: // FUNCTION_NAME: "'set'"
          { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("set"))); }
    break;

  case 997: // FUNCTION_NAME: "'index'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("index"))); }
    break;

  case 998: // FUNCTION_NAME: "'unique'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("unique"))); }
    break;

  case 999: // FUNCTION_NAME: "'non'"
          { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("non"))); }
    break;

  case 1000: // FUNCTION_NAME: "'on'"
         { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("on"))); }
    break;

  case 1001: // FUNCTION_NAME: "'range'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("range"))); }
    break;

  case 1002: // FUNCTION_NAME: "'equality'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("equality"))); }
    break;

  case 1003: // FUNCTION_NAME: "'manually'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("manually"))); }
    break;

  case 1004: // FUNCTION_NAME: "'automatically'"
                    { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("automatically"))); }
    break;

  case 1005: // FUNCTION_NAME: "'maintained'"
                 { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("maintained"))); }
    break;

  case 1006: // FUNCTION_NAME: "'decimal-format'"
                     { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("decimal-format"))); }
    break;

  case 1007: // FUNCTION_NAME: "'decimal-separator'"
                        { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("decimal-separator"))); }
    break;

  case 1008: // FUNCTION_NAME: "'grouping-separator'"
                         { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("grouping-separator"))); }
    break;

  case 1009: // FUNCTION_NAME: "'infinity'"
                     { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("infinity"))); }
    break;

  case 1010: // FUNCTION_NAME: "'minus-sign'"
                 { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("minus-sign"))); }
    break;

  case 1011: // FUNCTION_NAME: "'NaN'"
          { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("NaN"))); }
    break;

  case 1012: // FUNCTION_NAME: "'percent'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("percent"))); }
    break;

  case 1013: // FUNCTION_NAME: "'per-mille'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("per-mille"))); }
    break;

  case 1014: // FUNCTION_NAME: "'zero-digit'"
                 { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("zero-digit"))); }
    break;

  case 1015: // FUNCTION_NAME: "'digit'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("digit"))); }
    break;

  case 1016: // FUNCTION_NAME: "'pattern-separator'"
                        { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("pattern-separator"))); }
    break;

  case 1017: // FUNCTION_NAME: "'collection'"
                 { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("collection"))); }
    break;

  case 1018: // FUNCTION_NAME: "'const'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("const"))); }
    break;

  case 1019: // FUNCTION_NAME: "'append-only'"
                  { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("append-only"))); }
    break;

  case 1020: // FUNCTION_NAME: "'queue'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("queue"))); }
    break;

  case 1021: // FUNCTION_NAME: "'mutable'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("mutable"))); }
    break;

  case 1022: // FUNCTION_NAME: "'read-only'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("read-only"))); }
    break;

  case 1023: // FUNCTION_NAME: "'integrity'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("integrity"))); }
    break;

  case 1024: // FUNCTION_NAME: "'constraint'"
                 { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("constraint"))); }
    break;

  case 1025: // FUNCTION_NAME: "'check'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("check"))); }
    break;

  case 1026: // FUNCTION_NAME: "'key'"
          { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("key"))); }
    break;

  case 1027: // FUNCTION_NAME: "'foreach'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("foreach"))); }
    break;

  case 1028: // FUNCTION_NAME: "'foreign'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("foreign"))); }
    break;

  case 1029: // FUNCTION_NAME: "'keys'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("keys"))); }
    break;

  case 1030: // FUNCTION_NAME: "'ancestor'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("ancestor"))); }
    break;

  case 1031: // FUNCTION_NAME: "'child'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("child"))); }
    break;

  case 1032: // FUNCTION_NAME: "'descendant'"
                 { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("descendant"))); }
    break;

  case 1033: // FUNCTION_NAME: "'parent'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("parent"))); }
    break;

  case 1034: // FUNCTION_NAME: "'preceding'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("preceding"))); }
    break;

  case 1035: // FUNCTION_NAME: "'self'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("self"))); }
    break;

  case 1036: // FUNCTION_NAME: "'following'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("following"))); }
    break;

  case 1037: // FUNCTION_NAME: "'ancestor-or-self'"
                       { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("ancestor-or-self"))); }
    break;

  case 1038: // FUNCTION_NAME: "'descendant-or-self'"
                         { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("descendant-or-self"))); }
    break;

  case 1039: // FUNCTION_NAME: "'following-sibling'"
                        { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("following-sibling"))); }
    break;

  case 1040: // FUNCTION_NAME: "'preceding-sibling'"
                        { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("preceding-sibling"))); }
    break;

  case 1041: // FUNCTION_NAME: "'position'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("position"))); }
    break;

  case 1042: // FUNCTION_NAME: "'null'"
                 { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("null"))); }
    break;

  case 1043: // FUNCTION_NAME: "'true'"
                 { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("true"))); }
    break;

  case 1044: // FUNCTION_NAME: "'false'"
                  { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("false"))); }
    break;

  case 1045: // FUNCTION_NAME: "'select'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("select"))); }
    break;

  case 1046: // FUNCTION_NAME: "'jsoniq'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("jsoniq"))); }
    break;

  case 1047: // EQNAME: "'EQName'"
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
  jsoniq_parser::error (const syntax_error& yyexc)
  {
    error (yyexc.location, yyexc.what ());
  }

  /* Return YYSTR after stripping away unnecessary quotes and
     backslashes, so that it's suitable for yyerror.  The heuristic is
     that double-quoting is unnecessary unless the string contains an
     apostrophe, a comma, or backslash (other than backslash-backslash).
     YYSTR is taken from yytname.  */
  std::string
  jsoniq_parser::yytnamerr_ (const char *yystr)
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
  jsoniq_parser::symbol_name (symbol_kind_type yysymbol)
  {
    return yytnamerr_ (yytname_[yysymbol]);
  }



  // jsoniq_parser::context.
  jsoniq_parser::context::context (const jsoniq_parser& yyparser, const symbol_type& yyla)
    : yyparser_ (yyparser)
    , yyla_ (yyla)
  {}

  int
  jsoniq_parser::context::expected_tokens (symbol_kind_type yyarg[], int yyargn) const
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
  jsoniq_parser::yy_syntax_error_arguments_ (const context& yyctx,
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
  jsoniq_parser::yysyntax_error_ (const context& yyctx) const
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


  const short jsoniq_parser::yypact_ninf_ = -1530;

  const short jsoniq_parser::yytable_ninf_ = -884;

  const short
  jsoniq_parser::yypact_[] =
  {
    5520, -1530, -1530, -1530, -1530,  6472,  6472,  6472,  6472, -1530,
   -1530,   217,   255, -1530,   139,     2, -1530, -1530, -1530,   447,
   -1530, -1530, -1530,   246,   385,   786,  5005,   521,   547,   140,
   -1530,    11, -1530,   626, -1530, -1530, -1530, -1530, -1530,   866,
   -1530,   634,   640, -1530, -1530, -1530, -1530,   450, -1530,   868,
   -1530,   646,   706, -1530,   357, -1530, -1530, -1530, -1530, -1530,
   -1530, -1530, -1530, -1530, -1530, -1530, -1530, -1530, -1530,    68,
     654, -1530, -1530, -1530, -1530,   796, 14056, -1530, -1530, -1530,
     759, -1530, -1530, -1530,   792, -1530,   821,   845, -1530, -1530,
   18609, -1530, -1530, -1530, -1530, -1530, -1530, -1530, -1530,   856,
   -1530, -1530,   861,   888, -1530, -1530, -1530, -1530, -1530, -1530,
   -1530,  6472,  9000,  9316, 18609, -1530, -1530,   771, -1530, -1530,
   -1530, -1530,   878, -1530, -1530,   923, 18609, -1530, 15287,   925,
     929, -1530, -1530, -1530, -1530,   943, -1530, -1530, 13108, -1530,
   -1530, -1530, -1530, -1530, -1530,   914, -1530, -1530, -1530, -1530,
   -1530, -1530, -1530, -1530,   123, -1530,   857, -1530, -1530, -1530,
   -1530, -1530, -1530, -1530, -1530, -1530, -1530, -1530,   211,   919,
     263, -1530,  -109,   444, -1530, -1530, -1530, -1530, -1530, -1530,
     960, -1530,   831,   834,   835, -1530, -1530,   924,   932, -1530,
   -1530, -1530, -1530, -1530, -1530, -1530, -1530, -1530, -1530, -1530,
   -1530,  5232, -1530, 13740, -1530, -1530, -1530, -1530, -1530, -1530,
   -1530, -1530, -1530, -1530, -1530, -1530, -1530, -1530, -1530, -1530,
   -1530, -1530, -1530, -1530, -1530, -1530, -1530, -1530, -1530, -1530,
   -1530, -1530, -1530, -1530, -1530, -1530, -1530, -1530, -1530, -1530,
   -1530, -1530, -1530, -1530, -1530, -1530, -1530, -1530, -1530, -1530,
   -1530, -1530,  9632,  9948, -1530,   764, -1530, -1530, -1530, -1530,
   -1530,  5838,  6788,  1088, -1530,  7104, -1530, -1530,   389,   104,
   -1530, -1530, -1530, -1530, -1530, -1530, -1530, -1530, -1530, -1530,
   -1530, -1530, -1530, -1530,   115, -1530, -1530, -1530, -1530, -1530,
   -1530,   182, -1530, -1530, -1530, -1530, -1530, -1530, -1530, -1530,
    6472, -1530, -1530, -1530, -1530,    47, -1530, -1530, -1530, -1530,
   -1530, -1530, -1530, -1530, -1530,   172, -1530,   897, -1530, -1530,
   -1530,   351, -1530,   455, -1530, -1530, -1530, -1530, -1530, -1530,
   -1530,   922,   999, -1530, -1530,   793,   838,   994,   714,   543,
     765,   628, -1530,  1045,   895,   996,   997, 11212, -1530,   904,
   -1530, -1530,   359, -1530, -1530, 13424, -1530,   802, -1530,   945,
   14056, -1530,   945, 14056, -1530, -1530, -1530,   754, -1530, -1530,
   -1530, -1530, -1530, -1530, -1530, -1530, -1530, -1530, -1530, -1530,
   -1530, -1530, -1530, -1530, -1530, -1530, -1530, -1530, -1530, -1530,
   -1530, -1530, -1530, -1530, -1530,   944, -1530, -1530, -1530, -1530,
   -1530, -1530, -1530, -1530, -1530, -1530, -1530, -1530, -1530, -1530,
   -1530, -1530, -1530, -1530, -1530, -1530, -1530, -1530, -1530, -1530,
   -1530, -1530, -1530,   820, -1530, -1530, -1530, -1530, -1530,   920,
   -1530,  6472,   921,   926,   927,  6472,   596,   596,  1074,   -15,
     695,   610, 19209, 18609,    -9,  1061, 18609,   955,   991,   637,
   18609,   813,   862, 18609, 18609,   803,   870,    76, -1530, -1530,
   -1530, 18609,  6472,   928,  6472,   136, 11528, 15589, 18609,   327,
     962, 11528,  1114,    85,    98, 18609,  1000,   976,  1010, -1530,
     933, 11528, 15891, 18609, 18609, 18609,  6472,   934, 11528, 11528,
   18609,  6472,   968,   969, -1530, -1530, -1530, 11528, 16193,   967,
   -1530,   970, -1530, -1530, -1530, -1530,   972, -1530,   973, -1530,
   -1530, -1530, -1530, -1530,   975, -1530, -1530, -1530, -1530, -1530,
   -1530, -1530, -1530, -1530, -1530, -1530, -1530, -1530, -1530, -1530,
   -1530, -1530, -1530, -1530, -1530, -1530, -1530, -1530, -1530, -1530,
   -1530, -1530, -1530, -1530, -1530, 18609, -1530, -1530, -1530, -1530,
   -1530, -1530, -1530, -1530, -1530, -1530, -1530, -1530, -1530,   982,
   18609, -1530, -1530,   942,  6156,  1009,    41,  1012,   978,   979,
     984, 18609,  6472, -1530,   988,   243, -1530,   351, -1530,    96,
    1109, 11528, 11528, -1530,   106, -1530, -1530,  1129, -1530, -1530,
   -1530, -1530, 11528,   931, -1530,  1118, 11528, 11528, 18915, 11528,
   11528, 11528, 11528, 18915, 11528,   912, 18609,   958,   961, 11528,
   11528,  6472,   900, 18609, -1530, -1530,    63, -1530,    54, 11528,
    6788, -1530, -1530, -1530, -1530, -1530,   139,   140,   120,   128,
    1149,  7420,  3719,  7736,  4046,   792, -1530, -1530,   390,   792,
   -1530, 11528,  4678, -1530,  1035,   689,    11,   989,   990,   995,
   -1530, -1530, 11528,   780, -1530, -1530,  6472, -1530, -1530, -1530,
   -1530, -1530, -1530, -1530,   869,   872, 18609,  1041, 11844, 11844,
   12160, -1530, 12160, 12160, -1530, 12160, -1530, 12160, -1530, -1530,
   -1530, -1530, -1530, -1530, -1530, 12160, 12160,  1097, 12160, 12160,
   12160, 12160, 12160, 12160, 12160, 12160, 12160, 12160, 12160, 12160,
     935,  1075,  1076,  1078, -1530, -1530, -1530, 12792,  6472, -1530,
   -1530, 13108, 13108, 11528,   945, -1530, -1530,   945, -1530, 14674,
   10264,  8368,   945,   981,  1023,  8684, -1530, -1530, -1530, -1530,
     277, -1530,   329, -1530, -1530, -1530, -1530, -1530, -1530,  1068,
    1069,   521,  1148, -1530, -1530, 19209,   974,   738, 18609,  1017,
    1018,   974,  1074,  1053,  1049, -1530, -1530, -1530,    83,   936,
    1089,   884, 18609,  1046, 11528,  1072, 18609, 18609, -1530,  1058,
    1008, -1530,  1014,   970,   640, -1530,  1006,  1007,   112, -1530,
     333,   367,  1092, -1530,    49, 18609,  1102,   371, -1530,  6472,
   -1530,   226, -1530, 18609,  1106,  1161, 18609,  1074,  1111,   598,
   18609, 11528,    11, -1530,   275,  1021, -1530,  1022,  1025,  1026,
      42, -1530,    74,  1031, -1530,   227,   236,  1062, -1530,  1033,
    6472,  6472,   292, -1530,   420,   530,   601, 11528,   278, -1530,
   -1530, 11528, 11528, -1530, 11528, 11528, 11528, 11528, -1530, 11528,
   -1530, 11528, -1530, 18609,  1109, -1530,   469,   410,   454, -1530,
   -1530, -1530,   482, -1530,    14, -1530, -1530,  1065,  1066,  1067,
    1070,  1071,   754,   820,    24,   300,    -5,    17,  1128,   693,
     985,  -105,   133, -1530,  1080, -1530, -1530,  1037,   268,  6156,
     484, 14372,   900, -1530,  1092, -1530, -1530,   992, -1530,   389,
     811,  1194,   169, -1530, -1530,   135, -1530, -1530, -1530,   138,
   -1530,   160, -1530, -1530, -1530, -1530, -1530,  4362, -1530, -1530,
   -1530, 18609,  1059, 11528, 12476, -1530, -1530, -1530, -1530, -1530,
    1092, 18609,    34,   999, -1530, -1530, -1530, -1530, -1530, 12160,
   -1530, -1530, -1530,    58, -1530,   543,   543,   608,   765,   765,
     765,   765,   628,   628, -1530, -1530, 17703, 17703, 18609, 18609,
   -1530,  1047, -1530, -1530,   325, -1530, 10580, -1530, -1530, -1530,
   -1530, -1530, -1530, -1530, -1530,   546, -1530, -1530, -1530,   558,
     596, -1530, -1530,   700,   748,   767, -1530,   521, -1530, -1530,
   -1530, -1530, -1530, -1530, -1530, -1530, -1530, -1530,   974, -1530,
    1099, 18005,  1090, 11528, -1530, -1530, -1530,  1140,  1074,  1074,
     974, -1530,   889,  1074,   722, 18609,   694,   701,  1206, -1530,
   -1530,   946,   597, -1530, -1530, -1530, -1530, -1530, -1530, -1530,
   -1530, -1530, -1530, -1530,    83,    39,  1004,   675, 11528, -1530,
   18609,  1144,   937,  1074, -1530, -1530, -1530, -1530,  1085, 18609,
   -1530, 18609, -1530, 18307,  1112, 17703,  1124, 11528,     8,  1095,
      59,  1140, 17703,  1126,  1151,  1064,  1050,  1113,  1074,  1091,
    1117,  1155,  1074, 11528,   -24, -1530, -1530, -1530,  1100, -1530,
   -1530, -1530, -1530,  1135, 11528, 11528,  1107, -1530,  1156,  1157,
    6472, -1530,  1081,  1082,  1104, 18609, -1530, 18609, -1530, 11528,
    1116,  1073, 11528, -1530,  1138,  1145, -1530,   288,   328,   341,
    1228, -1530,   544, -1530,   526,  1119, -1530, -1530,  1239, -1530,
     789, 11528, 11528, 11528,   805, 11528, 11528, 11528, 11528, 11528,
   11528, 11528, 11528, 11528, 18915,  1152, 11528, 11528, -1530,  8052,
     422,  1024, -1530,   466, -1530, 11528,   151,   192,   160,  7736,
    4046,  7736,  1192, -1530, -1530, 11528,   865,  1165, -1530, 18609,
    1169,  1170, -1530,   557,  1019, -1530, -1530,   737,    34, -1530,
   11528,    58,   451,   619, -1530,   993,   149,  1027,  1015, -1530,
   -1530,   862, -1530,  1013,   686,  1125,  1127, 16495,  1131,  1132,
    1136,  1137,  1139, -1530,   663, -1530, -1530, -1530, -1530, -1530,
   -1530, -1530, -1530, -1530, -1530,  1141, -1530, -1530, -1530, -1530,
   -1530,   344, 10896, -1530, -1530, -1530, -1530, -1530, -1530, -1530,
   -1530, -1530,   816, -1530,  1252, 18005,   871, -1530, -1530, -1530,
   -1530, -1530, -1530, -1530, -1530, -1530,  1001, -1530, -1530,  1266,
   -1530, -1530, -1530, -1530, -1530,   734,  1268, -1530,   306, -1530,
   -1530, -1530,   967,   385,   626,   972,   634,   973,   975, -1530,
    1132,  1136,  1137, -1530,   663,   663, 12792,  1016,   998, -1530,
    1140,    39,  1083,  1110,  6472,  1115,  1120,  1160,  1123,  1133,
   18609, -1530,   407, -1530, 18609, -1530, 11528,  1158, 11528,  1172,
   11528,    73,  1164, -1530, -1530, -1530, 17703, -1530,  6472,  1074,
    1209, -1530, -1530, -1530,  1074,  1209, -1530, 11528,  1175,  6472,
   18609, -1530, -1530, 11528, 11528,   462, -1530,   456,   724, -1530,
   16797,   775, -1530,   799, -1530,  1142, -1530, -1530,  6472,  1134,
    1147, -1530, 11528, -1530, -1530, 11528, 11528,  1143,  1156,  1213,
   -1530,  1186, -1530,   561, -1530, -1530,  1316, -1530, -1530,  6472,
   18609, -1530,   648, -1530, -1530, -1530,  6472,  1146,  1086,  1087,
   -1530, -1530, -1530,  1093,  1094, -1530, -1530, -1530,  1231, -1530,
   -1530, -1530, -1530,  1098,   421, 18609,   841, -1530,  1171,  1178,
    1179,  1187, -1530,   897,   351, 14372,  1024, -1530,  8052, 14372,
   -1530, -1530,  1194,   642, -1530, -1530, -1530,  1165, -1530,  1074,
   -1530,   907, -1530,   725,  1237, -1530, 11528,   620,  1074, -1530,
   12476, 11528,  1198, -1530,  1229,  1230, 11528, 18609,   959,  1273,
   -1530, -1530, -1530,   488,  1168, -1530,    58,  1101, -1530, -1530,
     -60, -1530,   723,   353,  1103,    58,   723, 12160, -1530,   167,
   -1530, -1530, -1530, -1530, -1530, -1530,    58,  1204,  1084,   936,
     353, -1530, -1530,  1077,  1279, -1530, -1530, -1530, 14976,  1173,
   -1530,  1176,  1182,  1183,  1185,  1188,  1189, -1530, -1530, -1530,
   -1530, -1530, -1530, -1530, -1530, -1530, -1530, -1530, -1530, -1530,
   -1530,  1330,    15,  1333,    15,  1105,  1267, -1530, -1530,  1203,
   18609,  1122, -1530, -1530, 12792, -1530,  1195, -1530, -1530, -1530,
   -1530, -1530, -1530, 11528,  1234, -1530, -1530, 11528, -1530,   668,
   -1530, 11528,  1238, 11528, -1530,  1251,  1258,  1291,  1074,  1209,
   -1530, 11528,  1205, -1530, -1530, -1530,   841, -1530,   456, 11528,
    6472,   841,   456, -1530, 18609,   741, -1530, 17099,   297, -1530,
   17401,   841, -1530, -1530,  1210, -1530, -1530, -1530, -1530, -1530,
   11528,   807,  1228, 18609,   833, -1530,  1212,  1228, 18609, -1530,
    1214, -1530, 11528, 11528, 11528, 11528,  1079, 11528, 11528, -1530,
   11528, 11528, 11528, 11528,  8052,   542, -1530, -1530, -1530, -1530,
   -1530,  1237, -1530, -1530, -1530, 11528,  1245, -1530, -1530, -1530,
   11528, 18609, 18609, -1530,   641, -1530, 11528, -1530, -1530, -1530,
    1215, 12160, -1530, -1530, -1530, -1530, -1530,    93, 12160, 12160,
     684, -1530,  1027, -1530,   427, -1530,  1015,    58,  1243, -1530,
   -1530,  1121, -1530, -1530, -1530, -1530,  1307,  1216, -1530,   562,
   -1530, -1530, -1530, -1530, -1530, -1530, -1530, -1530,   108,   108,
   -1530,    15, -1530, -1530,   563, -1530,  1375,   -20,  1309,  1220,
   12792,   -38,  1130,  1240, -1530, -1530, 11528, -1530, 11528,  1260,
   -1530, 11528, -1530,  8052, 11528,  1074, -1530, -1530, -1530, 11528,
   11528, -1530, -1530, -1530,  8052,  8052,  1317, 17703,  6472, 18609,
     741, 18609, 11528, 18609,   741,  8052, -1530,   523,    37,  1228,
   18609, -1530,  1217,  1228, -1530, -1530, -1530, -1530, -1530, 11528,
   -1530, -1530, -1530,   358,   398,   411, 11528, -1530, -1530,  1169,
   11528, -1530, -1530,  1359, -1530, -1530, -1530,   765, 12160, 12160,
     608,   728, -1530, -1530, -1530, -1530, -1530, -1530, 11528, -1530,
   17703, -1530, 17703,  1319, -1530, -1530, -1530,  1385, -1530, -1530,
   -1530,  1150,  1310, -1530, -1530,  1311, -1530,    87, 18609,  1298,
    1191, 18609, 12792, -1530, -1530, 11528, -1530, -1530, -1530,  1209,
   -1530, -1530, 17703, -1530, -1530, -1530,  1325, 11528,   841, -1530,
    1332,  8052, -1530, 18609,   632,   674, -1530,  1226,  1228, -1530,
    1232, -1530,  1233,  1156,  1157,   489, -1530,  1169,  1313,   608,
     608, 12160,   492, -1530, -1530, 17703, -1530, -1530,  1309, 12792,
   -1530,  1237,  1154, 18609,  1315,  1197,  1311, -1530,   741, 17703,
    6472, 17703,   841, -1530, -1530,  1335,   635, -1530, -1530, -1530,
   -1530,  1246,   703, -1530, -1530, -1530,  1236, -1530,  8052,   829,
   -1530, -1530, 18609,   608, -1530, -1530, -1530, -1530, -1530, 11528,
    1159, 18609,  1318,  6472,   741, -1530, -1530,   741, 11528, -1530,
   -1530, -1530, -1530, -1530, -1530, -1530,  1320, -1530, -1530,  1162,
    1163, 18609, -1530, -1530, 11528,  8052, 18609,  1167, 11528,  1174,
     841, 12792, -1530, 12792,  8052, -1530,  1235,  1153, 18609,  1211,
    1321, 18609,  1180, 12792, -1530
  };

  const short
  jsoniq_parser::yydefact_[] =
  {
     165,   473,   474,   819,   475,   168,   168,   168,     0,   842,
    1047,   120,   122,   648,   935,   944,   884,   847,   845,   821,
     936,   939,   891,   851,   822,   824,     0,   945,   826,   942,
     913,   893,   867,   823,   888,   889,   940,   937,   887,   828,
     943,   829,   830,   984,   996,   983,   885,   904,   898,   831,
     886,   833,   832,   985,   922,   923,   890,   864,  1006,  1007,
    1008,  1009,  1010,  1011,  1012,  1013,  1014,  1015,  1016,  1030,
    1037,   912,   908,   899,   878,   820,     0,   907,   915,   924,
    1031,   903,   506,   879,   880,   938,  1032,  1038,   900,   917,
       0,   512,   462,   508,   910,   844,   901,   902,   931,   905,
     921,   930,  1036,  1039,   850,   892,   933,   507,   920,   926,
     825,   168,     0,     0,     0,   409,   918,   929,   934,   932,
     911,   897,   986,   895,   896,  1033,     0,   408,     0,  1034,
    1040,   927,   881,   906,  1045,  1035,   211,   440,     0,   472,
     928,   909,   916,   925,   919,   987,   872,   877,   876,   875,
     874,   873,   834,   894,     0,   843,  1046,   941,   865,  1042,
    1043,  1044,   977,   976,   978,   849,   848,   868,   975,   827,
     972,   980,   974,   973,   871,   914,   970,   982,   981,   969,
     869,   979,   991,   992,   989,   990,   988,   835,   993,   994,
     995,   961,   960,   947,   863,   856,   954,   950,   866,   862,
     962,     0,   852,   853,   846,   855,   959,   958,   955,   951,
     967,   968,   966,   957,   953,   946,   854,   965,   964,   858,
     860,   859,   952,   956,   948,   861,   949,   857,   963,  1017,
    1018,  1019,  1020,  1021,  1022,   998,   999,   997,  1003,  1004,
    1005,  1000,  1001,  1002,   870,  1023,  1024,  1025,  1026,  1027,
    1028,  1029,     0,     0,   836,   971,  1041,   838,   837,   839,
     840,   165,   165,     0,     2,   165,     9,    11,    23,     0,
      28,    31,    36,    37,    38,    39,    40,    41,    42,    32,
      58,    59,    33,    34,     0,    76,    79,    80,    35,    81,
      82,     0,   118,    83,    84,    85,    86,    18,   162,   163,
     164,   171,   174,   499,   175,     0,   176,   177,   178,   179,
     180,   181,   182,   185,   186,     0,   212,   220,   215,   249,
     255,     0,   247,     0,   248,   222,   216,   184,   217,   183,
     218,   221,   358,   360,   362,   364,   374,   376,   380,   382,
     385,   390,   393,   396,   398,   400,   402,     0,   406,   413,
     412,   414,     0,   431,   415,   436,   439,   441,   444,   446,
       0,   451,   448,     0,   459,   469,   471,   445,   476,   490,
     504,   491,   492,   493,   496,   497,   494,   498,   521,   522,
     495,   525,   527,   528,   529,   526,   574,   575,   576,   577,
     578,   579,   580,   468,   620,     0,   611,   619,   618,   617,
     616,   613,   615,   612,   614,   505,    43,   223,   224,   226,
     225,   227,   219,   501,   502,   503,   500,   229,   232,   228,
     230,   231,   470,   818,   841,   935,   944,   942,  1046,     0,
     166,   167,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   126,   128,
     129,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   280,   281,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   464,   467,   454,     0,     0,   821,
     851,   824,   882,   945,   826,   893,   828,   904,   831,   833,
     832,   922,  1030,  1037,   820,  1031,   880,  1032,  1038,   905,
    1036,  1039,   929,   986,  1033,  1034,  1040,  1035,   987,   975,
     972,   974,   973,   869,   991,   992,   989,   835,   993,   883,
     853,   971,   460,   818,   452,     0,   192,   453,   456,   821,
     822,   824,   823,   828,   829,   830,   831,   820,   827,   509,
       0,   458,   457,     0,     0,   212,     0,   470,   826,   833,
     832,     0,   168,   796,   993,     0,   220,     0,   510,     0,
     541,     0,     0,   463,     0,   434,   435,     0,   466,   465,
     455,   438,     0,     0,   570,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   363,   798,     0,   800,     0,     0,
     165,     3,     4,     1,    10,    12,     0,     0,     0,     0,
       6,   165,     0,   165,     0,     0,   119,   172,     0,     0,
     190,     0,     0,   199,     0,     0,     0,     0,     0,     0,
     235,   236,     0,     0,   204,   233,     0,   254,   250,   256,
     251,   253,   252,   260,     0,     0,     0,     0,     0,     0,
       0,   425,     0,     0,   423,     0,   369,     0,   424,   417,
     422,   421,   420,   419,   418,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   411,   410,   407,     0,   168,   432,
     437,     0,     0,     0,   447,   487,   450,   449,   461,     0,
       0,     0,   477,     0,     0,     0,   582,   584,   588,   590,
       0,   124,     0,   817,    47,    44,    45,    48,    49,     0,
       0,     0,     0,    50,    51,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   655,   656,   657,     0,   103,
     142,     0,     0,   112,     0,     0,     0,     0,   127,     0,
       0,   628,     0,     0,     0,   623,     0,     0,     0,   638,
       0,     0,   258,   263,     0,     0,   134,     0,   138,   168,
     524,     0,    60,     0,    69,     0,     0,     0,    61,     0,
       0,     0,     0,   626,     0,     0,   629,     0,     0,     0,
       0,   329,     0,     0,   627,     0,     0,     0,   646,     0,
       0,     0,     0,   632,     0,     0,   194,     0,     0,   188,
     187,     0,     0,   802,     0,     0,     0,     0,   509,     0,
     797,     0,   511,   542,   541,   538,     0,     0,     0,   572,
     571,   433,     0,   569,     0,   668,   669,     0,     0,     0,
       0,     0,   811,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   674,     0,   202,   203,     0,     0,     0,
       0,     0,   207,   208,   259,   799,   801,     0,     5,    24,
       0,    25,     0,     7,    29,     0,    15,     8,    30,     0,
      19,   935,    77,    16,    78,    20,   193,     0,   191,   213,
     214,     0,     0,     0,     0,   298,   205,   234,   237,   238,
     257,     0,     0,   359,   361,   367,   373,   372,   371,     0,
     368,   365,   366,     0,   377,   384,   383,   381,   387,   388,
     389,   386,   391,   392,   395,   394,     0,     0,     0,     0,
     416,     0,   442,   443,     0,   488,     0,   484,   485,   486,
     481,   480,   517,   478,   519,     0,   621,   523,   515,     0,
       0,   121,   123,     0,     0,     0,   102,     0,    92,    94,
      95,    96,    97,    99,   100,   101,    93,    98,    88,    89,
       0,     0,   108,     0,   104,   106,   107,   114,     0,     0,
      87,    46,     0,     0,     0,     0,     0,     0,     0,   746,
     751,     0,     0,   747,   781,   734,   736,   737,   738,   740,
     742,   741,   739,   743,     0,     0,     0,     0,     0,   111,
       0,   144,     0,     0,   587,   581,   624,   625,     0,     0,
     642,     0,   639,     0,     0,     0,     0,     0,     0,     0,
       0,   140,     0,     0,   135,     0,     0,     0,     0,     0,
       0,    62,     0,     0,     0,   279,   285,   282,     0,   631,
     630,   637,   645,     0,     0,     0,     0,   586,     0,     0,
       0,   426,     0,     0,     0,     0,   635,     0,   633,     0,
     195,     0,     0,   804,     0,   470,   803,     0,     0,     0,
     541,   539,     0,   530,     0,     0,   513,   514,     0,    13,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   200,     0,
       0,     0,   682,     0,   209,     0,     0,     0,     0,   165,
       0,   165,     0,   173,   246,     0,   313,   309,   311,     0,
     306,   299,   300,     0,     0,   239,   240,     0,   245,   370,
       0,     0,     0,     0,   712,   378,   685,   689,   691,   693,
     695,   698,   705,   706,   714,   945,   825,     0,   834,   838,
     837,   839,   840,   397,   593,   600,   601,   604,   649,   650,
     605,   606,   609,   399,   401,   404,   610,   403,   430,   489,
     482,     0,     0,   479,   516,   125,    56,    57,    54,    55,
     131,   130,     0,    90,     0,     0,     0,   109,   110,   115,
      74,    75,    52,    53,    73,   752,     0,   755,   782,     0,
     745,   744,   749,   748,   780,     0,     0,   757,     0,   753,
     756,   735,     0,     0,     0,     0,     0,     0,     0,   622,
       0,     0,     0,   143,   146,   148,     0,     0,     0,   113,
     116,     0,     0,     0,   168,     0,     0,   646,     0,     0,
       0,   265,     0,   592,     0,   266,     0,     0,     0,     0,
       0,     0,     0,   141,   136,   139,     0,   189,     0,     0,
      71,    65,    68,    67,     0,    63,   283,     0,     0,   168,
       0,   327,   331,     0,     0,     0,   334,     0,     0,   340,
       0,     0,   347,     0,   351,     0,   428,   427,   168,     0,
       0,   196,     0,   198,   328,     0,     0,     0,     0,     0,
     542,     0,   532,     0,   565,   562,     0,   566,   567,     0,
       0,   561,     0,   536,   564,   563,     0,     0,     0,     0,
     661,   662,   658,     0,     0,   666,   667,   663,   807,   808,
     672,   812,   670,     0,     0,     0,     0,   676,   826,   833,
     832,   993,   201,     0,     0,     0,   677,   678,     0,     0,
     210,   810,    26,     0,    17,    21,    22,   310,   322,     0,
     323,     0,   314,   315,   316,   317,     0,   509,     0,   307,
       0,     0,     0,   241,     0,     0,     0,     0,   292,     0,
     289,   294,   244,     0,     0,   696,   709,     0,   375,   379,
       0,   728,     0,     0,     0,     0,     0,     0,   684,   686,
     687,   723,   724,   725,   727,   726,     0,     0,   700,   699,
       0,   703,   707,   721,   719,   718,   711,   715,     0,     0,
     596,     0,     0,     0,     0,     0,     0,   597,   599,   598,
     594,   405,   483,   518,   520,   133,   132,    91,   105,   769,
     750,     0,   774,     0,   774,   763,   758,   147,   149,     0,
       0,     0,   117,   145,     0,    27,     0,   643,   644,   647,
     640,   641,   264,     0,     0,   278,   270,     0,   274,     0,
     268,     0,     0,     0,   137,     0,     0,    70,     0,    64,
     284,     0,     0,   330,   332,   337,     0,   335,     0,     0,
       0,     0,     0,   341,     0,     0,   355,     0,     0,   348,
       0,     0,   352,   429,     0,   636,   634,   197,   805,   806,
       0,     0,   541,     0,     0,   573,     0,   541,     0,   537,
       0,    14,     0,     0,     0,     0,     0,     0,     0,   675,
       0,     0,     0,     0,     0,     0,   679,   683,   326,   324,
     325,   318,   319,   320,   312,     0,     0,   308,   301,   287,
       0,     0,     0,   243,   290,   293,     0,   713,   704,   710,
       0,     0,   783,   784,   794,   793,   792,     0,     0,     0,
       0,   785,   690,   791,     0,   688,   692,     0,     0,   697,
     701,     0,   722,   717,   720,   716,     0,     0,   607,     0,
     602,   654,   595,   816,   815,   814,   603,   770,     0,     0,
     768,   775,   776,   772,     0,   767,     0,   765,     0,     0,
       0,     0,     0,     0,   583,   267,     0,   276,     0,     0,
     272,     0,   275,     0,     0,     0,    66,   286,   589,     0,
       0,   338,   342,   336,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   585,     0,     0,   541,
       0,   568,     0,   541,   591,   659,   660,   664,   665,     0,
     671,   813,   673,     0,     0,     0,     0,   680,   321,   302,
       0,   288,   296,   297,   291,   242,   708,   795,     0,     0,
     787,     0,   733,   732,   731,   730,   729,   694,     0,   786,
       0,   651,     0,     0,   779,   778,   777,     0,   771,   764,
     762,     0,   759,   760,   754,   150,   152,   154,     0,     0,
       0,     0,     0,   271,   269,     0,   277,   206,   357,    72,
     333,   339,     0,   356,   353,   349,     0,     0,     0,   343,
       0,     0,   345,     0,   551,   545,   540,     0,   541,   531,
       0,   809,     0,     0,     0,     0,   305,   303,     0,   789,
     788,     0,     0,   652,   608,     0,   773,   766,     0,     0,
     156,   155,     0,     0,     0,     0,   151,   273,     0,     0,
       0,     0,     0,   559,   553,     0,   552,   554,   560,   557,
     547,     0,   546,   548,   558,   533,     0,   534,     0,     0,
     681,   304,     0,   790,   702,   653,   761,   153,   157,     0,
       0,     0,     0,     0,     0,   346,   344,     0,     0,   544,
     555,   556,   543,   549,   550,   535,     0,   295,   158,     0,
       0,     0,   354,   350,     0,     0,     0,     0,     0,     0,
       0,     0,   160,     0,     0,   159,     0,     0,     0,     0,
       0,     0,     0,     0,   161
  };

  const short
  jsoniq_parser::yypgoto_[] =
  {
   -1530, -1530,  -240,  -190, -1530,  1190,  1202, -1530,  1200,  -557,
   -1530, -1530, -1530, -1530, -1530, -1530, -1530, -1530, -1530, -1530,
   -1036, -1530, -1530, -1530, -1530,  -227,  -588, -1530,   690,  -797,
   -1530, -1530, -1530, -1530, -1530,   228,   458, -1530, -1530,    -4,
    -116,  1011, -1530,   986, -1530, -1530,  -668, -1530,   394, -1530,
     193, -1530,  -286,  -332, -1530,  -573, -1530,    13,   127,     6,
    -272,  -184, -1530,  -899, -1530, -1530,  -136, -1530, -1530, -1530,
   -1530, -1530, -1530, -1530, -1530, -1530, -1530,   577, -1530,  -110,
    1896,     0, -1530, -1530,  -320, -1530, -1530,   305, -1530, -1530,
    -306, -1530,    16, -1530, -1530,    32,  -956,  -759,  -733, -1530,
   -1530,   667, -1530, -1530,  -112,    67, -1530, -1530, -1530,    77,
   -1165, -1530,   331,    84, -1530, -1530,    88, -1328, -1530,   930,
     183, -1530, -1530,   179, -1040, -1530, -1530,   177, -1530, -1530,
   -1266, -1258, -1530,   174, -1441, -1530, -1530,   823,  -164, -1530,
   -1530,  -579,   800, -1530, -1530,  -674,   362,  -658,   369,   372,
   -1530, -1530, -1530,   545, -1530, -1530,  1166, -1530, -1530, -1530,
   -1530, -1530,  -876,  -322,  -698, -1530,   -88, -1530, -1530, -1530,
   -1530, -1530, -1530, -1530,   -14,  -825, -1530,  -551,   677,   269,
   -1530,  -388, -1530,   774, -1530,   776, -1530, -1530, -1530,   772,
   -1530, -1530, -1530, -1530, -1034, -1530,   163, -1530,   655,  -812,
   -1530, -1530, -1530, -1530, -1530,  -314,  -305, -1242,  -993, -1530,
   -1530, -1530, -1530, -1530, -1530, -1530, -1530, -1530, -1530, -1530,
    -772,  -904,  -182,  -879, -1530, -1530, -1530,  -867, -1530, -1530,
   -1530, -1530, -1530, -1530, -1530, -1530, -1530,  1038,  1040,  -217,
     459,  -688, -1530, -1530, -1530, -1530, -1530, -1530, -1530, -1530,
   -1530, -1530, -1530,   137, -1530, -1530,   131, -1530,   134, -1094,
   -1530, -1530, -1530,    86,    72,   -97,   350, -1530, -1530, -1530,
   -1530, -1530, -1530, -1530, -1530, -1530, -1530, -1530, -1530, -1530,
      89, -1530, -1530, -1530,   -89,   345,   495, -1530, -1530, -1530,
   -1530, -1530,   283, -1530, -1530, -1529, -1530, -1530, -1530,  -554,
   -1530,    48, -1530,  -108, -1530, -1530, -1530, -1530, -1332, -1530,
      99, -1530, -1530, -1530, -1530, -1530,   938, -1530, -1530, -1530,
   -1530, -1530,  -943,  -405,  1439,   419, -1530
  };

  const short
  jsoniq_parser::yydefgoto_[] =
  {
       0,   263,   642,   264,   265,   266,   267,   268,   269,   270,
     271,   272,   273,   274,   275,   276,   277,   278,   279,   280,
    1290,   797,   281,   282,   283,   284,   285,   286,   988,   989,
     990,   287,   288,   289,   994,   995,   996,   290,   456,   291,
     292,   730,   293,   458,   459,   460,   470,   787,   788,   294,
    1253,   295,  1735,  1736,   296,   297,   298,   563,   299,   300,
     301,   302,   303,   790,   304,   305,   546,   306,   307,   308,
     309,   310,   311,   654,   312,   313,   882,   883,   314,   315,
     316,   576,   318,   655,  1672,   667,  1157,  1158,   319,   657,
     320,   659,   577,   322,   323,   782,   783,  1408,   477,   324,
     478,   479,   922,  1409,  1410,  1411,   660,   661,  1151,  1152,
    1399,   662,  1147,  1148,  1392,  1393,  1394,  1395,   325,   810,
     811,   326,  1305,  1306,  1518,   327,  1308,  1309,   328,   329,
    1311,  1312,  1313,  1314,  1525,   330,   331,   332,   333,   334,
     929,   335,   336,  1418,   337,   338,   339,   340,   341,   342,
     343,   344,   345,  1194,   346,   347,   348,   349,   685,   686,
     350,   351,   352,   353,   354,   355,   356,   357,   358,   359,
     360,   361,   362,   363,   364,   365,   366,   367,   722,   715,
     368,   369,   370,   371,   372,   373,   374,   375,   376,   965,
     377,   378,   379,   380,   381,   382,  1342,   844,   845,   846,
    1766,  1811,  1812,  1805,  1806,  1813,  1807,  1343,  1344,   383,
     384,  1345,   385,   386,   387,   388,   389,   390,   391,   392,
    1050,  1526,  1460,  1184,  1619,  1185,  1195,   393,   394,   395,
     396,   397,   398,   399,   400,   401,   402,   403,   404,   817,
    1266,   405,  1187,  1188,  1189,  1190,   406,   407,   408,   409,
     410,   411,   872,   873,   412,  1376,  1377,  1697,  1133,  1165,
    1428,  1429,  1166,  1167,  1168,  1169,  1170,  1438,  1609,  1610,
    1171,  1441,  1172,  1590,  1173,  1174,  1446,  1447,  1615,  1613,
    1430,  1431,  1432,  1433,  1715,   759,  1015,  1016,  1017,  1018,
    1019,  1020,  1239,  1639,  1732,  1240,  1637,  1730,  1021,  1474,
    1634,  1630,  1631,  1632,  1022,  1023,  1434,  1442,  1600,  1435,
    1596,  1419,   413,   414,   415,   416,   566,   417,   418,   419,
     420,   421,  1191,  1291,   422,   423,   424
  };

  const short
  jsoniq_parser::yytable_[] =
  {
     317,   656,   575,   579,   612,   317,   317,   317,   317,   950,
     457,   431,   431,   431,   435,   658,   321,   937,   429,   432,
     433,   321,   321,   321,   321,  1048,  1295,  1064,   637,   632,
     709,   959,  1102,   734,   938,   939,   940,   941,  1307,   614,
    1076,   629,  1183,  1193,   634,   902,   904,   862,   731,   731,
     591,  1049,   869,  1529,  1090,   630,  1132,  1163,   896,   900,
     903,   905,   542,  1541,   630,  1572,  1573,  1414,   794,   798,
    1341,   621,   622,   975,   894,   898,  1211,  1597,  1242,  1186,
    1186,  1738,  1255,  1243,   773,  1044,  1670,  1271,    13,  1674,
    1154,   925,  1244,   926,   927,  1279,   928,   630,   930,  1245,
    1549,  1246,   774,    11,    12,   630,   931,   932,  1611,  1733,
    1247,   317,  1216,   630,   849,   733,   630,   564,   831,  1110,
    1297,   630,   602,   442,  1186,   832,  1123,   321,   733,   630,
     475,  1764,   430,   430,   430,   434,   630,  1248,  1004,   630,
    1005,  1273,   616,   618,  1045,  1708,  1046,  1108,  1284,   832,
    1073,   445,   630,  1628,   593,   639,  1046,  1276,  1254,   637,
    1592,   743,   641,   637,   461,   472,    11,    12,   735,  1045,
     897,   641,  -169,   630,   473,   636,   438,   439,  1186,   440,
     441,   773,  1045,   736,  1006,  1186,   442,    11,    12,   765,
     638,  1213,   494,  1739,  1389,   744,   443,   444,  1047,   774,
    1593,   474,  1138,  1213,   641,  1471,  1109,   442,  1280,    11,
      12,   603,   795,  1160,   445,  1124,  1161,   443,  1629,   446,
     641,  1765,  1501,  1075,  1709,  1219,   833,   630,   630,   438,
     439,  1074,   440,   441,  1118,   445,   128,   630,   430,   640,
     446,  1125,  1598,  1698,   630,  1164,  1420,   793,  1111,  1112,
     444,   447,   448,   449,  1155,   476,  1119,   841,  1509,  1826,
     796,   317,   317,  1113,  1420,   317,  1473,   710,  1522,   630,
    1249,   658,   447,   448,  1142,  1740,   630,   321,   321,  1283,
     641,   321,   850,  1529,   842,  1163,  1163,   476,  1331,   630,
     476,  1281,   637,   630,  1156,   635,   631,  1038,  1451,  1341,
     317,   450,  1549,   902,   904,  1731,  1383,   633,  1341,  1212,
    1186,  1798,   889,  1466,  1492,  1277,   321,  1282,  1255,  1421,
     891,   638,  1589,  1162,   775,   638,   630,  1139,   476,   630,
    1141,  1298,   894,   898,   641,   641,  1451,  1421,   476,   594,
    1007,   636,   630,  1382,   641,   630,   716,  1001,  1186,   718,
    1159,   641,   476,  1008,   450,  1009,   778,  -169,  1834,   630,
    1837,   791,  1250,  1251,  1252,   451,  1010,  1011,  1012,   766,
    1013,   804,  1014,   886,  1126,   436,   641,   767,   815,   816,
     885,  1402,  1504,   641,   916,   970,  1073,   822,   890,   892,
    -170,   630,  1061,   644,  1254,  1114,   641,   645,   907,   630,
     641,   464,   646,  1236,   895,   899,   452,  1422,  1423,  1790,
    1424,   630,   630,   437,  1056,  1078,  1671,  1425,   451,  1186,
    1237,   647,   626,   630,  1079,  1422,  1423,  1426,  1424,   840,
     888,   317,   453,   641,   490,   317,   641,   970,   627,  1427,
     454,  1039,   596,  1186,   597,  1426,   785,   321,   455,   641,
    1597,   321,   641,   453,   638,   630,  1129,  1427,   663,   452,
    1068,   454,   317,  1828,   317,   971,   641,  1092,   435,   455,
     435,   847,   848,  1164,  1164,  1041,  1327,  1084,   321,  1053,
     321,    13,   852,   630,   650,   630,   317,   651,  1482,   630,
     630,   317,   435,   630,   599,   543,   600,   435,   641,   906,
     878,   880,   321,   908,  1046,   924,   641,   321,  1819,   543,
    1341,  1199,   491,  1494,   708,   786,  1328,   972,   641,   641,
     648,  1040,   664,   649,  1115,  1116,  1467,   665,  1085,  1329,
     641,   598,  1462,   543,  1776,  1334,  1335,   128,   650,  1117,
    1163,   651,   719,   465,  1618,   543,  1772,   543,  1502,  1163,
    1132,   902,   904,   902,  1567,  1042,  1493,  1519,  1479,  1054,
    1163,  1529,   641,  1304,   317,   652,  1384,  1385,  1386,   485,
    1334,  1335,   317,  1364,   666,  -170,  1516,   720,   431,   721,
     321,  1186,  1205,   601,   492,   493,  1773,   689,   321,   770,
     641,   772,   641,  1220,  1221,  1105,   641,   641,  1224,  1774,
     641,  1103,   462,   954,   690,   463,  1160,   637,  1086,  1161,
     954,   317,  1821,   813,   653,  1598,  1599,   879,   819,  1749,
     317,  1378,   457,   952,   953,  1576,    13,   321,  1263,   128,
    1336,   317,   317,   317,   317,  1143,   321,  1803,  1087,  1106,
    1803,  1334,  1763,   650,  1334,   884,   651,   321,   321,   321,
     321,  1337,  1045,  1338,  1202,  1140,   317,  1334,  1335,   892,
    1186,   692,  1136,  1186,  1558,  1336,  1202,  1107,  1104,  1131,
    1722,  1727,   321,  1587,  1820,   604,  1332,  1824,  1379,   469,
     605,  1339,   749,  1334,   750,   114,  1337,   749,  1338,   750,
     468,   693,  -261,  1045,  1236,  1404,  1045,  1696,   920,   430,
    1405,   126,  1712,  1809,   694,   471,  1401,    82,   317,  1713,
     650,  1237,  1334,   651,   431,  1045,  1339,   751,  1088,  1714,
     114,   951,    93,  1340,   321,  1097,  1098,  1099,  1164,   880,
    1678,  1163,  1809,  1522,  1203,  1682,   126,  1164,  1046,   695,
    1649,   154,  1063,  1333,   752,  1089,  1204,   107,  1164,   752,
    1723,  1728,  1336,  1604,  1379,  1238,  -261,  1337,  1543,  1338,
    1337,  -261,  1338,  1753,  1575,  1046,   698,  1804,   689,  1130,
    1840,  1808,  1814,  1337,  1416,  1338,   154,   739,   495,   543,
     699,   613,   543,  -262,   480,   690,  1643,  1339,  1121,   317,
    1339,  1404,   483,   740,   902,   431,  1405,   128,   484,  1337,
    1186,  1338,  1055,  1339,   488,   321,   753,   114,  -261,  1457,
    1810,   753,   897,  1808,   719,  1406,  1783,  1648,  1784,  1814,
     317,   317,  1265,   126,  1268,  1304,   435,   435,  1337,  1339,
    1338,  1471,  1389,   991,  1046,   430,   321,   321,  1521,  1843,
    1458,   709,  1334,  1335,  1137,  1548,  1201,  -262,  1472,   720,
    1206,   721,  -262,  1186,  1391,  1186,  1407,  1372,  1339,  1459,
     627,   741,   543,   154,   489,   543,  1207,  1767,  1319,   543,
    1320,  1770,   543,   543,   689,   719,  1527,   992,   737,   317,
     543,  1825,   993,   544,  1507,  1186,   543,   543,   689,  1528,
     907,   690,  1473,   738,   543,   321,   892,   457,  1208,  -262,
    1530,   543,   543,   543,   543,   690,  1210,   317,  1527,   543,
     720,   545,   721,  1531,  1209,   691,   430,   543,  1186,  1164,
     496,  1677,   789,   321,  1710,  1711,   581,   670,   650,  1781,
    1527,   651,  1186,  1707,  1186,   671,   672,  1336,   673,  1443,
    1444,   466,  1737,  1846,   467,   547,   674,  1082,  1083,  1380,
     675,   497,   676,  1667,   498,  1465,  1816,   677,  1337,  1712,
    1338,  1388,  1445,  1230,   543,  1791,  1713,   724,   696,   548,
    1232,   789,  1389,   678,  1231,   560,  1714,   697,   725,   543,
    1257,  1233,  1390,   955,  1568,   561,   955,  1520,  1339,  1258,
     543,   955,   114,  1577,  1391,   711,   712,  1594,   679,   680,
     681,   682,   683,   684,  1595,  1225,  1226,  1227,   126,   763,
     992,  1228,   562,  1404,   764,   993,  1505,   863,  1405,  1222,
    1223,   481,   863,   486,   482,   543,   487,  1348,   650,  1349,
    1680,   651,   543,   582,  1779,  1780,   714,  1569,  1570,   717,
     755,   756,   757,  1353,  1737,  1354,  1560,   583,   154,   588,
    1413,   935,   936,   589,  1564,   978,   979,   980,   981,   982,
     983,   984,   985,   986,   987,   942,   943,   590,   658,   592,
     944,   945,  1477,  1478,  1724,  1725,   595,  -622,   607,   606,
     317,   608,   610,   609,   619,   543,   435,   611,   623,   643,
     668,  1737,   669,   687,   688,   700,   321,   701,   702,   707,
     703,   713,   723,  1656,   733,   726,   727,  1823,   745,   747,
     748,   728,   729,   762,   758,   792,   771,   789,   802,   800,
     801,   803,   814,   820,   821,   463,   827,   829,   467,  1373,
     482,   487,   831,   498,   457,   834,   835,   836,   543,   317,
     317,   317,   837,   839,   843,  1374,   851,   853,   854,   871,
     875,   881,   893,   876,   911,   321,   321,   321,   912,   918,
     921,   913,   919,  1865,   977,  1866,   914,   543,   933,   966,
     947,   948,   946,   949,   967,  1874,   973,   974,   976,   998,
     999,   543,  1002,  1003,  1025,   543,   543,   612,  1024,  1026,
    1028,  1030,  1033,  1034,  1036,  1037,  1659,  1052,  1660,  1035,
    1043,  1664,  1665,  1058,   543,  1668,  1059,  1315,  1062,  1069,
    1070,  1675,   543,  1071,  1072,   543,  1077,  1080,  1081,   543,
     462,   466,   481,  1120,  1127,   486,   497,  1138,  1122,  1128,
    1145,  1135,  1198,  1214,   993,  1045,  1234,  1256,  1235,  1261,
    1264,  1272,  1262,  1274,  1278,   785,  1286,  1289,  1662,  1287,
    1288,  1293,  1294,  1292,  1300,  1299,  1303,  1304,  1310,  1318,
    1322,  1325,   543,  1330,   317,  1323,  1316,  1317,  1326,  1347,
     431,  1365,   751,  1396,  1346,  1375,  1398,  1486,  1400,  1403,
     321,  1437,    13,  1448,  1417,  1449,  1440,  1461,   317,  1452,
    1453,  1436,   916,  1469,  1454,  1455,  1470,  1456,  1475,   317,
     543,  1499,  1485,  1487,   321,   431,  1489,  1497,  1488,  1480,
    1481,  1490,  1512,  1503,  1527,   321,  1484,  1508,   317,  1511,
    1542,  1491,  1535,  1545,   431,  1552,  1553,  1533,  1556,  1561,
     543,  1534,  1554,  1555,   321,  1536,  1562,  1563,  1551,   317,
     543,  1557,  1378,  1540,  1389,   435,   317,  1580,  1581,  1582,
    1757,  1586,   435,  1607,  1761,   321,  1588,  1591,  1614,  1601,
    1627,  1620,   321,  1633,  1621,   543,   543,   543,   543,  1612,
    1622,  1623,  1608,  1624,  1640,  1638,  1625,  1626,  1373,  1653,
    1644,  1747,  1636,  1646,   879,  1642,  1654,  1651,  1655,  1700,
    1658,   430,  1751,  1662,  1374,  1676,  1754,  1681,  1718,  1684,
    1706,  1689,  1720,  1762,  1721,  1729,  1236,  1719,  1734,  1745,
     543,  1742,  1752,  1778,  1785,  1786,  1769,  1793,  1788,  1789,
    1799,  1787,  1794,  1741,   543,  1815,   430,  1801,  1832,  1839,
    1842,  1817,  1822,  1818,  1831,  1845,  1867,  1851,  1800,  1856,
    1871,  1000,  1870,   768,  1468,   430,  1868,  1285,   732,   543,
    1217,  1693,  1694,  1695,  1483,   624,  1796,  1827,   543,  1134,
     543,  1857,   543,  1412,   543,  1829,  1546,   625,   628,  1067,
    1849,   543,  1704,  1550,  1858,  1585,  1387,  1578,  1833,  1861,
    1574,  1571,  1838,  1513,  1517,  1523,  1863,  1532,   934,  1754,
     828,   923,  1873,   957,  1197,   958,  1544,   969,  1844,  1101,
    1269,  1841,  1559,   776,   543,   777,   543,  1566,  1606,  1565,
    1717,  1602,  1415,   706,  1854,  1716,  1439,  1855,  1605,  1241,
     317,  1476,  1635,  1726,     0,  1603,     0,     0,  1835,   559,
       0,     0,     0,     0,     0,     0,   321,     0,     0,   867,
    1864,     0,     0,   863,     0,     0,  1505,     0,     0,     0,
     567,     0,     0,   580,     0,     0,     0,     0,     0,     0,
       0,  1852,     0,     0,  1373,   584,     0,   587,   543,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1374,     0,     0,  1852,     0,     0,  1775,     0,     0,     0,
       0,     0,  1835,     0,     0,     0,   543,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1782,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   543,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1373,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1373,  1373,     0,     0,   317,  1374,
       0,     0,     0,     0,     0,  1373,     0,     0,     0,     0,
    1374,  1374,     0,     0,   321,     0,     0,     0,     0,   543,
       0,  1374,     0,   543,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   543,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   543,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   543,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   543,
       0,  1373,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1374,     0,     0,
       0,     0,     0,     0,   543,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   543,     0,     0,     0,   543,     0,
     317,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   321,     0,  1373,     0,
       0,     0,     0,     0,     0,     0,   543,     0,     0,     0,
       0,     0,     0,   317,  1374,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   321,
       0,     0,     0,     0,     0,  1373,     0,     0,     0,     0,
       0,     0,     0,     0,  1373,     0,     0,   543,     0,     0,
       0,  1374,     0,     0,     0,     0,     0,     0,     0,     0,
    1374,     0,   742,     0,     0,   746,     0,     0,     0,   754,
       0,     0,   760,   761,     0,     0,     0,     0,     0,   543,
     769,     0,     0,     0,     0,     0,   781,   784,     0,     0,
       0,     0,     0,     0,   799,     0,     0,     0,     0,     0,
       0,   807,   808,   809,   812,     0,     0,     0,     0,   818,
       0,     0,     0,     0,     0,     0,     0,   825,     0,     0,
       0,     0,     0,   543,     0,     0,   543,     0,     0,   543,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   543,     0,     0,     0,     0,   543,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   826,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   812,
     543,   543,     0,     0,     0,     0,     0,   565,     0,     0,
     838,   567,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     567,     0,     0,     0,     0,   874,     0,     0,     0,     0,
       0,     0,   784,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   543,     0,   543,     0,
     543,     0,   543,     0,     0,     0,     0,     0,     0,   543,
       0,     0,     0,     0,     0,   784,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   543,
       0,   543,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   543,   960,     0,
     543,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   543,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   543,     0,     0,     0,     0,   997,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1027,     0,     0,   543,  1031,  1032,     0,     0,     0,
       0,     0,   543,     0,     0,     0,     0,     0,   543,     0,
     543,     0,     0,     0,  1051,     0,     0,     0,     0,     0,
       0,     0,  1057,     0,     0,  1060,     0,     0,     0,  1065,
       0,   543,     0,     0,     0,     0,     0,     0,     0,     0,
     543,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     543,  1095,     0,     0,     0,   543,     0,     0,     0,     0,
       0,     0,  1100,     0,     0,     0,     0,   543,     0,     0,
     543,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1144,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1153,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1192,  1192,  1196,  1196,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1192,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1229,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   565,  1260,
       0,     0,     0,     0,     0,     0,     0,     0,  1267,     0,
    1267,     0,   784,     0,  1192,     0,     0,     0,     0,     0,
       0,  1192,   855,   856,     0,   864,   865,   866,   868,     0,
     870,     0,     0,     0,     0,   877,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   887,     0,     0,     0,     0,
       0,     0,     0,     0,   818,     0,   818,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   909,   910,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   915,     0,
       0,     0,   917,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1397,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1192,   964,     0,     0,
       0,   964,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1192,     0,     0,     0,     0,     0,
    1029,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1066,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   784,
       0,     0,     0,  1495,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1091,     0,  1192,     0,  1093,  1094,     0,
    1096,     0,     0,     0,     0,     0,     0,   917,     0,   812,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1192,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1547,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   910,   874,     0,     0,     0,     0,  1146,
    1150,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1584,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1192,     0,  1218,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1641,
       0,     0,     0,     0,  1259,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1275,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1296,
       0,     0,     0,  1666,     0,     0,  1192,     0,     0,  1192,
    1301,  1302,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1679,     0,     0,  1321,     0,  1683,  1324,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1350,  1351,  1352,
       0,  1355,  1356,  1357,  1358,  1359,  1360,  1361,  1362,  1363,
    1702,  1703,  1366,  1367,     0,     0,     0,     0,     0,     0,
       0,  1381,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1146,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1464,     0,
       0,     0,     0,     0,     0,     0,  1192,     0,  1756,     0,
    1758,     0,  1760,     0,     0,     0,     0,     0,     0,  1768,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1192,
       0,  1192,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1496,     0,  1498,     0,  1500,  1792,     0,     0,
    1795,     0,     0,     0,  1506,     0,     0,     0,     0,     0,
       0,  1192,     0,  1510,     0,     0,     0,     0,     0,  1514,
    1515,     0,  1802,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1537,     0,
       0,  1538,  1539,     0,  1192,     0,     0,     0,     0,     0,
       0,     0,  1830,     0,     0,     0,     0,     0,  1192,     0,
    1192,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1847,     0,     0,     0,     0,     0,     0,     0,     0,
    1850,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1859,     0,  1146,     0,     0,  1860,  1150,  1579,     0,     0,
       0,     0,  1583,     0,     0,     0,     0,  1869,     0,     0,
    1872,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1645,
       0,     0,     0,  1647,     0,     0,     0,  1650,     0,  1652,
       0,     0,     0,     0,     0,     0,     0,  1657,     0,     0,
       0,     0,     0,     0,     0,  1661,  1663,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1506,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1685,  1686,
    1687,  1688,     0,  1690,  1691,     0,  1692,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1699,     0,     0,     0,     0,  1701,     0,     0,     0,
       0,     0,  1705,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1743,     0,  1744,     0,     0,  1746,     0,     0,
    1748,     0,     0,     0,     0,  1750,  1663,     0,     0,     0,
       0,     0,     0,     0,  1755,     0,     0,     0,  1759,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1771,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1777,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1797,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1755,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1836,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  -165,
     897,     0,     0,     0,     0,  1848,     0,     0,     0,  1853,
       1,     2,     0,     0,  1836,     3,     0,     0,     0,     4,
       5,     6,     7,     8,     9,    10,    11,    12,     0,    13,
    1853,     0,    14,   426,  1862,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,     0,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,     0,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,     0,    72,    73,    74,    75,    76,     0,
      77,    78,    79,     0,     0,    80,    81,     0,     0,     0,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
       0,    92,     0,     0,     0,    93,     0,    94,    95,    96,
      97,     0,    98,     0,     0,     0,    99,   100,   101,   102,
     103,     0,     0,     0,     0,     0,     0,   104,   105,   106,
     107,   108,   109,   110,   111,   112,     0,   113,   114,   115,
     116,   117,     0,     0,     0,   118,   119,   120,   121,   122,
     123,   124,   125,     0,   126,     0,   127,   128,     0,     0,
     129,   130,   131,     0,     0,     0,   132,     0,   133,   134,
     135,   136,   137,   138,     0,   139,     0,   140,     0,   141,
     142,   143,   144,   145,   146,   147,   148,   149,   150,   151,
       0,     0,   152,   153,   154,     0,   155,   428,   157,   158,
     159,   160,   161,   162,   163,   164,   165,   166,   167,   168,
     169,   170,   171,   172,   173,   174,   175,   176,   177,   178,
     179,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,   197,   198,
     199,   200,   201,     0,   202,   203,   204,     0,   205,   206,
     207,   208,   209,   210,   211,   212,   213,   214,   215,   216,
     217,   218,   219,   220,   221,   222,   223,   224,   225,   226,
     227,   228,   229,   230,   231,   232,   233,   234,   235,   236,
     237,   238,   239,   240,   241,   242,   243,   244,   245,   246,
     247,   248,   249,   250,   251,   252,     0,   253,     0,   254,
     255,   256,   257,   258,   259,   260,  -165,   897,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     1,     2,     0,
       0,     0,     3,     0,     0,     0,     4,     5,     6,     7,
       8,     9,    10,    11,    12,     0,    13,     0,     0,   901,
     426,     0,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,   427,    30,    31,    32,    33,
      34,     0,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,     0,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
       0,    72,    73,    74,    75,    76,     0,    77,    78,    79,
       0,     0,    80,    81,     0,     0,     0,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,     0,    92,     0,
       0,     0,    93,     0,    94,    95,    96,    97,     0,    98,
       0,     0,     0,    99,   100,   101,   102,   103,     0,     0,
       0,     0,     0,     0,   104,   105,   106,   107,   108,   109,
     110,   111,   112,     0,   113,   114,   115,   116,   117,     0,
       0,     0,   118,   119,   120,   121,   122,   123,   124,   125,
       0,   126,     0,   127,   128,     0,     0,   129,   130,   131,
       0,     0,     0,   132,     0,   133,   134,   135,   136,   137,
     138,     0,   139,     0,   140,     0,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,   151,     0,     0,   152,
     153,   154,     0,   155,   428,   157,   158,   159,   160,   161,
     162,   163,   164,   165,   166,   167,   168,   169,   170,   171,
     172,   173,   174,   175,   176,   177,   178,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,   197,   198,   199,   200,   201,
       0,   202,   203,   204,     0,   205,   206,   207,   208,   209,
     210,   211,   212,   213,   214,   215,   216,   217,   218,   219,
     220,   221,   222,   223,   224,   225,   226,   227,   228,   229,
     230,   231,   232,   233,   234,   235,   236,   237,   238,   239,
     240,   241,   242,   243,   244,   245,   246,   247,   248,   249,
     250,   251,   252,   897,   253,     0,   254,   255,   256,   257,
     258,   259,   260,     1,     2,     0,     0,     0,     3,     0,
       0,     0,     4,     5,     6,     7,     8,     9,    10,    11,
      12,     0,    13,     0,     0,   425,   426,     0,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,   427,    30,    31,    32,    33,    34,     0,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,     0,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,     0,    72,    73,    74,
      75,    76,     0,    77,    78,    79,     0,     0,    80,    81,
       0,     0,     0,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,     0,    92,     0,     0,     0,    93,     0,
      94,    95,    96,    97,     0,    98,     0,     0,     0,    99,
     100,   101,   102,   103,     0,     0,     0,     0,     0,     0,
     104,   105,   106,   107,   108,   109,   110,   111,   112,     0,
     113,   114,   115,   116,   117,     0,     0,     0,   118,   119,
     120,   121,   122,   123,   124,   125,     0,   126,     0,   127,
     128,     0,     0,   129,   130,   131,     0,     0,     0,   132,
       0,   133,   134,   135,   136,   137,   138,     0,   139,     0,
     140,     0,   141,   142,   143,   144,   145,   146,   147,   148,
     149,   150,   151,     0,     0,   152,   153,   154,     0,   155,
     428,   157,   158,   159,   160,   161,   162,   163,   164,   165,
     166,   167,   168,   169,   170,   171,   172,   173,   174,   175,
     176,   177,   178,   179,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   197,   198,   199,   200,   201,     0,   202,   203,   204,
       0,   205,   206,   207,   208,   209,   210,   211,   212,   213,
     214,   215,   216,   217,   218,   219,   220,   221,   222,   223,
     224,   225,   226,   227,   228,   229,   230,   231,   232,   233,
     234,   235,   236,   237,   238,   239,   240,   241,   242,   243,
     244,   245,   246,   247,   248,   249,   250,   251,   252,   897,
     253,     0,   254,   255,   256,   257,   258,   259,   260,     1,
       2,     0,     0,     0,     3,     0,     0,     0,     4,     5,
       6,     7,     8,     9,    10,     0,     0,     0,    13,     0,
       0,   425,   426,     0,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,   568,   427,    30,    31,
      32,    33,    34,     0,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,     0,    48,    49,
      50,   569,   570,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,     0,    72,    73,    74,    75,    76,     0,    77,
      78,    79,     0,     0,    80,    81,     0,     0,     0,    82,
      83,   516,    85,    86,    87,    88,    89,   571,    91,     0,
      92,     0,     0,     0,    93,     0,    94,    95,    96,    97,
       0,    98,     0,     0,     0,    99,   100,   101,   102,   103,
       0,     0,     0,     0,     0,     0,   104,   105,   106,   107,
     108,   109,   110,   572,   112,     0,   113,   114,   115,   116,
     117,     0,     0,     0,   118,   119,   120,   121,   122,   123,
     124,   125,     0,   126,     0,   127,   128,     0,     0,   129,
     130,   131,     0,     0,     0,   132,     0,   133,   134,   135,
       0,   137,   138,     0,   139,     0,   140,     0,   141,   142,
     143,   144,   145,   146,   147,   148,   149,   150,   151,     0,
       0,   152,   153,   154,     0,   155,   428,   157,   158,   159,
     160,   161,   162,   163,   164,   165,   166,   167,   168,   169,
     170,   171,   172,   173,   174,   175,   176,   177,   178,   179,
     180,   181,   534,   535,   536,   185,   186,   537,   574,   189,
     190,   191,   192,   193,   194,   195,   196,   197,   198,   199,
     200,   201,     0,   202,   203,   204,     0,   205,   206,   207,
     208,   209,   210,   211,   212,   213,   214,   215,   216,   217,
     218,   219,   220,   221,   222,   223,   224,   225,   226,   227,
     228,   229,   230,   231,   232,   233,   234,   235,   236,   237,
     238,   239,   240,   241,   242,   243,   244,   245,   246,   247,
     248,   249,   250,   251,   252,     0,   253,     0,   254,   255,
     256,   257,   258,   259,   260,  -882,   468,     0,  -261,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  -882,     0,     0,
       0,  -882,     0,     0,     0,  -882,  -882,     0,     0,     0,
       0,  -882,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  -261,     0,     0,  -882,     0,  -261,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  -882,     0,
    -882,  -882,  -882,     0,     0,  -882,  -882,  -882,  -882,     0,
       0,     0,  -882,  -882,     0,     0,     0,     0,     0,  -882,
       0,     0,  -882,  -882,  -261,     0,     0,     0,  -882,     0,
       0,     0,     0,  -882,  -882,     0,     0,     0,     0,  -882,
       0,     0,     0,  -882,     0,     0,     0,  -882,  -882,     0,
    -882,     0,  -882,  -882,     0,     0,     0,  -882,  -882,     0,
       0,  -882,  -882,  -882,  -882,  -882,  -882,     0,     0,  -882,
       0,     0,     0,  -882,  -882,     0,     0,  -882,     0,     0,
       0,     0,  -882,     0,     0,  -882,     0,     0,     0,     0,
    -882,  -882,  -882,  -882,  -882,  -882,     0,  -882,  -882,  -882,
    -882,  -882,     0,     0,     0,     0,  -882,  -882,  -882,     0,
    -882,  -882,  -882,  -882,  -882,  -882,     0,  -882,     0,  -882,
       0,     0,     0,     0,     0,  -882,     0,     0,     0,  -882,
    -882,     0,  -883,   613,     0,  -262,     0,     0,     0,     0,
       0,     0,     0,     0,  -882,     0,  -882,     0,  -882,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    -882,     0,     0,     0,     0,  -882,     0,     0,  -882,     0,
       0,     0,     0,     0,  -883,     0,     0,     0,  -883,     0,
       0,     0,  -883,  -883,     0,     0,     0,     0,  -883,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  -262,
       0,     0,  -883,     0,  -262,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  -882,     0,  -882,  -883,     0,  -883,  -883,  -883,
       0,     0,  -883,  -883,  -883,  -883,     0,     0,     0,  -883,
    -883,     0,     0,     0,     0,     0,  -883,     0,     0,  -883,
    -883,  -262,     0,     0,     0,  -883,     0,     0,     0,     0,
    -883,  -883,     0,     0,     0,     0,  -883,     0,     0,     0,
    -883,     0,     0,     0,  -883,  -883,     0,  -883,     0,  -883,
    -883,     0,     0,     0,  -883,  -883,     0,     0,  -883,  -883,
    -883,  -883,  -883,  -883,     0,     0,  -883,     0,     0,     0,
    -883,  -883,     0,     0,  -883,     0,     0,     0,     0,  -883,
       0,     0,  -883,     0,     0,     0,     0,  -883,  -883,  -883,
    -883,  -883,  -883,     0,  -883,  -883,  -883,  -883,  -883,     0,
       0,     0,     0,  -883,  -883,  -883,     0,  -883,  -883,  -883,
    -883,  -883,  -883,     0,  -883,     0,  -883,     0,     0,     0,
       0,     0,  -883,     0,     0,     0,  -883,  -883,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  -883,     0,  -883,     0,  -883,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  -883,     0,     0,
       0,     0,  -883,     0,     0,  -883,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     1,     2,     0,     0,     0,     3,     0,     0,     0,
       4,     5,     6,     7,     8,     9,    10,    11,    12,  -883,
      13,  -883,     0,    14,    15,     0,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,     0,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,     0,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,     0,    72,    73,    74,    75,    76,
       0,    77,    78,    79,     0,     0,    80,    81,     0,     0,
       0,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,     0,    92,     0,     0,     0,    93,     0,    94,    95,
      96,    97,     0,    98,     0,     0,     0,    99,   100,   101,
     102,   103,     0,     0,     0,     0,     0,     0,   104,   105,
     106,   107,   108,   109,   110,   111,   112,     0,   113,   114,
     115,   116,   117,     0,     0,     0,   118,   119,   120,   121,
     122,   123,   124,   125,     0,   126,     0,   127,   128,     0,
       0,   129,   130,   131,     0,     0,     0,   132,     0,   133,
     134,   135,   136,   137,   138,     0,   139,     0,   140,     0,
     141,   142,   143,   144,   145,   146,   147,   148,   149,   150,
     151,     0,     0,   152,   153,   154,     0,   155,   156,   157,
     158,   159,   160,   161,   162,   163,   164,   165,   166,   167,
     168,   169,   170,   171,   172,   173,   174,   175,   176,   177,
     178,   179,   180,   181,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   197,
     198,   199,   200,   201,     0,   202,   203,   204,     0,   205,
     206,   207,   208,   209,   210,   211,   212,   213,   214,   215,
     216,   217,   218,   219,   220,   221,   222,   223,   224,   225,
     226,   227,   228,   229,   230,   231,   232,   233,   234,   235,
     236,   237,   238,   239,   240,   241,   242,   243,   244,   245,
     246,   247,   248,   249,   250,   251,   252,     0,   253,     0,
     254,   255,   256,   257,   258,   259,   260,   261,   262,     1,
       2,     0,     0,     0,     3,     0,     0,     0,     4,     5,
       6,     7,     8,     9,    10,    11,    12,     0,    13,     0,
       0,    14,    15,     0,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,     0,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,     0,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,     0,    72,    73,    74,    75,    76,     0,    77,
      78,    79,     0,     0,    80,    81,     0,     0,     0,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,     0,
      92,     0,     0,     0,    93,     0,    94,    95,    96,    97,
       0,    98,     0,     0,     0,    99,   100,   101,   102,   103,
       0,     0,     0,     0,     0,     0,   104,   105,   106,   107,
     108,   109,   110,   111,   112,     0,   113,   114,   115,   116,
     117,     0,     0,     0,   118,   119,   120,   121,   122,   123,
     124,   125,     0,   126,     0,   127,   128,     0,     0,   129,
     130,   131,     0,     0,     0,   132,     0,   133,   134,   135,
     136,   137,   138,     0,   139,     0,   140,     0,   141,   142,
     143,   144,   145,   146,   147,   148,   149,   150,   151,     0,
       0,   152,   153,   154,     0,   155,   156,   157,   158,   159,
     160,   161,   162,   163,   164,   165,   166,   167,   168,   169,
     170,   171,   172,   173,   174,   175,   176,   177,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,   197,   198,   199,
     200,   201,     0,   202,   203,   204,     0,   205,   206,   207,
     208,   209,   210,   211,   212,   213,   214,   215,   216,   217,
     218,   219,   220,   221,   222,   223,   224,   225,   226,   227,
     228,   229,   230,   231,   232,   233,   234,   235,   236,   237,
     238,   239,   240,   241,   242,   243,   244,   245,   246,   247,
     248,   249,   250,   251,   252,     0,   253,     0,   254,   255,
     256,   257,   258,   259,   260,     0,   620,     1,     2,     0,
       0,     0,     3,     0,     0,     0,     4,     5,     6,     7,
       8,     9,    10,    11,    12,     0,    13,     0,     0,   425,
     426,     0,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,   427,    30,    31,    32,    33,
      34,     0,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,     0,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
       0,    72,    73,    74,    75,    76,     0,    77,    78,    79,
       0,     0,    80,    81,     0,     0,     0,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,     0,    92,     0,
       0,     0,    93,     0,    94,    95,    96,    97,     0,    98,
       0,     0,     0,    99,   100,   101,   102,   103,     0,     0,
       0,     0,     0,     0,   104,   105,   106,   107,   108,   109,
     110,   111,   112,     0,   113,   114,   115,   116,   117,     0,
       0,     0,   118,   119,   120,   121,   122,   123,   124,   125,
       0,   126,     0,   127,   128,     0,     0,   129,   130,   131,
       0,   830,     0,   132,     0,   133,   134,   135,   136,   137,
     138,     0,   139,     0,   140,     0,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,   151,     0,     0,   152,
     153,   154,     0,   155,   428,   157,   158,   159,   160,   161,
     162,   163,   164,   165,   166,   167,   168,   169,   170,   171,
     172,   173,   174,   175,   176,   177,   178,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,   197,   198,   199,   200,   201,
       0,   202,   203,   204,     0,   205,   206,   207,   208,   209,
     210,   211,   212,   213,   214,   215,   216,   217,   218,   219,
     220,   221,   222,   223,   224,   225,   226,   227,   228,   229,
     230,   231,   232,   233,   234,   235,   236,   237,   238,   239,
     240,   241,   242,   243,   244,   245,   246,   247,   248,   249,
     250,   251,   252,     0,   253,     0,   254,   255,   256,   257,
     258,   259,   260,     1,     2,     0,     0,     0,     3,     0,
       0,     0,     4,     5,     6,     7,     8,     9,    10,    11,
      12,     0,    13,     0,     0,   425,   426,     0,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,   427,    30,    31,    32,    33,    34,     0,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,     0,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,     0,    72,    73,    74,
      75,    76,     0,    77,    78,    79,     0,     0,    80,    81,
       0,     0,     0,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,     0,    92,     0,     0,     0,    93,     0,
      94,    95,    96,    97,     0,    98,     0,     0,     0,    99,
     100,   101,   102,   103,     0,     0,     0,     0,     0,     0,
     104,   105,   106,   107,   108,   109,   110,   111,   112,     0,
     113,   114,   115,   116,   117,     0,     0,     0,   118,   119,
     120,   121,   122,   123,   124,   125,     0,   126,     0,   127,
     128,     0,     0,   129,   130,   131,     0,     0,     0,   132,
       0,   133,   134,   135,   136,   137,   138,     0,   139,     0,
     140,     0,   141,   142,   143,   144,   145,   146,   147,   148,
     149,   150,   151,     0,     0,   152,   153,   154,     0,   155,
     428,   157,   158,   159,   160,   161,   162,   163,   164,   165,
     166,   167,   168,   169,   170,   171,   172,   173,   174,   175,
     176,   177,   178,   179,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   197,   198,   199,   200,   201,     0,   202,   203,   204,
       0,   205,   206,   207,   208,   209,   210,   211,   212,   213,
     214,   215,   216,   217,   218,   219,   220,   221,   222,   223,
     224,   225,   226,   227,   228,   229,   230,   231,   232,   233,
     234,   235,   236,   237,   238,   239,   240,   241,   242,   243,
     244,   245,   246,   247,   248,   249,   250,   251,   252,     0,
     253,     0,   254,   255,   256,   257,   258,   259,   260,     1,
       2,     0,     0,     0,     3,     0,     0,     0,     4,     5,
       6,     7,     8,     9,    10,    11,    12,     0,    13,     0,
       0,    14,    15,     0,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,     0,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,     0,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,     0,    72,    73,    74,    75,    76,     0,    77,
      78,    79,     0,     0,    80,    81,     0,     0,     0,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,     0,
      92,     0,     0,     0,    93,     0,    94,    95,    96,    97,
       0,    98,     0,     0,     0,    99,   100,   101,   102,   103,
       0,     0,     0,     0,     0,     0,   104,   105,   106,   107,
     108,   109,   110,   111,   112,     0,   113,   114,   115,   116,
     117,     0,     0,     0,   118,   119,   120,   121,   122,   123,
     124,   125,     0,   126,     0,   127,   128,     0,     0,   129,
     130,   131,     0,     0,     0,   132,     0,   133,   134,   135,
     136,   137,   138,     0,   139,     0,   140,     0,   141,   142,
     143,   144,   145,   146,   147,   148,   149,   150,   151,     0,
       0,   152,   153,   154,     0,   155,   156,   157,   158,   159,
     160,   161,   162,   163,   164,   165,   166,   167,   168,   169,
     170,   171,   172,   173,   174,   175,   176,   177,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,   197,   198,   199,
     200,   201,     0,   202,   203,   204,     0,   205,   206,   207,
     208,   209,   210,   211,   212,   213,   214,   215,   216,   217,
     218,   219,   220,   221,   222,   223,   224,   225,   226,   227,
     228,   229,   230,   231,   232,   233,   234,   235,   236,   237,
     238,   239,   240,   241,   242,   243,   244,   245,   246,   247,
     248,   249,   250,   251,   252,     0,   253,     0,   254,   255,
     256,   257,   258,   259,   260,     1,     2,     0,     0,     0,
       3,     0,     0,     0,     4,     5,     6,     7,     8,     9,
      10,    11,    12,     0,    13,     0,     0,    14,    15,     0,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,     0,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,     0,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,     0,    72,
      73,    74,    75,    76,     0,    77,    78,    79,     0,     0,
      80,    81,     0,     0,     0,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,     0,    92,     0,     0,     0,
      93,     0,    94,    95,    96,    97,     0,    98,     0,     0,
       0,    99,   100,   101,   102,   103,     0,     0,     0,     0,
       0,     0,   104,   105,   106,   107,   108,   109,   110,   111,
     112,     0,   113,   114,   115,   116,   117,     0,     0,     0,
     118,   119,   120,   121,   122,   123,   124,   125,     0,   126,
       0,   127,   128,     0,     0,   129,   130,   131,     0,     0,
       0,   132,     0,   133,   134,   135,   136,   137,   138,     0,
     139,     0,   140,     0,   141,   142,   143,   144,   145,   146,
     147,   148,   149,   150,   151,     0,     0,   152,   153,   154,
       0,   155,   428,   157,   158,   159,   160,   161,   162,   163,
     164,   165,   166,   167,   168,   169,   170,   171,   172,   173,
     174,   175,   176,   177,   178,   179,   180,   181,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,   197,   198,   199,   200,   201,     0,   202,
     203,   204,     0,   205,   206,   207,   208,   209,   210,   211,
     212,   213,   214,   215,   216,   217,   218,   219,   220,   221,
     222,   223,   224,   225,   226,   227,   228,   229,   230,   231,
     232,   233,   234,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,   245,   246,   247,   248,   249,   250,   251,
     252,     0,   253,     0,   254,   255,   256,   257,   258,   259,
     260,     1,     2,     0,     0,     0,     3,     0,     0,     0,
       4,     5,     6,     7,     8,     9,    10,    11,    12,     0,
      13,     0,     0,    14,   426,     0,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,     0,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,     0,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,     0,    72,    73,    74,    75,    76,
       0,    77,    78,    79,     0,     0,    80,    81,     0,     0,
       0,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,     0,    92,     0,     0,     0,    93,     0,    94,    95,
      96,    97,     0,    98,     0,     0,     0,    99,   100,   101,
     102,   103,     0,     0,     0,     0,     0,     0,   104,   105,
     106,   107,   108,   109,   110,   111,   112,     0,   113,   114,
     115,   116,   117,     0,     0,     0,   118,   119,   120,   121,
     122,   123,   124,   125,     0,   126,     0,   127,   128,     0,
       0,   129,   130,   131,     0,     0,     0,   132,     0,   133,
     134,   135,   136,   137,   138,     0,   139,     0,   140,     0,
     141,   142,   143,   144,   145,   146,   147,   148,   149,   150,
     151,     0,     0,   152,   153,   154,     0,   155,   428,   157,
     158,   159,   160,   161,   162,   163,   164,   165,   166,   167,
     168,   169,   170,   171,   172,   173,   174,   175,   176,   177,
     178,   179,   180,   181,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   197,
     198,   199,   200,   201,     0,   202,   203,   204,     0,   205,
     206,   207,   208,   209,   210,   211,   212,   213,   214,   215,
     216,   217,   218,   219,   220,   221,   222,   223,   224,   225,
     226,   227,   228,   229,   230,   231,   232,   233,   234,   235,
     236,   237,   238,   239,   240,   241,   242,   243,   244,   245,
     246,   247,   248,   249,   250,   251,   252,     0,   253,     0,
     254,   255,   256,   257,   258,   259,   260,     1,     2,     0,
       0,     0,     3,     0,     0,     0,     4,     5,     6,     7,
       8,     9,    10,    11,    12,     0,    13,     0,     0,   901,
     426,     0,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,   427,    30,    31,    32,    33,
      34,     0,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,     0,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
       0,    72,    73,    74,    75,    76,     0,    77,    78,    79,
       0,     0,    80,    81,     0,     0,     0,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,     0,    92,     0,
       0,     0,    93,     0,    94,    95,    96,    97,     0,    98,
       0,     0,     0,    99,   100,   101,   102,   103,     0,     0,
       0,     0,     0,     0,   104,   105,   106,   107,   108,   109,
     110,   111,   112,     0,   113,   114,   115,   116,   117,     0,
       0,     0,   118,   119,   120,   121,   122,   123,   124,   125,
       0,   126,     0,   127,   128,     0,     0,   129,   130,   131,
       0,     0,     0,   132,     0,   133,   134,   135,   136,   137,
     138,     0,   139,     0,   140,     0,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,   151,     0,     0,   152,
     153,   154,     0,   155,   428,   157,   158,   159,   160,   161,
     162,   163,   164,   165,   166,   167,   168,   169,   170,   171,
     172,   173,   174,   175,   176,   177,   178,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,   197,   198,   199,   200,   201,
       0,   202,   203,   204,     0,   205,   206,   207,   208,   209,
     210,   211,   212,   213,   214,   215,   216,   217,   218,   219,
     220,   221,   222,   223,   224,   225,   226,   227,   228,   229,
     230,   231,   232,   233,   234,   235,   236,   237,   238,   239,
     240,   241,   242,   243,   244,   245,   246,   247,   248,   249,
     250,   251,   252,     0,   253,     0,   254,   255,   256,   257,
     258,   259,   260,     1,     2,     0,     0,     0,     3,     0,
       0,     0,     4,     5,     6,     7,     8,     9,    10,    11,
      12,     0,    13,     0,     0,   425,   426,     0,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
    1368,   427,    30,    31,    32,    33,    34,     0,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,     0,    48,    49,    50,  1369,  1370,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,     0,    72,    73,    74,
      75,    76,     0,    77,    78,    79,     0,     0,    80,    81,
       0,     0,     0,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,     0,    92,     0,     0,     0,    93,     0,
      94,    95,    96,    97,     0,    98,     0,     0,     0,    99,
     100,   101,   102,   103,     0,     0,     0,     0,     0,     0,
     104,   105,   106,   107,   108,   109,   110,   111,   112,     0,
     113,   114,   115,   116,   117,     0,     0,     0,   118,   119,
     120,   121,   122,   123,   124,   125,     0,   126,     0,   127,
     128,     0,     0,   129,   130,   131,     0,     0,     0,   132,
       0,   133,   134,   135,   136,   137,   138,     0,   139,     0,
     140,     0,   141,   142,   143,   144,   145,   146,   147,   148,
     149,   150,   151,     0,     0,   152,   153,   154,     0,   155,
     428,   157,   158,   159,   160,   161,   162,   163,   164,   165,
     166,   167,   168,   169,   170,   171,   172,   173,   174,   175,
     176,   177,   178,   179,   180,   181,   182,   183,   184,   185,
     186,   187,  1371,   189,   190,   191,   192,   193,   194,   195,
     196,   197,   198,   199,   200,   201,     0,   202,   203,   204,
       0,   205,   206,   207,   208,   209,   210,   211,   212,   213,
     214,   215,   216,   217,   218,   219,   220,   221,   222,   223,
     224,   225,   226,   227,   228,   229,   230,   231,   232,   233,
     234,   235,   236,   237,   238,   239,   240,   241,   242,   243,
     244,   245,   246,   247,   248,   249,   250,   251,   252,     0,
     253,     0,   254,   255,   256,   257,   258,   259,   260,     1,
       2,     0,     0,     0,     3,     0,     0,     0,     4,     5,
       6,     7,     8,     9,    10,     0,     0,     0,    13,     0,
       0,   425,   426,     0,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,   568,   427,    30,    31,
      32,    33,    34,     0,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,     0,    48,    49,
      50,   569,   570,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,     0,    72,    73,    74,    75,    76,     0,    77,
      78,    79,     0,     0,    80,    81,     0,     0,     0,    82,
      83,   516,    85,    86,    87,    88,    89,   571,    91,     0,
      92,     0,     0,     0,    93,     0,    94,    95,    96,    97,
       0,    98,     0,     0,     0,    99,   100,   101,   102,   103,
       0,     0,     0,     0,   962,     0,   104,   105,   106,   107,
     108,   109,   110,   572,   112,     0,   113,   114,   115,   116,
     117,     0,     0,     0,   118,   119,   120,   121,   122,   123,
     124,   125,     0,   126,     0,   127,   128,     0,     0,   129,
     130,   131,     0,     0,     0,   132,   963,   133,   134,   135,
       0,   137,   138,     0,   139,     0,   140,     0,   141,   142,
     143,   144,   145,   146,   147,   148,   149,   150,   151,     0,
       0,   152,   153,   154,     0,   155,   428,   157,   158,   159,
     160,   161,   162,   163,   164,   165,   166,   167,   168,   169,
     170,   171,   172,   173,   174,   175,   176,   177,   178,   179,
     180,   181,   534,   535,   536,   185,   186,   537,   574,   189,
     190,   191,   192,   193,   194,   195,   196,   197,   198,   199,
     200,   201,     0,   202,   203,   204,     0,   205,   206,   207,
     208,   209,   210,   211,   212,   213,   214,   215,   216,   217,
     218,   219,   220,   221,   222,   223,   224,   225,   226,   227,
     228,   229,   230,   231,   232,   233,   234,   235,   236,   237,
     238,   239,   240,   241,   242,   243,   244,   245,   246,   247,
     248,   249,   250,   251,   252,     0,   253,     0,   254,   255,
     256,   257,   258,   259,   260,     1,     2,     0,     0,     0,
       3,     0,     0,     0,     4,     5,     6,     7,     8,     9,
      10,     0,     0,     0,    13,     0,     0,   425,   426,     0,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,   568,   427,    30,    31,    32,    33,    34,     0,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,     0,    48,    49,    50,   569,   570,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,     0,    72,
      73,    74,    75,    76,     0,    77,    78,    79,     0,     0,
      80,    81,     0,     0,     0,    82,    83,   516,    85,    86,
      87,    88,    89,   571,    91,     0,    92,     0,     0,     0,
      93,     0,    94,    95,    96,    97,     0,    98,     0,     0,
       0,    99,   100,   101,   102,   103,     0,     0,     0,     0,
     962,     0,   104,   105,   106,   107,   108,   109,   110,   572,
     112,     0,   113,   114,   115,   116,   117,     0,     0,     0,
     118,   119,   120,   121,   122,   123,   124,   125,     0,   126,
       0,   127,   128,     0,     0,   129,   130,   131,     0,     0,
       0,   132,   968,   133,   134,   135,     0,   137,   138,     0,
     139,     0,   140,     0,   141,   142,   143,   144,   145,   146,
     147,   148,   149,   150,   151,     0,     0,   152,   153,   154,
       0,   155,   428,   157,   158,   159,   160,   161,   162,   163,
     164,   165,   166,   167,   168,   169,   170,   171,   172,   173,
     174,   175,   176,   177,   178,   179,   180,   181,   534,   535,
     536,   185,   186,   537,   574,   189,   190,   191,   192,   193,
     194,   195,   196,   197,   198,   199,   200,   201,     0,   202,
     203,   204,     0,   205,   206,   207,   208,   209,   210,   211,
     212,   213,   214,   215,   216,   217,   218,   219,   220,   221,
     222,   223,   224,   225,   226,   227,   228,   229,   230,   231,
     232,   233,   234,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,   245,   246,   247,   248,   249,   250,   251,
     252,     0,   253,     0,   254,   255,   256,   257,   258,   259,
     260,     1,     2,     0,     0,     0,     3,     0,     0,     0,
       4,     5,     6,     7,     8,     9,    10,     0,     0,     0,
      13,     0,     0,   425,   426,     0,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,   568,   427,
      30,    31,    32,    33,    34,     0,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,     0,
      48,    49,    50,   569,   570,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,     0,    72,    73,    74,    75,    76,
       0,    77,    78,    79,     0,     0,    80,    81,     0,     0,
       0,    82,    83,   516,    85,    86,    87,    88,    89,   571,
      91,     0,    92,     0,     0,     0,    93,     0,    94,    95,
      96,    97,     0,    98,     0,     0,     0,    99,   100,   101,
     102,   103,     0,     0,     0,     0,     0,     0,   104,   105,
     106,   107,   108,   109,   110,   572,   112,     0,   113,   114,
     115,   116,   117,     0,     0,     0,   118,   119,   120,   121,
     122,   123,   124,   125,     0,   126,     0,   127,   128,     0,
       0,   129,   130,   131,     0,     0,   573,   132,     0,   133,
     134,   135,     0,   137,   138,     0,   139,     0,   140,     0,
     141,   142,   143,   144,   145,   146,   147,   148,   149,   150,
     151,     0,     0,   152,   153,   154,     0,   155,   428,   157,
     158,   159,   160,   161,   162,   163,   164,   165,   166,   167,
     168,   169,   170,   171,   172,   173,   174,   175,   176,   177,
     178,   179,   180,   181,   534,   535,   536,   185,   186,   537,
     574,   189,   190,   191,   192,   193,   194,   195,   196,   197,
     198,   199,   200,   201,     0,   202,   203,   204,     0,   205,
     206,   207,   208,   209,   210,   211,   212,   213,   214,   215,
     216,   217,   218,   219,   220,   221,   222,   223,   224,   225,
     226,   227,   228,   229,   230,   231,   232,   233,   234,   235,
     236,   237,   238,   239,   240,   241,   242,   243,   244,   245,
     246,   247,   248,   249,   250,   251,   252,     0,   253,     0,
     254,   255,   256,   257,   258,   259,   260,     1,     2,     0,
       0,     0,     3,     0,     0,     0,     4,     5,     6,     7,
       8,     9,    10,     0,     0,     0,    13,     0,     0,   425,
     426,     0,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,   568,   427,    30,    31,    32,    33,
      34,     0,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,     0,    48,    49,    50,   569,
     570,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
       0,    72,    73,    74,    75,    76,     0,    77,    78,    79,
       0,     0,    80,    81,     0,     0,     0,    82,    83,   516,
      85,    86,    87,    88,    89,   571,    91,     0,    92,     0,
       0,     0,    93,     0,    94,    95,    96,    97,     0,    98,
       0,     0,     0,    99,   100,   101,   102,   103,     0,     0,
       0,     0,     0,     0,   104,   105,   106,   107,   108,   109,
     110,   572,   112,     0,   113,   114,   115,   116,   117,     0,
       0,     0,   118,   119,   120,   121,   122,   123,   124,   125,
       0,   126,     0,   127,   128,     0,     0,   129,   130,   131,
       0,     0,     0,   132,   578,   133,   134,   135,     0,   137,
     138,     0,   139,     0,   140,     0,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,   151,     0,     0,   152,
     153,   154,     0,   155,   428,   157,   158,   159,   160,   161,
     162,   163,   164,   165,   166,   167,   168,   169,   170,   171,
     172,   173,   174,   175,   176,   177,   178,   179,   180,   181,
     534,   535,   536,   185,   186,   537,   574,   189,   190,   191,
     192,   193,   194,   195,   196,   197,   198,   199,   200,   201,
       0,   202,   203,   204,     0,   205,   206,   207,   208,   209,
     210,   211,   212,   213,   214,   215,   216,   217,   218,   219,
     220,   221,   222,   223,   224,   225,   226,   227,   228,   229,
     230,   231,   232,   233,   234,   235,   236,   237,   238,   239,
     240,   241,   242,   243,   244,   245,   246,   247,   248,   249,
     250,   251,   252,     0,   253,     0,   254,   255,   256,   257,
     258,   259,   260,     1,     2,     0,     0,     0,     3,     0,
       0,     0,     4,     5,     6,     7,     8,     9,    10,     0,
       0,     0,    13,     0,     0,   425,   426,     0,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
     568,   427,    30,    31,    32,    33,    34,     0,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,     0,    48,    49,    50,   569,   570,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,     0,    72,    73,    74,
      75,    76,     0,    77,    78,    79,     0,     0,    80,    81,
       0,     0,     0,    82,    83,   516,    85,    86,    87,    88,
      89,   571,    91,     0,    92,     0,     0,     0,    93,     0,
      94,    95,    96,    97,     0,    98,     0,     0,     0,    99,
     100,   101,   102,   103,     0,     0,     0,     0,     0,     0,
     104,   105,   106,   107,   108,   109,   110,   572,   112,     0,
     113,   114,   115,   116,   117,     0,     0,     0,   118,   119,
     120,   121,   122,   123,   124,   125,     0,   126,     0,   127,
     128,     0,     0,   129,   130,   131,     0,     0,     0,   132,
       0,   133,   134,   135,     0,   137,   138,     0,   139,     0,
     140,     0,   141,   142,   143,   144,   145,   146,   147,   148,
     149,   150,   151,     0,     0,   152,   153,   154,     0,   155,
     428,   157,   158,   159,   160,   161,   162,   163,   164,   165,
     166,   167,   168,   169,   170,   171,   172,   173,   174,   175,
     176,   177,   178,   179,   180,   181,   534,   535,   536,   185,
     186,   537,   574,   189,   190,   191,   192,   193,   194,   195,
     196,   197,   198,   199,   200,   201,     0,   202,   203,   204,
       0,   205,   206,   207,   208,   209,   210,   211,   212,   213,
     214,   215,   216,   217,   218,   219,   220,   221,   222,   223,
     224,   225,   226,   227,   228,   229,   230,   231,   232,   233,
     234,   235,   236,   237,   238,   239,   240,   241,   242,   243,
     244,   245,   246,   247,   248,   249,   250,   251,   252,   615,
     253,     0,   254,   255,   256,   257,   258,   259,   260,     1,
       2,     0,     0,     0,     3,     0,     0,     0,     4,     5,
       6,     7,     8,     9,    10,     0,     0,     0,    13,     0,
       0,   425,   426,     0,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,   568,   427,    30,    31,
      32,    33,    34,     0,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,     0,    48,    49,
      50,   569,   570,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,     0,    72,    73,    74,    75,    76,     0,    77,
      78,    79,     0,     0,    80,    81,     0,     0,     0,    82,
      83,   516,    85,    86,    87,    88,    89,   571,    91,     0,
      92,     0,     0,     0,    93,     0,    94,    95,    96,    97,
       0,    98,     0,     0,     0,    99,   100,   101,   102,   103,
       0,     0,     0,     0,     0,     0,   104,   105,   106,   107,
     108,   109,   110,   572,   112,     0,   113,   114,   115,   116,
     117,     0,     0,     0,   118,   119,   120,   121,   122,   123,
     124,   125,     0,   126,     0,   127,   128,     0,     0,   129,
     130,   131,     0,     0,     0,   132,     0,   133,   134,   135,
       0,   137,   138,     0,   139,     0,   140,     0,   141,   142,
     143,   144,   145,   146,   147,   148,   149,   150,   151,     0,
       0,   152,   153,   154,     0,   155,   428,   157,   158,   159,
     160,   161,   162,   163,   164,   165,   166,   167,   168,   169,
     170,   171,   172,   173,   174,   175,   176,   177,   178,   179,
     180,   181,   534,   535,   536,   185,   186,   537,   574,   189,
     190,   191,   192,   193,   194,   195,   196,   197,   198,   199,
     200,   201,     0,   202,   203,   204,     0,   205,   206,   207,
     208,   209,   210,   211,   212,   213,   214,   215,   216,   217,
     218,   219,   220,   221,   222,   223,   224,   225,   226,   227,
     228,   229,   230,   231,   232,   233,   234,   235,   236,   237,
     238,   239,   240,   241,   242,   243,   244,   245,   246,   247,
     248,   249,   250,   251,   252,     0,   253,   617,   254,   255,
     256,   257,   258,   259,   260,     1,     2,     0,     0,     0,
       3,     0,     0,     0,     4,     5,     6,     7,     8,     9,
      10,     0,     0,     0,    13,     0,     0,   425,   426,     0,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,   568,   427,    30,    31,    32,    33,    34,     0,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,     0,    48,    49,    50,   569,   570,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,     0,    72,
      73,    74,    75,    76,     0,    77,    78,    79,     0,     0,
      80,    81,     0,     0,     0,    82,    83,   516,    85,    86,
      87,    88,    89,   571,    91,     0,    92,     0,     0,     0,
      93,     0,    94,    95,    96,    97,     0,    98,     0,     0,
       0,    99,   100,   101,   102,   103,     0,     0,     0,     0,
       0,     0,   104,   105,   106,   107,   108,   109,   110,   572,
     112,     0,   113,   114,   115,   116,   117,     0,     0,     0,
     118,   119,   120,   121,   122,   123,   124,   125,     0,   126,
       0,   127,   128,     0,     0,   129,   130,   131,     0,     0,
     961,   132,     0,   133,   134,   135,     0,   137,   138,     0,
     139,     0,   140,     0,   141,   142,   143,   144,   145,   146,
     147,   148,   149,   150,   151,     0,     0,   152,   153,   154,
       0,   155,   428,   157,   158,   159,   160,   161,   162,   163,
     164,   165,   166,   167,   168,   169,   170,   171,   172,   173,
     174,   175,   176,   177,   178,   179,   180,   181,   534,   535,
     536,   185,   186,   537,   574,   189,   190,   191,   192,   193,
     194,   195,   196,   197,   198,   199,   200,   201,     0,   202,
     203,   204,     0,   205,   206,   207,   208,   209,   210,   211,
     212,   213,   214,   215,   216,   217,   218,   219,   220,   221,
     222,   223,   224,   225,   226,   227,   228,   229,   230,   231,
     232,   233,   234,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,   245,   246,   247,   248,   249,   250,   251,
     252,     0,   253,     0,   254,   255,   256,   257,   258,   259,
     260,     1,     2,     0,     0,     0,     3,     0,     0,     0,
       4,     5,     6,     7,     8,     9,    10,     0,     0,     0,
      13,     0,     0,   425,   426,     0,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,   568,   427,
      30,    31,    32,    33,    34,     0,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,     0,
      48,    49,    50,   569,   570,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,     0,    72,    73,    74,    75,    76,
       0,    77,    78,    79,     0,     0,    80,    81,     0,     0,
       0,    82,    83,   516,    85,    86,    87,    88,    89,   571,
      91,     0,    92,     0,     0,     0,    93,     0,    94,    95,
      96,    97,     0,    98,     0,     0,     0,    99,   100,   101,
     102,   103,     0,     0,     0,     0,     0,     0,   104,   105,
     106,   107,   108,   109,   110,   572,   112,     0,   113,   114,
     115,   116,   117,     0,     0,     0,   118,   119,   120,   121,
     122,   123,   124,   125,     0,   126,     0,   127,   128,     0,
       0,   129,   130,   131,     0,     0,     0,   132,  1200,   133,
     134,   135,     0,   137,   138,     0,   139,     0,   140,     0,
     141,   142,   143,   144,   145,   146,   147,   148,   149,   150,
     151,     0,     0,   152,   153,   154,     0,   155,   428,   157,
     158,   159,   160,   161,   162,   163,   164,   165,   166,   167,
     168,   169,   170,   171,   172,   173,   174,   175,   176,   177,
     178,   179,   180,   181,   534,   535,   536,   185,   186,   537,
     574,   189,   190,   191,   192,   193,   194,   195,   196,   197,
     198,   199,   200,   201,     0,   202,   203,   204,     0,   205,
     206,   207,   208,   209,   210,   211,   212,   213,   214,   215,
     216,   217,   218,   219,   220,   221,   222,   223,   224,   225,
     226,   227,   228,   229,   230,   231,   232,   233,   234,   235,
     236,   237,   238,   239,   240,   241,   242,   243,   244,   245,
     246,   247,   248,   249,   250,   251,   252,     0,   253,     0,
     254,   255,   256,   257,   258,   259,   260,     1,     2,     0,
       0,     0,     3,     0,     0,     0,     4,     5,     6,     7,
       8,     9,    10,     0,     0,     0,    13,     0,     0,   425,
     426,     0,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,   568,   427,    30,    31,    32,    33,
      34,     0,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,     0,    48,    49,    50,   569,
     570,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
       0,    72,    73,    74,    75,    76,     0,    77,    78,    79,
       0,     0,    80,    81,     0,     0,     0,    82,    83,   516,
      85,    86,    87,    88,    89,   571,    91,     0,    92,     0,
       0,     0,    93,     0,    94,    95,    96,    97,     0,    98,
       0,     0,     0,    99,   100,   101,   102,   103,     0,     0,
       0,     0,  1463,     0,   104,   105,   106,   107,   108,   109,
     110,   572,   112,     0,   113,   114,   115,   116,   117,     0,
       0,     0,   118,   119,   120,   121,   122,   123,   124,   125,
       0,   126,     0,   127,   128,     0,     0,   129,   130,   131,
       0,     0,     0,   132,     0,   133,   134,   135,     0,   137,
     138,     0,   139,     0,   140,     0,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,   151,     0,     0,   152,
     153,   154,     0,   155,   428,   157,   158,   159,   160,   161,
     162,   163,   164,   165,   166,   167,   168,   169,   170,   171,
     172,   173,   174,   175,   176,   177,   178,   179,   180,   181,
     534,   535,   536,   185,   186,   537,   574,   189,   190,   191,
     192,   193,   194,   195,   196,   197,   198,   199,   200,   201,
       0,   202,   203,   204,     0,   205,   206,   207,   208,   209,
     210,   211,   212,   213,   214,   215,   216,   217,   218,   219,
     220,   221,   222,   223,   224,   225,   226,   227,   228,   229,
     230,   231,   232,   233,   234,   235,   236,   237,   238,   239,
     240,   241,   242,   243,   244,   245,   246,   247,   248,   249,
     250,   251,   252,     0,   253,     0,   254,   255,   256,   257,
     258,   259,   260,     1,     2,     0,     0,     0,     3,     0,
       0,     0,     4,     5,     6,     7,     8,     9,    10,     0,
       0,     0,    13,     0,     0,   425,   426,     0,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,   502,    27,
     504,   427,    30,   505,    32,    33,    34,     0,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
     507,     0,    48,    49,    50,   509,   510,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,     0,    72,    73,    74,
      75,    76,     0,    77,    78,    79,     0,     0,    80,    81,
       0,     0,     0,    82,    83,   516,    85,    86,    87,    88,
      89,   571,    91,     0,    92,     0,     0,     0,    93,     0,
      94,    95,    96,    97,     0,    98,     0,     0,     0,   519,
     100,   101,   102,   103,     0,     0,     0,     0,     0,     0,
     104,   105,   106,   107,   108,   109,   110,   572,   112,     0,
     113,   114,   704,   116,   117,     0,     0,     0,   118,   119,
     120,   121,   122,   123,   124,   125,     0,   126,     0,   705,
     128,     0,     0,   129,   130,   131,     0,     0,     0,   132,
       0,   133,   134,   135,     0,   137,   138,     0,   139,     0,
     140,     0,   141,   142,   143,   144,   145,   146,   147,   148,
     149,   150,   151,     0,     0,   152,   153,   154,     0,   155,
     428,   157,   158,   159,   160,   161,   162,   163,   164,   165,
     166,   167,   529,   169,   530,   171,   531,   532,   174,   175,
     176,   177,   178,   179,   533,   181,   534,   535,   536,   185,
     186,   537,   538,   189,   190,   191,   192,   193,   194,   195,
     196,   197,   198,   199,   200,   539,     0,   202,   540,   204,
       0,   205,   206,   207,   208,   209,   210,   211,   212,   213,
     214,   215,   216,   217,   218,   219,   220,   221,   222,   223,
     224,   225,   226,   227,   228,   229,   230,   231,   232,   233,
     234,   235,   236,   237,   238,   239,   240,   241,   242,   243,
     244,   245,   246,   247,   248,   249,   250,   251,   252,     0,
     253,     0,   254,   541,   256,   257,   258,   259,   260,     1,
       2,     0,     0,     0,     3,     0,     0,     0,     4,     5,
       6,     7,     8,     9,    10,     0,     0,     0,    13,     0,
       0,   425,   426,     0,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,   568,   427,    30,    31,
      32,    33,    34,     0,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,     0,    48,    49,
      50,   569,   570,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,     0,    72,    73,    74,    75,    76,     0,    77,
      78,    79,     0,     0,    80,    81,     0,     0,     0,    82,
      83,   516,    85,    86,    87,    88,    89,   571,    91,     0,
      92,     0,     0,     0,    93,     0,    94,    95,    96,    97,
       0,    98,     0,     0,     0,    99,   100,   101,   102,   103,
       0,     0,     0,     0,     0,     0,   104,   105,   106,   107,
     108,   109,   110,   572,   112,     0,   113,   114,   115,   116,
     117,     0,     0,     0,   118,   119,   120,   121,   122,   123,
     124,   125,     0,   126,     0,   127,   128,     0,     0,   129,
     130,   131,     0,     0,     0,   132,     0,   133,   134,   135,
       0,   137,   138,     0,   139,     0,   140,     0,   141,   142,
     143,   144,   145,   146,   147,   148,   149,   150,   151,     0,
       0,   152,   153,   154,     0,   155,   428,   157,   158,   159,
     160,   161,   162,   163,   164,   165,   166,   167,   168,   169,
     170,   171,   172,   173,   174,   175,   176,   177,   178,   179,
     180,   181,   534,   535,   536,   185,   186,   537,   574,   189,
     190,   191,   192,   193,   194,   195,   196,   197,   198,   199,
     200,   201,     0,   202,   203,   204,     0,   205,   206,   207,
     208,   209,   210,   211,   212,   213,   214,   215,   216,   217,
     218,   219,   220,   221,   222,   223,   224,   225,   226,   227,
     228,   229,   230,   231,   232,   233,   234,   235,   236,   237,
     238,   239,   240,   241,   242,   243,   244,   245,   246,   247,
     248,   249,   250,   251,   252,     0,   253,     0,   254,   255,
     256,   257,   258,   259,   260,     1,     2,     0,     0,     0,
       3,     0,     0,     0,     4,     5,     6,     7,     8,     9,
      10,     0,     0,     0,    13,     0,     0,   425,   426,     0,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
     502,    27,   504,   427,    30,   505,    32,    33,    34,     0,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,   507,     0,    48,    49,    50,   509,   510,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,     0,    72,
      73,    74,    75,    76,     0,    77,    78,    79,     0,     0,
      80,    81,     0,     0,     0,    82,    83,   516,    85,    86,
      87,    88,    89,   571,    91,     0,    92,     0,     0,     0,
      93,     0,    94,    95,    96,    97,     0,    98,     0,     0,
       0,   519,   100,   101,   102,   103,     0,     0,     0,     0,
       0,     0,   104,   105,   106,   107,   108,   109,   110,   572,
     112,     0,   113,   114,   115,   116,   117,     0,     0,     0,
     118,   119,   120,   121,   122,   123,   124,   125,     0,   126,
       0,   127,   128,     0,     0,   129,   130,   131,     0,     0,
       0,   132,     0,   133,   134,   135,     0,   137,   138,     0,
     139,     0,   140,     0,   141,   142,   143,   144,   145,   146,
     147,   148,   149,   150,   151,     0,     0,   152,   153,   154,
       0,   155,   428,   157,   158,   159,   160,   161,   162,   163,
     164,   165,   166,   167,   529,   169,   530,   171,   531,   532,
     174,   175,   176,   177,   178,   179,   533,   181,   534,   535,
     536,   185,   186,   537,   538,   189,   190,   191,   192,   193,
     194,   195,   196,   197,   198,   199,   200,   539,     0,   202,
     203,   204,     0,   205,   206,   207,   208,   209,   210,   211,
     212,   213,   214,   215,   216,   217,   218,   219,   220,   221,
     222,   223,   224,   225,   226,   227,   228,   229,   230,   231,
     232,   233,   234,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,   245,   246,   247,   248,   249,   250,   251,
     252,     0,   253,     0,   254,   541,   256,   257,   258,   259,
     260,     1,     2,     0,     0,     0,     3,     0,     0,     0,
       4,     5,     6,     7,     8,     9,    10,     0,     0,     0,
      13,     0,     0,   425,   426,     0,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,   502,    27,   504,   427,
      30,   505,    32,    33,    34,     0,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,   507,     0,
      48,    49,    50,   509,   510,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,     0,    72,    73,    74,    75,    76,
       0,    77,    78,    79,     0,     0,    80,    81,     0,     0,
       0,    82,    83,   516,    85,    86,    87,    88,    89,   571,
      91,     0,    92,     0,     0,     0,    93,     0,    94,    95,
      96,    97,     0,    98,     0,     0,     0,   519,   100,   101,
     102,   103,     0,     0,     0,     0,     0,     0,   104,   105,
     106,   107,   108,   109,   110,   572,   112,     0,   113,   114,
     115,   116,   117,     0,     0,     0,   118,   119,   120,   121,
     122,   123,   124,   125,     0,   126,     0,   127,   128,     0,
       0,   129,   130,   131,     0,     0,     0,   132,     0,   133,
     134,   135,     0,   137,   138,     0,   139,     0,   140,     0,
     141,   142,   143,   144,   145,   146,   147,   148,   149,   150,
     151,     0,     0,   152,   153,   154,     0,   155,   428,   157,
     158,   159,   160,   161,   162,   163,   164,   165,   166,   167,
     529,   169,   530,   171,   531,   532,   174,   175,   176,   177,
     178,   179,   533,   181,   534,   535,   536,   185,   186,   537,
     538,   189,   190,   191,   192,   193,   194,   195,   196,   197,
     198,   199,   200,   539,     0,   202,   540,   204,     0,   205,
     206,   207,   208,   209,   210,   211,   212,   213,   214,   215,
     216,   217,   218,   219,   220,   221,   222,   223,   224,   225,
     226,   227,   228,   229,   230,   231,   232,   233,   234,   235,
     236,   237,   238,   239,   240,   241,   242,   243,   244,   245,
     246,   247,   248,   249,   250,   251,   252,     0,   253,     0,
     254,   541,   256,   257,   258,   259,   260,     1,     2,     0,
       0,     0,     3,     0,     0,     0,     4,     5,     6,     7,
       8,     9,    10,     0,     0,     0,    13,     0,     0,   425,
     426,     0,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,   568,   427,    30,    31,    32,    33,
      34,     0,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,     0,    48,    49,    50,   569,
     570,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
       0,    72,    73,    74,    75,    76,     0,    77,    78,    79,
       0,     0,    80,    81,     0,     0,     0,    82,    83,   516,
      85,    86,    87,    88,    89,  1149,    91,     0,    92,     0,
       0,     0,    93,     0,    94,    95,    96,    97,     0,    98,
       0,     0,     0,    99,   100,   101,   102,   103,     0,     0,
       0,     0,     0,     0,   104,   105,   106,   107,   108,   109,
     110,   572,   112,     0,   113,   114,   115,   116,   117,     0,
       0,     0,   118,   119,   120,   121,   122,   123,   124,   125,
       0,   126,     0,   127,   128,     0,     0,   129,   130,   131,
       0,     0,     0,   132,     0,   133,   134,   135,     0,   137,
     138,     0,   139,     0,   140,     0,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,   151,     0,     0,   152,
     153,   154,     0,   155,   428,   157,   158,   159,   160,   161,
     162,   163,   164,   165,   166,   167,   168,   169,   170,   171,
     172,   173,   174,   175,   176,   177,   178,   179,   180,   181,
     534,   535,   536,   185,   186,   537,   574,   189,   190,   191,
     192,   193,   194,   195,   196,   197,   198,   199,   200,   201,
       0,   202,   203,   204,     0,   205,   206,   207,   208,   209,
     210,   211,   212,   213,   214,   215,   216,   217,   218,   219,
     220,   221,   222,   223,   224,   225,   226,   227,   228,   229,
     230,   231,   232,   233,   234,   235,   236,   237,   238,   239,
     240,   241,   242,   243,   244,   245,   246,   247,   248,   249,
     250,   251,   252,     0,   253,     0,   254,   255,   256,   257,
     258,   259,   260,     1,     2,     0,     0,     0,     3,     0,
       0,     0,     4,     5,     6,     7,     8,     9,    10,     0,
       0,     0,    13,     0,     0,   425,   426,     0,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,   502,    27,
     504,   427,    30,   505,    32,    33,    34,     0,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
     507,     0,    48,    49,    50,   509,   510,    53,   511,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,     0,    72,    73,    74,
      75,    76,     0,    77,    78,    79,     0,     0,    80,    81,
       0,     0,     0,    82,    83,   516,    85,    86,    87,    88,
      89,   571,    91,     0,    92,     0,     0,     0,    93,     0,
      94,    95,    96,    97,     0,    98,     0,     0,     0,   519,
     100,   101,   102,   103,     0,     0,     0,     0,     0,     0,
     104,   105,   106,   107,   108,   109,   110,   572,   112,     0,
     113,   114,     0,   116,   117,     0,     0,     0,   118,   119,
     120,   121,   122,   123,   124,   125,     0,   126,     0,     0,
       0,     0,     0,   129,   130,   131,     0,     0,     0,   132,
       0,   133,   134,   135,     0,   137,   138,     0,   139,     0,
     140,     0,   141,   142,   143,   144,   145,   146,   147,   148,
     149,   150,   151,     0,     0,   152,   153,   154,     0,   155,
     428,   157,   158,   159,   160,   161,   162,   163,   164,   165,
     166,   167,   529,   169,   530,   171,   531,   532,   174,   175,
     176,   177,   178,   179,   533,   181,   534,   535,   536,   185,
     186,   537,   538,   189,   190,   191,   192,   193,   194,   195,
     196,   197,   198,   199,   200,   539,     0,   202,   540,   204,
       0,   205,   206,   207,   208,   209,   210,   211,   212,   213,
     214,   215,   216,   217,   218,   219,   220,   221,   222,   223,
     224,   225,   226,   227,   228,   229,   230,   231,   232,   233,
     234,   235,   236,   237,   238,   239,   240,   241,   242,   243,
     244,   245,   246,   247,   248,   249,   250,   251,   252,     0,
     253,     0,   254,   541,   256,   257,   258,   259,   260,     1,
       2,     0,     0,     0,     3,     0,     0,     0,     4,     5,
       6,     7,     8,     9,    10,     0,     0,     0,    13,     0,
       0,   425,   426,     0,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,   502,    27,   504,   427,    30,   505,
      32,    33,    34,     0,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,   507,     0,    48,    49,
      50,   509,   510,    53,   511,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,     0,    72,    73,    74,    75,    76,     0,    77,
      78,    79,     0,     0,    80,    81,     0,     0,     0,    82,
      83,   516,    85,    86,    87,    88,    89,   571,    91,     0,
      92,     0,     0,     0,    93,     0,    94,    95,    96,    97,
       0,    98,     0,     0,     0,   519,   100,   101,   102,   103,
       0,     0,     0,     0,     0,     0,   104,   105,   106,   107,
     108,   109,   110,   572,   112,     0,   113,   114,     0,   116,
     117,     0,     0,     0,   118,   119,   120,   121,   122,   123,
     124,   125,     0,   126,     0,     0,     0,     0,     0,   129,
     130,   131,     0,     0,     0,   132,     0,   133,   134,   135,
       0,     0,     0,     0,   139,     0,   140,     0,   141,   142,
     143,   144,   145,   146,   147,   148,   149,   150,   151,     0,
       0,   152,   153,   154,     0,   155,   428,   157,   158,   159,
     160,   161,   162,   163,   164,   165,   166,   167,   529,   169,
     530,   171,   531,   532,   174,   175,   176,   177,   178,   179,
     533,   181,   534,   535,   536,   185,   186,   537,   538,   189,
     190,   191,   192,   193,   194,   195,   196,   197,   198,   199,
     200,   539,     0,   202,   540,   204,     0,   205,   206,   207,
     208,   209,   210,   211,   212,   213,   214,   215,   216,   217,
     218,   219,   220,   221,   222,   223,   224,   225,   226,   227,
     228,   229,   230,   231,   232,   233,   234,   235,   236,   237,
     238,   239,   240,   241,   242,   243,   244,   245,   246,   247,
     248,   249,   250,   251,   252,     0,   253,     0,   254,   541,
     256,   257,   258,   259,   260,     1,     2,     0,     0,     0,
       3,     0,     0,     0,     4,     5,     6,     7,     8,     9,
      10,     0,     0,     0,    13,     0,     0,   425,   426,     0,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
     502,    27,   504,   427,    30,   505,    32,    33,    34,     0,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,   507,     0,    48,    49,    50,   509,   510,    53,
     511,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,     0,    72,
      73,    74,    75,    76,     0,    77,    78,    79,     0,     0,
      80,    81,     0,     0,     0,    82,    83,   516,    85,    86,
      87,    88,    89,   571,    91,     0,    92,     0,     0,     0,
      93,     0,    94,    95,    96,    97,     0,    98,     0,     0,
       0,   519,     0,   101,   102,   103,     0,     0,     0,     0,
       0,     0,   104,   105,   106,   107,     0,   109,   110,   572,
     112,     0,   113,   114,     0,   116,   117,     0,     0,     0,
     118,   119,   120,   121,   122,   123,   124,   125,     0,   126,
       0,     0,     0,     0,     0,   129,   130,   131,     0,     0,
       0,   132,     0,   133,   134,   135,     0,     0,     0,     0,
     139,     0,   140,     0,   141,     0,   143,     0,   145,   146,
     147,   148,   149,   150,   151,     0,     0,   152,   153,   154,
       0,   155,   428,   157,   158,   159,   160,   161,   162,   163,
     164,   165,   166,   167,   529,   169,   530,   171,   531,   532,
     174,   175,   176,   177,   178,   179,   533,   181,   534,   535,
     536,   185,   186,   537,   538,   189,   190,   191,   192,   193,
     194,   195,   196,   197,   198,   199,   200,   539,     0,   202,
     540,   204,     0,   205,   206,   207,   208,   209,   210,   211,
     212,   213,   214,   215,   216,   217,   218,   219,   220,   221,
     222,   223,   224,   225,   226,   227,   228,   229,   230,   231,
     232,   233,   234,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,   245,   246,   247,   248,   249,   250,   251,
     252,     0,   253,     0,   254,   541,   256,   257,   258,   259,
     260,     1,     2,     0,     0,     0,     3,     0,     0,     0,
       4,     5,     6,     7,     8,     9,    10,     0,     0,     0,
      13,     0,     0,   425,   426,     0,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,   502,    27,   504,   427,
      30,   505,    32,    33,    34,     0,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,   507,     0,
      48,    49,    50,   509,   510,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,     0,    72,    73,    74,    75,    76,
       0,    77,    78,    79,     0,     0,    80,    81,     0,     0,
       0,    82,    83,   516,    85,    86,    87,    88,     0,   571,
      91,     0,    92,     0,     0,     0,    93,     0,    94,    95,
      96,    97,     0,    98,     0,     0,     0,   519,     0,   101,
     102,   103,     0,     0,     0,     0,     0,     0,     0,   105,
     106,   107,     0,   109,   110,   572,     0,     0,     0,   114,
       0,     0,   117,     0,     0,     0,   118,   119,   120,   121,
     122,   123,   124,   125,     0,   126,     0,     0,   128,     0,
       0,   129,   130,   131,     0,     0,     0,   132,     0,   133,
     134,   135,     0,     0,     0,     0,     0,     0,   140,     0,
     141,     0,   143,     0,   145,   146,   147,   148,   149,   150,
     151,     0,     0,   152,   153,   154,     0,   155,   428,   157,
     158,   159,   160,   161,   162,   163,   164,   165,   166,   167,
     529,   169,   530,   171,   531,   532,   174,   175,   176,   177,
     178,   179,   533,   181,   534,   535,   536,   185,   186,   537,
     538,   189,   190,   191,   192,   193,   194,   195,   196,   197,
     198,   199,   200,   539,     0,   202,   203,   204,     0,   205,
     206,   207,   208,   209,   210,   211,   212,   213,   214,   215,
     216,   217,   218,   219,   220,   221,   222,   223,   224,   225,
     226,   227,   228,   229,   230,   231,   232,   233,   234,   235,
     236,   237,   238,   239,   240,   241,   242,   243,   244,   245,
     246,   247,   248,   249,   250,   251,   252,     0,   253,     0,
     254,   541,   256,   257,   258,   259,   260,     1,     2,     0,
       0,     0,     3,     0,     0,     0,     4,     0,     0,     0,
       0,     9,    10,     0,     0,     0,     0,     0,     0,   425,
     426,     0,    16,    17,    18,   499,    20,    21,    22,   500,
      24,   501,   502,   503,   504,   427,    30,   505,    32,    33,
      34,     0,    35,    36,    37,    38,   506,    40,    41,    42,
      43,    44,    45,    46,   507,     0,    48,   508,    50,   509,
     510,    53,   511,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,   512,   513,    71,
       0,    72,    73,    74,   514,     0,     0,    77,    78,    79,
       0,     0,   515,    81,     0,     0,     0,     0,    83,   516,
      85,   517,   518,    88,    89,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    94,    95,    96,    97,     0,    98,
       0,     0,     0,   519,   100,   101,   520,   521,     0,     0,
       0,     0,     0,     0,   104,   105,   106,     0,   108,   109,
     110,     0,     0,     0,     0,     0,     0,   116,   522,     0,
       0,     0,   118,   119,   120,   121,   523,   123,   124,   524,
       0,     0,     0,     0,     0,     0,     0,   525,   526,   131,
       0,     0,     0,   132,     0,   133,   134,   527,     0,     0,
       0,     0,   139,     0,   140,     0,   141,   142,   143,   144,
     528,   146,   147,   148,   149,   150,   151,     0,     0,   152,
     153,     0,     0,   155,   428,   157,   158,   159,   160,   161,
     162,   163,   164,   165,   166,   167,   529,   169,   530,   171,
     531,   532,   174,   175,   176,   177,   178,   179,   533,   181,
     534,   535,   536,   185,   186,   537,   538,   189,   190,   191,
     192,   193,   194,   195,   196,   197,   198,   199,   200,   539,
       0,   202,   540,   204,     0,   205,   206,   207,   208,   209,
     210,   211,   212,   213,   214,   215,   216,   217,   218,   219,
     220,   221,   222,   223,   224,   225,   226,   227,   228,   229,
     230,   231,   232,   233,   234,   235,   236,   237,   238,   239,
     240,   241,   242,   243,   244,   245,   246,   247,   248,   249,
     250,   251,     0,     0,     0,     0,   254,   541,   256,   257,
     258,   259,   260,     1,     2,     0,     0,     0,     3,     0,
       0,     0,     4,     0,     0,     0,     0,     9,    10,     0,
       0,     0,     0,     0,     0,   425,   426,     0,    16,    17,
      18,   549,    20,    21,    22,   500,   550,   551,   502,   503,
     504,   427,    30,   505,    32,   552,    34,     0,    35,    36,
      37,    38,   553,    40,   554,   555,    43,    44,    45,    46,
     507,     0,    48,   556,    50,   509,   510,    53,   511,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,   512,   513,    71,     0,    72,    73,    74,
     557,     0,     0,    77,    78,    79,     0,     0,   515,    81,
       0,     0,     0,     0,    83,   516,    85,   517,   518,    88,
      89,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      94,    95,    96,    97,     0,    98,     0,     0,     0,   519,
     100,   101,   520,   521,     0,     0,     0,     0,     0,     0,
     104,   105,   106,     0,   108,   109,   110,     0,     0,     0,
       0,     0,     0,   116,   522,     0,     0,     0,   118,   119,
     120,   121,   523,   123,   124,   524,     0,     0,     0,     0,
       0,     0,     0,   525,   526,   131,     0,     0,     0,   132,
       0,   133,   134,   527,     0,     0,     0,     0,   139,     0,
     140,     0,   141,   142,   143,   144,   528,   146,   147,   148,
     149,   150,   151,     0,     0,   152,   153,     0,     0,   155,
     428,   157,   158,   159,   160,   161,   162,   163,   164,   165,
     166,   167,   529,   558,   530,   171,   531,   532,   174,   175,
     176,   177,   178,   179,   533,   181,   534,   535,   536,   185,
     186,   537,   538,   189,   190,   191,   192,   193,   194,   195,
     196,   197,   198,   199,   200,   539,     0,   202,   540,   204,
       0,   205,   206,   207,   208,   209,   210,   211,   212,   213,
     214,   215,   216,   217,   218,   219,   220,   221,   222,   223,
     224,   225,   226,   227,   228,   229,   230,   231,   232,   233,
     234,   235,   236,   237,   238,   239,   240,   241,   242,   243,
     244,   245,   246,   247,   248,   249,   250,   251,     0,     0,
       3,     0,   254,   541,   256,   257,   258,   259,   260,     9,
      10,     0,     0,     0,    13,     0,     0,   425,   426,     0,
      16,    17,    18,   549,    20,    21,    22,   500,   550,   551,
     502,   503,   504,   427,    30,   505,    32,   552,    34,     0,
      35,    36,    37,    38,   553,    40,   554,   555,    43,    44,
      45,    46,   507,     0,    48,   556,    50,   509,   510,    53,
     511,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,   512,   513,    71,     0,    72,
      73,    74,   557,     0,     0,    77,    78,    79,     0,     0,
     515,    81,     0,     0,     0,     0,    83,   516,    85,   517,
     518,    88,    89,   571,    91,     0,     0,     0,     0,     0,
       0,     0,    94,    95,    96,    97,     0,    98,     0,     0,
       0,   519,   100,   101,   520,   521,     0,     0,     0,     0,
       0,     0,   104,   105,   106,     0,   108,   109,   110,     0,
       0,     0,   956,     0,     0,   116,   522,     0,     0,     0,
     118,   119,   120,   121,   523,   123,   124,   524,     0,     0,
       0,     0,     0,     0,     0,   525,   526,   131,     0,     0,
       0,   132,     0,   133,   134,   527,     0,     0,     0,     0,
       0,     0,   140,     0,   141,   142,   143,   144,   528,   146,
     147,   148,   149,   150,   151,     0,     0,   152,   153,     0,
       0,   155,   428,   157,   158,   159,   160,   161,   162,   163,
     164,   165,   166,   167,   529,   558,   530,   171,   531,   532,
     174,   175,   176,   177,   178,   179,   533,   181,   534,   535,
     536,   185,   186,   537,   538,   189,   190,   191,   192,   193,
     194,   195,   196,   197,   198,   199,   200,   539,     0,   202,
     540,   204,     0,   205,   206,   207,   208,   209,   210,   211,
     212,   213,   214,   215,   216,   217,   218,   219,   220,   221,
     222,   223,   224,   225,   226,   227,   228,   229,   230,   231,
     232,   233,   234,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,   245,   246,   247,   248,   249,   250,   251,
       0,     0,     3,     0,   254,   541,   256,   257,   258,   259,
     260,     9,    10,     0,     0,     0,     0,     0,     0,   425,
     426,     0,    16,    17,    18,   499,    20,    21,    22,   500,
      24,   501,   502,  1175,   504,   427,    30,   505,    32,    33,
      34,     0,    35,    36,    37,    38,   506,    40,    41,    42,
      43,    44,    45,    46,   507,     0,    48,   508,    50,   509,
     510,    53,   511,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,   512,   513,    71,
       0,    72,    73,    74,   514,     0,     0,    77,    78,    79,
       0,     0,   515,    81,     0,     0,     0,     0,    83,   516,
      85,   517,   518,    88,    89,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    94,    95,    96,    97,     0,    98,
       0,     0,     0,   519,   100,   101,   520,   521,     0,     0,
       0,     0,     0,     0,   104,   105,   106,     0,   108,   109,
    1176,     0,     0,     0,  1177,     0,     0,   116,   522,     0,
       0,     0,   118,   119,   120,   121,   523,   123,   124,   524,
       0,     0,     0,     0,     0,     0,     0,   525,   526,   131,
       0,     0,     0,   132,  1616,   133,   134,   527,     0,     0,
       0,     0,  1617,     0,   140,     0,   141,   142,   143,   144,
     528,   146,   147,   148,   149,   150,   151,     0,     0,  1178,
     153,     0,     0,   155,   428,   157,   158,   159,   160,   161,
     162,   163,   164,   165,   166,   167,   529,   169,   530,   171,
     531,   532,   174,   175,   176,   177,   178,   179,   533,   181,
     534,   535,   536,   185,   186,   537,   538,   189,   190,   191,
     192,   193,   194,   195,   196,   197,   198,   199,   200,   539,
       0,   202,   540,   204,     0,   205,   206,   207,   208,   209,
     210,   211,   212,   213,   214,   215,   216,   217,   218,   219,
     220,   221,   222,   223,   224,   225,   226,   227,   228,   229,
     230,   231,   232,   233,   234,   235,   236,   237,   238,   239,
     240,   241,   242,   243,   244,   245,   246,   247,   248,   249,
     250,   251,     0,     0,     0,     0,   254,   541,   256,  1179,
    1180,  1181,  1182,     3,     0,   585,   586,     0,     0,     0,
       0,     0,     9,    10,     0,     0,     0,     0,     0,     0,
     425,   426,     0,    16,    17,    18,   549,    20,    21,    22,
     500,   550,   551,   502,   503,   504,   427,    30,   505,    32,
     552,    34,     0,    35,    36,    37,    38,   553,    40,   554,
     555,    43,    44,    45,    46,   507,     0,    48,   556,    50,
     509,   510,    53,   511,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,   512,   513,
      71,     0,    72,    73,    74,   557,     0,     0,    77,    78,
      79,     0,     0,   515,    81,     0,     0,     0,     0,    83,
     516,    85,   517,   518,    88,    89,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    94,    95,    96,    97,     0,
      98,     0,     0,     0,   519,   100,   101,   520,   521,     0,
       0,     0,     0,     0,     0,   104,   105,   106,     0,   108,
     109,   110,     0,     0,     0,     0,     0,     0,   116,   522,
       0,     0,     0,   118,   119,   120,   121,   523,   123,   124,
     524,     0,     0,     0,     0,     0,     0,     0,   525,   526,
     131,     0,     0,     0,   132,     0,   133,   134,   527,     0,
       0,     0,     0,     0,     0,   140,     0,   141,   142,   143,
     144,   528,   146,   147,   148,   149,   150,   151,     0,     0,
     152,   153,     0,     0,   155,   428,   157,   158,   159,   160,
     161,   162,   163,   164,   165,   166,   167,   529,   558,   530,
     171,   531,   532,   174,   175,   176,   177,   178,   179,   533,
     181,   534,   535,   536,   185,   186,   537,   538,   189,   190,
     191,   192,   193,   194,   195,   196,   197,   198,   199,   200,
     539,     0,   202,   540,   204,     0,   205,   206,   207,   208,
     209,   210,   211,   212,   213,   214,   215,   216,   217,   218,
     219,   220,   221,   222,   223,   224,   225,   226,   227,   228,
     229,   230,   231,   232,   233,   234,   235,   236,   237,   238,
     239,   240,   241,   242,   243,   244,   245,   246,   247,   248,
     249,   250,   251,     0,     0,     3,     0,   254,   541,   256,
     257,   258,   259,   260,     9,    10,     0,     0,     0,     0,
       0,     0,   425,   426,     0,    16,    17,    18,   549,    20,
      21,    22,   500,   550,   551,   502,   503,   504,   427,    30,
     505,    32,   552,    34,     0,    35,    36,    37,    38,   553,
      40,   554,   555,    43,    44,    45,    46,   507,     0,    48,
     556,    50,   509,   510,    53,   511,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
     512,   513,    71,     0,    72,    73,    74,   557,     0,     0,
      77,    78,    79,     0,     0,   515,    81,     0,     0,     0,
       0,    83,   516,    85,   517,   518,    88,    89,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    94,    95,    96,
      97,     0,    98,     0,     0,     0,   519,   100,   101,   520,
     521,     0,     0,     0,     0,     0,     0,   104,   105,   106,
       0,   108,   109,   110,     0,     0,     0,     0,     0,     0,
     116,   522,     0,     0,     0,   118,   119,   120,   121,   523,
     123,   124,   524,     0,     0,     0,     0,     0,     0,     0,
     525,   526,   131,     0,     0,     0,   132,   779,   133,   134,
     527,     0,     0,     0,     0,   780,     0,   140,     0,   141,
     142,   143,   144,   528,   146,   147,   148,   149,   150,   151,
       0,     0,   152,   153,     0,     0,   155,   428,   157,   158,
     159,   160,   161,   162,   163,   164,   165,   166,   167,   529,
     558,   530,   171,   531,   532,   174,   175,   176,   177,   178,
     179,   533,   181,   534,   535,   536,   185,   186,   537,   538,
     189,   190,   191,   192,   193,   194,   195,   196,   197,   198,
     199,   200,   539,     0,   202,   540,   204,     0,   205,   206,
     207,   208,   209,   210,   211,   212,   213,   214,   215,   216,
     217,   218,   219,   220,   221,   222,   223,   224,   225,   226,
     227,   228,   229,   230,   231,   232,   233,   234,   235,   236,
     237,   238,   239,   240,   241,   242,   243,   244,   245,   246,
     247,   248,   249,   250,   251,     0,     0,     3,     0,   254,
     541,   256,   257,   258,   259,   260,     9,    10,     0,     0,
       0,   805,     0,     0,   425,   426,     0,    16,    17,    18,
     549,    20,    21,    22,   500,   550,   551,   502,   503,   504,
     427,    30,   505,    32,   552,    34,     0,    35,    36,    37,
      38,   553,    40,   554,   555,    43,    44,    45,    46,   507,
       0,    48,   556,    50,   509,   510,    53,   511,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,   512,   513,    71,     0,    72,    73,    74,   557,
       0,     0,    77,    78,    79,     0,     0,   515,    81,     0,
       0,     0,     0,    83,   516,    85,   517,   518,    88,    89,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    94,
      95,    96,    97,     0,    98,     0,     0,     0,   519,   100,
     101,   520,   521,     0,     0,     0,     0,     0,     0,   104,
     105,   106,     0,   108,   109,   110,     0,     0,     0,     0,
       0,     0,   116,   522,     0,     0,     0,   118,   119,   120,
     121,   523,   123,   124,   524,     0,     0,     0,     0,     0,
       0,     0,   525,   526,   131,     0,     0,     0,   132,   806,
     133,   134,   527,     0,     0,     0,     0,     0,     0,   140,
       0,   141,   142,   143,   144,   528,   146,   147,   148,   149,
     150,   151,     0,     0,   152,   153,     0,     0,   155,   428,
     157,   158,   159,   160,   161,   162,   163,   164,   165,   166,
     167,   529,   558,   530,   171,   531,   532,   174,   175,   176,
     177,   178,   179,   533,   181,   534,   535,   536,   185,   186,
     537,   538,   189,   190,   191,   192,   193,   194,   195,   196,
     197,   198,   199,   200,   539,     0,   202,   540,   204,     0,
     205,   206,   207,   208,   209,   210,   211,   212,   213,   214,
     215,   216,   217,   218,   219,   220,   221,   222,   223,   224,
     225,   226,   227,   228,   229,   230,   231,   232,   233,   234,
     235,   236,   237,   238,   239,   240,   241,   242,   243,   244,
     245,   246,   247,   248,   249,   250,   251,     0,     0,     3,
       0,   254,   541,   256,   257,   258,   259,   260,     9,    10,
       0,     0,     0,     0,     0,     0,   425,   426,     0,    16,
      17,    18,   549,    20,    21,    22,   500,   550,   551,   502,
     503,   504,   427,    30,   505,    32,   552,    34,     0,    35,
      36,    37,    38,   553,    40,   554,   555,    43,    44,    45,
      46,   507,     0,    48,   556,    50,   509,   510,    53,   511,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,   512,   513,    71,     0,    72,    73,
      74,   557,     0,     0,    77,    78,    79,     0,     0,   515,
      81,     0,     0,     0,     0,    83,   516,    85,   517,   518,
      88,    89,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    94,    95,    96,    97,     0,    98,     0,     0,     0,
     519,   100,   101,   520,   521,     0,     0,     0,     0,     0,
       0,   104,   105,   106,     0,   108,   109,   110,     0,     0,
       0,     0,     0,     0,   116,   522,     0,     0,     0,   118,
     119,   120,   121,   523,   123,   124,   524,     0,     0,     0,
       0,     0,     0,     0,   525,   526,   131,     0,     0,     0,
     132,   823,   133,   134,   527,     0,     0,     0,     0,   824,
       0,   140,     0,   141,   142,   143,   144,   528,   146,   147,
     148,   149,   150,   151,     0,     0,   152,   153,     0,     0,
     155,   428,   157,   158,   159,   160,   161,   162,   163,   164,
     165,   166,   167,   529,   558,   530,   171,   531,   532,   174,
     175,   176,   177,   178,   179,   533,   181,   534,   535,   536,
     185,   186,   537,   538,   189,   190,   191,   192,   193,   194,
     195,   196,   197,   198,   199,   200,   539,     0,   202,   540,
     204,     0,   205,   206,   207,   208,   209,   210,   211,   212,
     213,   214,   215,   216,   217,   218,   219,   220,   221,   222,
     223,   224,   225,   226,   227,   228,   229,   230,   231,   232,
     233,   234,   235,   236,   237,   238,   239,   240,   241,   242,
     243,   244,   245,   246,   247,   248,   249,   250,   251,     0,
       0,     3,     0,   254,   541,   256,   257,   258,   259,   260,
       9,    10,     0,     0,     0,     0,     0,     0,   425,   426,
       0,    16,    17,    18,   499,    20,    21,    22,   500,    24,
     501,   502,  1175,   504,   427,    30,   505,    32,    33,    34,
       0,    35,    36,    37,    38,   506,    40,    41,    42,    43,
      44,    45,    46,   507,     0,    48,   508,    50,   509,   510,
      53,   511,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,   512,   513,    71,     0,
      72,    73,    74,   514,     0,     0,    77,    78,    79,     0,
       0,   515,    81,     0,     0,     0,     0,    83,   516,    85,
     517,   518,    88,    89,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    94,    95,    96,    97,     0,    98,     0,
       0,     0,   519,   100,   101,   520,   521,     0,     0,     0,
       0,     0,     0,   104,   105,   106,     0,   108,   109,  1176,
       0,     0,     0,  1215,     0,     0,   116,   522,     0,     0,
       0,   118,   119,   120,   121,   523,   123,   124,   524,     0,
       0,     0,     0,     0,     0,     0,   525,   526,   131,     0,
       0,     0,   132,  1450,   133,   134,   527,     0,     0,     0,
       0,     0,     0,   140,     0,   141,   142,   143,   144,   528,
     146,   147,   148,   149,   150,   151,     0,     0,   152,   153,
       0,     0,   155,   428,   157,   158,   159,   160,   161,   162,
     163,   164,   165,   166,   167,   529,   169,   530,   171,   531,
     532,   174,   175,   176,   177,   178,   179,   533,   181,   534,
     535,   536,   185,   186,   537,   538,   189,   190,   191,   192,
     193,   194,   195,   196,   197,   198,   199,   200,   539,     0,
     202,   540,   204,     0,   205,   206,   207,   208,   209,   210,
     211,   212,   213,   214,   215,   216,   217,   218,   219,   220,
     221,   222,   223,   224,   225,   226,   227,   228,   229,   230,
     231,   232,   233,   234,   235,   236,   237,   238,   239,   240,
     241,   242,   243,   244,   245,   246,   247,   248,   249,   250,
     251,     0,     0,     3,     0,   254,   541,   256,  1179,  1180,
    1181,  1182,     9,    10,     0,     0,     0,     0,     0,     0,
     425,   426,     0,    16,    17,    18,   499,    20,    21,    22,
     500,    24,   501,   502,  1175,   504,   427,    30,   505,    32,
      33,    34,     0,    35,    36,    37,    38,   506,    40,    41,
      42,    43,    44,    45,    46,   507,     0,    48,   508,    50,
     509,   510,    53,   511,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,   512,   513,
      71,     0,    72,    73,    74,   514,     0,     0,    77,    78,
      79,     0,     0,   515,    81,     0,     0,     0,     0,    83,
     516,    85,   517,   518,    88,    89,  1524,     0,     0,     0,
       0,     0,     0,     0,     0,    94,    95,    96,    97,     0,
      98,     0,     0,     0,   519,   100,   101,   520,   521,     0,
       0,     0,     0,     0,     0,   104,   105,   106,     0,   108,
     109,  1176,     0,     0,     0,  1177,     0,     0,   116,   522,
       0,     0,     0,   118,   119,   120,   121,   523,   123,   124,
     524,     0,     0,     0,     0,     0,     0,     0,   525,   526,
     131,     0,     0,     0,   132,     0,   133,   134,   527,     0,
       0,     0,     0,     0,     0,   140,     0,   141,   142,   143,
     144,   528,   146,   147,   148,   149,   150,   151,     0,     0,
    1178,   153,     0,     0,   155,   428,   157,   158,   159,   160,
     161,   162,   163,   164,   165,   166,   167,   529,   169,   530,
     171,   531,   532,   174,   175,   176,   177,   178,   179,   533,
     181,   534,   535,   536,   185,   186,   537,   538,   189,   190,
     191,   192,   193,   194,   195,   196,   197,   198,   199,   200,
     539,     0,   202,   540,   204,     0,   205,   206,   207,   208,
     209,   210,   211,   212,   213,   214,   215,   216,   217,   218,
     219,   220,   221,   222,   223,   224,   225,   226,   227,   228,
     229,   230,   231,   232,   233,   234,   235,   236,   237,   238,
     239,   240,   241,   242,   243,   244,   245,   246,   247,   248,
     249,   250,   251,     0,     0,     3,     0,   254,   541,   256,
    1179,  1180,  1181,  1182,     9,    10,     0,     0,     0,     0,
       0,     0,   425,   426,     0,    16,    17,    18,   499,    20,
      21,    22,   500,    24,   501,   502,  1175,   504,   427,    30,
     505,    32,    33,    34,     0,    35,    36,    37,    38,   506,
      40,    41,    42,    43,    44,    45,    46,   507,     0,    48,
     508,    50,   509,   510,    53,   511,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
     512,   513,    71,     0,    72,    73,    74,   514,     0,     0,
      77,    78,    79,     0,     0,   515,    81,     0,     0,     0,
       0,    83,   516,    85,   517,   518,    88,    89,  1669,     0,
       0,     0,     0,     0,     0,     0,     0,    94,    95,    96,
      97,     0,    98,     0,     0,     0,   519,   100,   101,   520,
     521,     0,     0,     0,     0,     0,     0,   104,   105,   106,
       0,   108,   109,  1176,     0,     0,     0,  1177,     0,     0,
     116,   522,     0,     0,     0,   118,   119,   120,   121,   523,
     123,   124,   524,     0,     0,     0,     0,     0,     0,     0,
     525,   526,   131,     0,     0,     0,   132,     0,   133,   134,
     527,     0,     0,     0,     0,     0,     0,   140,     0,   141,
     142,   143,   144,   528,   146,   147,   148,   149,   150,   151,
       0,     0,  1178,   153,     0,     0,   155,   428,   157,   158,
     159,   160,   161,   162,   163,   164,   165,   166,   167,   529,
     169,   530,   171,   531,   532,   174,   175,   176,   177,   178,
     179,   533,   181,   534,   535,   536,   185,   186,   537,   538,
     189,   190,   191,   192,   193,   194,   195,   196,   197,   198,
     199,   200,   539,     0,   202,   540,   204,     0,   205,   206,
     207,   208,   209,   210,   211,   212,   213,   214,   215,   216,
     217,   218,   219,   220,   221,   222,   223,   224,   225,   226,
     227,   228,   229,   230,   231,   232,   233,   234,   235,   236,
     237,   238,   239,   240,   241,   242,   243,   244,   245,   246,
     247,   248,   249,   250,   251,     0,     0,     3,     0,   254,
     541,   256,  1179,  1180,  1181,  1182,     9,    10,     0,     0,
       0,     0,     0,     0,   425,   426,     0,    16,    17,    18,
     499,    20,    21,    22,   500,    24,   501,   502,  1175,   504,
     427,    30,   505,    32,    33,    34,     0,    35,    36,    37,
      38,   506,    40,    41,    42,    43,    44,    45,    46,   507,
       0,    48,   508,    50,   509,   510,    53,   511,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,   512,   513,    71,     0,    72,    73,    74,   514,
       0,     0,    77,    78,    79,     0,     0,   515,    81,     0,
       0,     0,     0,    83,   516,    85,   517,   518,    88,    89,
    1673,     0,     0,     0,     0,     0,     0,     0,     0,    94,
      95,    96,    97,     0,    98,     0,     0,     0,   519,   100,
     101,   520,   521,     0,     0,     0,     0,     0,     0,   104,
     105,   106,     0,   108,   109,  1176,     0,     0,     0,  1177,
       0,     0,   116,   522,     0,     0,     0,   118,   119,   120,
     121,   523,   123,   124,   524,     0,     0,     0,     0,     0,
       0,     0,   525,   526,   131,     0,     0,     0,   132,     0,
     133,   134,   527,     0,     0,     0,     0,     0,     0,   140,
       0,   141,   142,   143,   144,   528,   146,   147,   148,   149,
     150,   151,     0,     0,  1178,   153,     0,     0,   155,   428,
     157,   158,   159,   160,   161,   162,   163,   164,   165,   166,
     167,   529,   169,   530,   171,   531,   532,   174,   175,   176,
     177,   178,   179,   533,   181,   534,   535,   536,   185,   186,
     537,   538,   189,   190,   191,   192,   193,   194,   195,   196,
     197,   198,   199,   200,   539,     0,   202,   540,   204,     0,
     205,   206,   207,   208,   209,   210,   211,   212,   213,   214,
     215,   216,   217,   218,   219,   220,   221,   222,   223,   224,
     225,   226,   227,   228,   229,   230,   231,   232,   233,   234,
     235,   236,   237,   238,   239,   240,   241,   242,   243,   244,
     245,   246,   247,   248,   249,   250,   251,     0,     0,     3,
       0,   254,   541,   256,  1179,  1180,  1181,  1182,     9,    10,
       0,     0,     0,     0,     0,     0,   425,   426,     0,    16,
      17,    18,   499,    20,    21,    22,   500,    24,   501,   502,
    1175,   504,   427,    30,   505,    32,    33,    34,     0,    35,
      36,    37,    38,   506,    40,    41,    42,    43,    44,    45,
      46,   507,     0,    48,   508,    50,   509,   510,    53,   511,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,   512,   513,    71,     0,    72,    73,
      74,   514,     0,     0,    77,    78,    79,     0,     0,   515,
      81,     0,     0,     0,     0,    83,   516,    85,   517,   518,
      88,    89,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    94,    95,    96,    97,     0,    98,     0,     0,     0,
     519,   100,   101,   520,   521,     0,     0,     0,     0,     0,
       0,   104,   105,   106,     0,   108,   109,  1176,     0,     0,
       0,  1177,     0,     0,   116,   522,     0,     0,     0,   118,
     119,   120,   121,   523,   123,   124,   524,     0,     0,     0,
       0,     0,     0,     0,   525,   526,   131,     0,     0,     0,
     132,     0,   133,   134,   527,     0,     0,     0,     0,     0,
       0,   140,     0,   141,   142,   143,   144,   528,   146,   147,
     148,   149,   150,   151,     0,     0,  1178,   153,     0,     0,
     155,   428,   157,   158,   159,   160,   161,   162,   163,   164,
     165,   166,   167,   529,   169,   530,   171,   531,   532,   174,
     175,   176,   177,   178,   179,   533,   181,   534,   535,   536,
     185,   186,   537,   538,   189,   190,   191,   192,   193,   194,
     195,   196,   197,   198,   199,   200,   539,     0,   202,   540,
     204,     0,   205,   206,   207,   208,   209,   210,   211,   212,
     213,   214,   215,   216,   217,   218,   219,   220,   221,   222,
     223,   224,   225,   226,   227,   228,   229,   230,   231,   232,
     233,   234,   235,   236,   237,   238,   239,   240,   241,   242,
     243,   244,   245,   246,   247,   248,   249,   250,   251,     0,
       0,     3,     0,   254,   541,   256,  1179,  1180,  1181,  1182,
       9,    10,     0,     0,     0,     0,     0,     0,   425,   426,
       0,    16,    17,    18,   499,    20,    21,    22,   500,    24,
     501,   502,  1175,   504,   427,    30,   505,    32,    33,    34,
       0,    35,    36,    37,    38,   506,    40,    41,    42,    43,
      44,    45,    46,   507,     0,    48,   508,    50,   509,   510,
      53,   511,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,   512,   513,    71,     0,
      72,    73,    74,   514,     0,     0,    77,    78,    79,     0,
       0,   515,    81,     0,     0,     0,     0,    83,   516,    85,
     517,   518,    88,    89,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    94,    95,    96,    97,     0,    98,     0,
       0,     0,   519,   100,   101,   520,   521,     0,     0,     0,
       0,     0,     0,   104,   105,   106,     0,   108,   109,  1176,
       0,     0,     0,  1215,     0,     0,   116,   522,     0,     0,
       0,   118,   119,   120,   121,   523,   123,   124,   524,     0,
       0,     0,     0,     0,     0,     0,   525,   526,   131,     0,
       0,     0,   132,     0,   133,   134,   527,     0,     0,     0,
       0,     0,     0,   140,     0,   141,   142,   143,   144,   528,
     146,   147,   148,   149,   150,   151,     0,     0,   152,   153,
       0,     0,   155,   428,   157,   158,   159,   160,   161,   162,
     163,   164,   165,   166,   167,   529,   169,   530,   171,   531,
     532,   174,   175,   176,   177,   178,   179,   533,   181,   534,
     535,   536,   185,   186,   537,   538,   189,   190,   191,   192,
     193,   194,   195,   196,   197,   198,   199,   200,   539,     0,
     202,   540,   204,     0,   205,   206,   207,   208,   209,   210,
     211,   212,   213,   214,   215,   216,   217,   218,   219,   220,
     221,   222,   223,   224,   225,   226,   227,   228,   229,   230,
     231,   232,   233,   234,   235,   236,   237,   238,   239,   240,
     241,   242,   243,   244,   245,   246,   247,   248,   249,   250,
     251,     0,     0,     3,     0,   254,   541,   256,  1179,  1180,
    1181,  1182,     9,    10,     0,     0,     0,     0,     0,     0,
     425,   426,     0,    16,    17,    18,   549,    20,    21,    22,
     500,   550,   551,   502,   503,   504,   427,    30,   505,    32,
     552,    34,     0,    35,    36,    37,    38,   553,    40,   554,
     555,    43,    44,    45,    46,   507,     0,    48,   556,    50,
     509,   510,    53,   511,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,   512,   513,
      71,     0,    72,    73,    74,   557,     0,     0,    77,    78,
      79,     0,     0,   515,    81,     0,     0,     0,     0,    83,
     516,    85,   517,   518,    88,    89,  1270,     0,     0,     0,
       0,     0,     0,     0,     0,    94,    95,    96,    97,     0,
      98,     0,     0,     0,   519,   100,   101,   520,   521,     0,
       0,     0,     0,     0,     0,   104,   105,   106,     0,   108,
     109,   110,     0,     0,     0,     0,     0,     0,   116,   522,
       0,     0,     0,   118,   119,   120,   121,   523,   123,   124,
     524,     0,     0,     0,     0,     0,     0,     0,   525,   526,
     131,     0,     0,     0,   132,     0,   133,   134,   527,     0,
       0,     0,     0,     0,     0,   140,     0,   141,   142,   143,
     144,   528,   146,   147,   148,   149,   150,   151,     0,     0,
     152,   153,     0,     0,   155,   428,   157,   158,   159,   160,
     161,   162,   163,   164,   165,   166,   167,   529,   558,   530,
     171,   531,   532,   174,   175,   176,   177,   178,   179,   533,
     181,   534,   535,   536,   185,   186,   537,   538,   189,   190,
     191,   192,   193,   194,   195,   196,   197,   198,   199,   200,
     539,     0,   202,   540,   204,     0,   205,   206,   207,   208,
     209,   210,   211,   212,   213,   214,   215,   216,   217,   218,
     219,   220,   221,   222,   223,   224,   225,   226,   227,   228,
     229,   230,   231,   232,   233,   234,   235,   236,   237,   238,
     239,   240,   241,   242,   243,   244,   245,   246,   247,   248,
     249,   250,   251,     0,     0,     3,     0,   254,   541,   256,
     257,   258,   259,   260,     9,    10,     0,     0,     0,     0,
       0,     0,   425,   426,     0,    16,    17,    18,   549,    20,
      21,    22,   500,   550,   551,   502,   503,   504,   427,    30,
     505,    32,   552,    34,     0,    35,    36,    37,    38,   553,
      40,   554,   555,    43,    44,    45,    46,   507,     0,    48,
     556,    50,   509,   510,    53,   511,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
     512,   513,    71,     0,    72,    73,    74,   557,     0,     0,
      77,    78,    79,     0,     0,   515,    81,     0,     0,     0,
       0,    83,   516,    85,   517,   518,    88,    89,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    94,    95,    96,
      97,     0,    98,     0,     0,     0,   519,   100,   101,   520,
     521,     0,     0,     0,     0,     0,     0,   104,   105,   106,
       0,   108,   109,   110,     0,     0,     0,     0,     0,     0,
     116,   522,     0,     0,     0,   118,   119,   120,   121,   523,
     123,   124,   524,     0,     0,     0,     0,     0,     0,     0,
     525,   526,   131,     0,     0,     0,   132,     0,   133,   134,
     527,     0,     0,     0,     0,     0,     0,   140,     0,   141,
     142,   143,   144,   528,   146,   147,   148,   149,   150,   151,
       0,     0,   152,   153,     0,     0,   155,   428,   157,   158,
     159,   160,   161,   162,   163,   164,   165,   166,   167,   529,
     558,   530,   171,   531,   532,   174,   175,   176,   177,   178,
     179,   533,   181,   534,   535,   536,   185,   186,   537,   538,
     189,   190,   191,   192,   193,   194,   195,   196,   197,   198,
     199,   200,   539,     0,   202,   540,   204,     0,   205,   206,
     207,   208,   209,   210,   211,   212,   213,   214,   215,   216,
     217,   218,   219,   220,   221,   222,   223,   224,   225,   226,
     227,   228,   229,   230,   231,   232,   233,   234,   235,   236,
     237,   238,   239,   240,   241,   242,   243,   244,   245,   246,
     247,   248,   249,   250,   251,     0,     0,     0,     0,   254,
     541,   256,   257,   258,   259,   260,     5,     6,     7,     8,
       9,    10,     0,     0,     0,    13,     0,     0,   425,   426,
       0,    16,    17,    18,   857,    20,    21,    22,    23,     0,
     858,   502,    27,     0,   427,    30,   505,    32,     0,    34,
       0,    35,    36,    37,    38,   859,    40,     0,     0,    43,
      44,    45,    46,   507,     0,    48,   860,    50,     0,     0,
      53,   511,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,   512,   513,    71,     0,
      72,    73,    74,   861,     0,     0,    77,    78,    79,     0,
       0,   515,    81,     0,     0,     0,    82,    83,   516,    85,
     517,   518,    88,    89,   571,    91,     0,     0,     0,     0,
       0,    93,     0,    94,    95,    96,    97,     0,    98,     0,
       0,     0,   519,   100,   101,   520,   521,     0,     0,     0,
       0,     0,     0,   104,   105,   106,   107,   108,   109,     0,
     572,   112,     0,   113,   114,     0,   116,   117,     0,     0,
       0,   118,   119,   120,   121,   122,   123,   124,   524,     0,
     126,     0,     0,     0,     0,     0,   525,   526,   131,     0,
       0,     0,   132,     0,   133,   134,   527,     0,     0,     0,
       0,     0,     0,   140,     0,   141,   142,   143,   144,   145,
     146,   147,   148,   149,   150,   151,     0,     0,     0,   153,
     154,     0,   155,   428,   157,   158,   159,   160,   161,   162,
     163,   164,   165,   166,   167,   529,     0,   530,   171,   531,
     532,   174,   175,   176,   177,   178,   179,   533,   181,   534,
     535,   536,   185,   186,     0,   538,   189,   190,   191,   192,
     193,   194,   195,   196,   197,   198,   199,   200,   539,     0,
     202,   540,   204,     0,   205,   206,   207,   208,   209,   210,
     211,   212,   213,   214,   215,   216,   217,   218,   219,   220,
     221,   222,   223,   224,   225,   226,   227,   228,   229,   230,
     231,   232,   233,   234,   235,   236,   237,   238,   239,   240,
     241,   242,   243,   244,   245,   246,   247,   248,   249,   250,
     251,   252,     0,   253,     9,    10,   541,   256,     0,     0,
       0,     0,   425,   426,     0,    16,    17,    18,     0,    20,
      21,    22,   500,     0,     0,   502,   503,     0,   427,    30,
     505,    32,     0,    34,     0,    35,    36,    37,    38,     0,
      40,     0,     0,    43,    44,    45,    46,   507,     0,    48,
       0,    50,     0,     0,    53,   511,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
     512,   513,    71,     0,    72,    73,    74,     0,     0,     0,
      77,    78,    79,     0,     0,   515,    81,     0,     0,     0,
       0,    83,   516,    85,   517,   518,    88,    89,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    94,    95,    96,
      97,     0,    98,     0,     0,     0,   519,   100,   101,   520,
     521,     0,     0,     0,     0,     0,     0,   104,   105,   106,
       0,   108,   109,     0,     0,     0,     0,     0,     0,     0,
     116,   522,     0,     0,     0,   118,   119,   120,   121,   523,
     123,   124,   524,     0,     0,     0,     0,     0,     0,     0,
     525,   526,   131,     0,     0,     0,   132,     0,   133,   134,
     527,     0,     0,     0,     0,     0,     0,   140,     0,   141,
     142,   143,   144,   528,   146,   147,   148,   149,   150,   151,
       0,     0,     0,   153,     0,     0,   155,   428,   157,   158,
     159,   160,   161,   162,   163,   164,   165,   166,   167,   529,
       0,   530,   171,   531,   532,   174,   175,   176,   177,   178,
     179,   533,   181,   534,   535,   536,   185,   186,     0,   538,
     189,   190,   191,   192,   193,   194,   195,   196,   197,   198,
     199,   200,   539,     0,   202,   540,   204,     0,   205,   206,
     207,   208,   209,   210,   211,   212,   213,   214,   215,   216,
     217,   218,   219,   220,   221,   222,   223,   224,   225,   226,
     227,   228,   229,   230,   231,   232,   233,   234,   235,   236,
     237,   238,   239,   240,   241,   242,   243,   244,   245,   246,
     247,   248,   249,   250,   251,     0,     0,     0,     0,     0,
     541,   256
  };

  const short
  jsoniq_parser::yycheck_[] =
  {
       0,   321,   112,   113,   188,     5,     6,     7,     8,   707,
      14,     5,     6,     7,     8,   321,     0,   691,     5,     6,
       7,     5,     6,     7,     8,   784,  1062,   799,   300,   269,
     352,   719,   844,   438,   692,   693,   694,   695,  1078,   203,
     812,   268,   946,   947,   284,   633,   634,   598,   436,   437,
     138,   784,   603,  1311,   826,     1,   881,   933,   631,   632,
     633,   634,    76,  1329,     1,  1393,  1394,  1161,   473,   474,
    1104,   261,   262,   741,   631,   632,   975,    97,    39,   946,
     947,   119,  1025,    44,    45,    36,  1527,  1043,    30,  1530,
      56,   670,    53,   672,   673,    36,   675,     1,   677,    60,
    1342,    62,    63,    27,    28,     1,   685,   686,  1440,  1638,
      71,   111,   991,     1,     8,    30,     1,   111,   123,    95,
     144,     1,   231,    47,   991,   108,   231,   111,    30,     1,
     119,    94,     5,     6,     7,     8,     1,    98,    55,     1,
      57,  1045,   252,   253,    95,    52,    97,   133,  1052,   108,
     108,    75,     1,   138,    31,   108,    97,   149,  1025,   431,
     220,   170,   108,   435,   162,    25,    27,    28,   183,    95,
       1,   108,     0,     1,    34,   291,    37,    38,  1045,    40,
      41,    45,    95,   198,   101,  1052,    47,    27,    28,   113,
     300,   988,   124,   231,   107,   204,    57,    58,   149,    63,
     260,    61,    33,  1000,   108,    97,   192,    47,   149,    27,
      28,   320,   114,   155,    75,   320,   158,    57,   203,    80,
     108,   184,   149,   149,   131,   997,   185,     1,     1,    37,
      38,   189,    40,    41,   239,    75,   178,     1,   111,   192,
      80,   108,   262,  1571,     1,   933,    97,   162,   224,   225,
      58,   112,   113,   114,   220,   279,   239,   577,  1294,  1788,
     162,   261,   262,   239,    97,   265,   158,   355,  1308,     1,
     231,   577,   112,   113,   114,   313,     1,   261,   262,  1051,
     108,   265,   176,  1541,   188,  1161,  1162,   279,  1100,     1,
     279,  1050,   564,     1,   260,   113,   192,   185,  1177,  1333,
     300,   162,  1544,   891,   892,  1637,   114,   192,  1342,   977,
    1177,  1752,   192,  1212,  1270,  1048,   300,  1050,  1261,   170,
     192,   431,  1416,   265,   188,   435,     1,   192,   279,     1,
     192,  1064,   889,   890,   108,   108,  1215,   170,   279,   216,
     257,   457,     1,   192,   108,     1,   360,   752,  1215,   363,
     929,   108,   279,   270,   162,   272,   466,   185,  1799,     1,
    1801,   471,   323,   324,   325,   226,   283,   284,   285,   293,
     287,   481,   289,   319,   241,   158,   108,   301,   488,   489,
     317,  1153,  1286,   108,   656,   108,   108,   497,   628,   629,
       0,     1,   797,    42,  1261,    95,   108,    46,   638,     1,
     108,   155,    51,    97,   631,   632,   267,   258,   259,  1737,
     261,     1,     1,   158,   188,   188,   119,   268,   226,  1286,
     114,    70,    33,     1,   188,   258,   259,   278,   261,   186,
     620,   431,   293,   108,    77,   435,   108,   108,    49,   290,
     301,   108,   231,  1310,   233,   278,   119,   431,   309,   108,
      97,   435,   108,   293,   564,     1,   188,   290,     3,   267,
     185,   301,   462,  1791,   464,   188,   108,   189,   462,   309,
     464,   581,   582,  1161,  1162,   108,   188,   185,   462,   108,
     464,    30,   592,     1,   187,     1,   486,   190,  1260,     1,
       1,   491,   486,     1,   231,    76,   233,   491,   108,   635,
     610,   611,   486,   639,    97,   669,   108,   491,  1774,    90,
    1544,   186,   155,  1272,   155,   188,   188,   188,   108,   108,
     169,   188,    67,   172,   224,   225,  1214,    72,   108,   188,
     108,   320,   188,   114,  1699,     9,    10,   178,   187,   239,
    1416,   190,   121,   158,  1448,   126,   188,   128,  1281,  1425,
    1375,  1139,  1140,  1141,  1379,   188,   149,   101,  1256,   188,
    1436,  1819,   108,   101,   564,   214,  1139,  1140,  1141,   119,
       9,    10,   572,  1124,   119,   185,   114,   156,   572,   158,
     564,  1448,   970,   320,   227,   228,   188,   160,   572,   462,
     108,   464,   108,   998,   999,   185,   108,   108,  1003,   188,
     108,   132,   155,   713,   177,   158,   155,   879,   188,   158,
     720,   611,  1777,   486,   263,   262,   263,   611,   491,  1655,
     620,   155,   626,   711,   712,  1397,    30,   611,  1033,   178,
     104,   631,   632,   633,   634,   907,   620,     5,   108,   185,
       5,     9,   119,   187,     9,   613,   190,   631,   632,   633,
     634,   125,    95,   127,   108,   895,   656,     9,    10,   899,
    1527,   118,   889,  1530,   243,   104,   108,   185,   199,   185,
     108,   108,   656,   185,   185,   231,   132,   185,   212,   158,
     236,   155,    45,     9,    47,   159,   125,    45,   127,    47,
       1,   148,     3,    95,    97,    54,    95,   155,   666,   572,
      59,   175,   275,    29,   161,   158,   149,   111,   708,   282,
     187,   114,     9,   190,   708,    95,   155,    80,   188,   292,
     159,   708,   126,   197,   708,   835,   836,   837,  1416,   839,
    1542,  1607,    29,  1773,   188,  1547,   175,  1425,    97,   196,
    1499,   215,   144,   199,   107,   144,   188,   151,  1436,   107,
     188,   188,   104,  1427,   212,   158,    67,   125,   197,   127,
     125,    72,   127,  1667,   144,    97,   138,   135,   160,   879,
     135,  1764,  1765,   125,   155,   127,   215,   167,   124,   360,
     152,     1,   363,     3,   158,   177,  1484,   155,    95,   789,
     155,    54,   158,   183,  1382,   789,    59,   178,   158,   125,
    1667,   127,   789,   155,   158,   789,   169,   159,   119,   146,
     136,   169,     1,  1806,   121,    78,  1720,   149,  1722,  1812,
     820,   821,  1039,   175,  1041,   101,   820,   821,   125,   155,
     127,    97,   107,    95,    97,   708,   820,   821,   114,   136,
     177,  1163,     9,    10,    33,   197,   956,    67,   114,   156,
     150,   158,    72,  1720,   129,  1722,   119,  1129,   155,   196,
      49,   442,   443,   215,   158,   446,   166,  1679,  1085,   450,
    1087,  1683,   453,   454,   160,   121,   101,   139,   183,   879,
     461,  1785,   144,   124,  1289,  1752,   467,   468,   160,   114,
    1130,   177,   158,   198,   475,   879,  1136,   901,   150,   119,
     101,   482,   483,   484,   485,   177,   139,   907,   101,   490,
     156,   119,   158,   114,   166,   201,   789,   498,  1785,  1607,
     124,   114,   155,   907,  1598,  1599,   155,   134,   187,   201,
     101,   190,  1799,  1591,  1801,   142,   143,   104,   145,   253,
     254,   155,  1640,   114,   158,   124,   153,   820,   821,  1133,
     157,   155,   159,   212,   158,   139,  1768,   164,   125,   275,
     127,    96,   276,   269,   545,  1737,   282,   147,   203,   124,
     269,   155,   107,   180,   280,   119,   292,   212,   158,   560,
     305,   280,   117,   714,  1389,   124,   717,  1307,   155,   314,
     571,   722,   159,  1398,   129,   193,   194,   274,   205,   206,
     207,   208,   209,   210,   281,   283,   284,   285,   175,   139,
     139,   289,   124,    54,   144,   144,  1288,   598,    59,   130,
     131,   155,   603,   155,   158,   606,   158,   238,   187,   240,
     197,   190,   613,   155,  1708,  1709,   359,   130,   131,   362,
     227,   228,   229,   238,  1742,   240,  1366,   124,   215,   124,
    1160,   689,   690,   124,  1374,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    90,   696,   697,   124,  1374,   155,
     698,   699,  1254,  1255,  1628,  1629,   219,   158,   247,   119,
    1080,   247,   158,   248,   320,   666,  1080,   155,     0,   192,
     168,  1789,    93,   255,   100,    50,  1080,   202,   102,   195,
     103,   156,   158,  1508,    30,   185,   185,  1781,    47,   154,
     119,   185,   185,   310,   252,     1,   188,   155,   108,   119,
     144,   188,   188,   155,   155,   158,   144,   185,   158,  1129,
     158,   158,   123,   158,  1138,   123,   158,   158,   719,  1139,
    1140,  1141,   158,   155,    35,  1129,    17,   216,    30,   237,
     192,   251,     3,   192,   119,  1139,  1140,  1141,   169,   290,
     119,   171,   290,  1861,   745,  1863,   171,   748,    71,   188,
      95,    95,   237,    95,   151,  1873,   108,   108,    30,   162,
     162,   762,   129,   134,    95,   766,   767,  1371,   252,   305,
     144,   119,   134,   185,   188,   188,  1516,    95,  1518,   185,
     108,  1521,  1522,    97,   785,  1525,    45,  1080,    97,   188,
     188,  1531,   793,   188,   188,   796,   185,   155,   185,   800,
     155,   155,   155,    95,   144,   155,   155,    33,   243,   192,
     171,   239,   185,   134,   144,    95,    30,   233,   292,    95,
     155,   129,   305,   119,   149,   119,    95,   134,  1520,   185,
     200,   134,    97,   162,   119,   155,   149,   101,   101,   155,
     144,   123,   843,    35,  1264,   192,   185,   185,   123,    30,
    1264,   119,    80,   108,   155,   251,   107,  1264,   108,   260,
    1264,   266,    30,   158,   291,   158,   273,   146,  1288,   158,
     158,   264,  1564,   292,   158,   158,    30,   158,    30,  1299,
     881,   129,   192,   188,  1288,  1299,   146,   149,   188,   293,
     312,   188,  1299,   149,   101,  1299,   233,   108,  1318,   144,
     134,   188,   188,     7,  1318,   239,   239,   185,    97,   158,
     911,  1318,   239,   239,  1318,   188,   158,   158,   192,  1339,
     921,   243,   155,   200,   107,  1339,  1346,   149,   119,   119,
    1670,    78,  1346,   149,  1674,  1339,   188,   256,    79,   256,
      30,   188,  1346,    30,   188,   946,   947,   948,   949,   292,
     188,   188,   288,   188,   171,   108,   188,   188,  1378,   128,
     185,  1653,   277,   149,  1378,   263,   128,   149,    97,   144,
     185,  1264,  1664,  1665,  1378,   185,  1668,   185,   155,   185,
     185,   322,    95,  1675,   188,    30,    97,   286,   188,   149,
     991,   171,    95,    54,    95,    30,   199,   119,   108,   108,
      95,   271,   231,   293,  1005,   199,  1299,    95,   231,    94,
     184,   199,   119,   200,   119,   199,   201,   119,  1758,   119,
     119,   751,   231,   457,  1216,  1318,   293,  1053,   437,  1030,
     992,  1561,  1562,  1563,  1261,   265,  1742,  1789,  1039,   882,
    1041,   299,  1043,  1158,  1045,   311,  1339,   265,   268,   802,
     311,  1052,  1584,  1346,   311,  1408,  1145,  1400,  1798,   312,
    1396,  1393,  1802,  1300,  1305,  1308,   312,  1313,   688,  1761,
     560,   668,   312,   719,   949,   719,  1333,   725,  1812,   844,
    1041,  1806,  1365,   465,  1085,   465,  1087,  1376,  1436,  1375,
    1607,  1425,  1162,   347,  1834,  1604,  1171,  1837,  1429,  1024,
    1520,  1238,  1474,  1631,    -1,  1426,    -1,    -1,  1800,    90,
      -1,    -1,    -1,    -1,    -1,    -1,  1520,    -1,    -1,   601,
    1860,    -1,    -1,  1124,    -1,    -1,  1818,    -1,    -1,    -1,
     111,    -1,    -1,   114,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1833,    -1,    -1,  1564,   126,    -1,   128,  1149,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1564,    -1,    -1,  1855,    -1,    -1,  1696,    -1,    -1,    -1,
      -1,    -1,  1864,    -1,    -1,    -1,  1177,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1718,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1215,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1653,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1664,  1665,    -1,    -1,  1668,  1653,
      -1,    -1,    -1,    -1,    -1,  1675,    -1,    -1,    -1,    -1,
    1664,  1665,    -1,    -1,  1668,    -1,    -1,    -1,    -1,  1270,
      -1,  1675,    -1,  1274,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1286,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1300,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1310,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1340,
      -1,  1761,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1761,    -1,    -1,
      -1,    -1,    -1,    -1,  1365,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1375,    -1,    -1,    -1,  1379,    -1,
    1800,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1800,    -1,  1818,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1407,    -1,    -1,    -1,
      -1,    -1,    -1,  1833,  1818,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1833,
      -1,    -1,    -1,    -1,    -1,  1855,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1864,    -1,    -1,  1448,    -1,    -1,
      -1,  1855,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1864,    -1,   443,    -1,    -1,   446,    -1,    -1,    -1,   450,
      -1,    -1,   453,   454,    -1,    -1,    -1,    -1,    -1,  1480,
     461,    -1,    -1,    -1,    -1,    -1,   467,   468,    -1,    -1,
      -1,    -1,    -1,    -1,   475,    -1,    -1,    -1,    -1,    -1,
      -1,   482,   483,   484,   485,    -1,    -1,    -1,    -1,   490,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   498,    -1,    -1,
      -1,    -1,    -1,  1524,    -1,    -1,  1527,    -1,    -1,  1530,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1543,    -1,    -1,    -1,    -1,  1548,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   545,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   560,
    1581,  1582,    -1,    -1,    -1,    -1,    -1,   111,    -1,    -1,
     571,   572,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     601,    -1,    -1,    -1,    -1,   606,    -1,    -1,    -1,    -1,
      -1,    -1,   613,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1667,    -1,  1669,    -1,
    1671,    -1,  1673,    -1,    -1,    -1,    -1,    -1,    -1,  1680,
      -1,    -1,    -1,    -1,    -1,   666,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1720,
      -1,  1722,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1738,   719,    -1,
    1741,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1752,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1763,    -1,    -1,    -1,    -1,   748,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   762,    -1,    -1,  1785,   766,   767,    -1,    -1,    -1,
      -1,    -1,  1793,    -1,    -1,    -1,    -1,    -1,  1799,    -1,
    1801,    -1,    -1,    -1,   785,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   793,    -1,    -1,   796,    -1,    -1,    -1,   800,
      -1,  1822,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1831,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1851,   832,    -1,    -1,    -1,  1856,    -1,    -1,    -1,    -1,
      -1,    -1,   843,    -1,    -1,    -1,    -1,  1868,    -1,    -1,
    1871,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     911,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     921,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   946,   947,   948,   949,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     991,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1005,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   572,  1030,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1039,    -1,
    1041,    -1,  1043,    -1,  1045,    -1,    -1,    -1,    -1,    -1,
      -1,  1052,   596,   597,    -1,   599,   600,   601,   602,    -1,
     604,    -1,    -1,    -1,    -1,   609,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   619,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1085,    -1,  1087,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   641,   642,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   652,    -1,
      -1,    -1,   656,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1149,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1177,   721,    -1,    -1,
      -1,   725,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1215,    -1,    -1,    -1,    -1,    -1,
     764,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   801,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1270,
      -1,    -1,    -1,  1274,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   827,    -1,  1286,    -1,   831,   832,    -1,
     834,    -1,    -1,    -1,    -1,    -1,    -1,   841,    -1,  1300,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1310,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1340,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   907,  1365,    -1,    -1,    -1,    -1,   913,
     914,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1407,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1448,    -1,   993,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1480,
      -1,    -1,    -1,    -1,  1028,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1047,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1063,
      -1,    -1,    -1,  1524,    -1,    -1,  1527,    -1,    -1,  1530,
    1074,  1075,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1543,    -1,    -1,  1089,    -1,  1548,  1092,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1111,  1112,  1113,
      -1,  1115,  1116,  1117,  1118,  1119,  1120,  1121,  1122,  1123,
    1581,  1582,  1126,  1127,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1135,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1145,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1202,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1667,    -1,  1669,    -1,
    1671,    -1,  1673,    -1,    -1,    -1,    -1,    -1,    -1,  1680,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1720,
      -1,  1722,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1276,    -1,  1278,    -1,  1280,  1738,    -1,    -1,
    1741,    -1,    -1,    -1,  1288,    -1,    -1,    -1,    -1,    -1,
      -1,  1752,    -1,  1297,    -1,    -1,    -1,    -1,    -1,  1303,
    1304,    -1,  1763,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1322,    -1,
      -1,  1325,  1326,    -1,  1785,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1793,    -1,    -1,    -1,    -1,    -1,  1799,    -1,
    1801,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1822,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1831,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1851,    -1,  1396,    -1,    -1,  1856,  1400,  1401,    -1,    -1,
      -1,    -1,  1406,    -1,    -1,    -1,    -1,  1868,    -1,    -1,
    1871,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1493,
      -1,    -1,    -1,  1497,    -1,    -1,    -1,  1501,    -1,  1503,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1511,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1519,  1520,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1540,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1552,  1553,
    1554,  1555,    -1,  1557,  1558,    -1,  1560,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1575,    -1,    -1,    -1,    -1,  1580,    -1,    -1,    -1,
      -1,    -1,  1586,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1646,    -1,  1648,    -1,    -1,  1651,    -1,    -1,
    1654,    -1,    -1,    -1,    -1,  1659,  1660,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1668,    -1,    -1,    -1,  1672,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1689,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1700,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1745,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1757,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1800,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     0,
       1,    -1,    -1,    -1,    -1,  1829,    -1,    -1,    -1,  1833,
      11,    12,    -1,    -1,  1838,    16,    -1,    -1,    -1,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    -1,    30,
    1854,    -1,    33,    34,  1858,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    -1,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    -1,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    -1,    95,    96,    97,    98,    99,    -1,
     101,   102,   103,    -1,    -1,   106,   107,    -1,    -1,    -1,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
      -1,   122,    -1,    -1,    -1,   126,    -1,   128,   129,   130,
     131,    -1,   133,    -1,    -1,    -1,   137,   138,   139,   140,
     141,    -1,    -1,    -1,    -1,    -1,    -1,   148,   149,   150,
     151,   152,   153,   154,   155,   156,    -1,   158,   159,   160,
     161,   162,    -1,    -1,    -1,   166,   167,   168,   169,   170,
     171,   172,   173,    -1,   175,    -1,   177,   178,    -1,    -1,
     181,   182,   183,    -1,    -1,    -1,   187,    -1,   189,   190,
     191,   192,   193,   194,    -1,   196,    -1,   198,    -1,   200,
     201,   202,   203,   204,   205,   206,   207,   208,   209,   210,
      -1,    -1,   213,   214,   215,    -1,   217,   218,   219,   220,
     221,   222,   223,   224,   225,   226,   227,   228,   229,   230,
     231,   232,   233,   234,   235,   236,   237,   238,   239,   240,
     241,   242,   243,   244,   245,   246,   247,   248,   249,   250,
     251,   252,   253,   254,   255,   256,   257,   258,   259,   260,
     261,   262,   263,    -1,   265,   266,   267,    -1,   269,   270,
     271,   272,   273,   274,   275,   276,   277,   278,   279,   280,
     281,   282,   283,   284,   285,   286,   287,   288,   289,   290,
     291,   292,   293,   294,   295,   296,   297,   298,   299,   300,
     301,   302,   303,   304,   305,   306,   307,   308,   309,   310,
     311,   312,   313,   314,   315,   316,    -1,   318,    -1,   320,
     321,   322,   323,   324,   325,   326,     0,     1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    11,    12,    -1,
      -1,    -1,    16,    -1,    -1,    -1,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    -1,    30,    -1,    -1,    33,
      34,    -1,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    -1,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    -1,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      -1,    95,    96,    97,    98,    99,    -1,   101,   102,   103,
      -1,    -1,   106,   107,    -1,    -1,    -1,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,    -1,   122,    -1,
      -1,    -1,   126,    -1,   128,   129,   130,   131,    -1,   133,
      -1,    -1,    -1,   137,   138,   139,   140,   141,    -1,    -1,
      -1,    -1,    -1,    -1,   148,   149,   150,   151,   152,   153,
     154,   155,   156,    -1,   158,   159,   160,   161,   162,    -1,
      -1,    -1,   166,   167,   168,   169,   170,   171,   172,   173,
      -1,   175,    -1,   177,   178,    -1,    -1,   181,   182,   183,
      -1,    -1,    -1,   187,    -1,   189,   190,   191,   192,   193,
     194,    -1,   196,    -1,   198,    -1,   200,   201,   202,   203,
     204,   205,   206,   207,   208,   209,   210,    -1,    -1,   213,
     214,   215,    -1,   217,   218,   219,   220,   221,   222,   223,
     224,   225,   226,   227,   228,   229,   230,   231,   232,   233,
     234,   235,   236,   237,   238,   239,   240,   241,   242,   243,
     244,   245,   246,   247,   248,   249,   250,   251,   252,   253,
     254,   255,   256,   257,   258,   259,   260,   261,   262,   263,
      -1,   265,   266,   267,    -1,   269,   270,   271,   272,   273,
     274,   275,   276,   277,   278,   279,   280,   281,   282,   283,
     284,   285,   286,   287,   288,   289,   290,   291,   292,   293,
     294,   295,   296,   297,   298,   299,   300,   301,   302,   303,
     304,   305,   306,   307,   308,   309,   310,   311,   312,   313,
     314,   315,   316,     1,   318,    -1,   320,   321,   322,   323,
     324,   325,   326,    11,    12,    -1,    -1,    -1,    16,    -1,
      -1,    -1,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    -1,    30,    -1,    -1,    33,    34,    -1,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    -1,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    -1,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    -1,    95,    96,    97,
      98,    99,    -1,   101,   102,   103,    -1,    -1,   106,   107,
      -1,    -1,    -1,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,    -1,   122,    -1,    -1,    -1,   126,    -1,
     128,   129,   130,   131,    -1,   133,    -1,    -1,    -1,   137,
     138,   139,   140,   141,    -1,    -1,    -1,    -1,    -1,    -1,
     148,   149,   150,   151,   152,   153,   154,   155,   156,    -1,
     158,   159,   160,   161,   162,    -1,    -1,    -1,   166,   167,
     168,   169,   170,   171,   172,   173,    -1,   175,    -1,   177,
     178,    -1,    -1,   181,   182,   183,    -1,    -1,    -1,   187,
      -1,   189,   190,   191,   192,   193,   194,    -1,   196,    -1,
     198,    -1,   200,   201,   202,   203,   204,   205,   206,   207,
     208,   209,   210,    -1,    -1,   213,   214,   215,    -1,   217,
     218,   219,   220,   221,   222,   223,   224,   225,   226,   227,
     228,   229,   230,   231,   232,   233,   234,   235,   236,   237,
     238,   239,   240,   241,   242,   243,   244,   245,   246,   247,
     248,   249,   250,   251,   252,   253,   254,   255,   256,   257,
     258,   259,   260,   261,   262,   263,    -1,   265,   266,   267,
      -1,   269,   270,   271,   272,   273,   274,   275,   276,   277,
     278,   279,   280,   281,   282,   283,   284,   285,   286,   287,
     288,   289,   290,   291,   292,   293,   294,   295,   296,   297,
     298,   299,   300,   301,   302,   303,   304,   305,   306,   307,
     308,   309,   310,   311,   312,   313,   314,   315,   316,     1,
     318,    -1,   320,   321,   322,   323,   324,   325,   326,    11,
      12,    -1,    -1,    -1,    16,    -1,    -1,    -1,    20,    21,
      22,    23,    24,    25,    26,    -1,    -1,    -1,    30,    -1,
      -1,    33,    34,    -1,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    -1,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    -1,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    -1,    95,    96,    97,    98,    99,    -1,   101,
     102,   103,    -1,    -1,   106,   107,    -1,    -1,    -1,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,    -1,
     122,    -1,    -1,    -1,   126,    -1,   128,   129,   130,   131,
      -1,   133,    -1,    -1,    -1,   137,   138,   139,   140,   141,
      -1,    -1,    -1,    -1,    -1,    -1,   148,   149,   150,   151,
     152,   153,   154,   155,   156,    -1,   158,   159,   160,   161,
     162,    -1,    -1,    -1,   166,   167,   168,   169,   170,   171,
     172,   173,    -1,   175,    -1,   177,   178,    -1,    -1,   181,
     182,   183,    -1,    -1,    -1,   187,    -1,   189,   190,   191,
      -1,   193,   194,    -1,   196,    -1,   198,    -1,   200,   201,
     202,   203,   204,   205,   206,   207,   208,   209,   210,    -1,
      -1,   213,   214,   215,    -1,   217,   218,   219,   220,   221,
     222,   223,   224,   225,   226,   227,   228,   229,   230,   231,
     232,   233,   234,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,   245,   246,   247,   248,   249,   250,   251,
     252,   253,   254,   255,   256,   257,   258,   259,   260,   261,
     262,   263,    -1,   265,   266,   267,    -1,   269,   270,   271,
     272,   273,   274,   275,   276,   277,   278,   279,   280,   281,
     282,   283,   284,   285,   286,   287,   288,   289,   290,   291,
     292,   293,   294,   295,   296,   297,   298,   299,   300,   301,
     302,   303,   304,   305,   306,   307,   308,   309,   310,   311,
     312,   313,   314,   315,   316,    -1,   318,    -1,   320,   321,
     322,   323,   324,   325,   326,     0,     1,    -1,     3,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    42,    -1,    -1,
      -1,    46,    -1,    -1,    -1,    50,    51,    -1,    -1,    -1,
      -1,    56,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    67,    -1,    -1,    70,    -1,    72,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    93,    -1,
      95,    96,    97,    -1,    -1,   100,   101,   102,   103,    -1,
      -1,    -1,   107,   108,    -1,    -1,    -1,    -1,    -1,   114,
      -1,    -1,   117,   118,   119,    -1,    -1,    -1,   123,    -1,
      -1,    -1,    -1,   128,   129,    -1,    -1,    -1,    -1,   134,
      -1,    -1,    -1,   138,    -1,    -1,    -1,   142,   143,    -1,
     145,    -1,   147,   148,    -1,    -1,    -1,   152,   153,    -1,
      -1,   156,   157,   158,   159,   160,   161,    -1,    -1,   164,
      -1,    -1,    -1,   168,   169,    -1,    -1,   172,    -1,    -1,
      -1,    -1,   177,    -1,    -1,   180,    -1,    -1,    -1,    -1,
     185,   186,   187,   188,   189,   190,    -1,   192,   193,   194,
     195,   196,    -1,    -1,    -1,    -1,   201,   202,   203,    -1,
     205,   206,   207,   208,   209,   210,    -1,   212,    -1,   214,
      -1,    -1,    -1,    -1,    -1,   220,    -1,    -1,    -1,   224,
     225,    -1,     0,     1,    -1,     3,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   239,    -1,   241,    -1,   243,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     255,    -1,    -1,    -1,    -1,   260,    -1,    -1,   263,    -1,
      -1,    -1,    -1,    -1,    42,    -1,    -1,    -1,    46,    -1,
      -1,    -1,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,
      -1,    -1,    70,    -1,    72,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   317,    -1,   319,    93,    -1,    95,    96,    97,
      -1,    -1,   100,   101,   102,   103,    -1,    -1,    -1,   107,
     108,    -1,    -1,    -1,    -1,    -1,   114,    -1,    -1,   117,
     118,   119,    -1,    -1,    -1,   123,    -1,    -1,    -1,    -1,
     128,   129,    -1,    -1,    -1,    -1,   134,    -1,    -1,    -1,
     138,    -1,    -1,    -1,   142,   143,    -1,   145,    -1,   147,
     148,    -1,    -1,    -1,   152,   153,    -1,    -1,   156,   157,
     158,   159,   160,   161,    -1,    -1,   164,    -1,    -1,    -1,
     168,   169,    -1,    -1,   172,    -1,    -1,    -1,    -1,   177,
      -1,    -1,   180,    -1,    -1,    -1,    -1,   185,   186,   187,
     188,   189,   190,    -1,   192,   193,   194,   195,   196,    -1,
      -1,    -1,    -1,   201,   202,   203,    -1,   205,   206,   207,
     208,   209,   210,    -1,   212,    -1,   214,    -1,    -1,    -1,
      -1,    -1,   220,    -1,    -1,    -1,   224,   225,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   239,    -1,   241,    -1,   243,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   255,    -1,    -1,
      -1,    -1,   260,    -1,    -1,   263,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    11,    12,    -1,    -1,    -1,    16,    -1,    -1,    -1,
      20,    21,    22,    23,    24,    25,    26,    27,    28,   317,
      30,   319,    -1,    33,    34,    -1,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    -1,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    -1,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    -1,    95,    96,    97,    98,    99,
      -1,   101,   102,   103,    -1,    -1,   106,   107,    -1,    -1,
      -1,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,    -1,   122,    -1,    -1,    -1,   126,    -1,   128,   129,
     130,   131,    -1,   133,    -1,    -1,    -1,   137,   138,   139,
     140,   141,    -1,    -1,    -1,    -1,    -1,    -1,   148,   149,
     150,   151,   152,   153,   154,   155,   156,    -1,   158,   159,
     160,   161,   162,    -1,    -1,    -1,   166,   167,   168,   169,
     170,   171,   172,   173,    -1,   175,    -1,   177,   178,    -1,
      -1,   181,   182,   183,    -1,    -1,    -1,   187,    -1,   189,
     190,   191,   192,   193,   194,    -1,   196,    -1,   198,    -1,
     200,   201,   202,   203,   204,   205,   206,   207,   208,   209,
     210,    -1,    -1,   213,   214,   215,    -1,   217,   218,   219,
     220,   221,   222,   223,   224,   225,   226,   227,   228,   229,
     230,   231,   232,   233,   234,   235,   236,   237,   238,   239,
     240,   241,   242,   243,   244,   245,   246,   247,   248,   249,
     250,   251,   252,   253,   254,   255,   256,   257,   258,   259,
     260,   261,   262,   263,    -1,   265,   266,   267,    -1,   269,
     270,   271,   272,   273,   274,   275,   276,   277,   278,   279,
     280,   281,   282,   283,   284,   285,   286,   287,   288,   289,
     290,   291,   292,   293,   294,   295,   296,   297,   298,   299,
     300,   301,   302,   303,   304,   305,   306,   307,   308,   309,
     310,   311,   312,   313,   314,   315,   316,    -1,   318,    -1,
     320,   321,   322,   323,   324,   325,   326,   327,   328,    11,
      12,    -1,    -1,    -1,    16,    -1,    -1,    -1,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    -1,    30,    -1,
      -1,    33,    34,    -1,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    -1,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    -1,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    -1,    95,    96,    97,    98,    99,    -1,   101,
     102,   103,    -1,    -1,   106,   107,    -1,    -1,    -1,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,    -1,
     122,    -1,    -1,    -1,   126,    -1,   128,   129,   130,   131,
      -1,   133,    -1,    -1,    -1,   137,   138,   139,   140,   141,
      -1,    -1,    -1,    -1,    -1,    -1,   148,   149,   150,   151,
     152,   153,   154,   155,   156,    -1,   158,   159,   160,   161,
     162,    -1,    -1,    -1,   166,   167,   168,   169,   170,   171,
     172,   173,    -1,   175,    -1,   177,   178,    -1,    -1,   181,
     182,   183,    -1,    -1,    -1,   187,    -1,   189,   190,   191,
     192,   193,   194,    -1,   196,    -1,   198,    -1,   200,   201,
     202,   203,   204,   205,   206,   207,   208,   209,   210,    -1,
      -1,   213,   214,   215,    -1,   217,   218,   219,   220,   221,
     222,   223,   224,   225,   226,   227,   228,   229,   230,   231,
     232,   233,   234,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,   245,   246,   247,   248,   249,   250,   251,
     252,   253,   254,   255,   256,   257,   258,   259,   260,   261,
     262,   263,    -1,   265,   266,   267,    -1,   269,   270,   271,
     272,   273,   274,   275,   276,   277,   278,   279,   280,   281,
     282,   283,   284,   285,   286,   287,   288,   289,   290,   291,
     292,   293,   294,   295,   296,   297,   298,   299,   300,   301,
     302,   303,   304,   305,   306,   307,   308,   309,   310,   311,
     312,   313,   314,   315,   316,    -1,   318,    -1,   320,   321,
     322,   323,   324,   325,   326,    -1,   328,    11,    12,    -1,
      -1,    -1,    16,    -1,    -1,    -1,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    -1,    30,    -1,    -1,    33,
      34,    -1,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    -1,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    -1,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      -1,    95,    96,    97,    98,    99,    -1,   101,   102,   103,
      -1,    -1,   106,   107,    -1,    -1,    -1,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,    -1,   122,    -1,
      -1,    -1,   126,    -1,   128,   129,   130,   131,    -1,   133,
      -1,    -1,    -1,   137,   138,   139,   140,   141,    -1,    -1,
      -1,    -1,    -1,    -1,   148,   149,   150,   151,   152,   153,
     154,   155,   156,    -1,   158,   159,   160,   161,   162,    -1,
      -1,    -1,   166,   167,   168,   169,   170,   171,   172,   173,
      -1,   175,    -1,   177,   178,    -1,    -1,   181,   182,   183,
      -1,   185,    -1,   187,    -1,   189,   190,   191,   192,   193,
     194,    -1,   196,    -1,   198,    -1,   200,   201,   202,   203,
     204,   205,   206,   207,   208,   209,   210,    -1,    -1,   213,
     214,   215,    -1,   217,   218,   219,   220,   221,   222,   223,
     224,   225,   226,   227,   228,   229,   230,   231,   232,   233,
     234,   235,   236,   237,   238,   239,   240,   241,   242,   243,
     244,   245,   246,   247,   248,   249,   250,   251,   252,   253,
     254,   255,   256,   257,   258,   259,   260,   261,   262,   263,
      -1,   265,   266,   267,    -1,   269,   270,   271,   272,   273,
     274,   275,   276,   277,   278,   279,   280,   281,   282,   283,
     284,   285,   286,   287,   288,   289,   290,   291,   292,   293,
     294,   295,   296,   297,   298,   299,   300,   301,   302,   303,
     304,   305,   306,   307,   308,   309,   310,   311,   312,   313,
     314,   315,   316,    -1,   318,    -1,   320,   321,   322,   323,
     324,   325,   326,    11,    12,    -1,    -1,    -1,    16,    -1,
      -1,    -1,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    -1,    30,    -1,    -1,    33,    34,    -1,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    -1,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    -1,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    -1,    95,    96,    97,
      98,    99,    -1,   101,   102,   103,    -1,    -1,   106,   107,
      -1,    -1,    -1,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,    -1,   122,    -1,    -1,    -1,   126,    -1,
     128,   129,   130,   131,    -1,   133,    -1,    -1,    -1,   137,
     138,   139,   140,   141,    -1,    -1,    -1,    -1,    -1,    -1,
     148,   149,   150,   151,   152,   153,   154,   155,   156,    -1,
     158,   159,   160,   161,   162,    -1,    -1,    -1,   166,   167,
     168,   169,   170,   171,   172,   173,    -1,   175,    -1,   177,
     178,    -1,    -1,   181,   182,   183,    -1,    -1,    -1,   187,
      -1,   189,   190,   191,   192,   193,   194,    -1,   196,    -1,
     198,    -1,   200,   201,   202,   203,   204,   205,   206,   207,
     208,   209,   210,    -1,    -1,   213,   214,   215,    -1,   217,
     218,   219,   220,   221,   222,   223,   224,   225,   226,   227,
     228,   229,   230,   231,   232,   233,   234,   235,   236,   237,
     238,   239,   240,   241,   242,   243,   244,   245,   246,   247,
     248,   249,   250,   251,   252,   253,   254,   255,   256,   257,
     258,   259,   260,   261,   262,   263,    -1,   265,   266,   267,
      -1,   269,   270,   271,   272,   273,   274,   275,   276,   277,
     278,   279,   280,   281,   282,   283,   284,   285,   286,   287,
     288,   289,   290,   291,   292,   293,   294,   295,   296,   297,
     298,   299,   300,   301,   302,   303,   304,   305,   306,   307,
     308,   309,   310,   311,   312,   313,   314,   315,   316,    -1,
     318,    -1,   320,   321,   322,   323,   324,   325,   326,    11,
      12,    -1,    -1,    -1,    16,    -1,    -1,    -1,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    -1,    30,    -1,
      -1,    33,    34,    -1,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    -1,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    -1,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    -1,    95,    96,    97,    98,    99,    -1,   101,
     102,   103,    -1,    -1,   106,   107,    -1,    -1,    -1,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,    -1,
     122,    -1,    -1,    -1,   126,    -1,   128,   129,   130,   131,
      -1,   133,    -1,    -1,    -1,   137,   138,   139,   140,   141,
      -1,    -1,    -1,    -1,    -1,    -1,   148,   149,   150,   151,
     152,   153,   154,   155,   156,    -1,   158,   159,   160,   161,
     162,    -1,    -1,    -1,   166,   167,   168,   169,   170,   171,
     172,   173,    -1,   175,    -1,   177,   178,    -1,    -1,   181,
     182,   183,    -1,    -1,    -1,   187,    -1,   189,   190,   191,
     192,   193,   194,    -1,   196,    -1,   198,    -1,   200,   201,
     202,   203,   204,   205,   206,   207,   208,   209,   210,    -1,
      -1,   213,   214,   215,    -1,   217,   218,   219,   220,   221,
     222,   223,   224,   225,   226,   227,   228,   229,   230,   231,
     232,   233,   234,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,   245,   246,   247,   248,   249,   250,   251,
     252,   253,   254,   255,   256,   257,   258,   259,   260,   261,
     262,   263,    -1,   265,   266,   267,    -1,   269,   270,   271,
     272,   273,   274,   275,   276,   277,   278,   279,   280,   281,
     282,   283,   284,   285,   286,   287,   288,   289,   290,   291,
     292,   293,   294,   295,   296,   297,   298,   299,   300,   301,
     302,   303,   304,   305,   306,   307,   308,   309,   310,   311,
     312,   313,   314,   315,   316,    -1,   318,    -1,   320,   321,
     322,   323,   324,   325,   326,    11,    12,    -1,    -1,    -1,
      16,    -1,    -1,    -1,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    -1,    30,    -1,    -1,    33,    34,    -1,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    -1,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    -1,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    -1,    95,
      96,    97,    98,    99,    -1,   101,   102,   103,    -1,    -1,
     106,   107,    -1,    -1,    -1,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,    -1,   122,    -1,    -1,    -1,
     126,    -1,   128,   129,   130,   131,    -1,   133,    -1,    -1,
      -1,   137,   138,   139,   140,   141,    -1,    -1,    -1,    -1,
      -1,    -1,   148,   149,   150,   151,   152,   153,   154,   155,
     156,    -1,   158,   159,   160,   161,   162,    -1,    -1,    -1,
     166,   167,   168,   169,   170,   171,   172,   173,    -1,   175,
      -1,   177,   178,    -1,    -1,   181,   182,   183,    -1,    -1,
      -1,   187,    -1,   189,   190,   191,   192,   193,   194,    -1,
     196,    -1,   198,    -1,   200,   201,   202,   203,   204,   205,
     206,   207,   208,   209,   210,    -1,    -1,   213,   214,   215,
      -1,   217,   218,   219,   220,   221,   222,   223,   224,   225,
     226,   227,   228,   229,   230,   231,   232,   233,   234,   235,
     236,   237,   238,   239,   240,   241,   242,   243,   244,   245,
     246,   247,   248,   249,   250,   251,   252,   253,   254,   255,
     256,   257,   258,   259,   260,   261,   262,   263,    -1,   265,
     266,   267,    -1,   269,   270,   271,   272,   273,   274,   275,
     276,   277,   278,   279,   280,   281,   282,   283,   284,   285,
     286,   287,   288,   289,   290,   291,   292,   293,   294,   295,
     296,   297,   298,   299,   300,   301,   302,   303,   304,   305,
     306,   307,   308,   309,   310,   311,   312,   313,   314,   315,
     316,    -1,   318,    -1,   320,   321,   322,   323,   324,   325,
     326,    11,    12,    -1,    -1,    -1,    16,    -1,    -1,    -1,
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
     120,    -1,   122,    -1,    -1,    -1,   126,    -1,   128,   129,
     130,   131,    -1,   133,    -1,    -1,    -1,   137,   138,   139,
     140,   141,    -1,    -1,    -1,    -1,    -1,    -1,   148,   149,
     150,   151,   152,   153,   154,   155,   156,    -1,   158,   159,
     160,   161,   162,    -1,    -1,    -1,   166,   167,   168,   169,
     170,   171,   172,   173,    -1,   175,    -1,   177,   178,    -1,
      -1,   181,   182,   183,    -1,    -1,    -1,   187,    -1,   189,
     190,   191,   192,   193,   194,    -1,   196,    -1,   198,    -1,
     200,   201,   202,   203,   204,   205,   206,   207,   208,   209,
     210,    -1,    -1,   213,   214,   215,    -1,   217,   218,   219,
     220,   221,   222,   223,   224,   225,   226,   227,   228,   229,
     230,   231,   232,   233,   234,   235,   236,   237,   238,   239,
     240,   241,   242,   243,   244,   245,   246,   247,   248,   249,
     250,   251,   252,   253,   254,   255,   256,   257,   258,   259,
     260,   261,   262,   263,    -1,   265,   266,   267,    -1,   269,
     270,   271,   272,   273,   274,   275,   276,   277,   278,   279,
     280,   281,   282,   283,   284,   285,   286,   287,   288,   289,
     290,   291,   292,   293,   294,   295,   296,   297,   298,   299,
     300,   301,   302,   303,   304,   305,   306,   307,   308,   309,
     310,   311,   312,   313,   314,   315,   316,    -1,   318,    -1,
     320,   321,   322,   323,   324,   325,   326,    11,    12,    -1,
      -1,    -1,    16,    -1,    -1,    -1,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    -1,    30,    -1,    -1,    33,
      34,    -1,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    -1,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    -1,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      -1,    95,    96,    97,    98,    99,    -1,   101,   102,   103,
      -1,    -1,   106,   107,    -1,    -1,    -1,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,    -1,   122,    -1,
      -1,    -1,   126,    -1,   128,   129,   130,   131,    -1,   133,
      -1,    -1,    -1,   137,   138,   139,   140,   141,    -1,    -1,
      -1,    -1,    -1,    -1,   148,   149,   150,   151,   152,   153,
     154,   155,   156,    -1,   158,   159,   160,   161,   162,    -1,
      -1,    -1,   166,   167,   168,   169,   170,   171,   172,   173,
      -1,   175,    -1,   177,   178,    -1,    -1,   181,   182,   183,
      -1,    -1,    -1,   187,    -1,   189,   190,   191,   192,   193,
     194,    -1,   196,    -1,   198,    -1,   200,   201,   202,   203,
     204,   205,   206,   207,   208,   209,   210,    -1,    -1,   213,
     214,   215,    -1,   217,   218,   219,   220,   221,   222,   223,
     224,   225,   226,   227,   228,   229,   230,   231,   232,   233,
     234,   235,   236,   237,   238,   239,   240,   241,   242,   243,
     244,   245,   246,   247,   248,   249,   250,   251,   252,   253,
     254,   255,   256,   257,   258,   259,   260,   261,   262,   263,
      -1,   265,   266,   267,    -1,   269,   270,   271,   272,   273,
     274,   275,   276,   277,   278,   279,   280,   281,   282,   283,
     284,   285,   286,   287,   288,   289,   290,   291,   292,   293,
     294,   295,   296,   297,   298,   299,   300,   301,   302,   303,
     304,   305,   306,   307,   308,   309,   310,   311,   312,   313,
     314,   315,   316,    -1,   318,    -1,   320,   321,   322,   323,
     324,   325,   326,    11,    12,    -1,    -1,    -1,    16,    -1,
      -1,    -1,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    -1,    30,    -1,    -1,    33,    34,    -1,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    -1,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    -1,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    -1,    95,    96,    97,
      98,    99,    -1,   101,   102,   103,    -1,    -1,   106,   107,
      -1,    -1,    -1,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,    -1,   122,    -1,    -1,    -1,   126,    -1,
     128,   129,   130,   131,    -1,   133,    -1,    -1,    -1,   137,
     138,   139,   140,   141,    -1,    -1,    -1,    -1,    -1,    -1,
     148,   149,   150,   151,   152,   153,   154,   155,   156,    -1,
     158,   159,   160,   161,   162,    -1,    -1,    -1,   166,   167,
     168,   169,   170,   171,   172,   173,    -1,   175,    -1,   177,
     178,    -1,    -1,   181,   182,   183,    -1,    -1,    -1,   187,
      -1,   189,   190,   191,   192,   193,   194,    -1,   196,    -1,
     198,    -1,   200,   201,   202,   203,   204,   205,   206,   207,
     208,   209,   210,    -1,    -1,   213,   214,   215,    -1,   217,
     218,   219,   220,   221,   222,   223,   224,   225,   226,   227,
     228,   229,   230,   231,   232,   233,   234,   235,   236,   237,
     238,   239,   240,   241,   242,   243,   244,   245,   246,   247,
     248,   249,   250,   251,   252,   253,   254,   255,   256,   257,
     258,   259,   260,   261,   262,   263,    -1,   265,   266,   267,
      -1,   269,   270,   271,   272,   273,   274,   275,   276,   277,
     278,   279,   280,   281,   282,   283,   284,   285,   286,   287,
     288,   289,   290,   291,   292,   293,   294,   295,   296,   297,
     298,   299,   300,   301,   302,   303,   304,   305,   306,   307,
     308,   309,   310,   311,   312,   313,   314,   315,   316,    -1,
     318,    -1,   320,   321,   322,   323,   324,   325,   326,    11,
      12,    -1,    -1,    -1,    16,    -1,    -1,    -1,    20,    21,
      22,    23,    24,    25,    26,    -1,    -1,    -1,    30,    -1,
      -1,    33,    34,    -1,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    -1,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    -1,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    -1,    95,    96,    97,    98,    99,    -1,   101,
     102,   103,    -1,    -1,   106,   107,    -1,    -1,    -1,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,    -1,
     122,    -1,    -1,    -1,   126,    -1,   128,   129,   130,   131,
      -1,   133,    -1,    -1,    -1,   137,   138,   139,   140,   141,
      -1,    -1,    -1,    -1,   146,    -1,   148,   149,   150,   151,
     152,   153,   154,   155,   156,    -1,   158,   159,   160,   161,
     162,    -1,    -1,    -1,   166,   167,   168,   169,   170,   171,
     172,   173,    -1,   175,    -1,   177,   178,    -1,    -1,   181,
     182,   183,    -1,    -1,    -1,   187,   188,   189,   190,   191,
      -1,   193,   194,    -1,   196,    -1,   198,    -1,   200,   201,
     202,   203,   204,   205,   206,   207,   208,   209,   210,    -1,
      -1,   213,   214,   215,    -1,   217,   218,   219,   220,   221,
     222,   223,   224,   225,   226,   227,   228,   229,   230,   231,
     232,   233,   234,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,   245,   246,   247,   248,   249,   250,   251,
     252,   253,   254,   255,   256,   257,   258,   259,   260,   261,
     262,   263,    -1,   265,   266,   267,    -1,   269,   270,   271,
     272,   273,   274,   275,   276,   277,   278,   279,   280,   281,
     282,   283,   284,   285,   286,   287,   288,   289,   290,   291,
     292,   293,   294,   295,   296,   297,   298,   299,   300,   301,
     302,   303,   304,   305,   306,   307,   308,   309,   310,   311,
     312,   313,   314,   315,   316,    -1,   318,    -1,   320,   321,
     322,   323,   324,   325,   326,    11,    12,    -1,    -1,    -1,
      16,    -1,    -1,    -1,    20,    21,    22,    23,    24,    25,
      26,    -1,    -1,    -1,    30,    -1,    -1,    33,    34,    -1,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    -1,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    -1,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    -1,    95,
      96,    97,    98,    99,    -1,   101,   102,   103,    -1,    -1,
     106,   107,    -1,    -1,    -1,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,    -1,   122,    -1,    -1,    -1,
     126,    -1,   128,   129,   130,   131,    -1,   133,    -1,    -1,
      -1,   137,   138,   139,   140,   141,    -1,    -1,    -1,    -1,
     146,    -1,   148,   149,   150,   151,   152,   153,   154,   155,
     156,    -1,   158,   159,   160,   161,   162,    -1,    -1,    -1,
     166,   167,   168,   169,   170,   171,   172,   173,    -1,   175,
      -1,   177,   178,    -1,    -1,   181,   182,   183,    -1,    -1,
      -1,   187,   188,   189,   190,   191,    -1,   193,   194,    -1,
     196,    -1,   198,    -1,   200,   201,   202,   203,   204,   205,
     206,   207,   208,   209,   210,    -1,    -1,   213,   214,   215,
      -1,   217,   218,   219,   220,   221,   222,   223,   224,   225,
     226,   227,   228,   229,   230,   231,   232,   233,   234,   235,
     236,   237,   238,   239,   240,   241,   242,   243,   244,   245,
     246,   247,   248,   249,   250,   251,   252,   253,   254,   255,
     256,   257,   258,   259,   260,   261,   262,   263,    -1,   265,
     266,   267,    -1,   269,   270,   271,   272,   273,   274,   275,
     276,   277,   278,   279,   280,   281,   282,   283,   284,   285,
     286,   287,   288,   289,   290,   291,   292,   293,   294,   295,
     296,   297,   298,   299,   300,   301,   302,   303,   304,   305,
     306,   307,   308,   309,   310,   311,   312,   313,   314,   315,
     316,    -1,   318,    -1,   320,   321,   322,   323,   324,   325,
     326,    11,    12,    -1,    -1,    -1,    16,    -1,    -1,    -1,
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
     120,    -1,   122,    -1,    -1,    -1,   126,    -1,   128,   129,
     130,   131,    -1,   133,    -1,    -1,    -1,   137,   138,   139,
     140,   141,    -1,    -1,    -1,    -1,    -1,    -1,   148,   149,
     150,   151,   152,   153,   154,   155,   156,    -1,   158,   159,
     160,   161,   162,    -1,    -1,    -1,   166,   167,   168,   169,
     170,   171,   172,   173,    -1,   175,    -1,   177,   178,    -1,
      -1,   181,   182,   183,    -1,    -1,   186,   187,    -1,   189,
     190,   191,    -1,   193,   194,    -1,   196,    -1,   198,    -1,
     200,   201,   202,   203,   204,   205,   206,   207,   208,   209,
     210,    -1,    -1,   213,   214,   215,    -1,   217,   218,   219,
     220,   221,   222,   223,   224,   225,   226,   227,   228,   229,
     230,   231,   232,   233,   234,   235,   236,   237,   238,   239,
     240,   241,   242,   243,   244,   245,   246,   247,   248,   249,
     250,   251,   252,   253,   254,   255,   256,   257,   258,   259,
     260,   261,   262,   263,    -1,   265,   266,   267,    -1,   269,
     270,   271,   272,   273,   274,   275,   276,   277,   278,   279,
     280,   281,   282,   283,   284,   285,   286,   287,   288,   289,
     290,   291,   292,   293,   294,   295,   296,   297,   298,   299,
     300,   301,   302,   303,   304,   305,   306,   307,   308,   309,
     310,   311,   312,   313,   314,   315,   316,    -1,   318,    -1,
     320,   321,   322,   323,   324,   325,   326,    11,    12,    -1,
      -1,    -1,    16,    -1,    -1,    -1,    20,    21,    22,    23,
      24,    25,    26,    -1,    -1,    -1,    30,    -1,    -1,    33,
      34,    -1,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    -1,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    -1,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      -1,    95,    96,    97,    98,    99,    -1,   101,   102,   103,
      -1,    -1,   106,   107,    -1,    -1,    -1,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,    -1,   122,    -1,
      -1,    -1,   126,    -1,   128,   129,   130,   131,    -1,   133,
      -1,    -1,    -1,   137,   138,   139,   140,   141,    -1,    -1,
      -1,    -1,    -1,    -1,   148,   149,   150,   151,   152,   153,
     154,   155,   156,    -1,   158,   159,   160,   161,   162,    -1,
      -1,    -1,   166,   167,   168,   169,   170,   171,   172,   173,
      -1,   175,    -1,   177,   178,    -1,    -1,   181,   182,   183,
      -1,    -1,    -1,   187,   188,   189,   190,   191,    -1,   193,
     194,    -1,   196,    -1,   198,    -1,   200,   201,   202,   203,
     204,   205,   206,   207,   208,   209,   210,    -1,    -1,   213,
     214,   215,    -1,   217,   218,   219,   220,   221,   222,   223,
     224,   225,   226,   227,   228,   229,   230,   231,   232,   233,
     234,   235,   236,   237,   238,   239,   240,   241,   242,   243,
     244,   245,   246,   247,   248,   249,   250,   251,   252,   253,
     254,   255,   256,   257,   258,   259,   260,   261,   262,   263,
      -1,   265,   266,   267,    -1,   269,   270,   271,   272,   273,
     274,   275,   276,   277,   278,   279,   280,   281,   282,   283,
     284,   285,   286,   287,   288,   289,   290,   291,   292,   293,
     294,   295,   296,   297,   298,   299,   300,   301,   302,   303,
     304,   305,   306,   307,   308,   309,   310,   311,   312,   313,
     314,   315,   316,    -1,   318,    -1,   320,   321,   322,   323,
     324,   325,   326,    11,    12,    -1,    -1,    -1,    16,    -1,
      -1,    -1,    20,    21,    22,    23,    24,    25,    26,    -1,
      -1,    -1,    30,    -1,    -1,    33,    34,    -1,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    -1,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    -1,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    -1,    95,    96,    97,
      98,    99,    -1,   101,   102,   103,    -1,    -1,   106,   107,
      -1,    -1,    -1,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,    -1,   122,    -1,    -1,    -1,   126,    -1,
     128,   129,   130,   131,    -1,   133,    -1,    -1,    -1,   137,
     138,   139,   140,   141,    -1,    -1,    -1,    -1,    -1,    -1,
     148,   149,   150,   151,   152,   153,   154,   155,   156,    -1,
     158,   159,   160,   161,   162,    -1,    -1,    -1,   166,   167,
     168,   169,   170,   171,   172,   173,    -1,   175,    -1,   177,
     178,    -1,    -1,   181,   182,   183,    -1,    -1,    -1,   187,
      -1,   189,   190,   191,    -1,   193,   194,    -1,   196,    -1,
     198,    -1,   200,   201,   202,   203,   204,   205,   206,   207,
     208,   209,   210,    -1,    -1,   213,   214,   215,    -1,   217,
     218,   219,   220,   221,   222,   223,   224,   225,   226,   227,
     228,   229,   230,   231,   232,   233,   234,   235,   236,   237,
     238,   239,   240,   241,   242,   243,   244,   245,   246,   247,
     248,   249,   250,   251,   252,   253,   254,   255,   256,   257,
     258,   259,   260,   261,   262,   263,    -1,   265,   266,   267,
      -1,   269,   270,   271,   272,   273,   274,   275,   276,   277,
     278,   279,   280,   281,   282,   283,   284,   285,   286,   287,
     288,   289,   290,   291,   292,   293,   294,   295,   296,   297,
     298,   299,   300,   301,   302,   303,   304,   305,   306,   307,
     308,   309,   310,   311,   312,   313,   314,   315,   316,   317,
     318,    -1,   320,   321,   322,   323,   324,   325,   326,    11,
      12,    -1,    -1,    -1,    16,    -1,    -1,    -1,    20,    21,
      22,    23,    24,    25,    26,    -1,    -1,    -1,    30,    -1,
      -1,    33,    34,    -1,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    -1,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    -1,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    -1,    95,    96,    97,    98,    99,    -1,   101,
     102,   103,    -1,    -1,   106,   107,    -1,    -1,    -1,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,    -1,
     122,    -1,    -1,    -1,   126,    -1,   128,   129,   130,   131,
      -1,   133,    -1,    -1,    -1,   137,   138,   139,   140,   141,
      -1,    -1,    -1,    -1,    -1,    -1,   148,   149,   150,   151,
     152,   153,   154,   155,   156,    -1,   158,   159,   160,   161,
     162,    -1,    -1,    -1,   166,   167,   168,   169,   170,   171,
     172,   173,    -1,   175,    -1,   177,   178,    -1,    -1,   181,
     182,   183,    -1,    -1,    -1,   187,    -1,   189,   190,   191,
      -1,   193,   194,    -1,   196,    -1,   198,    -1,   200,   201,
     202,   203,   204,   205,   206,   207,   208,   209,   210,    -1,
      -1,   213,   214,   215,    -1,   217,   218,   219,   220,   221,
     222,   223,   224,   225,   226,   227,   228,   229,   230,   231,
     232,   233,   234,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,   245,   246,   247,   248,   249,   250,   251,
     252,   253,   254,   255,   256,   257,   258,   259,   260,   261,
     262,   263,    -1,   265,   266,   267,    -1,   269,   270,   271,
     272,   273,   274,   275,   276,   277,   278,   279,   280,   281,
     282,   283,   284,   285,   286,   287,   288,   289,   290,   291,
     292,   293,   294,   295,   296,   297,   298,   299,   300,   301,
     302,   303,   304,   305,   306,   307,   308,   309,   310,   311,
     312,   313,   314,   315,   316,    -1,   318,   319,   320,   321,
     322,   323,   324,   325,   326,    11,    12,    -1,    -1,    -1,
      16,    -1,    -1,    -1,    20,    21,    22,    23,    24,    25,
      26,    -1,    -1,    -1,    30,    -1,    -1,    33,    34,    -1,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    -1,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    -1,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    -1,    95,
      96,    97,    98,    99,    -1,   101,   102,   103,    -1,    -1,
     106,   107,    -1,    -1,    -1,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,    -1,   122,    -1,    -1,    -1,
     126,    -1,   128,   129,   130,   131,    -1,   133,    -1,    -1,
      -1,   137,   138,   139,   140,   141,    -1,    -1,    -1,    -1,
      -1,    -1,   148,   149,   150,   151,   152,   153,   154,   155,
     156,    -1,   158,   159,   160,   161,   162,    -1,    -1,    -1,
     166,   167,   168,   169,   170,   171,   172,   173,    -1,   175,
      -1,   177,   178,    -1,    -1,   181,   182,   183,    -1,    -1,
     186,   187,    -1,   189,   190,   191,    -1,   193,   194,    -1,
     196,    -1,   198,    -1,   200,   201,   202,   203,   204,   205,
     206,   207,   208,   209,   210,    -1,    -1,   213,   214,   215,
      -1,   217,   218,   219,   220,   221,   222,   223,   224,   225,
     226,   227,   228,   229,   230,   231,   232,   233,   234,   235,
     236,   237,   238,   239,   240,   241,   242,   243,   244,   245,
     246,   247,   248,   249,   250,   251,   252,   253,   254,   255,
     256,   257,   258,   259,   260,   261,   262,   263,    -1,   265,
     266,   267,    -1,   269,   270,   271,   272,   273,   274,   275,
     276,   277,   278,   279,   280,   281,   282,   283,   284,   285,
     286,   287,   288,   289,   290,   291,   292,   293,   294,   295,
     296,   297,   298,   299,   300,   301,   302,   303,   304,   305,
     306,   307,   308,   309,   310,   311,   312,   313,   314,   315,
     316,    -1,   318,    -1,   320,   321,   322,   323,   324,   325,
     326,    11,    12,    -1,    -1,    -1,    16,    -1,    -1,    -1,
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
     120,    -1,   122,    -1,    -1,    -1,   126,    -1,   128,   129,
     130,   131,    -1,   133,    -1,    -1,    -1,   137,   138,   139,
     140,   141,    -1,    -1,    -1,    -1,    -1,    -1,   148,   149,
     150,   151,   152,   153,   154,   155,   156,    -1,   158,   159,
     160,   161,   162,    -1,    -1,    -1,   166,   167,   168,   169,
     170,   171,   172,   173,    -1,   175,    -1,   177,   178,    -1,
      -1,   181,   182,   183,    -1,    -1,    -1,   187,   188,   189,
     190,   191,    -1,   193,   194,    -1,   196,    -1,   198,    -1,
     200,   201,   202,   203,   204,   205,   206,   207,   208,   209,
     210,    -1,    -1,   213,   214,   215,    -1,   217,   218,   219,
     220,   221,   222,   223,   224,   225,   226,   227,   228,   229,
     230,   231,   232,   233,   234,   235,   236,   237,   238,   239,
     240,   241,   242,   243,   244,   245,   246,   247,   248,   249,
     250,   251,   252,   253,   254,   255,   256,   257,   258,   259,
     260,   261,   262,   263,    -1,   265,   266,   267,    -1,   269,
     270,   271,   272,   273,   274,   275,   276,   277,   278,   279,
     280,   281,   282,   283,   284,   285,   286,   287,   288,   289,
     290,   291,   292,   293,   294,   295,   296,   297,   298,   299,
     300,   301,   302,   303,   304,   305,   306,   307,   308,   309,
     310,   311,   312,   313,   314,   315,   316,    -1,   318,    -1,
     320,   321,   322,   323,   324,   325,   326,    11,    12,    -1,
      -1,    -1,    16,    -1,    -1,    -1,    20,    21,    22,    23,
      24,    25,    26,    -1,    -1,    -1,    30,    -1,    -1,    33,
      34,    -1,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    -1,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    -1,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      -1,    95,    96,    97,    98,    99,    -1,   101,   102,   103,
      -1,    -1,   106,   107,    -1,    -1,    -1,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,    -1,   122,    -1,
      -1,    -1,   126,    -1,   128,   129,   130,   131,    -1,   133,
      -1,    -1,    -1,   137,   138,   139,   140,   141,    -1,    -1,
      -1,    -1,   146,    -1,   148,   149,   150,   151,   152,   153,
     154,   155,   156,    -1,   158,   159,   160,   161,   162,    -1,
      -1,    -1,   166,   167,   168,   169,   170,   171,   172,   173,
      -1,   175,    -1,   177,   178,    -1,    -1,   181,   182,   183,
      -1,    -1,    -1,   187,    -1,   189,   190,   191,    -1,   193,
     194,    -1,   196,    -1,   198,    -1,   200,   201,   202,   203,
     204,   205,   206,   207,   208,   209,   210,    -1,    -1,   213,
     214,   215,    -1,   217,   218,   219,   220,   221,   222,   223,
     224,   225,   226,   227,   228,   229,   230,   231,   232,   233,
     234,   235,   236,   237,   238,   239,   240,   241,   242,   243,
     244,   245,   246,   247,   248,   249,   250,   251,   252,   253,
     254,   255,   256,   257,   258,   259,   260,   261,   262,   263,
      -1,   265,   266,   267,    -1,   269,   270,   271,   272,   273,
     274,   275,   276,   277,   278,   279,   280,   281,   282,   283,
     284,   285,   286,   287,   288,   289,   290,   291,   292,   293,
     294,   295,   296,   297,   298,   299,   300,   301,   302,   303,
     304,   305,   306,   307,   308,   309,   310,   311,   312,   313,
     314,   315,   316,    -1,   318,    -1,   320,   321,   322,   323,
     324,   325,   326,    11,    12,    -1,    -1,    -1,    16,    -1,
      -1,    -1,    20,    21,    22,    23,    24,    25,    26,    -1,
      -1,    -1,    30,    -1,    -1,    33,    34,    -1,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    -1,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    -1,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    -1,    95,    96,    97,
      98,    99,    -1,   101,   102,   103,    -1,    -1,   106,   107,
      -1,    -1,    -1,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,    -1,   122,    -1,    -1,    -1,   126,    -1,
     128,   129,   130,   131,    -1,   133,    -1,    -1,    -1,   137,
     138,   139,   140,   141,    -1,    -1,    -1,    -1,    -1,    -1,
     148,   149,   150,   151,   152,   153,   154,   155,   156,    -1,
     158,   159,   160,   161,   162,    -1,    -1,    -1,   166,   167,
     168,   169,   170,   171,   172,   173,    -1,   175,    -1,   177,
     178,    -1,    -1,   181,   182,   183,    -1,    -1,    -1,   187,
      -1,   189,   190,   191,    -1,   193,   194,    -1,   196,    -1,
     198,    -1,   200,   201,   202,   203,   204,   205,   206,   207,
     208,   209,   210,    -1,    -1,   213,   214,   215,    -1,   217,
     218,   219,   220,   221,   222,   223,   224,   225,   226,   227,
     228,   229,   230,   231,   232,   233,   234,   235,   236,   237,
     238,   239,   240,   241,   242,   243,   244,   245,   246,   247,
     248,   249,   250,   251,   252,   253,   254,   255,   256,   257,
     258,   259,   260,   261,   262,   263,    -1,   265,   266,   267,
      -1,   269,   270,   271,   272,   273,   274,   275,   276,   277,
     278,   279,   280,   281,   282,   283,   284,   285,   286,   287,
     288,   289,   290,   291,   292,   293,   294,   295,   296,   297,
     298,   299,   300,   301,   302,   303,   304,   305,   306,   307,
     308,   309,   310,   311,   312,   313,   314,   315,   316,    -1,
     318,    -1,   320,   321,   322,   323,   324,   325,   326,    11,
      12,    -1,    -1,    -1,    16,    -1,    -1,    -1,    20,    21,
      22,    23,    24,    25,    26,    -1,    -1,    -1,    30,    -1,
      -1,    33,    34,    -1,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    -1,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    -1,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    -1,    95,    96,    97,    98,    99,    -1,   101,
     102,   103,    -1,    -1,   106,   107,    -1,    -1,    -1,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,    -1,
     122,    -1,    -1,    -1,   126,    -1,   128,   129,   130,   131,
      -1,   133,    -1,    -1,    -1,   137,   138,   139,   140,   141,
      -1,    -1,    -1,    -1,    -1,    -1,   148,   149,   150,   151,
     152,   153,   154,   155,   156,    -1,   158,   159,   160,   161,
     162,    -1,    -1,    -1,   166,   167,   168,   169,   170,   171,
     172,   173,    -1,   175,    -1,   177,   178,    -1,    -1,   181,
     182,   183,    -1,    -1,    -1,   187,    -1,   189,   190,   191,
      -1,   193,   194,    -1,   196,    -1,   198,    -1,   200,   201,
     202,   203,   204,   205,   206,   207,   208,   209,   210,    -1,
      -1,   213,   214,   215,    -1,   217,   218,   219,   220,   221,
     222,   223,   224,   225,   226,   227,   228,   229,   230,   231,
     232,   233,   234,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,   245,   246,   247,   248,   249,   250,   251,
     252,   253,   254,   255,   256,   257,   258,   259,   260,   261,
     262,   263,    -1,   265,   266,   267,    -1,   269,   270,   271,
     272,   273,   274,   275,   276,   277,   278,   279,   280,   281,
     282,   283,   284,   285,   286,   287,   288,   289,   290,   291,
     292,   293,   294,   295,   296,   297,   298,   299,   300,   301,
     302,   303,   304,   305,   306,   307,   308,   309,   310,   311,
     312,   313,   314,   315,   316,    -1,   318,    -1,   320,   321,
     322,   323,   324,   325,   326,    11,    12,    -1,    -1,    -1,
      16,    -1,    -1,    -1,    20,    21,    22,    23,    24,    25,
      26,    -1,    -1,    -1,    30,    -1,    -1,    33,    34,    -1,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    -1,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    -1,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    -1,    95,
      96,    97,    98,    99,    -1,   101,   102,   103,    -1,    -1,
     106,   107,    -1,    -1,    -1,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,    -1,   122,    -1,    -1,    -1,
     126,    -1,   128,   129,   130,   131,    -1,   133,    -1,    -1,
      -1,   137,   138,   139,   140,   141,    -1,    -1,    -1,    -1,
      -1,    -1,   148,   149,   150,   151,   152,   153,   154,   155,
     156,    -1,   158,   159,   160,   161,   162,    -1,    -1,    -1,
     166,   167,   168,   169,   170,   171,   172,   173,    -1,   175,
      -1,   177,   178,    -1,    -1,   181,   182,   183,    -1,    -1,
      -1,   187,    -1,   189,   190,   191,    -1,   193,   194,    -1,
     196,    -1,   198,    -1,   200,   201,   202,   203,   204,   205,
     206,   207,   208,   209,   210,    -1,    -1,   213,   214,   215,
      -1,   217,   218,   219,   220,   221,   222,   223,   224,   225,
     226,   227,   228,   229,   230,   231,   232,   233,   234,   235,
     236,   237,   238,   239,   240,   241,   242,   243,   244,   245,
     246,   247,   248,   249,   250,   251,   252,   253,   254,   255,
     256,   257,   258,   259,   260,   261,   262,   263,    -1,   265,
     266,   267,    -1,   269,   270,   271,   272,   273,   274,   275,
     276,   277,   278,   279,   280,   281,   282,   283,   284,   285,
     286,   287,   288,   289,   290,   291,   292,   293,   294,   295,
     296,   297,   298,   299,   300,   301,   302,   303,   304,   305,
     306,   307,   308,   309,   310,   311,   312,   313,   314,   315,
     316,    -1,   318,    -1,   320,   321,   322,   323,   324,   325,
     326,    11,    12,    -1,    -1,    -1,    16,    -1,    -1,    -1,
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
     120,    -1,   122,    -1,    -1,    -1,   126,    -1,   128,   129,
     130,   131,    -1,   133,    -1,    -1,    -1,   137,   138,   139,
     140,   141,    -1,    -1,    -1,    -1,    -1,    -1,   148,   149,
     150,   151,   152,   153,   154,   155,   156,    -1,   158,   159,
     160,   161,   162,    -1,    -1,    -1,   166,   167,   168,   169,
     170,   171,   172,   173,    -1,   175,    -1,   177,   178,    -1,
      -1,   181,   182,   183,    -1,    -1,    -1,   187,    -1,   189,
     190,   191,    -1,   193,   194,    -1,   196,    -1,   198,    -1,
     200,   201,   202,   203,   204,   205,   206,   207,   208,   209,
     210,    -1,    -1,   213,   214,   215,    -1,   217,   218,   219,
     220,   221,   222,   223,   224,   225,   226,   227,   228,   229,
     230,   231,   232,   233,   234,   235,   236,   237,   238,   239,
     240,   241,   242,   243,   244,   245,   246,   247,   248,   249,
     250,   251,   252,   253,   254,   255,   256,   257,   258,   259,
     260,   261,   262,   263,    -1,   265,   266,   267,    -1,   269,
     270,   271,   272,   273,   274,   275,   276,   277,   278,   279,
     280,   281,   282,   283,   284,   285,   286,   287,   288,   289,
     290,   291,   292,   293,   294,   295,   296,   297,   298,   299,
     300,   301,   302,   303,   304,   305,   306,   307,   308,   309,
     310,   311,   312,   313,   314,   315,   316,    -1,   318,    -1,
     320,   321,   322,   323,   324,   325,   326,    11,    12,    -1,
      -1,    -1,    16,    -1,    -1,    -1,    20,    21,    22,    23,
      24,    25,    26,    -1,    -1,    -1,    30,    -1,    -1,    33,
      34,    -1,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    -1,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    -1,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      -1,    95,    96,    97,    98,    99,    -1,   101,   102,   103,
      -1,    -1,   106,   107,    -1,    -1,    -1,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,    -1,   122,    -1,
      -1,    -1,   126,    -1,   128,   129,   130,   131,    -1,   133,
      -1,    -1,    -1,   137,   138,   139,   140,   141,    -1,    -1,
      -1,    -1,    -1,    -1,   148,   149,   150,   151,   152,   153,
     154,   155,   156,    -1,   158,   159,   160,   161,   162,    -1,
      -1,    -1,   166,   167,   168,   169,   170,   171,   172,   173,
      -1,   175,    -1,   177,   178,    -1,    -1,   181,   182,   183,
      -1,    -1,    -1,   187,    -1,   189,   190,   191,    -1,   193,
     194,    -1,   196,    -1,   198,    -1,   200,   201,   202,   203,
     204,   205,   206,   207,   208,   209,   210,    -1,    -1,   213,
     214,   215,    -1,   217,   218,   219,   220,   221,   222,   223,
     224,   225,   226,   227,   228,   229,   230,   231,   232,   233,
     234,   235,   236,   237,   238,   239,   240,   241,   242,   243,
     244,   245,   246,   247,   248,   249,   250,   251,   252,   253,
     254,   255,   256,   257,   258,   259,   260,   261,   262,   263,
      -1,   265,   266,   267,    -1,   269,   270,   271,   272,   273,
     274,   275,   276,   277,   278,   279,   280,   281,   282,   283,
     284,   285,   286,   287,   288,   289,   290,   291,   292,   293,
     294,   295,   296,   297,   298,   299,   300,   301,   302,   303,
     304,   305,   306,   307,   308,   309,   310,   311,   312,   313,
     314,   315,   316,    -1,   318,    -1,   320,   321,   322,   323,
     324,   325,   326,    11,    12,    -1,    -1,    -1,    16,    -1,
      -1,    -1,    20,    21,    22,    23,    24,    25,    26,    -1,
      -1,    -1,    30,    -1,    -1,    33,    34,    -1,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    -1,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    -1,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    -1,    95,    96,    97,
      98,    99,    -1,   101,   102,   103,    -1,    -1,   106,   107,
      -1,    -1,    -1,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,    -1,   122,    -1,    -1,    -1,   126,    -1,
     128,   129,   130,   131,    -1,   133,    -1,    -1,    -1,   137,
     138,   139,   140,   141,    -1,    -1,    -1,    -1,    -1,    -1,
     148,   149,   150,   151,   152,   153,   154,   155,   156,    -1,
     158,   159,    -1,   161,   162,    -1,    -1,    -1,   166,   167,
     168,   169,   170,   171,   172,   173,    -1,   175,    -1,    -1,
      -1,    -1,    -1,   181,   182,   183,    -1,    -1,    -1,   187,
      -1,   189,   190,   191,    -1,   193,   194,    -1,   196,    -1,
     198,    -1,   200,   201,   202,   203,   204,   205,   206,   207,
     208,   209,   210,    -1,    -1,   213,   214,   215,    -1,   217,
     218,   219,   220,   221,   222,   223,   224,   225,   226,   227,
     228,   229,   230,   231,   232,   233,   234,   235,   236,   237,
     238,   239,   240,   241,   242,   243,   244,   245,   246,   247,
     248,   249,   250,   251,   252,   253,   254,   255,   256,   257,
     258,   259,   260,   261,   262,   263,    -1,   265,   266,   267,
      -1,   269,   270,   271,   272,   273,   274,   275,   276,   277,
     278,   279,   280,   281,   282,   283,   284,   285,   286,   287,
     288,   289,   290,   291,   292,   293,   294,   295,   296,   297,
     298,   299,   300,   301,   302,   303,   304,   305,   306,   307,
     308,   309,   310,   311,   312,   313,   314,   315,   316,    -1,
     318,    -1,   320,   321,   322,   323,   324,   325,   326,    11,
      12,    -1,    -1,    -1,    16,    -1,    -1,    -1,    20,    21,
      22,    23,    24,    25,    26,    -1,    -1,    -1,    30,    -1,
      -1,    33,    34,    -1,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    -1,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    -1,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    -1,    95,    96,    97,    98,    99,    -1,   101,
     102,   103,    -1,    -1,   106,   107,    -1,    -1,    -1,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,    -1,
     122,    -1,    -1,    -1,   126,    -1,   128,   129,   130,   131,
      -1,   133,    -1,    -1,    -1,   137,   138,   139,   140,   141,
      -1,    -1,    -1,    -1,    -1,    -1,   148,   149,   150,   151,
     152,   153,   154,   155,   156,    -1,   158,   159,    -1,   161,
     162,    -1,    -1,    -1,   166,   167,   168,   169,   170,   171,
     172,   173,    -1,   175,    -1,    -1,    -1,    -1,    -1,   181,
     182,   183,    -1,    -1,    -1,   187,    -1,   189,   190,   191,
      -1,    -1,    -1,    -1,   196,    -1,   198,    -1,   200,   201,
     202,   203,   204,   205,   206,   207,   208,   209,   210,    -1,
      -1,   213,   214,   215,    -1,   217,   218,   219,   220,   221,
     222,   223,   224,   225,   226,   227,   228,   229,   230,   231,
     232,   233,   234,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,   245,   246,   247,   248,   249,   250,   251,
     252,   253,   254,   255,   256,   257,   258,   259,   260,   261,
     262,   263,    -1,   265,   266,   267,    -1,   269,   270,   271,
     272,   273,   274,   275,   276,   277,   278,   279,   280,   281,
     282,   283,   284,   285,   286,   287,   288,   289,   290,   291,
     292,   293,   294,   295,   296,   297,   298,   299,   300,   301,
     302,   303,   304,   305,   306,   307,   308,   309,   310,   311,
     312,   313,   314,   315,   316,    -1,   318,    -1,   320,   321,
     322,   323,   324,   325,   326,    11,    12,    -1,    -1,    -1,
      16,    -1,    -1,    -1,    20,    21,    22,    23,    24,    25,
      26,    -1,    -1,    -1,    30,    -1,    -1,    33,    34,    -1,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    -1,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    -1,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    -1,    95,
      96,    97,    98,    99,    -1,   101,   102,   103,    -1,    -1,
     106,   107,    -1,    -1,    -1,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,    -1,   122,    -1,    -1,    -1,
     126,    -1,   128,   129,   130,   131,    -1,   133,    -1,    -1,
      -1,   137,    -1,   139,   140,   141,    -1,    -1,    -1,    -1,
      -1,    -1,   148,   149,   150,   151,    -1,   153,   154,   155,
     156,    -1,   158,   159,    -1,   161,   162,    -1,    -1,    -1,
     166,   167,   168,   169,   170,   171,   172,   173,    -1,   175,
      -1,    -1,    -1,    -1,    -1,   181,   182,   183,    -1,    -1,
      -1,   187,    -1,   189,   190,   191,    -1,    -1,    -1,    -1,
     196,    -1,   198,    -1,   200,    -1,   202,    -1,   204,   205,
     206,   207,   208,   209,   210,    -1,    -1,   213,   214,   215,
      -1,   217,   218,   219,   220,   221,   222,   223,   224,   225,
     226,   227,   228,   229,   230,   231,   232,   233,   234,   235,
     236,   237,   238,   239,   240,   241,   242,   243,   244,   245,
     246,   247,   248,   249,   250,   251,   252,   253,   254,   255,
     256,   257,   258,   259,   260,   261,   262,   263,    -1,   265,
     266,   267,    -1,   269,   270,   271,   272,   273,   274,   275,
     276,   277,   278,   279,   280,   281,   282,   283,   284,   285,
     286,   287,   288,   289,   290,   291,   292,   293,   294,   295,
     296,   297,   298,   299,   300,   301,   302,   303,   304,   305,
     306,   307,   308,   309,   310,   311,   312,   313,   314,   315,
     316,    -1,   318,    -1,   320,   321,   322,   323,   324,   325,
     326,    11,    12,    -1,    -1,    -1,    16,    -1,    -1,    -1,
      20,    21,    22,    23,    24,    25,    26,    -1,    -1,    -1,
      30,    -1,    -1,    33,    34,    -1,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    -1,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    -1,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    -1,    95,    96,    97,    98,    99,
      -1,   101,   102,   103,    -1,    -1,   106,   107,    -1,    -1,
      -1,   111,   112,   113,   114,   115,   116,   117,    -1,   119,
     120,    -1,   122,    -1,    -1,    -1,   126,    -1,   128,   129,
     130,   131,    -1,   133,    -1,    -1,    -1,   137,    -1,   139,
     140,   141,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   149,
     150,   151,    -1,   153,   154,   155,    -1,    -1,    -1,   159,
      -1,    -1,   162,    -1,    -1,    -1,   166,   167,   168,   169,
     170,   171,   172,   173,    -1,   175,    -1,    -1,   178,    -1,
      -1,   181,   182,   183,    -1,    -1,    -1,   187,    -1,   189,
     190,   191,    -1,    -1,    -1,    -1,    -1,    -1,   198,    -1,
     200,    -1,   202,    -1,   204,   205,   206,   207,   208,   209,
     210,    -1,    -1,   213,   214,   215,    -1,   217,   218,   219,
     220,   221,   222,   223,   224,   225,   226,   227,   228,   229,
     230,   231,   232,   233,   234,   235,   236,   237,   238,   239,
     240,   241,   242,   243,   244,   245,   246,   247,   248,   249,
     250,   251,   252,   253,   254,   255,   256,   257,   258,   259,
     260,   261,   262,   263,    -1,   265,   266,   267,    -1,   269,
     270,   271,   272,   273,   274,   275,   276,   277,   278,   279,
     280,   281,   282,   283,   284,   285,   286,   287,   288,   289,
     290,   291,   292,   293,   294,   295,   296,   297,   298,   299,
     300,   301,   302,   303,   304,   305,   306,   307,   308,   309,
     310,   311,   312,   313,   314,   315,   316,    -1,   318,    -1,
     320,   321,   322,   323,   324,   325,   326,    11,    12,    -1,
      -1,    -1,    16,    -1,    -1,    -1,    20,    -1,    -1,    -1,
      -1,    25,    26,    -1,    -1,    -1,    -1,    -1,    -1,    33,
      34,    -1,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    -1,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    -1,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      -1,    95,    96,    97,    98,    -1,    -1,   101,   102,   103,
      -1,    -1,   106,   107,    -1,    -1,    -1,    -1,   112,   113,
     114,   115,   116,   117,   118,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   128,   129,   130,   131,    -1,   133,
      -1,    -1,    -1,   137,   138,   139,   140,   141,    -1,    -1,
      -1,    -1,    -1,    -1,   148,   149,   150,    -1,   152,   153,
     154,    -1,    -1,    -1,    -1,    -1,    -1,   161,   162,    -1,
      -1,    -1,   166,   167,   168,   169,   170,   171,   172,   173,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   181,   182,   183,
      -1,    -1,    -1,   187,    -1,   189,   190,   191,    -1,    -1,
      -1,    -1,   196,    -1,   198,    -1,   200,   201,   202,   203,
     204,   205,   206,   207,   208,   209,   210,    -1,    -1,   213,
     214,    -1,    -1,   217,   218,   219,   220,   221,   222,   223,
     224,   225,   226,   227,   228,   229,   230,   231,   232,   233,
     234,   235,   236,   237,   238,   239,   240,   241,   242,   243,
     244,   245,   246,   247,   248,   249,   250,   251,   252,   253,
     254,   255,   256,   257,   258,   259,   260,   261,   262,   263,
      -1,   265,   266,   267,    -1,   269,   270,   271,   272,   273,
     274,   275,   276,   277,   278,   279,   280,   281,   282,   283,
     284,   285,   286,   287,   288,   289,   290,   291,   292,   293,
     294,   295,   296,   297,   298,   299,   300,   301,   302,   303,
     304,   305,   306,   307,   308,   309,   310,   311,   312,   313,
     314,   315,    -1,    -1,    -1,    -1,   320,   321,   322,   323,
     324,   325,   326,    11,    12,    -1,    -1,    -1,    16,    -1,
      -1,    -1,    20,    -1,    -1,    -1,    -1,    25,    26,    -1,
      -1,    -1,    -1,    -1,    -1,    33,    34,    -1,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    -1,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    -1,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    -1,    95,    96,    97,
      98,    -1,    -1,   101,   102,   103,    -1,    -1,   106,   107,
      -1,    -1,    -1,    -1,   112,   113,   114,   115,   116,   117,
     118,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     128,   129,   130,   131,    -1,   133,    -1,    -1,    -1,   137,
     138,   139,   140,   141,    -1,    -1,    -1,    -1,    -1,    -1,
     148,   149,   150,    -1,   152,   153,   154,    -1,    -1,    -1,
      -1,    -1,    -1,   161,   162,    -1,    -1,    -1,   166,   167,
     168,   169,   170,   171,   172,   173,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   181,   182,   183,    -1,    -1,    -1,   187,
      -1,   189,   190,   191,    -1,    -1,    -1,    -1,   196,    -1,
     198,    -1,   200,   201,   202,   203,   204,   205,   206,   207,
     208,   209,   210,    -1,    -1,   213,   214,    -1,    -1,   217,
     218,   219,   220,   221,   222,   223,   224,   225,   226,   227,
     228,   229,   230,   231,   232,   233,   234,   235,   236,   237,
     238,   239,   240,   241,   242,   243,   244,   245,   246,   247,
     248,   249,   250,   251,   252,   253,   254,   255,   256,   257,
     258,   259,   260,   261,   262,   263,    -1,   265,   266,   267,
      -1,   269,   270,   271,   272,   273,   274,   275,   276,   277,
     278,   279,   280,   281,   282,   283,   284,   285,   286,   287,
     288,   289,   290,   291,   292,   293,   294,   295,   296,   297,
     298,   299,   300,   301,   302,   303,   304,   305,   306,   307,
     308,   309,   310,   311,   312,   313,   314,   315,    -1,    -1,
      16,    -1,   320,   321,   322,   323,   324,   325,   326,    25,
      26,    -1,    -1,    -1,    30,    -1,    -1,    33,    34,    -1,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    -1,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    -1,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    -1,    95,
      96,    97,    98,    -1,    -1,   101,   102,   103,    -1,    -1,
     106,   107,    -1,    -1,    -1,    -1,   112,   113,   114,   115,
     116,   117,   118,   119,   120,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   128,   129,   130,   131,    -1,   133,    -1,    -1,
      -1,   137,   138,   139,   140,   141,    -1,    -1,    -1,    -1,
      -1,    -1,   148,   149,   150,    -1,   152,   153,   154,    -1,
      -1,    -1,   158,    -1,    -1,   161,   162,    -1,    -1,    -1,
     166,   167,   168,   169,   170,   171,   172,   173,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   181,   182,   183,    -1,    -1,
      -1,   187,    -1,   189,   190,   191,    -1,    -1,    -1,    -1,
      -1,    -1,   198,    -1,   200,   201,   202,   203,   204,   205,
     206,   207,   208,   209,   210,    -1,    -1,   213,   214,    -1,
      -1,   217,   218,   219,   220,   221,   222,   223,   224,   225,
     226,   227,   228,   229,   230,   231,   232,   233,   234,   235,
     236,   237,   238,   239,   240,   241,   242,   243,   244,   245,
     246,   247,   248,   249,   250,   251,   252,   253,   254,   255,
     256,   257,   258,   259,   260,   261,   262,   263,    -1,   265,
     266,   267,    -1,   269,   270,   271,   272,   273,   274,   275,
     276,   277,   278,   279,   280,   281,   282,   283,   284,   285,
     286,   287,   288,   289,   290,   291,   292,   293,   294,   295,
     296,   297,   298,   299,   300,   301,   302,   303,   304,   305,
     306,   307,   308,   309,   310,   311,   312,   313,   314,   315,
      -1,    -1,    16,    -1,   320,   321,   322,   323,   324,   325,
     326,    25,    26,    -1,    -1,    -1,    -1,    -1,    -1,    33,
      34,    -1,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    -1,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    -1,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      -1,    95,    96,    97,    98,    -1,    -1,   101,   102,   103,
      -1,    -1,   106,   107,    -1,    -1,    -1,    -1,   112,   113,
     114,   115,   116,   117,   118,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   128,   129,   130,   131,    -1,   133,
      -1,    -1,    -1,   137,   138,   139,   140,   141,    -1,    -1,
      -1,    -1,    -1,    -1,   148,   149,   150,    -1,   152,   153,
     154,    -1,    -1,    -1,   158,    -1,    -1,   161,   162,    -1,
      -1,    -1,   166,   167,   168,   169,   170,   171,   172,   173,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   181,   182,   183,
      -1,    -1,    -1,   187,   188,   189,   190,   191,    -1,    -1,
      -1,    -1,   196,    -1,   198,    -1,   200,   201,   202,   203,
     204,   205,   206,   207,   208,   209,   210,    -1,    -1,   213,
     214,    -1,    -1,   217,   218,   219,   220,   221,   222,   223,
     224,   225,   226,   227,   228,   229,   230,   231,   232,   233,
     234,   235,   236,   237,   238,   239,   240,   241,   242,   243,
     244,   245,   246,   247,   248,   249,   250,   251,   252,   253,
     254,   255,   256,   257,   258,   259,   260,   261,   262,   263,
      -1,   265,   266,   267,    -1,   269,   270,   271,   272,   273,
     274,   275,   276,   277,   278,   279,   280,   281,   282,   283,
     284,   285,   286,   287,   288,   289,   290,   291,   292,   293,
     294,   295,   296,   297,   298,   299,   300,   301,   302,   303,
     304,   305,   306,   307,   308,   309,   310,   311,   312,   313,
     314,   315,    -1,    -1,    -1,    -1,   320,   321,   322,   323,
     324,   325,   326,    16,    -1,    18,    19,    -1,    -1,    -1,
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
      -1,    -1,    -1,    -1,    -1,   128,   129,   130,   131,    -1,
     133,    -1,    -1,    -1,   137,   138,   139,   140,   141,    -1,
      -1,    -1,    -1,    -1,    -1,   148,   149,   150,    -1,   152,
     153,   154,    -1,    -1,    -1,    -1,    -1,    -1,   161,   162,
      -1,    -1,    -1,   166,   167,   168,   169,   170,   171,   172,
     173,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   181,   182,
     183,    -1,    -1,    -1,   187,    -1,   189,   190,   191,    -1,
      -1,    -1,    -1,    -1,    -1,   198,    -1,   200,   201,   202,
     203,   204,   205,   206,   207,   208,   209,   210,    -1,    -1,
     213,   214,    -1,    -1,   217,   218,   219,   220,   221,   222,
     223,   224,   225,   226,   227,   228,   229,   230,   231,   232,
     233,   234,   235,   236,   237,   238,   239,   240,   241,   242,
     243,   244,   245,   246,   247,   248,   249,   250,   251,   252,
     253,   254,   255,   256,   257,   258,   259,   260,   261,   262,
     263,    -1,   265,   266,   267,    -1,   269,   270,   271,   272,
     273,   274,   275,   276,   277,   278,   279,   280,   281,   282,
     283,   284,   285,   286,   287,   288,   289,   290,   291,   292,
     293,   294,   295,   296,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,   311,   312,
     313,   314,   315,    -1,    -1,    16,    -1,   320,   321,   322,
     323,   324,   325,   326,    25,    26,    -1,    -1,    -1,    -1,
      -1,    -1,    33,    34,    -1,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    -1,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    -1,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    -1,    95,    96,    97,    98,    -1,    -1,
     101,   102,   103,    -1,    -1,   106,   107,    -1,    -1,    -1,
      -1,   112,   113,   114,   115,   116,   117,   118,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   128,   129,   130,
     131,    -1,   133,    -1,    -1,    -1,   137,   138,   139,   140,
     141,    -1,    -1,    -1,    -1,    -1,    -1,   148,   149,   150,
      -1,   152,   153,   154,    -1,    -1,    -1,    -1,    -1,    -1,
     161,   162,    -1,    -1,    -1,   166,   167,   168,   169,   170,
     171,   172,   173,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     181,   182,   183,    -1,    -1,    -1,   187,   188,   189,   190,
     191,    -1,    -1,    -1,    -1,   196,    -1,   198,    -1,   200,
     201,   202,   203,   204,   205,   206,   207,   208,   209,   210,
      -1,    -1,   213,   214,    -1,    -1,   217,   218,   219,   220,
     221,   222,   223,   224,   225,   226,   227,   228,   229,   230,
     231,   232,   233,   234,   235,   236,   237,   238,   239,   240,
     241,   242,   243,   244,   245,   246,   247,   248,   249,   250,
     251,   252,   253,   254,   255,   256,   257,   258,   259,   260,
     261,   262,   263,    -1,   265,   266,   267,    -1,   269,   270,
     271,   272,   273,   274,   275,   276,   277,   278,   279,   280,
     281,   282,   283,   284,   285,   286,   287,   288,   289,   290,
     291,   292,   293,   294,   295,   296,   297,   298,   299,   300,
     301,   302,   303,   304,   305,   306,   307,   308,   309,   310,
     311,   312,   313,   314,   315,    -1,    -1,    16,    -1,   320,
     321,   322,   323,   324,   325,   326,    25,    26,    -1,    -1,
      -1,    30,    -1,    -1,    33,    34,    -1,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    -1,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      -1,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    -1,    95,    96,    97,    98,
      -1,    -1,   101,   102,   103,    -1,    -1,   106,   107,    -1,
      -1,    -1,    -1,   112,   113,   114,   115,   116,   117,   118,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   128,
     129,   130,   131,    -1,   133,    -1,    -1,    -1,   137,   138,
     139,   140,   141,    -1,    -1,    -1,    -1,    -1,    -1,   148,
     149,   150,    -1,   152,   153,   154,    -1,    -1,    -1,    -1,
      -1,    -1,   161,   162,    -1,    -1,    -1,   166,   167,   168,
     169,   170,   171,   172,   173,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   181,   182,   183,    -1,    -1,    -1,   187,   188,
     189,   190,   191,    -1,    -1,    -1,    -1,    -1,    -1,   198,
      -1,   200,   201,   202,   203,   204,   205,   206,   207,   208,
     209,   210,    -1,    -1,   213,   214,    -1,    -1,   217,   218,
     219,   220,   221,   222,   223,   224,   225,   226,   227,   228,
     229,   230,   231,   232,   233,   234,   235,   236,   237,   238,
     239,   240,   241,   242,   243,   244,   245,   246,   247,   248,
     249,   250,   251,   252,   253,   254,   255,   256,   257,   258,
     259,   260,   261,   262,   263,    -1,   265,   266,   267,    -1,
     269,   270,   271,   272,   273,   274,   275,   276,   277,   278,
     279,   280,   281,   282,   283,   284,   285,   286,   287,   288,
     289,   290,   291,   292,   293,   294,   295,   296,   297,   298,
     299,   300,   301,   302,   303,   304,   305,   306,   307,   308,
     309,   310,   311,   312,   313,   314,   315,    -1,    -1,    16,
      -1,   320,   321,   322,   323,   324,   325,   326,    25,    26,
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
      -1,   128,   129,   130,   131,    -1,   133,    -1,    -1,    -1,
     137,   138,   139,   140,   141,    -1,    -1,    -1,    -1,    -1,
      -1,   148,   149,   150,    -1,   152,   153,   154,    -1,    -1,
      -1,    -1,    -1,    -1,   161,   162,    -1,    -1,    -1,   166,
     167,   168,   169,   170,   171,   172,   173,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   181,   182,   183,    -1,    -1,    -1,
     187,   188,   189,   190,   191,    -1,    -1,    -1,    -1,   196,
      -1,   198,    -1,   200,   201,   202,   203,   204,   205,   206,
     207,   208,   209,   210,    -1,    -1,   213,   214,    -1,    -1,
     217,   218,   219,   220,   221,   222,   223,   224,   225,   226,
     227,   228,   229,   230,   231,   232,   233,   234,   235,   236,
     237,   238,   239,   240,   241,   242,   243,   244,   245,   246,
     247,   248,   249,   250,   251,   252,   253,   254,   255,   256,
     257,   258,   259,   260,   261,   262,   263,    -1,   265,   266,
     267,    -1,   269,   270,   271,   272,   273,   274,   275,   276,
     277,   278,   279,   280,   281,   282,   283,   284,   285,   286,
     287,   288,   289,   290,   291,   292,   293,   294,   295,   296,
     297,   298,   299,   300,   301,   302,   303,   304,   305,   306,
     307,   308,   309,   310,   311,   312,   313,   314,   315,    -1,
      -1,    16,    -1,   320,   321,   322,   323,   324,   325,   326,
      25,    26,    -1,    -1,    -1,    -1,    -1,    -1,    33,    34,
      -1,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      -1,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    -1,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    -1,
      95,    96,    97,    98,    -1,    -1,   101,   102,   103,    -1,
      -1,   106,   107,    -1,    -1,    -1,    -1,   112,   113,   114,
     115,   116,   117,   118,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   128,   129,   130,   131,    -1,   133,    -1,
      -1,    -1,   137,   138,   139,   140,   141,    -1,    -1,    -1,
      -1,    -1,    -1,   148,   149,   150,    -1,   152,   153,   154,
      -1,    -1,    -1,   158,    -1,    -1,   161,   162,    -1,    -1,
      -1,   166,   167,   168,   169,   170,   171,   172,   173,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   181,   182,   183,    -1,
      -1,    -1,   187,   188,   189,   190,   191,    -1,    -1,    -1,
      -1,    -1,    -1,   198,    -1,   200,   201,   202,   203,   204,
     205,   206,   207,   208,   209,   210,    -1,    -1,   213,   214,
      -1,    -1,   217,   218,   219,   220,   221,   222,   223,   224,
     225,   226,   227,   228,   229,   230,   231,   232,   233,   234,
     235,   236,   237,   238,   239,   240,   241,   242,   243,   244,
     245,   246,   247,   248,   249,   250,   251,   252,   253,   254,
     255,   256,   257,   258,   259,   260,   261,   262,   263,    -1,
     265,   266,   267,    -1,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,    -1,    -1,    16,    -1,   320,   321,   322,   323,   324,
     325,   326,    25,    26,    -1,    -1,    -1,    -1,    -1,    -1,
      33,    34,    -1,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    -1,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    -1,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    -1,    95,    96,    97,    98,    -1,    -1,   101,   102,
     103,    -1,    -1,   106,   107,    -1,    -1,    -1,    -1,   112,
     113,   114,   115,   116,   117,   118,   119,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   128,   129,   130,   131,    -1,
     133,    -1,    -1,    -1,   137,   138,   139,   140,   141,    -1,
      -1,    -1,    -1,    -1,    -1,   148,   149,   150,    -1,   152,
     153,   154,    -1,    -1,    -1,   158,    -1,    -1,   161,   162,
      -1,    -1,    -1,   166,   167,   168,   169,   170,   171,   172,
     173,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   181,   182,
     183,    -1,    -1,    -1,   187,    -1,   189,   190,   191,    -1,
      -1,    -1,    -1,    -1,    -1,   198,    -1,   200,   201,   202,
     203,   204,   205,   206,   207,   208,   209,   210,    -1,    -1,
     213,   214,    -1,    -1,   217,   218,   219,   220,   221,   222,
     223,   224,   225,   226,   227,   228,   229,   230,   231,   232,
     233,   234,   235,   236,   237,   238,   239,   240,   241,   242,
     243,   244,   245,   246,   247,   248,   249,   250,   251,   252,
     253,   254,   255,   256,   257,   258,   259,   260,   261,   262,
     263,    -1,   265,   266,   267,    -1,   269,   270,   271,   272,
     273,   274,   275,   276,   277,   278,   279,   280,   281,   282,
     283,   284,   285,   286,   287,   288,   289,   290,   291,   292,
     293,   294,   295,   296,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,   311,   312,
     313,   314,   315,    -1,    -1,    16,    -1,   320,   321,   322,
     323,   324,   325,   326,    25,    26,    -1,    -1,    -1,    -1,
      -1,    -1,    33,    34,    -1,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    -1,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    -1,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    -1,    95,    96,    97,    98,    -1,    -1,
     101,   102,   103,    -1,    -1,   106,   107,    -1,    -1,    -1,
      -1,   112,   113,   114,   115,   116,   117,   118,   119,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   128,   129,   130,
     131,    -1,   133,    -1,    -1,    -1,   137,   138,   139,   140,
     141,    -1,    -1,    -1,    -1,    -1,    -1,   148,   149,   150,
      -1,   152,   153,   154,    -1,    -1,    -1,   158,    -1,    -1,
     161,   162,    -1,    -1,    -1,   166,   167,   168,   169,   170,
     171,   172,   173,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     181,   182,   183,    -1,    -1,    -1,   187,    -1,   189,   190,
     191,    -1,    -1,    -1,    -1,    -1,    -1,   198,    -1,   200,
     201,   202,   203,   204,   205,   206,   207,   208,   209,   210,
      -1,    -1,   213,   214,    -1,    -1,   217,   218,   219,   220,
     221,   222,   223,   224,   225,   226,   227,   228,   229,   230,
     231,   232,   233,   234,   235,   236,   237,   238,   239,   240,
     241,   242,   243,   244,   245,   246,   247,   248,   249,   250,
     251,   252,   253,   254,   255,   256,   257,   258,   259,   260,
     261,   262,   263,    -1,   265,   266,   267,    -1,   269,   270,
     271,   272,   273,   274,   275,   276,   277,   278,   279,   280,
     281,   282,   283,   284,   285,   286,   287,   288,   289,   290,
     291,   292,   293,   294,   295,   296,   297,   298,   299,   300,
     301,   302,   303,   304,   305,   306,   307,   308,   309,   310,
     311,   312,   313,   314,   315,    -1,    -1,    16,    -1,   320,
     321,   322,   323,   324,   325,   326,    25,    26,    -1,    -1,
      -1,    -1,    -1,    -1,    33,    34,    -1,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    -1,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      -1,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    -1,    95,    96,    97,    98,
      -1,    -1,   101,   102,   103,    -1,    -1,   106,   107,    -1,
      -1,    -1,    -1,   112,   113,   114,   115,   116,   117,   118,
     119,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   128,
     129,   130,   131,    -1,   133,    -1,    -1,    -1,   137,   138,
     139,   140,   141,    -1,    -1,    -1,    -1,    -1,    -1,   148,
     149,   150,    -1,   152,   153,   154,    -1,    -1,    -1,   158,
      -1,    -1,   161,   162,    -1,    -1,    -1,   166,   167,   168,
     169,   170,   171,   172,   173,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   181,   182,   183,    -1,    -1,    -1,   187,    -1,
     189,   190,   191,    -1,    -1,    -1,    -1,    -1,    -1,   198,
      -1,   200,   201,   202,   203,   204,   205,   206,   207,   208,
     209,   210,    -1,    -1,   213,   214,    -1,    -1,   217,   218,
     219,   220,   221,   222,   223,   224,   225,   226,   227,   228,
     229,   230,   231,   232,   233,   234,   235,   236,   237,   238,
     239,   240,   241,   242,   243,   244,   245,   246,   247,   248,
     249,   250,   251,   252,   253,   254,   255,   256,   257,   258,
     259,   260,   261,   262,   263,    -1,   265,   266,   267,    -1,
     269,   270,   271,   272,   273,   274,   275,   276,   277,   278,
     279,   280,   281,   282,   283,   284,   285,   286,   287,   288,
     289,   290,   291,   292,   293,   294,   295,   296,   297,   298,
     299,   300,   301,   302,   303,   304,   305,   306,   307,   308,
     309,   310,   311,   312,   313,   314,   315,    -1,    -1,    16,
      -1,   320,   321,   322,   323,   324,   325,   326,    25,    26,
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
      -1,   128,   129,   130,   131,    -1,   133,    -1,    -1,    -1,
     137,   138,   139,   140,   141,    -1,    -1,    -1,    -1,    -1,
      -1,   148,   149,   150,    -1,   152,   153,   154,    -1,    -1,
      -1,   158,    -1,    -1,   161,   162,    -1,    -1,    -1,   166,
     167,   168,   169,   170,   171,   172,   173,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   181,   182,   183,    -1,    -1,    -1,
     187,    -1,   189,   190,   191,    -1,    -1,    -1,    -1,    -1,
      -1,   198,    -1,   200,   201,   202,   203,   204,   205,   206,
     207,   208,   209,   210,    -1,    -1,   213,   214,    -1,    -1,
     217,   218,   219,   220,   221,   222,   223,   224,   225,   226,
     227,   228,   229,   230,   231,   232,   233,   234,   235,   236,
     237,   238,   239,   240,   241,   242,   243,   244,   245,   246,
     247,   248,   249,   250,   251,   252,   253,   254,   255,   256,
     257,   258,   259,   260,   261,   262,   263,    -1,   265,   266,
     267,    -1,   269,   270,   271,   272,   273,   274,   275,   276,
     277,   278,   279,   280,   281,   282,   283,   284,   285,   286,
     287,   288,   289,   290,   291,   292,   293,   294,   295,   296,
     297,   298,   299,   300,   301,   302,   303,   304,   305,   306,
     307,   308,   309,   310,   311,   312,   313,   314,   315,    -1,
      -1,    16,    -1,   320,   321,   322,   323,   324,   325,   326,
      25,    26,    -1,    -1,    -1,    -1,    -1,    -1,    33,    34,
      -1,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      -1,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    -1,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    -1,
      95,    96,    97,    98,    -1,    -1,   101,   102,   103,    -1,
      -1,   106,   107,    -1,    -1,    -1,    -1,   112,   113,   114,
     115,   116,   117,   118,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   128,   129,   130,   131,    -1,   133,    -1,
      -1,    -1,   137,   138,   139,   140,   141,    -1,    -1,    -1,
      -1,    -1,    -1,   148,   149,   150,    -1,   152,   153,   154,
      -1,    -1,    -1,   158,    -1,    -1,   161,   162,    -1,    -1,
      -1,   166,   167,   168,   169,   170,   171,   172,   173,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   181,   182,   183,    -1,
      -1,    -1,   187,    -1,   189,   190,   191,    -1,    -1,    -1,
      -1,    -1,    -1,   198,    -1,   200,   201,   202,   203,   204,
     205,   206,   207,   208,   209,   210,    -1,    -1,   213,   214,
      -1,    -1,   217,   218,   219,   220,   221,   222,   223,   224,
     225,   226,   227,   228,   229,   230,   231,   232,   233,   234,
     235,   236,   237,   238,   239,   240,   241,   242,   243,   244,
     245,   246,   247,   248,   249,   250,   251,   252,   253,   254,
     255,   256,   257,   258,   259,   260,   261,   262,   263,    -1,
     265,   266,   267,    -1,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,    -1,    -1,    16,    -1,   320,   321,   322,   323,   324,
     325,   326,    25,    26,    -1,    -1,    -1,    -1,    -1,    -1,
      33,    34,    -1,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    -1,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    -1,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    -1,    95,    96,    97,    98,    -1,    -1,   101,   102,
     103,    -1,    -1,   106,   107,    -1,    -1,    -1,    -1,   112,
     113,   114,   115,   116,   117,   118,   119,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   128,   129,   130,   131,    -1,
     133,    -1,    -1,    -1,   137,   138,   139,   140,   141,    -1,
      -1,    -1,    -1,    -1,    -1,   148,   149,   150,    -1,   152,
     153,   154,    -1,    -1,    -1,    -1,    -1,    -1,   161,   162,
      -1,    -1,    -1,   166,   167,   168,   169,   170,   171,   172,
     173,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   181,   182,
     183,    -1,    -1,    -1,   187,    -1,   189,   190,   191,    -1,
      -1,    -1,    -1,    -1,    -1,   198,    -1,   200,   201,   202,
     203,   204,   205,   206,   207,   208,   209,   210,    -1,    -1,
     213,   214,    -1,    -1,   217,   218,   219,   220,   221,   222,
     223,   224,   225,   226,   227,   228,   229,   230,   231,   232,
     233,   234,   235,   236,   237,   238,   239,   240,   241,   242,
     243,   244,   245,   246,   247,   248,   249,   250,   251,   252,
     253,   254,   255,   256,   257,   258,   259,   260,   261,   262,
     263,    -1,   265,   266,   267,    -1,   269,   270,   271,   272,
     273,   274,   275,   276,   277,   278,   279,   280,   281,   282,
     283,   284,   285,   286,   287,   288,   289,   290,   291,   292,
     293,   294,   295,   296,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,   311,   312,
     313,   314,   315,    -1,    -1,    16,    -1,   320,   321,   322,
     323,   324,   325,   326,    25,    26,    -1,    -1,    -1,    -1,
      -1,    -1,    33,    34,    -1,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    -1,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    -1,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    -1,    95,    96,    97,    98,    -1,    -1,
     101,   102,   103,    -1,    -1,   106,   107,    -1,    -1,    -1,
      -1,   112,   113,   114,   115,   116,   117,   118,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   128,   129,   130,
     131,    -1,   133,    -1,    -1,    -1,   137,   138,   139,   140,
     141,    -1,    -1,    -1,    -1,    -1,    -1,   148,   149,   150,
      -1,   152,   153,   154,    -1,    -1,    -1,    -1,    -1,    -1,
     161,   162,    -1,    -1,    -1,   166,   167,   168,   169,   170,
     171,   172,   173,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     181,   182,   183,    -1,    -1,    -1,   187,    -1,   189,   190,
     191,    -1,    -1,    -1,    -1,    -1,    -1,   198,    -1,   200,
     201,   202,   203,   204,   205,   206,   207,   208,   209,   210,
      -1,    -1,   213,   214,    -1,    -1,   217,   218,   219,   220,
     221,   222,   223,   224,   225,   226,   227,   228,   229,   230,
     231,   232,   233,   234,   235,   236,   237,   238,   239,   240,
     241,   242,   243,   244,   245,   246,   247,   248,   249,   250,
     251,   252,   253,   254,   255,   256,   257,   258,   259,   260,
     261,   262,   263,    -1,   265,   266,   267,    -1,   269,   270,
     271,   272,   273,   274,   275,   276,   277,   278,   279,   280,
     281,   282,   283,   284,   285,   286,   287,   288,   289,   290,
     291,   292,   293,   294,   295,   296,   297,   298,   299,   300,
     301,   302,   303,   304,   305,   306,   307,   308,   309,   310,
     311,   312,   313,   314,   315,    -1,    -1,    -1,    -1,   320,
     321,   322,   323,   324,   325,   326,    21,    22,    23,    24,
      25,    26,    -1,    -1,    -1,    30,    -1,    -1,    33,    34,
      -1,    36,    37,    38,    39,    40,    41,    42,    43,    -1,
      45,    46,    47,    -1,    49,    50,    51,    52,    -1,    54,
      -1,    56,    57,    58,    59,    60,    61,    -1,    -1,    64,
      65,    66,    67,    68,    -1,    70,    71,    72,    -1,    -1,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    -1,
      95,    96,    97,    98,    -1,    -1,   101,   102,   103,    -1,
      -1,   106,   107,    -1,    -1,    -1,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,    -1,    -1,    -1,    -1,
      -1,   126,    -1,   128,   129,   130,   131,    -1,   133,    -1,
      -1,    -1,   137,   138,   139,   140,   141,    -1,    -1,    -1,
      -1,    -1,    -1,   148,   149,   150,   151,   152,   153,    -1,
     155,   156,    -1,   158,   159,    -1,   161,   162,    -1,    -1,
      -1,   166,   167,   168,   169,   170,   171,   172,   173,    -1,
     175,    -1,    -1,    -1,    -1,    -1,   181,   182,   183,    -1,
      -1,    -1,   187,    -1,   189,   190,   191,    -1,    -1,    -1,
      -1,    -1,    -1,   198,    -1,   200,   201,   202,   203,   204,
     205,   206,   207,   208,   209,   210,    -1,    -1,    -1,   214,
     215,    -1,   217,   218,   219,   220,   221,   222,   223,   224,
     225,   226,   227,   228,   229,   230,    -1,   232,   233,   234,
     235,   236,   237,   238,   239,   240,   241,   242,   243,   244,
     245,   246,   247,   248,    -1,   250,   251,   252,   253,   254,
     255,   256,   257,   258,   259,   260,   261,   262,   263,    -1,
     265,   266,   267,    -1,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,    -1,   318,    25,    26,   321,   322,    -1,    -1,
      -1,    -1,    33,    34,    -1,    36,    37,    38,    -1,    40,
      41,    42,    43,    -1,    -1,    46,    47,    -1,    49,    50,
      51,    52,    -1,    54,    -1,    56,    57,    58,    59,    -1,
      61,    -1,    -1,    64,    65,    66,    67,    68,    -1,    70,
      -1,    72,    -1,    -1,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    -1,    95,    96,    97,    -1,    -1,    -1,
     101,   102,   103,    -1,    -1,   106,   107,    -1,    -1,    -1,
      -1,   112,   113,   114,   115,   116,   117,   118,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   128,   129,   130,
     131,    -1,   133,    -1,    -1,    -1,   137,   138,   139,   140,
     141,    -1,    -1,    -1,    -1,    -1,    -1,   148,   149,   150,
      -1,   152,   153,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     161,   162,    -1,    -1,    -1,   166,   167,   168,   169,   170,
     171,   172,   173,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     181,   182,   183,    -1,    -1,    -1,   187,    -1,   189,   190,
     191,    -1,    -1,    -1,    -1,    -1,    -1,   198,    -1,   200,
     201,   202,   203,   204,   205,   206,   207,   208,   209,   210,
      -1,    -1,    -1,   214,    -1,    -1,   217,   218,   219,   220,
     221,   222,   223,   224,   225,   226,   227,   228,   229,   230,
      -1,   232,   233,   234,   235,   236,   237,   238,   239,   240,
     241,   242,   243,   244,   245,   246,   247,   248,    -1,   250,
     251,   252,   253,   254,   255,   256,   257,   258,   259,   260,
     261,   262,   263,    -1,   265,   266,   267,    -1,   269,   270,
     271,   272,   273,   274,   275,   276,   277,   278,   279,   280,
     281,   282,   283,   284,   285,   286,   287,   288,   289,   290,
     291,   292,   293,   294,   295,   296,   297,   298,   299,   300,
     301,   302,   303,   304,   305,   306,   307,   308,   309,   310,
     311,   312,   313,   314,   315,    -1,    -1,    -1,    -1,    -1,
     321,   322
  };

  const short
  jsoniq_parser::yystos_[] =
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
     119,   120,   122,   126,   128,   129,   130,   131,   133,   137,
     138,   139,   140,   141,   148,   149,   150,   151,   152,   153,
     154,   155,   156,   158,   159,   160,   161,   162,   166,   167,
     168,   169,   170,   171,   172,   173,   175,   177,   178,   181,
     182,   183,   187,   189,   190,   191,   192,   193,   194,   196,
     198,   200,   201,   202,   203,   204,   205,   206,   207,   208,
     209,   210,   213,   214,   215,   217,   218,   219,   220,   221,
     222,   223,   224,   225,   226,   227,   228,   229,   230,   231,
     232,   233,   234,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,   245,   246,   247,   248,   249,   250,   251,
     252,   253,   254,   255,   256,   257,   258,   259,   260,   261,
     262,   263,   265,   266,   267,   269,   270,   271,   272,   273,
     274,   275,   276,   277,   278,   279,   280,   281,   282,   283,
     284,   285,   286,   287,   288,   289,   290,   291,   292,   293,
     294,   295,   296,   297,   298,   299,   300,   301,   302,   303,
     304,   305,   306,   307,   308,   309,   310,   311,   312,   313,
     314,   315,   316,   318,   320,   321,   322,   323,   324,   325,
     326,   327,   328,   341,   343,   344,   345,   346,   347,   348,
     349,   350,   351,   352,   353,   354,   355,   356,   357,   358,
     359,   362,   363,   364,   365,   366,   367,   371,   372,   373,
     377,   379,   380,   382,   389,   391,   394,   395,   396,   398,
     399,   400,   401,   402,   404,   405,   407,   408,   409,   410,
     411,   412,   414,   415,   418,   419,   420,   421,   422,   428,
     430,   432,   433,   434,   439,   458,   461,   465,   468,   469,
     475,   476,   477,   478,   479,   481,   482,   484,   485,   486,
     487,   488,   489,   490,   491,   492,   494,   495,   496,   497,
     500,   501,   502,   503,   504,   505,   506,   507,   508,   509,
     510,   511,   512,   513,   514,   515,   516,   517,   520,   521,
     522,   523,   524,   525,   526,   527,   528,   530,   531,   532,
     533,   534,   535,   549,   550,   552,   553,   554,   555,   556,
     557,   558,   559,   567,   568,   569,   570,   571,   572,   573,
     574,   575,   576,   577,   578,   581,   586,   587,   588,   589,
     590,   591,   594,   652,   653,   654,   655,   657,   658,   659,
     660,   661,   664,   665,   666,    33,    34,    49,   218,   397,
     398,   399,   397,   397,   398,   399,   158,   158,    37,    38,
      40,    41,    47,    57,    58,    75,    80,   112,   113,   114,
     162,   226,   267,   293,   301,   309,   378,   379,   383,   384,
     385,   162,   155,   158,   155,   158,   155,   158,     1,   158,
     386,   158,    25,    34,    61,   119,   279,   438,   440,   441,
     158,   155,   158,   158,   158,   119,   155,   158,   158,   158,
      77,   155,   227,   228,   124,   124,   124,   155,   158,    39,
      43,    45,    46,    47,    48,    51,    60,    68,    71,    73,
      74,    76,    91,    92,    98,   106,   113,   115,   116,   137,
     140,   141,   162,   170,   173,   181,   182,   191,   204,   230,
     232,   234,   235,   242,   244,   245,   246,   249,   250,   263,
     266,   321,   514,   665,   124,   119,   406,   124,   124,    39,
      44,    45,    53,    60,    62,    63,    71,    98,   231,   664,
     119,   124,   124,   397,   399,   420,   656,   664,    48,    73,
      74,   119,   155,   186,   250,   419,   421,   432,   188,   419,
     664,   155,   155,   124,   664,    18,    19,   664,   124,   124,
     124,   506,   155,    31,   216,   219,   231,   233,   320,   231,
     233,   320,   231,   320,   231,   236,   119,   247,   247,   248,
     158,   155,   401,     1,   478,   317,   419,   319,   419,   320,
     328,   343,   343,     0,   345,   346,    33,    49,   348,   365,
       1,   192,   342,   192,   342,   113,   380,   400,   419,   108,
     192,   108,   342,   192,    42,    46,    51,    70,   169,   172,
     187,   190,   214,   263,   413,   423,   424,   429,   430,   431,
     446,   447,   451,     3,    67,    72,   119,   425,   168,    93,
     134,   142,   143,   145,   153,   157,   159,   164,   180,   205,
     206,   207,   208,   209,   210,   498,   499,   255,   100,   160,
     177,   201,   118,   148,   161,   196,   203,   212,   138,   152,
      50,   202,   102,   103,   160,   177,   496,   195,   155,   503,
     506,   193,   194,   156,   518,   519,   514,   518,   514,   121,
     156,   158,   518,   158,   147,   158,   185,   185,   185,   185,
     381,   521,   381,    30,   663,   183,   198,   183,   198,   167,
     183,   665,   664,   170,   204,    47,   664,   154,   119,    45,
      47,    80,   107,   169,   664,   227,   228,   229,   252,   625,
     664,   664,   310,   139,   144,   113,   293,   301,   383,   664,
     398,   188,   398,    45,    63,   188,   577,   578,   419,   188,
     196,   664,   435,   436,   664,   119,   188,   387,   388,   155,
     403,   419,     1,   162,   663,   114,   162,   361,   663,   664,
     119,   144,   108,   188,   419,    30,   188,   664,   664,   664,
     459,   460,   664,   398,   188,   419,   419,   579,   664,   398,
     155,   155,   419,   188,   196,   664,   664,   144,   459,   185,
     185,   123,   108,   185,   123,   158,   158,   158,   664,   155,
     186,   424,   188,    35,   537,   538,   539,   419,   419,     8,
     176,    17,   419,   216,    30,   420,   420,    39,    45,    60,
      71,    98,   517,   665,   420,   420,   420,   656,   420,   517,
     420,   237,   592,   593,   664,   192,   192,   420,   419,   399,
     419,   251,   416,   417,   435,   317,   319,   420,   343,   192,
     342,   192,   342,     3,   349,   365,   395,     1,   349,   365,
     395,    33,   366,   395,   366,   395,   406,   342,   406,   420,
     420,   119,   169,   171,   171,   420,   400,   420,   290,   290,
     435,   119,   442,   477,   478,   481,   481,   481,   481,   480,
     481,   481,   481,    71,   482,   486,   486,   485,   487,   487,
     487,   487,   488,   488,   489,   489,   237,    95,    95,    95,
     504,   397,   506,   506,   419,   519,   158,   523,   525,   581,
     664,   186,   146,   188,   420,   529,   188,   151,   188,   529,
     108,   188,   188,   108,   108,   386,    30,   665,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,   368,   369,
     370,    95,   139,   144,   374,   375,   376,   664,   162,   162,
     368,   663,   129,   134,    55,    57,   101,   257,   270,   272,
     283,   284,   285,   287,   289,   626,   627,   628,   629,   630,
     631,   638,   644,   645,   252,    95,   305,   664,   144,   420,
     119,   664,   664,   134,   185,   185,   188,   188,   185,   108,
     188,   108,   188,   108,    36,    95,    97,   149,   437,   438,
     560,   664,    95,   108,   188,   397,   188,   664,    97,    45,
     664,   663,    97,   144,   560,   664,   420,   441,   185,   188,
     188,   188,   188,   108,   189,   149,   560,   185,   188,   188,
     155,   185,   398,   398,   185,   108,   188,   108,   188,   144,
     560,   420,   189,   420,   420,   664,   420,   419,   419,   419,
     664,   538,   539,   132,   199,   185,   185,   185,   133,   192,
      95,   224,   225,   239,    95,   224,   225,   239,   239,   239,
      95,    95,   243,   231,   320,   108,   241,   144,   192,   188,
     419,   185,   515,   598,   417,   239,   365,    33,    33,   192,
     342,   192,   114,   400,   664,   171,   420,   452,   453,   119,
     420,   448,   449,   664,    56,   220,   260,   426,   427,   481,
     155,   158,   265,   502,   581,   599,   602,   603,   604,   605,
     606,   610,   612,   614,   615,    47,   154,   158,   213,   323,
     324,   325,   326,   561,   563,   565,   567,   582,   583,   584,
     585,   662,   664,   561,   493,   566,   664,   493,   185,   186,
     188,   419,   108,   188,   188,   521,   150,   166,   150,   166,
     139,   403,   386,   369,   134,   158,   563,   376,   420,   560,
     663,   663,   130,   131,   663,   283,   284,   285,   289,   664,
     269,   280,   269,   280,    30,   292,    97,   114,   158,   632,
     635,   626,    39,    44,    53,    60,    62,    71,    98,   231,
     323,   324,   325,   390,   567,   662,   233,   305,   314,   420,
     664,    95,   305,   663,   155,   579,   580,   664,   579,   580,
     119,   436,   129,   561,   119,   420,   149,   438,   149,    36,
     149,   437,   438,   560,   561,   388,    95,   185,   200,   134,
     360,   663,   162,   134,    97,   360,   420,   144,   438,   155,
     119,   420,   420,   149,   101,   462,   463,   464,   466,   467,
     101,   470,   471,   472,   473,   398,   185,   185,   155,   579,
     579,   420,   144,   192,   420,   123,   123,   188,   188,   188,
      35,   539,   132,   199,     9,    10,   104,   125,   127,   155,
     197,   534,   536,   547,   548,   551,   155,    30,   238,   240,
     420,   420,   420,   238,   240,   420,   420,   420,   420,   420,
     420,   420,   420,   420,   517,   119,   420,   420,    48,    73,
      74,   250,   400,   421,   432,   251,   595,   596,   155,   212,
     401,   420,   192,   114,   395,   395,   395,   452,    96,   107,
     117,   129,   454,   455,   456,   457,   108,   664,   107,   450,
     108,   149,   560,   260,    54,    59,    78,   119,   437,   443,
     444,   445,   427,   419,   599,   606,   155,   291,   483,   651,
      97,   170,   258,   259,   261,   268,   278,   290,   600,   601,
     620,   621,   622,   623,   646,   649,   264,   266,   607,   625,
     273,   611,   647,   253,   254,   276,   616,   617,   158,   158,
     188,   563,   158,   158,   158,   158,   158,   146,   177,   196,
     562,   146,   188,   146,   420,   139,   403,   581,   375,   292,
      30,    97,   114,   158,   639,    30,   632,   562,   562,   504,
     293,   312,   560,   390,   233,   192,   397,   188,   188,   146,
     188,   188,   436,   149,   437,   664,   420,   149,   420,   129,
     420,   149,   438,   149,   561,   400,   420,   663,   108,   360,
     420,   144,   397,   460,   420,   420,   114,   463,   464,   101,
     424,   114,   464,   467,   119,   474,   561,   101,   114,   471,
     101,   114,   473,   185,   397,   188,   188,   420,   420,   420,
     200,   470,   134,   197,   536,     7,   398,   664,   197,   547,
     398,   192,   239,   239,   239,   239,    97,   243,   243,   593,
     424,   158,   158,   158,   424,   598,   596,   515,   663,   130,
     131,   456,   457,   457,   453,   144,   560,   663,   449,   420,
     149,   119,   119,   420,   664,   445,    78,   185,   188,   599,
     613,   256,   220,   260,   274,   281,   650,    97,   262,   263,
     648,   256,   603,   650,   485,   620,   604,   149,   288,   608,
     609,   648,   292,   619,    79,   618,   188,   196,   561,   564,
     188,   188,   188,   188,   188,   188,   188,    30,   138,   203,
     641,   642,   643,    30,   640,   641,   277,   636,   108,   633,
     171,   664,   263,   504,   185,   420,   149,   420,   149,   437,
     420,   149,   420,   128,   128,    97,   663,   420,   185,   424,
     424,   420,   400,   420,   424,   424,   664,   212,   424,   119,
     474,   119,   424,   119,   474,   424,   185,   114,   539,   664,
     197,   185,   539,   664,   185,   420,   420,   420,   420,   322,
     420,   420,   420,   419,   419,   419,   155,   597,   457,   420,
     144,   420,   664,   664,   444,   420,   185,   487,    52,   131,
     485,   485,   275,   282,   292,   624,   624,   605,   155,   286,
      95,   188,   108,   188,   639,   639,   643,   108,   188,    30,
     637,   648,   634,   635,   188,   392,   393,   504,   119,   231,
     313,   293,   171,   420,   420,   149,   420,   400,   420,   360,
     420,   400,    95,   561,   400,   420,   664,   424,   664,   420,
     664,   424,   400,   119,    94,   184,   540,   539,   664,   199,
     539,   420,   188,   188,   188,   419,   450,   420,    54,   485,
     485,   201,   419,   561,   561,    95,    30,   271,   108,   108,
     457,   560,   664,   119,   231,   664,   392,   420,   474,    95,
     424,    95,   664,     5,   135,   543,   544,   546,   548,    29,
     136,   541,   542,   545,   548,   199,   539,   199,   200,   470,
     185,   450,   119,   485,   185,   561,   635,   393,   457,   311,
     664,   119,   231,   424,   474,   400,   420,   474,   424,    94,
     135,   546,   184,   136,   545,   199,   114,   664,   420,   311,
     664,   119,   400,   420,   424,   424,   119,   299,   311,   664,
     664,   312,   420,   312,   424,   504,   504,   201,   293,   664,
     231,   119,   664,   312,   504
  };

  const short
  jsoniq_parser::yyr1_[] =
  {
       0,   340,   341,   341,   341,   341,   342,   342,   342,   343,
     343,   343,   343,   344,   344,   345,   345,   345,   345,   345,
     345,   345,   345,   346,   346,   346,   346,   347,   348,   348,
     348,   349,   349,   349,   349,   349,   350,   350,   350,   350,
     350,   350,   350,   350,   351,   351,   352,   353,   354,   354,
     355,   355,   356,   356,   357,   357,   357,   357,   358,   358,
     358,   359,   359,   359,   359,   360,   360,   361,   361,   362,
     362,   362,   362,   363,   364,   364,   365,   365,   365,   366,
     366,   366,   366,   366,   366,   366,   366,   367,   367,   368,
     368,   369,   370,   370,   370,   370,   370,   370,   370,   370,
     370,   370,   371,   372,   373,   374,   374,   375,   375,   375,
     376,   377,   377,   377,   378,   378,   378,   378,   379,   379,
     380,   380,   380,   380,   381,   381,   382,   382,   383,   383,
     384,   384,   385,   385,   386,   386,   386,   386,   387,   387,
     388,   388,   389,   389,   389,   389,   390,   390,   390,   390,
     391,   391,   392,   392,   393,   393,   393,   393,   394,   394,
     394,   394,   395,   396,   396,   396,   397,   397,   397,   398,
     398,   399,   399,   399,   400,   400,   400,   400,   400,   400,
     400,   400,   400,   400,   400,   400,   400,   401,   402,   403,
     404,   405,   405,   405,   406,   406,   406,   406,   407,   408,
     409,   410,   411,   411,   412,   413,   414,   415,   416,   416,
     417,   418,   419,   419,   419,   420,   420,   420,   420,   420,
     420,   421,   421,   421,   421,   421,   421,   421,   421,   421,
     421,   421,   421,   422,   423,   424,   424,   425,   425,   426,
     426,   426,   427,   427,   428,   428,   429,   430,   430,   430,
     431,   431,   431,   431,   431,   432,   432,   433,   433,   433,
     433,   434,   434,   435,   435,   435,   436,   436,   436,   436,
     436,   436,   436,   436,   436,   436,   436,   436,   437,   438,
     439,   440,   440,   441,   441,   441,   441,   442,   442,   443,
     443,   443,   444,   444,   444,   445,   445,   445,   446,   447,
     448,   448,   449,   449,   449,   449,   449,   449,   450,   451,
     451,   452,   452,   453,   453,   454,   454,   454,   454,   454,
     454,   454,   455,   455,   456,   456,   457,   458,   458,   459,
     459,   460,   460,   461,   462,   462,   463,   464,   464,   465,
     466,   466,   467,   468,   468,   469,   469,   470,   470,   471,
     471,   472,   472,   473,   473,   474,   474,   475,   476,   476,
     477,   477,   478,   478,   479,   479,   479,   479,   479,   480,
     479,   479,   479,   479,   481,   481,   482,   482,   483,   483,
     484,   484,   485,   485,   485,   486,   486,   486,   486,   486,
     487,   487,   487,   488,   488,   488,   489,   489,   490,   490,
     491,   491,   492,   492,   493,   493,   494,   494,   495,   495,
     495,   495,   496,   496,   496,   497,   497,   498,   498,   498,
     498,   498,   498,   499,   499,   499,   500,   500,   500,   500,
     501,   502,   502,   503,   503,   503,   504,   504,   504,   504,
     505,   506,   506,   506,   507,   507,   508,   508,   508,   508,
     509,   509,   510,   510,   510,   510,   510,   510,   510,   511,
     511,   512,   512,   513,   513,   513,   513,   513,   514,   514,
     515,   515,   516,   516,   516,   516,   517,   517,   517,   517,
     517,   517,   517,   517,   517,   517,   517,   518,   518,   519,
     520,   520,   520,   520,   520,   520,   520,   520,   520,   520,
     520,   520,   520,   520,   521,   521,   522,   522,   522,   523,
     524,   524,   525,   526,   527,   528,   528,   529,   529,   529,
     529,   530,   530,   531,   532,   533,   533,   534,   534,   534,
     535,   535,   535,   535,   535,   535,   536,   536,   537,   537,
     538,   539,   539,   540,   540,   541,   541,   542,   542,   542,
     542,   543,   543,   544,   544,   544,   544,   545,   545,   546,
     546,   547,   547,   547,   547,   548,   548,   548,   548,   549,
     549,   550,   550,   551,   552,   552,   552,   552,   552,   552,
     552,   553,   554,   554,   555,   555,   556,   557,   558,   558,
     559,   559,   560,   561,   561,   561,   561,   562,   562,   562,
     563,   563,   563,   563,   563,   563,   563,   564,   564,   565,
     566,   567,   567,   567,   567,   567,   567,   567,   567,   567,
     567,   568,   569,   570,   570,   570,   571,   572,   573,   574,
     574,   574,   575,   575,   575,   575,   575,   576,   577,   577,
     577,   577,   577,   577,   577,   578,   579,   580,   581,   582,
     582,   583,   584,   584,   585,   586,   586,   586,   587,   587,
     587,   587,   587,   587,   587,   587,   587,   587,   588,   588,
     589,   589,   590,   591,   592,   592,   593,   594,   595,   595,
     596,   597,   598,   598,   599,   600,   600,   601,   601,   602,
     602,   603,   603,   604,   604,   605,   605,   606,   607,   607,
     608,   608,   609,   610,   610,   610,   611,   611,   612,   613,
     613,   614,   615,   615,   616,   616,   617,   617,   617,   618,
     618,   619,   619,   620,   620,   620,   620,   620,   621,   622,
     623,   624,   624,   624,   625,   625,   626,   626,   626,   626,
     626,   626,   626,   626,   627,   627,   627,   627,   628,   628,
     629,   630,   630,   631,   631,   631,   632,   632,   633,   633,
     634,   634,   635,   636,   636,   637,   637,   638,   638,   638,
     639,   639,   640,   640,   641,   641,   642,   642,   643,   643,
     644,   645,   645,   646,   646,   646,   647,   648,   648,   648,
     648,   649,   649,   650,   650,   651,   652,   652,   653,   653,
     654,   654,   655,   656,   656,   656,   656,   657,   657,   657,
     658,   659,   660,   661,   662,   662,   662,   663,   664,   664,
     664,   664,   664,   664,   664,   664,   664,   664,   664,   664,
     664,   664,   664,   664,   664,   664,   664,   664,   664,   664,
     664,   665,   665,   665,   665,   665,   665,   665,   665,   665,
     665,   665,   665,   665,   665,   665,   665,   665,   665,   665,
     665,   665,   665,   665,   665,   665,   665,   665,   665,   665,
     665,   665,   665,   665,   665,   665,   665,   665,   665,   665,
     665,   665,   665,   665,   665,   665,   665,   665,   665,   665,
     665,   665,   665,   665,   665,   665,   665,   665,   665,   665,
     665,   665,   665,   665,   665,   665,   665,   665,   665,   665,
     665,   665,   665,   665,   665,   665,   665,   665,   665,   665,
     665,   665,   665,   665,   665,   665,   665,   665,   665,   665,
     665,   665,   665,   665,   665,   665,   665,   665,   665,   665,
     665,   665,   665,   665,   665,   665,   665,   665,   665,   665,
     665,   665,   665,   665,   665,   665,   665,   665,   665,   665,
     665,   665,   665,   665,   665,   665,   665,   665,   665,   665,
     665,   665,   665,   665,   665,   665,   665,   665,   665,   665,
     665,   665,   665,   665,   665,   665,   665,   665,   665,   665,
     665,   665,   665,   665,   665,   665,   665,   665,   665,   665,
     665,   665,   665,   665,   665,   665,   665,   665,   665,   665,
     665,   665,   665,   665,   665,   665,   665,   665,   665,   665,
     665,   665,   665,   665,   665,   665,   665,   665,   665,   665,
     665,   665,   665,   665,   665,   665,   665,   665,   665,   665,
     665,   665,   665,   665,   665,   665,   665,   666
  };

  const signed char
  jsoniq_parser::yyr2_[] =
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
       1,     1,     1,     1,     1,     1,     1,     3,     3,     3,
       2,     3,     2,     3,     2,     3,     4,     5,     5,     2,
       4,     5,     3,     3,     2,     2,     8,     3,     1,     2,
       3,     1,     1,     3,     3,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     2,     2,     1,     1,     2,     2,     1,
       1,     2,     4,     3,     5,     4,     3,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     2,     3,     3,     3,
       2,     1,     1,     1,     4,     3,     3,     5,     4,     6,
       4,     6,     5,     7,     4,     5,     5,     6,     3,     3,
       2,     1,     3,     4,     5,     3,     6,     4,     5,     1,
       2,     3,     1,     2,     1,     6,     3,     3,     2,     3,
       1,     3,     4,     5,     6,     5,     1,     2,     2,     3,
       4,     1,     3,     1,     2,     1,     1,     1,     2,     2,
       2,     3,     1,     1,     2,     2,     2,     5,     5,     1,
       4,     3,     4,     8,     1,     2,     3,     2,     3,     8,
       1,     2,     3,     8,    10,     8,    10,     1,     2,     4,
       7,     1,     2,     4,     7,     1,     3,     8,     1,     3,
       1,     3,     1,     2,     1,     3,     3,     3,     3,     0,
       4,     3,     3,     3,     1,     5,     1,     3,     0,     1,
       1,     3,     1,     3,     3,     1,     3,     3,     3,     3,
       1,     3,     3,     1,     3,     3,     1,     4,     1,     4,
       1,     4,     1,     4,     1,     2,     1,     2,     1,     1,
       2,     2,     1,     1,     1,     1,     3,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     4,     5,     5,     6,
       4,     1,     2,     3,     2,     2,     1,     2,     2,     1,
       1,     1,     3,     3,     1,     1,     1,     2,     1,     2,
       2,     1,     2,     2,     2,     2,     2,     2,     2,     1,
       2,     2,     1,     2,     2,     2,     2,     2,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     2,     3,     4,
       3,     3,     4,     5,     3,     3,     3,     1,     2,     3,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     2,
       2,     3,     1,     4,     4,     3,     4,     1,     3,     1,
       3,     1,     1,     3,     3,     1,     1,     1,     1,     1,
       4,     8,     5,     9,     9,    10,     1,     2,     1,     2,
       6,     0,     1,     3,     3,     0,     1,     1,     1,     2,
       2,     0,     1,     1,     1,     2,     2,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     3,     3,
       2,     3,     3,     2,     1,     1,     1,     1,     1,     1,
       1,     4,     3,     7,     3,     7,     4,     4,     3,     7,
       3,     7,     2,     1,     2,     3,     2,     1,     1,     1,
       1,     1,     3,     3,     1,     1,     1,     1,     3,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     3,     1,     3,     4,     4,     3,     3,     3,     3,
       4,     4,     3,     4,     6,     4,     6,     4,     3,     4,
       6,     6,     4,     6,     6,     4,     1,     2,     1,     1,
       1,     4,     5,     6,     3,     3,     3,     3,     5,     7,
       7,     5,     5,     5,     7,     7,     5,     5,     3,     3,
       5,     7,     5,     7,     1,     4,     3,     5,     1,     2,
       3,     3,     1,     3,     2,     0,     1,     1,     2,     1,
       3,     1,     3,     1,     4,     1,     2,     3,     0,     1,
       0,     1,     4,     2,     3,     1,     0,     1,     4,     0,
       1,     2,     1,     3,     0,     1,     2,     2,     1,     0,
       1,     0,     1,     1,     1,     1,     1,     1,     1,     3,
       3,     1,     1,     1,     2,     3,     1,     1,     1,     1,
       1,     1,     1,     1,     2,     2,     1,     1,     2,     2,
       3,     1,     2,     2,     5,     2,     1,     1,     0,     2,
       1,     3,     4,     0,     2,     0,     2,     4,     4,     3,
       2,     3,     1,     3,     0,     1,     1,     2,     2,     2,
       2,     1,     2,     2,     2,     2,     3,     2,     3,     3,
       4,     2,     2,     1,     1,     3,     2,     3,     2,     3,
       2,     3,     3,     3,     3,     5,     5,     5,     5,     8,
       5,     3,     5,     7,     3,     3,     3,     1,     1,     1,
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
       1,     1,     1,     1,     1,     1,     1,     1
  };


#if YYDEBUG || 1
  // YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
  // First, the terminals, then, starting at \a YYNTOKENS, nonterminals.
  const char*
  const jsoniq_parser::yytname_[] =
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
  "\"'$_'\"", "\"'.'\"", "\"'..'\"", "\"':'\"", "\"'::'\"", "\"'{{'\"",
  "\"'DOUBLE'\"", "\"'<double {>'\"", "\"'else'\"", "\"'empty'\"",
  "\"'greatest'\"", "\"'least'\"", "\"'/>'\"", "\"'encoding'\"", "\"'='\"",
  "\"''''\"", "\"'\\\"\\\"'\"", "\"'every'\"", "\"'except'\"",
  "\"'external'\"", "\"'following'\"", "\"'following-sibling'\"",
  "\"'follows'\"", "\"'>='\"", "\"':='\"", "\"'>'\"", "\"'?'\"", "\"'#'\"",
  "\"'idiv'\"", "\"'in'\"", "\"'inherit'\"", "\"'INTEGER'\"",
  "\"'intersect'\"", "\"'is'\"", "\"'item'\"", "\"'{'\"", "\"'['\"",
  "\"'<='\"", "\"'('\"", "\"'<'\"", "\"'-'\"", "\"'mod'\"",
  "\"'namespace'\"", "\"'nan'\"", "\"'!='\"", "\"'nodecomp'\"",
  "\"'<no inherit>'\"", "\"'<no preserve>'\"", "\"'or'\"", "\"'order'\"",
  "\"'ordered'\"", "\"'by'\"", "\"'group'\"", "\"'parent'\"", "\"'%'\"",
  "\"'<?'\"", "\"'?>'\"", "\"'+'\"", "\"'(#'\"", "\"'#)'\"", "\"'<<'\"",
  "\"'preceding'\"", "\"'preceding-sibling'\"", "\"'preserve'\"",
  "\"'\\\"'\"", "\"'}'\"", "\"']'\"", "\"'return'\"", "\"')'\"",
  "\"'satisfies'\"", "\"'select'\"", "\"'self'\"", "\"';'\"", "\"'/'\"",
  "\"'//'\"", "\"'!'\"", "\"'*'\"", "\"'</ (start tag end)'\"",
  "\"'strip'\"", "\"'> (tag end)'\"", "\"'then'\"", "\"'to'\"",
  "\"'treat'\"", "\"'union'\"", "\"'unordered'\"", "\"'eq'\"", "\"'ge'\"",
  "\"'gt'\"", "\"'le'\"", "\"'lt'\"", "\"'ne'\"", "\"'VALUECOMP'\"",
  "\"'|'\"", "\"'empty-sequence'\"", "\"'where'\"", "\"'<!--'\"",
  "\"'-->'\"", "\"'xquery'\"", "\"'jsoniq'\"", "\"'version'\"",
  "\"'start'\"", "\"'null'\"", "\"'true'\"", "\"'false'\"", "\"'after'\"",
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
  "STEP_REDUCE", "SIMPLEMAPEXPR_REDUCE", "MULTIPLICATIVE_REDUCE",
  "JSONLOOKUPEXPR_REDUCE", "$accept", "Module", "ERROR",
  "ModuleWithoutBOM", "VersionDecl", "MainModule", "LibraryModule",
  "ModuleDecl", "SIND_DeclList", "SIND_Decl", "Setter",
  "BoundarySpaceDecl", "DefaultCollationDecl", "BaseURIDecl",
  "ConstructionDecl", "OrderingModeDecl", "EmptyOrderDecl",
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
  "OrExpr", "AndExpr", "NotExpr", "ComparisonExpr", "$@1",
  "FTContainsExpr", "StringConcatExpr", "opt_FTIgnoreOption", "RangeExpr",
  "AdditiveExpr", "MultiplicativeExpr", "UnionExpr", "IntersectExceptExpr",
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
  jsoniq_parser::yyrline_[] =
  {
       0,   943,   943,   944,   948,   952,   958,   962,   966,   972,
     978,   986,   992,  1001,  1006,  1012,  1018,  1024,  1032,  1037,
    1047,  1057,  1068,  1080,  1085,  1091,  1097,  1106,  1114,  1121,
    1127,  1138,  1139,  1140,  1141,  1142,  1145,  1146,  1147,  1148,
    1149,  1150,  1151,  1152,  1155,  1160,  1166,  1172,  1178,  1183,
    1189,  1194,  1200,  1205,  1211,  1216,  1221,  1226,  1232,  1234,
    1236,  1244,  1249,  1257,  1265,  1274,  1281,  1289,  1295,  1301,
    1307,  1314,  1322,  1333,  1340,  1347,  1355,  1362,  1368,  1379,
    1380,  1381,  1382,  1383,  1384,  1385,  1386,  1389,  1395,  1402,
    1409,  1417,  1425,  1426,  1427,  1428,  1429,  1430,  1431,  1432,
    1433,  1434,  1437,  1451,  1457,  1463,  1470,  1476,  1483,  1488,
    1494,  1500,  1512,  1524,  1537,  1542,  1550,  1558,  1567,  1572,
    1579,  1584,  1591,  1596,  1604,  1609,  1616,  1622,  1631,  1632,
    1635,  1649,  1662,  1677,  1690,  1695,  1700,  1705,  1711,  1718,
    1726,  1731,  1739,  1747,  1755,  1763,  1773,  1777,  1783,  1787,
    1794,  1803,  1814,  1820,  1827,  1832,  1839,  1846,  1855,  1865,
    1875,  1885,  1900,  1916,  1920,  1925,  1930,  1934,  1939,  1944,
    1949,  1957,  1964,  1971,  1984,  1985,  1986,  1987,  1988,  1989,
    1990,  1991,  1992,  1993,  1994,  1995,  1996,  1999,  2005,  2031,
    2037,  2043,  2050,  2057,  2066,  2075,  2084,  2093,  2104,  2110,
    2116,  2122,  2134,  2139,  2145,  2157,  2170,  2190,  2196,  2203,
    2212,  2219,  2225,  2230,  2242,  2254,  2255,  2256,  2257,  2258,
    2259,  2262,  2263,  2264,  2265,  2266,  2267,  2268,  2269,  2270,
    2271,  2272,  2273,  2276,  2288,  2294,  2295,  2300,  2305,  2311,
    2316,  2321,  2327,  2336,  2346,  2356,  2367,  2373,  2374,  2375,
    2378,  2379,  2380,  2381,  2382,  2385,  2392,  2400,  2404,  2412,
    2420,  2428,  2429,  2434,  2441,  2448,  2459,  2469,  2479,  2489,
    2499,  2509,  2519,  2529,  2539,  2548,  2558,  2568,  2580,  2586,
    2592,  2598,  2604,  2612,  2620,  2628,  2636,  2646,  2652,  2661,
    2662,  2666,  2673,  2677,  2682,  2685,  2689,  2693,  2699,  2705,
    2711,  2717,  2725,  2729,  2737,  2745,  2753,  2761,  2771,  2777,
    2783,  2791,  2797,  2805,  2809,  2817,  2823,  2829,  2835,  2844,
    2853,  2862,  2873,  2877,  2883,  2889,  2897,  2903,  2912,  2923,
    2929,  2937,  2941,  2950,  2956,  2962,  2970,  2976,  2982,  2990,
    2996,  3002,  3010,  3016,  3023,  3033,  3040,  3050,  3056,  3064,
    3070,  3079,  3085,  3093,  3099,  3108,  3114,  3122,  3128,  3132,
    3138,  3143,  3149,  3153,  3164,  3168,  3177,  3183,  3192,  3202,
    3201,  3214,  3223,  3232,  3243,  3247,  3258,  3262,  3268,  3271,
    3277,  3281,  3287,  3291,  3295,  3301,  3305,  3311,  3317,  3323,
    3331,  3335,  3339,  3345,  3349,  3355,  3363,  3367,  3375,  3379,
    3387,  3391,  3399,  3403,  3411,  3415,  3421,  3425,  3431,  3435,
    3439,  3443,  3451,  3452,  3453,  3456,  3461,  3467,  3471,  3475,
    3479,  3483,  3487,  3493,  3497,  3501,  3507,  3511,  3515,  3519,
    3528,  3536,  3542,  3550,  3554,  3558,  3564,  3568,  3576,  3584,
    3622,  3628,  3638,  3642,  3648,  3649,  3652,  3658,  3666,  3672,
    3682,  3688,  3696,  3700,  3704,  3708,  3712,  3718,  3724,  3730,
    3734,  3740,  3744,  3753,  3757,  3761,  3767,  3771,  3779,  3780,
    3783,  3787,  3793,  3797,  3801,  3805,  3811,  3815,  3819,  3823,
    3827,  3831,  3837,  3843,  3849,  3853,  3857,  3863,  3869,  3877,
    3883,  3884,  3885,  3886,  3887,  3888,  3889,  3890,  3891,  3892,
    3893,  3894,  3895,  3896,  3899,  3903,  3909,  3916,  3923,  3932,
    3938,  3942,  3948,  3955,  3961,  3967,  3971,  3989,  3995,  4001,
    4007,  4015,  4019,  4025,  4031,  4041,  4042,  4045,  4046,  4047,
    4050,  4058,  4075,  4083,  4100,  4119,  4140,  4146,  4155,  4161,
    4170,  4177,  4178,  4180,  4185,  4192,  4195,  4201,  4207,  4213,
    4220,  4229,  4232,  4238,  4244,  4250,  4257,  4266,  4270,  4276,
    4280,  4286,  4290,  4294,  4299,  4306,  4310,  4314,  4318,  4324,
    4329,  4335,  4341,  4348,  4354,  4359,  4364,  4369,  4374,  4379,
    4384,  4387,  4393,  4397,  4403,  4407,  4413,  4419,  4425,  4429,
    4435,  4439,  4445,  4451,  4455,  4459,  4463,  4470,  4476,  4482,
    4490,  4529,  4533,  4537,  4541,  4545,  4549,  4555,  4561,  4569,
    4575,  4581,  4585,  4589,  4593,  4597,  4601,  4605,  4609,  4613,
    4617,  4623,  4629,  4635,  4639,  4643,  4649,  4654,  4660,  4666,
    4670,  4675,  4681,  4685,  4691,  4697,  4701,  4709,  4715,  4719,
    4725,  4734,  4743,  4749,  4755,  4763,  4769,  4775,  4781,  4787,
    4791,  4797,  4803,  4807,  4815,  4821,  4827,  4833,  4841,  4845,
    4851,  4857,  4861,  4867,  4871,  4877,  4883,  4890,  4898,  4903,
    4909,  4915,  4923,  4929,  4936,  4942,  4951,  4957,  4963,  4969,
    4978,  4985,  4991,  4997,  5006,  5013,  5017,  5023,  5028,  5035,
    5039,  5045,  5049,  5055,  5059,  5065,  5069,  5077,  5088,  5091,
    5097,  5100,  5106,  5112,  5120,  5124,  5130,  5133,  5139,  5149,
    5152,  5158,  5168,  5174,  5182,  5185,  5191,  5195,  5199,  5205,
    5208,  5214,  5217,  5223,  5227,  5231,  5235,  5239,  5245,  5251,
    5261,  5271,  5275,  5279,  5285,  5291,  5299,  5303,  5307,  5311,
    5315,  5319,  5323,  5327,  5333,  5337,  5341,  5345,  5351,  5357,
    5365,  5373,  5377,  5383,  5393,  5404,  5410,  5414,  5420,  5423,
    5429,  5434,  5441,  5449,  5452,  5458,  5462,  5468,  5477,  5484,
    5492,  5496,  5502,  5508,  5516,  5519,  5525,  5530,  5540,  5548,
    5558,  5564,  5568,  5574,  5578,  5582,  5588,  5594,  5598,  5602,
    5606,  5612,  5620,  5630,  5634,  5640,  5646,  5650,  5656,  5660,
    5666,  5670,  5676,  5683,  5693,  5699,  5706,  5717,  5723,  5733,
    5739,  5745,  5763,  5788,  5808,  5812,  5816,  5822,  5825,  5826,
    5827,  5828,  5829,  5830,  5831,  5832,  5833,  5834,  5835,  5836,
    5837,  5838,  5839,  5840,  5841,  5842,  5843,  5844,  5845,  5846,
    5847,  5850,  5851,  5852,  5853,  5854,  5855,  5856,  5857,  5858,
    5859,  5860,  5861,  5862,  5863,  5864,  5865,  5866,  5867,  5868,
    5869,  5870,  5871,  5872,  5873,  5874,  5875,  5876,  5877,  5878,
    5879,  5880,  5881,  5882,  5883,  5884,  5885,  5886,  5887,  5888,
    5889,  5890,  5891,  5892,  5893,  5894,  5895,  5896,  5897,  5898,
    5899,  5900,  5901,  5902,  5903,  5904,  5905,  5906,  5907,  5908,
    5909,  5910,  5911,  5912,  5913,  5914,  5915,  5916,  5917,  5918,
    5919,  5920,  5921,  5922,  5923,  5924,  5925,  5926,  5927,  5928,
    5929,  5930,  5931,  5932,  5933,  5934,  5935,  5936,  5937,  5938,
    5939,  5940,  5941,  5942,  5943,  5944,  5945,  5946,  5947,  5948,
    5949,  5950,  5951,  5952,  5953,  5954,  5955,  5956,  5957,  5958,
    5959,  5960,  5961,  5962,  5963,  5964,  5965,  5966,  5967,  5968,
    5969,  5970,  5971,  5972,  5973,  5974,  5975,  5976,  5977,  5978,
    5979,  5980,  5981,  5982,  5983,  5984,  5985,  5986,  5987,  5988,
    5989,  5990,  5991,  5992,  5993,  5994,  5995,  5996,  5997,  5998,
    5999,  6000,  6001,  6002,  6003,  6004,  6005,  6006,  6007,  6008,
    6009,  6010,  6011,  6012,  6013,  6014,  6015,  6016,  6017,  6018,
    6019,  6020,  6021,  6022,  6023,  6024,  6025,  6026,  6027,  6028,
    6029,  6030,  6031,  6032,  6033,  6034,  6035,  6036,  6037,  6038,
    6039,  6040,  6041,  6042,  6043,  6044,  6045,  6046,  6047,  6048,
    6049,  6050,  6051,  6052,  6053,  6054,  6055,  6058
  };

  void
  jsoniq_parser::yy_stack_print_ () const
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
  jsoniq_parser::yy_reduce_print_ (int yyrule) const
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

  jsoniq_parser::symbol_kind_type
  jsoniq_parser::yytranslate_ (int t) YY_NOEXCEPT
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
     335,   336,   337,   338,   339
    };
    // Last valid token kind.
    const int code_max = 594;

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
void jsoniq_parser::error(zorba::jsoniq_parser::location_type const& loc, string const& msg)
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
