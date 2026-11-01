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

  case 525: // InlineFunction: AnnotationList "'function'" FunctionSig EnclosedStatementsAndOptionalExpr
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

  case 526: // Constructor: DirectConstructor
    { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 527: // Constructor: ComputedConstructor
  { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 528: // DirectConstructor: DirElemConstructor
    { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 529: // DirectConstructor: DirCommentConstructor
    { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 530: // DirectConstructor: DirPIConstructor
    { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 531: // DirElemConstructor: "'<'" QNAME OptionalBlank "'/>'"
    {
      (yylhs.value.expr) = new DirElemConstructor(LOC(yylhs.location),
                                  static_cast<QName*>((yystack_[2].value.expr)),
                                  NULL,
                                  NULL,
                                  NULL);
    }
    break;

  case 532: // DirElemConstructor: "'<'" QNAME OptionalBlank "'> (tag end)'" "'</ (start tag end)'" QNAME OptionalBlank "'> (tag end)'"
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

  case 533: // DirElemConstructor: "'<'" QNAME DirAttributeList OptionalBlank "'/>'"
    {
      (yylhs.value.expr) = new DirElemConstructor(LOC(yylhs.location),
                                  static_cast<QName*>((yystack_[3].value.expr)),
                                  NULL,
                                  dynamic_cast<DirAttributeList*>((yystack_[2].value.node)),
                                  NULL);
    }
    break;

  case 534: // DirElemConstructor: "'<'" QNAME DirAttributeList OptionalBlank "'> (tag end)'" "'</ (start tag end)'" QNAME OptionalBlank "'> (tag end)'"
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

  case 535: // DirElemConstructor: "'<'" QNAME OptionalBlank "'> (tag end)'" DirElemContentList "'</ (start tag end)'" QNAME OptionalBlank "'> (tag end)'"
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

  case 536: // DirElemConstructor: "'<'" QNAME DirAttributeList OptionalBlank "'> (tag end)'" DirElemContentList "'</ (start tag end)'" QNAME OptionalBlank "'> (tag end)'"
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

  case 537: // DirElemContentList: DirElemContent
    {
      DirElemContentList *decl = new DirElemContentList( LOC(yylhs.location) );
      decl->push_back( dynamic_cast<DirElemContent*>((yystack_[0].value.expr)) );
      (yylhs.value.node) = decl;
    }
    break;

  case 538: // DirElemContentList: DirElemContentList DirElemContent
    {
      DirElemContentList *decl = dynamic_cast<DirElemContentList*>((yystack_[1].value.node));
      if ( decl )
        decl->push_back( dynamic_cast<DirElemContent*>((yystack_[0].value.expr)) );
      (yylhs.value.node) = (yystack_[1].value.node);
    }
    break;

  case 539: // DirAttributeList: DirAttr
    {
      DirAttributeList *dal = new DirAttributeList( LOC(yylhs.location) );
      dal->push_back( dynamic_cast<DirAttr*>((yystack_[0].value.node)) );
      (yylhs.value.node) = dal;
    }
    break;

  case 540: // DirAttributeList: DirAttributeList DirAttr
    {
      DirAttributeList *dal = dynamic_cast<DirAttributeList*>((yystack_[1].value.node));
      if ( dal )
        dal->push_back( dynamic_cast<DirAttr*>((yystack_[0].value.node)) );
      (yylhs.value.node) = (yystack_[1].value.node);
    }
    break;

  case 541: // DirAttr: "'<blank>'" QNAME OptionalBlank "'='" OptionalBlank DirAttributeValue
    {
      (yylhs.value.node) = new DirAttr(LOC(yylhs.location),
                       static_cast<QName*>((yystack_[4].value.expr)),
                       dynamic_cast<DirAttributeValue*>((yystack_[0].value.node)));
    }
    break;

  case 544: // DirAttributeValue: "'\"'" opt_QuoteAttrContentList "'\"'"
    {
      (yylhs.value.node) = new DirAttributeValue(LOC(yylhs.location),
                                 dynamic_cast<QuoteAttrContentList*>((yystack_[1].value.node)));
    }
    break;

  case 545: // DirAttributeValue: "'''" opt_AposAttrContentList "'''"
    {
      (yylhs.value.node) = new DirAttributeValue( LOC(yylhs.location),
                                  dynamic_cast<AposAttrContentList*>((yystack_[1].value.node)));
    }
    break;

  case 546: // opt_QuoteAttrContentList: %empty
    {
      (yylhs.value.node) = new QuoteAttrContentList( LOC(yylhs.location) );
    }
    break;

  case 547: // opt_QuoteAttrContentList: QuoteAttrContentList
    {
      (yylhs.value.node) = (yystack_[0].value.node);
    }
    break;

  case 548: // QuoteAttrContentList: "'\"\"'"
    {
      QuoteAttrContentList *qacl = new QuoteAttrContentList( LOC(yylhs.location) );
      qacl->push_back( new QuoteAttrValueContent( LOC(yylhs.location), "\"" ) );
      (yylhs.value.node) = qacl;
    }
    break;

  case 549: // QuoteAttrContentList: QuoteAttrValueContent
    {
      QuoteAttrContentList *qacl = new QuoteAttrContentList( LOC(yylhs.location) );
      qacl->push_back( dynamic_cast<QuoteAttrValueContent*>((yystack_[0].value.node)) );
      (yylhs.value.node) = qacl;
    }
    break;

  case 550: // QuoteAttrContentList: QuoteAttrContentList "'\"\"'"
    {
      QuoteAttrContentList* qacl = dynamic_cast<QuoteAttrContentList*>((yystack_[1].value.node));
      if ( qacl )
        qacl->push_back( new QuoteAttrValueContent( LOC(yylhs.location), "\"" ) );
      (yylhs.value.node) = (yystack_[1].value.node);
    }
    break;

  case 551: // QuoteAttrContentList: QuoteAttrContentList QuoteAttrValueContent
    {
      QuoteAttrContentList *qacl = dynamic_cast<QuoteAttrContentList*>((yystack_[1].value.node));
      if ( qacl )
        qacl->push_back( dynamic_cast<QuoteAttrValueContent*>((yystack_[0].value.node)) );
      (yylhs.value.node) = (yystack_[1].value.node);
    }
    break;

  case 552: // opt_AposAttrContentList: %empty
    {
      (yylhs.value.node) = new AposAttrContentList( LOC(yylhs.location) );
    }
    break;

  case 553: // opt_AposAttrContentList: AposAttrContentList
    {
      (yylhs.value.node) = (yystack_[0].value.node);
    }
    break;

  case 554: // AposAttrContentList: "''''"
    {
      AposAttrContentList *aacl = new AposAttrContentList( LOC(yylhs.location) );
      aacl->push_back( new AposAttrValueContent( LOC(yylhs.location),"'") );
      (yylhs.value.node) = aacl;
    }
    break;

  case 555: // AposAttrContentList: AposAttrValueContent
    {
      AposAttrContentList *aacl = new AposAttrContentList( LOC(yylhs.location) );
      aacl->push_back( dynamic_cast<AposAttrValueContent*>((yystack_[0].value.node)) );
      (yylhs.value.node) = aacl;
    }
    break;

  case 556: // AposAttrContentList: AposAttrContentList "''''"
    {
      AposAttrContentList *aacl = dynamic_cast<AposAttrContentList*>((yystack_[1].value.node));
      if (aacl)
        aacl->push_back( new AposAttrValueContent( LOC(yylhs.location),"'") );
      (yylhs.value.node) = (yystack_[1].value.node);
    }
    break;

  case 557: // AposAttrContentList: AposAttrContentList AposAttrValueContent
    {
      AposAttrContentList *aacl = dynamic_cast<AposAttrContentList*>((yystack_[1].value.node));
      if ( aacl )
        aacl->push_back( dynamic_cast<AposAttrValueContent*>((yystack_[0].value.node)) );
      (yylhs.value.node) = (yystack_[1].value.node);
    }
    break;

  case 558: // QuoteAttrValueContent: "'quote attribute content'"
    {
      (yylhs.value.node) = new QuoteAttrValueContent( LOC(yylhs.location), SYMTAB((yystack_[0].value.sval)) );
    }
    break;

  case 559: // QuoteAttrValueContent: CommonContent
    {
      (yylhs.value.node) = new QuoteAttrValueContent(LOC(yylhs.location), dynamic_cast<CommonContent*>((yystack_[0].value.expr)));
    }
    break;

  case 560: // AposAttrValueContent: "'apos attribute content'"
    {
      (yylhs.value.node) = new AposAttrValueContent( LOC(yylhs.location), SYMTAB((yystack_[0].value.sval)) );
    }
    break;

  case 561: // AposAttrValueContent: CommonContent
    {
      (yylhs.value.node) = new AposAttrValueContent(LOC(yylhs.location), dynamic_cast<CommonContent*>((yystack_[0].value.expr)));
    }
    break;

  case 562: // DirElemContent: DirectConstructor
    {
      (yylhs.value.expr) = new DirElemContent( LOC(yylhs.location), (yystack_[0].value.expr) );
    }
    break;

  case 563: // DirElemContent: "'element content'"
    {
      (yylhs.value.expr) = new DirElemContent( LOC(yylhs.location), SYMTAB((yystack_[0].value.sval)) );
    }
    break;

  case 564: // DirElemContent: CDataSection
    {
      rchandle<CDataSection> cdata_h = dynamic_cast<CDataSection*>((yystack_[0].value.expr));
      (yylhs.value.expr) = new DirElemContent( LOC(yylhs.location), cdata_h );
    }
    break;

  case 565: // DirElemContent: CommonContent
    {
      rchandle<CommonContent> cont_h = dynamic_cast<CommonContent*>((yystack_[0].value.expr));
      (yylhs.value.expr) = new DirElemContent( LOC(yylhs.location), cont_h );
    }
    break;

  case 566: // CommonContent: "'&#charref;'"
    {
      (yylhs.value.expr) = new CommonContent(LOC(yylhs.location), ParseConstants::cont_charref, SYMTAB((yystack_[0].value.sval)));
    }
    break;

  case 567: // CommonContent: "'{{'"
    {
      (yylhs.value.expr) = new CommonContent(LOC(yylhs.location), ParseConstants::cont_escape_lbrace);
    }
    break;

  case 568: // CommonContent: "'<double {>'"
    {
      (yylhs.value.expr) = new CommonContent(LOC(yylhs.location), ParseConstants::cont_escape_rbrace);
    }
    break;

  case 569: // CommonContent: "'{'" StatementsAndExpr "'}'"
    {
      (yylhs.value.expr) = new CommonContent(LOC(yylhs.location), new EnclosedExpr(LOC(yylhs.location), (yystack_[1].value.expr)));
    }
    break;

  case 570: // DirCommentConstructor: "'<!--'" "'XML comment'" "'-->'"
    {
      (yylhs.value.expr) = new DirCommentConstructor( LOC(yylhs.location), SYMTAB((yystack_[1].value.sval)) );
    }
    break;

  case 571: // DirCommentConstructor: "'<!--'" "'-->'"
    {
      (yylhs.value.expr) = new DirCommentConstructor( LOC(yylhs.location), "" );
    }
    break;

  case 572: // DirPIConstructor: "'<?'" QNAME "'?>'"
    {
      ERROR_IF_QNAME_NOT_NCNAME((yystack_[1].value.expr), yystack_[1].location);
      (yylhs.value.expr) = new DirPIConstructor( LOC(yylhs.location), static_cast<QName*>((yystack_[1].value.expr)) );
    }
    break;

  case 573: // DirPIConstructor: "'<?'" QNAME "'char literal and pi end'"
    {
      ERROR_IF_QNAME_NOT_NCNAME((yystack_[1].value.expr), yystack_[1].location);
      (yylhs.value.expr) = new DirPIConstructor( LOC(yylhs.location), static_cast<QName*>((yystack_[1].value.expr)), SYMTAB((yystack_[0].value.sval)) );
    }
    break;

  case 574: // CDataSection: "'CDATA[['" "'char literal]]>'"
    {
      (yylhs.value.expr) = new CDataSection( LOC(yylhs.location),SYMTAB((yystack_[0].value.sval)) );
    }
    break;

  case 575: // ComputedConstructor: CompDocConstructor
    {
      (yylhs.value.expr) = (yystack_[0].value.expr);
    }
    break;

  case 576: // ComputedConstructor: CompElemConstructor
    {
      (yylhs.value.expr) = (yystack_[0].value.expr);
    }
    break;

  case 577: // ComputedConstructor: CompAttrConstructor
    {
      (yylhs.value.expr) = (yystack_[0].value.expr);
    }
    break;

  case 578: // ComputedConstructor: CompTextConstructor
    {
      (yylhs.value.expr) = (yystack_[0].value.expr);
    }
    break;

  case 579: // ComputedConstructor: CompCommentConstructor
    {
      (yylhs.value.expr) = (yystack_[0].value.expr);
    }
    break;

  case 580: // ComputedConstructor: CompPIConstructor
    {
      (yylhs.value.expr) = (yystack_[0].value.expr);
    }
    break;

  case 581: // ComputedConstructor: CompNamespaceConstructor
    { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 582: // CompDocConstructor: "'document'" "'{'" StatementsAndExpr "'}'"
    {
      (yylhs.value.expr) = new CompDocConstructor( LOC(yylhs.location), (yystack_[1].value.expr) );
    }
    break;

  case 583: // CompElemConstructor: "'element QName {'" StatementsAndOptionalExpr "'}'"
    {
      (yylhs.value.expr) = new CompElemConstructor(LOC(yylhs.location), new QName(LOC(yylhs.location), SYMTAB((yystack_[2].value.sval))), (yystack_[1].value.expr));
    }
    break;

  case 584: // CompElemConstructor: "'element EQName {'" StatementsAndOptionalExpr "'}'"
    {
      (yylhs.value.expr) = new CompElemConstructor(LOC(yylhs.location), new QName(LOC(yylhs.location), SYMTAB((yystack_[2].value.sval)), true), (yystack_[1].value.expr));
    }
    break;

  case 585: // CompElemConstructor: "'element'" "'{'" Expr "'}'" "'{'" StatementsAndOptionalExpr "'}'"
    {
      (yylhs.value.expr) = new CompElemConstructor( LOC(yylhs.location), (yystack_[4].value.expr), (yystack_[1].value.expr) );
    }
    break;

  case 586: // CompAttrConstructor: "'attribute QName {'" StatementsAndOptionalExpr "'}'"
    {
      (yylhs.value.expr) = new CompAttrConstructor( LOC(yylhs.location), new QName(LOC(yylhs.location), SYMTAB((yystack_[2].value.sval))), (yystack_[1].value.expr) );
    }
    break;

  case 587: // CompAttrConstructor: "'attribute EQName {'" StatementsAndOptionalExpr "'}'"
    {
      (yylhs.value.expr) = new CompAttrConstructor( LOC(yylhs.location), new QName(LOC(yylhs.location), SYMTAB((yystack_[2].value.sval)), true), (yystack_[1].value.expr) );
    }
    break;

  case 588: // CompAttrConstructor: "'attribute'" "'{'" Expr "'}'" "'{'" StatementsAndOptionalExpr "'}'"
    {
      (yylhs.value.expr) = new CompAttrConstructor( LOC(yylhs.location), (yystack_[4].value.expr), (yystack_[1].value.expr) );
    }
    break;

  case 589: // CompTextConstructor: "'text'" "'{'" StatementsAndExpr "'}'"
    {
      (yylhs.value.expr) = new CompTextConstructor( LOC(yylhs.location), (yystack_[1].value.expr) );
    }
    break;

  case 590: // CompCommentConstructor: "'comment'" "'{'" StatementsAndExpr "'}'"
    {
      (yylhs.value.expr) = new CompCommentConstructor( LOC(yylhs.location), (yystack_[1].value.expr) );
    }
    break;

  case 591: // CompPIConstructor: "'processing-instruction NCName {'" StatementsAndOptionalExpr "'}'"
    {
      (yylhs.value.expr) = new CompPIConstructor(LOC(yylhs.location), SYMTAB((yystack_[2].value.sval)), (yystack_[1].value.expr));
    }
    break;

  case 592: // CompPIConstructor: "'processing-instruction'" "'{'" Expr "'}'" "'{'" StatementsAndOptionalExpr "'}'"
    {
      (yylhs.value.expr) = new CompPIConstructor(LOC(yylhs.location), (yystack_[4].value.expr), (yystack_[1].value.expr));
    }
    break;

  case 593: // CompNamespaceConstructor: "'namespace NCName {'" StatementsAndExpr "'}'"
    {
      (yylhs.value.expr) = new CompNamespaceConstructor(LOC(yylhs.location), SYMTAB((yystack_[2].value.sval)), (yystack_[1].value.expr));
    }
    break;

  case 594: // CompNamespaceConstructor: "'namespace'" "'{'" Expr "'}'" "'{'" StatementsAndExpr "'}'"
    {
      (yylhs.value.expr) = new CompNamespaceConstructor(LOC(yylhs.location), (yystack_[4].value.expr), (yystack_[1].value.expr));
    }
    break;

  case 595: // TypeDeclaration: "'as'" SequenceType
    {
      (yylhs.value.node) = (yystack_[0].value.node);
    }
    break;

  case 596: // SequenceType: ItemType
        {
            (yylhs.value.node) = new SequenceTypeAST( LOC(yylhs.location), (yystack_[0].value.node), NULL );
        }
    break;

  case 597: // SequenceType: ItemType OccurrenceIndicator
        {
            (yylhs.value.node) = new SequenceTypeAST(LOC(yylhs.location), (yystack_[1].value.node), dynamic_cast<OccurrenceIndicator*>((yystack_[0].value.node)));
        }
    break;

  case 598: // SequenceType: "'empty-sequence'" "'('" "')'"
        {
            (yylhs.value.node) = new SequenceTypeAST( LOC(yylhs.location), NULL, NULL );
        }
    break;

  case 599: // SequenceType: "'('" "')'"
        {
            driver.addCommonLanguageWarning(yystack_[1].location, ZED(ZWST0009_JSONIQ_EMPTY_SEQUENCE));
            (yylhs.value.node) = new SequenceTypeAST( LOC(yylhs.location), NULL, NULL );
        }
    break;

  case 600: // OccurrenceIndicator: "'?'"
        {
            (yylhs.value.node) = new OccurrenceIndicator(
                LOC(yylhs.location), ParseConstants::occurs_optionally
            );
        }
    break;

  case 601: // OccurrenceIndicator: "'*'"
        {
            (yylhs.value.node) = new OccurrenceIndicator(
                LOC(yylhs.location), ParseConstants::occurs_zero_or_more
            );
        }
    break;

  case 602: // OccurrenceIndicator: "'+'"
        {
            (yylhs.value.node) = new OccurrenceIndicator(
                LOC(yylhs.location), ParseConstants::occurs_one_or_more
            );
        }
    break;

  case 603: // ItemType: GeneralizedAtomicType
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

  case 604: // ItemType: KindTest
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 605: // ItemType: "'item'" "'('" "')'"
        {
            (yylhs.value.node) = new ItemType( LOC(yylhs.location), true );
        }
    break;

  case 606: // ItemType: "'structured-item'" "'('" "')'"
        {
            (yylhs.value.node) = new StructuredItemType(LOC(yylhs.location));
        }
    break;

  case 607: // ItemType: FunctionTest
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 608: // ItemType: ParenthesizedItemType
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 609: // ItemType: JSONTest
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 610: // TypeList: SequenceType
        {
          TypeList* aTypeList = new TypeList(LOC (yylhs.location));
          aTypeList->push_back(dynamic_cast<SequenceTypeAST *>((yystack_[0].value.node)));
          (yylhs.value.node) = aTypeList;
        }
    break;

  case 611: // TypeList: TypeList "','" SequenceType
        {
          TypeList* aTypeList = dynamic_cast<TypeList *>((yystack_[2].value.node));
          aTypeList->push_back(dynamic_cast<SequenceTypeAST *>((yystack_[0].value.node)));
          (yylhs.value.node) = (yystack_[2].value.node);
        }
    break;

  case 612: // GeneralizedAtomicType: QNAME
    {
      (yylhs.value.node) = new GeneralizedAtomicType( LOC(yylhs.location), static_cast<QName*>((yystack_[0].value.expr)) );
    }
    break;

  case 613: // SimpleType: QNAME
    {
      (yylhs.value.node) = new SimpleType( LOC(yylhs.location), static_cast<QName*>((yystack_[0].value.expr)) );
    }
    break;

  case 614: // KindTest: DocumentTest
    {
      (yylhs.value.node) = (yystack_[0].value.node);
    }
    break;

  case 615: // KindTest: ElementTest
    {
      (yylhs.value.node) = (yystack_[0].value.node);
    }
    break;

  case 616: // KindTest: AttributeTest
    {
      (yylhs.value.node) = (yystack_[0].value.node);
    }
    break;

  case 617: // KindTest: SchemaElementTest
    {
      (yylhs.value.node) = (yystack_[0].value.node);
    }
    break;

  case 618: // KindTest: SchemaAttributeTest
    {
      (yylhs.value.node) = (yystack_[0].value.node);
    }
    break;

  case 619: // KindTest: PITest
    {
      (yylhs.value.node) = (yystack_[0].value.node);
    }
    break;

  case 620: // KindTest: CommentTest
    {
      (yylhs.value.node) = (yystack_[0].value.node);
    }
    break;

  case 621: // KindTest: TextTest
    {
      (yylhs.value.node) = (yystack_[0].value.node);
    }
    break;

  case 622: // KindTest: NamespaceTest
    {
      (yylhs.value.node) = (yystack_[0].value.node);
    }
    break;

  case 623: // KindTest: AnyKindTest
    {
      (yylhs.value.node) = (yystack_[0].value.node);
    }
    break;

  case 624: // AnyKindTest: NODE_KIND_TEST "'('" "')'"
    {
      (yylhs.value.node) = new AnyKindTest( LOC(yylhs.location) );
    }
    break;

  case 625: // NODE_KIND_TEST: "'node'"
    {
      (yylhs.value.expr) = NULL;
    }
    break;

  case 626: // DocumentTest: "'document-node'" "'('" "')'"
    {
      (yylhs.value.node) = new DocumentTest(LOC(yylhs.location));
    }
    break;

  case 627: // DocumentTest: "'document-node'" "'('" ElementTest "')'"
    {
      (yylhs.value.node) = new DocumentTest(LOC(yylhs.location), dynamic_cast<ElementTest*>((yystack_[1].value.node)));
    }
    break;

  case 628: // DocumentTest: "'document-node'" "'('" SchemaElementTest "')'"
    {
      (yylhs.value.node) = new DocumentTest(LOC(yylhs.location), dynamic_cast<SchemaElementTest*>((yystack_[1].value.node)));
    }
    break;

  case 629: // NamespaceTest: "'namespace-node'" "'('" "')'"
    {
      (yylhs.value.node) = new NamespaceTest(LOC(yylhs.location));
    }
    break;

  case 630: // TextTest: "'text'" "'('" "')'"
    {
      (yylhs.value.node) = new TextTest(LOC(yylhs.location));
    }
    break;

  case 631: // CommentTest: "'comment'" "'('" "')'"
    {
      (yylhs.value.node) = new CommentTest(LOC(yylhs.location));
    }
    break;

  case 632: // PITest: "'processing-instruction'" "'('" "')'"
        {
            (yylhs.value.node) = new PITest( LOC(yylhs.location), "" );
        }
    break;

  case 633: // PITest: "'processing-instruction'" "'('" QNAME "')'"
        {
            ERROR_IF_QNAME_NOT_NCNAME((yystack_[1].value.expr), yystack_[1].location);
            (yylhs.value.node) = new PITest( LOC(yylhs.location), static_cast<QName*>((yystack_[1].value.expr)) );
        }
    break;

  case 634: // PITest: "'processing-instruction'" "'('" "'STRING'" "')'"
        {
            (yylhs.value.node) = new PITest( LOC(yylhs.location), SYMTAB((yystack_[1].value.sval)) );
        }
    break;

  case 635: // AttributeTest: "'attribute'" "'('" "')'"
        {
            (yylhs.value.node) = new AttributeTest( LOC(yylhs.location), NULL, NULL );
        }
    break;

  case 636: // AttributeTest: "'attribute'" "'('" QNAME "')'"
        {
            (yylhs.value.node) = new AttributeTest(
                LOC(yylhs.location), static_cast<QName*>((yystack_[1].value.expr)), NULL
            );
        }
    break;

  case 637: // AttributeTest: "'attribute'" "'('" QNAME "','" TypeName "')'"
        {
            (yylhs.value.node) = new AttributeTest(
                LOC(yylhs.location), static_cast<QName*>((yystack_[3].value.expr)), dynamic_cast<TypeName*>((yystack_[1].value.node))
            );
        }
    break;

  case 638: // AttributeTest: "'attribute'" "'('" "'*'" "')'"
        {
            (yylhs.value.node) = new AttributeTest( LOC(yylhs.location), NULL, NULL );
        }
    break;

  case 639: // AttributeTest: "'attribute'" "'('" "'*'" "','" TypeName "')'"
        {
            (yylhs.value.node) = new AttributeTest(
                LOC(yylhs.location), NULL, dynamic_cast<TypeName*>((yystack_[1].value.node))
            );
        }
    break;

  case 640: // SchemaAttributeTest: "'schema-attribute'" "'('" QNAME "')'"
        {
            (yylhs.value.node) = new SchemaAttributeTest( LOC(yylhs.location), static_cast<QName*>((yystack_[1].value.expr)) );
        }
    break;

  case 641: // ElementTest: "'element'" "'('" "')'"
        {
            (yylhs.value.node) = new ElementTest( LOC(yylhs.location), NULL, NULL, true );
        }
    break;

  case 642: // ElementTest: "'element'" "'('" QNAME "')'"
        {
            (yylhs.value.node) = new ElementTest(
                LOC(yylhs.location), static_cast<QName*>((yystack_[1].value.expr)), NULL, true
            );
        }
    break;

  case 643: // ElementTest: "'element'" "'('" QNAME "','" TypeName "')'"
        {
            (yylhs.value.node) = new ElementTest(
                LOC(yylhs.location),
                static_cast<QName*>((yystack_[3].value.expr)),
                dynamic_cast<TypeName*>((yystack_[1].value.node)),
                false
            );
        }
    break;

  case 644: // ElementTest: "'element'" "'('" QNAME "','" TypeName_WITH_HOOK "')'"
        {
            (yylhs.value.node) = new ElementTest(
                LOC(yylhs.location),
                static_cast<QName*>((yystack_[3].value.expr)),
                dynamic_cast<TypeName*>((yystack_[1].value.node)),
                true
            );
        }
    break;

  case 645: // ElementTest: "'element'" "'('" "'*'" "')'"
        {
            (yylhs.value.node) = new ElementTest(
                LOC(yylhs.location), NULL, NULL, true
            );
        }
    break;

  case 646: // ElementTest: "'element'" "'('" "'*'" "','" TypeName "')'"
        {
            (yylhs.value.node) = new ElementTest(
                LOC(yylhs.location), NULL, dynamic_cast<TypeName*>((yystack_[1].value.node)), false
            );
        }
    break;

  case 647: // ElementTest: "'element'" "'('" "'*'" "','" TypeName_WITH_HOOK "')'"
        {
            (yylhs.value.node) = new ElementTest(
                LOC(yylhs.location), NULL, dynamic_cast<TypeName*>((yystack_[1].value.node)), true
            );
        }
    break;

  case 648: // SchemaElementTest: "'schema-element'" "'('" QNAME "')'"
        {
            (yylhs.value.node) = new SchemaElementTest( LOC(yylhs.location), static_cast<QName*>((yystack_[1].value.expr)) );
        }
    break;

  case 649: // TypeName: QNAME
        {
            (yylhs.value.node) = new TypeName( LOC(yylhs.location), static_cast<QName*>((yystack_[0].value.expr)) );
        }
    break;

  case 650: // TypeName_WITH_HOOK: QNAME "'?'"
        {
            (yylhs.value.node) = new TypeName( LOC(yylhs.location), static_cast<QName*>((yystack_[1].value.expr)), true );
        }
    break;

  case 651: // StringLiteral: "'STRING'"
        {
            (yylhs.value.expr) = new StringLiteral( LOC(yylhs.location), SYMTAB((yystack_[0].value.sval)) );
        }
    break;

  case 652: // FunctionTest: AnyFunctionTest
    {
      (yylhs.value.node) = (yystack_[0].value.node);
    }
    break;

  case 653: // FunctionTest: TypedFunctionTest
    {
      (yylhs.value.node) = (yystack_[0].value.node);
    }
    break;

  case 654: // FunctionTest: AnnotationList AnyFunctionTest
    {
      static_cast<AnyFunctionTest*>((yystack_[0].value.node))->
      setAnnotations(static_cast<AnnotationListParsenode*>((yystack_[1].value.node)));
      (yylhs.value.node) = (yystack_[0].value.node);
    }
    break;

  case 655: // FunctionTest: AnnotationList TypedFunctionTest
    {
      static_cast<TypedFunctionTest*>((yystack_[0].value.node))->
      setAnnotations(static_cast<AnnotationListParsenode*>((yystack_[1].value.node)));
      (yylhs.value.node) = (yystack_[0].value.node);
    }
    break;

  case 656: // AnyFunctionTest: "'function'" "'('" "'*'" "')'"
    {
      (yylhs.value.node) = new AnyFunctionTest(LOC(yylhs.location));
    }
    break;

  case 657: // TypedFunctionTest: "'function'" "'('" "')'" "'as'" SequenceType
        {
          (yylhs.value.node) = new TypedFunctionTest(LOC (yylhs.location), dynamic_cast<SequenceTypeAST *>((yystack_[0].value.node)));
        }
    break;

  case 658: // TypedFunctionTest: "'function'" "'('" TypeList "')'" "'as'" SequenceType
        {
          (yylhs.value.node) = new TypedFunctionTest(LOC (yylhs.location),
              dynamic_cast<TypeList *>((yystack_[3].value.node)),
              dynamic_cast<SequenceTypeAST *>((yystack_[0].value.node)));
        }
    break;

  case 659: // ParenthesizedItemType: "'('" ItemType "')'"
        {
           (yylhs.value.node) = (yystack_[1].value.node);
        }
    break;

  case 660: // RevalidationDecl: "'declare'" "'revalidation'" "'strict'"
        {
            (yylhs.value.node) = new RevalidationDecl(
                LOC(yylhs.location), StaticContextConsts::strict_validation
            );
        }
    break;

  case 661: // RevalidationDecl: "'declare'" "'revalidation'" "'lax'"
        {
            (yylhs.value.node) = new RevalidationDecl(
                LOC(yylhs.location), StaticContextConsts::lax_validation
            );
        }
    break;

  case 662: // RevalidationDecl: "'declare'" "'revalidation'" "'skip'"
        {
            (yylhs.value.node) = new RevalidationDecl(
                LOC(yylhs.location), StaticContextConsts::skip_validation
            );
        }
    break;

  case 663: // InsertExpr: "'insert'" "'node'" ExprSingle "'into'" ExprSingle
        {
            (yylhs.value.expr) = new InsertExpr( LOC(yylhs.location), store::UpdateConsts::INTO, (yystack_[2].value.expr), (yystack_[0].value.expr) );
        }
    break;

  case 664: // InsertExpr: "'insert'" "'node'" ExprSingle "'as'" "'first'" "'into'" ExprSingle
        {
            (yylhs.value.expr) = new InsertExpr(
                LOC(yylhs.location), store::UpdateConsts::AS_FIRST_INTO, (yystack_[4].value.expr), (yystack_[0].value.expr)
            );
        }
    break;

  case 665: // InsertExpr: "'insert'" "'node'" ExprSingle "'as'" "'last'" "'into'" ExprSingle
        {
            (yylhs.value.expr) = new InsertExpr(
                LOC(yylhs.location), store::UpdateConsts::AS_LAST_INTO, (yystack_[4].value.expr), (yystack_[0].value.expr)
            );
        }
    break;

  case 666: // InsertExpr: "'insert'" "'node'" ExprSingle "'after'" ExprSingle
        {
            (yylhs.value.expr) = new InsertExpr( LOC(yylhs.location), store::UpdateConsts::AFTER, (yystack_[2].value.expr), (yystack_[0].value.expr) );
        }
    break;

  case 667: // InsertExpr: "'insert'" "'node'" ExprSingle "'before'" ExprSingle
        {
            (yylhs.value.expr) = new InsertExpr(
                LOC (yylhs.location), store::UpdateConsts::BEFORE, (yystack_[2].value.expr), (yystack_[0].value.expr)
            );
        }
    break;

  case 668: // InsertExpr: "'insert'" "'nodes'" ExprSingle "'into'" ExprSingle
        {
            (yylhs.value.expr) = new InsertExpr( LOC(yylhs.location), store::UpdateConsts::INTO, (yystack_[2].value.expr), (yystack_[0].value.expr) );
        }
    break;

  case 669: // InsertExpr: "'insert'" "'nodes'" ExprSingle "'as'" "'first'" "'into'" ExprSingle
        {
            (yylhs.value.expr) = new InsertExpr(
                LOC(yylhs.location), store::UpdateConsts::AS_FIRST_INTO, (yystack_[4].value.expr), (yystack_[0].value.expr)
            );
        }
    break;

  case 670: // InsertExpr: "'insert'" "'nodes'" ExprSingle "'as'" "'last'" "'into'" ExprSingle
        {
            (yylhs.value.expr) = new InsertExpr(
                  LOC(yylhs.location), store::UpdateConsts::AS_LAST_INTO, (yystack_[4].value.expr), (yystack_[0].value.expr)
            );
        }
    break;

  case 671: // InsertExpr: "'insert'" "'nodes'" ExprSingle "'after'" ExprSingle
        {
            (yylhs.value.expr) = new InsertExpr(
                LOC (yylhs.location),
                store::UpdateConsts::AFTER, (yystack_[2].value.expr), (yystack_[0].value.expr)
            );
        }
    break;

  case 672: // InsertExpr: "'insert'" "'nodes'" ExprSingle "'before'" ExprSingle
        {
            (yylhs.value.expr) = new InsertExpr(
                LOC (yylhs.location), store::UpdateConsts::BEFORE, (yystack_[2].value.expr), (yystack_[0].value.expr)
            );
        }
    break;

  case 673: // DeleteExpr: "'delete'" "'node'" ExprSingle
        {
            (yylhs.value.expr) = new DeleteExpr( LOC (yylhs.location), (yystack_[0].value.expr) );
        }
    break;

  case 674: // DeleteExpr: "'delete'" "'nodes'" ExprSingle
        {
            (yylhs.value.expr) = new DeleteExpr( LOC (yylhs.location), (yystack_[0].value.expr) );
        }
    break;

  case 675: // ReplaceExpr: "'replace'" "'node'" ExprSingle "'with'" ExprSingle
        {
            (yylhs.value.expr) = new ReplaceExpr(
                LOC(yylhs.location), store::UpdateConsts::NODE, (yystack_[2].value.expr), (yystack_[0].value.expr)
            );
        }
    break;

  case 676: // ReplaceExpr: "'replace'" "'value'" "'of'" "'node'" ExprSingle "'with'" ExprSingle
        {
            (yylhs.value.expr) = new ReplaceExpr(
                LOC(yylhs.location), store::UpdateConsts::VALUE_OF_NODE, (yystack_[2].value.expr), (yystack_[0].value.expr)
            );
        }
    break;

  case 677: // RenameExpr: "'rename'" "'node'" ExprSingle "'as'" ExprSingle
        {
            (yylhs.value.expr) = new RenameExpr( LOC (yylhs.location), (yystack_[2].value.expr), (yystack_[0].value.expr) );
        }
    break;

  case 678: // TransformExpr: "'copy'" "'$'" VarNameList "'modify'" ExprSingle ReturnOrSelect ExprSingle
    {
      CopyVarList *cvl = dynamic_cast<CopyVarList*>((yystack_[4].value.expr));
      (yylhs.value.expr) = new TransformExpr( LOC(yylhs.location), cvl, (yystack_[2].value.expr), (yystack_[0].value.expr) );
    }
    break;

  case 679: // VarNameList: VarNameDecl
    {
      CopyVarList* lList = new CopyVarList(LOC(yylhs.location));
      lList->push_back (dynamic_cast<VarBinding*> ((yystack_[0].value.expr)));
      (yylhs.value.expr) = lList;
    }
    break;

  case 680: // VarNameList: VarNameList "','" "'$'" VarNameDecl
    {
      CopyVarList* lList = dynamic_cast<CopyVarList*>((yystack_[3].value.expr));
      VarBinding* lBinding = dynamic_cast<VarBinding*>((yystack_[0].value.expr));
      lList->push_back(lBinding);
      (yylhs.value.expr) = lList;
    }
    break;

  case 681: // VarNameDecl: QNAME "':='" ExprSingle
    {
       (yylhs.value.expr) = new VarBinding(LOC(yylhs.location), static_cast<QName*>((yystack_[2].value.expr)), (yystack_[0].value.expr));
    }
    break;

  case 682: // TryExpr: "'try'" "'{'" Expr "'}'" CatchListExpr
        {
            (yylhs.value.expr) = new TryExpr( LOC(yylhs.location), (yystack_[2].value.expr), (yystack_[0].value.expr) );
        }
    break;

  case 683: // CatchListExpr: CatchExpr
        {
            CatchListExpr *cle = new CatchListExpr( LOC(yylhs.location) );
            cle->push_back( static_cast<CatchExpr*>((yystack_[0].value.expr)) );
            (yylhs.value.expr) = cle;
        }
    break;

  case 684: // CatchListExpr: CatchListExpr CatchExpr
        {
            CatchListExpr *cle = dynamic_cast<CatchListExpr*>((yystack_[1].value.expr));
            if ( cle )
                cle->push_back( static_cast<CatchExpr*>((yystack_[0].value.expr)) );
            (yylhs.value.expr) = (yystack_[1].value.expr);
        }
    break;

  case 685: // CatchExpr: "'catch'" NameTestList BracedExpr
    {
       (yylhs.value.expr) = new CatchExpr(LOC(yylhs.location), *(yystack_[1].value.name_test_list), (yystack_[0].value.expr));
       delete (yystack_[1].value.name_test_list);
    }
    break;

  case 686: // BracedExpr: "'{'" Expr "'}'"
    {
      (yylhs.value.expr) = (yystack_[1].value.expr);
    }
    break;

  case 687: // NameTestList: NameTest
        {
            CatchExpr::NameTestList *ntl = new CatchExpr::NameTestList;
            ntl->push_back( static_cast<NameTest*>((yystack_[0].value.node)) );
            (yylhs.value.name_test_list) = ntl;
        }
    break;

  case 688: // NameTestList: NameTestList "'|'" NameTest
        {
            CatchExpr::NameTestList *ntl =
                static_cast<CatchExpr::NameTestList*>((yystack_[2].value.name_test_list));
            ntl->push_back( static_cast<NameTest*>((yystack_[0].value.node)) );
            (yylhs.value.name_test_list) = ntl;
        }
    break;

  case 689: // FTSelection: FTOr opt_FTPosFilter_list
        {
            (yylhs.value.node) = new FTSelection( LOC(yylhs.location), (yystack_[1].value.node), (yystack_[0].value.pos_filter_list) );
            delete (yystack_[0].value.pos_filter_list);
        }
    break;

  case 690: // opt_FTPosFilter_list: %empty
        {
            (yylhs.value.pos_filter_list) = NULL;
        }
    break;

  case 691: // opt_FTPosFilter_list: FTPosFilter_list
        {
            (yylhs.value.pos_filter_list) = (yystack_[0].value.pos_filter_list);
        }
    break;

  case 692: // FTPosFilter_list: FTPosFilter
        {
            (yylhs.value.pos_filter_list) = new FTSelection::pos_filter_list_t;
            (yylhs.value.pos_filter_list)->push_back( dynamic_cast<FTPosFilter*>((yystack_[0].value.node)) );
        }
    break;

  case 693: // FTPosFilter_list: FTPosFilter_list FTPosFilter
        {
            (yystack_[1].value.pos_filter_list)->push_back( dynamic_cast<FTPosFilter*>((yystack_[0].value.node)) );
            (yylhs.value.pos_filter_list) = (yystack_[1].value.pos_filter_list);
        }
    break;

  case 694: // FTOr: FTAnd
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 695: // FTOr: FTOr "'ftor'" FTAnd
        {
            (yylhs.value.node) = new FTOr( LOC(yylhs.location), (yystack_[2].value.node), (yystack_[0].value.node) );
        }
    break;

  case 696: // FTAnd: FTMildNot
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 697: // FTAnd: FTAnd "'ftand'" FTMildNot
        {
            (yylhs.value.node) = new FTAnd( LOC(yylhs.location), (yystack_[2].value.node), (yystack_[0].value.node) );
        }
    break;

  case 698: // FTMildNot: FTUnaryNot
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 699: // FTMildNot: FTMildNot "'not'" "'in'" FTUnaryNot
        {
            (yylhs.value.node) = new FTMildNot( LOC(yylhs.location), (yystack_[3].value.node), (yystack_[0].value.node) );
        }
    break;

  case 700: // FTUnaryNot: FTPrimaryWithOptions
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 701: // FTUnaryNot: "'ftnot'" FTPrimaryWithOptions
        {
            (yylhs.value.node) = new FTUnaryNot(
                LOC(yylhs.location), dynamic_cast<FTPrimaryWithOptions*>((yystack_[0].value.node))
            );
        }
    break;

  case 702: // FTPrimaryWithOptions: FTPrimary opt_FTMatchOptions opt_FTWeight
        {
            (yylhs.value.node) = new FTPrimaryWithOptions(
                LOC(yylhs.location),
                dynamic_cast<FTPrimary*>((yystack_[2].value.node)),
                dynamic_cast<FTMatchOptions*>((yystack_[1].value.node)),
                dynamic_cast<FTWeight*>((yystack_[0].value.node))
            );
        }
    break;

  case 703: // opt_FTMatchOptions: %empty
        {
            (yylhs.value.node) = NULL;
        }
    break;

  case 704: // opt_FTMatchOptions: FTMatchOptions
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 705: // opt_FTWeight: %empty
        {
            (yylhs.value.node) = NULL;
        }
    break;

  case 706: // opt_FTWeight: FTWeight
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 707: // FTWeight: "'weight'" "'{'" Expr "'}'"
        {
            (yylhs.value.node) = new FTWeight( LOC(yylhs.location), dynamic_cast<exprnode*>((yystack_[1].value.expr)) );
        }
    break;

  case 708: // FTPrimary: FTWords opt_FTTimes
        {
            (yylhs.value.node) = new FTWordsTimes(
                LOC(yylhs.location),
                dynamic_cast<FTWords*>((yystack_[1].value.node)),
                dynamic_cast<FTTimes*>((yystack_[0].value.node))
            );
        }
    break;

  case 709: // FTPrimary: "'('" FTSelection "')'"
        {
            (yylhs.value.node) = (yystack_[1].value.node);
        }
    break;

  case 710: // FTPrimary: FTExtensionSelection
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 711: // opt_FTTimes: %empty
        {
            (yylhs.value.node) = NULL;
        }
    break;

  case 712: // opt_FTTimes: FTTimes
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 713: // FTExtensionSelection: Pragma_list "'{'" opt_FTSelection "'}'"
        {
            (yylhs.value.node) = new FTExtensionSelection(
                LOC(yylhs.location),
                dynamic_cast<PragmaList*>((yystack_[3].value.node)),
                dynamic_cast<FTSelection*>((yystack_[1].value.node))
            );
        }
    break;

  case 714: // opt_FTSelection: %empty
        {
            (yylhs.value.node) = NULL;
        }
    break;

  case 715: // opt_FTSelection: FTSelection
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 716: // FTWords: FTWordsValue opt_FTAnyallOption
        {
            (yylhs.value.node) = new FTWords(
                LOC(yylhs.location),
                dynamic_cast<FTWordsValue*>((yystack_[1].value.node)),
                dynamic_cast<FTAnyallOption*>((yystack_[0].value.node))
            );
        }
    break;

  case 717: // FTWordsValue: StringLiteral
        {
            (yylhs.value.node) = new FTWordsValue(
                LOC(yylhs.location), static_cast<StringLiteral*>((yystack_[0].value.expr)), NULL
            );
        }
    break;

  case 718: // FTWordsValue: "'{'" Expr "'}'"
        {
            (yylhs.value.node) = new FTWordsValue(
                LOC(yylhs.location), NULL, dynamic_cast<exprnode*>((yystack_[1].value.expr))
            );
        }
    break;

  case 719: // opt_FTAnyallOption: %empty
        {
            (yylhs.value.node) = new FTAnyallOption( LOC(yylhs.location), ft_anyall_mode::any );
        }
    break;

  case 720: // opt_FTAnyallOption: FTAnyallOption
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 721: // FTAnyallOption: "'any'" opt_word
        {
            (yylhs.value.node) = new FTAnyallOption( LOC(yylhs.location), (yystack_[0].value.ft_anyall_value) );
        }
    break;

  case 722: // FTAnyallOption: "'all'" opt_words
        {
            (yylhs.value.node) = new FTAnyallOption( LOC(yylhs.location), (yystack_[0].value.ft_anyall_value) );
        }
    break;

  case 723: // FTAnyallOption: "'phrase'"
        {
            (yylhs.value.node) = new FTAnyallOption( LOC(yylhs.location), ft_anyall_mode::phrase );
        }
    break;

  case 724: // opt_word: %empty
        {
            (yylhs.value.ft_anyall_value) = ft_anyall_mode::any;
        }
    break;

  case 725: // opt_word: "'word'"
        {
            (yylhs.value.ft_anyall_value) = ft_anyall_mode::any_word;
        }
    break;

  case 726: // opt_words: %empty
        {
            (yylhs.value.ft_anyall_value) = ft_anyall_mode::all;
        }
    break;

  case 727: // opt_words: "'words'"
        {
            (yylhs.value.ft_anyall_value) = ft_anyall_mode::all_words;
        }
    break;

  case 728: // FTPosFilter: FTOrder
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 729: // FTPosFilter: FTWindow
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 730: // FTPosFilter: FTDistance
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 731: // FTPosFilter: FTScope
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 732: // FTPosFilter: FTContent
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 733: // FTOrder: "'ordered'"
        {
            (yylhs.value.node) = new FTOrder( LOC(yylhs.location) );
        }
    break;

  case 734: // FTWindow: "'window'" AdditiveExpr FTUnit
        {
            (yylhs.value.node) = new FTWindow(
                LOC(yylhs.location),
                static_cast<AdditiveExpr*>((yystack_[1].value.expr)),
                static_cast<FTUnit*>((yystack_[0].value.node))
            );
        }
    break;

  case 735: // FTDistance: "'distance'" FTRange FTUnit
        {
            (yylhs.value.node) = new FTDistance(
                LOC(yylhs.location),
                dynamic_cast<FTRange*>((yystack_[1].value.node)),
                dynamic_cast<FTUnit*>((yystack_[0].value.node))
            );
        }
    break;

  case 736: // FTUnit: "'words'"
        {
            (yylhs.value.node) = new FTUnit( LOC(yylhs.location), ft_unit::words );
        }
    break;

  case 737: // FTUnit: "'sentences'"
        {
            (yylhs.value.node) = new FTUnit( LOC(yylhs.location), ft_unit::sentences );
        }
    break;

  case 738: // FTUnit: "'paragraphs'"
        {
            (yylhs.value.node) = new FTUnit( LOC(yylhs.location), ft_unit::paragraphs );
        }
    break;

  case 739: // FTMatchOptions: "'using'" FTMatchOption
        {
            FTMatchOptions *mo = new FTMatchOptions( LOC(yylhs.location) );
            mo->push_back( dynamic_cast<FTMatchOption*>((yystack_[0].value.node)) );
            (yylhs.value.node) = mo;
        }
    break;

  case 740: // FTMatchOptions: FTMatchOptions "'using'" FTMatchOption
        {
            FTMatchOptions *mo = dynamic_cast<FTMatchOptions*>((yystack_[2].value.node));
            mo->push_back( dynamic_cast<FTMatchOption*>((yystack_[0].value.node)) );
            (yylhs.value.node) = (yystack_[2].value.node);
        }
    break;

  case 741: // FTMatchOption: FTCaseOption
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 742: // FTMatchOption: FTDiacriticsOption
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 743: // FTMatchOption: FTExtensionOption
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 744: // FTMatchOption: FTLanguageOption
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 745: // FTMatchOption: FTStemOption
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 746: // FTMatchOption: FTStopWordOption
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 747: // FTMatchOption: FTThesaurusOption
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 748: // FTMatchOption: FTWildCardOption
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 749: // FTCaseOption: "'case'" "'sensitive'"
        {
            (yylhs.value.node) = new FTCaseOption( LOC(yylhs.location), ft_case_mode::sensitive );
        }
    break;

  case 750: // FTCaseOption: "'case'" "'insensitive'"
        {
            (yylhs.value.node) = new FTCaseOption( LOC(yylhs.location), ft_case_mode::insensitive );
        }
    break;

  case 751: // FTCaseOption: "'lowercase'"
        {
            (yylhs.value.node) = new FTCaseOption( LOC(yylhs.location), ft_case_mode::lower );
        }
    break;

  case 752: // FTCaseOption: "'uppercase'"
        {
            (yylhs.value.node) = new FTCaseOption( LOC(yylhs.location), ft_case_mode::upper );
        }
    break;

  case 753: // FTDiacriticsOption: "'diacritics'" "'sensitive'"
        {
            (yylhs.value.node) = new FTDiacriticsOption(
                LOC(yylhs.location), ft_diacritics_mode::sensitive
            );
        }
    break;

  case 754: // FTDiacriticsOption: "'diacritics'" "'insensitive'"
        {
            (yylhs.value.node) = new FTDiacriticsOption(
                LOC(yylhs.location), ft_diacritics_mode::insensitive
            );
        }
    break;

  case 755: // FTExtensionOption: "'option'" QNAME "'STRING'"
        {
            (yylhs.value.node) = new FTExtensionOption(
                LOC(yylhs.location), static_cast<QName*>((yystack_[1].value.expr)), SYMTAB((yystack_[0].value.sval))
            );
        }
    break;

  case 756: // FTStemOption: "'stemming'"
        {
            (yylhs.value.node) = new FTStemOption( LOC(yylhs.location), ft_stem_mode::stemming );
        }
    break;

  case 757: // FTStemOption: "'no'" "'stemming'"
        {
            (yylhs.value.node) = new FTStemOption( LOC(yylhs.location), ft_stem_mode::no_stemming );
        }
    break;

  case 758: // FTThesaurusOption: "'thesaurus'" FTThesaurusID_or_default
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

  case 759: // FTThesaurusOption: "'thesaurus'" "'('" FTThesaurusID_or_default opt_FTThesaurus_list "')'"
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

  case 760: // FTThesaurusOption: "'no'" "'thesaurus'"
        {
            (yylhs.value.node) = new FTThesaurusOption( LOC(yylhs.location), NULL, false, true );
        }
    break;

  case 761: // FTThesaurusID_or_default: FTThesaurusID
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 762: // FTThesaurusID_or_default: "'default'"
        {
            (yylhs.value.node) = NULL;
        }
    break;

  case 763: // opt_FTThesaurus_list: %empty
        {
            (yylhs.value.thesaurus_id_list) = NULL;
        }
    break;

  case 764: // opt_FTThesaurus_list: "','" FTThesaurus_list
        {
            (yylhs.value.thesaurus_id_list) = (yystack_[0].value.thesaurus_id_list);
        }
    break;

  case 765: // FTThesaurus_list: FTThesaurusID
        {
            (yylhs.value.thesaurus_id_list) = new FTThesaurusOption::thesaurus_id_list_t;
            (yylhs.value.thesaurus_id_list)->push_back( dynamic_cast<FTThesaurusID*>((yystack_[0].value.node)) );
        }
    break;

  case 766: // FTThesaurus_list: FTThesaurus_list "','" FTThesaurusID
        {
            (yystack_[2].value.thesaurus_id_list)->push_back( dynamic_cast<FTThesaurusID*>((yystack_[0].value.node)) );
            (yylhs.value.thesaurus_id_list) = (yystack_[2].value.thesaurus_id_list);
        }
    break;

  case 767: // FTThesaurusID: "'at'" "'STRING'" opt_relationship opt_levels
        {
            (yylhs.value.node) = new FTThesaurusID(
                LOC(yylhs.location), SYMTAB((yystack_[2].value.sval)), SYMTAB((yystack_[1].value.sval)), dynamic_cast<FTRange*>((yystack_[0].value.node))
            );
        }
    break;

  case 768: // opt_relationship: %empty
        {
            (yylhs.value.sval) = 0;
        }
    break;

  case 769: // opt_relationship: "'relationship'" "'STRING'"
        {
            (yylhs.value.sval) = (yystack_[0].value.sval);
        }
    break;

  case 770: // opt_levels: %empty
        {
            (yylhs.value.node) = NULL;
        }
    break;

  case 771: // opt_levels: FTRange "'levels'"
        {
            (yylhs.value.node) = (yystack_[1].value.node);
        }
    break;

  case 772: // FTStopWordOption: "'stop'" "'words'" FTStopWords opt_FTStopWordsInclExcl_list
        {
            (yylhs.value.node) = new FTStopWordOption(
                LOC(yylhs.location),
                dynamic_cast<FTStopWords*>((yystack_[1].value.node)), (yystack_[0].value.incl_excl_list),
                ft_stop_words_mode::with
            );
            delete (yystack_[0].value.incl_excl_list);
        }
    break;

  case 773: // FTStopWordOption: "'stop'" "'words'" "'default'" opt_FTStopWordsInclExcl_list
        {
            (yylhs.value.node) = new FTStopWordOption(
                LOC(yylhs.location), NULL, (yystack_[0].value.incl_excl_list), ft_stop_words_mode::with_default
            );
            delete (yystack_[0].value.incl_excl_list);
        }
    break;

  case 774: // FTStopWordOption: "'no'" "'stop'" "'words'"
        {
            (yylhs.value.node) = new FTStopWordOption(
                LOC(yylhs.location), NULL, NULL, ft_stop_words_mode::without
            );
        }
    break;

  case 775: // FTStopWords: "'at'" "'STRING'"
        {
            (yylhs.value.node) = new FTStopWords( LOC(yylhs.location), SYMTAB((yystack_[0].value.sval)), NULL );
        }
    break;

  case 776: // FTStopWords: "'('" STRING_LITERAL_list "')'"
        {
            (yylhs.value.node) = new FTStopWords( LOC(yylhs.location), "", (yystack_[1].value.strlist) );
        }
    break;

  case 777: // STRING_LITERAL_list: "'STRING'"
        {
            string_list_t *sl = new string_list_t;
            sl->push_back( SYMTAB((yystack_[0].value.sval)) );
            (yylhs.value.strlist) = sl;
        }
    break;

  case 778: // STRING_LITERAL_list: STRING_LITERAL_list "','" "'STRING'"
        {
            if ( (yystack_[2].value.strlist) )
                (yystack_[2].value.strlist)->push_back( SYMTAB((yystack_[0].value.sval)) );
            (yylhs.value.strlist) = (yystack_[2].value.strlist);
        }
    break;

  case 779: // opt_FTStopWordsInclExcl_list: %empty
        {
            (yylhs.value.incl_excl_list) = NULL;
        }
    break;

  case 780: // opt_FTStopWordsInclExcl_list: FTStopWordsInclExcl_list
        {
            (yylhs.value.incl_excl_list) = (yystack_[0].value.incl_excl_list);
        }
    break;

  case 781: // FTStopWordsInclExcl_list: FTStopWordsInclExcl
        {
            (yylhs.value.incl_excl_list) = new FTStopWordOption::incl_excl_list_t;
            (yylhs.value.incl_excl_list)->push_back( dynamic_cast<FTStopWordsInclExcl*>((yystack_[0].value.node)) );
        }
    break;

  case 782: // FTStopWordsInclExcl_list: FTStopWordsInclExcl_list FTStopWordsInclExcl
        {
            FTStopWordOption::incl_excl_list_t *iel = (yystack_[1].value.incl_excl_list);
            if ( !iel )
                iel = new FTStopWordOption::incl_excl_list_t;
            iel->push_back( dynamic_cast<FTStopWordsInclExcl*>((yystack_[0].value.node)) );
            (yylhs.value.incl_excl_list) = iel;
        }
    break;

  case 783: // FTStopWordsInclExcl: "'union'" FTStopWords
        {
            (yylhs.value.node) = new FTStopWordsInclExcl(
                LOC(yylhs.location),
                dynamic_cast<FTStopWords*>((yystack_[0].value.node)),
                ft_stop_words_unex::union_
            );
        }
    break;

  case 784: // FTStopWordsInclExcl: "'except'" FTStopWords
        {
            (yylhs.value.node) = new FTStopWordsInclExcl(
                LOC(yylhs.location),
                dynamic_cast<FTStopWords*>((yystack_[0].value.node)),
                ft_stop_words_unex::except
            );
        }
    break;

  case 785: // FTLanguageOption: "'language'" "'STRING'"
        {
            (yylhs.value.node) = new FTLanguageOption( LOC(yylhs.location), SYMTAB((yystack_[0].value.sval)) );
        }
    break;

  case 786: // FTWildCardOption: "'wildcards'"
        {
            (yylhs.value.node) = new FTWildCardOption( LOC(yylhs.location), ft_wild_card_mode::with );
        }
    break;

  case 787: // FTWildCardOption: "'no'" "'wildcards'"
        {
            (yylhs.value.node) = new FTWildCardOption( LOC(yylhs.location), ft_wild_card_mode::without );
        }
    break;

  case 788: // FTContent: "'at'" "'start'"
        {
            (yylhs.value.node) = new FTContent( LOC(yylhs.location), ft_content_mode::at_start );
        }
    break;

  case 789: // FTContent: "'at'" "'end'"
        {
            (yylhs.value.node) = new FTContent( LOC(yylhs.location), ft_content_mode::at_end );
        }
    break;

  case 790: // FTContent: "'entire'" "'content'"
        {
            (yylhs.value.node) = new FTContent( LOC(yylhs.location), ft_content_mode::entire );
        }
    break;

  case 791: // FTTimes: "'occurs'" FTRange "'times'"
        {
            (yylhs.value.node) = new FTTimes( LOC(yylhs.location), dynamic_cast<FTRange*>((yystack_[1].value.node)) );
        }
    break;

  case 792: // FTRange: "'exactly'" AdditiveExpr
        {
            (yylhs.value.node) = new FTRange( LOC(yylhs.location), ft_range_mode::exactly, (yystack_[0].value.expr) );
        }
    break;

  case 793: // FTRange: "'at'" "'least'" AdditiveExpr
        {
            (yylhs.value.node) = new FTRange( LOC(yylhs.location), ft_range_mode::at_least, (yystack_[0].value.expr) );
        }
    break;

  case 794: // FTRange: "'at'" "'most'" AdditiveExpr
        {
            (yylhs.value.node) = new FTRange( LOC(yylhs.location), ft_range_mode::at_most, (yystack_[0].value.expr) );
        }
    break;

  case 795: // FTRange: "'from'" AdditiveExpr "'to'" AdditiveExpr
        {
            (yylhs.value.node) = new FTRange( LOC(yylhs.location), ft_range_mode::from_to, (yystack_[2].value.expr), (yystack_[0].value.expr) );
        }
    break;

  case 796: // FTScope: "'same'" FTBigUnit
        {
            (yylhs.value.node) = new FTScope(
                LOC(yylhs.location),
                ft_scope::same,
                dynamic_cast<FTBigUnit*>((yystack_[0].value.node))
            );
        }
    break;

  case 797: // FTScope: "'different'" FTBigUnit
        {
            (yylhs.value.node) = new FTScope(
                LOC(yylhs.location),
                ft_scope::different,
                dynamic_cast<FTBigUnit*>((yystack_[0].value.node))
            );
        }
    break;

  case 798: // FTBigUnit: "'sentence'"
        {
            (yylhs.value.node) = new FTBigUnit( LOC(yylhs.location), ft_big_unit::sentence );
        }
    break;

  case 799: // FTBigUnit: "'paragraph'"
        {
            (yylhs.value.node) = new FTBigUnit( LOC(yylhs.location), ft_big_unit::paragraph );
        }
    break;

  case 800: // FTIgnoreOption: "'without'" "'content'" UnionExpr
        {
            (yylhs.value.node) = new FTIgnoreOption( LOC(yylhs.location), static_cast<UnionExpr*>((yystack_[0].value.expr)) );
        }
    break;

  case 801: // JSONArrayConstructor: "'['" "']'"
        {
          (yylhs.value.expr) = new JSONArrayConstructor( LOC(yylhs.location), NULL );
        }
    break;

  case 802: // JSONArrayConstructor: "'['" Expr "']'"
        {
          (yylhs.value.expr) = new JSONArrayConstructor( LOC(yylhs.location), (yystack_[1].value.expr) );
        }
    break;

  case 803: // JSONSimpleObjectUnion: "'{|'" "'|}'"
        {
          (yylhs.value.expr) = new JSONObjectConstructor(LOC(yylhs.location), NULL, false);
        }
    break;

  case 804: // JSONSimpleObjectUnion: "'{|'" Expr "'|}'"
        {
          (yylhs.value.expr) = new JSONObjectConstructor(LOC(yylhs.location), (yystack_[1].value.expr), false);
        }
    break;

  case 805: // JSONAccumulatorObjectUnion: "'{['" "']}'"
        {
          (yylhs.value.expr) = new JSONObjectConstructor(LOC(yylhs.location), NULL, true);
        }
    break;

  case 806: // JSONAccumulatorObjectUnion: "'{['" Expr "']}'"
        {
          (yylhs.value.expr) = new JSONObjectConstructor(LOC(yylhs.location), (yystack_[1].value.expr), true);
        }
    break;

  case 807: // JSONObjectConstructor: "'{'" JSONPairList "'}'"
        {
          (yylhs.value.expr) = new JSONDirectObjectConstructor(LOC(yylhs.location),
                                               dynamic_cast<JSONPairList*>((yystack_[1].value.node)));
        }
    break;

  case 808: // JSONPairList: QNAME "':'" ExprSingle
        {
          ERROR_IF_QNAME_NOT_NCNAME((yystack_[2].value.expr), yystack_[2].location, (yystack_[0].value.expr));
          driver.addCommonLanguageWarning(yystack_[2].location, ZED(ZWST0009_OBJECT_KEY_NOT_QUOTED));
          StringLiteral* sl = new StringLiteral( LOC(yylhs.location), static_cast<QName*>((yystack_[2].value.expr)) );
          JSONPairList* jpl = new JSONPairList(LOC(yylhs.location));
          jpl->push_back(new JSONPairConstructor(LOC(yylhs.location), sl, (yystack_[0].value.expr)));
          (yylhs.value.node) = jpl;
        }
    break;

  case 809: // JSONPairList: ExprSingle "':'" ExprSingle
        {
          JSONPairList* jpl = new JSONPairList(LOC(yylhs.location));
          jpl->push_back(new JSONPairConstructor(LOC(yylhs.location), (yystack_[2].value.expr), (yystack_[0].value.expr)));
          (yylhs.value.node) = jpl;
        }
    break;

  case 810: // JSONPairList: JSONPairList "','" ExprSingle "':'" ExprSingle
        {
          JSONPairList* jpl = dynamic_cast<JSONPairList*>((yystack_[4].value.node));
          assert(jpl);
          jpl->push_back(new JSONPairConstructor(LOC(yylhs.location), (yystack_[2].value.expr), (yystack_[0].value.expr)));
          (yylhs.value.node) = jpl;
        }
    break;

  case 811: // JSONPairList: JSONPairList "','" QNAME "':'" ExprSingle
        {
          ERROR_IF_QNAME_NOT_NCNAME((yystack_[2].value.expr), yystack_[2].location, (yystack_[4].value.node), (yystack_[0].value.expr));
          JSONPairList* jpl = dynamic_cast<JSONPairList*>((yystack_[4].value.node));
          assert(jpl);
          StringLiteral* sl = new StringLiteral( LOC(yylhs.location), static_cast<QName*>((yystack_[2].value.expr)) );
          jpl->push_back(new JSONPairConstructor(LOC(yylhs.location), sl, (yystack_[0].value.expr)));
          (yylhs.value.node) = jpl;
        }
    break;

  case 812: // JSONInsertExpr: "'insert'" "'json'" ExprSingle "'into'" ExprSingle
        {
          (yylhs.value.expr) = new JSONObjectInsertExpr(LOC(yylhs.location),
                                        (yystack_[2].value.expr),
                                        (yystack_[0].value.expr));
        }
    break;

  case 813: // JSONInsertExpr: "'insert'" "'json'" JSONPairList "'into'" ExprSingle
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

  case 814: // JSONInsertExpr: "'insert'" "'json'" ExprSingle "'into'" ExprSingle "'at'" "'position'" ExprSingle
        {
          (yylhs.value.expr) = new JSONArrayInsertExpr(LOC(yylhs.location), (yystack_[5].value.expr), (yystack_[3].value.expr), (yystack_[0].value.expr));
        }
    break;

  case 815: // JSONAppendExpr: "'append'" "'json'" ExprSingle "'into'" ExprSingle
        {
          (yylhs.value.expr) = new JSONArrayAppendExpr(LOC(yylhs.location), (yystack_[2].value.expr), (yystack_[0].value.expr));
        }
    break;

  case 816: // JSONDeleteExpr: "'delete'" "'json'" PostfixExpr
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

  case 817: // JSONRenameExpr: "'rename'" "'json'" PostfixExpr "'as'" ExprSingle
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

  case 818: // JSONReplaceExpr: "'replace'" "'value'" "'of'" "'json'" PostfixExpr "'with'" ExprSingle
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

  case 819: // JSONTest: "'json-item'" "'('" "')'"
        {
          (yylhs.value.node) = new JSON_Test(LOC(yylhs.location), store::StoreConsts::jsonItem);
        }
    break;

  case 820: // JSONTest: "'array'" "'('" "')'"
        {
          (yylhs.value.node) = new JSON_Test(LOC(yylhs.location), store::StoreConsts::jsonArray);
        }
    break;

  case 821: // JSONTest: "'object'" "'('" "')'"
        {
          (yylhs.value.node) = new JSON_Test(LOC(yylhs.location), store::StoreConsts::jsonObject);
        }
    break;

  case 822: // URI_LITERAL: "'STRING'"
        { (yylhs.value.sval) = (yystack_[0].value.sval); }
    break;

  case 823: // QNAME: FUNCTION_NAME
        { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 824: // QNAME: "'NCName_sval'"
                  { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB((yystack_[0].value.sval))); }
    break;

  case 825: // QNAME: "'attribute'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("attribute"))); }
    break;

  case 826: // QNAME: "'comment'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("comment"))); }
    break;

  case 827: // QNAME: "'document-node'"
                    { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("document-node"))); }
    break;

  case 828: // QNAME: "'namespace-node'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("namespace-node"))); }
    break;

  case 829: // QNAME: "'element'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("element"))); }
    break;

  case 830: // QNAME: "'item'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("item"))); }
    break;

  case 831: // QNAME: "'if'"
         { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("if"))); }
    break;

  case 832: // QNAME: "'node'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("node"))); }
    break;

  case 833: // QNAME: "'processing-instruction'"
                             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("processing-instruction"))); }
    break;

  case 834: // QNAME: "'schema-attribute'"
                       { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("schema-attribute"))); }
    break;

  case 835: // QNAME: "'schema-element'"
                     { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("schema-element"))); }
    break;

  case 836: // QNAME: "'text'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("text"))); }
    break;

  case 837: // QNAME: "'typeswitch'"
                 { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("typeswitch"))); }
    break;

  case 838: // QNAME: "'switch'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("switch"))); }
    break;

  case 839: // QNAME: "'empty-sequence'"
                     { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("empty-sequence"))); }
    break;

  case 840: // QNAME: "'while'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("while"))); }
    break;

  case 841: // QNAME: "'json'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("json"))); }
    break;

  case 842: // QNAME: "'array'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("array"))); }
    break;

  case 843: // QNAME: "'object'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("object"))); }
    break;

  case 844: // QNAME: "'json-item'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("json-item"))); }
    break;

  case 845: // QNAME: "'structured-item'"
                      { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("structured-item"))); }
    break;

  case 846: // FUNCTION_NAME: EQNAME
        { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 847: // FUNCTION_NAME: "'QName'"
                 { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB((yystack_[0].value.sval))); }
    break;

  case 848: // FUNCTION_NAME: "'xquery'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("xquery"))); }
    break;

  case 849: // FUNCTION_NAME: "'empty'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("empty"))); }
    break;

  case 850: // FUNCTION_NAME: "'boundary-space'"
                     { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("boundary-space"))); }
    break;

  case 851: // FUNCTION_NAME: "'ft-option'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("ft-option"))); }
    break;

  case 852: // FUNCTION_NAME: "'base-uri'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("base-uri"))); }
    break;

  case 853: // FUNCTION_NAME: "'lax'"
          { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("lax"))); }
    break;

  case 854: // FUNCTION_NAME: "'strict'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("strict"))); }
    break;

  case 855: // FUNCTION_NAME: "'idiv'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("idiv"))); }
    break;

  case 856: // FUNCTION_NAME: "'document'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("document"))); }
    break;

  case 857: // FUNCTION_NAME: "'ftnot'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("ftnot"))); }
    break;

  case 858: // FUNCTION_NAME: "'not'"
          { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("not"))); }
    break;

  case 859: // FUNCTION_NAME: "'sensitive'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("sensitive"))); }
    break;

  case 860: // FUNCTION_NAME: "'insensitive'"
                  { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("insensitive"))); }
    break;

  case 861: // FUNCTION_NAME: "'diacritics'"
                 { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("diacritics"))); }
    break;

  case 862: // FUNCTION_NAME: "'without'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("without"))); }
    break;

  case 863: // FUNCTION_NAME: "'stemming'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("stemming"))); }
    break;

  case 864: // FUNCTION_NAME: "'thesaurus'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("thesaurus"))); }
    break;

  case 865: // FUNCTION_NAME: "'stop'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("stop"))); }
    break;

  case 866: // FUNCTION_NAME: "'wildcards'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("wildcards"))); }
    break;

  case 867: // FUNCTION_NAME: "'entire'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("entire"))); }
    break;

  case 868: // FUNCTION_NAME: "'content'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("content"))); }
    break;

  case 869: // FUNCTION_NAME: "'word'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("word"))); }
    break;

  case 870: // FUNCTION_NAME: "'start'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("start"))); }
    break;

  case 871: // FUNCTION_NAME: "'end'"
          { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("end"))); }
    break;

  case 872: // FUNCTION_NAME: "'most'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("most"))); }
    break;

  case 873: // FUNCTION_NAME: "'skip'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("skip"))); }
    break;

  case 874: // FUNCTION_NAME: "'copy'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("copy"))); }
    break;

  case 875: // FUNCTION_NAME: "'general'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("general"))); }
    break;

  case 876: // FUNCTION_NAME: "'value'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("value"))); }
    break;

  case 877: // FUNCTION_NAME: "'eq'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("eq"))); }
    break;

  case 878: // FUNCTION_NAME: "'ne'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("ne"))); }
    break;

  case 879: // FUNCTION_NAME: "'lt'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("lt"))); }
    break;

  case 880: // FUNCTION_NAME: "'le'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("le"))); }
    break;

  case 881: // FUNCTION_NAME: "'gt'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("gt"))); }
    break;

  case 882: // FUNCTION_NAME: "'ge'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("ge"))); }
    break;

  case 883: // FUNCTION_NAME: "'at'"
         { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("at"))); }
    break;

  case 884: // FUNCTION_NAME: "'context'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("context"))); }
    break;

  case 885: // FUNCTION_NAME: "'variable'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("variable"))); }
    break;

  case 886: // FUNCTION_NAME: "'return'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("return"))); }
    break;

  case 887: // FUNCTION_NAME: "'for'"
          { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("for"))); }
    break;

  case 888: // FUNCTION_NAME: "'from'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("from"))); }
    break;

  case 889: // FUNCTION_NAME: "'allowing'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("allowing"))); }
    break;

  case 890: // FUNCTION_NAME: "'sliding'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("sliding"))); }
    break;

  case 891: // FUNCTION_NAME: "'tumbling'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("tumbling"))); }
    break;

  case 892: // FUNCTION_NAME: "'previous'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("previous"))); }
    break;

  case 893: // FUNCTION_NAME: "'next'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("next"))); }
    break;

  case 894: // FUNCTION_NAME: "'only'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("only"))); }
    break;

  case 895: // FUNCTION_NAME: "'when'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("when"))); }
    break;

  case 896: // FUNCTION_NAME: "'count'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("count"))); }
    break;

  case 897: // FUNCTION_NAME: "'in'"
          { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("in"))); }
    break;

  case 898: // FUNCTION_NAME: "'let'"
          { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("let"))); }
    break;

  case 899: // FUNCTION_NAME: "'where'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("where"))); }
    break;

  case 900: // FUNCTION_NAME: "'by'"
         { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("by"))); }
    break;

  case 901: // FUNCTION_NAME: "'group'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("group"))); }
    break;

  case 902: // FUNCTION_NAME: "'order'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("order"))); }
    break;

  case 903: // FUNCTION_NAME: "'stable'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("stable"))); }
    break;

  case 904: // FUNCTION_NAME: "'ascending'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("ascending"))); }
    break;

  case 905: // FUNCTION_NAME: "'descending'"
                 { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("descending"))); }
    break;

  case 906: // FUNCTION_NAME: "'greatest'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("greatest"))); }
    break;

  case 907: // FUNCTION_NAME: "'least'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("least"))); }
    break;

  case 908: // FUNCTION_NAME: "'collation'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("collation"))); }
    break;

  case 909: // FUNCTION_NAME: "'some'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("some"))); }
    break;

  case 910: // FUNCTION_NAME: "'every'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("every"))); }
    break;

  case 911: // FUNCTION_NAME: "'satisfies'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("satisfies"))); }
    break;

  case 912: // FUNCTION_NAME: "'case'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("case"))); }
    break;

  case 913: // FUNCTION_NAME: "'as'"
         { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("as"))); }
    break;

  case 914: // FUNCTION_NAME: "'then'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("then"))); }
    break;

  case 915: // FUNCTION_NAME: "'else'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("else"))); }
    break;

  case 916: // FUNCTION_NAME: "'or'"
         { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("or"))); }
    break;

  case 917: // FUNCTION_NAME: "'and'"
          { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("and"))); }
    break;

  case 918: // FUNCTION_NAME: "'instance'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("instance"))); }
    break;

  case 919: // FUNCTION_NAME: "'of'"
         { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("of"))); }
    break;

  case 920: // FUNCTION_NAME: "'castable'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("castable"))); }
    break;

  case 921: // FUNCTION_NAME: "'to'"
         { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("to"))); }
    break;

  case 922: // FUNCTION_NAME: "'div'"
          { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("div"))); }
    break;

  case 923: // FUNCTION_NAME: "'mod'"
          { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("mod"))); }
    break;

  case 924: // FUNCTION_NAME: "'union'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("union"))); }
    break;

  case 925: // FUNCTION_NAME: "'intersect'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("intersect"))); }
    break;

  case 926: // FUNCTION_NAME: "'except'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("except"))); }
    break;

  case 927: // FUNCTION_NAME: "'validate'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("validate"))); }
    break;

  case 928: // FUNCTION_NAME: "'type'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("type"))); }
    break;

  case 929: // FUNCTION_NAME: "'cast'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("cast"))); }
    break;

  case 930: // FUNCTION_NAME: "'treat'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("treat"))); }
    break;

  case 931: // FUNCTION_NAME: "'is'"
         { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("is"))); }
    break;

  case 932: // FUNCTION_NAME: "'preserve'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("preserve"))); }
    break;

  case 933: // FUNCTION_NAME: "'strip'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("strip"))); }
    break;

  case 934: // FUNCTION_NAME: "'namespace'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("namespace"))); }
    break;

  case 935: // FUNCTION_NAME: "'external'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("external"))); }
    break;

  case 936: // FUNCTION_NAME: "'encoding'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("encoding"))); }
    break;

  case 937: // FUNCTION_NAME: "'<no preserve>'"
                  { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("no-preserve"))); }
    break;

  case 938: // FUNCTION_NAME: "'inherit'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("inherit"))); }
    break;

  case 939: // FUNCTION_NAME: "'<no inherit>'"
                 { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("no-inherit"))); }
    break;

  case 940: // FUNCTION_NAME: "'declare'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("declare"))); }
    break;

  case 941: // FUNCTION_NAME: "'construction'"
                   { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("construction"))); }
    break;

  case 942: // FUNCTION_NAME: "'ordering'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("ordering"))); }
    break;

  case 943: // FUNCTION_NAME: "'default'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("default"))); }
    break;

  case 944: // FUNCTION_NAME: "'copy-namespaces'"
                      { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("copy-namespaces"))); }
    break;

  case 945: // FUNCTION_NAME: "'option'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("option"))); }
    break;

  case 946: // FUNCTION_NAME: "'version'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("version"))); }
    break;

  case 947: // FUNCTION_NAME: "'import'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("import"))); }
    break;

  case 948: // FUNCTION_NAME: "'schema'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("schema"))); }
    break;

  case 949: // FUNCTION_NAME: "'module'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("module"))); }
    break;

  case 950: // FUNCTION_NAME: "'function'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("function"))); }
    break;

  case 951: // FUNCTION_NAME: "'score'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("score"))); }
    break;

  case 952: // FUNCTION_NAME: "'contains'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("contains"))); }
    break;

  case 953: // FUNCTION_NAME: "'weight'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("weight"))); }
    break;

  case 954: // FUNCTION_NAME: "'window'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("window"))); }
    break;

  case 955: // FUNCTION_NAME: "'distance'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("distance"))); }
    break;

  case 956: // FUNCTION_NAME: "'occurs'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("occurs"))); }
    break;

  case 957: // FUNCTION_NAME: "'times'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("times"))); }
    break;

  case 958: // FUNCTION_NAME: "'same'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("same"))); }
    break;

  case 959: // FUNCTION_NAME: "'different'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("different"))); }
    break;

  case 960: // FUNCTION_NAME: "'lowercase'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("lowercase"))); }
    break;

  case 961: // FUNCTION_NAME: "'uppercase'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("uppercase"))); }
    break;

  case 962: // FUNCTION_NAME: "'relationship'"
                   { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("relationship"))); }
    break;

  case 963: // FUNCTION_NAME: "'levels'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("levels"))); }
    break;

  case 964: // FUNCTION_NAME: "'language'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("language"))); }
    break;

  case 965: // FUNCTION_NAME: "'any'"
          { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("any"))); }
    break;

  case 966: // FUNCTION_NAME: "'all'"
          { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("all"))); }
    break;

  case 967: // FUNCTION_NAME: "'exactly'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("exactly"))); }
    break;

  case 968: // FUNCTION_NAME: "'words'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("words"))); }
    break;

  case 969: // FUNCTION_NAME: "'sentences'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("sentences"))); }
    break;

  case 970: // FUNCTION_NAME: "'sentence'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("sentence"))); }
    break;

  case 971: // FUNCTION_NAME: "'phrase'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("phrase"))); }
    break;

  case 972: // FUNCTION_NAME: "'paragraph'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("paragraph"))); }
    break;

  case 973: // FUNCTION_NAME: "'paragraphs'"
                 { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("paragraphs"))); }
    break;

  case 974: // FUNCTION_NAME: "'modify'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("modify"))); }
    break;

  case 975: // FUNCTION_NAME: "'first'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("first"))); }
    break;

  case 976: // FUNCTION_NAME: "'append'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("append"))); }
    break;

  case 977: // FUNCTION_NAME: "'insert'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("insert"))); }
    break;

  case 978: // FUNCTION_NAME: "'replace'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("replace"))); }
    break;

  case 979: // FUNCTION_NAME: "'rename'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("rename"))); }
    break;

  case 980: // FUNCTION_NAME: "'delete'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("delete"))); }
    break;

  case 981: // FUNCTION_NAME: "'before'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("before"))); }
    break;

  case 982: // FUNCTION_NAME: "'after'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("after"))); }
    break;

  case 983: // FUNCTION_NAME: "'revalidation'"
                   { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("revalidation"))); }
    break;

  case 984: // FUNCTION_NAME: "'with'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("with"))); }
    break;

  case 985: // FUNCTION_NAME: "'nodes'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("nodes"))); }
    break;

  case 986: // FUNCTION_NAME: "'last'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("last"))); }
    break;

  case 987: // FUNCTION_NAME: "'into'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("into"))); }
    break;

  case 988: // FUNCTION_NAME: "'simple'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("simple"))); }
    break;

  case 989: // FUNCTION_NAME: "'sequential'"
                 { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("sequential"))); }
    break;

  case 990: // FUNCTION_NAME: "'updating'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("updating"))); }
    break;

  case 991: // FUNCTION_NAME: "'ordered'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("ordered"))); }
    break;

  case 992: // FUNCTION_NAME: "'unordered'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("unordered"))); }
    break;

  case 993: // FUNCTION_NAME: "'returning'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("returning"))); }
    break;

  case 994: // FUNCTION_NAME: "'exit'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("exit"))); }
    break;

  case 995: // FUNCTION_NAME: "'loop'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("loop"))); }
    break;

  case 996: // FUNCTION_NAME: "'break'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("break"))); }
    break;

  case 997: // FUNCTION_NAME: "'continue'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("continue"))); }
    break;

  case 998: // FUNCTION_NAME: "'try'"
          { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("try"))); }
    break;

  case 999: // FUNCTION_NAME: "'catch'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("catch"))); }
    break;

  case 1000: // FUNCTION_NAME: "'using'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("using"))); }
    break;

  case 1001: // FUNCTION_NAME: "'set'"
          { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("set"))); }
    break;

  case 1002: // FUNCTION_NAME: "'index'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("index"))); }
    break;

  case 1003: // FUNCTION_NAME: "'unique'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("unique"))); }
    break;

  case 1004: // FUNCTION_NAME: "'non'"
          { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("non"))); }
    break;

  case 1005: // FUNCTION_NAME: "'on'"
         { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("on"))); }
    break;

  case 1006: // FUNCTION_NAME: "'range'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("range"))); }
    break;

  case 1007: // FUNCTION_NAME: "'equality'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("equality"))); }
    break;

  case 1008: // FUNCTION_NAME: "'manually'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("manually"))); }
    break;

  case 1009: // FUNCTION_NAME: "'automatically'"
                    { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("automatically"))); }
    break;

  case 1010: // FUNCTION_NAME: "'maintained'"
                 { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("maintained"))); }
    break;

  case 1011: // FUNCTION_NAME: "'decimal-format'"
                     { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("decimal-format"))); }
    break;

  case 1012: // FUNCTION_NAME: "'decimal-separator'"
                        { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("decimal-separator"))); }
    break;

  case 1013: // FUNCTION_NAME: "'grouping-separator'"
                         { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("grouping-separator"))); }
    break;

  case 1014: // FUNCTION_NAME: "'infinity'"
                     { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("infinity"))); }
    break;

  case 1015: // FUNCTION_NAME: "'minus-sign'"
                 { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("minus-sign"))); }
    break;

  case 1016: // FUNCTION_NAME: "'NaN'"
          { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("NaN"))); }
    break;

  case 1017: // FUNCTION_NAME: "'percent'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("percent"))); }
    break;

  case 1018: // FUNCTION_NAME: "'per-mille'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("per-mille"))); }
    break;

  case 1019: // FUNCTION_NAME: "'zero-digit'"
                 { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("zero-digit"))); }
    break;

  case 1020: // FUNCTION_NAME: "'digit'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("digit"))); }
    break;

  case 1021: // FUNCTION_NAME: "'pattern-separator'"
                        { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("pattern-separator"))); }
    break;

  case 1022: // FUNCTION_NAME: "'collection'"
                 { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("collection"))); }
    break;

  case 1023: // FUNCTION_NAME: "'const'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("const"))); }
    break;

  case 1024: // FUNCTION_NAME: "'append-only'"
                  { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("append-only"))); }
    break;

  case 1025: // FUNCTION_NAME: "'queue'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("queue"))); }
    break;

  case 1026: // FUNCTION_NAME: "'mutable'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("mutable"))); }
    break;

  case 1027: // FUNCTION_NAME: "'read-only'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("read-only"))); }
    break;

  case 1028: // FUNCTION_NAME: "'integrity'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("integrity"))); }
    break;

  case 1029: // FUNCTION_NAME: "'constraint'"
                 { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("constraint"))); }
    break;

  case 1030: // FUNCTION_NAME: "'check'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("check"))); }
    break;

  case 1031: // FUNCTION_NAME: "'key'"
          { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("key"))); }
    break;

  case 1032: // FUNCTION_NAME: "'foreach'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("foreach"))); }
    break;

  case 1033: // FUNCTION_NAME: "'foreign'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("foreign"))); }
    break;

  case 1034: // FUNCTION_NAME: "'keys'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("keys"))); }
    break;

  case 1035: // FUNCTION_NAME: "'ancestor'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("ancestor"))); }
    break;

  case 1036: // FUNCTION_NAME: "'child'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("child"))); }
    break;

  case 1037: // FUNCTION_NAME: "'descendant'"
                 { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("descendant"))); }
    break;

  case 1038: // FUNCTION_NAME: "'parent'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("parent"))); }
    break;

  case 1039: // FUNCTION_NAME: "'preceding'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("preceding"))); }
    break;

  case 1040: // FUNCTION_NAME: "'self'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("self"))); }
    break;

  case 1041: // FUNCTION_NAME: "'following'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("following"))); }
    break;

  case 1042: // FUNCTION_NAME: "'ancestor-or-self'"
                       { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("ancestor-or-self"))); }
    break;

  case 1043: // FUNCTION_NAME: "'descendant-or-self'"
                         { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("descendant-or-self"))); }
    break;

  case 1044: // FUNCTION_NAME: "'following-sibling'"
                        { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("following-sibling"))); }
    break;

  case 1045: // FUNCTION_NAME: "'preceding-sibling'"
                        { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("preceding-sibling"))); }
    break;

  case 1046: // FUNCTION_NAME: "'position'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("position"))); }
    break;

  case 1047: // FUNCTION_NAME: "'null'"
                 { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("null"))); }
    break;

  case 1048: // FUNCTION_NAME: "'true'"
                 { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("true"))); }
    break;

  case 1049: // FUNCTION_NAME: "'false'"
                  { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("false"))); }
    break;

  case 1050: // FUNCTION_NAME: "'select'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("select"))); }
    break;

  case 1051: // FUNCTION_NAME: "'jsoniq'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("jsoniq"))); }
    break;

  case 1052: // EQNAME: "'EQName'"
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


  const short jsoniq_parser::yypact_ninf_ = -1520;

  const short jsoniq_parser::yytable_ninf_ = -889;

  const short
  jsoniq_parser::yypact_[] =
  {
    5468, -1520, -1520, -1520, -1520,  7062,  7062,  7062,  7062,  7062,
    7062, -1520, -1520,    38,   217, -1520,   933,    24, -1520, -1520,
   -1520,   811, -1520, -1520, -1520,   284,   353,   885,  5000,   386,
     390,   833, -1520,     8, -1520,   434, -1520, -1520, -1520, -1520,
   -1520,   886, -1520,   469,   509, -1520, -1520, -1520, -1520,   528,
   -1520,   893, -1520,   533,   639, -1520,    48, -1520, -1520, -1520,
   -1520, -1520, -1520, -1520, -1520, -1520, -1520, -1520, -1520, -1520,
   -1520,   706,   741, -1520, -1520, -1520, -1520,   775, 14058, -1520,
   -1520, -1520,   777, -1520, -1520, -1520,   813, -1520,   785,   872,
   -1520, -1520, 18604, -1520, -1520, -1520, -1520, -1520, -1520, -1520,
   -1520,   859, -1520, -1520,   882,   890, -1520, -1520, -1520, -1520,
   -1520, -1520, -1520,  7062,  7380,  7698, 18604, -1520, -1520,   832,
   -1520, -1520, -1520, -1520,   879, -1520, -1520,   894, 18604, -1520,
   17094,   918,   931, -1520, -1520, -1520, -1520,   957, -1520, -1520,
   13104, -1520, -1520, -1520, -1520, -1520, -1520,   922, -1520, -1520,
   -1520, -1520, -1520, -1520, -1520, -1520,   126, -1520,   867, -1520,
   -1520, -1520, -1520, -1520, -1520, -1520, -1520, -1520, -1520, -1520,
     256,   932,   370, -1520,   -88,   537, -1520, -1520, -1520, -1520,
   -1520, -1520,   972, -1520,   845,   847,   848, -1520, -1520,   935,
     942, -1520, -1520, -1520, -1520, -1520, -1520, -1520, -1520, -1520,
   -1520, -1520, -1520,  5180, -1520, 13740, -1520, -1520, -1520, -1520,
   -1520, -1520, -1520, -1520, -1520, -1520, -1520, -1520, -1520, -1520,
   -1520, -1520, -1520, -1520, -1520, -1520, -1520, -1520, -1520, -1520,
   -1520, -1520, -1520, -1520, -1520, -1520, -1520, -1520, -1520, -1520,
   -1520, -1520, -1520, -1520, -1520, -1520, -1520, -1520, -1520, -1520,
   -1520, -1520, -1520, -1520,  8016,  8334, -1520,   778, -1520, -1520,
   -1520, -1520, -1520,  5788,  8652,  1103, -1520,  8970, -1520, -1520,
     145,   113, -1520, -1520, -1520, -1520, -1520, -1520, -1520, -1520,
   -1520, -1520, -1520, -1520, -1520, -1520,   117, -1520, -1520, -1520,
   -1520, -1520, -1520,   382, -1520, -1520, -1520, -1520, -1520, -1520,
   -1520, -1520,  7062, -1520, -1520, -1520, -1520,    55, -1520, -1520,
   -1520, -1520, -1520, -1520, -1520, -1520, -1520,   285, -1520,   910,
   -1520, -1520, -1520,   381, -1520,   151, -1520, -1520, -1520, -1520,
   -1520, -1520, -1520,   937,  1010, -1520, -1520,   858,   851,  1007,
      21,    59,   273,   759, -1520,  1059,   908,  1009,  1012, 11196,
   -1520,   921, -1520, -1520,   548, -1520, -1520, 13422, -1520,   831,
   -1520,   961, 14058, -1520,   961, 14058, -1520, -1520, -1520,   643,
   -1520, -1520, -1520, -1520, -1520, -1520, -1520, -1520, -1520, -1520,
   -1520, -1520, -1520, -1520, -1520, -1520, -1520, -1520, -1520, -1520,
   -1520, -1520, -1520, -1520, -1520, -1520, -1520,   955, -1520, -1520,
   -1520, -1520, -1520, -1520, -1520, -1520, -1520, -1520, -1520, -1520,
   -1520, -1520, -1520, -1520, -1520, -1520, -1520, -1520, -1520, -1520,
   -1520, -1520, -1520, -1520, -1520,   789, -1520, -1520, -1520, -1520,
   -1520,   934, -1520,  7062,   938,   939,   954,   956,   958,  7062,
     625,   625,  1088,    23,   515,   687, 19206, 18604,   402,  1073,
   18604,   968,  1019,   678, 18604,   846,   892, 18604, 18604,   830,
     849,    66, -1520, -1520, -1520, 18604,  7062,   962,  7062,   191,
   11514, 17396, 18604,   311,   987, 11514,  1150,   341,   417, 18604,
    1032,  1008,  1046, -1520,   967, 11514, 17698, 18604, 18604, 18604,
    7062,   969, 11514, 11514, 18604,  7062,  1001,  1003, -1520, -1520,
   -1520, 11514, 18000,  1002, -1520,  1004, -1520, -1520, -1520, -1520,
    1011, -1520,  1013, -1520, -1520, -1520, -1520, -1520,  1014, -1520,
   -1520, -1520, -1520, -1520, -1520, -1520, -1520, -1520, -1520, -1520,
   -1520, -1520, -1520, -1520, -1520, -1520, -1520, -1520, -1520, -1520,
   -1520, -1520, -1520, -1520, -1520, -1520, -1520, -1520, -1520, 18604,
   -1520, -1520, -1520, -1520, -1520, -1520, -1520, -1520, -1520, -1520,
   -1520, -1520, -1520,  1017, 18604, -1520, -1520,   981,  6108,  1044,
      46,  1050,  1016,  1018,  1020, 18604,  7062, -1520,  1022,   861,
     137, -1520,   381, -1520,   119,  1133, 11514, 11514, -1520,   198,
   -1520, -1520,  1160, -1520, -1520, -1520, -1520, 11514,   963, -1520,
    1153, 11514, 11514, 18912, 11514, 11514, 11514, 11514, 18912, 11514,
     947, 18604,   994,   995, 11514, 11514,  7062,   940, 18604, -1520,
   -1520,    81, -1520,    76, 11514,  8652, -1520, -1520, -1520, -1520,
   -1520,   933,   833,   120,   121,  1187,  9288,  3706,  9606,  4035,
     386,   813, -1520, -1520,   338,   813, -1520, 11514,  4671, -1520,
    1070,   744,     8,  1021,  1023,  1024, -1520, -1520, 11514,   779,
   -1520, -1520,  7062, -1520, -1520, -1520, -1520, -1520, -1520, -1520,
     903,   906, 18604,  1079, 11832, 11832, 12150, -1520, 12150, 12150,
   -1520, 12150, -1520, 12150, -1520, -1520, -1520, -1520, -1520, -1520,
   -1520, 12150, 12150,  1130, 12150, 12150, 12150, 12150, 12150, 12150,
   12150, 12150, 12150, 12150, 12150, 12150,   965,  1108,  1109,  1110,
   -1520, -1520, -1520, 12786,  7062, -1520, -1520, 13104, 13104, 11514,
     961, -1520, -1520,   961, -1520, 14980,  9924,  6426,   961,  1025,
    1056,  6744, -1520, -1520, -1520, -1520, -1520, -1520,   100, -1520,
     298, -1520, -1520, -1520, -1520, -1520, -1520,  1102,  1106,   386,
    1186, -1520, -1520, 19206,  1045,   671, 18604,  1055,  1057,  1045,
    1088,  1089,  1086, -1520, -1520, -1520,   520,   970,  1128,   920,
   18604,  1083, 11514,  1111, 18604, 18604, -1520,  1094,  1047, -1520,
    1048,  1004,   509, -1520,  1041,  1043,   136, -1520,   330,   393,
    1127, -1520,    53, 18604,  1141,   394, -1520,  7062, -1520,   134,
   -1520, 18604,  1140,  1193, 18604,  1088,  1142,   299, 18604, 11514,
       8, -1520,   169,  1052, -1520,  1053,  1058,  1060,    52, -1520,
     570,  1062, -1520,   177,   225,  1090, -1520,  1064,  7062,  7062,
     327, -1520,   501,   508,   644, 11514,    67, -1520, -1520, 11514,
   11514, -1520, 11514, 11514, 11514, 11514, -1520, 11514, -1520, 11514,
   -1520, 18604,  1133, -1520,   325,   352,   369, -1520, -1520, -1520,
     374, -1520,    41, -1520, -1520,  1095,  1096,  1097,  1098,  1099,
     643,   789,   454,   462,   143,    32,  1148,   661,  1015,   129,
      16, -1520,  1100, -1520, -1520,  1063,   231,  6108,   384, 14376,
     940, -1520,  1127, -1520, -1520,  1027, -1520,   145,   476,  1223,
     552, -1520, -1520,   123, -1520, -1520, -1520,   124, -1520,    87,
   -1520, -1520, -1520, -1520,   987, -1520,  4353, -1520, -1520, -1520,
   18604,  1091, 11514, 12468, -1520, -1520, -1520, -1520, -1520,  1127,
   18604,    54,  1010, -1520, -1520, -1520, -1520, -1520, 12150, -1520,
   -1520, -1520,    70, -1520,    59,    59,   430,   273,   273,   273,
     273,   759,   759, -1520, -1520, 16490, 16490, 18604, 18604, -1520,
    1072, -1520, -1520,   152, -1520, 10242, -1520, -1520, -1520, -1520,
   -1520, -1520, -1520, -1520,   550, -1520, -1520, -1520,   572,   625,
   -1520, -1520,   796,   809,   790, -1520,   386, -1520, -1520, -1520,
   -1520, -1520, -1520, -1520, -1520, -1520, -1520,  1045, -1520,  1125,
   16792,  1116, 11514, -1520, -1520, -1520,  1166,  1088,  1088,  1045,
   -1520,   899,  1088,   748, 18604,   518,   657,  1233, -1520, -1520,
     975,   586, -1520, -1520, -1520, -1520, -1520, -1520, -1520, -1520,
   -1520, -1520, -1520,   520,    93,  1031,   653, 11514, -1520, 18604,
    1173,   964,  1088, -1520, -1520, -1520, -1520,  1115, 18604, -1520,
   18604, -1520, 18302,  1143, 16490,  1152, 11514,   102,  1126,    85,
    1166, 16490,  1155,  1181,  1092,  1078,  1145,  1088,  1118,  1147,
    1185,  1088, 11514,    14, -1520, -1520, -1520,  1129, -1520, -1520,
   -1520, -1520,  1164, 11514, 11514,  1136, -1520,  1188,  1190,  7062,
   -1520,  1101,  1107,  1132, 18604, -1520, 18604, -1520, 11514,  1144,
    1112, 11514, -1520,  1170,  1171, -1520,   240,   253,   254,  1263,
   -1520,   431, -1520,   265,  1146, -1520, -1520,  1269, -1520,   534,
   11514, 11514, 11514,   818, 11514, 11514, 11514, 11514, 11514, 11514,
   11514, 11514, 11514, 18912,  1183, 11514, 11514, -1520, 10560,   409,
    1049, -1520,    28, -1520, 11514,   127,   132,    87,  9606,  4035,
    9606,  1225, -1520, -1520, -1520, 11514,   776,  1195, -1520, 18604,
    1202,  1203, -1520,   640,  1061, -1520, -1520,   781,    54, -1520,
   11514,    70,   377,   576, -1520,  1026,   200,  1051,  1054, -1520,
   -1520,   892, -1520,  1037,   681,  1154,  1168, 15282,  1172,  1174,
    1175,  1176,  1177,   916, -1520,   604, -1520, -1520, -1520, -1520,
   -1520, -1520, -1520, -1520, -1520, -1520,  1167, -1520, -1520, -1520,
   -1520, -1520,   264, 10878, -1520, -1520, -1520, -1520, -1520, -1520,
   -1520, -1520, -1520,   826, -1520,  1288, 16792,   866, -1520, -1520,
   -1520, -1520, -1520, -1520, -1520, -1520, -1520,  1030, -1520, -1520,
    1294, -1520, -1520, -1520, -1520, -1520,   635,  1298, -1520,   682,
   -1520, -1520, -1520,  1002,   353,   434,  1011,   469,  1013,  1014,
   -1520,  1174,  1175,  1176, -1520,   604,   604, 12786,  1036,  1028,
   -1520,  1166,    93,  1104,  1149,  7062,  1151,  1157,  1192,  1159,
    1165, 18604, -1520,   489, -1520, 18604, -1520, 11514,  1194, 11514,
    1207, 11514,   229,  1205, -1520, -1520, -1520, 16490, -1520,  7062,
    1088,  1244, -1520, -1520, -1520,  1088,  1244, -1520, 11514,  1212,
    7062, 18604, -1520, -1520, 11514, 11514,   498, -1520,   304,   854,
   -1520, 15584,   863, -1520,   871, -1520,  1178, -1520, -1520,  7062,
    1169,  1179, -1520, 11514, -1520, -1520, 11514, 11514,  1162,  1188,
    1258, -1520,  1232, -1520,   698, -1520, -1520,  1363, -1520, -1520,
    7062, 18604, -1520,   709, -1520, -1520, -1520,  7062,  1182,  1134,
    1137, -1520, -1520, -1520,  1139,  1156, -1520, -1520, -1520,  1272,
   -1520, -1520, -1520, -1520,  1138,   551, 18604,   829, -1520,  1213,
    1217,  1221,  1228, -1520,   910,   381, 14376,  1049, -1520, 10560,
   14376, -1520, -1520,  1223,   367, -1520, -1520, -1520,  1195, -1520,
    1088, -1520,   919, -1520,    64,  1273, -1520, 11514,   664,  1088,
   -1520, 12468, 11514,  1235, -1520,  1266,  1267, 11514, 18604,   953,
    1310, -1520, -1520, -1520,   388,  1204, -1520,    70,  1135, -1520,
   -1520,   -23, -1520,   581,   141,  1158,    70,   581, 12150, -1520,
     463, -1520, -1520, -1520, -1520, -1520, -1520,    70,  1241,  1105,
     970,   141, -1520, -1520,  1114,  1315, -1520, -1520, -1520, 14678,
    1208, -1520,  1209,  1211,  1214,  1215,  1219,  1220,  1154, -1520,
   -1520, -1520, -1520, -1520, -1520, -1520, -1520, -1520, -1520, -1520,
   -1520, -1520, -1520, -1520, -1520,  1370,    -8,  1371,    -8,  1163,
    1301, -1520, -1520,  1239, 18604,  1161, -1520, -1520, 12786, -1520,
    1226, -1520, -1520, -1520, -1520, -1520, -1520, 11514,  1264, -1520,
   -1520, 11514, -1520,   559, -1520, 11514,  1268, 11514, -1520,  1284,
    1287,  1319,  1088,  1244, -1520, 11514,  1234, -1520, -1520, -1520,
     829, -1520,   304, 11514,  7062,   829,   304, -1520, 18604,   679,
   -1520, 15886,   543, -1520, 16188,   829, -1520, -1520,  1237, -1520,
   -1520, -1520, -1520, -1520, 11514,   883,  1263, 18604,   824, -1520,
    1238,  1263, 18604, -1520,  1240, -1520, 11514, 11514, 11514, 11514,
    1113, 11514, 11514, -1520, 11514, 11514, 11514, 11514, 10560,   429,
   -1520, -1520, -1520, -1520, -1520,  1273, -1520, -1520, -1520, 11514,
    1274, -1520, -1520, -1520, 11514, 18604, 18604, -1520,   783, -1520,
   11514, -1520, -1520, -1520,  1242, 12150, -1520, -1520, -1520, -1520,
   -1520,   332, 12150, 12150,   600, -1520,  1051, -1520,   465, -1520,
    1054,    70,  1265, -1520, -1520,  1180, -1520, -1520, -1520, -1520,
    1331,  1245, -1520,   577, -1520, -1520, -1520, -1520, -1520, -1520,
   -1520, -1520,    68,    68, -1520,    -8, -1520, -1520,   587, -1520,
    1398,    90,  1332,  1248, 12786,    -2,  1184,  1261, -1520, -1520,
   11514, -1520, 11514,  1295, -1520, 11514, -1520, 10560, 11514,  1088,
   -1520, -1520, -1520, 11514, 11514, -1520, -1520, -1520, 10560, 10560,
    1348, 16490,  7062, 18604,   679, 18604, 11514, 18604,   679, 10560,
   -1520,   580,    35,  1263, 18604, -1520,  1246,  1263, -1520, -1520,
   -1520, -1520, -1520, 11514, -1520, -1520, -1520,   268,   342,   345,
   11514, -1520, -1520,  1202, 11514, -1520, -1520,  1392, -1520, -1520,
   -1520,   273, 12150, 12150,   430,   673, -1520, -1520, -1520, -1520,
   -1520, -1520, 11514, -1520, 16490, -1520, 16490,  1352, -1520, -1520,
   -1520,  1422, -1520, -1520, -1520,  1189,  1345, -1520, -1520,  1350,
   -1520,   711, 18604,  1340,  1231, 18604, 12786, -1520, -1520, 11514,
   -1520, -1520, -1520,  1244, -1520, -1520, 16490, -1520, -1520, -1520,
    1369, 11514,   829, -1520,  1372, 10560, -1520, 18604,   549,   477,
   -1520,  1270,  1263, -1520,  1275, -1520,  1276,  1188,  1190,   412,
   -1520,  1202,  1346,   430,   430, 12150,   437, -1520, -1520, 16490,
   -1520, -1520,  1332, 12786, -1520,  1273,  1191, 18604,  1349,  1249,
    1350, -1520,   679, 16490,  7062, 16490,   829, -1520, -1520,  1381,
     636, -1520, -1520, -1520, -1520,  1297,   490, -1520, -1520, -1520,
    1280, -1520, 10560,   888, -1520, -1520, 18604,   430, -1520, -1520,
   -1520, -1520, -1520, 11514,  1196, 18604,  1364,  7062,   679, -1520,
   -1520,   679, 11514, -1520, -1520, -1520, -1520, -1520, -1520, -1520,
    1365, -1520, -1520,  1197,  1198, 18604, -1520, -1520, 11514, 10560,
   18604,  1199, 11514,  1200,   829, 12786, -1520, 12786, 10560, -1520,
    1286,  1201, 18604,  1254,  1374, 18604,  1206, 12786, -1520
  };

  const short
  jsoniq_parser::yydefact_[] =
  {
     165,   473,   474,   824,   475,   168,   168,   168,   168,   168,
       0,   847,  1052,   120,   122,   651,   940,   949,   889,   852,
     850,   826,   941,   944,   896,   856,   827,   829,     0,   950,
     831,   947,   918,   898,   872,   828,   893,   894,   945,   942,
     892,   833,   948,   834,   835,   989,  1001,   988,   890,   909,
     903,   836,   891,   838,   837,   990,   927,   928,   895,   869,
    1011,  1012,  1013,  1014,  1015,  1016,  1017,  1018,  1019,  1020,
    1021,  1035,  1042,   917,   913,   904,   883,   825,     0,   912,
     920,   929,  1036,   908,   506,   884,   885,   943,  1037,  1043,
     905,   922,     0,   512,   462,   508,   915,   849,   906,   907,
     936,   910,   926,   935,  1041,  1044,   855,   897,   938,   507,
     925,   931,   830,   168,     0,     0,     0,   409,   923,   934,
     939,   937,   916,   902,   991,   900,   901,  1038,     0,   408,
       0,  1039,  1045,   932,   886,   911,  1050,  1040,   211,   440,
       0,   472,   933,   914,   921,   930,   924,   992,   877,   882,
     881,   880,   879,   878,   839,   899,     0,   848,  1051,   946,
     870,  1047,  1048,  1049,   982,   981,   983,   854,   853,   873,
     980,   832,   977,   985,   979,   978,   876,   919,   975,   987,
     986,   974,   874,   984,   996,   997,   994,   995,   993,   840,
     998,   999,  1000,   966,   965,   952,   868,   861,   959,   955,
     871,   867,   967,     0,   857,   858,   851,   860,   964,   963,
     960,   956,   972,   973,   971,   962,   958,   951,   859,   970,
     969,   863,   865,   864,   957,   961,   953,   866,   954,   862,
     968,  1022,  1023,  1024,  1025,  1026,  1027,  1003,  1004,  1002,
    1008,  1009,  1010,  1005,  1006,  1007,   875,  1028,  1029,  1030,
    1031,  1032,  1033,  1034,     0,     0,   841,   976,  1046,   843,
     842,   844,   845,   165,   165,     0,     2,   165,     9,    11,
      23,     0,    28,    31,    36,    37,    38,    39,    40,    41,
      42,    32,    58,    59,    33,    34,     0,    76,    79,    80,
      35,    81,    82,     0,   118,    83,    84,    85,    86,    18,
     162,   163,   164,   171,   174,   499,   175,     0,   176,   177,
     178,   179,   180,   181,   182,   185,   186,     0,   212,   220,
     215,   249,   255,     0,   247,     0,   248,   222,   216,   184,
     217,   183,   218,   221,   358,   360,   362,   364,   374,   376,
     380,   382,   385,   390,   393,   396,   398,   400,   402,     0,
     406,   413,   412,   414,     0,   431,   415,   436,   439,   441,
     444,   446,     0,   451,   448,     0,   459,   469,   471,   445,
     476,   490,   504,   491,   492,   493,   496,   497,   494,   498,
     521,   522,   495,   526,   528,   529,   530,   527,   575,   576,
     577,   578,   579,   580,   581,   468,   623,     0,   614,   622,
     621,   620,   619,   616,   618,   615,   617,   505,    43,   223,
     224,   226,   225,   227,   219,   501,   502,   503,   500,   229,
     232,   228,   230,   231,   470,   823,   846,   940,   949,   947,
    1051,     0,   166,   167,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   126,   128,   129,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   280,   281,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   464,   467,
     454,     0,     0,   826,   856,   829,   887,   950,   831,   898,
     833,   909,   836,   838,   837,   927,  1035,  1042,   825,  1036,
     885,  1037,  1043,   910,  1041,  1044,   934,   991,  1038,  1039,
    1045,  1040,   992,   980,   977,   979,   978,   874,   996,   997,
     994,   840,   998,   888,   858,   976,   460,   823,   452,     0,
     192,   453,   456,   826,   827,   829,   828,   833,   834,   835,
     836,   825,   832,   509,     0,   458,   457,     0,     0,   212,
       0,   470,   831,   838,   837,     0,   168,   801,   998,     0,
       0,   220,     0,   510,     0,   542,     0,     0,   463,     0,
     434,   435,     0,   466,   465,   455,   438,     0,     0,   571,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   363,
     803,     0,   805,     0,     0,   165,     3,     4,     1,    10,
      12,     0,     0,     0,     0,     6,   165,     0,   165,     0,
       0,     0,   119,   172,     0,     0,   190,     0,     0,   199,
       0,     0,     0,     0,     0,     0,   235,   236,     0,     0,
     204,   233,     0,   254,   250,   256,   251,   253,   252,   260,
       0,     0,     0,     0,     0,     0,     0,   425,     0,     0,
     423,     0,   369,     0,   424,   417,   422,   421,   420,   419,
     418,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     411,   410,   407,     0,   168,   432,   437,     0,     0,     0,
     447,   487,   450,   449,   461,     0,     0,     0,   477,     0,
       0,     0,   583,   586,   584,   587,   591,   593,     0,   124,
       0,   822,    47,    44,    45,    48,    49,     0,     0,     0,
       0,    50,    51,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   660,   661,   662,     0,   103,   142,     0,
       0,   112,     0,     0,     0,     0,   127,     0,     0,   631,
       0,     0,     0,   626,     0,     0,     0,   641,     0,     0,
     258,   263,     0,     0,   134,     0,   138,   168,   524,     0,
      60,     0,    69,     0,     0,     0,    61,     0,     0,     0,
       0,   629,     0,     0,   632,     0,     0,     0,     0,   329,
       0,     0,   630,     0,     0,     0,   649,     0,     0,     0,
       0,   635,     0,     0,   194,     0,     0,   188,   187,     0,
       0,   807,     0,     0,     0,     0,   509,     0,   802,     0,
     511,   543,   542,   539,     0,     0,     0,   573,   572,   433,
       0,   570,     0,   673,   674,     0,     0,     0,     0,     0,
     816,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   679,     0,   202,   203,     0,     0,     0,     0,     0,
     207,   208,   259,   804,   806,     0,     5,    24,     0,    25,
       0,     7,    29,     0,    15,     8,    30,     0,    19,   940,
      77,    16,    78,    20,     0,   193,     0,   191,   213,   214,
       0,     0,     0,     0,   298,   205,   234,   237,   238,   257,
       0,     0,   359,   361,   367,   373,   372,   371,     0,   368,
     365,   366,     0,   377,   384,   383,   381,   387,   388,   389,
     386,   391,   392,   395,   394,     0,     0,     0,     0,   416,
       0,   442,   443,     0,   488,     0,   484,   485,   486,   481,
     480,   517,   478,   519,     0,   624,   523,   515,     0,     0,
     121,   123,     0,     0,     0,   102,     0,    92,    94,    95,
      96,    97,    99,   100,   101,    93,    98,    88,    89,     0,
       0,   108,     0,   104,   106,   107,   114,     0,     0,    87,
      46,     0,     0,     0,     0,     0,     0,     0,   751,   756,
       0,     0,   752,   786,   739,   741,   742,   743,   745,   747,
     746,   744,   748,     0,     0,     0,     0,     0,   111,     0,
     144,     0,     0,   590,   582,   627,   628,     0,     0,   645,
       0,   642,     0,     0,     0,     0,     0,     0,     0,     0,
     140,     0,     0,   135,     0,     0,     0,     0,     0,     0,
      62,     0,     0,     0,   279,   285,   282,     0,   634,   633,
     640,   648,     0,     0,     0,     0,   589,     0,     0,     0,
     426,     0,     0,     0,     0,   638,     0,   636,     0,   195,
       0,     0,   809,     0,   470,   808,     0,     0,     0,   542,
     540,     0,   531,     0,     0,   513,   514,     0,    13,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   200,     0,     0,
       0,   687,     0,   209,     0,     0,     0,     0,   165,     0,
     165,     0,   525,   173,   246,     0,   313,   309,   311,     0,
     306,   299,   300,     0,     0,   239,   240,     0,   245,   370,
       0,     0,     0,     0,   717,   378,   690,   694,   696,   698,
     700,   703,   710,   711,   719,   950,   830,     0,   839,   843,
     842,   844,   845,     0,   397,   596,   603,   604,   607,   652,
     653,   608,   609,   612,   399,   401,   404,   613,   403,   430,
     489,   482,     0,     0,   479,   516,   125,    56,    57,    54,
      55,   131,   130,     0,    90,     0,     0,     0,   109,   110,
     115,    74,    75,    52,    53,    73,   757,     0,   760,   787,
       0,   750,   749,   754,   753,   785,     0,     0,   762,     0,
     758,   761,   740,     0,     0,     0,     0,     0,     0,     0,
     625,     0,     0,     0,   143,   146,   148,     0,     0,     0,
     113,   116,     0,     0,     0,   168,     0,     0,   649,     0,
       0,     0,   265,     0,   595,     0,   266,     0,     0,     0,
       0,     0,     0,     0,   141,   136,   139,     0,   189,     0,
       0,    71,    65,    68,    67,     0,    63,   283,     0,     0,
     168,     0,   327,   331,     0,     0,     0,   334,     0,     0,
     340,     0,     0,   347,     0,   351,     0,   428,   427,   168,
       0,     0,   196,     0,   198,   328,     0,     0,     0,     0,
       0,   543,     0,   533,     0,   566,   563,     0,   567,   568,
       0,     0,   562,     0,   537,   565,   564,     0,     0,     0,
       0,   666,   667,   663,     0,     0,   671,   672,   668,   812,
     813,   677,   817,   675,     0,     0,     0,     0,   681,   831,
     838,   837,   998,   201,     0,     0,     0,   682,   683,     0,
       0,   210,   815,    26,     0,    17,    21,    22,   310,   322,
       0,   323,     0,   314,   315,   316,   317,     0,   509,     0,
     307,     0,     0,     0,   241,     0,     0,     0,     0,   292,
       0,   289,   294,   244,     0,     0,   701,   714,     0,   375,
     379,     0,   733,     0,     0,     0,     0,     0,     0,   689,
     691,   692,   728,   729,   730,   732,   731,     0,     0,   705,
     704,     0,   708,   712,   726,   724,   723,   716,   720,     0,
       0,   599,     0,     0,     0,     0,     0,     0,     0,   654,
     655,   600,   602,   601,   597,   405,   483,   518,   520,   133,
     132,    91,   105,   774,   755,     0,   779,     0,   779,   768,
     763,   147,   149,     0,     0,     0,   117,   145,     0,    27,
       0,   646,   647,   650,   643,   644,   264,     0,     0,   278,
     270,     0,   274,     0,   268,     0,     0,     0,   137,     0,
       0,    70,     0,    64,   284,     0,     0,   330,   332,   337,
       0,   335,     0,     0,     0,     0,     0,   341,     0,     0,
     355,     0,     0,   348,     0,     0,   352,   429,     0,   639,
     637,   197,   810,   811,     0,     0,   542,     0,     0,   574,
       0,   542,     0,   538,     0,    14,     0,     0,     0,     0,
       0,     0,     0,   680,     0,     0,     0,     0,     0,     0,
     684,   688,   326,   324,   325,   318,   319,   320,   312,     0,
       0,   308,   301,   287,     0,     0,     0,   243,   290,   293,
       0,   718,   709,   715,     0,     0,   788,   789,   799,   798,
     797,     0,     0,     0,     0,   790,   695,   796,     0,   693,
     697,     0,     0,   702,   706,     0,   727,   722,   725,   721,
       0,     0,   610,     0,   605,   659,   598,   821,   820,   819,
     606,   775,     0,     0,   773,   780,   781,   777,     0,   772,
       0,   770,     0,     0,     0,     0,     0,     0,   585,   267,
       0,   276,     0,     0,   272,     0,   275,     0,     0,     0,
      66,   286,   592,     0,     0,   338,   342,   336,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     588,     0,     0,   542,     0,   569,     0,   542,   594,   664,
     665,   669,   670,     0,   676,   818,   678,     0,     0,     0,
       0,   685,   321,   302,     0,   288,   296,   297,   291,   242,
     713,   800,     0,     0,   792,     0,   738,   737,   736,   735,
     734,   699,     0,   791,     0,   656,     0,     0,   784,   783,
     782,     0,   776,   769,   767,     0,   764,   765,   759,   150,
     152,   154,     0,     0,     0,     0,     0,   271,   269,     0,
     277,   206,   357,    72,   333,   339,     0,   356,   353,   349,
       0,     0,     0,   343,     0,     0,   345,     0,   552,   546,
     541,     0,   542,   532,     0,   814,     0,     0,     0,     0,
     305,   303,     0,   794,   793,     0,     0,   657,   611,     0,
     778,   771,     0,     0,   156,   155,     0,     0,     0,     0,
     151,   273,     0,     0,     0,     0,     0,   560,   554,     0,
     553,   555,   561,   558,   548,     0,   547,   549,   559,   534,
       0,   535,     0,     0,   686,   304,     0,   795,   707,   658,
     766,   153,   157,     0,     0,     0,     0,     0,     0,   346,
     344,     0,     0,   545,   556,   557,   544,   550,   551,   536,
       0,   295,   158,     0,     0,     0,   354,   350,     0,     0,
       0,     0,     0,     0,     0,     0,   160,     0,     0,   159,
       0,     0,     0,     0,     0,     0,     0,     0,   161
  };

  const short
  jsoniq_parser::yypgoto_[] =
  {
   -1520, -1520,  -187,  -190, -1520,  1224,  1230, -1520,  1222,  -539,
   -1520, -1520, -1520, -1520, -1520, -1520, -1520, -1520, -1520, -1520,
   -1017, -1520, -1520, -1520, -1520,  -234,  -607, -1520,   734,   -92,
   -1520, -1520, -1520, -1520, -1520,   272,   499, -1520, -1520,     0,
    -279,  1065, -1520,  1040, -1520, -1520,  -562, -1520,   440, -1520,
     233, -1520,  -253,  -296, -1520,  -528, -1520,    43,   209,    50,
    -226,  -188, -1520,  -835, -1520, -1520,  -451, -1520, -1520, -1520,
   -1520, -1520, -1520, -1520, -1520, -1520, -1520,   618, -1520,    97,
    1853,    17, -1520, -1520,  -322, -1520, -1520,   344, -1520, -1520,
    -311, -1520,    34, -1520, -1520,  -191,  -949,  -777,  -728, -1520,
   -1520,   700, -1520, -1520,   -83,    98, -1520, -1520, -1520,   105,
   -1322, -1520,   363,   114, -1520, -1520,   118, -1334, -1520,   959,
     213, -1520, -1520,   211, -1058, -1520, -1520,   206, -1520, -1520,
   -1279, -1318, -1520,   204, -1458, -1520, -1520,   856,  -152, -1520,
   -1520,  -591,   837, -1520, -1520,  -694,   376,  -680,   379,   380,
   -1520, -1520, -1520,   574, -1520, -1520,  1216, -1520, -1520, -1520,
   -1520, -1520,  -867,  -343,  -676, -1520,  -105, -1520, -1520, -1520,
   -1520, -1520, -1520, -1520,     3,  -842, -1520,  -570,   695,   302,
   -1520,  -372, -1520,   808, -1520,   812, -1520, -1520, -1520,   805,
   -1520, -1520, -1520, -1520, -1041, -1520,   195, -1520,   688,  -839,
   -1520, -1520, -1520, -1520, -1520,  -284,  -277, -1248,  -928, -1520,
   -1520, -1520, -1520, -1520, -1520, -1520, -1520, -1520, -1520, -1520,
    -740,  -910,  -179,  -896, -1520, -1520, -1520,  -893, -1520, -1520,
   -1520, -1520, -1520, -1520, -1520, -1520, -1520,  1076,  1077,  -167,
     497,  -697, -1520,   355,   360, -1520, -1520, -1520, -1520, -1520,
   -1520, -1520, -1520,   178, -1520, -1520,   168, -1520,   170, -1078,
   -1520, -1520, -1520,   122,   110,   -59,   391, -1520, -1520, -1520,
   -1520, -1520, -1520, -1520, -1520, -1520, -1520, -1520, -1520, -1520,
     130, -1520, -1520, -1520,   -54,   392,   536, -1520, -1520, -1520,
   -1520, -1520,   318, -1520, -1520, -1519, -1520, -1520, -1520,  -552,
   -1520,    83, -1520,   -73, -1520, -1520, -1520, -1520, -1345, -1520,
     138, -1520, -1520, -1520, -1520, -1520,   971, -1520, -1520, -1520,
   -1520, -1520,  -945,  -412,  1468,   986, -1520
  };

  const short
  jsoniq_parser::yydefgoto_[] =
  {
       0,   265,   648,   266,   267,   268,   269,   270,   271,   272,
     273,   274,   275,   276,   277,   278,   279,   280,   281,   282,
    1301,   805,   283,   284,   285,   286,   287,   288,   997,   998,
     999,   289,   290,   291,  1003,  1004,  1005,   292,   460,   579,
     294,   738,   295,   462,   463,   464,   474,   795,   796,   296,
    1264,   297,  1749,  1750,   298,   299,   300,   567,   301,   302,
     303,   304,   305,   798,   306,   307,   550,   308,   309,   310,
     311,   312,   313,   660,   314,   315,   890,   891,   316,   317,
     318,   581,   320,   661,  1686,   673,  1167,  1168,   321,   663,
     322,   665,   582,   324,   325,   790,   791,  1419,   481,   326,
     482,   483,   931,  1420,  1421,  1422,   666,   667,  1161,  1162,
    1410,   668,  1157,  1158,  1403,  1404,  1405,  1406,   327,   818,
     819,   328,  1316,  1317,  1532,   329,  1319,  1320,   330,   331,
    1322,  1323,  1324,  1325,  1539,   332,   333,   334,   335,   336,
     938,   337,   338,  1429,   339,   340,   341,   342,   343,   344,
     345,   346,   347,  1205,   348,   349,   350,   351,   691,   692,
     352,   353,   354,   355,   356,   357,   358,   359,   360,   361,
     362,   363,   364,   365,   366,   367,   368,   369,   728,   721,
     370,   371,   372,   373,   374,   375,   376,   377,   378,   974,
     379,   380,   381,   382,   383,   384,  1353,   852,   853,   854,
    1780,  1825,  1826,  1819,  1820,  1827,  1821,  1354,  1355,   385,
     386,  1356,   387,   388,   389,   390,   391,   392,   393,   394,
    1059,  1540,  1474,  1195,  1633,  1196,  1206,   395,   396,   397,
     398,   399,   400,   401,   402,   403,   404,   405,   406,   825,
    1277,   407,  1198,  1199,  1200,  1201,   408,   409,   410,   411,
     412,   413,   880,   881,   414,  1387,  1388,  1711,  1142,  1175,
    1439,  1440,  1176,  1177,  1178,  1179,  1180,  1449,  1623,  1624,
    1181,  1452,  1182,  1604,  1183,  1184,  1457,  1458,  1629,  1627,
    1441,  1442,  1443,  1444,  1729,   767,  1024,  1025,  1026,  1027,
    1028,  1029,  1250,  1653,  1746,  1251,  1651,  1744,  1030,  1488,
    1648,  1644,  1645,  1646,  1031,  1032,  1445,  1453,  1614,  1446,
    1610,  1430,   415,   416,   417,   418,   570,   419,   420,   421,
     422,   423,  1202,  1302,   424,   425,   426
  };

  const short
  jsoniq_parser::yytable_[] =
  {
     293,   662,   617,   946,  1543,   293,   293,   293,   293,   293,
     293,   715,   664,  1111,   642,  1057,   461,   319,   947,   948,
     949,   950,   319,   319,   319,   319,   319,   319,   968,  1318,
     742,   910,   912,   870,   323,   596,   634,   959,   877,   323,
     323,   323,   323,   323,   323,  1194,  1204,  1141,   431,   434,
     435,   436,   437,   619,  1306,   433,   433,   433,   433,   433,
     439,  1555,  1197,  1197,  1058,   802,   806,  1073,   739,   739,
    1586,  1587,  1352,   626,   627,  1173,   643,   635,   914,  1152,
    1085,   546,   635,  1684,   637,   934,  1688,   935,   936,  1266,
     937,  1053,   939,  1425,  1099,    13,    14,   902,   906,   639,
     940,   941,    15,  1282,  1227,  1563,  1625,  1197,   904,   908,
     911,   913,  1164,   293,   635,   446,    13,    14,   635,  1752,
     635,   635,   635,  1290,   635,   635,  1134,   494,   635,   479,
     319,  1778,  1642,  1747,  1253,   635,   446,   635,   635,  1254,
     781,  1265,   840,   449,  1284,   607,   447,   323,  1255,  1222,
    1054,  1295,  1055,   635,   669,  1256,   840,  1257,   782,   598,
    1308,  1197,  1082,   568,   449,   645,  1258,  1485,  1197,   450,
     635,   442,   443,  1400,   444,   445,  1117,  1082,   635,   698,
     631,   773,   642,   695,  1055,  1389,   647,   984,   465,  1611,
     915,   647,   448,  1259,   917,  1402,   632,  1643,   440,  1606,
     696,   451,   452,  1151,  1056,   495,   857,   643,   743,   699,
     979,   580,   584,   643,   432,   432,   432,   432,   432,   438,
     670,  1779,   700,   744,   697,   671,   635,  1170,  1487,   647,
    1171,  1753,   635,   841,   608,  1118,  1291,  1543,   781,  1607,
    1611,   635,  1390,  1083,   647,  1174,   647,   647,  1394,   646,
     130,  1712,   716,  1287,   635,   635,   782,   701,  1101,  1135,
     849,  1536,   647,   293,   293,   635,  1230,   293,   839,   635,
    1342,   664,   672,  1128,  1345,  1346,  1165,   496,   497,   647,
     319,   319,  1292,  1840,   319,  -169,   635,   647,  1523,   480,
     980,  1462,   910,   912,  1197,   480,   454,   323,   323,  1431,
     642,   323,   293,  1352,  1173,  1173,  1745,   636,  1812,   850,
    1563,   638,  1352,  1754,   897,   899,  1166,  1148,  1150,   319,
    1294,  1393,   432,  1047,  1065,   848,  1260,  1266,   635,  1288,
    1462,  1293,  1506,  1197,   480,   647,   323,  1172,  -170,   635,
    1210,   647,   643,   635,   599,  1309,   635,  1169,  1010,  1603,
     647,   621,   623,   635,  1612,  1848,  1077,  1851,   902,   906,
     455,   774,  1132,   647,   647,   722,   480,  1087,   724,   775,
     635,  1347,  1432,   741,   647,   635,   858,   441,   647,  1265,
    1515,   783,   457,   480,  1127,   635,  1722,  1518,  1480,   635,
     458,  1790,  1348,  1070,  1349,   647,  1054,   894,   459,   644,
     893,   456,   903,   907,  1197,  1612,  1613,  1533,   979,    15,
     635,    13,    14,   635,   757,  1088,   758,  1804,  1261,  1262,
    1263,  1138,  1350,  1413,  1223,   650,   116,   892,  1197,   651,
    1338,   640,   793,   293,   652,   896,   925,   647,   635,   293,
    1048,   468,   128,  1339,  1340,  1072,   898,   900,   647,   741,
     319,  1133,   647,   653,  1476,   647,   319,   916,  1786,  1112,
    1433,  1434,   647,  1435,  1351,  1723,   293,   323,   293,  1835,
    1436,  1842,  -169,   323,  1174,  1174,   760,   905,   702,   647,
    1437,   929,   156,   319,   647,   319,  1345,   703,   981,   601,
     293,   602,  1438,   656,   647,   293,   657,   641,   647,  1345,
     323,   794,   323,  1050,  1062,   801,  1508,   319,  1823,  1833,
     480,  1146,   319,   469,  1093,  1543,   439,  1352,   439,   647,
    1049,  1823,   647,   933,   323,  -170,  1113,   632,  1481,   323,
     644,  1496,  1787,   803,  1170,  1788,   644,  1171,   761,  1114,
     439,   910,   912,   910,  1141,   439,   473,   647,  1581,  1632,
     475,  1119,   654,   905,  1817,   655,  1115,   130,  1345,  1123,
    1173,  1116,  1431,  1375,  1516,  1343,  1197,   786,   293,  1173,
     656,  1140,   799,   657,   751,  1601,   293,  1013,   603,  1014,
    1173,   804,   812,  1051,  1063,   319,  1710,  1147,  1055,   823,
     824,  1493,   695,   319,   484,  1231,  1232,   658,   830,  1834,
    1235,  1315,   323,   604,  1348,   605,  1349,  1216,   752,   696,
     323,  1094,   961,   962,  1530,  1824,   293,  1348,  1096,  1349,
    1395,  1396,  1397,  1015,  1838,   293,   433,   695,  1857,   487,
    1274,   461,  1344,   319,  1350,  1432,   293,   293,   293,   293,
    1507,  1817,   319,  1390,   696,  1345,   659,  1350,  1197,   489,
     323,  1197,  1763,   319,   319,   319,   319,    15,  1055,   323,
    1213,   643,   293,  1145,  1685,   644,   887,  1054,  1590,   488,
     323,   323,   323,   323,   725,   778,  1348,   780,  1349,   319,
    1120,  1121,  1213,   855,   856,  1247,  1818,  1736,  1124,  1125,
    1153,  1095,   606,   492,   860,  1122,   323,  1741,  1097,   821,
     745,  1777,  1248,  1126,   827,   714,  1350,  1345,  1346,   726,
    1662,   727,   886,   888,   293,   746,  1149,  1692,  1345,  1346,
     900,  1084,  1696,  1433,  1434,   757,  1435,   758,   130,  1536,
    1174,   319,   656,  1427,  1485,   657,  1663,  1054,    84,  1174,
    1214,  1054,  1726,  1437,  1618,   472,  1249,  -261,   323,  1727,
    1174,  1486,  1471,    95,  1173,  1438,   130,   960,  1130,  1728,
     759,  1054,  1215,  1348,   433,  1349,   725,  1737,  1000,   656,
     609,  1767,   657,  1854,  1359,   610,  1360,  1742,   109,  1016,
     618,  1247,  -262,  1472,   725,   432,   910,   760,  1197,  1241,
    1098,  1412,  1017,  1350,  1018,  1487,  1572,   293,  1248,   493,
    1242,   726,  1473,   727,  1347,  1019,  1020,  1021,  1054,  1022,
    1589,  1023,  1001,  -261,   319,  1347,   963,  1002,  -261,   726,
    1400,   727,  1657,   963,  1797,  1348,  1798,  1349,   293,   293,
     715,   323,   498,  1345,  1346,   695,  1348,  1415,  1349,  1415,
    1064,  1197,  1416,  1197,  1416,   319,   319,   433,  -262,   761,
    1822,  1828,   696,  -262,  1781,  1350,   747,  1608,  1784,   116,
     476,  1417,   323,   323,  1609,  -261,  1350,   499,   656,   477,
     116,   657,   748,  1197,  1399,   128,  1795,  1726,   439,   439,
    1055,  1276,  1055,  1279,  1727,  1400,   128,   293,  1521,  1839,
      13,    14,  1822,  1681,  1728,  1401,   478,  1557,  1828,   704,
    -262,   500,  1418,   548,   319,  1224,  1197,  1402,  1562,   461,
     640,   551,  1383,   705,   642,   156,   293,  1224,  1724,  1725,
    1197,   323,  1197,   432,  1174,  1721,   156,  1330,  1243,  1331,
    1347,  1221,   501,   319,   549,   502,  1454,  1455,   730,  1244,
    1106,  1107,  1108,  1830,   888,    13,    14,   797,  1217,   731,
     323,  1348,   916,  1349,  1391,  1193,  1193,  1315,   900,  1456,
    1268,  1219,    13,    14,  1218,  1468,  1541,  1479,   466,  1269,
    1535,   467,   442,   443,  1544,   444,   445,  1220,  1751,  1542,
     564,  1350,   446,   797,  1139,   116,  1541,  1545,  1582,   586,
     771,  1541,   447,   448,   676,   772,  1534,  1591,   552,  1691,
    1193,   128,   677,   678,  1860,   679,   432,  1001,   565,  1415,
     449,  1805,  1002,   680,  1416,   450,   566,   681,   656,   682,
     588,   657,   964,  1694,   683,   964,   717,   718,  1793,  1794,
     964,  1233,  1234,  1236,  1237,  1238,   587,  1091,  1092,  1239,
     684,   156,   470,   485,   593,   471,   486,   451,   452,   453,
     490,  1583,  1584,   491,  1193,  1574,   720,   594,  1364,   723,
    1365,  1193,  1212,  1578,   547,   685,   686,   687,   688,   689,
     690,   944,   945,  1519,   664,   763,   764,   765,   547,   597,
    1751,   951,   952,   595,   953,   954,  1491,  1492,   600,   293,
    1738,  1739,  -625,   611,   612,   615,   613,   454,   614,   616,
     624,  1837,   547,   628,   649,   675,   319,   674,   693,   694,
    1670,   706,   707,   708,   547,   729,   547,   709,   713,   719,
     741,   732,   753,   323,   755,   733,   734,  1751,   987,   988,
     989,   990,   991,   992,   993,   994,   995,   996,   293,   439,
     756,   735,   770,   736,   797,   737,   766,   461,   293,   293,
     293,   800,   779,   808,   809,  1384,   810,   811,   828,   822,
     829,   455,   467,   835,   471,   319,   319,   319,   837,   839,
     851,   486,  1385,   491,   502,   842,   843,   859,   844,   847,
     845,   861,   323,   323,   323,   862,   879,  1193,   883,   884,
     901,   920,   921,   889,   617,   927,   922,   923,   928,  1879,
     930,  1880,   456,   942,   955,   956,   957,   958,  1673,   976,
    1674,  1888,   982,  1678,  1679,   975,   983,  1682,   985,  1007,
    1011,  1008,  1012,  1689,  1033,  1034,  1193,  1035,   457,  1037,
    1042,  1045,  1039,  1046,  1043,  1044,   458,  1052,  1061,  1067,
    1068,  1071,  1078,  1079,   459,  1129,  1136,  1089,  1080,  1086,
    1081,  1090,   466,   470,   485,   490,   501,  1137,  1147,  1209,
    1131,  1225,  1002,  1054,  1155,  1245,  1267,  1424,  1144,  1246,
    1272,  1273,  1275,  1285,  1283,   293,   793,  1289,  1297,  1298,
    1299,  1300,  1303,  1304,  1305,  1311,  1310,  1314,  1327,  1329,
    1333,  1315,   319,  1321,  1328,  1336,  1337,  1193,  1326,   293,
    1341,  1358,  1386,  1357,  1376,  1407,  1334,   759,  1676,   323,
     293,  1409,  1451,  1411,  1459,  1475,   319,  1447,  1500,  1428,
      15,  1193,  1448,  1414,  1483,   433,  1484,   319,  1460,   293,
    1489,  1494,  1463,   323,  1464,  1465,  1466,  1467,  1513,  1498,
    1503,  1501,  1495,  1499,   323,  1511,   319,  1502,   547,  1504,
     293,   547,   925,  1526,  1522,  1505,  1517,   293,  1525,  1549,
     433,  1541,  1771,   323,  1554,  1547,  1775,   319,  1556,  1550,
    1559,  1570,  1548,  1575,   319,  1566,  1565,  1576,  1567,   433,
    1568,  1577,  1400,  1571,   323,  1389,  1594,  1595,  1596,   293,
    1600,   323,  1621,  1605,  1602,  1622,  1628,  1569,  1634,  1635,
     439,  1636,  1641,  1647,  1637,  1638,  1384,   439,  1626,  1639,
    1640,  1652,  1654,  1658,  1667,  1660,  1615,  1668,  1669,  1665,
    1714,  1672,  1732,  1385,  1690,  1695,  1656,  1698,  1734,  1720,
    1743,  1247,   749,   547,  1756,  1735,   547,  1703,  1748,   887,
     547,  1761,  1650,   547,   547,  1766,  1759,  1783,  1792,  1799,
    1814,   547,  1765,  1676,  1800,  1802,  1768,   547,   547,  1193,
    1803,  1807,  1801,  1776,  1808,   547,  1813,  1836,  1733,  1815,
    1845,  1829,   547,   547,   547,   547,  1831,  1853,  1832,  1755,
     547,  1859,  1846,  1856,   432,  1865,  1870,  1884,   547,  1881,
    1847,   629,   633,  1009,  1852,  1885,  1882,   630,  1871,  1482,
    1228,   776,  1296,  1810,  1843,  1497,   740,  1841,  1143,  1863,
    1076,  1872,  1423,  1875,  1877,  1718,  1592,  1599,  1398,   432,
    1887,  1588,  1585,   836,  1527,  1537,  1868,  1531,  1546,  1869,
     932,   943,  1208,   966,   293,   547,   978,   967,   432,  1558,
    1110,  1193,  1858,  1855,  1193,   784,   785,  1280,  1469,  1768,
     547,   319,  1878,  1470,  1573,  1580,  1579,  1620,  1616,  1560,
     563,   547,  1731,  1426,  1730,   712,  1564,  1490,   323,  1252,
    1619,  1649,  1740,  1450,     0,  1617,     0,   875,   293,     0,
       0,   571,     0,     0,   585,     0,     0,     0,  1849,   871,
       0,     0,     0,     0,   871,  1384,   589,   547,   592,     0,
       0,     0,     0,     0,   547,     0,  1519,     0,     0,     0,
       0,     0,  1385,     0,     0,     0,     0,     0,     0,     0,
       0,  1866,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1866,     0,     0,     0,     0,     0,     0,
       0,     0,  1849,     0,     0,     0,     0,     0,   547,     0,
       0,     0,     0,     0,     0,     0,     0,   293,     0,     0,
       0,     0,  1707,  1708,  1709,     0,     0,     0,   293,   293,
       0,  1193,   293,     0,  1384,     0,     0,     0,     0,   293,
       0,     0,     0,     0,     0,  1384,  1384,     0,     0,   319,
       0,  1385,     0,     0,     0,     0,  1384,     0,     0,     0,
       0,   547,  1385,  1385,     0,     0,   323,     0,     0,     0,
       0,     0,     0,  1385,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1193,     0,  1193,     0,     0,   986,
       0,     0,   547,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   547,     0,     0,     0,
     547,   547,     0,     0,     0,     0,  1193,     0,     0,     0,
       0,     0,     0,     0,     0,   293,     0,     0,     0,   547,
       0,     0,     0,     0,     0,     0,     0,   547,     0,     0,
     547,     0,  1384,     0,   547,     0,     0,     0,     0,  1193,
       0,     0,     0,     0,     0,     0,     0,  1789,     0,  1385,
       0,     0,     0,  1193,   293,  1193,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1796,
       0,   319,   293,     0,     0,     0,     0,   547,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   293,   323,  1384,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   319,     0,  1385,     0,     0,   293,
       0,     0,     0,     0,     0,   547,     0,     0,   293,     0,
       0,   323,     0,     0,     0,     0,  1384,     0,     0,     0,
       0,     0,     0,     0,     0,  1384,     0,     0,     0,     0,
       0,     0,     0,  1385,     0,     0,   547,     0,     0,     0,
       0,     0,  1385,     0,     0,   750,   547,     0,   754,     0,
       0,     0,   762,     0,     0,   768,   769,     0,     0,     0,
       0,     0,     0,   777,     0,     0,     0,     0,     0,   789,
     792,   547,   547,   547,   547,     0,     0,   807,     0,     0,
       0,     0,     0,     0,   815,   816,   817,   820,     0,     0,
       0,     0,   826,     0,     0,     0,   569,     0,     0,     0,
     833,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   547,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     547,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   834,     0,     0,
       0,     0,     0,     0,     0,   547,     0,     0,     0,     0,
       0,     0,   820,     0,   547,     0,   547,     0,   547,     0,
     547,     0,     0,   846,   571,     0,     0,   547,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   571,     0,     0,     0,     0,   882,
     547,     0,   547,     0,     0,     0,   792,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   871,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     792,     0,     0,     0,     0,   547,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   547,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   969,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   547,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1006,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1036,     0,
       0,     0,  1040,  1041,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1060,     0,     0,     0,     0,     0,   547,     0,  1066,
       0,   547,  1069,     0,     0,     0,  1074,     0,     0,     0,
       0,     0,     0,   547,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   547,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   547,  1104,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1109,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   547,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   547,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   547,     0,     0,     0,   547,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1154,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1163,     0,
       0,     0,     0,     0,   547,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1203,  1203,  1207,  1207,     0,     0,   569,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   547,     0,     0,     0,     0,
       0,     0,     0,     0,   863,   864,     0,   872,   873,   874,
     876,     0,   878,     0,     0,     0,     0,   885,  1203,     0,
       0,     0,     0,     0,     0,     0,     0,   895,     0,     0,
     547,     0,  1240,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     918,   919,     0,     0,     0,     0,     0,  1271,     0,     0,
       0,   924,     0,     0,     0,   926,  1278,     0,  1278,     0,
     792,     0,  1203,     0,   547,     0,     0,   547,     0,  1203,
     547,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   547,     0,     0,     0,     0,   547,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   826,     0,   826,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     973,   547,   547,     0,   973,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1038,     0,  1408,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1203,     0,     0,     0,     0,
       0,     0,  1075,     0,     0,     0,     0,   547,     0,   547,
       0,   547,     0,   547,     0,     0,     0,     0,     0,     0,
     547,     0,     0,     0,     0,     0,     0,     0,  1100,     0,
       0,     0,  1102,  1103,  1203,  1105,     0,     0,     0,     0,
       0,     0,   926,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     547,     0,   547,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   547,     0,
       0,   547,     0,     0,     0,     0,     0,     0,     0,   792,
       0,     0,   547,  1509,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   547,     0,  1203,     0,     0,     0,   919,
       0,     0,     0,     0,     0,  1156,  1160,     0,     0,   820,
       0,     0,     0,     0,     0,   547,     0,     0,     0,  1203,
       0,     0,     0,   547,     0,     0,     0,     0,     0,   547,
       0,   547,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1561,
       0,     0,   547,     0,     0,     0,     0,     0,     0,     0,
       0,   547,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   882,     0,     0,     0,     0,     0,
       0,   547,     0,     0,     0,  1229,   547,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   547,     0,
       0,   547,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1598,     0,     0,     0,
    1270,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1286,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1307,     0,  1203,     0,     0,
       0,     0,     0,     0,     0,     0,  1312,  1313,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1332,     0,     0,  1335,     0,     0,     0,     0,     0,
       0,     0,  1655,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1361,  1362,  1363,     0,  1366,  1367,  1368,
    1369,  1370,  1371,  1372,  1373,  1374,     0,     0,  1377,  1378,
       0,     0,     0,     0,     0,     0,     0,  1392,     0,     0,
       0,     0,     0,     0,     0,     0,  1680,     0,  1156,  1203,
       0,     0,  1203,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1693,     0,     0,     0,     0,
    1697,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1716,  1717,     0,  1478,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1510,     0,  1512,     0,  1514,     0,     0,     0,     0,  1203,
       0,  1770,  1520,  1772,     0,  1774,     0,     0,     0,     0,
       0,  1524,  1782,     0,     0,     0,     0,  1528,  1529,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1551,     0,     0,  1552,
    1553,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1203,     0,  1203,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1806,     0,     0,  1809,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1203,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1816,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1156,     0,     0,     0,  1160,  1593,     0,  1203,     0,     0,
    1597,     0,     0,     0,     0,  1844,     0,     0,     0,     0,
       0,  1203,     0,  1203,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1861,     0,     0,     0,     0,     0,
       0,     0,     0,  1864,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1873,     0,     0,     0,     0,  1874,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1883,     0,     0,  1886,     0,     0,     0,     0,     0,     0,
    1659,     0,     0,     0,  1661,     0,     0,     0,  1664,     0,
    1666,     0,     0,     0,     0,     0,     0,     0,  1671,     0,
       0,     0,     0,     0,     0,     0,  1675,  1677,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1520,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1699,
    1700,  1701,  1702,     0,  1704,  1705,     0,  1706,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1713,     0,     0,     0,     0,  1715,     0,     0,
       0,     0,     0,  1719,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1757,     0,  1758,     0,     0,  1760,     0,
       0,  1762,     0,     0,     0,     0,  1764,  1677,     0,     0,
       0,     0,     0,     0,     0,  1769,     0,     0,     0,  1773,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1785,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1791,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1811,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1769,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1850,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1862,     0,     0,     0,
    1867,     0,     0,     0,     0,  1850,  -165,   905,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     1,     2,     0,
       0,  1867,     3,     0,     0,  1876,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,     0,    15,     0,
       0,    16,   428,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,     0,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,     0,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,     0,    74,    75,    76,    77,    78,     0,    79,
      80,    81,     0,     0,    82,    83,     0,     0,     0,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,     0,
      94,     0,     0,     0,    95,     0,    96,    97,    98,    99,
       0,   100,     0,     0,     0,   101,   102,   103,   104,   105,
       0,     0,     0,     0,     0,     0,   106,   107,   108,   109,
     110,   111,   112,   113,   114,     0,   115,   116,   117,   118,
     119,     0,     0,     0,   120,   121,   122,   123,   124,   125,
     126,   127,     0,   128,     0,   129,   130,     0,     0,   131,
     132,   133,     0,     0,     0,   134,     0,   135,   136,   137,
     138,   139,   140,     0,   141,     0,   142,     0,   143,   144,
     145,   146,   147,   148,   149,   150,   151,   152,   153,     0,
       0,   154,   155,   156,     0,   157,   430,   159,   160,   161,
     162,   163,   164,   165,   166,   167,   168,   169,   170,   171,
     172,   173,   174,   175,   176,   177,   178,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,   197,   198,   199,   200,   201,
     202,   203,     0,   204,   205,   206,     0,   207,   208,   209,
     210,   211,   212,   213,   214,   215,   216,   217,   218,   219,
     220,   221,   222,   223,   224,   225,   226,   227,   228,   229,
     230,   231,   232,   233,   234,   235,   236,   237,   238,   239,
     240,   241,   242,   243,   244,   245,   246,   247,   248,   249,
     250,   251,   252,   253,   254,     0,   255,     0,   256,   257,
     258,   259,   260,   261,   262,  -165,   905,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     1,     2,     0,     0,
       0,     3,     0,     0,     0,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,     0,    15,     0,     0,
     909,   428,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,   429,    32,    33,    34,
      35,    36,     0,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,     0,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,     0,    74,    75,    76,    77,    78,     0,    79,    80,
      81,     0,     0,    82,    83,     0,     0,     0,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,     0,    94,
       0,     0,     0,    95,     0,    96,    97,    98,    99,     0,
     100,     0,     0,     0,   101,   102,   103,   104,   105,     0,
       0,     0,     0,     0,     0,   106,   107,   108,   109,   110,
     111,   112,   113,   114,     0,   115,   116,   117,   118,   119,
       0,     0,     0,   120,   121,   122,   123,   124,   125,   126,
     127,     0,   128,     0,   129,   130,     0,     0,   131,   132,
     133,     0,     0,     0,   134,     0,   135,   136,   137,   138,
     139,   140,     0,   141,     0,   142,     0,   143,   144,   145,
     146,   147,   148,   149,   150,   151,   152,   153,     0,     0,
     154,   155,   156,     0,   157,   430,   159,   160,   161,   162,
     163,   164,   165,   166,   167,   168,   169,   170,   171,   172,
     173,   174,   175,   176,   177,   178,   179,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,   197,   198,   199,   200,   201,   202,
     203,     0,   204,   205,   206,     0,   207,   208,   209,   210,
     211,   212,   213,   214,   215,   216,   217,   218,   219,   220,
     221,   222,   223,   224,   225,   226,   227,   228,   229,   230,
     231,   232,   233,   234,   235,   236,   237,   238,   239,   240,
     241,   242,   243,   244,   245,   246,   247,   248,   249,   250,
     251,   252,   253,   254,   905,   255,     0,   256,   257,   258,
     259,   260,   261,   262,     1,     2,     0,     0,     0,     3,
       0,     0,     0,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,     0,    15,     0,     0,   427,   428,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,   429,    32,    33,    34,    35,    36,
       0,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,     0,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,     0,
      74,    75,    76,    77,    78,     0,    79,    80,    81,     0,
       0,    82,    83,     0,     0,     0,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,     0,    94,     0,     0,
       0,    95,     0,    96,    97,    98,    99,     0,   100,     0,
       0,     0,   101,   102,   103,   104,   105,     0,     0,     0,
       0,     0,     0,   106,   107,   108,   109,   110,   111,   112,
     113,   114,     0,   115,   116,   117,   118,   119,     0,     0,
       0,   120,   121,   122,   123,   124,   125,   126,   127,     0,
     128,     0,   129,   130,     0,     0,   131,   132,   133,     0,
       0,     0,   134,     0,   135,   136,   137,   138,   139,   140,
       0,   141,     0,   142,     0,   143,   144,   145,   146,   147,
     148,   149,   150,   151,   152,   153,     0,     0,   154,   155,
     156,     0,   157,   430,   159,   160,   161,   162,   163,   164,
     165,   166,   167,   168,   169,   170,   171,   172,   173,   174,
     175,   176,   177,   178,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,   197,   198,   199,   200,   201,   202,   203,     0,
     204,   205,   206,     0,   207,   208,   209,   210,   211,   212,
     213,   214,   215,   216,   217,   218,   219,   220,   221,   222,
     223,   224,   225,   226,   227,   228,   229,   230,   231,   232,
     233,   234,   235,   236,   237,   238,   239,   240,   241,   242,
     243,   244,   245,   246,   247,   248,   249,   250,   251,   252,
     253,   254,   905,   255,     0,   256,   257,   258,   259,   260,
     261,   262,     1,     2,     0,     0,     0,     3,     0,     0,
       0,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,     0,    15,     0,     0,   427,   428,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,   572,   429,    32,    33,    34,    35,    36,     0,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,     0,    50,    51,    52,   573,   574,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,     0,    74,    75,
      76,    77,    78,     0,    79,    80,    81,     0,     0,    82,
      83,     0,     0,     0,    84,    85,   520,    87,    88,    89,
      90,    91,   575,    93,     0,    94,     0,     0,     0,    95,
       0,    96,    97,    98,    99,     0,   100,     0,     0,     0,
     101,   102,   103,   104,   105,     0,     0,     0,     0,     0,
       0,   106,   107,   108,   109,   110,   111,   112,   576,   114,
       0,   115,   116,   117,   118,   119,     0,     0,     0,   120,
     121,   122,   123,   124,   125,   126,   127,     0,   128,     0,
     129,   130,     0,     0,   131,   132,   133,     0,     0,     0,
     134,     0,   135,   136,   137,     0,   139,   140,     0,   141,
       0,   142,     0,   143,   144,   145,   146,   147,   148,   149,
     150,   151,   152,   153,     0,     0,   154,   155,   156,     0,
     157,   430,   159,   160,   161,   162,   163,   164,   165,   166,
     167,   168,   169,   170,   171,   172,   173,   174,   175,   176,
     177,   178,   179,   180,   181,   182,   183,   538,   539,   540,
     187,   188,   541,   578,   191,   192,   193,   194,   195,   196,
     197,   198,   199,   200,   201,   202,   203,     0,   204,   205,
     206,     0,   207,   208,   209,   210,   211,   212,   213,   214,
     215,   216,   217,   218,   219,   220,   221,   222,   223,   224,
     225,   226,   227,   228,   229,   230,   231,   232,   233,   234,
     235,   236,   237,   238,   239,   240,   241,   242,   243,   244,
     245,   246,   247,   248,   249,   250,   251,   252,   253,   254,
       0,   255,     0,   256,   257,   258,   259,   260,   261,   262,
    -887,   472,     0,  -261,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  -887,     0,     0,     0,  -887,     0,
       0,     0,  -887,  -887,     0,     0,     0,     0,  -887,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  -261,
       0,     0,  -887,     0,  -261,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  -887,     0,  -887,  -887,  -887,
       0,     0,  -887,  -887,  -887,  -887,     0,     0,     0,  -887,
    -887,     0,     0,     0,     0,     0,  -887,     0,     0,  -887,
    -887,  -261,     0,     0,     0,  -887,     0,     0,     0,     0,
    -887,  -887,     0,     0,     0,     0,  -887,     0,     0,     0,
    -887,     0,     0,     0,  -887,  -887,     0,  -887,     0,  -887,
    -887,     0,     0,     0,  -887,  -887,     0,     0,  -887,  -887,
    -887,  -887,  -887,  -887,     0,     0,  -887,     0,     0,     0,
    -887,  -887,     0,     0,  -887,     0,     0,     0,     0,  -887,
    -888,   618,  -887,  -262,     0,     0,     0,  -887,  -887,  -887,
    -887,  -887,  -887,     0,  -887,  -887,  -887,  -887,  -887,     0,
       0,     0,     0,  -887,  -887,  -887,     0,  -887,  -887,  -887,
    -887,  -887,  -887,     0,  -887,     0,  -887,     0,     0,     0,
       0,     0,  -887,     0,  -888,     0,  -887,  -887,  -888,     0,
       0,     0,  -888,  -888,     0,     0,     0,     0,  -888,     0,
       0,  -887,     0,  -887,     0,  -887,     0,     0,     0,  -262,
       0,     0,  -888,     0,  -262,     0,     0,  -887,     0,     0,
       0,     0,  -887,     0,     0,  -887,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  -888,     0,  -888,  -888,  -888,
       0,     0,  -888,  -888,  -888,  -888,     0,     0,     0,  -888,
    -888,     0,     0,     0,     0,     0,  -888,     0,     0,  -888,
    -888,  -262,     0,     0,     0,  -888,     0,     0,     0,     0,
    -888,  -888,     0,     0,     0,     0,  -888,     0,     0,  -887,
    -888,  -887,     0,     0,  -888,  -888,     0,  -888,     0,  -888,
    -888,     0,     0,     0,  -888,  -888,     0,     0,  -888,  -888,
    -888,  -888,  -888,  -888,     0,     0,  -888,     0,     0,     0,
    -888,  -888,     0,     0,  -888,     0,     0,     0,     0,  -888,
       0,     0,  -888,     0,     0,     0,     0,  -888,  -888,  -888,
    -888,  -888,  -888,     0,  -888,  -888,  -888,  -888,  -888,     0,
       0,     0,     0,  -888,  -888,  -888,     0,  -888,  -888,  -888,
    -888,  -888,  -888,     0,  -888,     0,  -888,     0,     0,     0,
       0,     0,  -888,     0,     0,     0,  -888,  -888,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  -888,     0,  -888,     0,  -888,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  -888,     0,     0,
       0,     0,  -888,     0,     0,  -888,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     1,
       2,     0,     0,     0,     3,     0,     0,     0,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,  -888,
      15,  -888,     0,    16,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,     0,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,     0,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,     0,    74,    75,    76,    77,    78,
       0,    79,    80,    81,     0,     0,    82,    83,     0,     0,
       0,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,     0,    94,     0,     0,     0,    95,     0,    96,    97,
      98,    99,     0,   100,     0,     0,     0,   101,   102,   103,
     104,   105,     0,     0,     0,     0,     0,     0,   106,   107,
     108,   109,   110,   111,   112,   113,   114,     0,   115,   116,
     117,   118,   119,     0,     0,     0,   120,   121,   122,   123,
     124,   125,   126,   127,     0,   128,     0,   129,   130,     0,
       0,   131,   132,   133,     0,     0,     0,   134,     0,   135,
     136,   137,   138,   139,   140,     0,   141,     0,   142,     0,
     143,   144,   145,   146,   147,   148,   149,   150,   151,   152,
     153,     0,     0,   154,   155,   156,     0,   157,   158,   159,
     160,   161,   162,   163,   164,   165,   166,   167,   168,   169,
     170,   171,   172,   173,   174,   175,   176,   177,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,   197,   198,   199,
     200,   201,   202,   203,     0,   204,   205,   206,     0,   207,
     208,   209,   210,   211,   212,   213,   214,   215,   216,   217,
     218,   219,   220,   221,   222,   223,   224,   225,   226,   227,
     228,   229,   230,   231,   232,   233,   234,   235,   236,   237,
     238,   239,   240,   241,   242,   243,   244,   245,   246,   247,
     248,   249,   250,   251,   252,   253,   254,     0,   255,     0,
     256,   257,   258,   259,   260,   261,   262,   263,   264,     1,
       2,     0,     0,     0,     3,     0,     0,     0,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,     0,
      15,     0,     0,    16,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,     0,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,     0,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,     0,    74,    75,    76,    77,    78,
       0,    79,    80,    81,     0,     0,    82,    83,     0,     0,
       0,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,     0,    94,     0,     0,     0,    95,     0,    96,    97,
      98,    99,     0,   100,     0,     0,     0,   101,   102,   103,
     104,   105,     0,     0,     0,     0,     0,     0,   106,   107,
     108,   109,   110,   111,   112,   113,   114,     0,   115,   116,
     117,   118,   119,     0,     0,     0,   120,   121,   122,   123,
     124,   125,   126,   127,     0,   128,     0,   129,   130,     0,
       0,   131,   132,   133,     0,     0,     0,   134,     0,   135,
     136,   137,   138,   139,   140,     0,   141,     0,   142,     0,
     143,   144,   145,   146,   147,   148,   149,   150,   151,   152,
     153,     0,     0,   154,   155,   156,     0,   157,   158,   159,
     160,   161,   162,   163,   164,   165,   166,   167,   168,   169,
     170,   171,   172,   173,   174,   175,   176,   177,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,   197,   198,   199,
     200,   201,   202,   203,     0,   204,   205,   206,     0,   207,
     208,   209,   210,   211,   212,   213,   214,   215,   216,   217,
     218,   219,   220,   221,   222,   223,   224,   225,   226,   227,
     228,   229,   230,   231,   232,   233,   234,   235,   236,   237,
     238,   239,   240,   241,   242,   243,   244,   245,   246,   247,
     248,   249,   250,   251,   252,   253,   254,     0,   255,     0,
     256,   257,   258,   259,   260,   261,   262,     0,   625,     1,
       2,     0,     0,     0,     3,     0,     0,     0,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,     0,
      15,     0,     0,   427,   428,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,   429,
      32,    33,    34,    35,    36,     0,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,     0,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,     0,    74,    75,    76,    77,    78,
       0,    79,    80,    81,     0,     0,    82,    83,     0,     0,
       0,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,     0,    94,     0,     0,     0,    95,     0,    96,    97,
      98,    99,     0,   100,     0,     0,     0,   101,   102,   103,
     104,   105,     0,     0,     0,     0,     0,     0,   106,   107,
     108,   109,   110,   111,   112,   113,   114,     0,   115,   116,
     117,   118,   119,     0,     0,     0,   120,   121,   122,   123,
     124,   125,   126,   127,     0,   128,     0,   129,   130,     0,
       0,   131,   132,   133,     0,   838,     0,   134,     0,   135,
     136,   137,   138,   139,   140,     0,   141,     0,   142,     0,
     143,   144,   145,   146,   147,   148,   149,   150,   151,   152,
     153,     0,     0,   154,   155,   156,     0,   157,   430,   159,
     160,   161,   162,   163,   164,   165,   166,   167,   168,   169,
     170,   171,   172,   173,   174,   175,   176,   177,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,   197,   198,   199,
     200,   201,   202,   203,     0,   204,   205,   206,     0,   207,
     208,   209,   210,   211,   212,   213,   214,   215,   216,   217,
     218,   219,   220,   221,   222,   223,   224,   225,   226,   227,
     228,   229,   230,   231,   232,   233,   234,   235,   236,   237,
     238,   239,   240,   241,   242,   243,   244,   245,   246,   247,
     248,   249,   250,   251,   252,   253,   254,     0,   255,     0,
     256,   257,   258,   259,   260,   261,   262,     1,     2,     0,
       0,     0,     3,     0,     0,     0,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,     0,    15,     0,
       0,   427,   428,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,   572,   429,    32,    33,
      34,    35,    36,     0,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,     0,    50,    51,
      52,   573,   574,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,     0,    74,    75,    76,    77,    78,     0,    79,
      80,    81,     0,     0,    82,    83,     0,     0,     0,    84,
      85,   520,    87,    88,    89,    90,    91,   575,    93,     0,
      94,     0,     0,     0,    95,     0,    96,    97,    98,    99,
       0,   100,     0,     0,     0,   101,   102,   103,   104,   105,
       0,     0,     0,     0,   971,     0,   106,   107,   108,   109,
     110,   111,   112,   576,   114,     0,   115,   116,   117,   118,
     119,     0,     0,     0,   120,   121,   122,   123,   124,   125,
     126,   127,     0,   128,     0,   129,   130,     0,     0,   131,
     132,   133,     0,     0,     0,   134,   972,   135,   136,   137,
       0,   139,   140,     0,   141,     0,   142,     0,   143,   144,
     145,   146,   147,   148,   149,   150,   151,   152,   153,     0,
       0,   154,   155,   156,     0,   157,   430,   159,   160,   161,
     162,   163,   164,   165,   166,   167,   168,   169,   170,   171,
     172,   173,   174,   175,   176,   177,   178,   179,   180,   181,
     182,   183,   538,   539,   540,   187,   188,   541,   578,   191,
     192,   193,   194,   195,   196,   197,   198,   199,   200,   201,
     202,   203,     0,   204,   205,   206,     0,   207,   208,   209,
     210,   211,   212,   213,   214,   215,   216,   217,   218,   219,
     220,   221,   222,   223,   224,   225,   226,   227,   228,   229,
     230,   231,   232,   233,   234,   235,   236,   237,   238,   239,
     240,   241,   242,   243,   244,   245,   246,   247,   248,   249,
     250,   251,   252,   253,   254,     0,   255,     0,   256,   257,
     258,   259,   260,   261,   262,     1,     2,     0,     0,     0,
       3,     0,     0,     0,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,     0,    15,     0,     0,   427,
     428,     0,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,   572,   429,    32,    33,    34,    35,
      36,     0,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,     0,    50,    51,    52,   573,
     574,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
       0,    74,    75,    76,    77,    78,     0,    79,    80,    81,
       0,     0,    82,    83,     0,     0,     0,    84,    85,   520,
      87,    88,    89,    90,    91,   575,    93,     0,    94,     0,
       0,     0,    95,     0,    96,    97,    98,    99,     0,   100,
       0,     0,     0,   101,   102,   103,   104,   105,     0,     0,
       0,     0,   971,     0,   106,   107,   108,   109,   110,   111,
     112,   576,   114,     0,   115,   116,   117,   118,   119,     0,
       0,     0,   120,   121,   122,   123,   124,   125,   126,   127,
       0,   128,     0,   129,   130,     0,     0,   131,   132,   133,
       0,     0,     0,   134,   977,   135,   136,   137,     0,   139,
     140,     0,   141,     0,   142,     0,   143,   144,   145,   146,
     147,   148,   149,   150,   151,   152,   153,     0,     0,   154,
     155,   156,     0,   157,   430,   159,   160,   161,   162,   163,
     164,   165,   166,   167,   168,   169,   170,   171,   172,   173,
     174,   175,   176,   177,   178,   179,   180,   181,   182,   183,
     538,   539,   540,   187,   188,   541,   578,   191,   192,   193,
     194,   195,   196,   197,   198,   199,   200,   201,   202,   203,
       0,   204,   205,   206,     0,   207,   208,   209,   210,   211,
     212,   213,   214,   215,   216,   217,   218,   219,   220,   221,
     222,   223,   224,   225,   226,   227,   228,   229,   230,   231,
     232,   233,   234,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,   245,   246,   247,   248,   249,   250,   251,
     252,   253,   254,     0,   255,     0,   256,   257,   258,   259,
     260,   261,   262,     1,     2,     0,     0,     0,     3,     0,
       0,     0,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,     0,    15,     0,     0,   427,   428,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,   429,    32,    33,    34,    35,    36,     0,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,     0,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,     0,    74,
      75,    76,    77,    78,     0,    79,    80,    81,     0,     0,
      82,    83,     0,     0,     0,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,     0,    94,     0,     0,     0,
      95,     0,    96,    97,    98,    99,     0,   100,     0,     0,
       0,   101,   102,   103,   104,   105,     0,     0,     0,     0,
       0,     0,   106,   107,   108,   109,   110,   111,   112,   113,
     114,     0,   115,   116,   117,   118,   119,     0,     0,     0,
     120,   121,   122,   123,   124,   125,   126,   127,     0,   128,
       0,   129,   130,     0,     0,   131,   132,   133,     0,     0,
       0,   134,     0,   135,   136,   137,   138,   139,   140,     0,
     141,     0,   142,     0,   143,   144,   145,   146,   147,   148,
     149,   150,   151,   152,   153,     0,     0,   154,   155,   156,
       0,   157,   430,   159,   160,   161,   162,   163,   164,   165,
     166,   167,   168,   169,   170,   171,   172,   173,   174,   175,
     176,   177,   178,   179,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   197,   198,   199,   200,   201,   202,   203,     0,   204,
     205,   206,     0,   207,   208,   209,   210,   211,   212,   213,
     214,   215,   216,   217,   218,   219,   220,   221,   222,   223,
     224,   225,   226,   227,   228,   229,   230,   231,   232,   233,
     234,   235,   236,   237,   238,   239,   240,   241,   242,   243,
     244,   245,   246,   247,   248,   249,   250,   251,   252,   253,
     254,     0,   255,     0,   256,   257,   258,   259,   260,   261,
     262,     1,     2,     0,     0,     0,     3,     0,     0,     0,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,     0,    15,     0,     0,   427,   428,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
     572,   429,    32,    33,    34,    35,    36,     0,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,     0,    50,    51,    52,   573,   574,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,     0,    74,    75,    76,
      77,    78,     0,    79,    80,    81,     0,     0,    82,    83,
       0,     0,     0,    84,    85,   520,    87,    88,    89,    90,
      91,   575,    93,     0,    94,     0,     0,     0,    95,     0,
      96,    97,    98,    99,     0,   100,     0,     0,     0,   101,
     102,   103,   104,   105,     0,     0,     0,     0,     0,     0,
     106,   107,   108,   109,   110,   111,   112,   576,   114,     0,
     115,   116,   117,   118,   119,     0,     0,     0,   120,   121,
     122,   123,   124,   125,   126,   127,     0,   128,     0,   129,
     130,     0,     0,   131,   132,   133,     0,     0,   577,   134,
       0,   135,   136,   137,     0,   139,   140,     0,   141,     0,
     142,     0,   143,   144,   145,   146,   147,   148,   149,   150,
     151,   152,   153,     0,     0,   154,   155,   156,     0,   157,
     430,   159,   160,   161,   162,   163,   164,   165,   166,   167,
     168,   169,   170,   171,   172,   173,   174,   175,   176,   177,
     178,   179,   180,   181,   182,   183,   538,   539,   540,   187,
     188,   541,   578,   191,   192,   193,   194,   195,   196,   197,
     198,   199,   200,   201,   202,   203,     0,   204,   205,   206,
       0,   207,   208,   209,   210,   211,   212,   213,   214,   215,
     216,   217,   218,   219,   220,   221,   222,   223,   224,   225,
     226,   227,   228,   229,   230,   231,   232,   233,   234,   235,
     236,   237,   238,   239,   240,   241,   242,   243,   244,   245,
     246,   247,   248,   249,   250,   251,   252,   253,   254,     0,
     255,     0,   256,   257,   258,   259,   260,   261,   262,     1,
       2,     0,     0,     0,     3,     0,     0,     0,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,     0,
      15,     0,     0,   427,   428,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,   572,   429,
      32,    33,    34,    35,    36,     0,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,     0,
      50,    51,    52,   573,   574,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,     0,    74,    75,    76,    77,    78,
       0,    79,    80,    81,     0,     0,    82,    83,     0,     0,
       0,    84,    85,   520,    87,    88,    89,    90,    91,   575,
      93,     0,    94,     0,     0,     0,    95,     0,    96,    97,
      98,    99,     0,   100,     0,     0,     0,   101,   102,   103,
     104,   105,     0,     0,     0,     0,     0,     0,   106,   107,
     108,   109,   110,   111,   112,   576,   114,     0,   115,   116,
     117,   118,   119,     0,     0,     0,   120,   121,   122,   123,
     124,   125,   126,   127,     0,   128,     0,   129,   130,     0,
       0,   131,   132,   133,     0,     0,     0,   134,   583,   135,
     136,   137,     0,   139,   140,     0,   141,     0,   142,     0,
     143,   144,   145,   146,   147,   148,   149,   150,   151,   152,
     153,     0,     0,   154,   155,   156,     0,   157,   430,   159,
     160,   161,   162,   163,   164,   165,   166,   167,   168,   169,
     170,   171,   172,   173,   174,   175,   176,   177,   178,   179,
     180,   181,   182,   183,   538,   539,   540,   187,   188,   541,
     578,   191,   192,   193,   194,   195,   196,   197,   198,   199,
     200,   201,   202,   203,     0,   204,   205,   206,     0,   207,
     208,   209,   210,   211,   212,   213,   214,   215,   216,   217,
     218,   219,   220,   221,   222,   223,   224,   225,   226,   227,
     228,   229,   230,   231,   232,   233,   234,   235,   236,   237,
     238,   239,   240,   241,   242,   243,   244,   245,   246,   247,
     248,   249,   250,   251,   252,   253,   254,     0,   255,     0,
     256,   257,   258,   259,   260,   261,   262,     1,     2,     0,
       0,     0,     3,     0,     0,     0,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,     0,    15,     0,
       0,   427,   428,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,   572,   429,    32,    33,
      34,    35,    36,     0,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,     0,    50,    51,
      52,   573,   574,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,     0,    74,    75,    76,    77,    78,     0,    79,
      80,    81,     0,     0,    82,    83,     0,     0,     0,    84,
      85,   520,    87,    88,    89,    90,    91,   575,    93,     0,
      94,     0,     0,     0,    95,     0,    96,    97,    98,    99,
       0,   100,     0,     0,     0,   101,   102,   103,   104,   105,
       0,     0,     0,     0,     0,     0,   106,   107,   108,   109,
     110,   111,   112,   576,   114,     0,   115,   116,   117,   118,
     119,     0,     0,     0,   120,   121,   122,   123,   124,   125,
     126,   127,     0,   128,     0,   129,   130,     0,     0,   131,
     132,   133,     0,     0,     0,   134,     0,   135,   136,   137,
       0,   139,   140,     0,   141,     0,   142,     0,   143,   144,
     145,   146,   147,   148,   149,   150,   151,   152,   153,     0,
       0,   154,   155,   156,     0,   157,   430,   159,   160,   161,
     162,   163,   164,   165,   166,   167,   168,   169,   170,   171,
     172,   173,   174,   175,   176,   177,   178,   179,   180,   181,
     182,   183,   538,   539,   540,   187,   188,   541,   578,   191,
     192,   193,   194,   195,   196,   197,   198,   199,   200,   201,
     202,   203,     0,   204,   205,   206,     0,   207,   208,   209,
     210,   211,   212,   213,   214,   215,   216,   217,   218,   219,
     220,   221,   222,   223,   224,   225,   226,   227,   228,   229,
     230,   231,   232,   233,   234,   235,   236,   237,   238,   239,
     240,   241,   242,   243,   244,   245,   246,   247,   248,   249,
     250,   251,   252,   253,   254,   620,   255,     0,   256,   257,
     258,   259,   260,   261,   262,     1,     2,     0,     0,     0,
       3,     0,     0,     0,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,     0,    15,     0,     0,   427,
     428,     0,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,   572,   429,    32,    33,    34,    35,
      36,     0,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,     0,    50,    51,    52,   573,
     574,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
       0,    74,    75,    76,    77,    78,     0,    79,    80,    81,
       0,     0,    82,    83,     0,     0,     0,    84,    85,   520,
      87,    88,    89,    90,    91,   575,    93,     0,    94,     0,
       0,     0,    95,     0,    96,    97,    98,    99,     0,   100,
       0,     0,     0,   101,   102,   103,   104,   105,     0,     0,
       0,     0,     0,     0,   106,   107,   108,   109,   110,   111,
     112,   576,   114,     0,   115,   116,   117,   118,   119,     0,
       0,     0,   120,   121,   122,   123,   124,   125,   126,   127,
       0,   128,     0,   129,   130,     0,     0,   131,   132,   133,
       0,     0,     0,   134,     0,   135,   136,   137,     0,   139,
     140,     0,   141,     0,   142,     0,   143,   144,   145,   146,
     147,   148,   149,   150,   151,   152,   153,     0,     0,   154,
     155,   156,     0,   157,   430,   159,   160,   161,   162,   163,
     164,   165,   166,   167,   168,   169,   170,   171,   172,   173,
     174,   175,   176,   177,   178,   179,   180,   181,   182,   183,
     538,   539,   540,   187,   188,   541,   578,   191,   192,   193,
     194,   195,   196,   197,   198,   199,   200,   201,   202,   203,
       0,   204,   205,   206,     0,   207,   208,   209,   210,   211,
     212,   213,   214,   215,   216,   217,   218,   219,   220,   221,
     222,   223,   224,   225,   226,   227,   228,   229,   230,   231,
     232,   233,   234,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,   245,   246,   247,   248,   249,   250,   251,
     252,   253,   254,     0,   255,   622,   256,   257,   258,   259,
     260,   261,   262,     1,     2,     0,     0,     0,     3,     0,
       0,     0,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,     0,    15,     0,     0,    16,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,     0,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,     0,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,     0,    74,
      75,    76,    77,    78,     0,    79,    80,    81,     0,     0,
      82,    83,     0,     0,     0,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,     0,    94,     0,     0,     0,
      95,     0,    96,    97,    98,    99,     0,   100,     0,     0,
       0,   101,   102,   103,   104,   105,     0,     0,     0,     0,
       0,     0,   106,   107,   108,   109,   110,   111,   112,   113,
     114,     0,   115,   116,   117,   118,   119,     0,     0,     0,
     120,   121,   122,   123,   124,   125,   126,   127,     0,   128,
       0,   129,   130,     0,     0,   131,   132,   133,     0,     0,
       0,   134,     0,   135,   136,   137,   138,   139,   140,     0,
     141,     0,   142,     0,   143,   144,   145,   146,   147,   148,
     149,   150,   151,   152,   153,     0,     0,   154,   155,   156,
       0,   157,   158,   159,   160,   161,   162,   163,   164,   165,
     166,   167,   168,   169,   170,   171,   172,   173,   174,   175,
     176,   177,   178,   179,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   197,   198,   199,   200,   201,   202,   203,     0,   204,
     205,   206,     0,   207,   208,   209,   210,   211,   212,   213,
     214,   215,   216,   217,   218,   219,   220,   221,   222,   223,
     224,   225,   226,   227,   228,   229,   230,   231,   232,   233,
     234,   235,   236,   237,   238,   239,   240,   241,   242,   243,
     244,   245,   246,   247,   248,   249,   250,   251,   252,   253,
     254,     0,   255,     0,   256,   257,   258,   259,   260,   261,
     262,     1,     2,     0,     0,     0,     3,     0,     0,     0,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,     0,    15,     0,     0,    16,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,     0,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,     0,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,     0,    74,    75,    76,
      77,    78,     0,    79,    80,    81,     0,     0,    82,    83,
       0,     0,     0,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,     0,    94,     0,     0,     0,    95,     0,
      96,    97,    98,    99,     0,   100,     0,     0,     0,   101,
     102,   103,   104,   105,     0,     0,     0,     0,     0,     0,
     106,   107,   108,   109,   110,   111,   112,   113,   114,     0,
     115,   116,   117,   118,   119,     0,     0,     0,   120,   121,
     122,   123,   124,   125,   126,   127,     0,   128,     0,   129,
     130,     0,     0,   131,   132,   133,     0,     0,     0,   134,
       0,   135,   136,   137,   138,   139,   140,     0,   141,     0,
     142,     0,   143,   144,   145,   146,   147,   148,   149,   150,
     151,   152,   153,     0,     0,   154,   155,   156,     0,   157,
     430,   159,   160,   161,   162,   163,   164,   165,   166,   167,
     168,   169,   170,   171,   172,   173,   174,   175,   176,   177,
     178,   179,   180,   181,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   197,
     198,   199,   200,   201,   202,   203,     0,   204,   205,   206,
       0,   207,   208,   209,   210,   211,   212,   213,   214,   215,
     216,   217,   218,   219,   220,   221,   222,   223,   224,   225,
     226,   227,   228,   229,   230,   231,   232,   233,   234,   235,
     236,   237,   238,   239,   240,   241,   242,   243,   244,   245,
     246,   247,   248,   249,   250,   251,   252,   253,   254,     0,
     255,     0,   256,   257,   258,   259,   260,   261,   262,     1,
       2,     0,     0,     0,     3,     0,     0,     0,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,     0,
      15,     0,     0,    16,   428,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,     0,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,     0,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,     0,    74,    75,    76,    77,    78,
       0,    79,    80,    81,     0,     0,    82,    83,     0,     0,
       0,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,     0,    94,     0,     0,     0,    95,     0,    96,    97,
      98,    99,     0,   100,     0,     0,     0,   101,   102,   103,
     104,   105,     0,     0,     0,     0,     0,     0,   106,   107,
     108,   109,   110,   111,   112,   113,   114,     0,   115,   116,
     117,   118,   119,     0,     0,     0,   120,   121,   122,   123,
     124,   125,   126,   127,     0,   128,     0,   129,   130,     0,
       0,   131,   132,   133,     0,     0,     0,   134,     0,   135,
     136,   137,   138,   139,   140,     0,   141,     0,   142,     0,
     143,   144,   145,   146,   147,   148,   149,   150,   151,   152,
     153,     0,     0,   154,   155,   156,     0,   157,   430,   159,
     160,   161,   162,   163,   164,   165,   166,   167,   168,   169,
     170,   171,   172,   173,   174,   175,   176,   177,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,   197,   198,   199,
     200,   201,   202,   203,     0,   204,   205,   206,     0,   207,
     208,   209,   210,   211,   212,   213,   214,   215,   216,   217,
     218,   219,   220,   221,   222,   223,   224,   225,   226,   227,
     228,   229,   230,   231,   232,   233,   234,   235,   236,   237,
     238,   239,   240,   241,   242,   243,   244,   245,   246,   247,
     248,   249,   250,   251,   252,   253,   254,     0,   255,     0,
     256,   257,   258,   259,   260,   261,   262,     1,     2,     0,
       0,     0,     3,     0,     0,     0,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,     0,    15,     0,
       0,   909,   428,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,   429,    32,    33,
      34,    35,    36,     0,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,     0,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,     0,    74,    75,    76,    77,    78,     0,    79,
      80,    81,     0,     0,    82,    83,     0,     0,     0,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,     0,
      94,     0,     0,     0,    95,     0,    96,    97,    98,    99,
       0,   100,     0,     0,     0,   101,   102,   103,   104,   105,
       0,     0,     0,     0,     0,     0,   106,   107,   108,   109,
     110,   111,   112,   113,   114,     0,   115,   116,   117,   118,
     119,     0,     0,     0,   120,   121,   122,   123,   124,   125,
     126,   127,     0,   128,     0,   129,   130,     0,     0,   131,
     132,   133,     0,     0,     0,   134,     0,   135,   136,   137,
     138,   139,   140,     0,   141,     0,   142,     0,   143,   144,
     145,   146,   147,   148,   149,   150,   151,   152,   153,     0,
       0,   154,   155,   156,     0,   157,   430,   159,   160,   161,
     162,   163,   164,   165,   166,   167,   168,   169,   170,   171,
     172,   173,   174,   175,   176,   177,   178,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,   197,   198,   199,   200,   201,
     202,   203,     0,   204,   205,   206,     0,   207,   208,   209,
     210,   211,   212,   213,   214,   215,   216,   217,   218,   219,
     220,   221,   222,   223,   224,   225,   226,   227,   228,   229,
     230,   231,   232,   233,   234,   235,   236,   237,   238,   239,
     240,   241,   242,   243,   244,   245,   246,   247,   248,   249,
     250,   251,   252,   253,   254,     0,   255,     0,   256,   257,
     258,   259,   260,   261,   262,     1,     2,     0,     0,     0,
       3,     0,     0,     0,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,     0,    15,     0,     0,   427,
     428,     0,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,   572,   429,    32,    33,    34,    35,
      36,     0,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,     0,    50,    51,    52,   573,
     574,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
       0,    74,    75,    76,    77,    78,     0,    79,    80,    81,
       0,     0,    82,    83,     0,     0,     0,    84,    85,   520,
      87,    88,    89,    90,    91,   575,    93,     0,    94,     0,
       0,     0,    95,     0,    96,    97,    98,    99,     0,   100,
       0,     0,     0,   101,   102,   103,   104,   105,     0,     0,
       0,     0,     0,     0,   106,   107,   108,   109,   110,   111,
     112,   576,   114,     0,   115,   116,   117,   118,   119,     0,
       0,     0,   120,   121,   122,   123,   124,   125,   126,   127,
       0,   128,     0,   129,   130,     0,     0,   131,   132,   133,
       0,     0,   970,   134,     0,   135,   136,   137,     0,   139,
     140,     0,   141,     0,   142,     0,   143,   144,   145,   146,
     147,   148,   149,   150,   151,   152,   153,     0,     0,   154,
     155,   156,     0,   157,   430,   159,   160,   161,   162,   163,
     164,   165,   166,   167,   168,   169,   170,   171,   172,   173,
     174,   175,   176,   177,   178,   179,   180,   181,   182,   183,
     538,   539,   540,   187,   188,   541,   578,   191,   192,   193,
     194,   195,   196,   197,   198,   199,   200,   201,   202,   203,
       0,   204,   205,   206,     0,   207,   208,   209,   210,   211,
     212,   213,   214,   215,   216,   217,   218,   219,   220,   221,
     222,   223,   224,   225,   226,   227,   228,   229,   230,   231,
     232,   233,   234,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,   245,   246,   247,   248,   249,   250,   251,
     252,   253,   254,     0,   255,     0,   256,   257,   258,   259,
     260,   261,   262,     1,     2,     0,     0,     0,     3,     0,
       0,     0,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,     0,    15,     0,     0,   427,   428,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,   572,   429,    32,    33,    34,    35,    36,     0,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,     0,    50,    51,    52,   573,   574,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,     0,    74,
      75,    76,    77,    78,     0,    79,    80,    81,     0,     0,
      82,    83,     0,     0,     0,    84,    85,   520,    87,    88,
      89,    90,    91,   575,    93,     0,    94,     0,     0,     0,
      95,     0,    96,    97,    98,    99,     0,   100,     0,     0,
       0,   101,   102,   103,   104,   105,     0,     0,     0,     0,
       0,     0,   106,   107,   108,   109,   110,   111,   112,   576,
     114,     0,   115,   116,   117,   118,   119,     0,     0,     0,
     120,   121,   122,   123,   124,   125,   126,   127,     0,   128,
       0,   129,   130,     0,     0,   131,   132,   133,     0,     0,
       0,   134,  1211,   135,   136,   137,     0,   139,   140,     0,
     141,     0,   142,     0,   143,   144,   145,   146,   147,   148,
     149,   150,   151,   152,   153,     0,     0,   154,   155,   156,
       0,   157,   430,   159,   160,   161,   162,   163,   164,   165,
     166,   167,   168,   169,   170,   171,   172,   173,   174,   175,
     176,   177,   178,   179,   180,   181,   182,   183,   538,   539,
     540,   187,   188,   541,   578,   191,   192,   193,   194,   195,
     196,   197,   198,   199,   200,   201,   202,   203,     0,   204,
     205,   206,     0,   207,   208,   209,   210,   211,   212,   213,
     214,   215,   216,   217,   218,   219,   220,   221,   222,   223,
     224,   225,   226,   227,   228,   229,   230,   231,   232,   233,
     234,   235,   236,   237,   238,   239,   240,   241,   242,   243,
     244,   245,   246,   247,   248,   249,   250,   251,   252,   253,
     254,     0,   255,     0,   256,   257,   258,   259,   260,   261,
     262,     1,     2,     0,     0,     0,     3,     0,     0,     0,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,     0,    15,     0,     0,   427,   428,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
    1379,   429,    32,    33,    34,    35,    36,     0,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,     0,    50,    51,    52,  1380,  1381,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,     0,    74,    75,    76,
      77,    78,     0,    79,    80,    81,     0,     0,    82,    83,
       0,     0,     0,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,     0,    94,     0,     0,     0,    95,     0,
      96,    97,    98,    99,     0,   100,     0,     0,     0,   101,
     102,   103,   104,   105,     0,     0,     0,     0,     0,     0,
     106,   107,   108,   109,   110,   111,   112,   113,   114,     0,
     115,   116,   117,   118,   119,     0,     0,     0,   120,   121,
     122,   123,   124,   125,   126,   127,     0,   128,     0,   129,
     130,     0,     0,   131,   132,   133,     0,     0,     0,   134,
       0,   135,   136,   137,   138,   139,   140,     0,   141,     0,
     142,     0,   143,   144,   145,   146,   147,   148,   149,   150,
     151,   152,   153,     0,     0,   154,   155,   156,     0,   157,
     430,   159,   160,   161,   162,   163,   164,   165,   166,   167,
     168,   169,   170,   171,   172,   173,   174,   175,   176,   177,
     178,   179,   180,   181,   182,   183,   184,   185,   186,   187,
     188,   189,  1382,   191,   192,   193,   194,   195,   196,   197,
     198,   199,   200,   201,   202,   203,     0,   204,   205,   206,
       0,   207,   208,   209,   210,   211,   212,   213,   214,   215,
     216,   217,   218,   219,   220,   221,   222,   223,   224,   225,
     226,   227,   228,   229,   230,   231,   232,   233,   234,   235,
     236,   237,   238,   239,   240,   241,   242,   243,   244,   245,
     246,   247,   248,   249,   250,   251,   252,   253,   254,     0,
     255,     0,   256,   257,   258,   259,   260,   261,   262,     1,
       2,     0,     0,     0,     3,     0,     0,     0,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,     0,
      15,     0,     0,   427,   428,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,   572,   429,
      32,    33,    34,    35,    36,     0,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,     0,
      50,    51,    52,   573,   574,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,     0,    74,    75,    76,    77,    78,
       0,    79,    80,    81,     0,     0,    82,    83,     0,     0,
       0,    84,    85,   520,    87,    88,    89,    90,    91,   575,
      93,     0,    94,     0,     0,     0,    95,     0,    96,    97,
      98,    99,     0,   100,     0,     0,     0,   101,   102,   103,
     104,   105,     0,     0,     0,     0,  1477,     0,   106,   107,
     108,   109,   110,   111,   112,   576,   114,     0,   115,   116,
     117,   118,   119,     0,     0,     0,   120,   121,   122,   123,
     124,   125,   126,   127,     0,   128,     0,   129,   130,     0,
       0,   131,   132,   133,     0,     0,     0,   134,     0,   135,
     136,   137,     0,   139,   140,     0,   141,     0,   142,     0,
     143,   144,   145,   146,   147,   148,   149,   150,   151,   152,
     153,     0,     0,   154,   155,   156,     0,   157,   430,   159,
     160,   161,   162,   163,   164,   165,   166,   167,   168,   169,
     170,   171,   172,   173,   174,   175,   176,   177,   178,   179,
     180,   181,   182,   183,   538,   539,   540,   187,   188,   541,
     578,   191,   192,   193,   194,   195,   196,   197,   198,   199,
     200,   201,   202,   203,     0,   204,   205,   206,     0,   207,
     208,   209,   210,   211,   212,   213,   214,   215,   216,   217,
     218,   219,   220,   221,   222,   223,   224,   225,   226,   227,
     228,   229,   230,   231,   232,   233,   234,   235,   236,   237,
     238,   239,   240,   241,   242,   243,   244,   245,   246,   247,
     248,   249,   250,   251,   252,   253,   254,     0,   255,     0,
     256,   257,   258,   259,   260,   261,   262,     1,     2,     0,
       0,     0,     3,     0,     0,     0,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,     0,    15,     0,
       0,   427,   428,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,   506,    29,   508,   429,    32,   509,
      34,    35,    36,     0,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,   511,     0,    50,    51,
      52,   513,   514,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,     0,    74,    75,    76,    77,    78,     0,    79,
      80,    81,     0,     0,    82,    83,     0,     0,     0,    84,
      85,   520,    87,    88,    89,    90,    91,   575,    93,     0,
      94,     0,     0,     0,    95,     0,    96,    97,    98,    99,
       0,   100,     0,     0,     0,   523,   102,   103,   104,   105,
       0,     0,     0,     0,     0,     0,   106,   107,   108,   109,
     110,   111,   112,   576,   114,     0,   115,   116,   710,   118,
     119,     0,     0,     0,   120,   121,   122,   123,   124,   125,
     126,   127,     0,   128,     0,   711,   130,     0,     0,   131,
     132,   133,     0,     0,     0,   134,     0,   135,   136,   137,
       0,   139,   140,     0,   141,     0,   142,     0,   143,   144,
     145,   146,   147,   148,   149,   150,   151,   152,   153,     0,
       0,   154,   155,   156,     0,   157,   430,   159,   160,   161,
     162,   163,   164,   165,   166,   167,   168,   169,   533,   171,
     534,   173,   535,   536,   176,   177,   178,   179,   180,   181,
     537,   183,   538,   539,   540,   187,   188,   541,   542,   191,
     192,   193,   194,   195,   196,   197,   198,   199,   200,   201,
     202,   543,     0,   204,   544,   206,     0,   207,   208,   209,
     210,   211,   212,   213,   214,   215,   216,   217,   218,   219,
     220,   221,   222,   223,   224,   225,   226,   227,   228,   229,
     230,   231,   232,   233,   234,   235,   236,   237,   238,   239,
     240,   241,   242,   243,   244,   245,   246,   247,   248,   249,
     250,   251,   252,   253,   254,     0,   255,     0,   256,   545,
     258,   259,   260,   261,   262,     1,     2,     0,     0,     0,
       3,     0,     0,     0,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,     0,    15,     0,     0,   427,
     428,     0,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,   572,   429,    32,    33,    34,    35,
      36,     0,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,     0,    50,    51,    52,   573,
     574,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
       0,    74,    75,    76,    77,    78,     0,    79,    80,    81,
       0,     0,    82,    83,     0,     0,     0,    84,    85,   520,
      87,    88,    89,    90,    91,   575,    93,     0,    94,     0,
       0,     0,    95,     0,    96,    97,    98,    99,     0,   100,
       0,     0,     0,   101,   102,   103,   104,   105,     0,     0,
       0,     0,     0,     0,   106,   107,   108,   109,   110,   111,
     112,   576,   114,     0,   115,   116,   117,   118,   119,     0,
       0,     0,   120,   121,   122,   123,   124,   125,   126,   127,
       0,   128,     0,   129,   130,     0,     0,   131,   132,   133,
       0,     0,     0,   134,     0,   135,   136,   137,     0,   139,
     140,     0,   141,     0,   142,     0,   143,   144,   145,   146,
     147,   148,   149,   150,   151,   152,   153,     0,     0,   154,
     155,   156,     0,   157,   430,   159,   160,   161,   162,   163,
     164,   165,   166,   167,   168,   169,   170,   171,   172,   173,
     174,   175,   176,   177,   178,   179,   180,   181,   182,   183,
     538,   539,   540,   187,   188,   541,   578,   191,   192,   193,
     194,   195,   196,   197,   198,   199,   200,   201,   202,   203,
       0,   204,   205,   206,     0,   207,   208,   209,   210,   211,
     212,   213,   214,   215,   216,   217,   218,   219,   220,   221,
     222,   223,   224,   225,   226,   227,   228,   229,   230,   231,
     232,   233,   234,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,   245,   246,   247,   248,   249,   250,   251,
     252,   253,   254,     0,   255,     0,   256,   257,   258,   259,
     260,   261,   262,     1,     2,     0,     0,     0,     3,     0,
       0,     0,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,     0,    15,     0,     0,   427,   428,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
     506,    29,   508,   429,    32,   509,    34,    35,    36,     0,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,   511,     0,    50,    51,    52,   513,   514,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,     0,    74,
      75,    76,    77,    78,     0,    79,    80,    81,     0,     0,
      82,    83,     0,     0,     0,    84,    85,   520,    87,    88,
      89,    90,    91,   575,    93,     0,    94,     0,     0,     0,
      95,     0,    96,    97,    98,    99,     0,   100,     0,     0,
       0,   523,   102,   103,   104,   105,     0,     0,     0,     0,
       0,     0,   106,   107,   108,   109,   110,   111,   112,   576,
     114,     0,   115,   116,   117,   118,   119,     0,     0,     0,
     120,   121,   122,   123,   124,   125,   126,   127,     0,   128,
       0,   129,   130,     0,     0,   131,   132,   133,     0,     0,
       0,   134,     0,   135,   136,   137,     0,   139,   140,     0,
     141,     0,   142,     0,   143,   144,   145,   146,   147,   148,
     149,   150,   151,   152,   153,     0,     0,   154,   155,   156,
       0,   157,   430,   159,   160,   161,   162,   163,   164,   165,
     166,   167,   168,   169,   533,   171,   534,   173,   535,   536,
     176,   177,   178,   179,   180,   181,   537,   183,   538,   539,
     540,   187,   188,   541,   542,   191,   192,   193,   194,   195,
     196,   197,   198,   199,   200,   201,   202,   543,     0,   204,
     205,   206,     0,   207,   208,   209,   210,   211,   212,   213,
     214,   215,   216,   217,   218,   219,   220,   221,   222,   223,
     224,   225,   226,   227,   228,   229,   230,   231,   232,   233,
     234,   235,   236,   237,   238,   239,   240,   241,   242,   243,
     244,   245,   246,   247,   248,   249,   250,   251,   252,   253,
     254,     0,   255,     0,   256,   545,   258,   259,   260,   261,
     262,     1,     2,     0,     0,     0,     3,     0,     0,     0,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,     0,    15,     0,     0,   427,   428,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,   506,    29,
     508,   429,    32,   509,    34,    35,    36,     0,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
     511,     0,    50,    51,    52,   513,   514,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,     0,    74,    75,    76,
      77,    78,     0,    79,    80,    81,     0,     0,    82,    83,
       0,     0,     0,    84,    85,   520,    87,    88,    89,    90,
      91,   575,    93,     0,    94,     0,     0,     0,    95,     0,
      96,    97,    98,    99,     0,   100,     0,     0,     0,   523,
     102,   103,   104,   105,     0,     0,     0,     0,     0,     0,
     106,   107,   108,   109,   110,   111,   112,   576,   114,     0,
     115,   116,   117,   118,   119,     0,     0,     0,   120,   121,
     122,   123,   124,   125,   126,   127,     0,   128,     0,   129,
     130,     0,     0,   131,   132,   133,     0,     0,     0,   134,
       0,   135,   136,   137,     0,   139,   140,     0,   141,     0,
     142,     0,   143,   144,   145,   146,   147,   148,   149,   150,
     151,   152,   153,     0,     0,   154,   155,   156,     0,   157,
     430,   159,   160,   161,   162,   163,   164,   165,   166,   167,
     168,   169,   533,   171,   534,   173,   535,   536,   176,   177,
     178,   179,   180,   181,   537,   183,   538,   539,   540,   187,
     188,   541,   542,   191,   192,   193,   194,   195,   196,   197,
     198,   199,   200,   201,   202,   543,     0,   204,   544,   206,
       0,   207,   208,   209,   210,   211,   212,   213,   214,   215,
     216,   217,   218,   219,   220,   221,   222,   223,   224,   225,
     226,   227,   228,   229,   230,   231,   232,   233,   234,   235,
     236,   237,   238,   239,   240,   241,   242,   243,   244,   245,
     246,   247,   248,   249,   250,   251,   252,   253,   254,     0,
     255,     0,   256,   545,   258,   259,   260,   261,   262,     1,
       2,     0,     0,     0,     3,     0,     0,     0,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,     0,
      15,     0,     0,   427,   428,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,   572,   429,
      32,    33,    34,    35,    36,     0,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,     0,
      50,    51,    52,   573,   574,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,     0,    74,    75,    76,    77,    78,
       0,    79,    80,    81,     0,     0,    82,    83,     0,     0,
       0,    84,    85,   520,    87,    88,    89,    90,    91,  1159,
      93,     0,    94,     0,     0,     0,    95,     0,    96,    97,
      98,    99,     0,   100,     0,     0,     0,   101,   102,   103,
     104,   105,     0,     0,     0,     0,     0,     0,   106,   107,
     108,   109,   110,   111,   112,   576,   114,     0,   115,   116,
     117,   118,   119,     0,     0,     0,   120,   121,   122,   123,
     124,   125,   126,   127,     0,   128,     0,   129,   130,     0,
       0,   131,   132,   133,     0,     0,     0,   134,     0,   135,
     136,   137,     0,   139,   140,     0,   141,     0,   142,     0,
     143,   144,   145,   146,   147,   148,   149,   150,   151,   152,
     153,     0,     0,   154,   155,   156,     0,   157,   430,   159,
     160,   161,   162,   163,   164,   165,   166,   167,   168,   169,
     170,   171,   172,   173,   174,   175,   176,   177,   178,   179,
     180,   181,   182,   183,   538,   539,   540,   187,   188,   541,
     578,   191,   192,   193,   194,   195,   196,   197,   198,   199,
     200,   201,   202,   203,     0,   204,   205,   206,     0,   207,
     208,   209,   210,   211,   212,   213,   214,   215,   216,   217,
     218,   219,   220,   221,   222,   223,   224,   225,   226,   227,
     228,   229,   230,   231,   232,   233,   234,   235,   236,   237,
     238,   239,   240,   241,   242,   243,   244,   245,   246,   247,
     248,   249,   250,   251,   252,   253,   254,     0,   255,     0,
     256,   257,   258,   259,   260,   261,   262,     1,     2,     0,
       0,     0,     3,     0,     0,     0,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,     0,    15,     0,
       0,   427,   428,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,   506,    29,   508,   429,    32,   509,
      34,    35,    36,     0,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,   511,     0,    50,    51,
      52,   513,   514,    55,   515,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,     0,    74,    75,    76,    77,    78,     0,    79,
      80,    81,     0,     0,    82,    83,     0,     0,     0,    84,
      85,   520,    87,    88,    89,    90,    91,   575,    93,     0,
      94,     0,     0,     0,    95,     0,    96,    97,    98,    99,
       0,   100,     0,     0,     0,   523,   102,   103,   104,   105,
       0,     0,     0,     0,     0,     0,   106,   107,   108,   109,
     110,   111,   112,   576,   114,     0,   115,   116,     0,   118,
     119,     0,     0,     0,   120,   121,   122,   123,   124,   125,
     126,   127,     0,   128,     0,     0,     0,     0,     0,   131,
     132,   133,     0,     0,     0,   134,     0,   135,   136,   137,
       0,   139,   140,     0,   141,     0,   142,     0,   143,   144,
     145,   146,   147,   148,   149,   150,   151,   152,   153,     0,
       0,   154,   155,   156,     0,   157,   430,   159,   160,   161,
     162,   163,   164,   165,   166,   167,   168,   169,   533,   171,
     534,   173,   535,   536,   176,   177,   178,   179,   180,   181,
     537,   183,   538,   539,   540,   187,   188,   541,   542,   191,
     192,   193,   194,   195,   196,   197,   198,   199,   200,   201,
     202,   543,     0,   204,   544,   206,     0,   207,   208,   209,
     210,   211,   212,   213,   214,   215,   216,   217,   218,   219,
     220,   221,   222,   223,   224,   225,   226,   227,   228,   229,
     230,   231,   232,   233,   234,   235,   236,   237,   238,   239,
     240,   241,   242,   243,   244,   245,   246,   247,   248,   249,
     250,   251,   252,   253,   254,     0,   255,     0,   256,   545,
     258,   259,   260,   261,   262,     1,     2,     0,     0,     0,
       3,     0,     0,     0,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,     0,    15,     0,     0,   427,
     428,     0,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,   506,    29,   508,   429,    32,   509,    34,    35,
      36,     0,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,   511,     0,    50,    51,    52,   513,
     514,    55,   515,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
       0,    74,    75,    76,    77,    78,     0,    79,    80,    81,
       0,     0,    82,    83,     0,     0,     0,    84,    85,   520,
      87,    88,    89,    90,    91,   575,    93,     0,    94,     0,
       0,     0,    95,     0,    96,    97,    98,    99,     0,   100,
       0,     0,     0,   523,   102,   103,   104,   105,     0,     0,
       0,     0,     0,     0,   106,   107,   108,   109,   110,   111,
     112,   576,   114,     0,   115,   116,     0,   118,   119,     0,
       0,     0,   120,   121,   122,   123,   124,   125,   126,   127,
       0,   128,     0,     0,     0,     0,     0,   131,   132,   133,
       0,     0,     0,   134,     0,   135,   136,   137,     0,     0,
       0,     0,   141,     0,   142,     0,   143,   144,   145,   146,
     147,   148,   149,   150,   151,   152,   153,     0,     0,   154,
     155,   156,     0,   157,   430,   159,   160,   161,   162,   163,
     164,   165,   166,   167,   168,   169,   533,   171,   534,   173,
     535,   536,   176,   177,   178,   179,   180,   181,   537,   183,
     538,   539,   540,   187,   188,   541,   542,   191,   192,   193,
     194,   195,   196,   197,   198,   199,   200,   201,   202,   543,
       0,   204,   544,   206,     0,   207,   208,   209,   210,   211,
     212,   213,   214,   215,   216,   217,   218,   219,   220,   221,
     222,   223,   224,   225,   226,   227,   228,   229,   230,   231,
     232,   233,   234,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,   245,   246,   247,   248,   249,   250,   251,
     252,   253,   254,     0,   255,     0,   256,   545,   258,   259,
     260,   261,   262,     1,     2,     0,     0,     0,     3,     0,
       0,     0,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,     0,    15,     0,     0,   427,   428,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
     506,    29,   508,   429,    32,   509,    34,    35,    36,     0,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,   511,     0,    50,    51,    52,   513,   514,    55,
     515,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,     0,    74,
      75,    76,    77,    78,     0,    79,    80,    81,     0,     0,
      82,    83,     0,     0,     0,    84,    85,   520,    87,    88,
      89,    90,    91,   575,    93,     0,    94,     0,     0,     0,
      95,     0,    96,    97,    98,    99,     0,   100,     0,     0,
       0,   523,     0,   103,   104,   105,     0,     0,     0,     0,
       0,     0,   106,   107,   108,   109,     0,   111,   112,   576,
     114,     0,   115,   116,     0,   118,   119,     0,     0,     0,
     120,   121,   122,   123,   124,   125,   126,   127,     0,   128,
       0,     0,     0,     0,     0,   131,   132,   133,     0,     0,
       0,   134,     0,   135,   136,   137,     0,     0,     0,     0,
     141,     0,   142,     0,   143,     0,   145,     0,   147,   148,
     149,   150,   151,   152,   153,     0,     0,   154,   155,   156,
       0,   157,   430,   159,   160,   161,   162,   163,   164,   165,
     166,   167,   168,   169,   533,   171,   534,   173,   535,   536,
     176,   177,   178,   179,   180,   181,   537,   183,   538,   539,
     540,   187,   188,   541,   542,   191,   192,   193,   194,   195,
     196,   197,   198,   199,   200,   201,   202,   543,     0,   204,
     544,   206,     0,   207,   208,   209,   210,   211,   212,   213,
     214,   215,   216,   217,   218,   219,   220,   221,   222,   223,
     224,   225,   226,   227,   228,   229,   230,   231,   232,   233,
     234,   235,   236,   237,   238,   239,   240,   241,   242,   243,
     244,   245,   246,   247,   248,   249,   250,   251,   252,   253,
     254,     0,   255,     0,   256,   545,   258,   259,   260,   261,
     262,     1,     2,     0,     0,     0,     3,     0,     0,     0,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,     0,    15,     0,     0,   427,   428,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,   506,    29,
     508,   429,    32,   509,    34,    35,    36,     0,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
     511,     0,    50,    51,    52,   513,   514,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,     0,    74,    75,    76,
      77,    78,     0,    79,    80,    81,     0,     0,    82,    83,
       0,     0,     0,    84,    85,   520,    87,    88,    89,    90,
       0,   575,    93,     0,    94,     0,     0,     0,    95,     0,
      96,    97,    98,    99,     0,   100,     0,     0,     0,   523,
       0,   103,   104,   105,     0,     0,     0,     0,     0,     0,
       0,   107,   108,   109,     0,   111,   112,   576,     0,     0,
       0,   116,     0,     0,   119,     0,     0,     0,   120,   121,
     122,   123,   124,   125,   126,   127,     0,   128,     0,     0,
     130,     0,     0,   131,   132,   133,     0,     0,     0,   134,
       0,   135,   136,   137,     0,     0,     0,     0,     0,     0,
     142,     0,   143,     0,   145,     0,   147,   148,   149,   150,
     151,   152,   153,     0,     0,   154,   155,   156,     0,   157,
     430,   159,   160,   161,   162,   163,   164,   165,   166,   167,
     168,   169,   533,   171,   534,   173,   535,   536,   176,   177,
     178,   179,   180,   181,   537,   183,   538,   539,   540,   187,
     188,   541,   542,   191,   192,   193,   194,   195,   196,   197,
     198,   199,   200,   201,   202,   543,     0,   204,   205,   206,
       0,   207,   208,   209,   210,   211,   212,   213,   214,   215,
     216,   217,   218,   219,   220,   221,   222,   223,   224,   225,
     226,   227,   228,   229,   230,   231,   232,   233,   234,   235,
     236,   237,   238,   239,   240,   241,   242,   243,   244,   245,
     246,   247,   248,   249,   250,   251,   252,   253,   254,     0,
     255,     0,   256,   545,   258,   259,   260,   261,   262,     1,
       2,     0,     0,     0,     3,     0,     0,     0,     4,     0,
       0,     0,     0,     0,     0,    11,    12,     0,     0,     0,
       0,     0,     0,   427,   428,     0,    18,    19,    20,   503,
      22,    23,    24,   504,    26,   505,   506,   507,   508,   429,
      32,   509,    34,    35,    36,     0,    37,    38,    39,    40,
     510,    42,    43,    44,    45,    46,    47,    48,   511,     0,
      50,   512,    52,   513,   514,    55,   515,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,   516,   517,    73,     0,    74,    75,    76,   518,     0,
       0,    79,    80,    81,     0,     0,   519,    83,     0,     0,
       0,     0,    85,   520,    87,   521,   522,    90,    91,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    96,    97,
      98,    99,     0,   100,     0,     0,     0,   523,   102,   103,
     524,   525,     0,     0,     0,     0,     0,     0,   106,   107,
     108,     0,   110,   111,   112,     0,     0,     0,     0,     0,
       0,   118,   526,     0,     0,     0,   120,   121,   122,   123,
     527,   125,   126,   528,     0,     0,     0,     0,     0,     0,
       0,   529,   530,   133,     0,     0,     0,   134,     0,   135,
     136,   531,     0,     0,     0,     0,   141,     0,   142,     0,
     143,   144,   145,   146,   532,   148,   149,   150,   151,   152,
     153,     0,     0,   154,   155,     0,     0,   157,   430,   159,
     160,   161,   162,   163,   164,   165,   166,   167,   168,   169,
     533,   171,   534,   173,   535,   536,   176,   177,   178,   179,
     180,   181,   537,   183,   538,   539,   540,   187,   188,   541,
     542,   191,   192,   193,   194,   195,   196,   197,   198,   199,
     200,   201,   202,   543,     0,   204,   544,   206,     0,   207,
     208,   209,   210,   211,   212,   213,   214,   215,   216,   217,
     218,   219,   220,   221,   222,   223,   224,   225,   226,   227,
     228,   229,   230,   231,   232,   233,   234,   235,   236,   237,
     238,   239,   240,   241,   242,   243,   244,   245,   246,   247,
     248,   249,   250,   251,   252,   253,     0,     0,     0,     0,
     256,   545,   258,   259,   260,   261,   262,     1,     2,     0,
       0,     0,     3,     0,     0,     0,     4,     0,     0,     0,
       0,     0,     0,    11,    12,     0,     0,     0,     0,     0,
       0,   427,   428,     0,    18,    19,    20,   553,    22,    23,
      24,   504,   554,   555,   506,   507,   508,   429,    32,   509,
      34,   556,    36,     0,    37,    38,    39,    40,   557,    42,
     558,   559,    45,    46,    47,    48,   511,     0,    50,   560,
      52,   513,   514,    55,   515,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,   516,
     517,    73,     0,    74,    75,    76,   561,     0,     0,    79,
      80,    81,     0,     0,   519,    83,     0,     0,     0,     0,
      85,   520,    87,   521,   522,    90,    91,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    96,    97,    98,    99,
       0,   100,     0,     0,     0,   523,   102,   103,   524,   525,
       0,     0,     0,     0,     0,     0,   106,   107,   108,     0,
     110,   111,   112,     0,     0,     0,     0,     0,     0,   118,
     526,     0,     0,     0,   120,   121,   122,   123,   527,   125,
     126,   528,     0,     0,     0,     0,     0,     0,     0,   529,
     530,   133,     0,     0,     0,   134,     0,   135,   136,   531,
       0,     0,     0,     0,   141,     0,   142,     0,   143,   144,
     145,   146,   532,   148,   149,   150,   151,   152,   153,     0,
       0,   154,   155,     0,     0,   157,   430,   159,   160,   161,
     162,   163,   164,   165,   166,   167,   168,   169,   533,   562,
     534,   173,   535,   536,   176,   177,   178,   179,   180,   181,
     537,   183,   538,   539,   540,   187,   188,   541,   542,   191,
     192,   193,   194,   195,   196,   197,   198,   199,   200,   201,
     202,   543,     0,   204,   544,   206,     0,   207,   208,   209,
     210,   211,   212,   213,   214,   215,   216,   217,   218,   219,
     220,   221,   222,   223,   224,   225,   226,   227,   228,   229,
     230,   231,   232,   233,   234,   235,   236,   237,   238,   239,
     240,   241,   242,   243,   244,   245,   246,   247,   248,   249,
     250,   251,   252,   253,     3,     0,     0,     0,   256,   545,
     258,   259,   260,   261,   262,    11,    12,    13,    14,     0,
       0,     0,     0,   427,   428,     0,    18,    19,    20,   503,
      22,    23,    24,   504,    26,   505,   506,  1185,   508,   429,
      32,   509,    34,    35,    36,     0,    37,    38,    39,    40,
     510,    42,    43,    44,    45,    46,    47,    48,   511,     0,
      50,   512,    52,   513,   514,    55,   515,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,   516,   517,    73,     0,    74,    75,    76,   518,     0,
       0,    79,    80,    81,     0,     0,   519,    83,     0,     0,
       0,     0,    85,   520,    87,   521,   522,    90,    91,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    96,    97,
      98,    99,     0,   100,     0,     0,     0,   523,   102,   103,
     524,   525,     0,     0,     0,     0,     0,     0,   106,   107,
     108,     0,   110,   111,  1186,     0,     0,     0,  1187,     0,
       0,   118,   526,     0,     0,     0,   120,   121,   122,   123,
     527,   125,   126,   528,     0,     0,     0,     0,     0,     0,
       0,   529,   530,   133,     0,     0,     0,   134,  1630,   135,
     136,   531,     0,     0,     0,     0,  1631,     0,   142,     0,
     143,   144,   145,   146,   532,   148,   149,   150,   151,   152,
     153,     0,     0,  1188,   155,     0,     0,   157,   430,   159,
     160,   161,   162,   163,   164,   165,   166,   167,   168,   169,
     533,   171,   534,   173,   535,   536,   176,   177,   178,   179,
     180,   181,   537,   183,   538,   539,   540,   187,   188,   541,
     542,   191,   192,   193,   194,   195,   196,   197,   198,   199,
     200,   201,   202,   543,     0,   204,   544,   206,     0,   207,
     208,   209,   210,   211,   212,   213,   214,   215,   216,   217,
     218,   219,   220,   221,   222,   223,   224,   225,   226,   227,
     228,   229,   230,   231,   232,   233,   234,   235,   236,   237,
     238,   239,   240,   241,   242,   243,   244,   245,   246,   247,
     248,   249,   250,   251,   252,   253,     3,     0,     0,     0,
     256,   545,   258,  1189,  1190,  1191,  1192,    11,    12,     0,
       0,     0,    15,     0,     0,   427,   428,     0,    18,    19,
      20,   553,    22,    23,    24,   504,   554,   555,   506,   507,
     508,   429,    32,   509,    34,   556,    36,     0,    37,    38,
      39,    40,   557,    42,   558,   559,    45,    46,    47,    48,
     511,     0,    50,   560,    52,   513,   514,    55,   515,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,   516,   517,    73,     0,    74,    75,    76,
     561,     0,     0,    79,    80,    81,     0,     0,   519,    83,
       0,     0,     0,     0,    85,   520,    87,   521,   522,    90,
      91,   575,    93,     0,     0,     0,     0,     0,     0,     0,
      96,    97,    98,    99,     0,   100,     0,     0,     0,   523,
     102,   103,   524,   525,     0,     0,     0,     0,     0,     0,
     106,   107,   108,     0,   110,   111,   112,     0,     0,     0,
     965,     0,     0,   118,   526,     0,     0,     0,   120,   121,
     122,   123,   527,   125,   126,   528,     0,     0,     0,     0,
       0,     0,     0,   529,   530,   133,     0,     0,     0,   134,
       0,   135,   136,   531,     0,     0,     0,     0,     0,     0,
     142,     0,   143,   144,   145,   146,   532,   148,   149,   150,
     151,   152,   153,     0,     0,   154,   155,     0,     0,   157,
     430,   159,   160,   161,   162,   163,   164,   165,   166,   167,
     168,   169,   533,   562,   534,   173,   535,   536,   176,   177,
     178,   179,   180,   181,   537,   183,   538,   539,   540,   187,
     188,   541,   542,   191,   192,   193,   194,   195,   196,   197,
     198,   199,   200,   201,   202,   543,     0,   204,   544,   206,
       0,   207,   208,   209,   210,   211,   212,   213,   214,   215,
     216,   217,   218,   219,   220,   221,   222,   223,   224,   225,
     226,   227,   228,   229,   230,   231,   232,   233,   234,   235,
     236,   237,   238,   239,   240,   241,   242,   243,   244,   245,
     246,   247,   248,   249,   250,   251,   252,   253,     3,     0,
       0,     0,   256,   545,   258,   259,   260,   261,   262,    11,
      12,    13,    14,     0,     0,     0,     0,   427,   428,     0,
      18,    19,    20,   503,    22,    23,    24,   504,    26,   505,
     506,  1185,   508,   429,    32,   509,    34,    35,    36,     0,
      37,    38,    39,    40,   510,    42,    43,    44,    45,    46,
      47,    48,   511,     0,    50,   512,    52,   513,   514,    55,
     515,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,   516,   517,    73,     0,    74,
      75,    76,   518,     0,     0,    79,    80,    81,     0,     0,
     519,    83,     0,     0,     0,     0,    85,   520,    87,   521,
     522,    90,    91,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    96,    97,    98,    99,     0,   100,     0,     0,
       0,   523,   102,   103,   524,   525,     0,     0,     0,     0,
       0,     0,   106,   107,   108,     0,   110,   111,  1186,     0,
       0,     0,  1226,     0,     0,   118,   526,     0,     0,     0,
     120,   121,   122,   123,   527,   125,   126,   528,     0,     0,
       0,     0,     0,     0,     0,   529,   530,   133,     0,     0,
       0,   134,  1461,   135,   136,   531,     0,     0,     0,     0,
       0,     0,   142,     0,   143,   144,   145,   146,   532,   148,
     149,   150,   151,   152,   153,     0,     0,   154,   155,     0,
       0,   157,   430,   159,   160,   161,   162,   163,   164,   165,
     166,   167,   168,   169,   533,   171,   534,   173,   535,   536,
     176,   177,   178,   179,   180,   181,   537,   183,   538,   539,
     540,   187,   188,   541,   542,   191,   192,   193,   194,   195,
     196,   197,   198,   199,   200,   201,   202,   543,     0,   204,
     544,   206,     0,   207,   208,   209,   210,   211,   212,   213,
     214,   215,   216,   217,   218,   219,   220,   221,   222,   223,
     224,   225,   226,   227,   228,   229,   230,   231,   232,   233,
     234,   235,   236,   237,   238,   239,   240,   241,   242,   243,
     244,   245,   246,   247,   248,   249,   250,   251,   252,   253,
       3,     0,     0,     0,   256,   545,   258,  1189,  1190,  1191,
    1192,    11,    12,    13,    14,     0,     0,     0,     0,   427,
     428,     0,    18,    19,    20,   503,    22,    23,    24,   504,
      26,   505,   506,  1185,   508,   429,    32,   509,    34,    35,
      36,     0,    37,    38,    39,    40,   510,    42,    43,    44,
      45,    46,    47,    48,   511,     0,    50,   512,    52,   513,
     514,    55,   515,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,   516,   517,    73,
       0,    74,    75,    76,   518,     0,     0,    79,    80,    81,
       0,     0,   519,    83,     0,     0,     0,     0,    85,   520,
      87,   521,   522,    90,    91,  1538,     0,     0,     0,     0,
       0,     0,     0,     0,    96,    97,    98,    99,     0,   100,
       0,     0,     0,   523,   102,   103,   524,   525,     0,     0,
       0,     0,     0,     0,   106,   107,   108,     0,   110,   111,
    1186,     0,     0,     0,  1187,     0,     0,   118,   526,     0,
       0,     0,   120,   121,   122,   123,   527,   125,   126,   528,
       0,     0,     0,     0,     0,     0,     0,   529,   530,   133,
       0,     0,     0,   134,     0,   135,   136,   531,     0,     0,
       0,     0,     0,     0,   142,     0,   143,   144,   145,   146,
     532,   148,   149,   150,   151,   152,   153,     0,     0,  1188,
     155,     0,     0,   157,   430,   159,   160,   161,   162,   163,
     164,   165,   166,   167,   168,   169,   533,   171,   534,   173,
     535,   536,   176,   177,   178,   179,   180,   181,   537,   183,
     538,   539,   540,   187,   188,   541,   542,   191,   192,   193,
     194,   195,   196,   197,   198,   199,   200,   201,   202,   543,
       0,   204,   544,   206,     0,   207,   208,   209,   210,   211,
     212,   213,   214,   215,   216,   217,   218,   219,   220,   221,
     222,   223,   224,   225,   226,   227,   228,   229,   230,   231,
     232,   233,   234,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,   245,   246,   247,   248,   249,   250,   251,
     252,   253,     3,     0,     0,     0,   256,   545,   258,  1189,
    1190,  1191,  1192,    11,    12,    13,    14,     0,     0,     0,
       0,   427,   428,     0,    18,    19,    20,   503,    22,    23,
      24,   504,    26,   505,   506,  1185,   508,   429,    32,   509,
      34,    35,    36,     0,    37,    38,    39,    40,   510,    42,
      43,    44,    45,    46,    47,    48,   511,     0,    50,   512,
      52,   513,   514,    55,   515,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,   516,
     517,    73,     0,    74,    75,    76,   518,     0,     0,    79,
      80,    81,     0,     0,   519,    83,     0,     0,     0,     0,
      85,   520,    87,   521,   522,    90,    91,  1683,     0,     0,
       0,     0,     0,     0,     0,     0,    96,    97,    98,    99,
       0,   100,     0,     0,     0,   523,   102,   103,   524,   525,
       0,     0,     0,     0,     0,     0,   106,   107,   108,     0,
     110,   111,  1186,     0,     0,     0,  1187,     0,     0,   118,
     526,     0,     0,     0,   120,   121,   122,   123,   527,   125,
     126,   528,     0,     0,     0,     0,     0,     0,     0,   529,
     530,   133,     0,     0,     0,   134,     0,   135,   136,   531,
       0,     0,     0,     0,     0,     0,   142,     0,   143,   144,
     145,   146,   532,   148,   149,   150,   151,   152,   153,     0,
       0,  1188,   155,     0,     0,   157,   430,   159,   160,   161,
     162,   163,   164,   165,   166,   167,   168,   169,   533,   171,
     534,   173,   535,   536,   176,   177,   178,   179,   180,   181,
     537,   183,   538,   539,   540,   187,   188,   541,   542,   191,
     192,   193,   194,   195,   196,   197,   198,   199,   200,   201,
     202,   543,     0,   204,   544,   206,     0,   207,   208,   209,
     210,   211,   212,   213,   214,   215,   216,   217,   218,   219,
     220,   221,   222,   223,   224,   225,   226,   227,   228,   229,
     230,   231,   232,   233,   234,   235,   236,   237,   238,   239,
     240,   241,   242,   243,   244,   245,   246,   247,   248,   249,
     250,   251,   252,   253,     3,     0,     0,     0,   256,   545,
     258,  1189,  1190,  1191,  1192,    11,    12,    13,    14,     0,
       0,     0,     0,   427,   428,     0,    18,    19,    20,   503,
      22,    23,    24,   504,    26,   505,   506,  1185,   508,   429,
      32,   509,    34,    35,    36,     0,    37,    38,    39,    40,
     510,    42,    43,    44,    45,    46,    47,    48,   511,     0,
      50,   512,    52,   513,   514,    55,   515,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,   516,   517,    73,     0,    74,    75,    76,   518,     0,
       0,    79,    80,    81,     0,     0,   519,    83,     0,     0,
       0,     0,    85,   520,    87,   521,   522,    90,    91,  1687,
       0,     0,     0,     0,     0,     0,     0,     0,    96,    97,
      98,    99,     0,   100,     0,     0,     0,   523,   102,   103,
     524,   525,     0,     0,     0,     0,     0,     0,   106,   107,
     108,     0,   110,   111,  1186,     0,     0,     0,  1187,     0,
       0,   118,   526,     0,     0,     0,   120,   121,   122,   123,
     527,   125,   126,   528,     0,     0,     0,     0,     0,     0,
       0,   529,   530,   133,     0,     0,     0,   134,     0,   135,
     136,   531,     0,     0,     0,     0,     0,     0,   142,     0,
     143,   144,   145,   146,   532,   148,   149,   150,   151,   152,
     153,     0,     0,  1188,   155,     0,     0,   157,   430,   159,
     160,   161,   162,   163,   164,   165,   166,   167,   168,   169,
     533,   171,   534,   173,   535,   536,   176,   177,   178,   179,
     180,   181,   537,   183,   538,   539,   540,   187,   188,   541,
     542,   191,   192,   193,   194,   195,   196,   197,   198,   199,
     200,   201,   202,   543,     0,   204,   544,   206,     0,   207,
     208,   209,   210,   211,   212,   213,   214,   215,   216,   217,
     218,   219,   220,   221,   222,   223,   224,   225,   226,   227,
     228,   229,   230,   231,   232,   233,   234,   235,   236,   237,
     238,   239,   240,   241,   242,   243,   244,   245,   246,   247,
     248,   249,   250,   251,   252,   253,     3,     0,     0,     0,
     256,   545,   258,  1189,  1190,  1191,  1192,    11,    12,    13,
      14,     0,     0,     0,     0,   427,   428,     0,    18,    19,
      20,   503,    22,    23,    24,   504,    26,   505,   506,  1185,
     508,   429,    32,   509,    34,    35,    36,     0,    37,    38,
      39,    40,   510,    42,    43,    44,    45,    46,    47,    48,
     511,     0,    50,   512,    52,   513,   514,    55,   515,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,   516,   517,    73,     0,    74,    75,    76,
     518,     0,     0,    79,    80,    81,     0,     0,   519,    83,
       0,     0,     0,     0,    85,   520,    87,   521,   522,    90,
      91,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      96,    97,    98,    99,     0,   100,     0,     0,     0,   523,
     102,   103,   524,   525,     0,     0,     0,     0,     0,     0,
     106,   107,   108,     0,   110,   111,  1186,     0,     0,     0,
    1187,     0,     0,   118,   526,     0,     0,     0,   120,   121,
     122,   123,   527,   125,   126,   528,     0,     0,     0,     0,
       0,     0,     0,   529,   530,   133,     0,     0,     0,   134,
       0,   135,   136,   531,     0,     0,     0,     0,     0,     0,
     142,     0,   143,   144,   145,   146,   532,   148,   149,   150,
     151,   152,   153,     0,     0,  1188,   155,     0,     0,   157,
     430,   159,   160,   161,   162,   163,   164,   165,   166,   167,
     168,   169,   533,   171,   534,   173,   535,   536,   176,   177,
     178,   179,   180,   181,   537,   183,   538,   539,   540,   187,
     188,   541,   542,   191,   192,   193,   194,   195,   196,   197,
     198,   199,   200,   201,   202,   543,     0,   204,   544,   206,
       0,   207,   208,   209,   210,   211,   212,   213,   214,   215,
     216,   217,   218,   219,   220,   221,   222,   223,   224,   225,
     226,   227,   228,   229,   230,   231,   232,   233,   234,   235,
     236,   237,   238,   239,   240,   241,   242,   243,   244,   245,
     246,   247,   248,   249,   250,   251,   252,   253,     3,     0,
       0,     0,   256,   545,   258,  1189,  1190,  1191,  1192,    11,
      12,    13,    14,     0,     0,     0,     0,   427,   428,     0,
      18,    19,    20,   503,    22,    23,    24,   504,    26,   505,
     506,  1185,   508,   429,    32,   509,    34,    35,    36,     0,
      37,    38,    39,    40,   510,    42,    43,    44,    45,    46,
      47,    48,   511,     0,    50,   512,    52,   513,   514,    55,
     515,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,   516,   517,    73,     0,    74,
      75,    76,   518,     0,     0,    79,    80,    81,     0,     0,
     519,    83,     0,     0,     0,     0,    85,   520,    87,   521,
     522,    90,    91,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    96,    97,    98,    99,     0,   100,     0,     0,
       0,   523,   102,   103,   524,   525,     0,     0,     0,     0,
       0,     0,   106,   107,   108,     0,   110,   111,  1186,     0,
       0,     0,  1226,     0,     0,   118,   526,     0,     0,     0,
     120,   121,   122,   123,   527,   125,   126,   528,     0,     0,
       0,     0,     0,     0,     0,   529,   530,   133,     0,     0,
       0,   134,     0,   135,   136,   531,     0,     0,     0,     0,
       0,     0,   142,     0,   143,   144,   145,   146,   532,   148,
     149,   150,   151,   152,   153,     0,     0,   154,   155,     0,
       0,   157,   430,   159,   160,   161,   162,   163,   164,   165,
     166,   167,   168,   169,   533,   171,   534,   173,   535,   536,
     176,   177,   178,   179,   180,   181,   537,   183,   538,   539,
     540,   187,   188,   541,   542,   191,   192,   193,   194,   195,
     196,   197,   198,   199,   200,   201,   202,   543,     0,   204,
     544,   206,     0,   207,   208,   209,   210,   211,   212,   213,
     214,   215,   216,   217,   218,   219,   220,   221,   222,   223,
     224,   225,   226,   227,   228,   229,   230,   231,   232,   233,
     234,   235,   236,   237,   238,   239,   240,   241,   242,   243,
     244,   245,   246,   247,   248,   249,   250,   251,   252,   253,
       3,     0,   590,   591,   256,   545,   258,  1189,  1190,  1191,
    1192,    11,    12,     0,     0,     0,     0,     0,     0,   427,
     428,     0,    18,    19,    20,   553,    22,    23,    24,   504,
     554,   555,   506,   507,   508,   429,    32,   509,    34,   556,
      36,     0,    37,    38,    39,    40,   557,    42,   558,   559,
      45,    46,    47,    48,   511,     0,    50,   560,    52,   513,
     514,    55,   515,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,   516,   517,    73,
       0,    74,    75,    76,   561,     0,     0,    79,    80,    81,
       0,     0,   519,    83,     0,     0,     0,     0,    85,   520,
      87,   521,   522,    90,    91,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    96,    97,    98,    99,     0,   100,
       0,     0,     0,   523,   102,   103,   524,   525,     0,     0,
       0,     0,     0,     0,   106,   107,   108,     0,   110,   111,
     112,     0,     0,     0,     0,     0,     0,   118,   526,     0,
       0,     0,   120,   121,   122,   123,   527,   125,   126,   528,
       0,     0,     0,     0,     0,     0,     0,   529,   530,   133,
       0,     0,     0,   134,     0,   135,   136,   531,     0,     0,
       0,     0,     0,     0,   142,     0,   143,   144,   145,   146,
     532,   148,   149,   150,   151,   152,   153,     0,     0,   154,
     155,     0,     0,   157,   430,   159,   160,   161,   162,   163,
     164,   165,   166,   167,   168,   169,   533,   562,   534,   173,
     535,   536,   176,   177,   178,   179,   180,   181,   537,   183,
     538,   539,   540,   187,   188,   541,   542,   191,   192,   193,
     194,   195,   196,   197,   198,   199,   200,   201,   202,   543,
       0,   204,   544,   206,     0,   207,   208,   209,   210,   211,
     212,   213,   214,   215,   216,   217,   218,   219,   220,   221,
     222,   223,   224,   225,   226,   227,   228,   229,   230,   231,
     232,   233,   234,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,   245,   246,   247,   248,   249,   250,   251,
     252,   253,     3,     0,     0,     0,   256,   545,   258,   259,
     260,   261,   262,    11,    12,     0,     0,     0,     0,     0,
       0,   427,   428,     0,    18,    19,    20,   553,    22,    23,
      24,   504,   554,   555,   506,   507,   508,   429,    32,   509,
      34,   556,    36,     0,    37,    38,    39,    40,   557,    42,
     558,   559,    45,    46,    47,    48,   511,     0,    50,   560,
      52,   513,   514,    55,   515,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,   516,
     517,    73,     0,    74,    75,    76,   561,     0,     0,    79,
      80,    81,     0,     0,   519,    83,     0,     0,     0,     0,
      85,   520,    87,   521,   522,    90,    91,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    96,    97,    98,    99,
       0,   100,     0,     0,     0,   523,   102,   103,   524,   525,
       0,     0,     0,     0,     0,     0,   106,   107,   108,     0,
     110,   111,   112,     0,     0,     0,     0,     0,     0,   118,
     526,     0,     0,     0,   120,   121,   122,   123,   527,   125,
     126,   528,     0,     0,     0,     0,     0,     0,     0,   529,
     530,   133,     0,     0,     0,   134,   787,   135,   136,   531,
       0,     0,     0,     0,   788,     0,   142,     0,   143,   144,
     145,   146,   532,   148,   149,   150,   151,   152,   153,     0,
       0,   154,   155,     0,     0,   157,   430,   159,   160,   161,
     162,   163,   164,   165,   166,   167,   168,   169,   533,   562,
     534,   173,   535,   536,   176,   177,   178,   179,   180,   181,
     537,   183,   538,   539,   540,   187,   188,   541,   542,   191,
     192,   193,   194,   195,   196,   197,   198,   199,   200,   201,
     202,   543,     0,   204,   544,   206,     0,   207,   208,   209,
     210,   211,   212,   213,   214,   215,   216,   217,   218,   219,
     220,   221,   222,   223,   224,   225,   226,   227,   228,   229,
     230,   231,   232,   233,   234,   235,   236,   237,   238,   239,
     240,   241,   242,   243,   244,   245,   246,   247,   248,   249,
     250,   251,   252,   253,     3,     0,     0,     0,   256,   545,
     258,   259,   260,   261,   262,    11,    12,     0,     0,     0,
     813,     0,     0,   427,   428,     0,    18,    19,    20,   553,
      22,    23,    24,   504,   554,   555,   506,   507,   508,   429,
      32,   509,    34,   556,    36,     0,    37,    38,    39,    40,
     557,    42,   558,   559,    45,    46,    47,    48,   511,     0,
      50,   560,    52,   513,   514,    55,   515,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,   516,   517,    73,     0,    74,    75,    76,   561,     0,
       0,    79,    80,    81,     0,     0,   519,    83,     0,     0,
       0,     0,    85,   520,    87,   521,   522,    90,    91,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    96,    97,
      98,    99,     0,   100,     0,     0,     0,   523,   102,   103,
     524,   525,     0,     0,     0,     0,     0,     0,   106,   107,
     108,     0,   110,   111,   112,     0,     0,     0,     0,     0,
       0,   118,   526,     0,     0,     0,   120,   121,   122,   123,
     527,   125,   126,   528,     0,     0,     0,     0,     0,     0,
       0,   529,   530,   133,     0,     0,     0,   134,   814,   135,
     136,   531,     0,     0,     0,     0,     0,     0,   142,     0,
     143,   144,   145,   146,   532,   148,   149,   150,   151,   152,
     153,     0,     0,   154,   155,     0,     0,   157,   430,   159,
     160,   161,   162,   163,   164,   165,   166,   167,   168,   169,
     533,   562,   534,   173,   535,   536,   176,   177,   178,   179,
     180,   181,   537,   183,   538,   539,   540,   187,   188,   541,
     542,   191,   192,   193,   194,   195,   196,   197,   198,   199,
     200,   201,   202,   543,     0,   204,   544,   206,     0,   207,
     208,   209,   210,   211,   212,   213,   214,   215,   216,   217,
     218,   219,   220,   221,   222,   223,   224,   225,   226,   227,
     228,   229,   230,   231,   232,   233,   234,   235,   236,   237,
     238,   239,   240,   241,   242,   243,   244,   245,   246,   247,
     248,   249,   250,   251,   252,   253,     3,     0,     0,     0,
     256,   545,   258,   259,   260,   261,   262,    11,    12,     0,
       0,     0,     0,     0,     0,   427,   428,     0,    18,    19,
      20,   553,    22,    23,    24,   504,   554,   555,   506,   507,
     508,   429,    32,   509,    34,   556,    36,     0,    37,    38,
      39,    40,   557,    42,   558,   559,    45,    46,    47,    48,
     511,     0,    50,   560,    52,   513,   514,    55,   515,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,   516,   517,    73,     0,    74,    75,    76,
     561,     0,     0,    79,    80,    81,     0,     0,   519,    83,
       0,     0,     0,     0,    85,   520,    87,   521,   522,    90,
      91,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      96,    97,    98,    99,     0,   100,     0,     0,     0,   523,
     102,   103,   524,   525,     0,     0,     0,     0,     0,     0,
     106,   107,   108,     0,   110,   111,   112,     0,     0,     0,
       0,     0,     0,   118,   526,     0,     0,     0,   120,   121,
     122,   123,   527,   125,   126,   528,     0,     0,     0,     0,
       0,     0,     0,   529,   530,   133,     0,     0,     0,   134,
     831,   135,   136,   531,     0,     0,     0,     0,   832,     0,
     142,     0,   143,   144,   145,   146,   532,   148,   149,   150,
     151,   152,   153,     0,     0,   154,   155,     0,     0,   157,
     430,   159,   160,   161,   162,   163,   164,   165,   166,   167,
     168,   169,   533,   562,   534,   173,   535,   536,   176,   177,
     178,   179,   180,   181,   537,   183,   538,   539,   540,   187,
     188,   541,   542,   191,   192,   193,   194,   195,   196,   197,
     198,   199,   200,   201,   202,   543,     0,   204,   544,   206,
       0,   207,   208,   209,   210,   211,   212,   213,   214,   215,
     216,   217,   218,   219,   220,   221,   222,   223,   224,   225,
     226,   227,   228,   229,   230,   231,   232,   233,   234,   235,
     236,   237,   238,   239,   240,   241,   242,   243,   244,   245,
     246,   247,   248,   249,   250,   251,   252,   253,     3,     0,
       0,     0,   256,   545,   258,   259,   260,   261,   262,    11,
      12,     0,     0,     0,     0,     0,     0,   427,   428,     0,
      18,    19,    20,   553,    22,    23,    24,   504,   554,   555,
     506,   507,   508,   429,    32,   509,    34,   556,    36,     0,
      37,    38,    39,    40,   557,    42,   558,   559,    45,    46,
      47,    48,   511,     0,    50,   560,    52,   513,   514,    55,
     515,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,   516,   517,    73,     0,    74,
      75,    76,   561,     0,     0,    79,    80,    81,     0,     0,
     519,    83,     0,     0,     0,     0,    85,   520,    87,   521,
     522,    90,    91,  1281,     0,     0,     0,     0,     0,     0,
       0,     0,    96,    97,    98,    99,     0,   100,     0,     0,
       0,   523,   102,   103,   524,   525,     0,     0,     0,     0,
       0,     0,   106,   107,   108,     0,   110,   111,   112,     0,
       0,     0,     0,     0,     0,   118,   526,     0,     0,     0,
     120,   121,   122,   123,   527,   125,   126,   528,     0,     0,
       0,     0,     0,     0,     0,   529,   530,   133,     0,     0,
       0,   134,     0,   135,   136,   531,     0,     0,     0,     0,
       0,     0,   142,     0,   143,   144,   145,   146,   532,   148,
     149,   150,   151,   152,   153,     0,     0,   154,   155,     0,
       0,   157,   430,   159,   160,   161,   162,   163,   164,   165,
     166,   167,   168,   169,   533,   562,   534,   173,   535,   536,
     176,   177,   178,   179,   180,   181,   537,   183,   538,   539,
     540,   187,   188,   541,   542,   191,   192,   193,   194,   195,
     196,   197,   198,   199,   200,   201,   202,   543,     0,   204,
     544,   206,     0,   207,   208,   209,   210,   211,   212,   213,
     214,   215,   216,   217,   218,   219,   220,   221,   222,   223,
     224,   225,   226,   227,   228,   229,   230,   231,   232,   233,
     234,   235,   236,   237,   238,   239,   240,   241,   242,   243,
     244,   245,   246,   247,   248,   249,   250,   251,   252,   253,
       3,     0,     0,     0,   256,   545,   258,   259,   260,   261,
     262,    11,    12,     0,     0,     0,     0,     0,     0,   427,
     428,     0,    18,    19,    20,   553,    22,    23,    24,   504,
     554,   555,   506,   507,   508,   429,    32,   509,    34,   556,
      36,     0,    37,    38,    39,    40,   557,    42,   558,   559,
      45,    46,    47,    48,   511,     0,    50,   560,    52,   513,
     514,    55,   515,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,   516,   517,    73,
       0,    74,    75,    76,   561,     0,     0,    79,    80,    81,
       0,     0,   519,    83,     0,     0,     0,     0,    85,   520,
      87,   521,   522,    90,    91,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    96,    97,    98,    99,     0,   100,
       0,     0,     0,   523,   102,   103,   524,   525,     0,     0,
       0,     0,     0,     0,   106,   107,   108,     0,   110,   111,
     112,     0,     0,     0,     0,     0,     0,   118,   526,     0,
       0,     0,   120,   121,   122,   123,   527,   125,   126,   528,
       0,     0,     0,     0,     0,     0,     0,   529,   530,   133,
       0,     0,     0,   134,     0,   135,   136,   531,     0,     0,
       0,     0,     0,     0,   142,     0,   143,   144,   145,   146,
     532,   148,   149,   150,   151,   152,   153,     0,     0,   154,
     155,     0,     0,   157,   430,   159,   160,   161,   162,   163,
     164,   165,   166,   167,   168,   169,   533,   562,   534,   173,
     535,   536,   176,   177,   178,   179,   180,   181,   537,   183,
     538,   539,   540,   187,   188,   541,   542,   191,   192,   193,
     194,   195,   196,   197,   198,   199,   200,   201,   202,   543,
       0,   204,   544,   206,     0,   207,   208,   209,   210,   211,
     212,   213,   214,   215,   216,   217,   218,   219,   220,   221,
     222,   223,   224,   225,   226,   227,   228,   229,   230,   231,
     232,   233,   234,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,   245,   246,   247,   248,   249,   250,   251,
     252,   253,     0,     0,     0,     0,   256,   545,   258,   259,
     260,   261,   262,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,     0,    15,     0,     0,   427,   428,     0,
      18,    19,    20,   865,    22,    23,    24,    25,     0,   866,
     506,    29,     0,   429,    32,   509,    34,     0,    36,     0,
      37,    38,    39,    40,   867,    42,     0,     0,    45,    46,
      47,    48,   511,     0,    50,   868,    52,     0,     0,    55,
     515,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,   516,   517,    73,     0,    74,
      75,    76,   869,     0,     0,    79,    80,    81,     0,     0,
     519,    83,     0,     0,     0,    84,    85,   520,    87,   521,
     522,    90,    91,   575,    93,     0,     0,     0,     0,     0,
      95,     0,    96,    97,    98,    99,     0,   100,     0,     0,
       0,   523,   102,   103,   524,   525,     0,     0,     0,     0,
       0,     0,   106,   107,   108,   109,   110,   111,     0,   576,
     114,     0,   115,   116,     0,   118,   119,     0,     0,     0,
     120,   121,   122,   123,   124,   125,   126,   528,     0,   128,
       0,     0,     0,     0,     0,   529,   530,   133,     0,     0,
       0,   134,     0,   135,   136,   531,     0,     0,     0,     0,
       0,     0,   142,     0,   143,   144,   145,   146,   147,   148,
     149,   150,   151,   152,   153,     0,     0,     0,   155,   156,
       0,   157,   430,   159,   160,   161,   162,   163,   164,   165,
     166,   167,   168,   169,   533,     0,   534,   173,   535,   536,
     176,   177,   178,   179,   180,   181,   537,   183,   538,   539,
     540,   187,   188,     0,   542,   191,   192,   193,   194,   195,
     196,   197,   198,   199,   200,   201,   202,   543,     0,   204,
     544,   206,     0,   207,   208,   209,   210,   211,   212,   213,
     214,   215,   216,   217,   218,   219,   220,   221,   222,   223,
     224,   225,   226,   227,   228,   229,   230,   231,   232,   233,
     234,   235,   236,   237,   238,   239,   240,   241,   242,   243,
     244,   245,   246,   247,   248,   249,   250,   251,   252,   253,
     254,     0,   255,    11,    12,   545,   258,     0,     0,     0,
       0,   427,   428,     0,    18,    19,    20,     0,    22,    23,
      24,   504,     0,     0,   506,   507,     0,   429,    32,   509,
      34,     0,    36,     0,    37,    38,    39,    40,     0,    42,
       0,     0,    45,    46,    47,    48,   511,     0,    50,     0,
      52,     0,     0,    55,   515,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,   516,
     517,    73,     0,    74,    75,    76,     0,     0,     0,    79,
      80,    81,     0,     0,   519,    83,     0,     0,     0,     0,
      85,   520,    87,   521,   522,    90,    91,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    96,    97,    98,    99,
       0,   100,     0,     0,     0,   523,   102,   103,   524,   525,
       0,     0,     0,     0,     0,     0,   106,   107,   108,     0,
     110,   111,     0,     0,     0,     0,     0,     0,     0,   118,
     526,     0,     0,     0,   120,   121,   122,   123,   527,   125,
     126,   528,     0,     0,     0,     0,     0,     0,     0,   529,
     530,   133,     0,     0,     0,   134,     0,   135,   136,   531,
       0,     0,     0,     0,     0,     0,   142,     0,   143,   144,
     145,   146,   532,   148,   149,   150,   151,   152,   153,     0,
       0,     0,   155,     0,     0,   157,   430,   159,   160,   161,
     162,   163,   164,   165,   166,   167,   168,   169,   533,     0,
     534,   173,   535,   536,   176,   177,   178,   179,   180,   181,
     537,   183,   538,   539,   540,   187,   188,     0,   542,   191,
     192,   193,   194,   195,   196,   197,   198,   199,   200,   201,
     202,   543,     0,   204,   544,   206,     0,   207,   208,   209,
     210,   211,   212,   213,   214,   215,   216,   217,   218,   219,
     220,   221,   222,   223,   224,   225,   226,   227,   228,   229,
     230,   231,   232,   233,   234,   235,   236,   237,   238,   239,
     240,   241,   242,   243,   244,   245,   246,   247,   248,   249,
     250,   251,   252,   253,     0,     0,     0,     0,     0,   545,
     258
  };

  const short
  jsoniq_parser::yycheck_[] =
  {
       0,   323,   190,   697,  1322,     5,     6,     7,     8,     9,
      10,   354,   323,   852,   293,   792,    16,     0,   698,   699,
     700,   701,     5,     6,     7,     8,     9,    10,   725,  1087,
     442,   638,   639,   603,     0,   140,   270,   713,   608,     5,
       6,     7,     8,     9,    10,   955,   956,   889,     5,     6,
       7,     8,     9,   205,  1071,     5,     6,     7,     8,     9,
      10,  1340,   955,   956,   792,   477,   478,   807,   440,   441,
    1404,  1405,  1113,   263,   264,   942,   302,     1,   640,   914,
     820,    78,     1,  1541,   271,   676,  1544,   678,   679,  1034,
     681,    38,   683,  1171,   834,    29,    30,   636,   637,   286,
     691,   692,    32,  1052,  1000,  1353,  1451,  1000,   636,   637,
     638,   639,    58,   113,     1,    49,    29,    30,     1,   121,
       1,     1,     1,    38,     1,     1,   110,    79,     1,   121,
     113,    96,   140,  1652,    41,     1,    49,     1,     1,    46,
      47,  1034,   110,    77,  1054,   233,    59,   113,    55,   984,
      97,  1061,    99,     1,     3,    62,   110,    64,    65,    33,
     146,  1054,   110,   113,    77,   110,    73,    99,  1061,    82,
       1,    39,    40,   109,    42,    43,   135,   110,     1,   120,
      35,   115,   461,   162,    99,   157,   110,   749,   164,    99,
     641,   110,    60,   100,   645,   131,    51,   205,   160,   222,
     179,   114,   115,   116,   151,   157,     8,   433,   185,   150,
     110,   114,   115,   439,     5,     6,     7,     8,     9,    10,
      69,   186,   163,   200,   203,    74,     1,   157,   160,   110,
     160,   233,     1,   187,   322,   194,   151,  1555,    47,   262,
      99,     1,   214,   191,   110,   942,   110,   110,   116,   194,
     180,  1585,   357,   151,     1,     1,    65,   198,   191,   243,
     582,  1319,   110,   263,   264,     1,  1006,   267,   125,     1,
    1109,   582,   121,   241,     9,    10,   222,   229,   230,   110,
     263,   264,  1059,  1802,   267,     0,     1,   110,  1305,   281,
     190,  1187,   899,   900,  1187,   281,   164,   263,   264,    99,
     579,   267,   302,  1344,  1171,  1172,  1651,   194,  1766,   190,
    1558,   194,  1353,   315,   194,   194,   262,   194,   194,   302,
    1060,   194,   113,   187,   190,   188,   233,  1272,     1,  1057,
    1226,  1059,  1281,  1226,   281,   110,   302,   267,     0,     1,
     188,   110,   568,     1,   218,  1073,     1,   938,   760,  1427,
     110,   254,   255,     1,   264,  1813,   187,  1815,   897,   898,
     228,   295,   233,   110,   110,   362,   281,   190,   365,   303,
       1,   106,   172,    32,   110,     1,   178,   160,   110,  1272,
     151,   190,   295,   281,   241,     1,    54,  1297,  1223,     1,
     303,  1713,   127,   805,   129,   110,    97,   321,   311,   302,
     319,   269,   636,   637,  1297,   264,   265,   103,   110,    32,
       1,    29,    30,     1,    47,   190,    49,  1751,   325,   326,
     327,   190,   157,  1163,   986,    44,   161,   618,  1321,    48,
     190,    49,   121,   433,    53,   625,   662,   110,     1,   439,
     110,   157,   177,   190,   190,   146,   633,   634,   110,    32,
     433,   322,   110,    72,   190,   110,   439,   644,   190,   134,
     260,   261,   110,   263,   199,   133,   466,   433,   468,  1791,
     270,  1805,   187,   439,  1171,  1172,   109,     1,   205,   110,
     280,   672,   217,   466,   110,   468,     9,   214,   190,   233,
     490,   235,   292,   189,   110,   495,   192,   115,   110,     9,
     466,   190,   468,   110,   110,   164,  1283,   490,    31,  1788,
     281,    35,   495,   160,   187,  1833,   466,  1558,   468,   110,
     190,    31,   110,   675,   490,   187,   201,    51,  1225,   495,
     433,  1271,   190,   116,   157,   190,   439,   160,   171,   187,
     490,  1148,  1149,  1150,  1386,   495,   160,   110,  1390,  1459,
     160,    97,   171,     1,     5,   174,   187,   180,     9,    97,
    1427,   187,    99,  1133,  1292,   134,  1459,   470,   568,  1436,
     189,   187,   475,   192,   172,   187,   576,    57,   322,    59,
    1447,   164,   485,   190,   190,   568,   157,    35,    99,   492,
     493,  1267,   162,   576,   160,  1007,  1008,   216,   501,   187,
    1012,   103,   568,   233,   127,   235,   129,   979,   206,   179,
     576,   110,   717,   718,   116,   138,   616,   127,   110,   129,
    1148,  1149,  1150,   103,   187,   625,   576,   162,   138,   160,
    1042,   631,   201,   616,   157,   172,   636,   637,   638,   639,
     151,     5,   625,   214,   179,     9,   265,   157,  1541,   121,
     616,  1544,  1669,   636,   637,   638,   639,    32,    99,   625,
     110,   887,   662,   897,   121,   568,   616,    97,  1408,   160,
     636,   637,   638,   639,   123,   466,   127,   468,   129,   662,
     226,   227,   110,   586,   587,    99,   137,   110,   226,   227,
     916,   190,   322,   160,   597,   241,   662,   110,   190,   490,
     185,   121,   116,   241,   495,   157,   157,     9,    10,   158,
     151,   160,   615,   616,   714,   200,   903,  1556,     9,    10,
     907,   151,  1561,   260,   261,    47,   263,    49,   180,  1787,
    1427,   714,   189,   157,    99,   192,  1513,    97,   113,  1436,
     190,    97,   277,   280,  1438,     1,   160,     3,   714,   284,
    1447,   116,   148,   128,  1621,   292,   180,   714,    97,   294,
      82,    97,   190,   127,   714,   129,   123,   190,    97,   189,
     233,  1681,   192,   137,   240,   238,   242,   190,   153,   259,
       1,    99,     3,   179,   123,   576,  1393,   109,  1681,   271,
     146,   151,   272,   157,   274,   160,   245,   797,   116,   160,
     282,   158,   198,   160,   106,   285,   286,   287,    97,   289,
     146,   291,   141,    69,   797,   106,   719,   146,    74,   158,
     109,   160,  1498,   726,  1734,   127,  1736,   129,   828,   829,
    1173,   797,   126,     9,    10,   162,   127,    56,   129,    56,
     797,  1734,    61,  1736,    61,   828,   829,   797,    69,   171,
    1778,  1779,   179,    74,  1693,   157,   169,   276,  1697,   161,
      27,    80,   828,   829,   283,   121,   157,   126,   189,    36,
     161,   192,   185,  1766,    98,   177,   203,   277,   828,   829,
      99,  1048,    99,  1050,   284,   109,   177,   887,  1300,  1799,
      29,    30,  1820,   214,   294,   119,    63,   199,  1826,   140,
     121,   126,   121,   126,   887,   997,  1799,   131,   199,   909,
      49,   126,  1138,   154,  1193,   217,   916,  1009,  1612,  1613,
    1813,   887,  1815,   714,  1621,  1605,   217,  1094,   271,  1096,
     106,   141,   157,   916,   121,   160,   255,   256,   149,   282,
     843,   844,   845,  1782,   847,    29,    30,   157,   152,   160,
     916,   127,  1139,   129,  1142,   955,   956,   103,  1145,   278,
     307,   152,    29,    30,   168,    49,   103,   141,   157,   316,
     116,   160,    39,    40,   103,    42,    43,   168,  1654,   116,
     121,   157,    49,   157,   887,   161,   103,   116,  1400,   157,
     141,   103,    59,    60,   136,   146,  1318,  1409,   126,   116,
    1000,   177,   144,   145,   116,   147,   797,   141,   126,    56,
      77,  1751,   146,   155,    61,    82,   126,   159,   189,   161,
     126,   192,   720,   199,   166,   723,   195,   196,  1722,  1723,
     728,   132,   133,   285,   286,   287,   157,   828,   829,   291,
     182,   217,   157,   157,   126,   160,   160,   114,   115,   116,
     157,   132,   133,   160,  1054,  1377,   361,   126,   240,   364,
     242,  1061,   965,  1385,    78,   207,   208,   209,   210,   211,
     212,   695,   696,  1299,  1385,   229,   230,   231,    92,   157,
    1756,   702,   703,   126,   704,   705,  1265,  1266,   221,  1089,
    1642,  1643,   160,   121,   249,   160,   249,   164,   250,   157,
     322,  1795,   116,     0,   194,    95,  1089,   170,   257,   102,
    1522,    52,   204,   104,   128,   160,   130,   105,   197,   158,
      32,   187,    49,  1089,   156,   187,   187,  1803,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,  1138,  1089,
     121,   187,   312,   187,   157,   187,   254,  1147,  1148,  1149,
    1150,     1,   190,   121,   146,  1138,   110,   190,   157,   190,
     157,   228,   160,   146,   160,  1148,  1149,  1150,   187,   125,
      37,   160,  1138,   160,   160,   125,   160,    17,   160,   157,
     160,   218,  1148,  1149,  1150,    32,   239,  1187,   194,   194,
       3,   121,   171,   253,  1382,   292,   173,   173,   292,  1875,
     121,  1877,   269,    73,   239,    97,    97,    97,  1530,   153,
    1532,  1887,   110,  1535,  1536,   190,   110,  1539,    32,   164,
     131,   164,   136,  1545,   254,    97,  1226,   307,   295,   146,
     136,   190,   121,   190,   187,   187,   303,   110,    97,    99,
      47,    99,   190,   190,   311,    97,   146,   157,   190,   187,
     190,   187,   157,   157,   157,   157,   157,   194,    35,   187,
     245,   136,   146,    97,   173,    32,   235,  1170,   241,   294,
      97,   307,   157,   121,   131,  1275,   121,   151,    97,   187,
     202,   136,   164,   136,    99,   121,   157,   151,   187,   157,
     146,   103,  1275,   103,   187,   125,   125,  1297,  1089,  1299,
      37,    32,   253,   157,   121,   110,   194,    82,  1534,  1275,
    1310,   109,   275,   110,   160,   148,  1299,   266,  1275,   293,
      32,  1321,   268,   262,   294,  1275,    32,  1310,   160,  1329,
      32,   295,   160,  1299,   160,   160,   160,   160,   131,   235,
     148,   190,   314,   194,  1310,   151,  1329,   190,   362,   190,
    1350,   365,  1578,  1310,   110,   190,   151,  1357,   146,   190,
    1310,   103,  1684,  1329,   202,   187,  1688,  1350,   136,   190,
       7,    99,  1329,   160,  1357,   241,   194,   160,   241,  1329,
     241,   160,   109,   245,  1350,   157,   151,   121,   121,  1389,
      80,  1357,   151,   258,   190,   290,    81,   241,   190,   190,
    1350,   190,    32,    32,   190,   190,  1389,  1357,   294,   190,
     190,   110,   173,   187,   130,   151,   258,   130,    99,   151,
     146,   187,   157,  1389,   187,   187,   265,   187,    97,   187,
      32,    99,   446,   447,   173,   190,   450,   324,   190,  1389,
     454,  1667,   279,   457,   458,    97,   151,   201,    56,    97,
    1772,   465,  1678,  1679,    32,   110,  1682,   471,   472,  1459,
     110,   121,   273,  1689,   233,   479,    97,   121,   288,    97,
     121,   201,   486,   487,   488,   489,   201,    96,   202,   295,
     494,   201,   233,   186,  1275,   121,   121,   233,   502,   203,
    1812,   267,   270,   759,  1816,   121,   295,   267,   301,  1227,
    1001,   461,  1062,  1756,   313,  1272,   441,  1803,   890,   313,
     810,   313,  1168,   314,   314,  1598,  1411,  1419,  1155,  1310,
     314,  1407,  1404,   564,  1311,  1319,  1848,  1316,  1324,  1851,
     674,   694,   958,   725,  1534,   549,   731,   725,  1329,  1344,
     852,  1541,  1826,  1820,  1544,   469,   469,  1050,  1193,  1775,
     564,  1534,  1874,  1193,  1376,  1387,  1386,  1447,  1436,  1350,
      92,   575,  1621,  1172,  1618,   349,  1357,  1249,  1534,  1033,
    1440,  1488,  1645,  1181,    -1,  1437,    -1,   606,  1578,    -1,
      -1,   113,    -1,    -1,   116,    -1,    -1,    -1,  1814,   603,
      -1,    -1,    -1,    -1,   608,  1578,   128,   611,   130,    -1,
      -1,    -1,    -1,    -1,   618,    -1,  1832,    -1,    -1,    -1,
      -1,    -1,  1578,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1847,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1869,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1878,    -1,    -1,    -1,    -1,    -1,   672,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1667,    -1,    -1,
      -1,    -1,  1575,  1576,  1577,    -1,    -1,    -1,  1678,  1679,
      -1,  1681,  1682,    -1,  1667,    -1,    -1,    -1,    -1,  1689,
      -1,    -1,    -1,    -1,    -1,  1678,  1679,    -1,    -1,  1682,
      -1,  1667,    -1,    -1,    -1,    -1,  1689,    -1,    -1,    -1,
      -1,   725,  1678,  1679,    -1,    -1,  1682,    -1,    -1,    -1,
      -1,    -1,    -1,  1689,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1734,    -1,  1736,    -1,    -1,   753,
      -1,    -1,   756,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   770,    -1,    -1,    -1,
     774,   775,    -1,    -1,    -1,    -1,  1766,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1775,    -1,    -1,    -1,   793,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   801,    -1,    -1,
     804,    -1,  1775,    -1,   808,    -1,    -1,    -1,    -1,  1799,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1710,    -1,  1775,
      -1,    -1,    -1,  1813,  1814,  1815,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1732,
      -1,  1814,  1832,    -1,    -1,    -1,    -1,   851,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1847,  1814,  1832,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1847,    -1,  1832,    -1,    -1,  1869,
      -1,    -1,    -1,    -1,    -1,   889,    -1,    -1,  1878,    -1,
      -1,  1847,    -1,    -1,    -1,    -1,  1869,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1878,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1869,    -1,    -1,   920,    -1,    -1,    -1,
      -1,    -1,  1878,    -1,    -1,   447,   930,    -1,   450,    -1,
      -1,    -1,   454,    -1,    -1,   457,   458,    -1,    -1,    -1,
      -1,    -1,    -1,   465,    -1,    -1,    -1,    -1,    -1,   471,
     472,   955,   956,   957,   958,    -1,    -1,   479,    -1,    -1,
      -1,    -1,    -1,    -1,   486,   487,   488,   489,    -1,    -1,
      -1,    -1,   494,    -1,    -1,    -1,   113,    -1,    -1,    -1,
     502,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1000,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1014,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   549,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1039,    -1,    -1,    -1,    -1,
      -1,    -1,   564,    -1,  1048,    -1,  1050,    -1,  1052,    -1,
    1054,    -1,    -1,   575,   576,    -1,    -1,  1061,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   606,    -1,    -1,    -1,    -1,   611,
    1094,    -1,  1096,    -1,    -1,    -1,   618,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1133,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     672,    -1,    -1,    -1,    -1,  1159,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1187,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   725,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1226,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   756,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   770,    -1,
      -1,    -1,   774,   775,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   793,    -1,    -1,    -1,    -1,    -1,  1281,    -1,   801,
      -1,  1285,   804,    -1,    -1,    -1,   808,    -1,    -1,    -1,
      -1,    -1,    -1,  1297,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1311,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1321,   840,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   851,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1351,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1376,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1386,    -1,    -1,    -1,  1390,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   920,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   930,    -1,
      -1,    -1,    -1,    -1,  1418,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   955,   956,   957,   958,    -1,    -1,   576,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1459,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   601,   602,    -1,   604,   605,   606,
     607,    -1,   609,    -1,    -1,    -1,    -1,   614,  1000,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   624,    -1,    -1,
    1494,    -1,  1014,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     647,   648,    -1,    -1,    -1,    -1,    -1,  1039,    -1,    -1,
      -1,   658,    -1,    -1,    -1,   662,  1048,    -1,  1050,    -1,
    1052,    -1,  1054,    -1,  1538,    -1,    -1,  1541,    -1,  1061,
    1544,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1557,    -1,    -1,    -1,    -1,  1562,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1094,    -1,  1096,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     727,  1595,  1596,    -1,   731,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   772,    -1,  1159,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1187,    -1,    -1,    -1,    -1,
      -1,    -1,   809,    -1,    -1,    -1,    -1,  1681,    -1,  1683,
      -1,  1685,    -1,  1687,    -1,    -1,    -1,    -1,    -1,    -1,
    1694,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   835,    -1,
      -1,    -1,   839,   840,  1226,   842,    -1,    -1,    -1,    -1,
      -1,    -1,   849,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1734,    -1,  1736,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1752,    -1,
      -1,  1755,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1281,
      -1,    -1,  1766,  1285,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1777,    -1,  1297,    -1,    -1,    -1,   916,
      -1,    -1,    -1,    -1,    -1,   922,   923,    -1,    -1,  1311,
      -1,    -1,    -1,    -1,    -1,  1799,    -1,    -1,    -1,  1321,
      -1,    -1,    -1,  1807,    -1,    -1,    -1,    -1,    -1,  1813,
      -1,  1815,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1351,
      -1,    -1,  1836,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1845,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1376,    -1,    -1,    -1,    -1,    -1,
      -1,  1865,    -1,    -1,    -1,  1002,  1870,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1882,    -1,
      -1,  1885,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1418,    -1,    -1,    -1,
    1037,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1056,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1072,    -1,  1459,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1083,  1084,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1098,    -1,    -1,  1101,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1494,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1120,  1121,  1122,    -1,  1124,  1125,  1126,
    1127,  1128,  1129,  1130,  1131,  1132,    -1,    -1,  1135,  1136,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1144,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1538,    -1,  1155,  1541,
      -1,    -1,  1544,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1557,    -1,    -1,    -1,    -1,
    1562,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1595,  1596,    -1,  1213,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1287,    -1,  1289,    -1,  1291,    -1,    -1,    -1,    -1,  1681,
      -1,  1683,  1299,  1685,    -1,  1687,    -1,    -1,    -1,    -1,
      -1,  1308,  1694,    -1,    -1,    -1,    -1,  1314,  1315,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1333,    -1,    -1,  1336,
    1337,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1734,    -1,  1736,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1752,    -1,    -1,  1755,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1766,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1777,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1407,    -1,    -1,    -1,  1411,  1412,    -1,  1799,    -1,    -1,
    1417,    -1,    -1,    -1,    -1,  1807,    -1,    -1,    -1,    -1,
      -1,  1813,    -1,  1815,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1836,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1845,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1865,    -1,    -1,    -1,    -1,  1870,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1882,    -1,    -1,  1885,    -1,    -1,    -1,    -1,    -1,    -1,
    1507,    -1,    -1,    -1,  1511,    -1,    -1,    -1,  1515,    -1,
    1517,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1525,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1533,  1534,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1554,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1566,
    1567,  1568,  1569,    -1,  1571,  1572,    -1,  1574,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1589,    -1,    -1,    -1,    -1,  1594,    -1,    -1,
      -1,    -1,    -1,  1600,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1660,    -1,  1662,    -1,    -1,  1665,    -1,
      -1,  1668,    -1,    -1,    -1,    -1,  1673,  1674,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1682,    -1,    -1,    -1,  1686,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1703,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1714,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1759,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1771,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1814,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1843,    -1,    -1,    -1,
    1847,    -1,    -1,    -1,    -1,  1852,     0,     1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    11,    12,    -1,
      -1,  1868,    16,    -1,    -1,  1872,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    -1,    32,    -1,
      -1,    35,    36,    -1,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    -1,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    -1,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    -1,    97,    98,    99,   100,   101,    -1,   103,
     104,   105,    -1,    -1,   108,   109,    -1,    -1,    -1,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,    -1,
     124,    -1,    -1,    -1,   128,    -1,   130,   131,   132,   133,
      -1,   135,    -1,    -1,    -1,   139,   140,   141,   142,   143,
      -1,    -1,    -1,    -1,    -1,    -1,   150,   151,   152,   153,
     154,   155,   156,   157,   158,    -1,   160,   161,   162,   163,
     164,    -1,    -1,    -1,   168,   169,   170,   171,   172,   173,
     174,   175,    -1,   177,    -1,   179,   180,    -1,    -1,   183,
     184,   185,    -1,    -1,    -1,   189,    -1,   191,   192,   193,
     194,   195,   196,    -1,   198,    -1,   200,    -1,   202,   203,
     204,   205,   206,   207,   208,   209,   210,   211,   212,    -1,
      -1,   215,   216,   217,    -1,   219,   220,   221,   222,   223,
     224,   225,   226,   227,   228,   229,   230,   231,   232,   233,
     234,   235,   236,   237,   238,   239,   240,   241,   242,   243,
     244,   245,   246,   247,   248,   249,   250,   251,   252,   253,
     254,   255,   256,   257,   258,   259,   260,   261,   262,   263,
     264,   265,    -1,   267,   268,   269,    -1,   271,   272,   273,
     274,   275,   276,   277,   278,   279,   280,   281,   282,   283,
     284,   285,   286,   287,   288,   289,   290,   291,   292,   293,
     294,   295,   296,   297,   298,   299,   300,   301,   302,   303,
     304,   305,   306,   307,   308,   309,   310,   311,   312,   313,
     314,   315,   316,   317,   318,    -1,   320,    -1,   322,   323,
     324,   325,   326,   327,   328,     0,     1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    11,    12,    -1,    -1,
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
     115,   116,   117,   118,   119,   120,   121,   122,    -1,   124,
      -1,    -1,    -1,   128,    -1,   130,   131,   132,   133,    -1,
     135,    -1,    -1,    -1,   139,   140,   141,   142,   143,    -1,
      -1,    -1,    -1,    -1,    -1,   150,   151,   152,   153,   154,
     155,   156,   157,   158,    -1,   160,   161,   162,   163,   164,
      -1,    -1,    -1,   168,   169,   170,   171,   172,   173,   174,
     175,    -1,   177,    -1,   179,   180,    -1,    -1,   183,   184,
     185,    -1,    -1,    -1,   189,    -1,   191,   192,   193,   194,
     195,   196,    -1,   198,    -1,   200,    -1,   202,   203,   204,
     205,   206,   207,   208,   209,   210,   211,   212,    -1,    -1,
     215,   216,   217,    -1,   219,   220,   221,   222,   223,   224,
     225,   226,   227,   228,   229,   230,   231,   232,   233,   234,
     235,   236,   237,   238,   239,   240,   241,   242,   243,   244,
     245,   246,   247,   248,   249,   250,   251,   252,   253,   254,
     255,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,    -1,   267,   268,   269,    -1,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,     1,   320,    -1,   322,   323,   324,
     325,   326,   327,   328,    11,    12,    -1,    -1,    -1,    16,
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
     117,   118,   119,   120,   121,   122,    -1,   124,    -1,    -1,
      -1,   128,    -1,   130,   131,   132,   133,    -1,   135,    -1,
      -1,    -1,   139,   140,   141,   142,   143,    -1,    -1,    -1,
      -1,    -1,    -1,   150,   151,   152,   153,   154,   155,   156,
     157,   158,    -1,   160,   161,   162,   163,   164,    -1,    -1,
      -1,   168,   169,   170,   171,   172,   173,   174,   175,    -1,
     177,    -1,   179,   180,    -1,    -1,   183,   184,   185,    -1,
      -1,    -1,   189,    -1,   191,   192,   193,   194,   195,   196,
      -1,   198,    -1,   200,    -1,   202,   203,   204,   205,   206,
     207,   208,   209,   210,   211,   212,    -1,    -1,   215,   216,
     217,    -1,   219,   220,   221,   222,   223,   224,   225,   226,
     227,   228,   229,   230,   231,   232,   233,   234,   235,   236,
     237,   238,   239,   240,   241,   242,   243,   244,   245,   246,
     247,   248,   249,   250,   251,   252,   253,   254,   255,   256,
     257,   258,   259,   260,   261,   262,   263,   264,   265,    -1,
     267,   268,   269,    -1,   271,   272,   273,   274,   275,   276,
     277,   278,   279,   280,   281,   282,   283,   284,   285,   286,
     287,   288,   289,   290,   291,   292,   293,   294,   295,   296,
     297,   298,   299,   300,   301,   302,   303,   304,   305,   306,
     307,   308,   309,   310,   311,   312,   313,   314,   315,   316,
     317,   318,     1,   320,    -1,   322,   323,   324,   325,   326,
     327,   328,    11,    12,    -1,    -1,    -1,    16,    -1,    -1,
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
     119,   120,   121,   122,    -1,   124,    -1,    -1,    -1,   128,
      -1,   130,   131,   132,   133,    -1,   135,    -1,    -1,    -1,
     139,   140,   141,   142,   143,    -1,    -1,    -1,    -1,    -1,
      -1,   150,   151,   152,   153,   154,   155,   156,   157,   158,
      -1,   160,   161,   162,   163,   164,    -1,    -1,    -1,   168,
     169,   170,   171,   172,   173,   174,   175,    -1,   177,    -1,
     179,   180,    -1,    -1,   183,   184,   185,    -1,    -1,    -1,
     189,    -1,   191,   192,   193,    -1,   195,   196,    -1,   198,
      -1,   200,    -1,   202,   203,   204,   205,   206,   207,   208,
     209,   210,   211,   212,    -1,    -1,   215,   216,   217,    -1,
     219,   220,   221,   222,   223,   224,   225,   226,   227,   228,
     229,   230,   231,   232,   233,   234,   235,   236,   237,   238,
     239,   240,   241,   242,   243,   244,   245,   246,   247,   248,
     249,   250,   251,   252,   253,   254,   255,   256,   257,   258,
     259,   260,   261,   262,   263,   264,   265,    -1,   267,   268,
     269,    -1,   271,   272,   273,   274,   275,   276,   277,   278,
     279,   280,   281,   282,   283,   284,   285,   286,   287,   288,
     289,   290,   291,   292,   293,   294,   295,   296,   297,   298,
     299,   300,   301,   302,   303,   304,   305,   306,   307,   308,
     309,   310,   311,   312,   313,   314,   315,   316,   317,   318,
      -1,   320,    -1,   322,   323,   324,   325,   326,   327,   328,
       0,     1,    -1,     3,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    44,    -1,    -1,    -1,    48,    -1,
      -1,    -1,    52,    53,    -1,    -1,    -1,    -1,    58,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,
      -1,    -1,    72,    -1,    74,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    95,    -1,    97,    98,    99,
      -1,    -1,   102,   103,   104,   105,    -1,    -1,    -1,   109,
     110,    -1,    -1,    -1,    -1,    -1,   116,    -1,    -1,   119,
     120,   121,    -1,    -1,    -1,   125,    -1,    -1,    -1,    -1,
     130,   131,    -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,
     140,    -1,    -1,    -1,   144,   145,    -1,   147,    -1,   149,
     150,    -1,    -1,    -1,   154,   155,    -1,    -1,   158,   159,
     160,   161,   162,   163,    -1,    -1,   166,    -1,    -1,    -1,
     170,   171,    -1,    -1,   174,    -1,    -1,    -1,    -1,   179,
       0,     1,   182,     3,    -1,    -1,    -1,   187,   188,   189,
     190,   191,   192,    -1,   194,   195,   196,   197,   198,    -1,
      -1,    -1,    -1,   203,   204,   205,    -1,   207,   208,   209,
     210,   211,   212,    -1,   214,    -1,   216,    -1,    -1,    -1,
      -1,    -1,   222,    -1,    44,    -1,   226,   227,    48,    -1,
      -1,    -1,    52,    53,    -1,    -1,    -1,    -1,    58,    -1,
      -1,   241,    -1,   243,    -1,   245,    -1,    -1,    -1,    69,
      -1,    -1,    72,    -1,    74,    -1,    -1,   257,    -1,    -1,
      -1,    -1,   262,    -1,    -1,   265,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    95,    -1,    97,    98,    99,
      -1,    -1,   102,   103,   104,   105,    -1,    -1,    -1,   109,
     110,    -1,    -1,    -1,    -1,    -1,   116,    -1,    -1,   119,
     120,   121,    -1,    -1,    -1,   125,    -1,    -1,    -1,    -1,
     130,   131,    -1,    -1,    -1,    -1,   136,    -1,    -1,   319,
     140,   321,    -1,    -1,   144,   145,    -1,   147,    -1,   149,
     150,    -1,    -1,    -1,   154,   155,    -1,    -1,   158,   159,
     160,   161,   162,   163,    -1,    -1,   166,    -1,    -1,    -1,
     170,   171,    -1,    -1,   174,    -1,    -1,    -1,    -1,   179,
      -1,    -1,   182,    -1,    -1,    -1,    -1,   187,   188,   189,
     190,   191,   192,    -1,   194,   195,   196,   197,   198,    -1,
      -1,    -1,    -1,   203,   204,   205,    -1,   207,   208,   209,
     210,   211,   212,    -1,   214,    -1,   216,    -1,    -1,    -1,
      -1,    -1,   222,    -1,    -1,    -1,   226,   227,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   241,    -1,   243,    -1,   245,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   257,    -1,    -1,
      -1,    -1,   262,    -1,    -1,   265,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    11,
      12,    -1,    -1,    -1,    16,    -1,    -1,    -1,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,   319,
      32,   321,    -1,    35,    36,    -1,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    -1,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    -1,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    -1,    97,    98,    99,   100,   101,
      -1,   103,   104,   105,    -1,    -1,   108,   109,    -1,    -1,
      -1,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,    -1,   124,    -1,    -1,    -1,   128,    -1,   130,   131,
     132,   133,    -1,   135,    -1,    -1,    -1,   139,   140,   141,
     142,   143,    -1,    -1,    -1,    -1,    -1,    -1,   150,   151,
     152,   153,   154,   155,   156,   157,   158,    -1,   160,   161,
     162,   163,   164,    -1,    -1,    -1,   168,   169,   170,   171,
     172,   173,   174,   175,    -1,   177,    -1,   179,   180,    -1,
      -1,   183,   184,   185,    -1,    -1,    -1,   189,    -1,   191,
     192,   193,   194,   195,   196,    -1,   198,    -1,   200,    -1,
     202,   203,   204,   205,   206,   207,   208,   209,   210,   211,
     212,    -1,    -1,   215,   216,   217,    -1,   219,   220,   221,
     222,   223,   224,   225,   226,   227,   228,   229,   230,   231,
     232,   233,   234,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,   245,   246,   247,   248,   249,   250,   251,
     252,   253,   254,   255,   256,   257,   258,   259,   260,   261,
     262,   263,   264,   265,    -1,   267,   268,   269,    -1,   271,
     272,   273,   274,   275,   276,   277,   278,   279,   280,   281,
     282,   283,   284,   285,   286,   287,   288,   289,   290,   291,
     292,   293,   294,   295,   296,   297,   298,   299,   300,   301,
     302,   303,   304,   305,   306,   307,   308,   309,   310,   311,
     312,   313,   314,   315,   316,   317,   318,    -1,   320,    -1,
     322,   323,   324,   325,   326,   327,   328,   329,   330,    11,
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
     122,    -1,   124,    -1,    -1,    -1,   128,    -1,   130,   131,
     132,   133,    -1,   135,    -1,    -1,    -1,   139,   140,   141,
     142,   143,    -1,    -1,    -1,    -1,    -1,    -1,   150,   151,
     152,   153,   154,   155,   156,   157,   158,    -1,   160,   161,
     162,   163,   164,    -1,    -1,    -1,   168,   169,   170,   171,
     172,   173,   174,   175,    -1,   177,    -1,   179,   180,    -1,
      -1,   183,   184,   185,    -1,    -1,    -1,   189,    -1,   191,
     192,   193,   194,   195,   196,    -1,   198,    -1,   200,    -1,
     202,   203,   204,   205,   206,   207,   208,   209,   210,   211,
     212,    -1,    -1,   215,   216,   217,    -1,   219,   220,   221,
     222,   223,   224,   225,   226,   227,   228,   229,   230,   231,
     232,   233,   234,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,   245,   246,   247,   248,   249,   250,   251,
     252,   253,   254,   255,   256,   257,   258,   259,   260,   261,
     262,   263,   264,   265,    -1,   267,   268,   269,    -1,   271,
     272,   273,   274,   275,   276,   277,   278,   279,   280,   281,
     282,   283,   284,   285,   286,   287,   288,   289,   290,   291,
     292,   293,   294,   295,   296,   297,   298,   299,   300,   301,
     302,   303,   304,   305,   306,   307,   308,   309,   310,   311,
     312,   313,   314,   315,   316,   317,   318,    -1,   320,    -1,
     322,   323,   324,   325,   326,   327,   328,    -1,   330,    11,
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
     122,    -1,   124,    -1,    -1,    -1,   128,    -1,   130,   131,
     132,   133,    -1,   135,    -1,    -1,    -1,   139,   140,   141,
     142,   143,    -1,    -1,    -1,    -1,    -1,    -1,   150,   151,
     152,   153,   154,   155,   156,   157,   158,    -1,   160,   161,
     162,   163,   164,    -1,    -1,    -1,   168,   169,   170,   171,
     172,   173,   174,   175,    -1,   177,    -1,   179,   180,    -1,
      -1,   183,   184,   185,    -1,   187,    -1,   189,    -1,   191,
     192,   193,   194,   195,   196,    -1,   198,    -1,   200,    -1,
     202,   203,   204,   205,   206,   207,   208,   209,   210,   211,
     212,    -1,    -1,   215,   216,   217,    -1,   219,   220,   221,
     222,   223,   224,   225,   226,   227,   228,   229,   230,   231,
     232,   233,   234,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,   245,   246,   247,   248,   249,   250,   251,
     252,   253,   254,   255,   256,   257,   258,   259,   260,   261,
     262,   263,   264,   265,    -1,   267,   268,   269,    -1,   271,
     272,   273,   274,   275,   276,   277,   278,   279,   280,   281,
     282,   283,   284,   285,   286,   287,   288,   289,   290,   291,
     292,   293,   294,   295,   296,   297,   298,   299,   300,   301,
     302,   303,   304,   305,   306,   307,   308,   309,   310,   311,
     312,   313,   314,   315,   316,   317,   318,    -1,   320,    -1,
     322,   323,   324,   325,   326,   327,   328,    11,    12,    -1,
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
     114,   115,   116,   117,   118,   119,   120,   121,   122,    -1,
     124,    -1,    -1,    -1,   128,    -1,   130,   131,   132,   133,
      -1,   135,    -1,    -1,    -1,   139,   140,   141,   142,   143,
      -1,    -1,    -1,    -1,   148,    -1,   150,   151,   152,   153,
     154,   155,   156,   157,   158,    -1,   160,   161,   162,   163,
     164,    -1,    -1,    -1,   168,   169,   170,   171,   172,   173,
     174,   175,    -1,   177,    -1,   179,   180,    -1,    -1,   183,
     184,   185,    -1,    -1,    -1,   189,   190,   191,   192,   193,
      -1,   195,   196,    -1,   198,    -1,   200,    -1,   202,   203,
     204,   205,   206,   207,   208,   209,   210,   211,   212,    -1,
      -1,   215,   216,   217,    -1,   219,   220,   221,   222,   223,
     224,   225,   226,   227,   228,   229,   230,   231,   232,   233,
     234,   235,   236,   237,   238,   239,   240,   241,   242,   243,
     244,   245,   246,   247,   248,   249,   250,   251,   252,   253,
     254,   255,   256,   257,   258,   259,   260,   261,   262,   263,
     264,   265,    -1,   267,   268,   269,    -1,   271,   272,   273,
     274,   275,   276,   277,   278,   279,   280,   281,   282,   283,
     284,   285,   286,   287,   288,   289,   290,   291,   292,   293,
     294,   295,   296,   297,   298,   299,   300,   301,   302,   303,
     304,   305,   306,   307,   308,   309,   310,   311,   312,   313,
     314,   315,   316,   317,   318,    -1,   320,    -1,   322,   323,
     324,   325,   326,   327,   328,    11,    12,    -1,    -1,    -1,
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
     116,   117,   118,   119,   120,   121,   122,    -1,   124,    -1,
      -1,    -1,   128,    -1,   130,   131,   132,   133,    -1,   135,
      -1,    -1,    -1,   139,   140,   141,   142,   143,    -1,    -1,
      -1,    -1,   148,    -1,   150,   151,   152,   153,   154,   155,
     156,   157,   158,    -1,   160,   161,   162,   163,   164,    -1,
      -1,    -1,   168,   169,   170,   171,   172,   173,   174,   175,
      -1,   177,    -1,   179,   180,    -1,    -1,   183,   184,   185,
      -1,    -1,    -1,   189,   190,   191,   192,   193,    -1,   195,
     196,    -1,   198,    -1,   200,    -1,   202,   203,   204,   205,
     206,   207,   208,   209,   210,   211,   212,    -1,    -1,   215,
     216,   217,    -1,   219,   220,   221,   222,   223,   224,   225,
     226,   227,   228,   229,   230,   231,   232,   233,   234,   235,
     236,   237,   238,   239,   240,   241,   242,   243,   244,   245,
     246,   247,   248,   249,   250,   251,   252,   253,   254,   255,
     256,   257,   258,   259,   260,   261,   262,   263,   264,   265,
      -1,   267,   268,   269,    -1,   271,   272,   273,   274,   275,
     276,   277,   278,   279,   280,   281,   282,   283,   284,   285,
     286,   287,   288,   289,   290,   291,   292,   293,   294,   295,
     296,   297,   298,   299,   300,   301,   302,   303,   304,   305,
     306,   307,   308,   309,   310,   311,   312,   313,   314,   315,
     316,   317,   318,    -1,   320,    -1,   322,   323,   324,   325,
     326,   327,   328,    11,    12,    -1,    -1,    -1,    16,    -1,
      -1,    -1,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    -1,    32,    -1,    -1,    35,    36,    -1,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    -1,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    -1,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    -1,    97,
      98,    99,   100,   101,    -1,   103,   104,   105,    -1,    -1,
     108,   109,    -1,    -1,    -1,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,    -1,   124,    -1,    -1,    -1,
     128,    -1,   130,   131,   132,   133,    -1,   135,    -1,    -1,
      -1,   139,   140,   141,   142,   143,    -1,    -1,    -1,    -1,
      -1,    -1,   150,   151,   152,   153,   154,   155,   156,   157,
     158,    -1,   160,   161,   162,   163,   164,    -1,    -1,    -1,
     168,   169,   170,   171,   172,   173,   174,   175,    -1,   177,
      -1,   179,   180,    -1,    -1,   183,   184,   185,    -1,    -1,
      -1,   189,    -1,   191,   192,   193,   194,   195,   196,    -1,
     198,    -1,   200,    -1,   202,   203,   204,   205,   206,   207,
     208,   209,   210,   211,   212,    -1,    -1,   215,   216,   217,
      -1,   219,   220,   221,   222,   223,   224,   225,   226,   227,
     228,   229,   230,   231,   232,   233,   234,   235,   236,   237,
     238,   239,   240,   241,   242,   243,   244,   245,   246,   247,
     248,   249,   250,   251,   252,   253,   254,   255,   256,   257,
     258,   259,   260,   261,   262,   263,   264,   265,    -1,   267,
     268,   269,    -1,   271,   272,   273,   274,   275,   276,   277,
     278,   279,   280,   281,   282,   283,   284,   285,   286,   287,
     288,   289,   290,   291,   292,   293,   294,   295,   296,   297,
     298,   299,   300,   301,   302,   303,   304,   305,   306,   307,
     308,   309,   310,   311,   312,   313,   314,   315,   316,   317,
     318,    -1,   320,    -1,   322,   323,   324,   325,   326,   327,
     328,    11,    12,    -1,    -1,    -1,    16,    -1,    -1,    -1,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    -1,    32,    -1,    -1,    35,    36,    -1,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    -1,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    -1,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    -1,    97,    98,    99,
     100,   101,    -1,   103,   104,   105,    -1,    -1,   108,   109,
      -1,    -1,    -1,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,    -1,   124,    -1,    -1,    -1,   128,    -1,
     130,   131,   132,   133,    -1,   135,    -1,    -1,    -1,   139,
     140,   141,   142,   143,    -1,    -1,    -1,    -1,    -1,    -1,
     150,   151,   152,   153,   154,   155,   156,   157,   158,    -1,
     160,   161,   162,   163,   164,    -1,    -1,    -1,   168,   169,
     170,   171,   172,   173,   174,   175,    -1,   177,    -1,   179,
     180,    -1,    -1,   183,   184,   185,    -1,    -1,   188,   189,
      -1,   191,   192,   193,    -1,   195,   196,    -1,   198,    -1,
     200,    -1,   202,   203,   204,   205,   206,   207,   208,   209,
     210,   211,   212,    -1,    -1,   215,   216,   217,    -1,   219,
     220,   221,   222,   223,   224,   225,   226,   227,   228,   229,
     230,   231,   232,   233,   234,   235,   236,   237,   238,   239,
     240,   241,   242,   243,   244,   245,   246,   247,   248,   249,
     250,   251,   252,   253,   254,   255,   256,   257,   258,   259,
     260,   261,   262,   263,   264,   265,    -1,   267,   268,   269,
      -1,   271,   272,   273,   274,   275,   276,   277,   278,   279,
     280,   281,   282,   283,   284,   285,   286,   287,   288,   289,
     290,   291,   292,   293,   294,   295,   296,   297,   298,   299,
     300,   301,   302,   303,   304,   305,   306,   307,   308,   309,
     310,   311,   312,   313,   314,   315,   316,   317,   318,    -1,
     320,    -1,   322,   323,   324,   325,   326,   327,   328,    11,
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
     122,    -1,   124,    -1,    -1,    -1,   128,    -1,   130,   131,
     132,   133,    -1,   135,    -1,    -1,    -1,   139,   140,   141,
     142,   143,    -1,    -1,    -1,    -1,    -1,    -1,   150,   151,
     152,   153,   154,   155,   156,   157,   158,    -1,   160,   161,
     162,   163,   164,    -1,    -1,    -1,   168,   169,   170,   171,
     172,   173,   174,   175,    -1,   177,    -1,   179,   180,    -1,
      -1,   183,   184,   185,    -1,    -1,    -1,   189,   190,   191,
     192,   193,    -1,   195,   196,    -1,   198,    -1,   200,    -1,
     202,   203,   204,   205,   206,   207,   208,   209,   210,   211,
     212,    -1,    -1,   215,   216,   217,    -1,   219,   220,   221,
     222,   223,   224,   225,   226,   227,   228,   229,   230,   231,
     232,   233,   234,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,   245,   246,   247,   248,   249,   250,   251,
     252,   253,   254,   255,   256,   257,   258,   259,   260,   261,
     262,   263,   264,   265,    -1,   267,   268,   269,    -1,   271,
     272,   273,   274,   275,   276,   277,   278,   279,   280,   281,
     282,   283,   284,   285,   286,   287,   288,   289,   290,   291,
     292,   293,   294,   295,   296,   297,   298,   299,   300,   301,
     302,   303,   304,   305,   306,   307,   308,   309,   310,   311,
     312,   313,   314,   315,   316,   317,   318,    -1,   320,    -1,
     322,   323,   324,   325,   326,   327,   328,    11,    12,    -1,
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
     114,   115,   116,   117,   118,   119,   120,   121,   122,    -1,
     124,    -1,    -1,    -1,   128,    -1,   130,   131,   132,   133,
      -1,   135,    -1,    -1,    -1,   139,   140,   141,   142,   143,
      -1,    -1,    -1,    -1,    -1,    -1,   150,   151,   152,   153,
     154,   155,   156,   157,   158,    -1,   160,   161,   162,   163,
     164,    -1,    -1,    -1,   168,   169,   170,   171,   172,   173,
     174,   175,    -1,   177,    -1,   179,   180,    -1,    -1,   183,
     184,   185,    -1,    -1,    -1,   189,    -1,   191,   192,   193,
      -1,   195,   196,    -1,   198,    -1,   200,    -1,   202,   203,
     204,   205,   206,   207,   208,   209,   210,   211,   212,    -1,
      -1,   215,   216,   217,    -1,   219,   220,   221,   222,   223,
     224,   225,   226,   227,   228,   229,   230,   231,   232,   233,
     234,   235,   236,   237,   238,   239,   240,   241,   242,   243,
     244,   245,   246,   247,   248,   249,   250,   251,   252,   253,
     254,   255,   256,   257,   258,   259,   260,   261,   262,   263,
     264,   265,    -1,   267,   268,   269,    -1,   271,   272,   273,
     274,   275,   276,   277,   278,   279,   280,   281,   282,   283,
     284,   285,   286,   287,   288,   289,   290,   291,   292,   293,
     294,   295,   296,   297,   298,   299,   300,   301,   302,   303,
     304,   305,   306,   307,   308,   309,   310,   311,   312,   313,
     314,   315,   316,   317,   318,   319,   320,    -1,   322,   323,
     324,   325,   326,   327,   328,    11,    12,    -1,    -1,    -1,
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
     116,   117,   118,   119,   120,   121,   122,    -1,   124,    -1,
      -1,    -1,   128,    -1,   130,   131,   132,   133,    -1,   135,
      -1,    -1,    -1,   139,   140,   141,   142,   143,    -1,    -1,
      -1,    -1,    -1,    -1,   150,   151,   152,   153,   154,   155,
     156,   157,   158,    -1,   160,   161,   162,   163,   164,    -1,
      -1,    -1,   168,   169,   170,   171,   172,   173,   174,   175,
      -1,   177,    -1,   179,   180,    -1,    -1,   183,   184,   185,
      -1,    -1,    -1,   189,    -1,   191,   192,   193,    -1,   195,
     196,    -1,   198,    -1,   200,    -1,   202,   203,   204,   205,
     206,   207,   208,   209,   210,   211,   212,    -1,    -1,   215,
     216,   217,    -1,   219,   220,   221,   222,   223,   224,   225,
     226,   227,   228,   229,   230,   231,   232,   233,   234,   235,
     236,   237,   238,   239,   240,   241,   242,   243,   244,   245,
     246,   247,   248,   249,   250,   251,   252,   253,   254,   255,
     256,   257,   258,   259,   260,   261,   262,   263,   264,   265,
      -1,   267,   268,   269,    -1,   271,   272,   273,   274,   275,
     276,   277,   278,   279,   280,   281,   282,   283,   284,   285,
     286,   287,   288,   289,   290,   291,   292,   293,   294,   295,
     296,   297,   298,   299,   300,   301,   302,   303,   304,   305,
     306,   307,   308,   309,   310,   311,   312,   313,   314,   315,
     316,   317,   318,    -1,   320,   321,   322,   323,   324,   325,
     326,   327,   328,    11,    12,    -1,    -1,    -1,    16,    -1,
      -1,    -1,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    -1,    32,    -1,    -1,    35,    36,    -1,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    -1,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    -1,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    -1,    97,
      98,    99,   100,   101,    -1,   103,   104,   105,    -1,    -1,
     108,   109,    -1,    -1,    -1,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,    -1,   124,    -1,    -1,    -1,
     128,    -1,   130,   131,   132,   133,    -1,   135,    -1,    -1,
      -1,   139,   140,   141,   142,   143,    -1,    -1,    -1,    -1,
      -1,    -1,   150,   151,   152,   153,   154,   155,   156,   157,
     158,    -1,   160,   161,   162,   163,   164,    -1,    -1,    -1,
     168,   169,   170,   171,   172,   173,   174,   175,    -1,   177,
      -1,   179,   180,    -1,    -1,   183,   184,   185,    -1,    -1,
      -1,   189,    -1,   191,   192,   193,   194,   195,   196,    -1,
     198,    -1,   200,    -1,   202,   203,   204,   205,   206,   207,
     208,   209,   210,   211,   212,    -1,    -1,   215,   216,   217,
      -1,   219,   220,   221,   222,   223,   224,   225,   226,   227,
     228,   229,   230,   231,   232,   233,   234,   235,   236,   237,
     238,   239,   240,   241,   242,   243,   244,   245,   246,   247,
     248,   249,   250,   251,   252,   253,   254,   255,   256,   257,
     258,   259,   260,   261,   262,   263,   264,   265,    -1,   267,
     268,   269,    -1,   271,   272,   273,   274,   275,   276,   277,
     278,   279,   280,   281,   282,   283,   284,   285,   286,   287,
     288,   289,   290,   291,   292,   293,   294,   295,   296,   297,
     298,   299,   300,   301,   302,   303,   304,   305,   306,   307,
     308,   309,   310,   311,   312,   313,   314,   315,   316,   317,
     318,    -1,   320,    -1,   322,   323,   324,   325,   326,   327,
     328,    11,    12,    -1,    -1,    -1,    16,    -1,    -1,    -1,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    -1,    32,    -1,    -1,    35,    36,    -1,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    -1,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    -1,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    -1,    97,    98,    99,
     100,   101,    -1,   103,   104,   105,    -1,    -1,   108,   109,
      -1,    -1,    -1,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,    -1,   124,    -1,    -1,    -1,   128,    -1,
     130,   131,   132,   133,    -1,   135,    -1,    -1,    -1,   139,
     140,   141,   142,   143,    -1,    -1,    -1,    -1,    -1,    -1,
     150,   151,   152,   153,   154,   155,   156,   157,   158,    -1,
     160,   161,   162,   163,   164,    -1,    -1,    -1,   168,   169,
     170,   171,   172,   173,   174,   175,    -1,   177,    -1,   179,
     180,    -1,    -1,   183,   184,   185,    -1,    -1,    -1,   189,
      -1,   191,   192,   193,   194,   195,   196,    -1,   198,    -1,
     200,    -1,   202,   203,   204,   205,   206,   207,   208,   209,
     210,   211,   212,    -1,    -1,   215,   216,   217,    -1,   219,
     220,   221,   222,   223,   224,   225,   226,   227,   228,   229,
     230,   231,   232,   233,   234,   235,   236,   237,   238,   239,
     240,   241,   242,   243,   244,   245,   246,   247,   248,   249,
     250,   251,   252,   253,   254,   255,   256,   257,   258,   259,
     260,   261,   262,   263,   264,   265,    -1,   267,   268,   269,
      -1,   271,   272,   273,   274,   275,   276,   277,   278,   279,
     280,   281,   282,   283,   284,   285,   286,   287,   288,   289,
     290,   291,   292,   293,   294,   295,   296,   297,   298,   299,
     300,   301,   302,   303,   304,   305,   306,   307,   308,   309,
     310,   311,   312,   313,   314,   315,   316,   317,   318,    -1,
     320,    -1,   322,   323,   324,   325,   326,   327,   328,    11,
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
     122,    -1,   124,    -1,    -1,    -1,   128,    -1,   130,   131,
     132,   133,    -1,   135,    -1,    -1,    -1,   139,   140,   141,
     142,   143,    -1,    -1,    -1,    -1,    -1,    -1,   150,   151,
     152,   153,   154,   155,   156,   157,   158,    -1,   160,   161,
     162,   163,   164,    -1,    -1,    -1,   168,   169,   170,   171,
     172,   173,   174,   175,    -1,   177,    -1,   179,   180,    -1,
      -1,   183,   184,   185,    -1,    -1,    -1,   189,    -1,   191,
     192,   193,   194,   195,   196,    -1,   198,    -1,   200,    -1,
     202,   203,   204,   205,   206,   207,   208,   209,   210,   211,
     212,    -1,    -1,   215,   216,   217,    -1,   219,   220,   221,
     222,   223,   224,   225,   226,   227,   228,   229,   230,   231,
     232,   233,   234,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,   245,   246,   247,   248,   249,   250,   251,
     252,   253,   254,   255,   256,   257,   258,   259,   260,   261,
     262,   263,   264,   265,    -1,   267,   268,   269,    -1,   271,
     272,   273,   274,   275,   276,   277,   278,   279,   280,   281,
     282,   283,   284,   285,   286,   287,   288,   289,   290,   291,
     292,   293,   294,   295,   296,   297,   298,   299,   300,   301,
     302,   303,   304,   305,   306,   307,   308,   309,   310,   311,
     312,   313,   314,   315,   316,   317,   318,    -1,   320,    -1,
     322,   323,   324,   325,   326,   327,   328,    11,    12,    -1,
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
     114,   115,   116,   117,   118,   119,   120,   121,   122,    -1,
     124,    -1,    -1,    -1,   128,    -1,   130,   131,   132,   133,
      -1,   135,    -1,    -1,    -1,   139,   140,   141,   142,   143,
      -1,    -1,    -1,    -1,    -1,    -1,   150,   151,   152,   153,
     154,   155,   156,   157,   158,    -1,   160,   161,   162,   163,
     164,    -1,    -1,    -1,   168,   169,   170,   171,   172,   173,
     174,   175,    -1,   177,    -1,   179,   180,    -1,    -1,   183,
     184,   185,    -1,    -1,    -1,   189,    -1,   191,   192,   193,
     194,   195,   196,    -1,   198,    -1,   200,    -1,   202,   203,
     204,   205,   206,   207,   208,   209,   210,   211,   212,    -1,
      -1,   215,   216,   217,    -1,   219,   220,   221,   222,   223,
     224,   225,   226,   227,   228,   229,   230,   231,   232,   233,
     234,   235,   236,   237,   238,   239,   240,   241,   242,   243,
     244,   245,   246,   247,   248,   249,   250,   251,   252,   253,
     254,   255,   256,   257,   258,   259,   260,   261,   262,   263,
     264,   265,    -1,   267,   268,   269,    -1,   271,   272,   273,
     274,   275,   276,   277,   278,   279,   280,   281,   282,   283,
     284,   285,   286,   287,   288,   289,   290,   291,   292,   293,
     294,   295,   296,   297,   298,   299,   300,   301,   302,   303,
     304,   305,   306,   307,   308,   309,   310,   311,   312,   313,
     314,   315,   316,   317,   318,    -1,   320,    -1,   322,   323,
     324,   325,   326,   327,   328,    11,    12,    -1,    -1,    -1,
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
     116,   117,   118,   119,   120,   121,   122,    -1,   124,    -1,
      -1,    -1,   128,    -1,   130,   131,   132,   133,    -1,   135,
      -1,    -1,    -1,   139,   140,   141,   142,   143,    -1,    -1,
      -1,    -1,    -1,    -1,   150,   151,   152,   153,   154,   155,
     156,   157,   158,    -1,   160,   161,   162,   163,   164,    -1,
      -1,    -1,   168,   169,   170,   171,   172,   173,   174,   175,
      -1,   177,    -1,   179,   180,    -1,    -1,   183,   184,   185,
      -1,    -1,   188,   189,    -1,   191,   192,   193,    -1,   195,
     196,    -1,   198,    -1,   200,    -1,   202,   203,   204,   205,
     206,   207,   208,   209,   210,   211,   212,    -1,    -1,   215,
     216,   217,    -1,   219,   220,   221,   222,   223,   224,   225,
     226,   227,   228,   229,   230,   231,   232,   233,   234,   235,
     236,   237,   238,   239,   240,   241,   242,   243,   244,   245,
     246,   247,   248,   249,   250,   251,   252,   253,   254,   255,
     256,   257,   258,   259,   260,   261,   262,   263,   264,   265,
      -1,   267,   268,   269,    -1,   271,   272,   273,   274,   275,
     276,   277,   278,   279,   280,   281,   282,   283,   284,   285,
     286,   287,   288,   289,   290,   291,   292,   293,   294,   295,
     296,   297,   298,   299,   300,   301,   302,   303,   304,   305,
     306,   307,   308,   309,   310,   311,   312,   313,   314,   315,
     316,   317,   318,    -1,   320,    -1,   322,   323,   324,   325,
     326,   327,   328,    11,    12,    -1,    -1,    -1,    16,    -1,
      -1,    -1,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    -1,    32,    -1,    -1,    35,    36,    -1,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    -1,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    -1,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    -1,    97,
      98,    99,   100,   101,    -1,   103,   104,   105,    -1,    -1,
     108,   109,    -1,    -1,    -1,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,    -1,   124,    -1,    -1,    -1,
     128,    -1,   130,   131,   132,   133,    -1,   135,    -1,    -1,
      -1,   139,   140,   141,   142,   143,    -1,    -1,    -1,    -1,
      -1,    -1,   150,   151,   152,   153,   154,   155,   156,   157,
     158,    -1,   160,   161,   162,   163,   164,    -1,    -1,    -1,
     168,   169,   170,   171,   172,   173,   174,   175,    -1,   177,
      -1,   179,   180,    -1,    -1,   183,   184,   185,    -1,    -1,
      -1,   189,   190,   191,   192,   193,    -1,   195,   196,    -1,
     198,    -1,   200,    -1,   202,   203,   204,   205,   206,   207,
     208,   209,   210,   211,   212,    -1,    -1,   215,   216,   217,
      -1,   219,   220,   221,   222,   223,   224,   225,   226,   227,
     228,   229,   230,   231,   232,   233,   234,   235,   236,   237,
     238,   239,   240,   241,   242,   243,   244,   245,   246,   247,
     248,   249,   250,   251,   252,   253,   254,   255,   256,   257,
     258,   259,   260,   261,   262,   263,   264,   265,    -1,   267,
     268,   269,    -1,   271,   272,   273,   274,   275,   276,   277,
     278,   279,   280,   281,   282,   283,   284,   285,   286,   287,
     288,   289,   290,   291,   292,   293,   294,   295,   296,   297,
     298,   299,   300,   301,   302,   303,   304,   305,   306,   307,
     308,   309,   310,   311,   312,   313,   314,   315,   316,   317,
     318,    -1,   320,    -1,   322,   323,   324,   325,   326,   327,
     328,    11,    12,    -1,    -1,    -1,    16,    -1,    -1,    -1,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    -1,    32,    -1,    -1,    35,    36,    -1,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    -1,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    -1,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    -1,    97,    98,    99,
     100,   101,    -1,   103,   104,   105,    -1,    -1,   108,   109,
      -1,    -1,    -1,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,    -1,   124,    -1,    -1,    -1,   128,    -1,
     130,   131,   132,   133,    -1,   135,    -1,    -1,    -1,   139,
     140,   141,   142,   143,    -1,    -1,    -1,    -1,    -1,    -1,
     150,   151,   152,   153,   154,   155,   156,   157,   158,    -1,
     160,   161,   162,   163,   164,    -1,    -1,    -1,   168,   169,
     170,   171,   172,   173,   174,   175,    -1,   177,    -1,   179,
     180,    -1,    -1,   183,   184,   185,    -1,    -1,    -1,   189,
      -1,   191,   192,   193,   194,   195,   196,    -1,   198,    -1,
     200,    -1,   202,   203,   204,   205,   206,   207,   208,   209,
     210,   211,   212,    -1,    -1,   215,   216,   217,    -1,   219,
     220,   221,   222,   223,   224,   225,   226,   227,   228,   229,
     230,   231,   232,   233,   234,   235,   236,   237,   238,   239,
     240,   241,   242,   243,   244,   245,   246,   247,   248,   249,
     250,   251,   252,   253,   254,   255,   256,   257,   258,   259,
     260,   261,   262,   263,   264,   265,    -1,   267,   268,   269,
      -1,   271,   272,   273,   274,   275,   276,   277,   278,   279,
     280,   281,   282,   283,   284,   285,   286,   287,   288,   289,
     290,   291,   292,   293,   294,   295,   296,   297,   298,   299,
     300,   301,   302,   303,   304,   305,   306,   307,   308,   309,
     310,   311,   312,   313,   314,   315,   316,   317,   318,    -1,
     320,    -1,   322,   323,   324,   325,   326,   327,   328,    11,
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
     122,    -1,   124,    -1,    -1,    -1,   128,    -1,   130,   131,
     132,   133,    -1,   135,    -1,    -1,    -1,   139,   140,   141,
     142,   143,    -1,    -1,    -1,    -1,   148,    -1,   150,   151,
     152,   153,   154,   155,   156,   157,   158,    -1,   160,   161,
     162,   163,   164,    -1,    -1,    -1,   168,   169,   170,   171,
     172,   173,   174,   175,    -1,   177,    -1,   179,   180,    -1,
      -1,   183,   184,   185,    -1,    -1,    -1,   189,    -1,   191,
     192,   193,    -1,   195,   196,    -1,   198,    -1,   200,    -1,
     202,   203,   204,   205,   206,   207,   208,   209,   210,   211,
     212,    -1,    -1,   215,   216,   217,    -1,   219,   220,   221,
     222,   223,   224,   225,   226,   227,   228,   229,   230,   231,
     232,   233,   234,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,   245,   246,   247,   248,   249,   250,   251,
     252,   253,   254,   255,   256,   257,   258,   259,   260,   261,
     262,   263,   264,   265,    -1,   267,   268,   269,    -1,   271,
     272,   273,   274,   275,   276,   277,   278,   279,   280,   281,
     282,   283,   284,   285,   286,   287,   288,   289,   290,   291,
     292,   293,   294,   295,   296,   297,   298,   299,   300,   301,
     302,   303,   304,   305,   306,   307,   308,   309,   310,   311,
     312,   313,   314,   315,   316,   317,   318,    -1,   320,    -1,
     322,   323,   324,   325,   326,   327,   328,    11,    12,    -1,
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
     114,   115,   116,   117,   118,   119,   120,   121,   122,    -1,
     124,    -1,    -1,    -1,   128,    -1,   130,   131,   132,   133,
      -1,   135,    -1,    -1,    -1,   139,   140,   141,   142,   143,
      -1,    -1,    -1,    -1,    -1,    -1,   150,   151,   152,   153,
     154,   155,   156,   157,   158,    -1,   160,   161,   162,   163,
     164,    -1,    -1,    -1,   168,   169,   170,   171,   172,   173,
     174,   175,    -1,   177,    -1,   179,   180,    -1,    -1,   183,
     184,   185,    -1,    -1,    -1,   189,    -1,   191,   192,   193,
      -1,   195,   196,    -1,   198,    -1,   200,    -1,   202,   203,
     204,   205,   206,   207,   208,   209,   210,   211,   212,    -1,
      -1,   215,   216,   217,    -1,   219,   220,   221,   222,   223,
     224,   225,   226,   227,   228,   229,   230,   231,   232,   233,
     234,   235,   236,   237,   238,   239,   240,   241,   242,   243,
     244,   245,   246,   247,   248,   249,   250,   251,   252,   253,
     254,   255,   256,   257,   258,   259,   260,   261,   262,   263,
     264,   265,    -1,   267,   268,   269,    -1,   271,   272,   273,
     274,   275,   276,   277,   278,   279,   280,   281,   282,   283,
     284,   285,   286,   287,   288,   289,   290,   291,   292,   293,
     294,   295,   296,   297,   298,   299,   300,   301,   302,   303,
     304,   305,   306,   307,   308,   309,   310,   311,   312,   313,
     314,   315,   316,   317,   318,    -1,   320,    -1,   322,   323,
     324,   325,   326,   327,   328,    11,    12,    -1,    -1,    -1,
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
     116,   117,   118,   119,   120,   121,   122,    -1,   124,    -1,
      -1,    -1,   128,    -1,   130,   131,   132,   133,    -1,   135,
      -1,    -1,    -1,   139,   140,   141,   142,   143,    -1,    -1,
      -1,    -1,    -1,    -1,   150,   151,   152,   153,   154,   155,
     156,   157,   158,    -1,   160,   161,   162,   163,   164,    -1,
      -1,    -1,   168,   169,   170,   171,   172,   173,   174,   175,
      -1,   177,    -1,   179,   180,    -1,    -1,   183,   184,   185,
      -1,    -1,    -1,   189,    -1,   191,   192,   193,    -1,   195,
     196,    -1,   198,    -1,   200,    -1,   202,   203,   204,   205,
     206,   207,   208,   209,   210,   211,   212,    -1,    -1,   215,
     216,   217,    -1,   219,   220,   221,   222,   223,   224,   225,
     226,   227,   228,   229,   230,   231,   232,   233,   234,   235,
     236,   237,   238,   239,   240,   241,   242,   243,   244,   245,
     246,   247,   248,   249,   250,   251,   252,   253,   254,   255,
     256,   257,   258,   259,   260,   261,   262,   263,   264,   265,
      -1,   267,   268,   269,    -1,   271,   272,   273,   274,   275,
     276,   277,   278,   279,   280,   281,   282,   283,   284,   285,
     286,   287,   288,   289,   290,   291,   292,   293,   294,   295,
     296,   297,   298,   299,   300,   301,   302,   303,   304,   305,
     306,   307,   308,   309,   310,   311,   312,   313,   314,   315,
     316,   317,   318,    -1,   320,    -1,   322,   323,   324,   325,
     326,   327,   328,    11,    12,    -1,    -1,    -1,    16,    -1,
      -1,    -1,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    -1,    32,    -1,    -1,    35,    36,    -1,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    -1,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    -1,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    -1,    97,
      98,    99,   100,   101,    -1,   103,   104,   105,    -1,    -1,
     108,   109,    -1,    -1,    -1,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,    -1,   124,    -1,    -1,    -1,
     128,    -1,   130,   131,   132,   133,    -1,   135,    -1,    -1,
      -1,   139,   140,   141,   142,   143,    -1,    -1,    -1,    -1,
      -1,    -1,   150,   151,   152,   153,   154,   155,   156,   157,
     158,    -1,   160,   161,   162,   163,   164,    -1,    -1,    -1,
     168,   169,   170,   171,   172,   173,   174,   175,    -1,   177,
      -1,   179,   180,    -1,    -1,   183,   184,   185,    -1,    -1,
      -1,   189,    -1,   191,   192,   193,    -1,   195,   196,    -1,
     198,    -1,   200,    -1,   202,   203,   204,   205,   206,   207,
     208,   209,   210,   211,   212,    -1,    -1,   215,   216,   217,
      -1,   219,   220,   221,   222,   223,   224,   225,   226,   227,
     228,   229,   230,   231,   232,   233,   234,   235,   236,   237,
     238,   239,   240,   241,   242,   243,   244,   245,   246,   247,
     248,   249,   250,   251,   252,   253,   254,   255,   256,   257,
     258,   259,   260,   261,   262,   263,   264,   265,    -1,   267,
     268,   269,    -1,   271,   272,   273,   274,   275,   276,   277,
     278,   279,   280,   281,   282,   283,   284,   285,   286,   287,
     288,   289,   290,   291,   292,   293,   294,   295,   296,   297,
     298,   299,   300,   301,   302,   303,   304,   305,   306,   307,
     308,   309,   310,   311,   312,   313,   314,   315,   316,   317,
     318,    -1,   320,    -1,   322,   323,   324,   325,   326,   327,
     328,    11,    12,    -1,    -1,    -1,    16,    -1,    -1,    -1,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    -1,    32,    -1,    -1,    35,    36,    -1,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    -1,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    -1,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    -1,    97,    98,    99,
     100,   101,    -1,   103,   104,   105,    -1,    -1,   108,   109,
      -1,    -1,    -1,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,    -1,   124,    -1,    -1,    -1,   128,    -1,
     130,   131,   132,   133,    -1,   135,    -1,    -1,    -1,   139,
     140,   141,   142,   143,    -1,    -1,    -1,    -1,    -1,    -1,
     150,   151,   152,   153,   154,   155,   156,   157,   158,    -1,
     160,   161,   162,   163,   164,    -1,    -1,    -1,   168,   169,
     170,   171,   172,   173,   174,   175,    -1,   177,    -1,   179,
     180,    -1,    -1,   183,   184,   185,    -1,    -1,    -1,   189,
      -1,   191,   192,   193,    -1,   195,   196,    -1,   198,    -1,
     200,    -1,   202,   203,   204,   205,   206,   207,   208,   209,
     210,   211,   212,    -1,    -1,   215,   216,   217,    -1,   219,
     220,   221,   222,   223,   224,   225,   226,   227,   228,   229,
     230,   231,   232,   233,   234,   235,   236,   237,   238,   239,
     240,   241,   242,   243,   244,   245,   246,   247,   248,   249,
     250,   251,   252,   253,   254,   255,   256,   257,   258,   259,
     260,   261,   262,   263,   264,   265,    -1,   267,   268,   269,
      -1,   271,   272,   273,   274,   275,   276,   277,   278,   279,
     280,   281,   282,   283,   284,   285,   286,   287,   288,   289,
     290,   291,   292,   293,   294,   295,   296,   297,   298,   299,
     300,   301,   302,   303,   304,   305,   306,   307,   308,   309,
     310,   311,   312,   313,   314,   315,   316,   317,   318,    -1,
     320,    -1,   322,   323,   324,   325,   326,   327,   328,    11,
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
     122,    -1,   124,    -1,    -1,    -1,   128,    -1,   130,   131,
     132,   133,    -1,   135,    -1,    -1,    -1,   139,   140,   141,
     142,   143,    -1,    -1,    -1,    -1,    -1,    -1,   150,   151,
     152,   153,   154,   155,   156,   157,   158,    -1,   160,   161,
     162,   163,   164,    -1,    -1,    -1,   168,   169,   170,   171,
     172,   173,   174,   175,    -1,   177,    -1,   179,   180,    -1,
      -1,   183,   184,   185,    -1,    -1,    -1,   189,    -1,   191,
     192,   193,    -1,   195,   196,    -1,   198,    -1,   200,    -1,
     202,   203,   204,   205,   206,   207,   208,   209,   210,   211,
     212,    -1,    -1,   215,   216,   217,    -1,   219,   220,   221,
     222,   223,   224,   225,   226,   227,   228,   229,   230,   231,
     232,   233,   234,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,   245,   246,   247,   248,   249,   250,   251,
     252,   253,   254,   255,   256,   257,   258,   259,   260,   261,
     262,   263,   264,   265,    -1,   267,   268,   269,    -1,   271,
     272,   273,   274,   275,   276,   277,   278,   279,   280,   281,
     282,   283,   284,   285,   286,   287,   288,   289,   290,   291,
     292,   293,   294,   295,   296,   297,   298,   299,   300,   301,
     302,   303,   304,   305,   306,   307,   308,   309,   310,   311,
     312,   313,   314,   315,   316,   317,   318,    -1,   320,    -1,
     322,   323,   324,   325,   326,   327,   328,    11,    12,    -1,
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
     114,   115,   116,   117,   118,   119,   120,   121,   122,    -1,
     124,    -1,    -1,    -1,   128,    -1,   130,   131,   132,   133,
      -1,   135,    -1,    -1,    -1,   139,   140,   141,   142,   143,
      -1,    -1,    -1,    -1,    -1,    -1,   150,   151,   152,   153,
     154,   155,   156,   157,   158,    -1,   160,   161,    -1,   163,
     164,    -1,    -1,    -1,   168,   169,   170,   171,   172,   173,
     174,   175,    -1,   177,    -1,    -1,    -1,    -1,    -1,   183,
     184,   185,    -1,    -1,    -1,   189,    -1,   191,   192,   193,
      -1,   195,   196,    -1,   198,    -1,   200,    -1,   202,   203,
     204,   205,   206,   207,   208,   209,   210,   211,   212,    -1,
      -1,   215,   216,   217,    -1,   219,   220,   221,   222,   223,
     224,   225,   226,   227,   228,   229,   230,   231,   232,   233,
     234,   235,   236,   237,   238,   239,   240,   241,   242,   243,
     244,   245,   246,   247,   248,   249,   250,   251,   252,   253,
     254,   255,   256,   257,   258,   259,   260,   261,   262,   263,
     264,   265,    -1,   267,   268,   269,    -1,   271,   272,   273,
     274,   275,   276,   277,   278,   279,   280,   281,   282,   283,
     284,   285,   286,   287,   288,   289,   290,   291,   292,   293,
     294,   295,   296,   297,   298,   299,   300,   301,   302,   303,
     304,   305,   306,   307,   308,   309,   310,   311,   312,   313,
     314,   315,   316,   317,   318,    -1,   320,    -1,   322,   323,
     324,   325,   326,   327,   328,    11,    12,    -1,    -1,    -1,
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
     116,   117,   118,   119,   120,   121,   122,    -1,   124,    -1,
      -1,    -1,   128,    -1,   130,   131,   132,   133,    -1,   135,
      -1,    -1,    -1,   139,   140,   141,   142,   143,    -1,    -1,
      -1,    -1,    -1,    -1,   150,   151,   152,   153,   154,   155,
     156,   157,   158,    -1,   160,   161,    -1,   163,   164,    -1,
      -1,    -1,   168,   169,   170,   171,   172,   173,   174,   175,
      -1,   177,    -1,    -1,    -1,    -1,    -1,   183,   184,   185,
      -1,    -1,    -1,   189,    -1,   191,   192,   193,    -1,    -1,
      -1,    -1,   198,    -1,   200,    -1,   202,   203,   204,   205,
     206,   207,   208,   209,   210,   211,   212,    -1,    -1,   215,
     216,   217,    -1,   219,   220,   221,   222,   223,   224,   225,
     226,   227,   228,   229,   230,   231,   232,   233,   234,   235,
     236,   237,   238,   239,   240,   241,   242,   243,   244,   245,
     246,   247,   248,   249,   250,   251,   252,   253,   254,   255,
     256,   257,   258,   259,   260,   261,   262,   263,   264,   265,
      -1,   267,   268,   269,    -1,   271,   272,   273,   274,   275,
     276,   277,   278,   279,   280,   281,   282,   283,   284,   285,
     286,   287,   288,   289,   290,   291,   292,   293,   294,   295,
     296,   297,   298,   299,   300,   301,   302,   303,   304,   305,
     306,   307,   308,   309,   310,   311,   312,   313,   314,   315,
     316,   317,   318,    -1,   320,    -1,   322,   323,   324,   325,
     326,   327,   328,    11,    12,    -1,    -1,    -1,    16,    -1,
      -1,    -1,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    -1,    32,    -1,    -1,    35,    36,    -1,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    -1,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    -1,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    -1,    97,
      98,    99,   100,   101,    -1,   103,   104,   105,    -1,    -1,
     108,   109,    -1,    -1,    -1,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,    -1,   124,    -1,    -1,    -1,
     128,    -1,   130,   131,   132,   133,    -1,   135,    -1,    -1,
      -1,   139,    -1,   141,   142,   143,    -1,    -1,    -1,    -1,
      -1,    -1,   150,   151,   152,   153,    -1,   155,   156,   157,
     158,    -1,   160,   161,    -1,   163,   164,    -1,    -1,    -1,
     168,   169,   170,   171,   172,   173,   174,   175,    -1,   177,
      -1,    -1,    -1,    -1,    -1,   183,   184,   185,    -1,    -1,
      -1,   189,    -1,   191,   192,   193,    -1,    -1,    -1,    -1,
     198,    -1,   200,    -1,   202,    -1,   204,    -1,   206,   207,
     208,   209,   210,   211,   212,    -1,    -1,   215,   216,   217,
      -1,   219,   220,   221,   222,   223,   224,   225,   226,   227,
     228,   229,   230,   231,   232,   233,   234,   235,   236,   237,
     238,   239,   240,   241,   242,   243,   244,   245,   246,   247,
     248,   249,   250,   251,   252,   253,   254,   255,   256,   257,
     258,   259,   260,   261,   262,   263,   264,   265,    -1,   267,
     268,   269,    -1,   271,   272,   273,   274,   275,   276,   277,
     278,   279,   280,   281,   282,   283,   284,   285,   286,   287,
     288,   289,   290,   291,   292,   293,   294,   295,   296,   297,
     298,   299,   300,   301,   302,   303,   304,   305,   306,   307,
     308,   309,   310,   311,   312,   313,   314,   315,   316,   317,
     318,    -1,   320,    -1,   322,   323,   324,   325,   326,   327,
     328,    11,    12,    -1,    -1,    -1,    16,    -1,    -1,    -1,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    -1,    32,    -1,    -1,    35,    36,    -1,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    -1,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    -1,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    -1,    97,    98,    99,
     100,   101,    -1,   103,   104,   105,    -1,    -1,   108,   109,
      -1,    -1,    -1,   113,   114,   115,   116,   117,   118,   119,
      -1,   121,   122,    -1,   124,    -1,    -1,    -1,   128,    -1,
     130,   131,   132,   133,    -1,   135,    -1,    -1,    -1,   139,
      -1,   141,   142,   143,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   151,   152,   153,    -1,   155,   156,   157,    -1,    -1,
      -1,   161,    -1,    -1,   164,    -1,    -1,    -1,   168,   169,
     170,   171,   172,   173,   174,   175,    -1,   177,    -1,    -1,
     180,    -1,    -1,   183,   184,   185,    -1,    -1,    -1,   189,
      -1,   191,   192,   193,    -1,    -1,    -1,    -1,    -1,    -1,
     200,    -1,   202,    -1,   204,    -1,   206,   207,   208,   209,
     210,   211,   212,    -1,    -1,   215,   216,   217,    -1,   219,
     220,   221,   222,   223,   224,   225,   226,   227,   228,   229,
     230,   231,   232,   233,   234,   235,   236,   237,   238,   239,
     240,   241,   242,   243,   244,   245,   246,   247,   248,   249,
     250,   251,   252,   253,   254,   255,   256,   257,   258,   259,
     260,   261,   262,   263,   264,   265,    -1,   267,   268,   269,
      -1,   271,   272,   273,   274,   275,   276,   277,   278,   279,
     280,   281,   282,   283,   284,   285,   286,   287,   288,   289,
     290,   291,   292,   293,   294,   295,   296,   297,   298,   299,
     300,   301,   302,   303,   304,   305,   306,   307,   308,   309,
     310,   311,   312,   313,   314,   315,   316,   317,   318,    -1,
     320,    -1,   322,   323,   324,   325,   326,   327,   328,    11,
      12,    -1,    -1,    -1,    16,    -1,    -1,    -1,    20,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    28,    -1,    -1,    -1,
      -1,    -1,    -1,    35,    36,    -1,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    -1,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    -1,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    -1,    97,    98,    99,   100,    -1,
      -1,   103,   104,   105,    -1,    -1,   108,   109,    -1,    -1,
      -1,    -1,   114,   115,   116,   117,   118,   119,   120,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   130,   131,
     132,   133,    -1,   135,    -1,    -1,    -1,   139,   140,   141,
     142,   143,    -1,    -1,    -1,    -1,    -1,    -1,   150,   151,
     152,    -1,   154,   155,   156,    -1,    -1,    -1,    -1,    -1,
      -1,   163,   164,    -1,    -1,    -1,   168,   169,   170,   171,
     172,   173,   174,   175,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   183,   184,   185,    -1,    -1,    -1,   189,    -1,   191,
     192,   193,    -1,    -1,    -1,    -1,   198,    -1,   200,    -1,
     202,   203,   204,   205,   206,   207,   208,   209,   210,   211,
     212,    -1,    -1,   215,   216,    -1,    -1,   219,   220,   221,
     222,   223,   224,   225,   226,   227,   228,   229,   230,   231,
     232,   233,   234,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,   245,   246,   247,   248,   249,   250,   251,
     252,   253,   254,   255,   256,   257,   258,   259,   260,   261,
     262,   263,   264,   265,    -1,   267,   268,   269,    -1,   271,
     272,   273,   274,   275,   276,   277,   278,   279,   280,   281,
     282,   283,   284,   285,   286,   287,   288,   289,   290,   291,
     292,   293,   294,   295,   296,   297,   298,   299,   300,   301,
     302,   303,   304,   305,   306,   307,   308,   309,   310,   311,
     312,   313,   314,   315,   316,   317,    -1,    -1,    -1,    -1,
     322,   323,   324,   325,   326,   327,   328,    11,    12,    -1,
      -1,    -1,    16,    -1,    -1,    -1,    20,    -1,    -1,    -1,
      -1,    -1,    -1,    27,    28,    -1,    -1,    -1,    -1,    -1,
      -1,    35,    36,    -1,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    -1,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    -1,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    -1,    97,    98,    99,   100,    -1,    -1,   103,
     104,   105,    -1,    -1,   108,   109,    -1,    -1,    -1,    -1,
     114,   115,   116,   117,   118,   119,   120,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   130,   131,   132,   133,
      -1,   135,    -1,    -1,    -1,   139,   140,   141,   142,   143,
      -1,    -1,    -1,    -1,    -1,    -1,   150,   151,   152,    -1,
     154,   155,   156,    -1,    -1,    -1,    -1,    -1,    -1,   163,
     164,    -1,    -1,    -1,   168,   169,   170,   171,   172,   173,
     174,   175,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   183,
     184,   185,    -1,    -1,    -1,   189,    -1,   191,   192,   193,
      -1,    -1,    -1,    -1,   198,    -1,   200,    -1,   202,   203,
     204,   205,   206,   207,   208,   209,   210,   211,   212,    -1,
      -1,   215,   216,    -1,    -1,   219,   220,   221,   222,   223,
     224,   225,   226,   227,   228,   229,   230,   231,   232,   233,
     234,   235,   236,   237,   238,   239,   240,   241,   242,   243,
     244,   245,   246,   247,   248,   249,   250,   251,   252,   253,
     254,   255,   256,   257,   258,   259,   260,   261,   262,   263,
     264,   265,    -1,   267,   268,   269,    -1,   271,   272,   273,
     274,   275,   276,   277,   278,   279,   280,   281,   282,   283,
     284,   285,   286,   287,   288,   289,   290,   291,   292,   293,
     294,   295,   296,   297,   298,   299,   300,   301,   302,   303,
     304,   305,   306,   307,   308,   309,   310,   311,   312,   313,
     314,   315,   316,   317,    16,    -1,    -1,    -1,   322,   323,
     324,   325,   326,   327,   328,    27,    28,    29,    30,    -1,
      -1,    -1,    -1,    35,    36,    -1,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    -1,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    -1,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    -1,    97,    98,    99,   100,    -1,
      -1,   103,   104,   105,    -1,    -1,   108,   109,    -1,    -1,
      -1,    -1,   114,   115,   116,   117,   118,   119,   120,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   130,   131,
     132,   133,    -1,   135,    -1,    -1,    -1,   139,   140,   141,
     142,   143,    -1,    -1,    -1,    -1,    -1,    -1,   150,   151,
     152,    -1,   154,   155,   156,    -1,    -1,    -1,   160,    -1,
      -1,   163,   164,    -1,    -1,    -1,   168,   169,   170,   171,
     172,   173,   174,   175,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   183,   184,   185,    -1,    -1,    -1,   189,   190,   191,
     192,   193,    -1,    -1,    -1,    -1,   198,    -1,   200,    -1,
     202,   203,   204,   205,   206,   207,   208,   209,   210,   211,
     212,    -1,    -1,   215,   216,    -1,    -1,   219,   220,   221,
     222,   223,   224,   225,   226,   227,   228,   229,   230,   231,
     232,   233,   234,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,   245,   246,   247,   248,   249,   250,   251,
     252,   253,   254,   255,   256,   257,   258,   259,   260,   261,
     262,   263,   264,   265,    -1,   267,   268,   269,    -1,   271,
     272,   273,   274,   275,   276,   277,   278,   279,   280,   281,
     282,   283,   284,   285,   286,   287,   288,   289,   290,   291,
     292,   293,   294,   295,   296,   297,   298,   299,   300,   301,
     302,   303,   304,   305,   306,   307,   308,   309,   310,   311,
     312,   313,   314,   315,   316,   317,    16,    -1,    -1,    -1,
     322,   323,   324,   325,   326,   327,   328,    27,    28,    -1,
      -1,    -1,    32,    -1,    -1,    35,    36,    -1,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    -1,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    -1,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    -1,    97,    98,    99,
     100,    -1,    -1,   103,   104,   105,    -1,    -1,   108,   109,
      -1,    -1,    -1,    -1,   114,   115,   116,   117,   118,   119,
     120,   121,   122,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     130,   131,   132,   133,    -1,   135,    -1,    -1,    -1,   139,
     140,   141,   142,   143,    -1,    -1,    -1,    -1,    -1,    -1,
     150,   151,   152,    -1,   154,   155,   156,    -1,    -1,    -1,
     160,    -1,    -1,   163,   164,    -1,    -1,    -1,   168,   169,
     170,   171,   172,   173,   174,   175,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   183,   184,   185,    -1,    -1,    -1,   189,
      -1,   191,   192,   193,    -1,    -1,    -1,    -1,    -1,    -1,
     200,    -1,   202,   203,   204,   205,   206,   207,   208,   209,
     210,   211,   212,    -1,    -1,   215,   216,    -1,    -1,   219,
     220,   221,   222,   223,   224,   225,   226,   227,   228,   229,
     230,   231,   232,   233,   234,   235,   236,   237,   238,   239,
     240,   241,   242,   243,   244,   245,   246,   247,   248,   249,
     250,   251,   252,   253,   254,   255,   256,   257,   258,   259,
     260,   261,   262,   263,   264,   265,    -1,   267,   268,   269,
      -1,   271,   272,   273,   274,   275,   276,   277,   278,   279,
     280,   281,   282,   283,   284,   285,   286,   287,   288,   289,
     290,   291,   292,   293,   294,   295,   296,   297,   298,   299,
     300,   301,   302,   303,   304,   305,   306,   307,   308,   309,
     310,   311,   312,   313,   314,   315,   316,   317,    16,    -1,
      -1,    -1,   322,   323,   324,   325,   326,   327,   328,    27,
      28,    29,    30,    -1,    -1,    -1,    -1,    35,    36,    -1,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    -1,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    -1,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    -1,    97,
      98,    99,   100,    -1,    -1,   103,   104,   105,    -1,    -1,
     108,   109,    -1,    -1,    -1,    -1,   114,   115,   116,   117,
     118,   119,   120,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   130,   131,   132,   133,    -1,   135,    -1,    -1,
      -1,   139,   140,   141,   142,   143,    -1,    -1,    -1,    -1,
      -1,    -1,   150,   151,   152,    -1,   154,   155,   156,    -1,
      -1,    -1,   160,    -1,    -1,   163,   164,    -1,    -1,    -1,
     168,   169,   170,   171,   172,   173,   174,   175,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   183,   184,   185,    -1,    -1,
      -1,   189,   190,   191,   192,   193,    -1,    -1,    -1,    -1,
      -1,    -1,   200,    -1,   202,   203,   204,   205,   206,   207,
     208,   209,   210,   211,   212,    -1,    -1,   215,   216,    -1,
      -1,   219,   220,   221,   222,   223,   224,   225,   226,   227,
     228,   229,   230,   231,   232,   233,   234,   235,   236,   237,
     238,   239,   240,   241,   242,   243,   244,   245,   246,   247,
     248,   249,   250,   251,   252,   253,   254,   255,   256,   257,
     258,   259,   260,   261,   262,   263,   264,   265,    -1,   267,
     268,   269,    -1,   271,   272,   273,   274,   275,   276,   277,
     278,   279,   280,   281,   282,   283,   284,   285,   286,   287,
     288,   289,   290,   291,   292,   293,   294,   295,   296,   297,
     298,   299,   300,   301,   302,   303,   304,   305,   306,   307,
     308,   309,   310,   311,   312,   313,   314,   315,   316,   317,
      16,    -1,    -1,    -1,   322,   323,   324,   325,   326,   327,
     328,    27,    28,    29,    30,    -1,    -1,    -1,    -1,    35,
      36,    -1,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    -1,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    -1,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    95,
      -1,    97,    98,    99,   100,    -1,    -1,   103,   104,   105,
      -1,    -1,   108,   109,    -1,    -1,    -1,    -1,   114,   115,
     116,   117,   118,   119,   120,   121,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   130,   131,   132,   133,    -1,   135,
      -1,    -1,    -1,   139,   140,   141,   142,   143,    -1,    -1,
      -1,    -1,    -1,    -1,   150,   151,   152,    -1,   154,   155,
     156,    -1,    -1,    -1,   160,    -1,    -1,   163,   164,    -1,
      -1,    -1,   168,   169,   170,   171,   172,   173,   174,   175,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   183,   184,   185,
      -1,    -1,    -1,   189,    -1,   191,   192,   193,    -1,    -1,
      -1,    -1,    -1,    -1,   200,    -1,   202,   203,   204,   205,
     206,   207,   208,   209,   210,   211,   212,    -1,    -1,   215,
     216,    -1,    -1,   219,   220,   221,   222,   223,   224,   225,
     226,   227,   228,   229,   230,   231,   232,   233,   234,   235,
     236,   237,   238,   239,   240,   241,   242,   243,   244,   245,
     246,   247,   248,   249,   250,   251,   252,   253,   254,   255,
     256,   257,   258,   259,   260,   261,   262,   263,   264,   265,
      -1,   267,   268,   269,    -1,   271,   272,   273,   274,   275,
     276,   277,   278,   279,   280,   281,   282,   283,   284,   285,
     286,   287,   288,   289,   290,   291,   292,   293,   294,   295,
     296,   297,   298,   299,   300,   301,   302,   303,   304,   305,
     306,   307,   308,   309,   310,   311,   312,   313,   314,   315,
     316,   317,    16,    -1,    -1,    -1,   322,   323,   324,   325,
     326,   327,   328,    27,    28,    29,    30,    -1,    -1,    -1,
      -1,    35,    36,    -1,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    -1,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    -1,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    -1,    97,    98,    99,   100,    -1,    -1,   103,
     104,   105,    -1,    -1,   108,   109,    -1,    -1,    -1,    -1,
     114,   115,   116,   117,   118,   119,   120,   121,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   130,   131,   132,   133,
      -1,   135,    -1,    -1,    -1,   139,   140,   141,   142,   143,
      -1,    -1,    -1,    -1,    -1,    -1,   150,   151,   152,    -1,
     154,   155,   156,    -1,    -1,    -1,   160,    -1,    -1,   163,
     164,    -1,    -1,    -1,   168,   169,   170,   171,   172,   173,
     174,   175,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   183,
     184,   185,    -1,    -1,    -1,   189,    -1,   191,   192,   193,
      -1,    -1,    -1,    -1,    -1,    -1,   200,    -1,   202,   203,
     204,   205,   206,   207,   208,   209,   210,   211,   212,    -1,
      -1,   215,   216,    -1,    -1,   219,   220,   221,   222,   223,
     224,   225,   226,   227,   228,   229,   230,   231,   232,   233,
     234,   235,   236,   237,   238,   239,   240,   241,   242,   243,
     244,   245,   246,   247,   248,   249,   250,   251,   252,   253,
     254,   255,   256,   257,   258,   259,   260,   261,   262,   263,
     264,   265,    -1,   267,   268,   269,    -1,   271,   272,   273,
     274,   275,   276,   277,   278,   279,   280,   281,   282,   283,
     284,   285,   286,   287,   288,   289,   290,   291,   292,   293,
     294,   295,   296,   297,   298,   299,   300,   301,   302,   303,
     304,   305,   306,   307,   308,   309,   310,   311,   312,   313,
     314,   315,   316,   317,    16,    -1,    -1,    -1,   322,   323,
     324,   325,   326,   327,   328,    27,    28,    29,    30,    -1,
      -1,    -1,    -1,    35,    36,    -1,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    -1,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    -1,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    -1,    97,    98,    99,   100,    -1,
      -1,   103,   104,   105,    -1,    -1,   108,   109,    -1,    -1,
      -1,    -1,   114,   115,   116,   117,   118,   119,   120,   121,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   130,   131,
     132,   133,    -1,   135,    -1,    -1,    -1,   139,   140,   141,
     142,   143,    -1,    -1,    -1,    -1,    -1,    -1,   150,   151,
     152,    -1,   154,   155,   156,    -1,    -1,    -1,   160,    -1,
      -1,   163,   164,    -1,    -1,    -1,   168,   169,   170,   171,
     172,   173,   174,   175,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   183,   184,   185,    -1,    -1,    -1,   189,    -1,   191,
     192,   193,    -1,    -1,    -1,    -1,    -1,    -1,   200,    -1,
     202,   203,   204,   205,   206,   207,   208,   209,   210,   211,
     212,    -1,    -1,   215,   216,    -1,    -1,   219,   220,   221,
     222,   223,   224,   225,   226,   227,   228,   229,   230,   231,
     232,   233,   234,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,   245,   246,   247,   248,   249,   250,   251,
     252,   253,   254,   255,   256,   257,   258,   259,   260,   261,
     262,   263,   264,   265,    -1,   267,   268,   269,    -1,   271,
     272,   273,   274,   275,   276,   277,   278,   279,   280,   281,
     282,   283,   284,   285,   286,   287,   288,   289,   290,   291,
     292,   293,   294,   295,   296,   297,   298,   299,   300,   301,
     302,   303,   304,   305,   306,   307,   308,   309,   310,   311,
     312,   313,   314,   315,   316,   317,    16,    -1,    -1,    -1,
     322,   323,   324,   325,   326,   327,   328,    27,    28,    29,
      30,    -1,    -1,    -1,    -1,    35,    36,    -1,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    -1,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    -1,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    -1,    97,    98,    99,
     100,    -1,    -1,   103,   104,   105,    -1,    -1,   108,   109,
      -1,    -1,    -1,    -1,   114,   115,   116,   117,   118,   119,
     120,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     130,   131,   132,   133,    -1,   135,    -1,    -1,    -1,   139,
     140,   141,   142,   143,    -1,    -1,    -1,    -1,    -1,    -1,
     150,   151,   152,    -1,   154,   155,   156,    -1,    -1,    -1,
     160,    -1,    -1,   163,   164,    -1,    -1,    -1,   168,   169,
     170,   171,   172,   173,   174,   175,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   183,   184,   185,    -1,    -1,    -1,   189,
      -1,   191,   192,   193,    -1,    -1,    -1,    -1,    -1,    -1,
     200,    -1,   202,   203,   204,   205,   206,   207,   208,   209,
     210,   211,   212,    -1,    -1,   215,   216,    -1,    -1,   219,
     220,   221,   222,   223,   224,   225,   226,   227,   228,   229,
     230,   231,   232,   233,   234,   235,   236,   237,   238,   239,
     240,   241,   242,   243,   244,   245,   246,   247,   248,   249,
     250,   251,   252,   253,   254,   255,   256,   257,   258,   259,
     260,   261,   262,   263,   264,   265,    -1,   267,   268,   269,
      -1,   271,   272,   273,   274,   275,   276,   277,   278,   279,
     280,   281,   282,   283,   284,   285,   286,   287,   288,   289,
     290,   291,   292,   293,   294,   295,   296,   297,   298,   299,
     300,   301,   302,   303,   304,   305,   306,   307,   308,   309,
     310,   311,   312,   313,   314,   315,   316,   317,    16,    -1,
      -1,    -1,   322,   323,   324,   325,   326,   327,   328,    27,
      28,    29,    30,    -1,    -1,    -1,    -1,    35,    36,    -1,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    -1,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    -1,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    -1,    97,
      98,    99,   100,    -1,    -1,   103,   104,   105,    -1,    -1,
     108,   109,    -1,    -1,    -1,    -1,   114,   115,   116,   117,
     118,   119,   120,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   130,   131,   132,   133,    -1,   135,    -1,    -1,
      -1,   139,   140,   141,   142,   143,    -1,    -1,    -1,    -1,
      -1,    -1,   150,   151,   152,    -1,   154,   155,   156,    -1,
      -1,    -1,   160,    -1,    -1,   163,   164,    -1,    -1,    -1,
     168,   169,   170,   171,   172,   173,   174,   175,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   183,   184,   185,    -1,    -1,
      -1,   189,    -1,   191,   192,   193,    -1,    -1,    -1,    -1,
      -1,    -1,   200,    -1,   202,   203,   204,   205,   206,   207,
     208,   209,   210,   211,   212,    -1,    -1,   215,   216,    -1,
      -1,   219,   220,   221,   222,   223,   224,   225,   226,   227,
     228,   229,   230,   231,   232,   233,   234,   235,   236,   237,
     238,   239,   240,   241,   242,   243,   244,   245,   246,   247,
     248,   249,   250,   251,   252,   253,   254,   255,   256,   257,
     258,   259,   260,   261,   262,   263,   264,   265,    -1,   267,
     268,   269,    -1,   271,   272,   273,   274,   275,   276,   277,
     278,   279,   280,   281,   282,   283,   284,   285,   286,   287,
     288,   289,   290,   291,   292,   293,   294,   295,   296,   297,
     298,   299,   300,   301,   302,   303,   304,   305,   306,   307,
     308,   309,   310,   311,   312,   313,   314,   315,   316,   317,
      16,    -1,    18,    19,   322,   323,   324,   325,   326,   327,
     328,    27,    28,    -1,    -1,    -1,    -1,    -1,    -1,    35,
      36,    -1,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    -1,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    -1,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    95,
      -1,    97,    98,    99,   100,    -1,    -1,   103,   104,   105,
      -1,    -1,   108,   109,    -1,    -1,    -1,    -1,   114,   115,
     116,   117,   118,   119,   120,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   130,   131,   132,   133,    -1,   135,
      -1,    -1,    -1,   139,   140,   141,   142,   143,    -1,    -1,
      -1,    -1,    -1,    -1,   150,   151,   152,    -1,   154,   155,
     156,    -1,    -1,    -1,    -1,    -1,    -1,   163,   164,    -1,
      -1,    -1,   168,   169,   170,   171,   172,   173,   174,   175,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   183,   184,   185,
      -1,    -1,    -1,   189,    -1,   191,   192,   193,    -1,    -1,
      -1,    -1,    -1,    -1,   200,    -1,   202,   203,   204,   205,
     206,   207,   208,   209,   210,   211,   212,    -1,    -1,   215,
     216,    -1,    -1,   219,   220,   221,   222,   223,   224,   225,
     226,   227,   228,   229,   230,   231,   232,   233,   234,   235,
     236,   237,   238,   239,   240,   241,   242,   243,   244,   245,
     246,   247,   248,   249,   250,   251,   252,   253,   254,   255,
     256,   257,   258,   259,   260,   261,   262,   263,   264,   265,
      -1,   267,   268,   269,    -1,   271,   272,   273,   274,   275,
     276,   277,   278,   279,   280,   281,   282,   283,   284,   285,
     286,   287,   288,   289,   290,   291,   292,   293,   294,   295,
     296,   297,   298,   299,   300,   301,   302,   303,   304,   305,
     306,   307,   308,   309,   310,   311,   312,   313,   314,   315,
     316,   317,    16,    -1,    -1,    -1,   322,   323,   324,   325,
     326,   327,   328,    27,    28,    -1,    -1,    -1,    -1,    -1,
      -1,    35,    36,    -1,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    -1,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    -1,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    -1,    97,    98,    99,   100,    -1,    -1,   103,
     104,   105,    -1,    -1,   108,   109,    -1,    -1,    -1,    -1,
     114,   115,   116,   117,   118,   119,   120,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   130,   131,   132,   133,
      -1,   135,    -1,    -1,    -1,   139,   140,   141,   142,   143,
      -1,    -1,    -1,    -1,    -1,    -1,   150,   151,   152,    -1,
     154,   155,   156,    -1,    -1,    -1,    -1,    -1,    -1,   163,
     164,    -1,    -1,    -1,   168,   169,   170,   171,   172,   173,
     174,   175,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   183,
     184,   185,    -1,    -1,    -1,   189,   190,   191,   192,   193,
      -1,    -1,    -1,    -1,   198,    -1,   200,    -1,   202,   203,
     204,   205,   206,   207,   208,   209,   210,   211,   212,    -1,
      -1,   215,   216,    -1,    -1,   219,   220,   221,   222,   223,
     224,   225,   226,   227,   228,   229,   230,   231,   232,   233,
     234,   235,   236,   237,   238,   239,   240,   241,   242,   243,
     244,   245,   246,   247,   248,   249,   250,   251,   252,   253,
     254,   255,   256,   257,   258,   259,   260,   261,   262,   263,
     264,   265,    -1,   267,   268,   269,    -1,   271,   272,   273,
     274,   275,   276,   277,   278,   279,   280,   281,   282,   283,
     284,   285,   286,   287,   288,   289,   290,   291,   292,   293,
     294,   295,   296,   297,   298,   299,   300,   301,   302,   303,
     304,   305,   306,   307,   308,   309,   310,   311,   312,   313,
     314,   315,   316,   317,    16,    -1,    -1,    -1,   322,   323,
     324,   325,   326,   327,   328,    27,    28,    -1,    -1,    -1,
      32,    -1,    -1,    35,    36,    -1,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    -1,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    -1,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    -1,    97,    98,    99,   100,    -1,
      -1,   103,   104,   105,    -1,    -1,   108,   109,    -1,    -1,
      -1,    -1,   114,   115,   116,   117,   118,   119,   120,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   130,   131,
     132,   133,    -1,   135,    -1,    -1,    -1,   139,   140,   141,
     142,   143,    -1,    -1,    -1,    -1,    -1,    -1,   150,   151,
     152,    -1,   154,   155,   156,    -1,    -1,    -1,    -1,    -1,
      -1,   163,   164,    -1,    -1,    -1,   168,   169,   170,   171,
     172,   173,   174,   175,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   183,   184,   185,    -1,    -1,    -1,   189,   190,   191,
     192,   193,    -1,    -1,    -1,    -1,    -1,    -1,   200,    -1,
     202,   203,   204,   205,   206,   207,   208,   209,   210,   211,
     212,    -1,    -1,   215,   216,    -1,    -1,   219,   220,   221,
     222,   223,   224,   225,   226,   227,   228,   229,   230,   231,
     232,   233,   234,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,   245,   246,   247,   248,   249,   250,   251,
     252,   253,   254,   255,   256,   257,   258,   259,   260,   261,
     262,   263,   264,   265,    -1,   267,   268,   269,    -1,   271,
     272,   273,   274,   275,   276,   277,   278,   279,   280,   281,
     282,   283,   284,   285,   286,   287,   288,   289,   290,   291,
     292,   293,   294,   295,   296,   297,   298,   299,   300,   301,
     302,   303,   304,   305,   306,   307,   308,   309,   310,   311,
     312,   313,   314,   315,   316,   317,    16,    -1,    -1,    -1,
     322,   323,   324,   325,   326,   327,   328,    27,    28,    -1,
      -1,    -1,    -1,    -1,    -1,    35,    36,    -1,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    -1,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    -1,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    -1,    97,    98,    99,
     100,    -1,    -1,   103,   104,   105,    -1,    -1,   108,   109,
      -1,    -1,    -1,    -1,   114,   115,   116,   117,   118,   119,
     120,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     130,   131,   132,   133,    -1,   135,    -1,    -1,    -1,   139,
     140,   141,   142,   143,    -1,    -1,    -1,    -1,    -1,    -1,
     150,   151,   152,    -1,   154,   155,   156,    -1,    -1,    -1,
      -1,    -1,    -1,   163,   164,    -1,    -1,    -1,   168,   169,
     170,   171,   172,   173,   174,   175,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   183,   184,   185,    -1,    -1,    -1,   189,
     190,   191,   192,   193,    -1,    -1,    -1,    -1,   198,    -1,
     200,    -1,   202,   203,   204,   205,   206,   207,   208,   209,
     210,   211,   212,    -1,    -1,   215,   216,    -1,    -1,   219,
     220,   221,   222,   223,   224,   225,   226,   227,   228,   229,
     230,   231,   232,   233,   234,   235,   236,   237,   238,   239,
     240,   241,   242,   243,   244,   245,   246,   247,   248,   249,
     250,   251,   252,   253,   254,   255,   256,   257,   258,   259,
     260,   261,   262,   263,   264,   265,    -1,   267,   268,   269,
      -1,   271,   272,   273,   274,   275,   276,   277,   278,   279,
     280,   281,   282,   283,   284,   285,   286,   287,   288,   289,
     290,   291,   292,   293,   294,   295,   296,   297,   298,   299,
     300,   301,   302,   303,   304,   305,   306,   307,   308,   309,
     310,   311,   312,   313,   314,   315,   316,   317,    16,    -1,
      -1,    -1,   322,   323,   324,   325,   326,   327,   328,    27,
      28,    -1,    -1,    -1,    -1,    -1,    -1,    35,    36,    -1,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    -1,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    -1,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    -1,    97,
      98,    99,   100,    -1,    -1,   103,   104,   105,    -1,    -1,
     108,   109,    -1,    -1,    -1,    -1,   114,   115,   116,   117,
     118,   119,   120,   121,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   130,   131,   132,   133,    -1,   135,    -1,    -1,
      -1,   139,   140,   141,   142,   143,    -1,    -1,    -1,    -1,
      -1,    -1,   150,   151,   152,    -1,   154,   155,   156,    -1,
      -1,    -1,    -1,    -1,    -1,   163,   164,    -1,    -1,    -1,
     168,   169,   170,   171,   172,   173,   174,   175,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   183,   184,   185,    -1,    -1,
      -1,   189,    -1,   191,   192,   193,    -1,    -1,    -1,    -1,
      -1,    -1,   200,    -1,   202,   203,   204,   205,   206,   207,
     208,   209,   210,   211,   212,    -1,    -1,   215,   216,    -1,
      -1,   219,   220,   221,   222,   223,   224,   225,   226,   227,
     228,   229,   230,   231,   232,   233,   234,   235,   236,   237,
     238,   239,   240,   241,   242,   243,   244,   245,   246,   247,
     248,   249,   250,   251,   252,   253,   254,   255,   256,   257,
     258,   259,   260,   261,   262,   263,   264,   265,    -1,   267,
     268,   269,    -1,   271,   272,   273,   274,   275,   276,   277,
     278,   279,   280,   281,   282,   283,   284,   285,   286,   287,
     288,   289,   290,   291,   292,   293,   294,   295,   296,   297,
     298,   299,   300,   301,   302,   303,   304,   305,   306,   307,
     308,   309,   310,   311,   312,   313,   314,   315,   316,   317,
      16,    -1,    -1,    -1,   322,   323,   324,   325,   326,   327,
     328,    27,    28,    -1,    -1,    -1,    -1,    -1,    -1,    35,
      36,    -1,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    -1,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    -1,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    95,
      -1,    97,    98,    99,   100,    -1,    -1,   103,   104,   105,
      -1,    -1,   108,   109,    -1,    -1,    -1,    -1,   114,   115,
     116,   117,   118,   119,   120,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   130,   131,   132,   133,    -1,   135,
      -1,    -1,    -1,   139,   140,   141,   142,   143,    -1,    -1,
      -1,    -1,    -1,    -1,   150,   151,   152,    -1,   154,   155,
     156,    -1,    -1,    -1,    -1,    -1,    -1,   163,   164,    -1,
      -1,    -1,   168,   169,   170,   171,   172,   173,   174,   175,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   183,   184,   185,
      -1,    -1,    -1,   189,    -1,   191,   192,   193,    -1,    -1,
      -1,    -1,    -1,    -1,   200,    -1,   202,   203,   204,   205,
     206,   207,   208,   209,   210,   211,   212,    -1,    -1,   215,
     216,    -1,    -1,   219,   220,   221,   222,   223,   224,   225,
     226,   227,   228,   229,   230,   231,   232,   233,   234,   235,
     236,   237,   238,   239,   240,   241,   242,   243,   244,   245,
     246,   247,   248,   249,   250,   251,   252,   253,   254,   255,
     256,   257,   258,   259,   260,   261,   262,   263,   264,   265,
      -1,   267,   268,   269,    -1,   271,   272,   273,   274,   275,
     276,   277,   278,   279,   280,   281,   282,   283,   284,   285,
     286,   287,   288,   289,   290,   291,   292,   293,   294,   295,
     296,   297,   298,   299,   300,   301,   302,   303,   304,   305,
     306,   307,   308,   309,   310,   311,   312,   313,   314,   315,
     316,   317,    -1,    -1,    -1,    -1,   322,   323,   324,   325,
     326,   327,   328,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    -1,    32,    -1,    -1,    35,    36,    -1,
      38,    39,    40,    41,    42,    43,    44,    45,    -1,    47,
      48,    49,    -1,    51,    52,    53,    54,    -1,    56,    -1,
      58,    59,    60,    61,    62,    63,    -1,    -1,    66,    67,
      68,    69,    70,    -1,    72,    73,    74,    -1,    -1,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    -1,    97,
      98,    99,   100,    -1,    -1,   103,   104,   105,    -1,    -1,
     108,   109,    -1,    -1,    -1,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,    -1,    -1,    -1,    -1,    -1,
     128,    -1,   130,   131,   132,   133,    -1,   135,    -1,    -1,
      -1,   139,   140,   141,   142,   143,    -1,    -1,    -1,    -1,
      -1,    -1,   150,   151,   152,   153,   154,   155,    -1,   157,
     158,    -1,   160,   161,    -1,   163,   164,    -1,    -1,    -1,
     168,   169,   170,   171,   172,   173,   174,   175,    -1,   177,
      -1,    -1,    -1,    -1,    -1,   183,   184,   185,    -1,    -1,
      -1,   189,    -1,   191,   192,   193,    -1,    -1,    -1,    -1,
      -1,    -1,   200,    -1,   202,   203,   204,   205,   206,   207,
     208,   209,   210,   211,   212,    -1,    -1,    -1,   216,   217,
      -1,   219,   220,   221,   222,   223,   224,   225,   226,   227,
     228,   229,   230,   231,   232,    -1,   234,   235,   236,   237,
     238,   239,   240,   241,   242,   243,   244,   245,   246,   247,
     248,   249,   250,    -1,   252,   253,   254,   255,   256,   257,
     258,   259,   260,   261,   262,   263,   264,   265,    -1,   267,
     268,   269,    -1,   271,   272,   273,   274,   275,   276,   277,
     278,   279,   280,   281,   282,   283,   284,   285,   286,   287,
     288,   289,   290,   291,   292,   293,   294,   295,   296,   297,
     298,   299,   300,   301,   302,   303,   304,   305,   306,   307,
     308,   309,   310,   311,   312,   313,   314,   315,   316,   317,
     318,    -1,   320,    27,    28,   323,   324,    -1,    -1,    -1,
      -1,    35,    36,    -1,    38,    39,    40,    -1,    42,    43,
      44,    45,    -1,    -1,    48,    49,    -1,    51,    52,    53,
      54,    -1,    56,    -1,    58,    59,    60,    61,    -1,    63,
      -1,    -1,    66,    67,    68,    69,    70,    -1,    72,    -1,
      74,    -1,    -1,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    -1,    97,    98,    99,    -1,    -1,    -1,   103,
     104,   105,    -1,    -1,   108,   109,    -1,    -1,    -1,    -1,
     114,   115,   116,   117,   118,   119,   120,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   130,   131,   132,   133,
      -1,   135,    -1,    -1,    -1,   139,   140,   141,   142,   143,
      -1,    -1,    -1,    -1,    -1,    -1,   150,   151,   152,    -1,
     154,   155,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   163,
     164,    -1,    -1,    -1,   168,   169,   170,   171,   172,   173,
     174,   175,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   183,
     184,   185,    -1,    -1,    -1,   189,    -1,   191,   192,   193,
      -1,    -1,    -1,    -1,    -1,    -1,   200,    -1,   202,   203,
     204,   205,   206,   207,   208,   209,   210,   211,   212,    -1,
      -1,    -1,   216,    -1,    -1,   219,   220,   221,   222,   223,
     224,   225,   226,   227,   228,   229,   230,   231,   232,    -1,
     234,   235,   236,   237,   238,   239,   240,   241,   242,   243,
     244,   245,   246,   247,   248,   249,   250,    -1,   252,   253,
     254,   255,   256,   257,   258,   259,   260,   261,   262,   263,
     264,   265,    -1,   267,   268,   269,    -1,   271,   272,   273,
     274,   275,   276,   277,   278,   279,   280,   281,   282,   283,
     284,   285,   286,   287,   288,   289,   290,   291,   292,   293,
     294,   295,   296,   297,   298,   299,   300,   301,   302,   303,
     304,   305,   306,   307,   308,   309,   310,   311,   312,   313,
     314,   315,   316,   317,    -1,    -1,    -1,    -1,    -1,   323,
     324
  };

  const short
  jsoniq_parser::yystos_[] =
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
     119,   120,   121,   122,   124,   128,   130,   131,   132,   133,
     135,   139,   140,   141,   142,   143,   150,   151,   152,   153,
     154,   155,   156,   157,   158,   160,   161,   162,   163,   164,
     168,   169,   170,   171,   172,   173,   174,   175,   177,   179,
     180,   183,   184,   185,   189,   191,   192,   193,   194,   195,
     196,   198,   200,   202,   203,   204,   205,   206,   207,   208,
     209,   210,   211,   212,   215,   216,   217,   219,   220,   221,
     222,   223,   224,   225,   226,   227,   228,   229,   230,   231,
     232,   233,   234,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,   245,   246,   247,   248,   249,   250,   251,
     252,   253,   254,   255,   256,   257,   258,   259,   260,   261,
     262,   263,   264,   265,   267,   268,   269,   271,   272,   273,
     274,   275,   276,   277,   278,   279,   280,   281,   282,   283,
     284,   285,   286,   287,   288,   289,   290,   291,   292,   293,
     294,   295,   296,   297,   298,   299,   300,   301,   302,   303,
     304,   305,   306,   307,   308,   309,   310,   311,   312,   313,
     314,   315,   316,   317,   318,   320,   322,   323,   324,   325,
     326,   327,   328,   329,   330,   343,   345,   346,   347,   348,
     349,   350,   351,   352,   353,   354,   355,   356,   357,   358,
     359,   360,   361,   364,   365,   366,   367,   368,   369,   373,
     374,   375,   379,   381,   382,   384,   391,   393,   396,   397,
     398,   400,   401,   402,   403,   404,   406,   407,   409,   410,
     411,   412,   413,   414,   416,   417,   420,   421,   422,   423,
     424,   430,   432,   434,   435,   436,   441,   460,   463,   467,
     470,   471,   477,   478,   479,   480,   481,   483,   484,   486,
     487,   488,   489,   490,   491,   492,   493,   494,   496,   497,
     498,   499,   502,   503,   504,   505,   506,   507,   508,   509,
     510,   511,   512,   513,   514,   515,   516,   517,   518,   519,
     522,   523,   524,   525,   526,   527,   528,   529,   530,   532,
     533,   534,   535,   536,   537,   551,   552,   554,   555,   556,
     557,   558,   559,   560,   561,   569,   570,   571,   572,   573,
     574,   575,   576,   577,   578,   579,   580,   583,   588,   589,
     590,   591,   592,   593,   596,   654,   655,   656,   657,   659,
     660,   661,   662,   663,   666,   667,   668,    35,    36,    51,
     220,   399,   400,   401,   399,   399,   399,   399,   400,   401,
     160,   160,    39,    40,    42,    43,    49,    59,    60,    77,
      82,   114,   115,   116,   164,   228,   269,   295,   303,   311,
     380,   381,   385,   386,   387,   164,   157,   160,   157,   160,
     157,   160,     1,   160,   388,   160,    27,    36,    63,   121,
     281,   440,   442,   443,   160,   157,   160,   160,   160,   121,
     157,   160,   160,   160,    79,   157,   229,   230,   126,   126,
     126,   157,   160,    41,    45,    47,    48,    49,    50,    53,
      62,    70,    73,    75,    76,    78,    93,    94,   100,   108,
     115,   117,   118,   139,   142,   143,   164,   172,   175,   183,
     184,   193,   206,   232,   234,   236,   237,   244,   246,   247,
     248,   251,   252,   265,   268,   323,   516,   667,   126,   121,
     408,   126,   126,    41,    46,    47,    55,    62,    64,    65,
      73,   100,   233,   666,   121,   126,   126,   399,   401,   422,
     658,   666,    50,    75,    76,   121,   157,   188,   252,   381,
     421,   423,   434,   190,   421,   666,   157,   157,   126,   666,
      18,    19,   666,   126,   126,   126,   508,   157,    33,   218,
     221,   233,   235,   322,   233,   235,   322,   233,   322,   233,
     238,   121,   249,   249,   250,   160,   157,   403,     1,   480,
     319,   421,   321,   421,   322,   330,   345,   345,     0,   347,
     348,    35,    51,   350,   367,     1,   194,   344,   194,   344,
      49,   115,   382,   402,   421,   110,   194,   110,   344,   194,
      44,    48,    53,    72,   171,   174,   189,   192,   216,   265,
     415,   425,   426,   431,   432,   433,   448,   449,   453,     3,
      69,    74,   121,   427,   170,    95,   136,   144,   145,   147,
     155,   159,   161,   166,   182,   207,   208,   209,   210,   211,
     212,   500,   501,   257,   102,   162,   179,   203,   120,   150,
     163,   198,   205,   214,   140,   154,    52,   204,   104,   105,
     162,   179,   498,   197,   157,   505,   508,   195,   196,   158,
     520,   521,   516,   520,   516,   123,   158,   160,   520,   160,
     149,   160,   187,   187,   187,   187,   187,   187,   383,   523,
     383,    32,   665,   185,   200,   185,   200,   169,   185,   667,
     666,   172,   206,    49,   666,   156,   121,    47,    49,    82,
     109,   171,   666,   229,   230,   231,   254,   627,   666,   666,
     312,   141,   146,   115,   295,   303,   385,   666,   400,   190,
     400,    47,    65,   190,   579,   580,   421,   190,   198,   666,
     437,   438,   666,   121,   190,   389,   390,   157,   405,   421,
       1,   164,   665,   116,   164,   363,   665,   666,   121,   146,
     110,   190,   421,    32,   190,   666,   666,   666,   461,   462,
     666,   400,   190,   421,   421,   581,   666,   400,   157,   157,
     421,   190,   198,   666,   666,   146,   461,   187,   187,   125,
     110,   187,   125,   160,   160,   160,   666,   157,   188,   426,
     190,    37,   539,   540,   541,   421,   421,     8,   178,    17,
     421,   218,    32,   422,   422,    41,    47,    62,    73,   100,
     519,   667,   422,   422,   422,   658,   422,   519,   422,   239,
     594,   595,   666,   194,   194,   422,   421,   401,   421,   253,
     418,   419,   437,   319,   321,   422,   345,   194,   344,   194,
     344,     3,   351,   367,   397,     1,   351,   367,   397,    35,
     368,   397,   368,   397,   388,   408,   344,   408,   422,   422,
     121,   171,   173,   173,   422,   402,   422,   292,   292,   437,
     121,   444,   479,   480,   483,   483,   483,   483,   482,   483,
     483,   483,    73,   484,   488,   488,   487,   489,   489,   489,
     489,   490,   490,   491,   491,   239,    97,    97,    97,   506,
     399,   508,   508,   421,   521,   160,   525,   527,   583,   666,
     188,   148,   190,   422,   531,   190,   153,   190,   531,   110,
     190,   190,   110,   110,   388,    32,   667,    83,    84,    85,
      86,    87,    88,    89,    90,    91,    92,   370,   371,   372,
      97,   141,   146,   376,   377,   378,   666,   164,   164,   370,
     665,   131,   136,    57,    59,   103,   259,   272,   274,   285,
     286,   287,   289,   291,   628,   629,   630,   631,   632,   633,
     640,   646,   647,   254,    97,   307,   666,   146,   422,   121,
     666,   666,   136,   187,   187,   190,   190,   187,   110,   190,
     110,   190,   110,    38,    97,    99,   151,   439,   440,   562,
     666,    97,   110,   190,   399,   190,   666,    99,    47,   666,
     665,    99,   146,   562,   666,   422,   443,   187,   190,   190,
     190,   190,   110,   191,   151,   562,   187,   190,   190,   157,
     187,   400,   400,   187,   110,   190,   110,   190,   146,   562,
     422,   191,   422,   422,   666,   422,   421,   421,   421,   666,
     540,   541,   134,   201,   187,   187,   187,   135,   194,    97,
     226,   227,   241,    97,   226,   227,   241,   241,   241,    97,
      97,   245,   233,   322,   110,   243,   146,   194,   190,   421,
     187,   517,   600,   419,   241,   367,    35,    35,   194,   344,
     194,   116,   405,   402,   666,   173,   422,   454,   455,   121,
     422,   450,   451,   666,    58,   222,   262,   428,   429,   483,
     157,   160,   267,   504,   583,   601,   604,   605,   606,   607,
     608,   612,   614,   616,   617,    49,   156,   160,   215,   325,
     326,   327,   328,   381,   563,   565,   567,   569,   584,   585,
     586,   587,   664,   666,   563,   495,   568,   666,   495,   187,
     188,   190,   421,   110,   190,   190,   523,   152,   168,   152,
     168,   141,   405,   388,   371,   136,   160,   565,   378,   422,
     562,   665,   665,   132,   133,   665,   285,   286,   287,   291,
     666,   271,   282,   271,   282,    32,   294,    99,   116,   160,
     634,   637,   628,    41,    46,    55,    62,    64,    73,   100,
     233,   325,   326,   327,   392,   569,   664,   235,   307,   316,
     422,   666,    97,   307,   665,   157,   581,   582,   666,   581,
     582,   121,   438,   131,   563,   121,   422,   151,   440,   151,
      38,   151,   439,   440,   562,   563,   390,    97,   187,   202,
     136,   362,   665,   164,   136,    99,   362,   422,   146,   440,
     157,   121,   422,   422,   151,   103,   464,   465,   466,   468,
     469,   103,   472,   473,   474,   475,   400,   187,   187,   157,
     581,   581,   422,   146,   194,   422,   125,   125,   190,   190,
     190,    37,   541,   134,   201,     9,    10,   106,   127,   129,
     157,   199,   536,   538,   549,   550,   553,   157,    32,   240,
     242,   422,   422,   422,   240,   242,   422,   422,   422,   422,
     422,   422,   422,   422,   422,   519,   121,   422,   422,    50,
      75,    76,   252,   402,   423,   434,   253,   597,   598,   157,
     214,   403,   422,   194,   116,   397,   397,   397,   454,    98,
     109,   119,   131,   456,   457,   458,   459,   110,   666,   109,
     452,   110,   151,   562,   262,    56,    61,    80,   121,   439,
     445,   446,   447,   429,   421,   601,   608,   157,   293,   485,
     653,    99,   172,   260,   261,   263,   270,   280,   292,   602,
     603,   622,   623,   624,   625,   648,   651,   266,   268,   609,
     627,   275,   613,   649,   255,   256,   278,   618,   619,   160,
     160,   190,   565,   160,   160,   160,   160,   160,    49,   585,
     586,   148,   179,   198,   564,   148,   190,   148,   422,   141,
     405,   583,   377,   294,    32,    99,   116,   160,   641,    32,
     634,   564,   564,   506,   295,   314,   562,   392,   235,   194,
     399,   190,   190,   148,   190,   190,   438,   151,   439,   666,
     422,   151,   422,   131,   422,   151,   440,   151,   563,   402,
     422,   665,   110,   362,   422,   146,   399,   462,   422,   422,
     116,   465,   466,   103,   426,   116,   466,   469,   121,   476,
     563,   103,   116,   473,   103,   116,   475,   187,   399,   190,
     190,   422,   422,   422,   202,   472,   136,   199,   538,     7,
     400,   666,   199,   549,   400,   194,   241,   241,   241,   241,
      99,   245,   245,   595,   426,   160,   160,   160,   426,   600,
     598,   517,   665,   132,   133,   458,   459,   459,   455,   146,
     562,   665,   451,   422,   151,   121,   121,   422,   666,   447,
      80,   187,   190,   601,   615,   258,   222,   262,   276,   283,
     652,    99,   264,   265,   650,   258,   605,   652,   487,   622,
     606,   151,   290,   610,   611,   650,   294,   621,    81,   620,
     190,   198,   563,   566,   190,   190,   190,   190,   190,   190,
     190,    32,   140,   205,   643,   644,   645,    32,   642,   643,
     279,   638,   110,   635,   173,   666,   265,   506,   187,   422,
     151,   422,   151,   439,   422,   151,   422,   130,   130,    99,
     665,   422,   187,   426,   426,   422,   402,   422,   426,   426,
     666,   214,   426,   121,   476,   121,   426,   121,   476,   426,
     187,   116,   541,   666,   199,   187,   541,   666,   187,   422,
     422,   422,   422,   324,   422,   422,   422,   421,   421,   421,
     157,   599,   459,   422,   146,   422,   666,   666,   446,   422,
     187,   489,    54,   133,   487,   487,   277,   284,   294,   626,
     626,   607,   157,   288,    97,   190,   110,   190,   641,   641,
     645,   110,   190,    32,   639,   650,   636,   637,   190,   394,
     395,   506,   121,   233,   315,   295,   173,   422,   422,   151,
     422,   402,   422,   362,   422,   402,    97,   563,   402,   422,
     666,   426,   666,   422,   666,   426,   402,   121,    96,   186,
     542,   541,   666,   201,   541,   422,   190,   190,   190,   421,
     452,   422,    56,   487,   487,   203,   421,   563,   563,    97,
      32,   273,   110,   110,   459,   562,   666,   121,   233,   666,
     394,   422,   476,    97,   426,    97,   666,     5,   137,   545,
     546,   548,   550,    31,   138,   543,   544,   547,   550,   201,
     541,   201,   202,   472,   187,   452,   121,   487,   187,   563,
     637,   395,   459,   313,   666,   121,   233,   426,   476,   402,
     422,   476,   426,    96,   137,   548,   186,   138,   547,   201,
     116,   666,   422,   313,   666,   121,   402,   422,   426,   426,
     121,   301,   313,   666,   666,   314,   422,   314,   426,   506,
     506,   203,   295,   666,   233,   121,   666,   314,   506
  };

  const short
  jsoniq_parser::yyr1_[] =
  {
       0,   342,   343,   343,   343,   343,   344,   344,   344,   345,
     345,   345,   345,   346,   346,   347,   347,   347,   347,   347,
     347,   347,   347,   348,   348,   348,   348,   349,   350,   350,
     350,   351,   351,   351,   351,   351,   352,   352,   352,   352,
     352,   352,   352,   352,   353,   353,   354,   355,   356,   356,
     357,   357,   358,   358,   359,   359,   359,   359,   360,   360,
     360,   361,   361,   361,   361,   362,   362,   363,   363,   364,
     364,   364,   364,   365,   366,   366,   367,   367,   367,   368,
     368,   368,   368,   368,   368,   368,   368,   369,   369,   370,
     370,   371,   372,   372,   372,   372,   372,   372,   372,   372,
     372,   372,   373,   374,   375,   376,   376,   377,   377,   377,
     378,   379,   379,   379,   380,   380,   380,   380,   381,   381,
     382,   382,   382,   382,   383,   383,   384,   384,   385,   385,
     386,   386,   387,   387,   388,   388,   388,   388,   389,   389,
     390,   390,   391,   391,   391,   391,   392,   392,   392,   392,
     393,   393,   394,   394,   395,   395,   395,   395,   396,   396,
     396,   396,   397,   398,   398,   398,   399,   399,   399,   400,
     400,   401,   401,   401,   402,   402,   402,   402,   402,   402,
     402,   402,   402,   402,   402,   402,   402,   403,   404,   405,
     406,   407,   407,   407,   408,   408,   408,   408,   409,   410,
     411,   412,   413,   413,   414,   415,   416,   417,   418,   418,
     419,   420,   421,   421,   421,   422,   422,   422,   422,   422,
     422,   423,   423,   423,   423,   423,   423,   423,   423,   423,
     423,   423,   423,   424,   425,   426,   426,   427,   427,   428,
     428,   428,   429,   429,   430,   430,   431,   432,   432,   432,
     433,   433,   433,   433,   433,   434,   434,   435,   435,   435,
     435,   436,   436,   437,   437,   437,   438,   438,   438,   438,
     438,   438,   438,   438,   438,   438,   438,   438,   439,   440,
     441,   442,   442,   443,   443,   443,   443,   444,   444,   445,
     445,   445,   446,   446,   446,   447,   447,   447,   448,   449,
     450,   450,   451,   451,   451,   451,   451,   451,   452,   453,
     453,   454,   454,   455,   455,   456,   456,   456,   456,   456,
     456,   456,   457,   457,   458,   458,   459,   460,   460,   461,
     461,   462,   462,   463,   464,   464,   465,   466,   466,   467,
     468,   468,   469,   470,   470,   471,   471,   472,   472,   473,
     473,   474,   474,   475,   475,   476,   476,   477,   478,   478,
     479,   479,   480,   480,   481,   481,   481,   481,   481,   482,
     481,   481,   481,   481,   483,   483,   484,   484,   485,   485,
     486,   486,   487,   487,   487,   488,   488,   488,   488,   488,
     489,   489,   489,   490,   490,   490,   491,   491,   492,   492,
     493,   493,   494,   494,   495,   495,   496,   496,   497,   497,
     497,   497,   498,   498,   498,   499,   499,   500,   500,   500,
     500,   500,   500,   501,   501,   501,   502,   502,   502,   502,
     503,   504,   504,   505,   505,   505,   506,   506,   506,   506,
     507,   508,   508,   508,   509,   509,   510,   510,   510,   510,
     511,   511,   512,   512,   512,   512,   512,   512,   512,   513,
     513,   514,   514,   515,   515,   515,   515,   515,   516,   516,
     517,   517,   518,   518,   518,   518,   519,   519,   519,   519,
     519,   519,   519,   519,   519,   519,   519,   520,   520,   521,
     522,   522,   522,   522,   522,   522,   522,   522,   522,   522,
     522,   522,   522,   522,   523,   523,   524,   524,   524,   525,
     526,   526,   527,   528,   529,   530,   530,   531,   531,   531,
     531,   532,   532,   533,   534,   534,   535,   535,   536,   536,
     536,   537,   537,   537,   537,   537,   537,   538,   538,   539,
     539,   540,   541,   541,   542,   542,   543,   543,   544,   544,
     544,   544,   545,   545,   546,   546,   546,   546,   547,   547,
     548,   548,   549,   549,   549,   549,   550,   550,   550,   550,
     551,   551,   552,   552,   553,   554,   554,   554,   554,   554,
     554,   554,   555,   556,   556,   556,   557,   557,   557,   558,
     559,   560,   560,   561,   561,   562,   563,   563,   563,   563,
     564,   564,   564,   565,   565,   565,   565,   565,   565,   565,
     566,   566,   567,   568,   569,   569,   569,   569,   569,   569,
     569,   569,   569,   569,   570,   571,   572,   572,   572,   573,
     574,   575,   576,   576,   576,   577,   577,   577,   577,   577,
     578,   579,   579,   579,   579,   579,   579,   579,   580,   581,
     582,   583,   584,   584,   584,   584,   585,   586,   586,   587,
     588,   588,   588,   589,   589,   589,   589,   589,   589,   589,
     589,   589,   589,   590,   590,   591,   591,   592,   593,   594,
     594,   595,   596,   597,   597,   598,   599,   600,   600,   601,
     602,   602,   603,   603,   604,   604,   605,   605,   606,   606,
     607,   607,   608,   609,   609,   610,   610,   611,   612,   612,
     612,   613,   613,   614,   615,   615,   616,   617,   617,   618,
     618,   619,   619,   619,   620,   620,   621,   621,   622,   622,
     622,   622,   622,   623,   624,   625,   626,   626,   626,   627,
     627,   628,   628,   628,   628,   628,   628,   628,   628,   629,
     629,   629,   629,   630,   630,   631,   632,   632,   633,   633,
     633,   634,   634,   635,   635,   636,   636,   637,   638,   638,
     639,   639,   640,   640,   640,   641,   641,   642,   642,   643,
     643,   644,   644,   645,   645,   646,   647,   647,   648,   648,
     648,   649,   650,   650,   650,   650,   651,   651,   652,   652,
     653,   654,   654,   655,   655,   656,   656,   657,   658,   658,
     658,   658,   659,   659,   659,   660,   661,   662,   663,   664,
     664,   664,   665,   666,   666,   666,   666,   666,   666,   666,
     666,   666,   666,   666,   666,   666,   666,   666,   666,   666,
     666,   666,   666,   666,   666,   666,   667,   667,   667,   667,
     667,   667,   667,   667,   667,   667,   667,   667,   667,   667,
     667,   667,   667,   667,   667,   667,   667,   667,   667,   667,
     667,   667,   667,   667,   667,   667,   667,   667,   667,   667,
     667,   667,   667,   667,   667,   667,   667,   667,   667,   667,
     667,   667,   667,   667,   667,   667,   667,   667,   667,   667,
     667,   667,   667,   667,   667,   667,   667,   667,   667,   667,
     667,   667,   667,   667,   667,   667,   667,   667,   667,   667,
     667,   667,   667,   667,   667,   667,   667,   667,   667,   667,
     667,   667,   667,   667,   667,   667,   667,   667,   667,   667,
     667,   667,   667,   667,   667,   667,   667,   667,   667,   667,
     667,   667,   667,   667,   667,   667,   667,   667,   667,   667,
     667,   667,   667,   667,   667,   667,   667,   667,   667,   667,
     667,   667,   667,   667,   667,   667,   667,   667,   667,   667,
     667,   667,   667,   667,   667,   667,   667,   667,   667,   667,
     667,   667,   667,   667,   667,   667,   667,   667,   667,   667,
     667,   667,   667,   667,   667,   667,   667,   667,   667,   667,
     667,   667,   667,   667,   667,   667,   667,   667,   667,   667,
     667,   667,   667,   667,   667,   667,   667,   667,   667,   667,
     667,   667,   667,   667,   667,   667,   667,   667,   667,   667,
     667,   667,   667,   667,   667,   667,   667,   667,   667,   667,
     667,   667,   668
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
       3,     1,     1,     3,     3,     4,     1,     1,     1,     1,
       1,     4,     8,     5,     9,     9,    10,     1,     2,     1,
       2,     6,     0,     1,     3,     3,     0,     1,     1,     1,
       2,     2,     0,     1,     1,     1,     2,     2,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     3,
       3,     2,     3,     3,     2,     1,     1,     1,     1,     1,
       1,     1,     4,     3,     3,     7,     3,     3,     7,     4,
       4,     3,     7,     3,     7,     2,     1,     2,     3,     2,
       1,     1,     1,     1,     1,     3,     3,     1,     1,     1,
       1,     3,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     3,     1,     3,     4,     4,     3,
       3,     3,     3,     4,     4,     3,     4,     6,     4,     6,
       4,     3,     4,     6,     6,     4,     6,     6,     4,     1,
       2,     1,     1,     1,     2,     2,     4,     5,     6,     3,
       3,     3,     3,     5,     7,     7,     5,     5,     5,     7,
       7,     5,     5,     3,     3,     5,     7,     5,     7,     1,
       4,     3,     5,     1,     2,     3,     3,     1,     3,     2,
       0,     1,     1,     2,     1,     3,     1,     3,     1,     4,
       1,     2,     3,     0,     1,     0,     1,     4,     2,     3,
       1,     0,     1,     4,     0,     1,     2,     1,     3,     0,
       1,     2,     2,     1,     0,     1,     0,     1,     1,     1,
       1,     1,     1,     1,     3,     3,     1,     1,     1,     2,
       3,     1,     1,     1,     1,     1,     1,     1,     1,     2,
       2,     1,     1,     2,     2,     3,     1,     2,     2,     5,
       2,     1,     1,     0,     2,     1,     3,     4,     0,     2,
       0,     2,     4,     4,     3,     2,     3,     1,     3,     0,
       1,     1,     2,     2,     2,     2,     1,     2,     2,     2,
       2,     3,     2,     3,     3,     4,     2,     2,     1,     1,
       3,     2,     3,     2,     3,     2,     3,     3,     3,     3,
       5,     5,     5,     5,     8,     5,     3,     5,     7,     3,
       3,     3,     1,     1,     1,     1,     1,     1,     1,     1,
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
       1,     1,     1
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
       0,   945,   945,   946,   950,   954,   960,   964,   968,   974,
     980,   988,   994,  1003,  1008,  1014,  1020,  1026,  1034,  1039,
    1049,  1059,  1070,  1082,  1087,  1093,  1099,  1108,  1116,  1123,
    1129,  1140,  1141,  1142,  1143,  1144,  1147,  1148,  1149,  1150,
    1151,  1152,  1153,  1154,  1157,  1162,  1168,  1174,  1180,  1185,
    1191,  1196,  1202,  1207,  1213,  1218,  1223,  1228,  1234,  1236,
    1238,  1246,  1251,  1259,  1267,  1276,  1283,  1291,  1297,  1303,
    1309,  1316,  1324,  1335,  1342,  1349,  1357,  1364,  1370,  1381,
    1382,  1383,  1384,  1385,  1386,  1387,  1388,  1391,  1397,  1404,
    1411,  1419,  1427,  1428,  1429,  1430,  1431,  1432,  1433,  1434,
    1435,  1436,  1439,  1453,  1459,  1465,  1472,  1478,  1485,  1490,
    1496,  1502,  1514,  1526,  1539,  1544,  1552,  1560,  1569,  1574,
    1581,  1586,  1593,  1598,  1606,  1611,  1618,  1624,  1633,  1634,
    1637,  1651,  1664,  1679,  1692,  1697,  1702,  1707,  1713,  1720,
    1728,  1733,  1741,  1749,  1757,  1765,  1775,  1779,  1785,  1789,
    1796,  1805,  1816,  1822,  1829,  1834,  1841,  1848,  1857,  1867,
    1877,  1887,  1902,  1918,  1922,  1927,  1932,  1936,  1941,  1946,
    1951,  1959,  1966,  1973,  1986,  1987,  1988,  1989,  1990,  1991,
    1992,  1993,  1994,  1995,  1996,  1997,  1998,  2001,  2007,  2033,
    2039,  2045,  2052,  2059,  2068,  2077,  2086,  2095,  2106,  2112,
    2118,  2124,  2136,  2141,  2147,  2159,  2172,  2192,  2198,  2205,
    2214,  2221,  2227,  2232,  2244,  2256,  2257,  2258,  2259,  2260,
    2261,  2264,  2265,  2266,  2267,  2268,  2269,  2270,  2271,  2272,
    2273,  2274,  2275,  2278,  2290,  2296,  2297,  2302,  2307,  2313,
    2318,  2323,  2329,  2338,  2348,  2358,  2369,  2375,  2376,  2377,
    2380,  2381,  2382,  2383,  2384,  2387,  2394,  2402,  2406,  2414,
    2422,  2430,  2431,  2436,  2443,  2450,  2461,  2471,  2481,  2491,
    2501,  2511,  2521,  2531,  2541,  2550,  2560,  2570,  2582,  2588,
    2594,  2600,  2606,  2614,  2622,  2630,  2638,  2648,  2654,  2663,
    2664,  2668,  2675,  2679,  2684,  2687,  2691,  2695,  2701,  2707,
    2713,  2719,  2727,  2731,  2739,  2747,  2755,  2763,  2773,  2779,
    2785,  2793,  2799,  2807,  2811,  2819,  2825,  2831,  2837,  2846,
    2855,  2864,  2875,  2879,  2885,  2891,  2899,  2905,  2914,  2925,
    2931,  2939,  2943,  2952,  2958,  2964,  2972,  2978,  2984,  2992,
    2998,  3004,  3012,  3018,  3025,  3035,  3042,  3052,  3058,  3066,
    3072,  3081,  3087,  3095,  3101,  3110,  3116,  3124,  3130,  3134,
    3140,  3145,  3151,  3155,  3166,  3170,  3179,  3185,  3194,  3204,
    3203,  3216,  3225,  3234,  3245,  3249,  3260,  3264,  3270,  3273,
    3279,  3283,  3289,  3293,  3297,  3303,  3307,  3313,  3319,  3325,
    3333,  3337,  3341,  3347,  3351,  3357,  3365,  3369,  3377,  3381,
    3389,  3393,  3401,  3405,  3413,  3417,  3423,  3427,  3433,  3437,
    3441,  3445,  3453,  3454,  3455,  3458,  3463,  3469,  3473,  3477,
    3481,  3485,  3489,  3495,  3499,  3503,  3509,  3513,  3517,  3521,
    3530,  3538,  3544,  3552,  3556,  3560,  3566,  3570,  3578,  3586,
    3624,  3630,  3640,  3644,  3650,  3651,  3654,  3660,  3668,  3674,
    3684,  3690,  3698,  3702,  3706,  3710,  3714,  3720,  3726,  3732,
    3736,  3742,  3746,  3755,  3759,  3763,  3769,  3773,  3781,  3782,
    3785,  3789,  3795,  3799,  3803,  3807,  3813,  3817,  3821,  3825,
    3829,  3833,  3839,  3845,  3851,  3855,  3859,  3865,  3871,  3879,
    3885,  3886,  3887,  3888,  3889,  3890,  3891,  3892,  3893,  3894,
    3895,  3896,  3897,  3898,  3901,  3905,  3911,  3918,  3925,  3934,
    3940,  3944,  3950,  3957,  3963,  3969,  3973,  3991,  3997,  4003,
    4009,  4017,  4021,  4027,  4033,  4042,  4054,  4055,  4058,  4059,
    4060,  4063,  4071,  4090,  4098,  4117,  4138,  4161,  4167,  4176,
    4182,  4191,  4198,  4199,  4201,  4206,  4213,  4216,  4222,  4228,
    4234,  4241,  4250,  4253,  4259,  4265,  4271,  4278,  4287,  4291,
    4297,  4301,  4307,  4311,  4315,  4320,  4327,  4331,  4335,  4339,
    4345,  4350,  4356,  4362,  4369,  4375,  4380,  4385,  4390,  4395,
    4400,  4405,  4408,  4414,  4418,  4422,  4428,  4432,  4436,  4442,
    4448,  4454,  4458,  4464,  4468,  4474,  4480,  4484,  4488,  4492,
    4499,  4505,  4511,  4519,  4558,  4562,  4566,  4570,  4574,  4578,
    4584,  4590,  4598,  4604,  4610,  4614,  4618,  4622,  4626,  4630,
    4634,  4638,  4642,  4646,  4652,  4658,  4664,  4668,  4672,  4678,
    4683,  4689,  4695,  4699,  4704,  4710,  4714,  4720,  4726,  4730,
    4738,  4744,  4748,  4754,  4763,  4772,  4778,  4784,  4792,  4798,
    4804,  4810,  4816,  4820,  4824,  4830,  4838,  4844,  4848,  4856,
    4862,  4868,  4874,  4882,  4886,  4892,  4898,  4902,  4908,  4912,
    4918,  4924,  4931,  4939,  4944,  4950,  4956,  4964,  4970,  4977,
    4983,  4992,  4998,  5004,  5010,  5019,  5026,  5032,  5038,  5047,
    5054,  5058,  5064,  5069,  5076,  5080,  5086,  5090,  5096,  5100,
    5106,  5110,  5118,  5129,  5132,  5138,  5141,  5147,  5153,  5161,
    5165,  5171,  5174,  5180,  5190,  5193,  5199,  5209,  5215,  5223,
    5226,  5232,  5236,  5240,  5246,  5249,  5255,  5258,  5264,  5268,
    5272,  5276,  5280,  5286,  5292,  5302,  5312,  5316,  5320,  5326,
    5332,  5340,  5344,  5348,  5352,  5356,  5360,  5364,  5368,  5374,
    5378,  5382,  5386,  5392,  5398,  5406,  5414,  5418,  5424,  5434,
    5445,  5451,  5455,  5461,  5464,  5470,  5475,  5482,  5490,  5493,
    5499,  5503,  5509,  5518,  5525,  5533,  5537,  5543,  5549,  5557,
    5560,  5566,  5571,  5581,  5589,  5599,  5605,  5609,  5615,  5619,
    5623,  5629,  5635,  5639,  5643,  5647,  5653,  5661,  5671,  5675,
    5681,  5687,  5691,  5697,  5701,  5707,  5711,  5717,  5724,  5734,
    5740,  5747,  5758,  5764,  5774,  5780,  5786,  5804,  5829,  5849,
    5853,  5857,  5863,  5866,  5867,  5868,  5869,  5870,  5871,  5872,
    5873,  5874,  5875,  5876,  5877,  5878,  5879,  5880,  5881,  5882,
    5883,  5884,  5885,  5886,  5887,  5888,  5891,  5892,  5893,  5894,
    5895,  5896,  5897,  5898,  5899,  5900,  5901,  5902,  5903,  5904,
    5905,  5906,  5907,  5908,  5909,  5910,  5911,  5912,  5913,  5914,
    5915,  5916,  5917,  5918,  5919,  5920,  5921,  5922,  5923,  5924,
    5925,  5926,  5927,  5928,  5929,  5930,  5931,  5932,  5933,  5934,
    5935,  5936,  5937,  5938,  5939,  5940,  5941,  5942,  5943,  5944,
    5945,  5946,  5947,  5948,  5949,  5950,  5951,  5952,  5953,  5954,
    5955,  5956,  5957,  5958,  5959,  5960,  5961,  5962,  5963,  5964,
    5965,  5966,  5967,  5968,  5969,  5970,  5971,  5972,  5973,  5974,
    5975,  5976,  5977,  5978,  5979,  5980,  5981,  5982,  5983,  5984,
    5985,  5986,  5987,  5988,  5989,  5990,  5991,  5992,  5993,  5994,
    5995,  5996,  5997,  5998,  5999,  6000,  6001,  6002,  6003,  6004,
    6005,  6006,  6007,  6008,  6009,  6010,  6011,  6012,  6013,  6014,
    6015,  6016,  6017,  6018,  6019,  6020,  6021,  6022,  6023,  6024,
    6025,  6026,  6027,  6028,  6029,  6030,  6031,  6032,  6033,  6034,
    6035,  6036,  6037,  6038,  6039,  6040,  6041,  6042,  6043,  6044,
    6045,  6046,  6047,  6048,  6049,  6050,  6051,  6052,  6053,  6054,
    6055,  6056,  6057,  6058,  6059,  6060,  6061,  6062,  6063,  6064,
    6065,  6066,  6067,  6068,  6069,  6070,  6071,  6072,  6073,  6074,
    6075,  6076,  6077,  6078,  6079,  6080,  6081,  6082,  6083,  6084,
    6085,  6086,  6087,  6088,  6089,  6090,  6091,  6092,  6093,  6094,
    6095,  6096,  6099
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
     335,   336,   337,   338,   339,   340,   341
    };
    // Last valid token kind.
    const int code_max = 596;

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
