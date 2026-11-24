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

  case 189: // BlockExpr: "'{'" StatementsAndOptionalExpr "'}'"
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

  case 237: // ReturnOrSelect: "'select'"
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

  case 260: // ForClause: "'from'" error VarInDeclList
    {
      (yylhs.value.node) = (yystack_[0].value.node);
      error(yystack_[1].location, "syntax error, unexpected qualified name \""
          + static_cast<VarInDeclList*>((yystack_[0].value.node))->operator[](0)->get_var_name()->get_qname().str() + "\" (missing \"$\" sign?)");
      delete (yystack_[0].value.node);
      YYERROR;
    }
    break;

  case 261: // ForClause: ForOrFrom "'unrecognized'"
    {
      (yylhs.value.node) = NULL;
      error(yystack_[0].location, "");
      YYERROR;
    }
    break;

  case 263: // ForOrFrom: "'from'"
    {
    }
    break;

  case 264: // VarInDeclList: VarInDecl
    {
      VarInDeclList* vdl = new VarInDeclList( LOC(yylhs.location) );
      vdl->push_back( dynamic_cast<VarInDecl*>((yystack_[0].value.node)) );
      (yylhs.value.node) = vdl;
    }
    break;

  case 265: // VarInDeclList: VarInDeclList "','" "'$'" VarInDecl
    {
      if ( VarInDeclList* vdl = dynamic_cast<VarInDeclList*>((yystack_[3].value.node)) )
        vdl->push_back( dynamic_cast<VarInDecl*>((yystack_[0].value.node)) );
      (yylhs.value.node) = (yystack_[3].value.node);
    }
    break;

  case 266: // VarInDeclList: VarInDeclList "','" VarInDecl
    {
      (yylhs.value.node) = (yystack_[2].value.node);
      error(yystack_[0].location, "syntax error, unexpected QName \""
          + static_cast<VarInDecl*>((yystack_[0].value.node))->get_var_name()->get_qname().str() + "\" (missing \"$\" sign?)");
      delete (yystack_[2].value.node);
      release_hack( (yystack_[0].value.node) );
      YYERROR;
    }
    break;

  case 267: // VarInDecl: QNAME "'in'" ExprSingle
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

  case 268: // VarInDecl: QNAME "'allowing'" "'empty'" "'in'" ExprSingle
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

  case 269: // VarInDecl: QNAME TypeDeclaration "'in'" ExprSingle
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

  case 270: // VarInDecl: QNAME TypeDeclaration "'allowing'" "'empty'" "'in'" ExprSingle
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

  case 271: // VarInDecl: QNAME PositionalVar "'in'" ExprSingle
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

  case 272: // VarInDecl: QNAME "'allowing'" "'empty'" PositionalVar "'in'" ExprSingle
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

  case 273: // VarInDecl: QNAME TypeDeclaration PositionalVar "'in'" ExprSingle
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

  case 274: // VarInDecl: QNAME TypeDeclaration "'allowing'" "'empty'" PositionalVar "'in'" ExprSingle
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

  case 275: // VarInDecl: QNAME FTScoreVar "'in'" ExprSingle
    {
      (yylhs.value.node) = new VarInDecl(LOC(yylhs.location),
                         static_cast<QName*>((yystack_[3].value.expr)),
                         NULL,NULL,
                         dynamic_cast<FTScoreVar*>((yystack_[2].value.node)),
                         (yystack_[0].value.expr),
                         false);
    }
    break;

  case 276: // VarInDecl: QNAME TypeDeclaration FTScoreVar "'in'" ExprSingle
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

  case 277: // VarInDecl: QNAME PositionalVar FTScoreVar "'in'" ExprSingle
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

  case 278: // VarInDecl: QNAME TypeDeclaration PositionalVar FTScoreVar "'in'" ExprSingle
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

  case 279: // PositionalVar: "'at'" "'$'" QNAME
    {
      (yylhs.value.node) = new PositionalVar(LOC(yylhs.location), static_cast<QName*>((yystack_[0].value.expr)));
    }
    break;

  case 280: // FTScoreVar: "'score'" "'$'" QNAME
    {
      (yylhs.value.node) = new FTScoreVar(LOC(yylhs.location), static_cast<QName*>((yystack_[0].value.expr)));
    }
    break;

  case 281: // LetClause: "'let'" VarGetsDeclList
        {
            (yylhs.value.node) = new LetClause( LOC(yylhs.location), dynamic_cast<VarGetsDeclList*>((yystack_[0].value.node)) );
        }
    break;

  case 282: // VarGetsDeclList: VarGetsDecl
        {
            VarGetsDeclList *vgdl = new VarGetsDeclList( LOC(yylhs.location) );
            vgdl->push_back( dynamic_cast<VarGetsDecl*>((yystack_[0].value.node)) );
            (yylhs.value.node) = vgdl;
        }
    break;

  case 283: // VarGetsDeclList: VarGetsDeclList "','" VarGetsDecl
        {
            if( VarGetsDeclList *vgdl = dynamic_cast<VarGetsDeclList*>((yystack_[2].value.node)) )
                vgdl->push_back( dynamic_cast<VarGetsDecl*>((yystack_[0].value.node)) );
            (yylhs.value.node) = (yystack_[2].value.node);
        }
    break;

  case 284: // VarGetsDecl: "'$'" QNAME "':='" ExprSingle
    {
      (yylhs.value.node) = new VarGetsDecl(LOC (yylhs.location),
                           static_cast<QName*>((yystack_[2].value.expr)),
                           NULL,
                           NULL,
                           (yystack_[0].value.expr));
    }
    break;

  case 285: // VarGetsDecl: "'$'" QNAME TypeDeclaration "':='" ExprSingle
    {
      (yylhs.value.node) = new VarGetsDecl(LOC (yylhs.location),
                           static_cast<QName*>((yystack_[3].value.expr)),
                           dynamic_cast<SequenceTypeAST *>((yystack_[2].value.node)),
                           NULL,
                           (yystack_[0].value.expr));
    }
    break;

  case 286: // VarGetsDecl: FTScoreVar "':='" ExprSingle
    {
      (yylhs.value.node) = new VarGetsDecl(LOC (yylhs.location),
                           dynamic_cast<FTScoreVar*>((yystack_[2].value.node))->get_var_name(),
                           NULL,
                           dynamic_cast<FTScoreVar*>((yystack_[2].value.node)),
                           (yystack_[0].value.expr));
     }
    break;

  case 287: // VarGetsDecl: "'$'" QNAME TypeDeclaration FTScoreVar "':='" ExprSingle
    {
      (yylhs.value.node) = new VarGetsDecl(LOC (yylhs.location),
                           static_cast<QName*>((yystack_[4].value.expr)),
                           dynamic_cast<SequenceTypeAST *>((yystack_[3].value.node)),
                           dynamic_cast<FTScoreVar*>((yystack_[2].value.node)),
                           (yystack_[0].value.expr));
    }
    break;

  case 288: // WindowVarDecl: "'$'" QNAME "'in'" ExprSingle
    {
      (yylhs.value.node) = new WindowVarDecl(LOC (yylhs.location),
                             static_cast<QName*>((yystack_[2].value.expr)),
                             NULL, (yystack_[0].value.expr));
    }
    break;

  case 289: // WindowVarDecl: "'$'" QNAME TypeDeclaration "'in'" ExprSingle
    {
      (yylhs.value.node) = new WindowVarDecl(LOC (yylhs.location),
                             static_cast<QName*>((yystack_[3].value.expr)),
                             dynamic_cast<SequenceTypeAST *>((yystack_[2].value.node)),
                             (yystack_[0].value.expr));
    }
    break;

  case 290: // WindowVars: WindowVars3
    { (yylhs.value.node) = (yystack_[0].value.node); }
    break;

  case 291: // WindowVars: "'$'" QNAME
    {
      (yylhs.value.node) = new WindowVars(LOC(yylhs.location), NULL, static_cast<QName*>((yystack_[0].value.expr)), NULL, NULL);
    }
    break;

  case 292: // WindowVars: "'$'" QNAME WindowVars3
    {
      (yylhs.value.node) = (yystack_[0].value.node);
      dynamic_cast<WindowVars *>((yylhs.value.node))->set_curr(static_cast<QName*>((yystack_[1].value.expr)));
    }
    break;

  case 293: // WindowVars3: PositionalVar
    {
      (yylhs.value.node) = new WindowVars(LOC(yylhs.location), dynamic_cast<PositionalVar*>((yystack_[0].value.node)), NULL, NULL, NULL);
    }
    break;

  case 294: // WindowVars3: PositionalVar WindowVars2
    {
      (yylhs.value.node) = (yystack_[0].value.node);
      dynamic_cast<WindowVars *>((yylhs.value.node))->set_posvar(dynamic_cast<PositionalVar*>((yystack_[1].value.node)));
    }
    break;

  case 295: // WindowVars3: WindowVars2
    { (yylhs.value.node) = (yystack_[0].value.node); }
    break;

  case 296: // WindowVars2: "'previous'" "'$'" QNAME "'next'" "'$'" QNAME
    {
      (yylhs.value.node) = new WindowVars(LOC(yylhs.location), NULL, NULL, static_cast<QName*>((yystack_[3].value.expr)), static_cast<QName*>((yystack_[0].value.expr)));
    }
    break;

  case 297: // WindowVars2: "'next'" "'$'" QNAME
    {
      (yylhs.value.node) = new WindowVars(LOC(yylhs.location), NULL, NULL, NULL, static_cast<QName*>((yystack_[0].value.expr)));
    }
    break;

  case 298: // WindowVars2: "'previous'" "'$'" QNAME
    {
      (yylhs.value.node) = new WindowVars(LOC(yylhs.location), NULL, NULL, static_cast<QName*>((yystack_[0].value.expr)), NULL);
    }
    break;

  case 299: // WhereClause: "'where'" ExprSingle
    {
      (yylhs.value.node) = new WhereClause(LOC (yylhs.location), (yystack_[0].value.expr));
    }
    break;

  case 300: // GroupByClause: "'group'" "'by'" GroupSpecList
    {
      (yylhs.value.node) = new GroupByClause(LOC(yylhs.location), dynamic_cast<GroupSpecList*>((yystack_[0].value.node)));
    }
    break;

  case 301: // GroupSpecList: GroupSpec
    {
      GroupSpecList* gsl = new GroupSpecList(LOC(yylhs.location));
      gsl->push_back(static_cast<GroupSpec*>((yystack_[0].value.node)));
      (yylhs.value.node) = gsl;
    }
    break;

  case 302: // GroupSpecList: GroupSpecList "','" GroupSpec
    {
      GroupSpecList* gsl = static_cast<GroupSpecList*>((yystack_[2].value.node));
      gsl->push_back(static_cast<GroupSpec*>((yystack_[0].value.node)));
      (yylhs.value.node) = gsl;
    }
    break;

  case 303: // GroupSpec: "'$'" QNAME "':='" ExprSingle
    {
      (yylhs.value.node) = new GroupSpec(LOC(yylhs.location), static_cast<QName*>((yystack_[2].value.expr)), NULL, (yystack_[0].value.expr), NULL);
    }
    break;

  case 304: // GroupSpec: "'$'" QNAME TypeDeclaration "':='" ExprSingle
    {
      (yylhs.value.node) = new GroupSpec(LOC(yylhs.location),
                         static_cast<QName*>((yystack_[3].value.expr)),
                         static_cast<SequenceTypeAST*>((yystack_[2].value.node)),
                         (yystack_[0].value.expr),
                         NULL);
    }
    break;

  case 305: // GroupSpec: "'$'" QNAME TypeDeclaration "':='" ExprSingle GroupCollationSpec
    {
      (yylhs.value.node) = new GroupSpec(LOC(yylhs.location),
                         static_cast<QName*>((yystack_[4].value.expr)),
                         static_cast<SequenceTypeAST*>((yystack_[3].value.node)),
                         (yystack_[1].value.expr),
                         static_cast<GroupCollationSpec*>((yystack_[0].value.node)));
    }
    break;

  case 306: // GroupSpec: "'$'" QNAME "':='" ExprSingle GroupCollationSpec
    {
      (yylhs.value.node) = new GroupSpec(LOC(yylhs.location),
                         static_cast<QName*>((yystack_[3].value.expr)),
                         NULL,
                         (yystack_[1].value.expr),
                         static_cast<GroupCollationSpec*>((yystack_[0].value.node)));
    }
    break;

  case 307: // GroupSpec: ExprSingle
    {
      VarRef* varRef = dynamic_cast<VarRef*>((yystack_[0].value.expr));
      if (varRef != NULL)
        (yylhs.value.node) = new GroupSpec(LOC(yylhs.location), varRef, NULL, NULL, NULL);
      else
        (yylhs.value.node) = new GroupSpec(LOC(yylhs.location), NULL, NULL, (yystack_[0].value.expr), NULL);
    }
    break;

  case 308: // GroupSpec: ExprSingle GroupCollationSpec
    {
      VarRef* varRef = dynamic_cast<VarRef*>((yystack_[1].value.expr));
      if (varRef != NULL)
        (yylhs.value.node) = new GroupSpec(LOC(yylhs.location), varRef, NULL, NULL, static_cast<GroupCollationSpec*>((yystack_[0].value.node)));
      else
        (yylhs.value.node) = new GroupSpec(LOC(yylhs.location), NULL, NULL, (yystack_[1].value.expr), static_cast<GroupCollationSpec*>((yystack_[0].value.node)));
    }
    break;

  case 309: // GroupCollationSpec: "'collation'" URI_LITERAL
    {
      (yylhs.value.node) = new GroupCollationSpec( LOC(yylhs.location), SYMTAB((yystack_[0].value.sval)) );
    }
    break;

  case 310: // OrderByClause: "'order'" "'by'" OrderSpecList
        {
            (yylhs.value.node) = new OrderByClause(
                LOC(yylhs.location), dynamic_cast<OrderSpecList*>((yystack_[0].value.node))
            );
        }
    break;

  case 311: // OrderByClause: "'stable'" "'order'" "'by'" OrderSpecList
        {
            (yylhs.value.node) = new OrderByClause(
                LOC(yylhs.location), dynamic_cast<OrderSpecList*>((yystack_[0].value.node)), true
            );
        }
    break;

  case 312: // OrderSpecList: OrderSpec
        {
            OrderSpecList *osl = new OrderSpecList( LOC(yylhs.location) );
            osl->push_back( dynamic_cast<OrderSpec*>((yystack_[0].value.node)) );
            (yylhs.value.node) = osl;
        }
    break;

  case 313: // OrderSpecList: OrderSpecList "','" OrderSpec
        {
            if ( OrderSpecList* osl = dynamic_cast<OrderSpecList*>((yystack_[2].value.node)) )
                osl->push_back( dynamic_cast<OrderSpec*>((yystack_[0].value.node)) );
            (yylhs.value.node) = (yystack_[2].value.node);
        }
    break;

  case 314: // OrderSpec: ExprSingle
        {
            (yylhs.value.node) = new OrderSpec( LOC(yylhs.location), (yystack_[0].value.expr), NULL );
        }
    break;

  case 315: // OrderSpec: ExprSingle OrderModifier
        {
            (yylhs.value.node) = new OrderSpec(
                LOC(yylhs.location), (yystack_[1].value.expr), dynamic_cast<OrderModifierPN*>((yystack_[0].value.node))
            );
        }
    break;

  case 316: // OrderModifier: OrderDirSpec
        {
            (yylhs.value.node) = new OrderModifierPN(
                LOC(yylhs.location), dynamic_cast<OrderDirSpec*>((yystack_[0].value.node)), NULL, NULL
            );
        }
    break;

  case 317: // OrderModifier: OrderEmptySpec
        {
            (yylhs.value.node) = new OrderModifierPN(
                LOC(yylhs.location), NULL, dynamic_cast<OrderEmptySpec*>((yystack_[0].value.node)), NULL
            );
        }
    break;

  case 318: // OrderModifier: OrderCollationSpec
        {
            (yylhs.value.node) = new OrderModifierPN(
                LOC(yylhs.location), NULL, NULL, dynamic_cast<OrderCollationSpec*>((yystack_[0].value.node))
            );
        }
    break;

  case 319: // OrderModifier: OrderDirSpec OrderEmptySpec
        {
            (yylhs.value.node) = new OrderModifierPN(
                LOC(yylhs.location),
                dynamic_cast<OrderDirSpec*>((yystack_[1].value.node)),
                dynamic_cast<OrderEmptySpec*>((yystack_[0].value.node)),
                NULL
            );
        }
    break;

  case 320: // OrderModifier: OrderDirSpec OrderCollationSpec
        {
            (yylhs.value.node) = new OrderModifierPN(
                LOC(yylhs.location),
                dynamic_cast<OrderDirSpec*>((yystack_[1].value.node)),
                NULL,
                dynamic_cast<OrderCollationSpec*>((yystack_[0].value.node))
            );
        }
    break;

  case 321: // OrderModifier: OrderEmptySpec OrderCollationSpec
        {
            (yylhs.value.node) = new OrderModifierPN(
                LOC(yylhs.location),
                NULL,
                dynamic_cast<OrderEmptySpec*>((yystack_[1].value.node)),
                dynamic_cast<OrderCollationSpec*>((yystack_[0].value.node))
            );
        }
    break;

  case 322: // OrderModifier: OrderDirSpec OrderEmptySpec OrderCollationSpec
        {
            (yylhs.value.node) = new OrderModifierPN(
                LOC(yylhs.location),
                dynamic_cast<OrderDirSpec*>((yystack_[2].value.node)),
                dynamic_cast<OrderEmptySpec*>((yystack_[1].value.node)),
                dynamic_cast<OrderCollationSpec*>((yystack_[0].value.node))
            );
        }
    break;

  case 323: // OrderDirSpec: "'ascending'"
        {
            (yylhs.value.node) = new OrderDirSpec( LOC(yylhs.location), ParseConstants::dir_ascending );
        }
    break;

  case 324: // OrderDirSpec: "'descending'"
        {
            (yylhs.value.node) = new OrderDirSpec( LOC(yylhs.location), ParseConstants::dir_descending );
        }
    break;

  case 325: // OrderEmptySpec: "'empty'" "'greatest'"
        {
            (yylhs.value.node) = new OrderEmptySpec(
                LOC(yylhs.location), StaticContextConsts::empty_greatest
            );
        }
    break;

  case 326: // OrderEmptySpec: "'empty'" "'least'"
        {
            (yylhs.value.node) = new OrderEmptySpec(
                LOC(yylhs.location), StaticContextConsts::empty_least
            );
        }
    break;

  case 327: // OrderCollationSpec: "'collation'" URI_LITERAL
        {
            (yylhs.value.node) = new OrderCollationSpec( LOC(yylhs.location), SYMTAB((yystack_[0].value.sval)) );
        }
    break;

  case 328: // QuantifiedExpr: "'some'" "'$'" QVarInDeclList "'satisfies'" ExprSingle
        {
            (yylhs.value.expr) = new QuantifiedExpr(
                LOC(yylhs.location),
                ParseConstants::quant_some,
                dynamic_cast<QVarInDeclList*>((yystack_[2].value.node)),
                (yystack_[0].value.expr)
            );
        }
    break;

  case 329: // QuantifiedExpr: "'every'" "'$'" QVarInDeclList "'satisfies'" ExprSingle
        {
            (yylhs.value.expr) = new QuantifiedExpr(
                LOC(yylhs.location),
                ParseConstants::quant_every,
                dynamic_cast<QVarInDeclList*>((yystack_[2].value.node)),
                (yystack_[0].value.expr)
            );
        }
    break;

  case 330: // QVarInDeclList: QVarInDecl
    {
      QVarInDeclList *qvidl = new QVarInDeclList( LOC(yylhs.location) );
      qvidl->push_back( dynamic_cast<QVarInDecl*>((yystack_[0].value.node)) );
      (yylhs.value.node) = qvidl;
    }
    break;

  case 331: // QVarInDeclList: QVarInDeclList "','" "'$'" QVarInDecl
    {
      QVarInDeclList *qvidl = dynamic_cast<QVarInDeclList*>((yystack_[3].value.node));
      qvidl->push_back( dynamic_cast<QVarInDecl*>((yystack_[0].value.node)) );
      (yylhs.value.node) = (yystack_[3].value.node);
    }
    break;

  case 332: // QVarInDecl: QNAME "'in'" ExprSingle
    {
      (yylhs.value.node) = new QVarInDecl(LOC(yylhs.location), static_cast<QName*>((yystack_[2].value.expr)), (yystack_[0].value.expr));
    }
    break;

  case 333: // QVarInDecl: QNAME TypeDeclaration "'in'" ExprSingle
    {
      (yylhs.value.node) = new QVarInDecl(LOC(yylhs.location),
                          static_cast<QName*>((yystack_[3].value.expr)),
                          dynamic_cast<SequenceTypeAST *>((yystack_[2].value.node)),
                          (yystack_[0].value.expr));
    }
    break;

  case 334: // SwitchExpr: "'switch'" "'('" Expr "')'" SwitchCaseClauseList "'default'" ReturnOrSelect ExprSingle
    {
      (yylhs.value.expr) = new SwitchExpr(LOC(yylhs.location), (yystack_[5].value.expr), static_cast<SwitchCaseClauseList*>((yystack_[3].value.node)), (yystack_[0].value.expr));
    }
    break;

  case 335: // SwitchCaseClauseList: SwitchCaseClause
    {
      SwitchCaseClauseList* scc_list_p = new SwitchCaseClauseList(LOC(yylhs.location));
      scc_list_p->push_back(static_cast<SwitchCaseClause*>((yystack_[0].value.node)));
      (yylhs.value.node) = scc_list_p;
    }
    break;

  case 336: // SwitchCaseClauseList: SwitchCaseClauseList SwitchCaseClause
    {
      SwitchCaseClauseList* scc_list_p = static_cast<SwitchCaseClauseList*>((yystack_[1].value.node));
      scc_list_p->push_back(static_cast<SwitchCaseClause*>((yystack_[0].value.node)));
      (yylhs.value.node) = (yystack_[1].value.node);
    }
    break;

  case 337: // SwitchCaseClause: SwitchCaseOperandList ReturnOrSelect ExprSingle
    {
      (yylhs.value.node) = new SwitchCaseClause(LOC(yylhs.location), dynamic_cast<SwitchCaseOperandList*>((yystack_[2].value.node)), (yystack_[0].value.expr));
    }
    break;

  case 338: // SwitchCaseOperandList: "'case'" ExprSingle
    {
      SwitchCaseOperandList* sco_list_p = new SwitchCaseOperandList(LOC(yylhs.location));
      sco_list_p->push_back((yystack_[0].value.expr));
      (yylhs.value.node) = sco_list_p;
    }
    break;

  case 339: // SwitchCaseOperandList: SwitchCaseOperandList "'case'" ExprSingle
    {
      SwitchCaseOperandList* sco_list_p = static_cast<SwitchCaseOperandList*>((yystack_[2].value.node));
      sco_list_p->push_back((yystack_[0].value.expr));
      (yylhs.value.node) = (yystack_[2].value.node);
    }
    break;

  case 340: // SwitchStatement: "'switch'" "'('" Expr "')'" SwitchCaseStatementList "'default'" ReturnOrSelect Statement
    {
      (yylhs.value.expr) = new SwitchExpr(LOC(yylhs.location), (yystack_[5].value.expr), static_cast<SwitchCaseClauseList*>((yystack_[3].value.node)), (yystack_[0].value.expr));
    }
    break;

  case 341: // SwitchCaseStatementList: SwitchCaseStatement
    {
      SwitchCaseClauseList* scc_list_p = new SwitchCaseClauseList(LOC(yylhs.location));
      scc_list_p->push_back(static_cast<SwitchCaseClause*>((yystack_[0].value.node)));
      (yylhs.value.node) = scc_list_p;
    }
    break;

  case 342: // SwitchCaseStatementList: SwitchCaseStatementList SwitchCaseStatement
    {
      SwitchCaseClauseList* scc_list_p = static_cast<SwitchCaseClauseList*>((yystack_[1].value.node));
      scc_list_p->push_back(static_cast<SwitchCaseClause*>((yystack_[0].value.node)));
      (yylhs.value.node) = (yystack_[1].value.node);
    }
    break;

  case 343: // SwitchCaseStatement: SwitchCaseOperandList ReturnOrSelect Statement
    {
      (yylhs.value.node) = new SwitchCaseClause(LOC(yylhs.location), dynamic_cast<SwitchCaseOperandList*>((yystack_[2].value.node)), (yystack_[0].value.expr));
    }
    break;

  case 344: // TypeswitchExpr: "'typeswitch'" "'('" Expr "')'" CaseClauseList "'default'" ReturnOrSelect ExprSingle
    {
      (yylhs.value.expr) = new TypeswitchExpr(LOC(yylhs.location),
                              (yystack_[5].value.expr),
                              static_cast<CaseClauseList*>((yystack_[3].value.node)),
                              (yystack_[0].value.expr));
    }
    break;

  case 345: // TypeswitchExpr: "'typeswitch'" "'('" Expr "')'" CaseClauseList "'default'" "'$'" QNAME ReturnOrSelect ExprSingle
    {
      (yylhs.value.expr) = new TypeswitchExpr(LOC (yylhs.location),
                              (yystack_[7].value.expr),
                              static_cast<CaseClauseList*>((yystack_[5].value.node)),
                              static_cast<QName*>((yystack_[2].value.expr)),
                              (yystack_[0].value.expr));
    }
    break;

  case 346: // TypeswitchStatement: "'typeswitch'" "'('" Expr "')'" CaseStatementList "'default'" ReturnOrSelect Statement
    {
      (yylhs.value.expr) = new TypeswitchExpr(LOC(yylhs.location),
                              (yystack_[5].value.expr),
                              static_cast<CaseClauseList*>((yystack_[3].value.node)),
                              (yystack_[0].value.expr));
    }
    break;

  case 347: // TypeswitchStatement: "'typeswitch'" "'('" Expr "')'" CaseClauseList "'default'" "'$'" QNAME ReturnOrSelect Statement
    {
      (yylhs.value.expr) = new TypeswitchExpr(LOC (yylhs.location),
                              (yystack_[7].value.expr),
                              static_cast<CaseClauseList*>((yystack_[5].value.node)),
                              static_cast<QName*>((yystack_[2].value.expr)),
                              (yystack_[0].value.expr));
    }
    break;

  case 348: // CaseClauseList: CaseClause
    {
      CaseClauseList* cc_list_p = new CaseClauseList(LOC (yylhs.location));
      cc_list_p->push_back(static_cast<CaseClause*>((yystack_[0].value.node)));
      (yylhs.value.node) = cc_list_p;
    }
    break;

  case 349: // CaseClauseList: CaseClauseList CaseClause
    {
      CaseClauseList* cc_list_p = dynamic_cast<CaseClauseList*>((yystack_[1].value.node));
      cc_list_p->push_back(static_cast<CaseClause*>((yystack_[0].value.node)));
      (yylhs.value.node) = (yystack_[1].value.node);
    }
    break;

  case 350: // CaseClause: "'case'" SequenceTypeList ReturnOrSelect ExprSingle
    {
      (yylhs.value.node) = new CaseClause(LOC (yylhs.location),
                          static_cast<SequenceTypeList*>((yystack_[2].value.node)),
                          (yystack_[0].value.expr));
    }
    break;

  case 351: // CaseClause: "'case'" "'$'" QNAME "'as'" SequenceTypeList ReturnOrSelect ExprSingle
    {
      (yylhs.value.node) = new CaseClause(LOC (yylhs.location),
                          static_cast<QName*>((yystack_[4].value.expr)),
                          static_cast<SequenceTypeList*>((yystack_[2].value.node)),
                          (yystack_[0].value.expr));
     }
    break;

  case 352: // CaseStatementList: CaseStatement
    {
      CaseClauseList* cc_list_p = new CaseClauseList(LOC (yylhs.location));
      cc_list_p->push_back(static_cast<CaseClause*>((yystack_[0].value.node)));
      (yylhs.value.node) = cc_list_p;
    }
    break;

  case 353: // CaseStatementList: CaseStatementList CaseStatement
    {
      CaseClauseList* cc_list_p = static_cast<CaseClauseList*>((yystack_[1].value.node));
      cc_list_p->push_back(static_cast<CaseClause*>((yystack_[0].value.node)));
      (yylhs.value.node) = (yystack_[1].value.node);
    }
    break;

  case 354: // CaseStatement: "'case'" SequenceTypeList ReturnOrSelect Statement
    {
      (yylhs.value.node) = new CaseClause(LOC (yylhs.location),
                          static_cast<SequenceTypeList*>((yystack_[2].value.node)),
                          (yystack_[0].value.expr));
    }
    break;

  case 355: // CaseStatement: "'case'" "'$'" QNAME "'as'" SequenceTypeList ReturnOrSelect Statement
    {
      (yylhs.value.node) = new CaseClause(LOC (yylhs.location),
                          static_cast<QName*>((yystack_[4].value.expr)),
                          static_cast<SequenceTypeList*>((yystack_[2].value.node)),
                          (yystack_[0].value.expr));
     }
    break;

  case 356: // SequenceTypeList: SequenceType
    {
      SequenceTypeList* seqList = new SequenceTypeList(LOC(yylhs.location));
      seqList->push_back(static_cast<SequenceTypeAST*>((yystack_[0].value.node)));
      (yylhs.value.node) = seqList;
    }
    break;

  case 357: // SequenceTypeList: SequenceTypeList "'|'" SequenceType
    {
      SequenceTypeList* seqList = static_cast<SequenceTypeList*>((yystack_[2].value.node));
      seqList->push_back(static_cast<SequenceTypeAST*>((yystack_[0].value.node)));
      (yylhs.value.node) = (yystack_[2].value.node);
    }
    break;

  case 358: // IfExpr: "'if'" "'('" Expr "')'" "'then'" ExprSingle "'else'" ExprSingle
    {
      (yylhs.value.expr) = new IfExpr(LOC (yylhs.location), (yystack_[5].value.expr), (yystack_[2].value.expr), (yystack_[0].value.expr));
    }
    break;

  case 359: // OrExpr: AndExpr
    {
      (yylhs.value.expr) = (yystack_[0].value.expr);
    }
    break;

  case 360: // OrExpr: OrExpr "'or'" AndExpr
    {
      (yylhs.value.expr) = new OrExpr( LOC(yylhs.location), (yystack_[2].value.expr), (yystack_[0].value.expr) );
    }
    break;

  case 361: // AndExpr: NotExpr
        {
            (yylhs.value.expr) = (yystack_[0].value.expr);
        }
    break;

  case 362: // AndExpr: AndExpr "'and'" NotExpr
        {
            (yylhs.value.expr) = new AndExpr( LOC(yylhs.location), (yystack_[2].value.expr), (yystack_[0].value.expr) );
        }
    break;

  case 363: // NotExpr: ComparisonExpr
        {
          (yylhs.value.expr) = (yystack_[0].value.expr);
        }
    break;

  case 364: // NotExpr: "'not'" NotExpr
        {
          (yylhs.value.expr) = new ComparisonExpr(
              LOC(yylhs.location),
              new ValueComp( LOC(yylhs.location), ParseConstants::op_val_not ),
              (yystack_[0].value.expr),
              NULL
          );
        }
    break;

  case 365: // ComparisonExpr: FTContainsExpr
        {
            (yylhs.value.expr) = (yystack_[0].value.expr);
        }
    break;

  case 366: // ComparisonExpr: FTContainsExpr ValueComp FTContainsExpr
        {
            (yylhs.value.expr) = new ComparisonExpr(
                LOC(yylhs.location),
                dynamic_cast<ValueComp*>((yystack_[1].value.node)),
                (yystack_[2].value.expr),
                (yystack_[0].value.expr)
            );
        }
    break;

  case 367: // ComparisonExpr: FTContainsExpr NodeComp FTContainsExpr
        {
            (yylhs.value.expr) = new ComparisonExpr(
                LOC(yylhs.location), dynamic_cast<NodeComp*>((yystack_[1].value.node)), (yystack_[2].value.expr), (yystack_[0].value.expr)
            );
        }
    break;

  case 368: // ComparisonExpr: FTContainsExpr "'='" FTContainsExpr
        {
            (yylhs.value.expr) = new ComparisonExpr(
                LOC(yylhs.location),
                new GeneralComp( LOC(yylhs.location), ParseConstants::op_eq ),
                (yystack_[2].value.expr),
                (yystack_[0].value.expr)
            );
        }
    break;

  case 369: // ComparisonExpr: FTContainsExpr "'!='" FTContainsExpr
        {
            (yylhs.value.expr) = new ComparisonExpr(
                LOC(yylhs.location),
                new GeneralComp( LOC(yylhs.location), ParseConstants::op_ne ),
                (yystack_[2].value.expr),
                (yystack_[0].value.expr)
            );
        }
    break;

  case 370: // $@1: %empty
        {
            driver.getJsoniqLexer()->interpretAsLessThan();
        }
    break;

  case 371: // ComparisonExpr: FTContainsExpr "'<'" $@1 FTContainsExpr
        {
            (yylhs.value.expr) = new ComparisonExpr(
                LOC(yylhs.location),
                new GeneralComp( LOC(yylhs.location), ParseConstants::op_lt ),
                (yystack_[3].value.expr),
                (yystack_[0].value.expr)
            );
        }
    break;

  case 372: // ComparisonExpr: FTContainsExpr "'<='" FTContainsExpr
        {
            (yylhs.value.expr) = new ComparisonExpr(
                LOC(yylhs.location),
                new GeneralComp( LOC(yylhs.location), ParseConstants::op_le ),
                (yystack_[2].value.expr),
                (yystack_[0].value.expr)
            );
        }
    break;

  case 373: // ComparisonExpr: FTContainsExpr "'>'" FTContainsExpr
        {
            (yylhs.value.expr) = new ComparisonExpr(
                LOC(yylhs.location),
                new GeneralComp( LOC(yylhs.location), ParseConstants::op_gt ),
                (yystack_[2].value.expr),
                (yystack_[0].value.expr)
            );
        }
    break;

  case 374: // ComparisonExpr: FTContainsExpr "'>='" FTContainsExpr
        {
            (yylhs.value.expr) = new ComparisonExpr(
                LOC(yylhs.location),
                new GeneralComp( LOC(yylhs.location), ParseConstants::op_ge ),
                (yystack_[2].value.expr),
                (yystack_[0].value.expr)
            );
        }
    break;

  case 375: // FTContainsExpr: StringConcatExpr
        {
            (yylhs.value.expr) = (yystack_[0].value.expr);
        }
    break;

  case 376: // FTContainsExpr: StringConcatExpr "'contains'" "'text'" FTSelection opt_FTIgnoreOption
        {
            (yylhs.value.expr) = new FTContainsExpr(
                LOC(yylhs.location),
                (yystack_[4].value.expr),
                dynamic_cast<FTSelection*>((yystack_[1].value.node)),
                dynamic_cast<FTIgnoreOption*>((yystack_[0].value.node))
            );
        }
    break;

  case 377: // StringConcatExpr: RangeExpr
        {
          (yylhs.value.expr) = (yystack_[0].value.expr);
        }
    break;

  case 378: // StringConcatExpr: RangeExpr "'||'" StringConcatExpr
        {
          (yylhs.value.expr) = new StringConcatExpr(LOC(yylhs.location), (yystack_[2].value.expr), (yystack_[0].value.expr));
        }
    break;

  case 379: // opt_FTIgnoreOption: %empty
        {
            (yylhs.value.node) = NULL;
        }
    break;

  case 380: // opt_FTIgnoreOption: FTIgnoreOption
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 381: // RangeExpr: AdditiveExpr
        {
            (yylhs.value.expr) = (yystack_[0].value.expr);
        }
    break;

  case 382: // RangeExpr: AdditiveExpr "'to'" AdditiveExpr
        {
            (yylhs.value.expr) = new RangeExpr( LOC(yylhs.location), (yystack_[2].value.expr), (yystack_[0].value.expr) );
        }
    break;

  case 383: // AdditiveExpr: MultiplicativeExpr
        {
            (yylhs.value.expr) = (yystack_[0].value.expr);
        }
    break;

  case 384: // AdditiveExpr: AdditiveExpr "'+'" MultiplicativeExpr
        {
            (yylhs.value.expr) = new AdditiveExpr( LOC(yylhs.location), ParseConstants::op_plus, (yystack_[2].value.expr), (yystack_[0].value.expr) );
        }
    break;

  case 385: // AdditiveExpr: AdditiveExpr "'-'" MultiplicativeExpr
        {
            (yylhs.value.expr) = new AdditiveExpr( LOC(yylhs.location), ParseConstants::op_minus, (yystack_[2].value.expr), (yystack_[0].value.expr) );
        }
    break;

  case 386: // MultiplicativeExpr: UnionExpr
        {
            (yylhs.value.expr) = (yystack_[0].value.expr);
        }
    break;

  case 387: // MultiplicativeExpr: MultiplicativeExpr "'*'" UnionExpr
        {
            (yylhs.value.expr) = new MultiplicativeExpr(
                LOC(yylhs.location), ParseConstants::op_mul, (yystack_[2].value.expr), (yystack_[0].value.expr)
            );
        }
    break;

  case 388: // MultiplicativeExpr: MultiplicativeExpr "'div'" UnionExpr
        {
            (yylhs.value.expr) = new MultiplicativeExpr(
                LOC(yylhs.location), ParseConstants::op_div, (yystack_[2].value.expr), (yystack_[0].value.expr)
            );
        }
    break;

  case 389: // MultiplicativeExpr: MultiplicativeExpr "'idiv'" UnionExpr
        {
            (yylhs.value.expr) = new MultiplicativeExpr(
                LOC(yylhs.location), ParseConstants::op_idiv, (yystack_[2].value.expr), (yystack_[0].value.expr)
            );
        }
    break;

  case 390: // MultiplicativeExpr: MultiplicativeExpr "'mod'" UnionExpr
        {
            (yylhs.value.expr) = new MultiplicativeExpr(
                LOC(yylhs.location), ParseConstants::op_mod, (yystack_[2].value.expr), (yystack_[0].value.expr)
            );
        }
    break;

  case 391: // UnionExpr: IntersectExceptExpr
        {
            (yylhs.value.expr) = (yystack_[0].value.expr);
        }
    break;

  case 392: // UnionExpr: UnionExpr "'union'" IntersectExceptExpr
        {
            (yylhs.value.expr) = new UnionExpr( LOC(yylhs.location), (yystack_[2].value.expr), (yystack_[0].value.expr) );
        }
    break;

  case 393: // UnionExpr: UnionExpr "'|'" IntersectExceptExpr
        {
            (yylhs.value.expr) = new UnionExpr( LOC(yylhs.location), (yystack_[2].value.expr), (yystack_[0].value.expr) );
        }
    break;

  case 394: // IntersectExceptExpr: InstanceofExpr
        {
            (yylhs.value.expr) = (yystack_[0].value.expr);
        }
    break;

  case 395: // IntersectExceptExpr: IntersectExceptExpr "'intersect'" InstanceofExpr
        {
            (yylhs.value.expr) = new IntersectExceptExpr(
                LOC(yylhs.location), ParseConstants::op_intersect, (yystack_[2].value.expr), (yystack_[0].value.expr)
            );
        }
    break;

  case 396: // IntersectExceptExpr: IntersectExceptExpr "'except'" InstanceofExpr
        {
            (yylhs.value.expr) = new IntersectExceptExpr(
                LOC(yylhs.location), ParseConstants::op_except, (yystack_[2].value.expr), (yystack_[0].value.expr)
            );
        }
    break;

  case 397: // InstanceofExpr: TreatExpr
        {
            (yylhs.value.expr) = (yystack_[0].value.expr);
        }
    break;

  case 398: // InstanceofExpr: TreatExpr "'instance'" "'of'" SequenceType
        {
            (yylhs.value.expr) = new InstanceofExpr(
                LOC(yylhs.location), (yystack_[3].value.expr), dynamic_cast<SequenceTypeAST*>((yystack_[0].value.node))
            );
        }
    break;

  case 399: // TreatExpr: CastableExpr
        {
            (yylhs.value.expr) = (yystack_[0].value.expr);
        }
    break;

  case 400: // TreatExpr: CastableExpr "'treat'" "'as'" SequenceType
        {
            (yylhs.value.expr) = new TreatExpr(
                LOC(yylhs.location), (yystack_[3].value.expr), dynamic_cast<SequenceTypeAST*>((yystack_[0].value.node))
            );
        }
    break;

  case 401: // CastableExpr: CastExpr
        {
            (yylhs.value.expr) = (yystack_[0].value.expr);
        }
    break;

  case 402: // CastableExpr: CastExpr "'castable'" "'as'" SingleType
        {
            (yylhs.value.expr) = new CastableExpr(
                LOC(yylhs.location), (yystack_[3].value.expr), dynamic_cast<SingleType*>((yystack_[0].value.node))
            );
        }
    break;

  case 403: // CastExpr: UnaryExpr
        {
            (yylhs.value.expr) = (yystack_[0].value.expr);
        }
    break;

  case 404: // CastExpr: UnaryExpr "'cast'" "'as'" SingleType
        {
            (yylhs.value.expr) = new CastExpr(
                LOC(yylhs.location), (yystack_[3].value.expr), dynamic_cast<SingleType*>((yystack_[0].value.node))
            );
        }
    break;

  case 405: // SingleType: SimpleType
    {
      (yylhs.value.node) = new SingleType(LOC(yylhs.location), dynamic_cast<SimpleType*>((yystack_[0].value.node)), false);
    }
    break;

  case 406: // SingleType: SimpleType "'?'"
    {
      (yylhs.value.node) = new SingleType(LOC(yylhs.location), dynamic_cast<SimpleType*>((yystack_[1].value.node)), true);
    }
    break;

  case 407: // UnaryExpr: ValueExpr
        {
            (yylhs.value.expr) = (yystack_[0].value.expr);
        }
    break;

  case 408: // UnaryExpr: SignList ValueExpr
        {
            (yylhs.value.expr) = new UnaryExpr( LOC(yylhs.location), dynamic_cast<SignList*>((yystack_[1].value.node)), (yystack_[0].value.expr) );
        }
    break;

  case 409: // SignList: "'+'"
        {
            (yylhs.value.node) = new SignList( LOC(yylhs.location), true );
        }
    break;

  case 410: // SignList: "'-'"
        {
            (yylhs.value.node) = new SignList( LOC(yylhs.location), false );
        }
    break;

  case 411: // SignList: SignList "'+'"
        {
            (yylhs.value.node) = (yystack_[1].value.node);
        }
    break;

  case 412: // SignList: SignList "'-'"
        {
            if ( SignList *sl = dynamic_cast<SignList*>((yystack_[1].value.node)) )
                sl->negate();
            (yylhs.value.node) = (yystack_[1].value.node);
        }
    break;

  case 413: // ValueExpr: ValidateExpr
        { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 414: // ValueExpr: SimpleMapExpr
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 415: // ValueExpr: ExtensionExpr
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 416: // SimpleMapExpr: PathExpr
      {
        (yylhs.value.expr) = (yystack_[0].value.expr);
      }
    break;

  case 417: // SimpleMapExpr: SimpleMapExpr "'!'" PathExpr
      {
        (yylhs.value.expr) = new SimpleMapExpr(LOC(yylhs.location), (yystack_[2].value.expr), (yystack_[0].value.expr));
      }
    break;

  case 418: // ValueComp: "'eq'"
        {
            (yylhs.value.node) = new ValueComp( LOC(yylhs.location), ParseConstants::op_val_eq );
        }
    break;

  case 419: // ValueComp: "'ne'"
        {
            (yylhs.value.node) = new ValueComp( LOC(yylhs.location), ParseConstants::op_val_ne );
        }
    break;

  case 420: // ValueComp: "'lt'"
        {
            (yylhs.value.node) = new ValueComp( LOC(yylhs.location), ParseConstants::op_val_lt );
        }
    break;

  case 421: // ValueComp: "'le'"
        {
            (yylhs.value.node) = new ValueComp( LOC(yylhs.location), ParseConstants::op_val_le );
        }
    break;

  case 422: // ValueComp: "'gt'"
        {
            (yylhs.value.node) = new ValueComp( LOC(yylhs.location), ParseConstants::op_val_gt );
        }
    break;

  case 423: // ValueComp: "'ge'"
        {
            (yylhs.value.node) = new ValueComp( LOC(yylhs.location), ParseConstants::op_val_ge );
        }
    break;

  case 424: // NodeComp: "'is'"
        {
            (yylhs.value.node) = new NodeComp( LOC(yylhs.location), ParseConstants::op_is );
        }
    break;

  case 425: // NodeComp: "'<<'"
        {
            (yylhs.value.node) = new NodeComp( LOC(yylhs.location), ParseConstants::op_precedes );
        }
    break;

  case 426: // NodeComp: "'follows'"
        {
            (yylhs.value.node) = new NodeComp( LOC(yylhs.location), ParseConstants::op_follows );
        }
    break;

  case 427: // ValidateExpr: "'validate'" "'{'" StatementsAndExpr "'}'"
        {
            (yylhs.value.expr) = new ValidateExpr( LOC(yylhs.location), "strict", (yystack_[1].value.expr) );
        }
    break;

  case 428: // ValidateExpr: "'validate'" "'lax'" "'{'" StatementsAndExpr "'}'"
        {
            (yylhs.value.expr) = new ValidateExpr( LOC(yylhs.location), "lax", (yystack_[1].value.expr) );
        }
    break;

  case 429: // ValidateExpr: "'validate'" "'strict'" "'{'" StatementsAndExpr "'}'"
        {
            (yylhs.value.expr) = new ValidateExpr( LOC(yylhs.location), "strict", (yystack_[1].value.expr) );
        }
    break;

  case 430: // ValidateExpr: "'validate'" "'type'" TypeName "'{'" StatementsAndExpr "'}'"
        {
            (yylhs.value.expr) = new ValidateExpr(
                LOC(yylhs.location), dynamic_cast<TypeName*>((yystack_[3].value.node))->get_name(), (yystack_[1].value.expr)
            );
            delete (yystack_[3].value.node);
        }
    break;

  case 431: // ExtensionExpr: Pragma_list "'{'" StatementsAndOptionalExpr "'}'"
        {
            (yylhs.value.expr) = new ExtensionExpr(
                LOC(yylhs.location), dynamic_cast<PragmaList*>((yystack_[3].value.node)), (yystack_[1].value.expr)
            );
        }
    break;

  case 432: // Pragma_list: Pragma
        {
            PragmaList *pl = new PragmaList( LOC(yylhs.location) );
            pl->push_back( dynamic_cast<Pragma*>((yystack_[0].value.node)) );
            (yylhs.value.node) = pl;
        }
    break;

  case 433: // Pragma_list: Pragma_list Pragma
        {
            if ( PragmaList *pl = dynamic_cast<PragmaList*>((yystack_[1].value.node)) )
                pl->push_back( dynamic_cast<Pragma*>((yystack_[0].value.node)) );
            (yylhs.value.node) = (yystack_[1].value.node);
        }
    break;

  case 434: // Pragma: "'(#'" QNAME "'pragma literal'"
        {
            (yylhs.value.node) = new Pragma( LOC(yylhs.location), static_cast<QName*>((yystack_[1].value.expr)), SYMTAB((yystack_[0].value.sval)) );
        }
    break;

  case 435: // Pragma: "'(#'" "'QName #)'"
        {
            (yylhs.value.node) = new Pragma( LOC(yylhs.location), new QName( LOC(yylhs.location), SYMTAB((yystack_[0].value.sval)) ), "" );
        }
    break;

  case 436: // Pragma: "'(#'" "'EQName #)'"
        {
            (yylhs.value.node) = new Pragma( LOC(yylhs.location), new QName( LOC(yylhs.location), SYMTAB((yystack_[0].value.sval)), true ), "" );
        }
    break;

  case 437: // PathExpr: LeadingSlash
    {
      (yylhs.value.expr) = new PathExpr(LOC(yylhs.location), ParseConstants::path_leading_lone_slash, NULL);
    }
    break;

  case 438: // PathExpr: LeadingSlash RelativePathExpr
    {
      RelativePathExpr* rpe;
      rpe = new RelativePathExpr(LOC(yylhs.location), ParseConstants::st_slash, NULL, (yystack_[0].value.expr), false);
      (yylhs.value.expr) = new PathExpr(LOC(yylhs.location),
                        ParseConstants::path_leading_slash,
                        rpe);
    }
    break;

  case 439: // PathExpr: "'//'" RelativePathExpr
    {
      RelativePathExpr* rpe;
      rpe = new RelativePathExpr(LOC(yylhs.location), ParseConstants::st_slashslash, NULL, (yystack_[0].value.expr), false);
      (yylhs.value.expr) = new PathExpr(LOC(yylhs.location),
                        ParseConstants::path_leading_slashslash,
                        rpe);
    }
    break;

  case 440: // PathExpr: RelativePathExpr
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

  case 441: // LeadingSlash: "'/'"
    {
      (yylhs.value.expr) = NULL;
    }
    break;

  case 442: // RelativePathExpr: StepExpr
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

  case 443: // RelativePathExpr: StepExpr "'/'" RelativePathExpr
    {
      (yylhs.value.expr) = new RelativePathExpr(LOC(yylhs.location), ParseConstants::st_slash, (yystack_[2].value.expr), (yystack_[0].value.expr), false);
    }
    break;

  case 444: // RelativePathExpr: StepExpr "'//'" RelativePathExpr
    {
      (yylhs.value.expr) = new RelativePathExpr(LOC(yylhs.location), ParseConstants::st_slashslash, (yystack_[2].value.expr), (yystack_[0].value.expr), false);
    }
    break;

  case 445: // StepExpr: AxisStep
    { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 446: // StepExpr: PostfixExpr
    { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 447: // AxisStep: ForwardStep
        {
            (yylhs.value.expr) = new AxisStep(
                LOC(yylhs.location), dynamic_cast<ForwardStep*>((yystack_[0].value.node)), NULL
            );
        }
    break;

  case 448: // AxisStep: ForwardStep PredicateList
        {
            (yylhs.value.expr) = new AxisStep(
                LOC(yylhs.location),
                dynamic_cast<ForwardStep*>((yystack_[1].value.node)),
                dynamic_cast<PredicateList*>((yystack_[0].value.node))
            );
        }
    break;

  case 449: // AxisStep: ReverseStep
        {
            (yylhs.value.expr) = new AxisStep(
                LOC(yylhs.location), dynamic_cast<ReverseStep*>((yystack_[0].value.node)), NULL
            );
        }
    break;

  case 450: // AxisStep: ReverseStep PredicateList
        {
            (yylhs.value.expr) = new AxisStep(
                LOC(yylhs.location),
                dynamic_cast<ReverseStep*>((yystack_[1].value.node)),
                dynamic_cast<PredicateList*>((yystack_[0].value.node))
            );
        }
    break;

  case 451: // ForwardStep: ForwardAxis NodeTest
        {
            (yylhs.value.node) = new ForwardStep(
                LOC(yylhs.location), dynamic_cast<ForwardAxis*>((yystack_[1].value.node)), (yystack_[0].value.node)
            );
        }
    break;

  case 452: // ForwardStep: AbbrevForwardStep
        {
            (yylhs.value.node) = new ForwardStep(
                LOC(yylhs.location), dynamic_cast<AbbrevForwardStep*>((yystack_[0].value.node))
            );
        }
    break;

  case 453: // ForwardAxis: "'child'" "'::'"
        {
            (yylhs.value.node) = new ForwardAxis( LOC(yylhs.location), ParseConstants::axis_child );
        }
    break;

  case 454: // ForwardAxis: "'descendant'" "'::'"
        {
            (yylhs.value.node) = new ForwardAxis( LOC(yylhs.location), ParseConstants::axis_descendant);
        }
    break;

  case 455: // ForwardAxis: "'attribute'" "'::'"
        {
            (yylhs.value.node) = new ForwardAxis( LOC(yylhs.location), ParseConstants::axis_attribute );
        }
    break;

  case 456: // ForwardAxis: "'self'" "'::'"
        {
            (yylhs.value.node) = new ForwardAxis( LOC(yylhs.location), ParseConstants::axis_self );
        }
    break;

  case 457: // ForwardAxis: "'descendant-or-self'" "'::'"
        {
            (yylhs.value.node) = new ForwardAxis(
                LOC(yylhs.location), ParseConstants::axis_descendant_or_self
            );
        }
    break;

  case 458: // ForwardAxis: "'following-sibling'" "'::'"
        {
            (yylhs.value.node) = new ForwardAxis(
                LOC(yylhs.location), ParseConstants::axis_following_sibling
            );
        }
    break;

  case 459: // ForwardAxis: "'following'" "'::'"
        {
            (yylhs.value.node) = new ForwardAxis( LOC(yylhs.location), ParseConstants::axis_following );
        }
    break;

  case 460: // AbbrevForwardStep: NodeTest
        {
            (yylhs.value.node) = new AbbrevForwardStep( LOC(yylhs.location), (yystack_[0].value.node), false );
        }
    break;

  case 461: // AbbrevForwardStep: "'@'" NodeTest
        {
            (yylhs.value.node) = new AbbrevForwardStep( LOC(yylhs.location), (yystack_[0].value.node), true );
        }
    break;

  case 462: // ReverseStep: ReverseAxis NodeTest
        {
            (yylhs.value.node) = new ReverseStep( LOC(yylhs.location), dynamic_cast<ReverseAxis*>((yystack_[1].value.node)), (yystack_[0].value.node) );
        }
    break;

  case 463: // ReverseStep: "'..'"
        {
            ReverseAxis *ra = new ReverseAxis(
                LOC(yylhs.location), ParseConstants::axis_parent
            );
            (yylhs.value.node) = new ReverseStep( LOC(yylhs.location), ra, NULL );
        }
    break;

  case 464: // ReverseAxis: "'parent'" "'::'"
        {
            (yylhs.value.node) = new ReverseAxis( LOC(yylhs.location), ParseConstants::axis_parent );
        }
    break;

  case 465: // ReverseAxis: "'ancestor'" "'::'"
        {
            (yylhs.value.node) = new ReverseAxis( LOC(yylhs.location), ParseConstants::axis_ancestor );
        }
    break;

  case 466: // ReverseAxis: "'preceding-sibling'" "'::'"
        {
            (yylhs.value.node) = new ReverseAxis(
                LOC(yylhs.location), ParseConstants::axis_preceding_sibling
            );
        }
    break;

  case 467: // ReverseAxis: "'preceding'" "'::'"
        {
            (yylhs.value.node) = new ReverseAxis( LOC(yylhs.location), ParseConstants::axis_preceding );
        }
    break;

  case 468: // ReverseAxis: "'ancestor-or-self'" "'::'"
        {
            (yylhs.value.node) = new ReverseAxis(
                LOC(yylhs.location), ParseConstants::axis_ancestor_or_self
            );
        }
    break;

  case 469: // NodeTest: KindTest
        { (yylhs.value.node) = (yystack_[0].value.node); }
    break;

  case 470: // NodeTest: NameTest
      { (yylhs.value.node) = (yystack_[0].value.node); }
    break;

  case 471: // NameTest: QNAME
        {
            (yylhs.value.node) = new NameTest( LOC(yylhs.location), static_cast<QName*>((yystack_[0].value.expr)) );
        }
    break;

  case 472: // NameTest: Wildcard
        {
            (yylhs.value.node) = new NameTest( LOC(yylhs.location), dynamic_cast<Wildcard*>((yystack_[0].value.node)) );
        }
    break;

  case 473: // Wildcard: "'*'"
    {
      (yylhs.value.node) = new Wildcard(LOC(yylhs.location), "", "", ParseConstants::wild_all, false);
    }
    break;

  case 474: // Wildcard: "'pref:*'"
    {
      (yylhs.value.node) = new Wildcard(LOC(yylhs.location), SYMTAB((yystack_[0].value.sval)), "", ParseConstants::wild_elem, false);
    }
    break;

  case 475: // Wildcard: "'ns:*'"
    {
      (yylhs.value.node) = new Wildcard(LOC(yylhs.location), SYMTAB((yystack_[0].value.sval)), "", ParseConstants::wild_elem, true);
    }
    break;

  case 476: // Wildcard: "'*:QName'"
    {
      (yylhs.value.node) = new Wildcard(LOC(yylhs.location), "", SYMTAB((yystack_[0].value.sval)), ParseConstants::wild_prefix, false);
    }
    break;

  case 477: // PostfixExpr: PrimaryExpr
     {
       (yylhs.value.expr) = (yystack_[0].value.expr);
     }
    break;

  case 478: // PostfixExpr: PostfixExpr PredicateList
     {
       (yylhs.value.expr) = new FilterExpr(LOC(yylhs.location), (yystack_[1].value.expr), dynamic_cast<PredicateList*>((yystack_[0].value.node)));
     }
    break;

  case 479: // PostfixExpr: PostfixExpr "'('" "')'"
     {
       (yylhs.value.expr) = new DynamicFunctionInvocation(LOC(yylhs.location), (yystack_[2].value.expr), false);
     }
    break;

  case 480: // PostfixExpr: PostfixExpr "'('" ArgList "')'"
     {
       (yylhs.value.expr) = new DynamicFunctionInvocation(LOC(yylhs.location), (yystack_[3].value.expr), dynamic_cast<ArgList*>((yystack_[1].value.node)), false);
     }
    break;

  case 481: // PostfixExpr: PostfixExpr "'['" "']'"
    {
      (yylhs.value.expr) = new JSONArrayUnboxing(LOC(yylhs.location), (yystack_[2].value.expr));
    }
    break;

  case 482: // PostfixExpr: PostfixExpr "'.'" QNAME
     {
       ERROR_IF_QNAME_NOT_NCNAME((yystack_[0].value.expr), yystack_[0].location, (yystack_[2].value.expr));
       StringLiteral* sl = new StringLiteral( LOC(yylhs.location), static_cast<QName*>((yystack_[0].value.expr)));
       (yylhs.value.expr) = new JSONObjectLookup(LOC(yylhs.location), LOC(yystack_[1].location), (yystack_[2].value.expr), sl);
     }
    break;

  case 483: // PostfixExpr: PostfixExpr "'.'" "'('" "')'"
     {
       (yylhs.value.expr) = new JSONObjectLookup(LOC(yylhs.location), LOC(yystack_[2].location),
                                 (yystack_[3].value.expr),
                                 new ParenthesizedExpr(LOC(yylhs.location), NULL));
     }
    break;

  case 484: // PostfixExpr: PostfixExpr "'.'" "'('" Expr "')'"
     {
       (yylhs.value.expr) = new JSONObjectLookup(LOC(yylhs.location), LOC(yystack_[3].location),
                                 (yystack_[4].value.expr),
                                 new ParenthesizedExpr(LOC(yylhs.location), (yystack_[1].value.expr)));
     }
    break;

  case 485: // PostfixExpr: PostfixExpr "'.'" VarRef
     {
        (yylhs.value.expr) = new JSONObjectLookup(LOC(yylhs.location), LOC(yystack_[1].location), (yystack_[2].value.expr), (yystack_[0].value.expr));
     }
    break;

  case 486: // PostfixExpr: PostfixExpr "'.'" ContextItemExpr
     {
        (yylhs.value.expr) = new JSONObjectLookup(LOC(yylhs.location), LOC(yystack_[1].location), (yystack_[2].value.expr), (yystack_[0].value.expr));
     }
    break;

  case 487: // PostfixExpr: PostfixExpr "'.'" StringLiteral
     {
       (yylhs.value.expr) = new JSONObjectLookup(LOC(yylhs.location), LOC(yystack_[1].location), (yystack_[2].value.expr), (yystack_[0].value.expr));
     }
    break;

  case 488: // PredicateList: Predicate
    {
      PredicateList* pl = new PredicateList( LOC(yylhs.location) );
      pl->push_back(dynamic_cast<exprnode*>((yystack_[0].value.expr)));
      (yylhs.value.node) = pl;
    }
    break;

  case 489: // PredicateList: PredicateList Predicate
    {
      if (PredicateList* pl = dynamic_cast<PredicateList*>((yystack_[1].value.node)))
        pl->push_back(dynamic_cast<exprnode*>((yystack_[0].value.expr)));
      (yylhs.value.node) = (yystack_[1].value.node);
    }
    break;

  case 490: // Predicate: "'['" Expr "']'"
    {
      (yylhs.value.expr) = (yystack_[1].value.expr);
    }
    break;

  case 491: // PrimaryExpr: Literal
        { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 492: // PrimaryExpr: VarRef
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 493: // PrimaryExpr: ParenthesizedExpr
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 494: // PrimaryExpr: ContextItemExpr
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 495: // PrimaryExpr: FunctionCall
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 496: // PrimaryExpr: Constructor
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 497: // PrimaryExpr: OrderedExpr
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 498: // PrimaryExpr: UnorderedExpr
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 499: // PrimaryExpr: FunctionItemExpr
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 500: // PrimaryExpr: BlockExpr
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 501: // PrimaryExpr: JSONObjectConstructor
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 502: // PrimaryExpr: JSONArrayConstructor
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 503: // PrimaryExpr: JSONSimpleObjectUnion
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 504: // PrimaryExpr: JSONAccumulatorObjectUnion
      { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 505: // Literal: NumericLiteral
        {
            (yylhs.value.expr) = (yystack_[0].value.expr);
        }
    break;

  case 506: // Literal: StringLiteral
        {
            (yylhs.value.expr) = (yystack_[0].value.expr);
        }
    break;

  case 507: // NumericLiteral: "'DECIMAL'"
        {
            (yylhs.value.expr) = NumericLiteral::new_literal(
                LOC(yylhs.location), ParseConstants::num_decimal, *(yystack_[0].value.decval)
            );
            delete (yystack_[0].value.decval);
        }
    break;

  case 508: // NumericLiteral: "'INTEGER'"
        {
            (yylhs.value.expr) = NumericLiteral::new_literal(
                LOC(yylhs.location), ParseConstants::num_integer, *(yystack_[0].value.ival)
            );
            delete (yystack_[0].value.ival);
        }
    break;

  case 509: // NumericLiteral: "'DOUBLE'"
        {
            (yylhs.value.expr) = NumericLiteral::new_literal(
                LOC(yylhs.location), ParseConstants::num_double, *(yystack_[0].value.dval)
            );
            delete (yystack_[0].value.dval);
        }
    break;

  case 510: // VarRef: "'$'" QNAME
        {
            (yylhs.value.expr) = new VarRef(LOC(yylhs.location), static_cast<QName*>((yystack_[0].value.expr)));
        }
    break;

  case 511: // ParenthesizedExpr: "'('" "')'"
        {
            (yylhs.value.expr) = new ParenthesizedExpr( LOC(yylhs.location), NULL);
        }
    break;

  case 512: // ParenthesizedExpr: "'('" Expr "')'"
        {
            (yylhs.value.expr) = new ParenthesizedExpr( LOC(yylhs.location), (yystack_[1].value.expr) );
        }
    break;

  case 513: // ContextItemExpr: "'$_'"
        {
            driver.addCommonLanguageWarning(yystack_[0].location, ZED(ZWST0009_CONTEXT_ITEM_EXPR));
            (yylhs.value.expr) = new ContextItemExpr( LOC(yylhs.location) );
        }
    break;

  case 514: // OrderedExpr: "'ordered'" "'{'" Expr "'}'"
        {
            (yylhs.value.expr) = new OrderedExpr( LOC(yylhs.location), (yystack_[1].value.expr) );
        }
    break;

  case 515: // UnorderedExpr: "'unordered'" "'{'" Expr "'}'"
        {
            (yylhs.value.expr) = new UnorderedExpr( LOC(yylhs.location), (yystack_[1].value.expr) );
        }
    break;

  case 516: // FunctionCall: FUNCTION_NAME "'('" "')'"
    {
      (yylhs.value.expr) = new FunctionCall(LOC(yylhs.location), static_cast<QName*>((yystack_[2].value.expr)), NULL);
    }
    break;

  case 517: // FunctionCall: FUNCTION_NAME "'('" ArgList "')'"
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

  case 518: // ArgList: "'?'"
    {
      ArgList* al = new ArgList(LOC(yylhs.location));
      al->push_back(new ArgumentPlaceholder(LOC(yylhs.location)));
      (yylhs.value.node) = al;
    }
    break;

  case 519: // ArgList: ArgList "','" "'?'"
    {
      if (ArgList* al = dynamic_cast<ArgList*>((yystack_[2].value.node)))
        al->push_back(new ArgumentPlaceholder(LOC(yylhs.location)));
      (yylhs.value.node) = (yystack_[2].value.node);
    }
    break;

  case 520: // ArgList: ExprSingle
    {
      ArgList *al = new ArgList( LOC(yylhs.location) );
      al->push_back( (yystack_[0].value.expr) );
      (yylhs.value.node) = al;
    }
    break;

  case 521: // ArgList: ArgList "','" ExprSingle
    {
      if ( ArgList *al = dynamic_cast<ArgList*>((yystack_[2].value.node)) )
        al->push_back( (yystack_[0].value.expr) );
      (yylhs.value.node) = (yystack_[2].value.node);
    }
    break;

  case 522: // FunctionItemExpr: LiteralFunctionItem
    {
      (yylhs.value.expr) = (yystack_[0].value.expr);
    }
    break;

  case 523: // FunctionItemExpr: InlineFunction
    {
      (yylhs.value.expr) = (yystack_[0].value.expr);
    }
    break;

  case 524: // LiteralFunctionItem: FUNCTION_NAME "'#'" "'INTEGER'"
    {
      (yylhs.value.expr) = new LiteralFunctionItem(LOC (yylhs.location), dynamic_cast<QName*>((yystack_[2].value.expr)), (yystack_[0].value.ival));
    }
    break;

  case 525: // InlineFunction: "'function'" FunctionSig EnclosedStatementsAndOptionalExpr
    {
      (yylhs.value.expr) = new InlineFunction(LOC(yylhs.location),
                              (yystack_[1].value.fnsig)->theParams.getp(),
                              (yystack_[1].value.fnsig)->theReturnType.getp(),
                              (yystack_[0].value.expr));
      delete (yystack_[1].value.fnsig);
    }
    break;

  case 526: // InlineFunction: AnnotationList "'function'" FunctionSig EnclosedStatementsAndOptionalExpr
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

  case 527: // Constructor: DirectConstructor
    { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 528: // Constructor: ComputedConstructor
  { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 529: // DirectConstructor: DirElemConstructor
    { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 530: // DirectConstructor: DirCommentConstructor
    { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 531: // DirectConstructor: DirPIConstructor
    { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 532: // DirElemConstructor: "'<'" QNAME OptionalBlank "'/>'"
    {
      (yylhs.value.expr) = new DirElemConstructor(LOC(yylhs.location),
                                  static_cast<QName*>((yystack_[2].value.expr)),
                                  NULL,
                                  NULL,
                                  NULL);
    }
    break;

  case 533: // DirElemConstructor: "'<'" QNAME OptionalBlank "'> (tag end)'" "'</ (start tag end)'" QNAME OptionalBlank "'> (tag end)'"
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

  case 534: // DirElemConstructor: "'<'" QNAME DirAttributeList OptionalBlank "'/>'"
    {
      (yylhs.value.expr) = new DirElemConstructor(LOC(yylhs.location),
                                  static_cast<QName*>((yystack_[3].value.expr)),
                                  NULL,
                                  dynamic_cast<DirAttributeList*>((yystack_[2].value.node)),
                                  NULL);
    }
    break;

  case 535: // DirElemConstructor: "'<'" QNAME DirAttributeList OptionalBlank "'> (tag end)'" "'</ (start tag end)'" QNAME OptionalBlank "'> (tag end)'"
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

  case 536: // DirElemConstructor: "'<'" QNAME OptionalBlank "'> (tag end)'" DirElemContentList "'</ (start tag end)'" QNAME OptionalBlank "'> (tag end)'"
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

  case 537: // DirElemConstructor: "'<'" QNAME DirAttributeList OptionalBlank "'> (tag end)'" DirElemContentList "'</ (start tag end)'" QNAME OptionalBlank "'> (tag end)'"
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

  case 538: // DirElemContentList: DirElemContent
    {
      DirElemContentList *decl = new DirElemContentList( LOC(yylhs.location) );
      decl->push_back( dynamic_cast<DirElemContent*>((yystack_[0].value.expr)) );
      (yylhs.value.node) = decl;
    }
    break;

  case 539: // DirElemContentList: DirElemContentList DirElemContent
    {
      DirElemContentList *decl = dynamic_cast<DirElemContentList*>((yystack_[1].value.node));
      if ( decl )
        decl->push_back( dynamic_cast<DirElemContent*>((yystack_[0].value.expr)) );
      (yylhs.value.node) = (yystack_[1].value.node);
    }
    break;

  case 540: // DirAttributeList: DirAttr
    {
      DirAttributeList *dal = new DirAttributeList( LOC(yylhs.location) );
      dal->push_back( dynamic_cast<DirAttr*>((yystack_[0].value.node)) );
      (yylhs.value.node) = dal;
    }
    break;

  case 541: // DirAttributeList: DirAttributeList DirAttr
    {
      DirAttributeList *dal = dynamic_cast<DirAttributeList*>((yystack_[1].value.node));
      if ( dal )
        dal->push_back( dynamic_cast<DirAttr*>((yystack_[0].value.node)) );
      (yylhs.value.node) = (yystack_[1].value.node);
    }
    break;

  case 542: // DirAttr: "'<blank>'" QNAME OptionalBlank "'='" OptionalBlank DirAttributeValue
    {
      (yylhs.value.node) = new DirAttr(LOC(yylhs.location),
                       static_cast<QName*>((yystack_[4].value.expr)),
                       dynamic_cast<DirAttributeValue*>((yystack_[0].value.node)));
    }
    break;

  case 545: // DirAttributeValue: "'\"'" opt_QuoteAttrContentList "'\"'"
    {
      (yylhs.value.node) = new DirAttributeValue(LOC(yylhs.location),
                                 dynamic_cast<QuoteAttrContentList*>((yystack_[1].value.node)));
    }
    break;

  case 546: // DirAttributeValue: "'''" opt_AposAttrContentList "'''"
    {
      (yylhs.value.node) = new DirAttributeValue( LOC(yylhs.location),
                                  dynamic_cast<AposAttrContentList*>((yystack_[1].value.node)));
    }
    break;

  case 547: // opt_QuoteAttrContentList: %empty
    {
      (yylhs.value.node) = new QuoteAttrContentList( LOC(yylhs.location) );
    }
    break;

  case 548: // opt_QuoteAttrContentList: QuoteAttrContentList
    {
      (yylhs.value.node) = (yystack_[0].value.node);
    }
    break;

  case 549: // QuoteAttrContentList: "'\"\"'"
    {
      QuoteAttrContentList *qacl = new QuoteAttrContentList( LOC(yylhs.location) );
      qacl->push_back( new QuoteAttrValueContent( LOC(yylhs.location), "\"" ) );
      (yylhs.value.node) = qacl;
    }
    break;

  case 550: // QuoteAttrContentList: QuoteAttrValueContent
    {
      QuoteAttrContentList *qacl = new QuoteAttrContentList( LOC(yylhs.location) );
      qacl->push_back( dynamic_cast<QuoteAttrValueContent*>((yystack_[0].value.node)) );
      (yylhs.value.node) = qacl;
    }
    break;

  case 551: // QuoteAttrContentList: QuoteAttrContentList "'\"\"'"
    {
      QuoteAttrContentList* qacl = dynamic_cast<QuoteAttrContentList*>((yystack_[1].value.node));
      if ( qacl )
        qacl->push_back( new QuoteAttrValueContent( LOC(yylhs.location), "\"" ) );
      (yylhs.value.node) = (yystack_[1].value.node);
    }
    break;

  case 552: // QuoteAttrContentList: QuoteAttrContentList QuoteAttrValueContent
    {
      QuoteAttrContentList *qacl = dynamic_cast<QuoteAttrContentList*>((yystack_[1].value.node));
      if ( qacl )
        qacl->push_back( dynamic_cast<QuoteAttrValueContent*>((yystack_[0].value.node)) );
      (yylhs.value.node) = (yystack_[1].value.node);
    }
    break;

  case 553: // opt_AposAttrContentList: %empty
    {
      (yylhs.value.node) = new AposAttrContentList( LOC(yylhs.location) );
    }
    break;

  case 554: // opt_AposAttrContentList: AposAttrContentList
    {
      (yylhs.value.node) = (yystack_[0].value.node);
    }
    break;

  case 555: // AposAttrContentList: "''''"
    {
      AposAttrContentList *aacl = new AposAttrContentList( LOC(yylhs.location) );
      aacl->push_back( new AposAttrValueContent( LOC(yylhs.location),"'") );
      (yylhs.value.node) = aacl;
    }
    break;

  case 556: // AposAttrContentList: AposAttrValueContent
    {
      AposAttrContentList *aacl = new AposAttrContentList( LOC(yylhs.location) );
      aacl->push_back( dynamic_cast<AposAttrValueContent*>((yystack_[0].value.node)) );
      (yylhs.value.node) = aacl;
    }
    break;

  case 557: // AposAttrContentList: AposAttrContentList "''''"
    {
      AposAttrContentList *aacl = dynamic_cast<AposAttrContentList*>((yystack_[1].value.node));
      if (aacl)
        aacl->push_back( new AposAttrValueContent( LOC(yylhs.location),"'") );
      (yylhs.value.node) = (yystack_[1].value.node);
    }
    break;

  case 558: // AposAttrContentList: AposAttrContentList AposAttrValueContent
    {
      AposAttrContentList *aacl = dynamic_cast<AposAttrContentList*>((yystack_[1].value.node));
      if ( aacl )
        aacl->push_back( dynamic_cast<AposAttrValueContent*>((yystack_[0].value.node)) );
      (yylhs.value.node) = (yystack_[1].value.node);
    }
    break;

  case 559: // QuoteAttrValueContent: "'quote attribute content'"
    {
      (yylhs.value.node) = new QuoteAttrValueContent( LOC(yylhs.location), SYMTAB((yystack_[0].value.sval)) );
    }
    break;

  case 560: // QuoteAttrValueContent: CommonContent
    {
      (yylhs.value.node) = new QuoteAttrValueContent(LOC(yylhs.location), dynamic_cast<CommonContent*>((yystack_[0].value.expr)));
    }
    break;

  case 561: // AposAttrValueContent: "'apos attribute content'"
    {
      (yylhs.value.node) = new AposAttrValueContent( LOC(yylhs.location), SYMTAB((yystack_[0].value.sval)) );
    }
    break;

  case 562: // AposAttrValueContent: CommonContent
    {
      (yylhs.value.node) = new AposAttrValueContent(LOC(yylhs.location), dynamic_cast<CommonContent*>((yystack_[0].value.expr)));
    }
    break;

  case 563: // DirElemContent: DirectConstructor
    {
      (yylhs.value.expr) = new DirElemContent( LOC(yylhs.location), (yystack_[0].value.expr) );
    }
    break;

  case 564: // DirElemContent: "'element content'"
    {
      (yylhs.value.expr) = new DirElemContent( LOC(yylhs.location), SYMTAB((yystack_[0].value.sval)) );
    }
    break;

  case 565: // DirElemContent: CDataSection
    {
      rchandle<CDataSection> cdata_h = dynamic_cast<CDataSection*>((yystack_[0].value.expr));
      (yylhs.value.expr) = new DirElemContent( LOC(yylhs.location), cdata_h );
    }
    break;

  case 566: // DirElemContent: CommonContent
    {
      rchandle<CommonContent> cont_h = dynamic_cast<CommonContent*>((yystack_[0].value.expr));
      (yylhs.value.expr) = new DirElemContent( LOC(yylhs.location), cont_h );
    }
    break;

  case 567: // CommonContent: "'&#charref;'"
    {
      (yylhs.value.expr) = new CommonContent(LOC(yylhs.location), ParseConstants::cont_charref, SYMTAB((yystack_[0].value.sval)));
    }
    break;

  case 568: // CommonContent: "'{{'"
    {
      (yylhs.value.expr) = new CommonContent(LOC(yylhs.location), ParseConstants::cont_escape_lbrace);
    }
    break;

  case 569: // CommonContent: "'<double {>'"
    {
      (yylhs.value.expr) = new CommonContent(LOC(yylhs.location), ParseConstants::cont_escape_rbrace);
    }
    break;

  case 570: // CommonContent: "'{'" StatementsAndExpr "'}'"
    {
      (yylhs.value.expr) = new CommonContent(LOC(yylhs.location), new EnclosedExpr(LOC(yylhs.location), (yystack_[1].value.expr)));
    }
    break;

  case 571: // DirCommentConstructor: "'<!--'" "'XML comment'" "'-->'"
    {
      (yylhs.value.expr) = new DirCommentConstructor( LOC(yylhs.location), SYMTAB((yystack_[1].value.sval)) );
    }
    break;

  case 572: // DirCommentConstructor: "'<!--'" "'-->'"
    {
      (yylhs.value.expr) = new DirCommentConstructor( LOC(yylhs.location), "" );
    }
    break;

  case 573: // DirPIConstructor: "'<?'" QNAME "'?>'"
    {
      ERROR_IF_QNAME_NOT_NCNAME((yystack_[1].value.expr), yystack_[1].location);
      (yylhs.value.expr) = new DirPIConstructor( LOC(yylhs.location), static_cast<QName*>((yystack_[1].value.expr)) );
    }
    break;

  case 574: // DirPIConstructor: "'<?'" QNAME "'char literal and pi end'"
    {
      ERROR_IF_QNAME_NOT_NCNAME((yystack_[1].value.expr), yystack_[1].location);
      (yylhs.value.expr) = new DirPIConstructor( LOC(yylhs.location), static_cast<QName*>((yystack_[1].value.expr)), SYMTAB((yystack_[0].value.sval)) );
    }
    break;

  case 575: // CDataSection: "'CDATA[['" "'char literal]]>'"
    {
      (yylhs.value.expr) = new CDataSection( LOC(yylhs.location),SYMTAB((yystack_[0].value.sval)) );
    }
    break;

  case 576: // ComputedConstructor: CompDocConstructor
    {
      (yylhs.value.expr) = (yystack_[0].value.expr);
    }
    break;

  case 577: // ComputedConstructor: CompElemConstructor
    {
      (yylhs.value.expr) = (yystack_[0].value.expr);
    }
    break;

  case 578: // ComputedConstructor: CompAttrConstructor
    {
      (yylhs.value.expr) = (yystack_[0].value.expr);
    }
    break;

  case 579: // ComputedConstructor: CompTextConstructor
    {
      (yylhs.value.expr) = (yystack_[0].value.expr);
    }
    break;

  case 580: // ComputedConstructor: CompCommentConstructor
    {
      (yylhs.value.expr) = (yystack_[0].value.expr);
    }
    break;

  case 581: // ComputedConstructor: CompPIConstructor
    {
      (yylhs.value.expr) = (yystack_[0].value.expr);
    }
    break;

  case 582: // ComputedConstructor: CompNamespaceConstructor
    { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 583: // CompDocConstructor: "'document'" "'{'" StatementsAndExpr "'}'"
    {
      (yylhs.value.expr) = new CompDocConstructor( LOC(yylhs.location), (yystack_[1].value.expr) );
    }
    break;

  case 584: // CompElemConstructor: "'element QName {'" StatementsAndOptionalExpr "'}'"
    {
      (yylhs.value.expr) = new CompElemConstructor(LOC(yylhs.location), new QName(LOC(yylhs.location), SYMTAB((yystack_[2].value.sval))), (yystack_[1].value.expr));
    }
    break;

  case 585: // CompElemConstructor: "'element EQName {'" StatementsAndOptionalExpr "'}'"
    {
      (yylhs.value.expr) = new CompElemConstructor(LOC(yylhs.location), new QName(LOC(yylhs.location), SYMTAB((yystack_[2].value.sval)), true), (yystack_[1].value.expr));
    }
    break;

  case 586: // CompElemConstructor: "'element'" "'{'" Expr "'}'" "'{'" StatementsAndOptionalExpr "'}'"
    {
      (yylhs.value.expr) = new CompElemConstructor( LOC(yylhs.location), (yystack_[4].value.expr), (yystack_[1].value.expr) );
    }
    break;

  case 587: // CompAttrConstructor: "'attribute QName {'" StatementsAndOptionalExpr "'}'"
    {
      (yylhs.value.expr) = new CompAttrConstructor( LOC(yylhs.location), new QName(LOC(yylhs.location), SYMTAB((yystack_[2].value.sval))), (yystack_[1].value.expr) );
    }
    break;

  case 588: // CompAttrConstructor: "'attribute EQName {'" StatementsAndOptionalExpr "'}'"
    {
      (yylhs.value.expr) = new CompAttrConstructor( LOC(yylhs.location), new QName(LOC(yylhs.location), SYMTAB((yystack_[2].value.sval)), true), (yystack_[1].value.expr) );
    }
    break;

  case 589: // CompAttrConstructor: "'attribute'" "'{'" Expr "'}'" "'{'" StatementsAndOptionalExpr "'}'"
    {
      (yylhs.value.expr) = new CompAttrConstructor( LOC(yylhs.location), (yystack_[4].value.expr), (yystack_[1].value.expr) );
    }
    break;

  case 590: // CompTextConstructor: "'text'" "'{'" StatementsAndExpr "'}'"
    {
      (yylhs.value.expr) = new CompTextConstructor( LOC(yylhs.location), (yystack_[1].value.expr) );
    }
    break;

  case 591: // CompCommentConstructor: "'comment'" "'{'" StatementsAndExpr "'}'"
    {
      (yylhs.value.expr) = new CompCommentConstructor( LOC(yylhs.location), (yystack_[1].value.expr) );
    }
    break;

  case 592: // CompPIConstructor: "'processing-instruction NCName {'" StatementsAndOptionalExpr "'}'"
    {
      (yylhs.value.expr) = new CompPIConstructor(LOC(yylhs.location), SYMTAB((yystack_[2].value.sval)), (yystack_[1].value.expr));
    }
    break;

  case 593: // CompPIConstructor: "'processing-instruction'" "'{'" Expr "'}'" "'{'" StatementsAndOptionalExpr "'}'"
    {
      (yylhs.value.expr) = new CompPIConstructor(LOC(yylhs.location), (yystack_[4].value.expr), (yystack_[1].value.expr));
    }
    break;

  case 594: // CompNamespaceConstructor: "'namespace NCName {'" StatementsAndExpr "'}'"
    {
      (yylhs.value.expr) = new CompNamespaceConstructor(LOC(yylhs.location), SYMTAB((yystack_[2].value.sval)), (yystack_[1].value.expr));
    }
    break;

  case 595: // CompNamespaceConstructor: "'namespace'" "'{'" Expr "'}'" "'{'" StatementsAndExpr "'}'"
    {
      (yylhs.value.expr) = new CompNamespaceConstructor(LOC(yylhs.location), (yystack_[4].value.expr), (yystack_[1].value.expr));
    }
    break;

  case 596: // TypeDeclaration: "'as'" SequenceType
    {
      (yylhs.value.node) = (yystack_[0].value.node);
    }
    break;

  case 597: // SequenceType: ItemType
        {
            (yylhs.value.node) = new SequenceTypeAST( LOC(yylhs.location), (yystack_[0].value.node), NULL );
        }
    break;

  case 598: // SequenceType: ItemType OccurrenceIndicator
        {
            (yylhs.value.node) = new SequenceTypeAST(LOC(yylhs.location), (yystack_[1].value.node), dynamic_cast<OccurrenceIndicator*>((yystack_[0].value.node)));
        }
    break;

  case 599: // SequenceType: "'empty-sequence'" "'('" "')'"
        {
            (yylhs.value.node) = new SequenceTypeAST( LOC(yylhs.location), NULL, NULL );
        }
    break;

  case 600: // SequenceType: "'('" "')'"
        {
            driver.addCommonLanguageWarning(yystack_[1].location, ZED(ZWST0009_JSONIQ_EMPTY_SEQUENCE));
            (yylhs.value.node) = new SequenceTypeAST( LOC(yylhs.location), NULL, NULL );
        }
    break;

  case 601: // OccurrenceIndicator: "'?'"
        {
            (yylhs.value.node) = new OccurrenceIndicator(
                LOC(yylhs.location), ParseConstants::occurs_optionally
            );
        }
    break;

  case 602: // OccurrenceIndicator: "'*'"
        {
            (yylhs.value.node) = new OccurrenceIndicator(
                LOC(yylhs.location), ParseConstants::occurs_zero_or_more
            );
        }
    break;

  case 603: // OccurrenceIndicator: "'+'"
        {
            (yylhs.value.node) = new OccurrenceIndicator(
                LOC(yylhs.location), ParseConstants::occurs_one_or_more
            );
        }
    break;

  case 604: // ItemType: GeneralizedAtomicType
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

  case 605: // ItemType: KindTest
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 606: // ItemType: "'item'" "'('" "')'"
        {
            (yylhs.value.node) = new ItemType( LOC(yylhs.location), true );
        }
    break;

  case 607: // ItemType: "'structured-item'" "'('" "')'"
        {
            (yylhs.value.node) = new StructuredItemType(LOC(yylhs.location));
        }
    break;

  case 608: // ItemType: FunctionTest
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 609: // ItemType: ParenthesizedItemType
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 610: // ItemType: JSONTest
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 611: // TypeList: SequenceType
        {
          TypeList* aTypeList = new TypeList(LOC (yylhs.location));
          aTypeList->push_back(dynamic_cast<SequenceTypeAST *>((yystack_[0].value.node)));
          (yylhs.value.node) = aTypeList;
        }
    break;

  case 612: // TypeList: TypeList "','" SequenceType
        {
          TypeList* aTypeList = dynamic_cast<TypeList *>((yystack_[2].value.node));
          aTypeList->push_back(dynamic_cast<SequenceTypeAST *>((yystack_[0].value.node)));
          (yylhs.value.node) = (yystack_[2].value.node);
        }
    break;

  case 613: // GeneralizedAtomicType: QNAME
    {
      (yylhs.value.node) = new GeneralizedAtomicType( LOC(yylhs.location), static_cast<QName*>((yystack_[0].value.expr)) );
    }
    break;

  case 614: // SimpleType: QNAME
    {
      (yylhs.value.node) = new SimpleType( LOC(yylhs.location), static_cast<QName*>((yystack_[0].value.expr)) );
    }
    break;

  case 615: // KindTest: DocumentTest
    {
      (yylhs.value.node) = (yystack_[0].value.node);
    }
    break;

  case 616: // KindTest: ElementTest
    {
      (yylhs.value.node) = (yystack_[0].value.node);
    }
    break;

  case 617: // KindTest: AttributeTest
    {
      (yylhs.value.node) = (yystack_[0].value.node);
    }
    break;

  case 618: // KindTest: SchemaElementTest
    {
      (yylhs.value.node) = (yystack_[0].value.node);
    }
    break;

  case 619: // KindTest: SchemaAttributeTest
    {
      (yylhs.value.node) = (yystack_[0].value.node);
    }
    break;

  case 620: // KindTest: PITest
    {
      (yylhs.value.node) = (yystack_[0].value.node);
    }
    break;

  case 621: // KindTest: CommentTest
    {
      (yylhs.value.node) = (yystack_[0].value.node);
    }
    break;

  case 622: // KindTest: TextTest
    {
      (yylhs.value.node) = (yystack_[0].value.node);
    }
    break;

  case 623: // KindTest: NamespaceTest
    {
      (yylhs.value.node) = (yystack_[0].value.node);
    }
    break;

  case 624: // KindTest: AnyKindTest
    {
      (yylhs.value.node) = (yystack_[0].value.node);
    }
    break;

  case 625: // AnyKindTest: NODE_KIND_TEST "'('" "')'"
    {
      (yylhs.value.node) = new AnyKindTest( LOC(yylhs.location) );
    }
    break;

  case 626: // NODE_KIND_TEST: "'node'"
    {
      (yylhs.value.expr) = NULL;
    }
    break;

  case 627: // DocumentTest: "'document-node'" "'('" "')'"
    {
      (yylhs.value.node) = new DocumentTest(LOC(yylhs.location));
    }
    break;

  case 628: // DocumentTest: "'document-node'" "'('" ElementTest "')'"
    {
      (yylhs.value.node) = new DocumentTest(LOC(yylhs.location), dynamic_cast<ElementTest*>((yystack_[1].value.node)));
    }
    break;

  case 629: // DocumentTest: "'document-node'" "'('" SchemaElementTest "')'"
    {
      (yylhs.value.node) = new DocumentTest(LOC(yylhs.location), dynamic_cast<SchemaElementTest*>((yystack_[1].value.node)));
    }
    break;

  case 630: // NamespaceTest: "'namespace-node'" "'('" "')'"
    {
      (yylhs.value.node) = new NamespaceTest(LOC(yylhs.location));
    }
    break;

  case 631: // TextTest: "'text'" "'('" "')'"
    {
      (yylhs.value.node) = new TextTest(LOC(yylhs.location));
    }
    break;

  case 632: // CommentTest: "'comment'" "'('" "')'"
    {
      (yylhs.value.node) = new CommentTest(LOC(yylhs.location));
    }
    break;

  case 633: // PITest: "'processing-instruction'" "'('" "')'"
        {
            (yylhs.value.node) = new PITest( LOC(yylhs.location), "" );
        }
    break;

  case 634: // PITest: "'processing-instruction'" "'('" QNAME "')'"
        {
            ERROR_IF_QNAME_NOT_NCNAME((yystack_[1].value.expr), yystack_[1].location);
            (yylhs.value.node) = new PITest( LOC(yylhs.location), static_cast<QName*>((yystack_[1].value.expr)) );
        }
    break;

  case 635: // PITest: "'processing-instruction'" "'('" "'STRING'" "')'"
        {
            (yylhs.value.node) = new PITest( LOC(yylhs.location), SYMTAB((yystack_[1].value.sval)) );
        }
    break;

  case 636: // AttributeTest: "'attribute'" "'('" "')'"
        {
            (yylhs.value.node) = new AttributeTest( LOC(yylhs.location), NULL, NULL );
        }
    break;

  case 637: // AttributeTest: "'attribute'" "'('" QNAME "')'"
        {
            (yylhs.value.node) = new AttributeTest(
                LOC(yylhs.location), static_cast<QName*>((yystack_[1].value.expr)), NULL
            );
        }
    break;

  case 638: // AttributeTest: "'attribute'" "'('" QNAME "','" TypeName "')'"
        {
            (yylhs.value.node) = new AttributeTest(
                LOC(yylhs.location), static_cast<QName*>((yystack_[3].value.expr)), dynamic_cast<TypeName*>((yystack_[1].value.node))
            );
        }
    break;

  case 639: // AttributeTest: "'attribute'" "'('" "'*'" "')'"
        {
            (yylhs.value.node) = new AttributeTest( LOC(yylhs.location), NULL, NULL );
        }
    break;

  case 640: // AttributeTest: "'attribute'" "'('" "'*'" "','" TypeName "')'"
        {
            (yylhs.value.node) = new AttributeTest(
                LOC(yylhs.location), NULL, dynamic_cast<TypeName*>((yystack_[1].value.node))
            );
        }
    break;

  case 641: // SchemaAttributeTest: "'schema-attribute'" "'('" QNAME "')'"
        {
            (yylhs.value.node) = new SchemaAttributeTest( LOC(yylhs.location), static_cast<QName*>((yystack_[1].value.expr)) );
        }
    break;

  case 642: // ElementTest: "'element'" "'('" "')'"
        {
            (yylhs.value.node) = new ElementTest( LOC(yylhs.location), NULL, NULL, true );
        }
    break;

  case 643: // ElementTest: "'element'" "'('" QNAME "')'"
        {
            (yylhs.value.node) = new ElementTest(
                LOC(yylhs.location), static_cast<QName*>((yystack_[1].value.expr)), NULL, true
            );
        }
    break;

  case 644: // ElementTest: "'element'" "'('" QNAME "','" TypeName "')'"
        {
            (yylhs.value.node) = new ElementTest(
                LOC(yylhs.location),
                static_cast<QName*>((yystack_[3].value.expr)),
                dynamic_cast<TypeName*>((yystack_[1].value.node)),
                false
            );
        }
    break;

  case 645: // ElementTest: "'element'" "'('" QNAME "','" TypeName_WITH_HOOK "')'"
        {
            (yylhs.value.node) = new ElementTest(
                LOC(yylhs.location),
                static_cast<QName*>((yystack_[3].value.expr)),
                dynamic_cast<TypeName*>((yystack_[1].value.node)),
                true
            );
        }
    break;

  case 646: // ElementTest: "'element'" "'('" "'*'" "')'"
        {
            (yylhs.value.node) = new ElementTest(
                LOC(yylhs.location), NULL, NULL, true
            );
        }
    break;

  case 647: // ElementTest: "'element'" "'('" "'*'" "','" TypeName "')'"
        {
            (yylhs.value.node) = new ElementTest(
                LOC(yylhs.location), NULL, dynamic_cast<TypeName*>((yystack_[1].value.node)), false
            );
        }
    break;

  case 648: // ElementTest: "'element'" "'('" "'*'" "','" TypeName_WITH_HOOK "')'"
        {
            (yylhs.value.node) = new ElementTest(
                LOC(yylhs.location), NULL, dynamic_cast<TypeName*>((yystack_[1].value.node)), true
            );
        }
    break;

  case 649: // SchemaElementTest: "'schema-element'" "'('" QNAME "')'"
        {
            (yylhs.value.node) = new SchemaElementTest( LOC(yylhs.location), static_cast<QName*>((yystack_[1].value.expr)) );
        }
    break;

  case 650: // TypeName: QNAME
        {
            (yylhs.value.node) = new TypeName( LOC(yylhs.location), static_cast<QName*>((yystack_[0].value.expr)) );
        }
    break;

  case 651: // TypeName_WITH_HOOK: QNAME "'?'"
        {
            (yylhs.value.node) = new TypeName( LOC(yylhs.location), static_cast<QName*>((yystack_[1].value.expr)), true );
        }
    break;

  case 652: // StringLiteral: "'STRING'"
        {
            (yylhs.value.expr) = new StringLiteral( LOC(yylhs.location), SYMTAB((yystack_[0].value.sval)) );
        }
    break;

  case 653: // FunctionTest: AnyFunctionTest
    {
      (yylhs.value.node) = (yystack_[0].value.node);
    }
    break;

  case 654: // FunctionTest: TypedFunctionTest
    {
      (yylhs.value.node) = (yystack_[0].value.node);
    }
    break;

  case 655: // FunctionTest: AnnotationList AnyFunctionTest
    {
      static_cast<AnyFunctionTest*>((yystack_[0].value.node))->
      setAnnotations(static_cast<AnnotationListParsenode*>((yystack_[1].value.node)));
      (yylhs.value.node) = (yystack_[0].value.node);
    }
    break;

  case 656: // FunctionTest: AnnotationList TypedFunctionTest
    {
      static_cast<TypedFunctionTest*>((yystack_[0].value.node))->
      setAnnotations(static_cast<AnnotationListParsenode*>((yystack_[1].value.node)));
      (yylhs.value.node) = (yystack_[0].value.node);
    }
    break;

  case 657: // AnyFunctionTest: "'function'" "'('" "'*'" "')'"
    {
      (yylhs.value.node) = new AnyFunctionTest(LOC(yylhs.location));
    }
    break;

  case 658: // TypedFunctionTest: "'function'" "'('" "')'" "'as'" SequenceType
        {
          (yylhs.value.node) = new TypedFunctionTest(LOC (yylhs.location), dynamic_cast<SequenceTypeAST *>((yystack_[0].value.node)));
        }
    break;

  case 659: // TypedFunctionTest: "'function'" "'('" TypeList "')'" "'as'" SequenceType
        {
          (yylhs.value.node) = new TypedFunctionTest(LOC (yylhs.location),
              dynamic_cast<TypeList *>((yystack_[3].value.node)),
              dynamic_cast<SequenceTypeAST *>((yystack_[0].value.node)));
        }
    break;

  case 660: // ParenthesizedItemType: "'('" ItemType "')'"
        {
           (yylhs.value.node) = (yystack_[1].value.node);
        }
    break;

  case 661: // RevalidationDecl: "'declare'" "'revalidation'" "'strict'"
        {
            (yylhs.value.node) = new RevalidationDecl(
                LOC(yylhs.location), StaticContextConsts::strict_validation
            );
        }
    break;

  case 662: // RevalidationDecl: "'declare'" "'revalidation'" "'lax'"
        {
            (yylhs.value.node) = new RevalidationDecl(
                LOC(yylhs.location), StaticContextConsts::lax_validation
            );
        }
    break;

  case 663: // RevalidationDecl: "'declare'" "'revalidation'" "'skip'"
        {
            (yylhs.value.node) = new RevalidationDecl(
                LOC(yylhs.location), StaticContextConsts::skip_validation
            );
        }
    break;

  case 664: // InsertExpr: "'insert'" "'node'" ExprSingle "'into'" ExprSingle
        {
            (yylhs.value.expr) = new InsertExpr( LOC(yylhs.location), store::UpdateConsts::INTO, (yystack_[2].value.expr), (yystack_[0].value.expr) );
        }
    break;

  case 665: // InsertExpr: "'insert'" "'node'" ExprSingle "'as'" "'first'" "'into'" ExprSingle
        {
            (yylhs.value.expr) = new InsertExpr(
                LOC(yylhs.location), store::UpdateConsts::AS_FIRST_INTO, (yystack_[4].value.expr), (yystack_[0].value.expr)
            );
        }
    break;

  case 666: // InsertExpr: "'insert'" "'node'" ExprSingle "'as'" "'last'" "'into'" ExprSingle
        {
            (yylhs.value.expr) = new InsertExpr(
                LOC(yylhs.location), store::UpdateConsts::AS_LAST_INTO, (yystack_[4].value.expr), (yystack_[0].value.expr)
            );
        }
    break;

  case 667: // InsertExpr: "'insert'" "'node'" ExprSingle "'after'" ExprSingle
        {
            (yylhs.value.expr) = new InsertExpr( LOC(yylhs.location), store::UpdateConsts::AFTER, (yystack_[2].value.expr), (yystack_[0].value.expr) );
        }
    break;

  case 668: // InsertExpr: "'insert'" "'node'" ExprSingle "'before'" ExprSingle
        {
            (yylhs.value.expr) = new InsertExpr(
                LOC (yylhs.location), store::UpdateConsts::BEFORE, (yystack_[2].value.expr), (yystack_[0].value.expr)
            );
        }
    break;

  case 669: // InsertExpr: "'insert'" "'nodes'" ExprSingle "'into'" ExprSingle
        {
            (yylhs.value.expr) = new InsertExpr( LOC(yylhs.location), store::UpdateConsts::INTO, (yystack_[2].value.expr), (yystack_[0].value.expr) );
        }
    break;

  case 670: // InsertExpr: "'insert'" "'nodes'" ExprSingle "'as'" "'first'" "'into'" ExprSingle
        {
            (yylhs.value.expr) = new InsertExpr(
                LOC(yylhs.location), store::UpdateConsts::AS_FIRST_INTO, (yystack_[4].value.expr), (yystack_[0].value.expr)
            );
        }
    break;

  case 671: // InsertExpr: "'insert'" "'nodes'" ExprSingle "'as'" "'last'" "'into'" ExprSingle
        {
            (yylhs.value.expr) = new InsertExpr(
                  LOC(yylhs.location), store::UpdateConsts::AS_LAST_INTO, (yystack_[4].value.expr), (yystack_[0].value.expr)
            );
        }
    break;

  case 672: // InsertExpr: "'insert'" "'nodes'" ExprSingle "'after'" ExprSingle
        {
            (yylhs.value.expr) = new InsertExpr(
                LOC (yylhs.location),
                store::UpdateConsts::AFTER, (yystack_[2].value.expr), (yystack_[0].value.expr)
            );
        }
    break;

  case 673: // InsertExpr: "'insert'" "'nodes'" ExprSingle "'before'" ExprSingle
        {
            (yylhs.value.expr) = new InsertExpr(
                LOC (yylhs.location), store::UpdateConsts::BEFORE, (yystack_[2].value.expr), (yystack_[0].value.expr)
            );
        }
    break;

  case 674: // DeleteExpr: "'delete'" "'node'" ExprSingle
        {
            (yylhs.value.expr) = new DeleteExpr( LOC (yylhs.location), (yystack_[0].value.expr) );
        }
    break;

  case 675: // DeleteExpr: "'delete'" "'nodes'" ExprSingle
        {
            (yylhs.value.expr) = new DeleteExpr( LOC (yylhs.location), (yystack_[0].value.expr) );
        }
    break;

  case 676: // ReplaceExpr: "'replace'" "'node'" ExprSingle "'with'" ExprSingle
        {
            (yylhs.value.expr) = new ReplaceExpr(
                LOC(yylhs.location), store::UpdateConsts::NODE, (yystack_[2].value.expr), (yystack_[0].value.expr)
            );
        }
    break;

  case 677: // ReplaceExpr: "'replace'" "'value'" "'of'" "'node'" ExprSingle "'with'" ExprSingle
        {
            (yylhs.value.expr) = new ReplaceExpr(
                LOC(yylhs.location), store::UpdateConsts::VALUE_OF_NODE, (yystack_[2].value.expr), (yystack_[0].value.expr)
            );
        }
    break;

  case 678: // RenameExpr: "'rename'" "'node'" ExprSingle "'as'" ExprSingle
        {
            (yylhs.value.expr) = new RenameExpr( LOC (yylhs.location), (yystack_[2].value.expr), (yystack_[0].value.expr) );
        }
    break;

  case 679: // TransformExpr: "'copy'" "'$'" VarNameList "'modify'" ExprSingle ReturnOrSelect ExprSingle
    {
      CopyVarList *cvl = dynamic_cast<CopyVarList*>((yystack_[4].value.expr));
      (yylhs.value.expr) = new TransformExpr( LOC(yylhs.location), cvl, (yystack_[2].value.expr), (yystack_[0].value.expr) );
    }
    break;

  case 680: // VarNameList: VarNameDecl
    {
      CopyVarList* lList = new CopyVarList(LOC(yylhs.location));
      lList->push_back (dynamic_cast<VarBinding*> ((yystack_[0].value.expr)));
      (yylhs.value.expr) = lList;
    }
    break;

  case 681: // VarNameList: VarNameList "','" "'$'" VarNameDecl
    {
      CopyVarList* lList = dynamic_cast<CopyVarList*>((yystack_[3].value.expr));
      VarBinding* lBinding = dynamic_cast<VarBinding*>((yystack_[0].value.expr));
      lList->push_back(lBinding);
      (yylhs.value.expr) = lList;
    }
    break;

  case 682: // VarNameDecl: QNAME "':='" ExprSingle
    {
       (yylhs.value.expr) = new VarBinding(LOC(yylhs.location), static_cast<QName*>((yystack_[2].value.expr)), (yystack_[0].value.expr));
    }
    break;

  case 683: // TryExpr: "'try'" "'{'" Expr "'}'" CatchListExpr
        {
            (yylhs.value.expr) = new TryExpr( LOC(yylhs.location), (yystack_[2].value.expr), (yystack_[0].value.expr) );
        }
    break;

  case 684: // CatchListExpr: CatchExpr
        {
            CatchListExpr *cle = new CatchListExpr( LOC(yylhs.location) );
            cle->push_back( static_cast<CatchExpr*>((yystack_[0].value.expr)) );
            (yylhs.value.expr) = cle;
        }
    break;

  case 685: // CatchListExpr: CatchListExpr CatchExpr
        {
            CatchListExpr *cle = dynamic_cast<CatchListExpr*>((yystack_[1].value.expr));
            if ( cle )
                cle->push_back( static_cast<CatchExpr*>((yystack_[0].value.expr)) );
            (yylhs.value.expr) = (yystack_[1].value.expr);
        }
    break;

  case 686: // CatchExpr: "'catch'" NameTestList BracedExpr
    {
       (yylhs.value.expr) = new CatchExpr(LOC(yylhs.location), *(yystack_[1].value.name_test_list), (yystack_[0].value.expr));
       delete (yystack_[1].value.name_test_list);
    }
    break;

  case 687: // BracedExpr: "'{'" Expr "'}'"
    {
      (yylhs.value.expr) = (yystack_[1].value.expr);
    }
    break;

  case 688: // NameTestList: NameTest
        {
            CatchExpr::NameTestList *ntl = new CatchExpr::NameTestList;
            ntl->push_back( static_cast<NameTest*>((yystack_[0].value.node)) );
            (yylhs.value.name_test_list) = ntl;
        }
    break;

  case 689: // NameTestList: NameTestList "'|'" NameTest
        {
            CatchExpr::NameTestList *ntl =
                static_cast<CatchExpr::NameTestList*>((yystack_[2].value.name_test_list));
            ntl->push_back( static_cast<NameTest*>((yystack_[0].value.node)) );
            (yylhs.value.name_test_list) = ntl;
        }
    break;

  case 690: // FTSelection: FTOr opt_FTPosFilter_list
        {
            (yylhs.value.node) = new FTSelection( LOC(yylhs.location), (yystack_[1].value.node), (yystack_[0].value.pos_filter_list) );
            delete (yystack_[0].value.pos_filter_list);
        }
    break;

  case 691: // opt_FTPosFilter_list: %empty
        {
            (yylhs.value.pos_filter_list) = NULL;
        }
    break;

  case 692: // opt_FTPosFilter_list: FTPosFilter_list
        {
            (yylhs.value.pos_filter_list) = (yystack_[0].value.pos_filter_list);
        }
    break;

  case 693: // FTPosFilter_list: FTPosFilter
        {
            (yylhs.value.pos_filter_list) = new FTSelection::pos_filter_list_t;
            (yylhs.value.pos_filter_list)->push_back( dynamic_cast<FTPosFilter*>((yystack_[0].value.node)) );
        }
    break;

  case 694: // FTPosFilter_list: FTPosFilter_list FTPosFilter
        {
            (yystack_[1].value.pos_filter_list)->push_back( dynamic_cast<FTPosFilter*>((yystack_[0].value.node)) );
            (yylhs.value.pos_filter_list) = (yystack_[1].value.pos_filter_list);
        }
    break;

  case 695: // FTOr: FTAnd
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 696: // FTOr: FTOr "'ftor'" FTAnd
        {
            (yylhs.value.node) = new FTOr( LOC(yylhs.location), (yystack_[2].value.node), (yystack_[0].value.node) );
        }
    break;

  case 697: // FTAnd: FTMildNot
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 698: // FTAnd: FTAnd "'ftand'" FTMildNot
        {
            (yylhs.value.node) = new FTAnd( LOC(yylhs.location), (yystack_[2].value.node), (yystack_[0].value.node) );
        }
    break;

  case 699: // FTMildNot: FTUnaryNot
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 700: // FTMildNot: FTMildNot "'not'" "'in'" FTUnaryNot
        {
            (yylhs.value.node) = new FTMildNot( LOC(yylhs.location), (yystack_[3].value.node), (yystack_[0].value.node) );
        }
    break;

  case 701: // FTUnaryNot: FTPrimaryWithOptions
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 702: // FTUnaryNot: "'ftnot'" FTPrimaryWithOptions
        {
            (yylhs.value.node) = new FTUnaryNot(
                LOC(yylhs.location), dynamic_cast<FTPrimaryWithOptions*>((yystack_[0].value.node))
            );
        }
    break;

  case 703: // FTPrimaryWithOptions: FTPrimary opt_FTMatchOptions opt_FTWeight
        {
            (yylhs.value.node) = new FTPrimaryWithOptions(
                LOC(yylhs.location),
                dynamic_cast<FTPrimary*>((yystack_[2].value.node)),
                dynamic_cast<FTMatchOptions*>((yystack_[1].value.node)),
                dynamic_cast<FTWeight*>((yystack_[0].value.node))
            );
        }
    break;

  case 704: // opt_FTMatchOptions: %empty
        {
            (yylhs.value.node) = NULL;
        }
    break;

  case 705: // opt_FTMatchOptions: FTMatchOptions
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 706: // opt_FTWeight: %empty
        {
            (yylhs.value.node) = NULL;
        }
    break;

  case 707: // opt_FTWeight: FTWeight
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 708: // FTWeight: "'weight'" "'{'" Expr "'}'"
        {
            (yylhs.value.node) = new FTWeight( LOC(yylhs.location), dynamic_cast<exprnode*>((yystack_[1].value.expr)) );
        }
    break;

  case 709: // FTPrimary: FTWords opt_FTTimes
        {
            (yylhs.value.node) = new FTWordsTimes(
                LOC(yylhs.location),
                dynamic_cast<FTWords*>((yystack_[1].value.node)),
                dynamic_cast<FTTimes*>((yystack_[0].value.node))
            );
        }
    break;

  case 710: // FTPrimary: "'('" FTSelection "')'"
        {
            (yylhs.value.node) = (yystack_[1].value.node);
        }
    break;

  case 711: // FTPrimary: FTExtensionSelection
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 712: // opt_FTTimes: %empty
        {
            (yylhs.value.node) = NULL;
        }
    break;

  case 713: // opt_FTTimes: FTTimes
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 714: // FTExtensionSelection: Pragma_list "'{'" opt_FTSelection "'}'"
        {
            (yylhs.value.node) = new FTExtensionSelection(
                LOC(yylhs.location),
                dynamic_cast<PragmaList*>((yystack_[3].value.node)),
                dynamic_cast<FTSelection*>((yystack_[1].value.node))
            );
        }
    break;

  case 715: // opt_FTSelection: %empty
        {
            (yylhs.value.node) = NULL;
        }
    break;

  case 716: // opt_FTSelection: FTSelection
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 717: // FTWords: FTWordsValue opt_FTAnyallOption
        {
            (yylhs.value.node) = new FTWords(
                LOC(yylhs.location),
                dynamic_cast<FTWordsValue*>((yystack_[1].value.node)),
                dynamic_cast<FTAnyallOption*>((yystack_[0].value.node))
            );
        }
    break;

  case 718: // FTWordsValue: StringLiteral
        {
            (yylhs.value.node) = new FTWordsValue(
                LOC(yylhs.location), static_cast<StringLiteral*>((yystack_[0].value.expr)), NULL
            );
        }
    break;

  case 719: // FTWordsValue: "'{'" Expr "'}'"
        {
            (yylhs.value.node) = new FTWordsValue(
                LOC(yylhs.location), NULL, dynamic_cast<exprnode*>((yystack_[1].value.expr))
            );
        }
    break;

  case 720: // opt_FTAnyallOption: %empty
        {
            (yylhs.value.node) = new FTAnyallOption( LOC(yylhs.location), ft_anyall_mode::any );
        }
    break;

  case 721: // opt_FTAnyallOption: FTAnyallOption
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 722: // FTAnyallOption: "'any'" opt_word
        {
            (yylhs.value.node) = new FTAnyallOption( LOC(yylhs.location), (yystack_[0].value.ft_anyall_value) );
        }
    break;

  case 723: // FTAnyallOption: "'all'" opt_words
        {
            (yylhs.value.node) = new FTAnyallOption( LOC(yylhs.location), (yystack_[0].value.ft_anyall_value) );
        }
    break;

  case 724: // FTAnyallOption: "'phrase'"
        {
            (yylhs.value.node) = new FTAnyallOption( LOC(yylhs.location), ft_anyall_mode::phrase );
        }
    break;

  case 725: // opt_word: %empty
        {
            (yylhs.value.ft_anyall_value) = ft_anyall_mode::any;
        }
    break;

  case 726: // opt_word: "'word'"
        {
            (yylhs.value.ft_anyall_value) = ft_anyall_mode::any_word;
        }
    break;

  case 727: // opt_words: %empty
        {
            (yylhs.value.ft_anyall_value) = ft_anyall_mode::all;
        }
    break;

  case 728: // opt_words: "'words'"
        {
            (yylhs.value.ft_anyall_value) = ft_anyall_mode::all_words;
        }
    break;

  case 729: // FTPosFilter: FTOrder
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 730: // FTPosFilter: FTWindow
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 731: // FTPosFilter: FTDistance
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 732: // FTPosFilter: FTScope
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 733: // FTPosFilter: FTContent
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 734: // FTOrder: "'ordered'"
        {
            (yylhs.value.node) = new FTOrder( LOC(yylhs.location) );
        }
    break;

  case 735: // FTWindow: "'window'" AdditiveExpr FTUnit
        {
            (yylhs.value.node) = new FTWindow(
                LOC(yylhs.location),
                (yystack_[1].value.expr),
                static_cast<FTUnit*>((yystack_[0].value.node))
            );
        }
    break;

  case 736: // FTDistance: "'distance'" FTRange FTUnit
        {
            (yylhs.value.node) = new FTDistance(
                LOC(yylhs.location),
                dynamic_cast<FTRange*>((yystack_[1].value.node)),
                dynamic_cast<FTUnit*>((yystack_[0].value.node))
            );
        }
    break;

  case 737: // FTUnit: "'words'"
        {
            (yylhs.value.node) = new FTUnit( LOC(yylhs.location), ft_unit::words );
        }
    break;

  case 738: // FTUnit: "'sentences'"
        {
            (yylhs.value.node) = new FTUnit( LOC(yylhs.location), ft_unit::sentences );
        }
    break;

  case 739: // FTUnit: "'paragraphs'"
        {
            (yylhs.value.node) = new FTUnit( LOC(yylhs.location), ft_unit::paragraphs );
        }
    break;

  case 740: // FTMatchOptions: "'using'" FTMatchOption
        {
            FTMatchOptions *mo = new FTMatchOptions( LOC(yylhs.location) );
            mo->push_back( dynamic_cast<FTMatchOption*>((yystack_[0].value.node)) );
            (yylhs.value.node) = mo;
        }
    break;

  case 741: // FTMatchOptions: FTMatchOptions "'using'" FTMatchOption
        {
            FTMatchOptions *mo = dynamic_cast<FTMatchOptions*>((yystack_[2].value.node));
            mo->push_back( dynamic_cast<FTMatchOption*>((yystack_[0].value.node)) );
            (yylhs.value.node) = (yystack_[2].value.node);
        }
    break;

  case 742: // FTMatchOption: FTCaseOption
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 743: // FTMatchOption: FTDiacriticsOption
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 744: // FTMatchOption: FTExtensionOption
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 745: // FTMatchOption: FTLanguageOption
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 746: // FTMatchOption: FTStemOption
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 747: // FTMatchOption: FTStopWordOption
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 748: // FTMatchOption: FTThesaurusOption
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 749: // FTMatchOption: FTWildCardOption
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 750: // FTCaseOption: "'case'" "'sensitive'"
        {
            (yylhs.value.node) = new FTCaseOption( LOC(yylhs.location), ft_case_mode::sensitive );
        }
    break;

  case 751: // FTCaseOption: "'case'" "'insensitive'"
        {
            (yylhs.value.node) = new FTCaseOption( LOC(yylhs.location), ft_case_mode::insensitive );
        }
    break;

  case 752: // FTCaseOption: "'lowercase'"
        {
            (yylhs.value.node) = new FTCaseOption( LOC(yylhs.location), ft_case_mode::lower );
        }
    break;

  case 753: // FTCaseOption: "'uppercase'"
        {
            (yylhs.value.node) = new FTCaseOption( LOC(yylhs.location), ft_case_mode::upper );
        }
    break;

  case 754: // FTDiacriticsOption: "'diacritics'" "'sensitive'"
        {
            (yylhs.value.node) = new FTDiacriticsOption(
                LOC(yylhs.location), ft_diacritics_mode::sensitive
            );
        }
    break;

  case 755: // FTDiacriticsOption: "'diacritics'" "'insensitive'"
        {
            (yylhs.value.node) = new FTDiacriticsOption(
                LOC(yylhs.location), ft_diacritics_mode::insensitive
            );
        }
    break;

  case 756: // FTExtensionOption: "'option'" QNAME "'STRING'"
        {
            (yylhs.value.node) = new FTExtensionOption(
                LOC(yylhs.location), static_cast<QName*>((yystack_[1].value.expr)), SYMTAB((yystack_[0].value.sval))
            );
        }
    break;

  case 757: // FTStemOption: "'stemming'"
        {
            (yylhs.value.node) = new FTStemOption( LOC(yylhs.location), ft_stem_mode::stemming );
        }
    break;

  case 758: // FTStemOption: "'no'" "'stemming'"
        {
            (yylhs.value.node) = new FTStemOption( LOC(yylhs.location), ft_stem_mode::no_stemming );
        }
    break;

  case 759: // FTThesaurusOption: "'thesaurus'" FTThesaurusID_or_default
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

  case 760: // FTThesaurusOption: "'thesaurus'" "'('" FTThesaurusID_or_default opt_FTThesaurus_list "')'"
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

  case 761: // FTThesaurusOption: "'no'" "'thesaurus'"
        {
            (yylhs.value.node) = new FTThesaurusOption( LOC(yylhs.location), NULL, false, true );
        }
    break;

  case 762: // FTThesaurusID_or_default: FTThesaurusID
        {
            (yylhs.value.node) = (yystack_[0].value.node);
        }
    break;

  case 763: // FTThesaurusID_or_default: "'default'"
        {
            (yylhs.value.node) = NULL;
        }
    break;

  case 764: // opt_FTThesaurus_list: %empty
        {
            (yylhs.value.thesaurus_id_list) = NULL;
        }
    break;

  case 765: // opt_FTThesaurus_list: "','" FTThesaurus_list
        {
            (yylhs.value.thesaurus_id_list) = (yystack_[0].value.thesaurus_id_list);
        }
    break;

  case 766: // FTThesaurus_list: FTThesaurusID
        {
            (yylhs.value.thesaurus_id_list) = new FTThesaurusOption::thesaurus_id_list_t;
            (yylhs.value.thesaurus_id_list)->push_back( dynamic_cast<FTThesaurusID*>((yystack_[0].value.node)) );
        }
    break;

  case 767: // FTThesaurus_list: FTThesaurus_list "','" FTThesaurusID
        {
            (yystack_[2].value.thesaurus_id_list)->push_back( dynamic_cast<FTThesaurusID*>((yystack_[0].value.node)) );
            (yylhs.value.thesaurus_id_list) = (yystack_[2].value.thesaurus_id_list);
        }
    break;

  case 768: // FTThesaurusID: "'at'" "'STRING'" opt_relationship opt_levels
        {
            (yylhs.value.node) = new FTThesaurusID(
                LOC(yylhs.location), SYMTAB((yystack_[2].value.sval)), SYMTAB((yystack_[1].value.sval)), dynamic_cast<FTRange*>((yystack_[0].value.node))
            );
        }
    break;

  case 769: // opt_relationship: %empty
        {
            (yylhs.value.sval) = 0;
        }
    break;

  case 770: // opt_relationship: "'relationship'" "'STRING'"
        {
            (yylhs.value.sval) = (yystack_[0].value.sval);
        }
    break;

  case 771: // opt_levels: %empty
        {
            (yylhs.value.node) = NULL;
        }
    break;

  case 772: // opt_levels: FTRange "'levels'"
        {
            (yylhs.value.node) = (yystack_[1].value.node);
        }
    break;

  case 773: // FTStopWordOption: "'stop'" "'words'" FTStopWords opt_FTStopWordsInclExcl_list
        {
            (yylhs.value.node) = new FTStopWordOption(
                LOC(yylhs.location),
                dynamic_cast<FTStopWords*>((yystack_[1].value.node)), (yystack_[0].value.incl_excl_list),
                ft_stop_words_mode::with
            );
            delete (yystack_[0].value.incl_excl_list);
        }
    break;

  case 774: // FTStopWordOption: "'stop'" "'words'" "'default'" opt_FTStopWordsInclExcl_list
        {
            (yylhs.value.node) = new FTStopWordOption(
                LOC(yylhs.location), NULL, (yystack_[0].value.incl_excl_list), ft_stop_words_mode::with_default
            );
            delete (yystack_[0].value.incl_excl_list);
        }
    break;

  case 775: // FTStopWordOption: "'no'" "'stop'" "'words'"
        {
            (yylhs.value.node) = new FTStopWordOption(
                LOC(yylhs.location), NULL, NULL, ft_stop_words_mode::without
            );
        }
    break;

  case 776: // FTStopWords: "'at'" "'STRING'"
        {
            (yylhs.value.node) = new FTStopWords( LOC(yylhs.location), SYMTAB((yystack_[0].value.sval)), NULL );
        }
    break;

  case 777: // FTStopWords: "'('" STRING_LITERAL_list "')'"
        {
            (yylhs.value.node) = new FTStopWords( LOC(yylhs.location), "", (yystack_[1].value.strlist) );
        }
    break;

  case 778: // STRING_LITERAL_list: "'STRING'"
        {
            string_list_t *sl = new string_list_t;
            sl->push_back( SYMTAB((yystack_[0].value.sval)) );
            (yylhs.value.strlist) = sl;
        }
    break;

  case 779: // STRING_LITERAL_list: STRING_LITERAL_list "','" "'STRING'"
        {
            if ( (yystack_[2].value.strlist) )
                (yystack_[2].value.strlist)->push_back( SYMTAB((yystack_[0].value.sval)) );
            (yylhs.value.strlist) = (yystack_[2].value.strlist);
        }
    break;

  case 780: // opt_FTStopWordsInclExcl_list: %empty
        {
            (yylhs.value.incl_excl_list) = NULL;
        }
    break;

  case 781: // opt_FTStopWordsInclExcl_list: FTStopWordsInclExcl_list
        {
            (yylhs.value.incl_excl_list) = (yystack_[0].value.incl_excl_list);
        }
    break;

  case 782: // FTStopWordsInclExcl_list: FTStopWordsInclExcl
        {
            (yylhs.value.incl_excl_list) = new FTStopWordOption::incl_excl_list_t;
            (yylhs.value.incl_excl_list)->push_back( dynamic_cast<FTStopWordsInclExcl*>((yystack_[0].value.node)) );
        }
    break;

  case 783: // FTStopWordsInclExcl_list: FTStopWordsInclExcl_list FTStopWordsInclExcl
        {
            FTStopWordOption::incl_excl_list_t *iel = (yystack_[1].value.incl_excl_list);
            if ( !iel )
                iel = new FTStopWordOption::incl_excl_list_t;
            iel->push_back( dynamic_cast<FTStopWordsInclExcl*>((yystack_[0].value.node)) );
            (yylhs.value.incl_excl_list) = iel;
        }
    break;

  case 784: // FTStopWordsInclExcl: "'union'" FTStopWords
        {
            (yylhs.value.node) = new FTStopWordsInclExcl(
                LOC(yylhs.location),
                dynamic_cast<FTStopWords*>((yystack_[0].value.node)),
                ft_stop_words_unex::union_
            );
        }
    break;

  case 785: // FTStopWordsInclExcl: "'except'" FTStopWords
        {
            (yylhs.value.node) = new FTStopWordsInclExcl(
                LOC(yylhs.location),
                dynamic_cast<FTStopWords*>((yystack_[0].value.node)),
                ft_stop_words_unex::except
            );
        }
    break;

  case 786: // FTLanguageOption: "'language'" "'STRING'"
        {
            (yylhs.value.node) = new FTLanguageOption( LOC(yylhs.location), SYMTAB((yystack_[0].value.sval)) );
        }
    break;

  case 787: // FTWildCardOption: "'wildcards'"
        {
            (yylhs.value.node) = new FTWildCardOption( LOC(yylhs.location), ft_wild_card_mode::with );
        }
    break;

  case 788: // FTWildCardOption: "'no'" "'wildcards'"
        {
            (yylhs.value.node) = new FTWildCardOption( LOC(yylhs.location), ft_wild_card_mode::without );
        }
    break;

  case 789: // FTContent: "'at'" "'start'"
        {
            (yylhs.value.node) = new FTContent( LOC(yylhs.location), ft_content_mode::at_start );
        }
    break;

  case 790: // FTContent: "'at'" "'end'"
        {
            (yylhs.value.node) = new FTContent( LOC(yylhs.location), ft_content_mode::at_end );
        }
    break;

  case 791: // FTContent: "'entire'" "'content'"
        {
            (yylhs.value.node) = new FTContent( LOC(yylhs.location), ft_content_mode::entire );
        }
    break;

  case 792: // FTTimes: "'occurs'" FTRange "'times'"
        {
            (yylhs.value.node) = new FTTimes( LOC(yylhs.location), dynamic_cast<FTRange*>((yystack_[1].value.node)) );
        }
    break;

  case 793: // FTRange: "'exactly'" AdditiveExpr
        {
            (yylhs.value.node) = new FTRange( LOC(yylhs.location), ft_range_mode::exactly, (yystack_[0].value.expr) );
        }
    break;

  case 794: // FTRange: "'at'" "'least'" AdditiveExpr
        {
            (yylhs.value.node) = new FTRange( LOC(yylhs.location), ft_range_mode::at_least, (yystack_[0].value.expr) );
        }
    break;

  case 795: // FTRange: "'at'" "'most'" AdditiveExpr
        {
            (yylhs.value.node) = new FTRange( LOC(yylhs.location), ft_range_mode::at_most, (yystack_[0].value.expr) );
        }
    break;

  case 796: // FTRange: "'from'" AdditiveExpr "'to'" AdditiveExpr
        {
            (yylhs.value.node) = new FTRange( LOC(yylhs.location), ft_range_mode::from_to, (yystack_[2].value.expr), (yystack_[0].value.expr) );
        }
    break;

  case 797: // FTScope: "'same'" FTBigUnit
        {
            (yylhs.value.node) = new FTScope(
                LOC(yylhs.location),
                ft_scope::same,
                dynamic_cast<FTBigUnit*>((yystack_[0].value.node))
            );
        }
    break;

  case 798: // FTScope: "'different'" FTBigUnit
        {
            (yylhs.value.node) = new FTScope(
                LOC(yylhs.location),
                ft_scope::different,
                dynamic_cast<FTBigUnit*>((yystack_[0].value.node))
            );
        }
    break;

  case 799: // FTBigUnit: "'sentence'"
        {
            (yylhs.value.node) = new FTBigUnit( LOC(yylhs.location), ft_big_unit::sentence );
        }
    break;

  case 800: // FTBigUnit: "'paragraph'"
        {
            (yylhs.value.node) = new FTBigUnit( LOC(yylhs.location), ft_big_unit::paragraph );
        }
    break;

  case 801: // FTIgnoreOption: "'without'" "'content'" UnionExpr
        {
            (yylhs.value.node) = new FTIgnoreOption( LOC(yylhs.location), (yystack_[0].value.expr) );
        }
    break;

  case 802: // JSONArrayConstructor: "'['" "']'"
        {
          (yylhs.value.expr) = new JSONArrayConstructor( LOC(yylhs.location), NULL );
        }
    break;

  case 803: // JSONArrayConstructor: "'['" Expr "']'"
        {
          (yylhs.value.expr) = new JSONArrayConstructor( LOC(yylhs.location), (yystack_[1].value.expr) );
        }
    break;

  case 804: // JSONSimpleObjectUnion: "'{|'" "'|}'"
        {
          (yylhs.value.expr) = new JSONObjectConstructor(LOC(yylhs.location), NULL, false);
        }
    break;

  case 805: // JSONSimpleObjectUnion: "'{|'" Expr "'|}'"
        {
          (yylhs.value.expr) = new JSONObjectConstructor(LOC(yylhs.location), (yystack_[1].value.expr), false);
        }
    break;

  case 806: // JSONAccumulatorObjectUnion: "'{['" "']}'"
        {
          (yylhs.value.expr) = new JSONObjectConstructor(LOC(yylhs.location), NULL, true);
        }
    break;

  case 807: // JSONAccumulatorObjectUnion: "'{['" Expr "']}'"
        {
          (yylhs.value.expr) = new JSONObjectConstructor(LOC(yylhs.location), (yystack_[1].value.expr), true);
        }
    break;

  case 808: // JSONObjectConstructor: "'{'" JSONPairList "'}'"
        {
          (yylhs.value.expr) = new JSONDirectObjectConstructor(LOC(yylhs.location),
                                               dynamic_cast<JSONPairList*>((yystack_[1].value.node)));
        }
    break;

  case 809: // JSONPairList: QNAME "':'" ExprSingle
        {
          ERROR_IF_QNAME_NOT_NCNAME((yystack_[2].value.expr), yystack_[2].location, (yystack_[0].value.expr));
          driver.addCommonLanguageWarning(yystack_[2].location, ZED(ZWST0009_OBJECT_KEY_NOT_QUOTED));
          StringLiteral* sl = new StringLiteral( LOC(yylhs.location), static_cast<QName*>((yystack_[2].value.expr)) );
          JSONPairList* jpl = new JSONPairList(LOC(yylhs.location));
          jpl->push_back(new JSONPairConstructor(LOC(yylhs.location), sl, (yystack_[0].value.expr)));
          (yylhs.value.node) = jpl;
        }
    break;

  case 810: // JSONPairList: ExprSingle "':'" ExprSingle
        {
          JSONPairList* jpl = new JSONPairList(LOC(yylhs.location));
          jpl->push_back(new JSONPairConstructor(LOC(yylhs.location), (yystack_[2].value.expr), (yystack_[0].value.expr)));
          (yylhs.value.node) = jpl;
        }
    break;

  case 811: // JSONPairList: JSONPairList "','" ExprSingle "':'" ExprSingle
        {
          JSONPairList* jpl = dynamic_cast<JSONPairList*>((yystack_[4].value.node));
          assert(jpl);
          jpl->push_back(new JSONPairConstructor(LOC(yylhs.location), (yystack_[2].value.expr), (yystack_[0].value.expr)));
          (yylhs.value.node) = jpl;
        }
    break;

  case 812: // JSONPairList: JSONPairList "','" QNAME "':'" ExprSingle
        {
          ERROR_IF_QNAME_NOT_NCNAME((yystack_[2].value.expr), yystack_[2].location, (yystack_[4].value.node), (yystack_[0].value.expr));
          JSONPairList* jpl = dynamic_cast<JSONPairList*>((yystack_[4].value.node));
          assert(jpl);
          StringLiteral* sl = new StringLiteral( LOC(yylhs.location), static_cast<QName*>((yystack_[2].value.expr)) );
          jpl->push_back(new JSONPairConstructor(LOC(yylhs.location), sl, (yystack_[0].value.expr)));
          (yylhs.value.node) = jpl;
        }
    break;

  case 813: // JSONInsertExpr: "'insert'" "'json'" ExprSingle "'into'" ExprSingle
        {
          (yylhs.value.expr) = new JSONObjectInsertExpr(LOC(yylhs.location),
                                        (yystack_[2].value.expr),
                                        (yystack_[0].value.expr));
        }
    break;

  case 814: // JSONInsertExpr: "'insert'" "'json'" JSONPairList "'into'" ExprSingle
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

  case 815: // JSONInsertExpr: "'insert'" "'json'" ExprSingle "'into'" ExprSingle "'at'" "'position'" ExprSingle
        {
          (yylhs.value.expr) = new JSONArrayInsertExpr(LOC(yylhs.location), (yystack_[5].value.expr), (yystack_[3].value.expr), (yystack_[0].value.expr));
        }
    break;

  case 816: // JSONAppendExpr: "'append'" "'json'" ExprSingle "'into'" ExprSingle
        {
          (yylhs.value.expr) = new JSONArrayAppendExpr(LOC(yylhs.location), (yystack_[2].value.expr), (yystack_[0].value.expr));
        }
    break;

  case 817: // JSONDeleteExpr: "'delete'" "'json'" PostfixExpr
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

  case 818: // JSONRenameExpr: "'rename'" "'json'" PostfixExpr "'as'" ExprSingle
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

  case 819: // JSONReplaceExpr: "'replace'" "'value'" "'of'" "'json'" PostfixExpr "'with'" ExprSingle
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

  case 820: // JSONTest: "'json-item'" "'('" "')'"
        {
          (yylhs.value.node) = new JSON_Test(LOC(yylhs.location), store::StoreConsts::jsonItem);
        }
    break;

  case 821: // JSONTest: "'array'" "'('" "')'"
        {
          (yylhs.value.node) = new JSON_Test(LOC(yylhs.location), store::StoreConsts::jsonArray);
        }
    break;

  case 822: // JSONTest: "'object'" "'('" "')'"
        {
          (yylhs.value.node) = new JSON_Test(LOC(yylhs.location), store::StoreConsts::jsonObject);
        }
    break;

  case 823: // URI_LITERAL: "'STRING'"
        { (yylhs.value.sval) = (yystack_[0].value.sval); }
    break;

  case 824: // QNAME: FUNCTION_NAME
        { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 825: // QNAME: "'NCName_sval'"
                  { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB((yystack_[0].value.sval))); }
    break;

  case 826: // QNAME: "'attribute'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("attribute"))); }
    break;

  case 827: // QNAME: "'comment'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("comment"))); }
    break;

  case 828: // QNAME: "'document-node'"
                    { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("document-node"))); }
    break;

  case 829: // QNAME: "'namespace-node'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("namespace-node"))); }
    break;

  case 830: // QNAME: "'element'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("element"))); }
    break;

  case 831: // QNAME: "'item'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("item"))); }
    break;

  case 832: // QNAME: "'if'"
         { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("if"))); }
    break;

  case 833: // QNAME: "'node'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("node"))); }
    break;

  case 834: // QNAME: "'processing-instruction'"
                             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("processing-instruction"))); }
    break;

  case 835: // QNAME: "'schema-attribute'"
                       { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("schema-attribute"))); }
    break;

  case 836: // QNAME: "'schema-element'"
                     { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("schema-element"))); }
    break;

  case 837: // QNAME: "'text'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("text"))); }
    break;

  case 838: // QNAME: "'typeswitch'"
                 { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("typeswitch"))); }
    break;

  case 839: // QNAME: "'switch'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("switch"))); }
    break;

  case 840: // QNAME: "'empty-sequence'"
                     { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("empty-sequence"))); }
    break;

  case 841: // QNAME: "'while'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("while"))); }
    break;

  case 842: // QNAME: "'json'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("json"))); }
    break;

  case 843: // QNAME: "'array'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("array"))); }
    break;

  case 844: // QNAME: "'object'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("object"))); }
    break;

  case 845: // QNAME: "'json-item'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("json-item"))); }
    break;

  case 846: // QNAME: "'structured-item'"
                      { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("structured-item"))); }
    break;

  case 847: // FUNCTION_NAME: EQNAME
        { (yylhs.value.expr) = (yystack_[0].value.expr); }
    break;

  case 848: // FUNCTION_NAME: "'QName'"
                 { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB((yystack_[0].value.sval))); }
    break;

  case 849: // FUNCTION_NAME: "'xquery'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("xquery"))); }
    break;

  case 850: // FUNCTION_NAME: "'empty'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("empty"))); }
    break;

  case 851: // FUNCTION_NAME: "'boundary-space'"
                     { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("boundary-space"))); }
    break;

  case 852: // FUNCTION_NAME: "'ft-option'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("ft-option"))); }
    break;

  case 853: // FUNCTION_NAME: "'base-uri'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("base-uri"))); }
    break;

  case 854: // FUNCTION_NAME: "'lax'"
          { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("lax"))); }
    break;

  case 855: // FUNCTION_NAME: "'strict'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("strict"))); }
    break;

  case 856: // FUNCTION_NAME: "'idiv'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("idiv"))); }
    break;

  case 857: // FUNCTION_NAME: "'document'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("document"))); }
    break;

  case 858: // FUNCTION_NAME: "'ftnot'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("ftnot"))); }
    break;

  case 859: // FUNCTION_NAME: "'not'"
          { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("not"))); }
    break;

  case 860: // FUNCTION_NAME: "'sensitive'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("sensitive"))); }
    break;

  case 861: // FUNCTION_NAME: "'insensitive'"
                  { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("insensitive"))); }
    break;

  case 862: // FUNCTION_NAME: "'diacritics'"
                 { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("diacritics"))); }
    break;

  case 863: // FUNCTION_NAME: "'without'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("without"))); }
    break;

  case 864: // FUNCTION_NAME: "'stemming'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("stemming"))); }
    break;

  case 865: // FUNCTION_NAME: "'thesaurus'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("thesaurus"))); }
    break;

  case 866: // FUNCTION_NAME: "'stop'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("stop"))); }
    break;

  case 867: // FUNCTION_NAME: "'wildcards'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("wildcards"))); }
    break;

  case 868: // FUNCTION_NAME: "'entire'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("entire"))); }
    break;

  case 869: // FUNCTION_NAME: "'content'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("content"))); }
    break;

  case 870: // FUNCTION_NAME: "'word'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("word"))); }
    break;

  case 871: // FUNCTION_NAME: "'start'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("start"))); }
    break;

  case 872: // FUNCTION_NAME: "'end'"
          { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("end"))); }
    break;

  case 873: // FUNCTION_NAME: "'most'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("most"))); }
    break;

  case 874: // FUNCTION_NAME: "'skip'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("skip"))); }
    break;

  case 875: // FUNCTION_NAME: "'copy'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("copy"))); }
    break;

  case 876: // FUNCTION_NAME: "'general'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("general"))); }
    break;

  case 877: // FUNCTION_NAME: "'value'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("value"))); }
    break;

  case 878: // FUNCTION_NAME: "'eq'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("eq"))); }
    break;

  case 879: // FUNCTION_NAME: "'ne'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("ne"))); }
    break;

  case 880: // FUNCTION_NAME: "'lt'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("lt"))); }
    break;

  case 881: // FUNCTION_NAME: "'le'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("le"))); }
    break;

  case 882: // FUNCTION_NAME: "'gt'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("gt"))); }
    break;

  case 883: // FUNCTION_NAME: "'ge'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("ge"))); }
    break;

  case 884: // FUNCTION_NAME: "'at'"
         { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("at"))); }
    break;

  case 885: // FUNCTION_NAME: "'context'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("context"))); }
    break;

  case 886: // FUNCTION_NAME: "'variable'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("variable"))); }
    break;

  case 887: // FUNCTION_NAME: "'return'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("return"))); }
    break;

  case 888: // FUNCTION_NAME: "'for'"
          { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("for"))); }
    break;

  case 889: // FUNCTION_NAME: "'from'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("from"))); }
    break;

  case 890: // FUNCTION_NAME: "'allowing'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("allowing"))); }
    break;

  case 891: // FUNCTION_NAME: "'sliding'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("sliding"))); }
    break;

  case 892: // FUNCTION_NAME: "'tumbling'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("tumbling"))); }
    break;

  case 893: // FUNCTION_NAME: "'previous'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("previous"))); }
    break;

  case 894: // FUNCTION_NAME: "'next'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("next"))); }
    break;

  case 895: // FUNCTION_NAME: "'only'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("only"))); }
    break;

  case 896: // FUNCTION_NAME: "'when'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("when"))); }
    break;

  case 897: // FUNCTION_NAME: "'count'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("count"))); }
    break;

  case 898: // FUNCTION_NAME: "'in'"
          { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("in"))); }
    break;

  case 899: // FUNCTION_NAME: "'let'"
          { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("let"))); }
    break;

  case 900: // FUNCTION_NAME: "'where'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("where"))); }
    break;

  case 901: // FUNCTION_NAME: "'by'"
         { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("by"))); }
    break;

  case 902: // FUNCTION_NAME: "'group'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("group"))); }
    break;

  case 903: // FUNCTION_NAME: "'order'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("order"))); }
    break;

  case 904: // FUNCTION_NAME: "'stable'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("stable"))); }
    break;

  case 905: // FUNCTION_NAME: "'ascending'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("ascending"))); }
    break;

  case 906: // FUNCTION_NAME: "'descending'"
                 { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("descending"))); }
    break;

  case 907: // FUNCTION_NAME: "'greatest'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("greatest"))); }
    break;

  case 908: // FUNCTION_NAME: "'least'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("least"))); }
    break;

  case 909: // FUNCTION_NAME: "'collation'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("collation"))); }
    break;

  case 910: // FUNCTION_NAME: "'some'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("some"))); }
    break;

  case 911: // FUNCTION_NAME: "'every'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("every"))); }
    break;

  case 912: // FUNCTION_NAME: "'satisfies'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("satisfies"))); }
    break;

  case 913: // FUNCTION_NAME: "'case'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("case"))); }
    break;

  case 914: // FUNCTION_NAME: "'as'"
         { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("as"))); }
    break;

  case 915: // FUNCTION_NAME: "'then'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("then"))); }
    break;

  case 916: // FUNCTION_NAME: "'else'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("else"))); }
    break;

  case 917: // FUNCTION_NAME: "'or'"
         { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("or"))); }
    break;

  case 918: // FUNCTION_NAME: "'and'"
          { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("and"))); }
    break;

  case 919: // FUNCTION_NAME: "'instance'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("instance"))); }
    break;

  case 920: // FUNCTION_NAME: "'of'"
         { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("of"))); }
    break;

  case 921: // FUNCTION_NAME: "'castable'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("castable"))); }
    break;

  case 922: // FUNCTION_NAME: "'to'"
         { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("to"))); }
    break;

  case 923: // FUNCTION_NAME: "'div'"
          { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("div"))); }
    break;

  case 924: // FUNCTION_NAME: "'mod'"
          { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("mod"))); }
    break;

  case 925: // FUNCTION_NAME: "'union'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("union"))); }
    break;

  case 926: // FUNCTION_NAME: "'intersect'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("intersect"))); }
    break;

  case 927: // FUNCTION_NAME: "'except'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("except"))); }
    break;

  case 928: // FUNCTION_NAME: "'validate'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("validate"))); }
    break;

  case 929: // FUNCTION_NAME: "'type'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("type"))); }
    break;

  case 930: // FUNCTION_NAME: "'cast'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("cast"))); }
    break;

  case 931: // FUNCTION_NAME: "'treat'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("treat"))); }
    break;

  case 932: // FUNCTION_NAME: "'is'"
         { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("is"))); }
    break;

  case 933: // FUNCTION_NAME: "'preserve'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("preserve"))); }
    break;

  case 934: // FUNCTION_NAME: "'strip'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("strip"))); }
    break;

  case 935: // FUNCTION_NAME: "'namespace'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("namespace"))); }
    break;

  case 936: // FUNCTION_NAME: "'external'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("external"))); }
    break;

  case 937: // FUNCTION_NAME: "'encoding'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("encoding"))); }
    break;

  case 938: // FUNCTION_NAME: "'<no preserve>'"
                  { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("no-preserve"))); }
    break;

  case 939: // FUNCTION_NAME: "'inherit'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("inherit"))); }
    break;

  case 940: // FUNCTION_NAME: "'<no inherit>'"
                 { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("no-inherit"))); }
    break;

  case 941: // FUNCTION_NAME: "'declare'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("declare"))); }
    break;

  case 942: // FUNCTION_NAME: "'construction'"
                   { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("construction"))); }
    break;

  case 943: // FUNCTION_NAME: "'ordering'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("ordering"))); }
    break;

  case 944: // FUNCTION_NAME: "'default'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("default"))); }
    break;

  case 945: // FUNCTION_NAME: "'copy-namespaces'"
                      { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("copy-namespaces"))); }
    break;

  case 946: // FUNCTION_NAME: "'option'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("option"))); }
    break;

  case 947: // FUNCTION_NAME: "'version'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("version"))); }
    break;

  case 948: // FUNCTION_NAME: "'import'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("import"))); }
    break;

  case 949: // FUNCTION_NAME: "'schema'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("schema"))); }
    break;

  case 950: // FUNCTION_NAME: "'module'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("module"))); }
    break;

  case 951: // FUNCTION_NAME: "'function'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("function"))); }
    break;

  case 952: // FUNCTION_NAME: "'score'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("score"))); }
    break;

  case 953: // FUNCTION_NAME: "'contains'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("contains"))); }
    break;

  case 954: // FUNCTION_NAME: "'weight'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("weight"))); }
    break;

  case 955: // FUNCTION_NAME: "'window'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("window"))); }
    break;

  case 956: // FUNCTION_NAME: "'distance'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("distance"))); }
    break;

  case 957: // FUNCTION_NAME: "'occurs'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("occurs"))); }
    break;

  case 958: // FUNCTION_NAME: "'times'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("times"))); }
    break;

  case 959: // FUNCTION_NAME: "'same'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("same"))); }
    break;

  case 960: // FUNCTION_NAME: "'different'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("different"))); }
    break;

  case 961: // FUNCTION_NAME: "'lowercase'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("lowercase"))); }
    break;

  case 962: // FUNCTION_NAME: "'uppercase'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("uppercase"))); }
    break;

  case 963: // FUNCTION_NAME: "'relationship'"
                   { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("relationship"))); }
    break;

  case 964: // FUNCTION_NAME: "'levels'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("levels"))); }
    break;

  case 965: // FUNCTION_NAME: "'language'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("language"))); }
    break;

  case 966: // FUNCTION_NAME: "'any'"
          { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("any"))); }
    break;

  case 967: // FUNCTION_NAME: "'all'"
          { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("all"))); }
    break;

  case 968: // FUNCTION_NAME: "'exactly'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("exactly"))); }
    break;

  case 969: // FUNCTION_NAME: "'words'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("words"))); }
    break;

  case 970: // FUNCTION_NAME: "'sentences'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("sentences"))); }
    break;

  case 971: // FUNCTION_NAME: "'sentence'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("sentence"))); }
    break;

  case 972: // FUNCTION_NAME: "'phrase'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("phrase"))); }
    break;

  case 973: // FUNCTION_NAME: "'paragraph'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("paragraph"))); }
    break;

  case 974: // FUNCTION_NAME: "'paragraphs'"
                 { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("paragraphs"))); }
    break;

  case 975: // FUNCTION_NAME: "'modify'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("modify"))); }
    break;

  case 976: // FUNCTION_NAME: "'first'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("first"))); }
    break;

  case 977: // FUNCTION_NAME: "'append'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("append"))); }
    break;

  case 978: // FUNCTION_NAME: "'insert'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("insert"))); }
    break;

  case 979: // FUNCTION_NAME: "'replace'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("replace"))); }
    break;

  case 980: // FUNCTION_NAME: "'rename'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("rename"))); }
    break;

  case 981: // FUNCTION_NAME: "'delete'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("delete"))); }
    break;

  case 982: // FUNCTION_NAME: "'before'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("before"))); }
    break;

  case 983: // FUNCTION_NAME: "'after'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("after"))); }
    break;

  case 984: // FUNCTION_NAME: "'revalidation'"
                   { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("revalidation"))); }
    break;

  case 985: // FUNCTION_NAME: "'with'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("with"))); }
    break;

  case 986: // FUNCTION_NAME: "'nodes'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("nodes"))); }
    break;

  case 987: // FUNCTION_NAME: "'last'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("last"))); }
    break;

  case 988: // FUNCTION_NAME: "'into'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("into"))); }
    break;

  case 989: // FUNCTION_NAME: "'simple'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("simple"))); }
    break;

  case 990: // FUNCTION_NAME: "'sequential'"
                 { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("sequential"))); }
    break;

  case 991: // FUNCTION_NAME: "'updating'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("updating"))); }
    break;

  case 992: // FUNCTION_NAME: "'ordered'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("ordered"))); }
    break;

  case 993: // FUNCTION_NAME: "'unordered'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("unordered"))); }
    break;

  case 994: // FUNCTION_NAME: "'returning'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("returning"))); }
    break;

  case 995: // FUNCTION_NAME: "'exit'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("exit"))); }
    break;

  case 996: // FUNCTION_NAME: "'loop'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("loop"))); }
    break;

  case 997: // FUNCTION_NAME: "'break'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("break"))); }
    break;

  case 998: // FUNCTION_NAME: "'continue'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("continue"))); }
    break;

  case 999: // FUNCTION_NAME: "'try'"
          { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("try"))); }
    break;

  case 1000: // FUNCTION_NAME: "'catch'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("catch"))); }
    break;

  case 1001: // FUNCTION_NAME: "'using'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("using"))); }
    break;

  case 1002: // FUNCTION_NAME: "'set'"
          { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("set"))); }
    break;

  case 1003: // FUNCTION_NAME: "'index'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("index"))); }
    break;

  case 1004: // FUNCTION_NAME: "'unique'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("unique"))); }
    break;

  case 1005: // FUNCTION_NAME: "'non'"
          { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("non"))); }
    break;

  case 1006: // FUNCTION_NAME: "'on'"
         { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("on"))); }
    break;

  case 1007: // FUNCTION_NAME: "'range'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("range"))); }
    break;

  case 1008: // FUNCTION_NAME: "'equality'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("equality"))); }
    break;

  case 1009: // FUNCTION_NAME: "'manually'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("manually"))); }
    break;

  case 1010: // FUNCTION_NAME: "'automatically'"
                    { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("automatically"))); }
    break;

  case 1011: // FUNCTION_NAME: "'maintained'"
                 { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("maintained"))); }
    break;

  case 1012: // FUNCTION_NAME: "'decimal-format'"
                     { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("decimal-format"))); }
    break;

  case 1013: // FUNCTION_NAME: "'decimal-separator'"
                        { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("decimal-separator"))); }
    break;

  case 1014: // FUNCTION_NAME: "'grouping-separator'"
                         { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("grouping-separator"))); }
    break;

  case 1015: // FUNCTION_NAME: "'infinity'"
                     { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("infinity"))); }
    break;

  case 1016: // FUNCTION_NAME: "'minus-sign'"
                 { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("minus-sign"))); }
    break;

  case 1017: // FUNCTION_NAME: "'NaN'"
          { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("NaN"))); }
    break;

  case 1018: // FUNCTION_NAME: "'percent'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("percent"))); }
    break;

  case 1019: // FUNCTION_NAME: "'per-mille'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("per-mille"))); }
    break;

  case 1020: // FUNCTION_NAME: "'zero-digit'"
                 { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("zero-digit"))); }
    break;

  case 1021: // FUNCTION_NAME: "'digit'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("digit"))); }
    break;

  case 1022: // FUNCTION_NAME: "'pattern-separator'"
                        { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("pattern-separator"))); }
    break;

  case 1023: // FUNCTION_NAME: "'collection'"
                 { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("collection"))); }
    break;

  case 1024: // FUNCTION_NAME: "'const'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("const"))); }
    break;

  case 1025: // FUNCTION_NAME: "'append-only'"
                  { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("append-only"))); }
    break;

  case 1026: // FUNCTION_NAME: "'queue'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("queue"))); }
    break;

  case 1027: // FUNCTION_NAME: "'mutable'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("mutable"))); }
    break;

  case 1028: // FUNCTION_NAME: "'read-only'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("read-only"))); }
    break;

  case 1029: // FUNCTION_NAME: "'integrity'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("integrity"))); }
    break;

  case 1030: // FUNCTION_NAME: "'constraint'"
                 { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("constraint"))); }
    break;

  case 1031: // FUNCTION_NAME: "'check'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("check"))); }
    break;

  case 1032: // FUNCTION_NAME: "'key'"
          { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("key"))); }
    break;

  case 1033: // FUNCTION_NAME: "'foreach'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("foreach"))); }
    break;

  case 1034: // FUNCTION_NAME: "'foreign'"
              { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("foreign"))); }
    break;

  case 1035: // FUNCTION_NAME: "'keys'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("keys"))); }
    break;

  case 1036: // FUNCTION_NAME: "'ancestor'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("ancestor"))); }
    break;

  case 1037: // FUNCTION_NAME: "'child'"
            { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("child"))); }
    break;

  case 1038: // FUNCTION_NAME: "'descendant'"
                 { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("descendant"))); }
    break;

  case 1039: // FUNCTION_NAME: "'parent'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("parent"))); }
    break;

  case 1040: // FUNCTION_NAME: "'preceding'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("preceding"))); }
    break;

  case 1041: // FUNCTION_NAME: "'self'"
           { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("self"))); }
    break;

  case 1042: // FUNCTION_NAME: "'following'"
                { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("following"))); }
    break;

  case 1043: // FUNCTION_NAME: "'ancestor-or-self'"
                       { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("ancestor-or-self"))); }
    break;

  case 1044: // FUNCTION_NAME: "'descendant-or-self'"
                         { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("descendant-or-self"))); }
    break;

  case 1045: // FUNCTION_NAME: "'following-sibling'"
                        { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("following-sibling"))); }
    break;

  case 1046: // FUNCTION_NAME: "'preceding-sibling'"
                        { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("preceding-sibling"))); }
    break;

  case 1047: // FUNCTION_NAME: "'position'"
               { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("position"))); }
    break;

  case 1048: // FUNCTION_NAME: "'null'"
                 { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("null"))); }
    break;

  case 1049: // FUNCTION_NAME: "'true'"
                 { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("true"))); }
    break;

  case 1050: // FUNCTION_NAME: "'false'"
                  { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("false"))); }
    break;

  case 1051: // FUNCTION_NAME: "'select'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("select"))); }
    break;

  case 1052: // FUNCTION_NAME: "'jsoniq'"
             { (yylhs.value.expr) = new QName(LOC(yylhs.location), SYMTAB(SYMTAB_PUT("jsoniq"))); }
    break;

  case 1053: // EQNAME: "'EQName'"
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


  const short jsoniq_parser::yypact_ninf_ = -1527;

  const short jsoniq_parser::yytable_ninf_ = -890;

  const short
  jsoniq_parser::yypact_[] =
  {
    5725, -1527, -1527, -1527, -1527,  7319,  7319,  7319,  7319,  7319,
    7319, -1527, -1527,    14,   282, -1527,   936,   368, -1527, -1527,
   -1527,   833, -1527, -1527, -1527,    45,   575,   842,  3584,   592,
     642,   830, -1527,   -11, -1527,   684, -1527, -1527, -1527, -1527,
   -1527,   855, -1527,   794,   803, -1527, -1527, -1527, -1527,   376,
   -1527,   859, -1527,   817,   823, -1527,   321, -1527, -1527, -1527,
   -1527, -1527, -1527, -1527, -1527, -1527, -1527, -1527, -1527, -1527,
   -1527,   462,   865, -1527, -1527, -1527, -1527,   738, 14315, -1527,
   -1527, -1527,   872, -1527, -1527, -1527,   887, -1527,   884,   905,
   -1527, -1527, 18861, -1527, -1527, -1527, -1527, -1527, -1527, -1527,
   -1527,   927, -1527, -1527,   931,   932, -1527, -1527, -1527, -1527,
   -1527, -1527, -1527,  7319,  7637,  7955, 18861, -1527, -1527,   903,
   -1527, -1527, -1527, -1527,   904, -1527, -1527,   940, 18861, -1527,
   17351,   941,   942, -1527, -1527, -1527, -1527,   943, -1527, -1527,
   13361, -1527, -1527, -1527, -1527, -1527, -1527,   913, -1527, -1527,
   -1527, -1527, -1527, -1527, -1527, -1527,   163, -1527,   850, -1527,
   -1527, -1527, -1527, -1527, -1527, -1527, -1527, -1527, -1527, -1527,
     -91,   917,   220, -1527,  -127,   674, -1527, -1527, -1527, -1527,
   -1527, -1527,   957, -1527,   831,   835,   837, -1527, -1527,   922,
     926, -1527, -1527, -1527, -1527, -1527, -1527, -1527, -1527, -1527,
   -1527, -1527, -1527,  5437, -1527, 13997, -1527, -1527, -1527, -1527,
   -1527, -1527, -1527, -1527, -1527, -1527, -1527, -1527, -1527, -1527,
   -1527, -1527, -1527, -1527, -1527, -1527, -1527, -1527, -1527, -1527,
   -1527, -1527, -1527, -1527, -1527, -1527, -1527, -1527, -1527, -1527,
   -1527, -1527, -1527, -1527, -1527, -1527, -1527, -1527, -1527, -1527,
   -1527, -1527, -1527, -1527,  8273,  8591, -1527,   763, -1527, -1527,
   -1527, -1527, -1527,  6045,  8909,  1086, -1527,  9227, -1527, -1527,
     759,   111, -1527, -1527, -1527, -1527, -1527, -1527, -1527, -1527,
   -1527, -1527, -1527, -1527, -1527, -1527,   113, -1527, -1527, -1527,
   -1527, -1527, -1527,   673, -1527, -1527, -1527, -1527, -1527, -1527,
   -1527, -1527,  7319, -1527, -1527, -1527, -1527,    54, -1527, -1527,
   -1527, -1527, -1527, -1527, -1527, -1527, -1527,   240, -1527,   894,
   -1527, -1527, -1527,   547, -1527,   571, -1527, -1527, -1527, -1527,
   -1527, -1527, -1527,   919,   996, -1527, -1527,  1096,   838,   991,
     629,   597,   593,   727, -1527,  1042,   892,   993,   999, 11453,
   -1527,   901, -1527, -1527,    23, -1527, -1527, 13679, -1527,   428,
   -1527,   947, 14315, -1527,   947, 14315, -1527, -1527, -1527,   710,
   -1527, -1527, -1527, -1527, -1527, -1527, -1527, -1527, -1527, -1527,
   -1527, -1527, -1527, -1527, -1527, -1527, -1527, -1527, -1527, -1527,
   -1527, -1527, -1527, -1527, -1527, -1527, -1527,   946, -1527, -1527,
   -1527, -1527, -1527, -1527, -1527, -1527, -1527, -1527, -1527, -1527,
   -1527, -1527, -1527, -1527, -1527, -1527, -1527, -1527, -1527, -1527,
   -1527, -1527, -1527, -1527, -1527,   676, -1527, -1527, -1527, -1527,
   -1527,   914, -1527,  7319,   921,   923,   924,   925,   929,  7319,
     521,   521,  1077,   414,   580,   513, 19463, 18861,   504,  1064,
   18861,   962,   998,   559, 18861,   797,   867, 18861, 18861,   810,
     731,    58, -1527, -1527, -1527, 18861,  7319,   935,  7319,   383,
   11771, 17653, 18861,   393,   966, 11771,  1125,    89,   364, 18861,
    1007,   983,  1020, -1527,   944, 11771, 17955, 18861, 18861, 18861,
    7319,   945, 11771, 11771, 18861,  7319,   974,   976, -1527, -1527,
   -1527, 11771, 18257,   977, -1527,   978, -1527, -1527, -1527, -1527,
     981, -1527,   982, -1527, -1527, -1527, -1527, -1527,   984, -1527,
   -1527, -1527, -1527, -1527, -1527, -1527, -1527, -1527, -1527, -1527,
   -1527, -1527, -1527, -1527, -1527, -1527, -1527, -1527, -1527, -1527,
   -1527, -1527, -1527, -1527, -1527, -1527, -1527, -1527, -1527, 18861,
   -1527, -1527, -1527, -1527, -1527, -1527, -1527, -1527, -1527, -1527,
   -1527, -1527, -1527,   990, 18861, -1527, -1527,   956,  6365,  1021,
     336,  1022,   985,   992,   994, 18861,  7319, -1527,  1000,   854,
     182, -1527,   547, -1527,   110,  1116, 11771, 11771, -1527,   213,
   -1527, -1527,  1138, -1527, -1527, -1527, -1527, 11771,   951, -1527,
    1126, 11771, 11771, 19169, 11771, 11771, 11771, 11771, 19169, 11771,
     920, 18861,   967,   968, 11771, 11771,  7319,   907, 18861, -1527,
   -1527,    82, -1527,    72, 11771,  8909, -1527, -1527, -1527, -1527,
   -1527,   936,   830,   121,   124,  1160,  9545,  4143,  9863,  4472,
     592,   887, -1527, -1527,   256,   887, -1527, 11771,  5108, -1527,
    1044,   705,   -11,  1001,   997,  1002, -1527, -1527, 11771,   768,
   -1527, -1527,  7319, -1527, -1527, -1527, -1527, -1527, -1527, -1527,
     879,   885, 18861,  1053, 12089, 12089, 12407, -1527, 12407, 12407,
   -1527, 12407, -1527, 12407, -1527, -1527, -1527, -1527, -1527, -1527,
   -1527, 12407, 12407,  1103, 12407, 12407, 12407, 12407, 12407, 12407,
   12407, 12407, 12407, 12407, 12407, 12407,   939,  1082,  1083,  1084,
   -1527, -1527, -1527, 13043,  7319, -1527, -1527, 13361, 13361, 11771,
     947, -1527, -1527,   947, -1527, 15237, 10181,  6683,   947,  1003,
    1029,  7001, -1527, -1527, -1527, -1527, -1527, -1527,    38, -1527,
      57, -1527, -1527, -1527, -1527, -1527, -1527,  1076,  1079,   592,
    1155, -1527, -1527, 19463,  1196,   702, 18861,  1026,  1027,  1196,
    1077,  1065,  1061, -1527, -1527, -1527,   103,   948,  1101,   893,
   18861,  1055, 11771,  1078, 18861, 18861, -1527,  1067,  1019, -1527,
    1023,   978,   803, -1527,  1017,  1018,   236, -1527,   287,   331,
    1102, -1527,    78, 18861,  1119,   371, -1527,  7319, -1527,   144,
   -1527, 18861,  1114,  1170, 18861,  1077,  1120,   514, 18861, 11771,
     -11, -1527,   250,  1030, -1527,  1031,  1032,  1033,   274, -1527,
      49,  1038, -1527,   148,   149,  1069, -1527,  1041,  7319,  7319,
     275, -1527,   372,   381,   647, 11771,   297, -1527, -1527, 11771,
   11771, -1527, 11771, 11771, 11771, 11771, -1527, 11771, -1527, 11771,
   -1527, 18861,  1116, -1527,   288,   339,   348, -1527, -1527, -1527,
     394, -1527,    16, -1527, -1527,  1072,  1073,  1080,  1081,  1085,
     710,   676,   360,   402,    46,    24,  1136,   632,   989,   -78,
     133, -1527,  1089, -1527, -1527,  1050,   164,  6365,   405, 14633,
     907, -1527,  1102, -1527, -1527,   995, -1527,   759,   752,   820,
     699, -1527, -1527,   127, -1527, -1527, -1527,   131, -1527,   109,
   -1527, -1527, -1527, -1527, -1527,   966, -1527,  4790, -1527, -1527,
   -1527, 18861,  1075, 11771, 12725, -1527, -1527, -1527, -1527, -1527,
    1102, 18861,    51,   996, -1527, -1527, -1527, -1527, -1527, 12407,
   -1527, -1527, -1527,    70, -1527,   597,   597,   307,   593,   593,
     593,   593,   727,   727, -1527, -1527, 16747, 16747, 18861, 18861,
   -1527,  1058, -1527, -1527,   225, -1527, 10499, -1527, -1527, -1527,
   -1527, -1527, -1527, -1527, -1527,   505, -1527, -1527, -1527,   525,
     521, -1527, -1527,   748,   750,   749, -1527,   592, -1527, -1527,
   -1527, -1527, -1527, -1527, -1527, -1527, -1527, -1527,  1196, -1527,
    1110, 17049,  1104, 11771, -1527, -1527, -1527,  1152,  1077,  1077,
    1196, -1527,   800,  1077,   695, 18861,   -73,   687,  1220, -1527,
   -1527,   959,   581, -1527, -1527, -1527, -1527, -1527, -1527, -1527,
   -1527, -1527, -1527, -1527,   103,    53,  1024,   154, 11771, -1527,
   18861,  1157,   949,  1077, -1527, -1527, -1527, -1527,  1106, 18861,
   -1527, 18861, -1527, 18559,  1127, 16747,  1139, 11771,   -32,  1113,
      42,  1152, 16747,  1140,  1168,  1087,  1068,  1130,  1077,  1105,
    1131,  1172,  1077, 11771,    62, -1527, -1527, -1527,  1115, -1527,
   -1527, -1527, -1527,  1154, 11771, 11771,  1122, -1527,  1174,  1186,
    7319, -1527,  1107,  1108,  1133, 18861, -1527, 18861, -1527, 11771,
    1145,  1098, 11771, -1527,  1171,  1176, -1527,   165,   175,   193,
    1260, -1527,   337, -1527,   302,  1156, -1527, -1527,  1270, -1527,
     427, 11771, 11771, 11771,   792, 11771, 11771, 11771, 11771, 11771,
   11771, 11771, 11771, 11771, 19169,  1191, 11771, 11771, -1527, 10817,
     136,  1062, -1527,   563, -1527, 11771,   132,   145,   109,  9863,
    4472,  9863,  1232, -1527, -1527, -1527, 11771,   777,  1206, -1527,
   18861,  1209,  1210, -1527,   341,  1059, -1527, -1527,   706,    51,
   -1527, 11771,    70,   400,   644, -1527,  1034,   184,  1057,  1056,
   -1527, -1527,   867, -1527,  1054,   -77,  1165,  1173, 15539,  1175,
    1177,  1178,  1179,  1180,   900, -1527,   641, -1527, -1527, -1527,
   -1527, -1527, -1527, -1527, -1527, -1527, -1527,  1183, -1527, -1527,
   -1527, -1527, -1527,   196, 11135, -1527, -1527, -1527, -1527, -1527,
   -1527, -1527, -1527, -1527,   770, -1527,  1300, 17049,   846, -1527,
   -1527, -1527, -1527, -1527, -1527, -1527, -1527, -1527,  1047, -1527,
   -1527,  1304, -1527, -1527, -1527, -1527, -1527,   612,  1310, -1527,
     589, -1527, -1527, -1527,   977,   575,   684,   981,   794,   982,
     984, -1527,  1177,  1178,  1179, -1527,   641,   641, 13043,  1048,
    1036, -1527,  1152,    53,  1109,  1161,  7319,  1158,  1166,  1198,
    1167,  1169, 18861, -1527,   476, -1527, 18861, -1527, 11771,  1211,
   11771,  1229, 11771,     1,  1214, -1527, -1527, -1527, 16747, -1527,
    7319,  1077,  1256, -1527, -1527, -1527,  1077,  1256, -1527, 11771,
    1223,  7319, 18861, -1527, -1527, 11771, 11771,   271, -1527,   295,
     832, -1527, 15841,   834, -1527,   836, -1527,  1184, -1527, -1527,
    7319,  1182,  1187, -1527, 11771, -1527, -1527, 11771, 11771,  1181,
    1174,  1267, -1527,  1238, -1527,   677, -1527, -1527,  1369, -1527,
   -1527,  7319, 18861, -1527,   882, -1527, -1527, -1527,  7319,  1185,
    1137,  1141, -1527, -1527, -1527,  1143,  1146, -1527, -1527, -1527,
    1282, -1527, -1527, -1527, -1527,  1144,   436, 18861,   828, -1527,
    1226,  1228,  1231,  1236, -1527,   894,   547, 14633,  1062, -1527,
   10817, 14633, -1527, -1527,   820,   461, -1527, -1527, -1527,  1206,
   -1527,  1077, -1527,   871, -1527,   650,  1285, -1527, 11771,   654,
    1077, -1527, 12725, 11771,  1244, -1527,  1275,  1276, 11771, 18861,
     933,  1318, -1527, -1527, -1527,   410,  1212, -1527,    70,  1142,
   -1527, -1527,   551, -1527,   655,   283,  1147,    70,   655, 12407,
   -1527,   233, -1527, -1527, -1527, -1527, -1527, -1527,    70,  1248,
    1121,   948,   283, -1527, -1527,  1112,  1322, -1527, -1527, -1527,
   14935,  1219, -1527,  1222,  1224,  1225,  1227,  1230,  1233,  1165,
   -1527, -1527, -1527, -1527, -1527, -1527, -1527, -1527, -1527, -1527,
   -1527, -1527, -1527, -1527, -1527, -1527,  1372,   358,  1378,   358,
    1134,  1306, -1527, -1527,  1245, 18861,  1162, -1527, -1527, 13043,
   -1527,  1234, -1527, -1527, -1527, -1527, -1527, -1527, 11771,  1268,
   -1527, -1527, 11771, -1527,   558, -1527, 11771,  1271, 11771, -1527,
    1295,  1296,  1329,  1077,  1256, -1527, 11771,  1242, -1527, -1527,
   -1527,   828, -1527,   295, 11771,  7319,   828,   295, -1527, 18861,
     696, -1527, 16143,   557, -1527, 16445,   828, -1527, -1527,  1243,
   -1527, -1527, -1527, -1527, -1527, 11771,   843,  1260, 18861,   915,
   -1527,  1249,  1260, 18861, -1527,  1251, -1527, 11771, 11771, 11771,
   11771,  1111, 11771, 11771, -1527, 11771, 11771, 11771, 11771, 10817,
     568, -1527, -1527, -1527, -1527, -1527,  1285, -1527, -1527, -1527,
   11771,  1286, -1527, -1527, -1527, 11771, 18861, 18861, -1527,   143,
   -1527, 11771, -1527, -1527, -1527,  1252, 12407, -1527, -1527, -1527,
   -1527, -1527,   421, 12407, 12407,   659, -1527,  1057, -1527,   -39,
   -1527,  1056,    70,  1274, -1527, -1527,  1159, -1527, -1527, -1527,
   -1527,  1346,  1258, -1527,   536, -1527, -1527, -1527, -1527, -1527,
   -1527, -1527, -1527,   447,   447, -1527,   358, -1527, -1527,   548,
   -1527,  1414,    32,  1350,  1265, 13043,    -1,  1188,  1277, -1527,
   -1527, 11771, -1527, 11771,  1305, -1527, 11771, -1527, 10817, 11771,
    1077, -1527, -1527, -1527, 11771, 11771, -1527, -1527, -1527, 10817,
   10817,  1364, 16747,  7319, 18861,   696, 18861, 11771, 18861,   696,
   10817, -1527,   569,    47,  1260, 18861, -1527,  1261,  1260, -1527,
   -1527, -1527, -1527, -1527, 11771, -1527, -1527, -1527,   234,   235,
     238, 11771, -1527, -1527,  1209, 11771, -1527, -1527,  1407, -1527,
   -1527, -1527,   593, 12407, 12407,   307,   679, -1527, -1527, -1527,
   -1527, -1527, -1527, 11771, -1527, 16747, -1527, 16747,  1368, -1527,
   -1527, -1527,  1435, -1527, -1527, -1527,  1195,  1359, -1527, -1527,
    1360, -1527,   864, 18861,  1351,  1246, 18861, 13043, -1527, -1527,
   11771, -1527, -1527, -1527,  1256, -1527, -1527, 16747, -1527, -1527,
   -1527,  1374, 11771,   828, -1527,  1380, 10817, -1527, 18861,   613,
     692, -1527,  1279,  1260, -1527,  1281, -1527,  1284,  1174,  1186,
     433, -1527,  1209,  1357,   307,   307, 12407,   434, -1527, -1527,
   16747, -1527, -1527,  1350, 13043, -1527,  1285,  1189, 18861,  1363,
    1254,  1360, -1527,   696, 16747,  7319, 16747,   828, -1527, -1527,
    1392,   639, -1527, -1527, -1527, -1527,  1307,   723, -1527, -1527,
   -1527,  1289, -1527, 10817,   844, -1527, -1527, 18861,   307, -1527,
   -1527, -1527, -1527, -1527, 11771,  1190, 18861,  1371,  7319,   696,
   -1527, -1527,   696, 11771, -1527, -1527, -1527, -1527, -1527, -1527,
   -1527,  1373, -1527, -1527,  1197,  1192, 18861, -1527, -1527, 11771,
   10817, 18861,  1193, 11771,  1194,   828, 13043, -1527, 13043, 10817,
   -1527,  1293,  1202, 18861,  1266,  1379, 18861,  1199, 13043, -1527
  };

  const short
  jsoniq_parser::yydefact_[] =
  {
     166,   474,   475,   825,   476,   169,   169,   169,   169,   169,
       0,   848,  1053,   121,   123,   652,   941,   950,   890,   853,
     851,   827,   942,   945,   897,   857,   828,   830,     0,   951,
     832,   948,   919,   899,   873,   829,   894,   895,   946,   943,
     893,   834,   949,   835,   836,   990,  1002,   989,   891,   910,
     904,   837,   892,   839,   838,   991,   928,   929,   896,   870,
    1012,  1013,  1014,  1015,  1016,  1017,  1018,  1019,  1020,  1021,
    1022,  1036,  1043,   918,   914,   905,   884,   826,     0,   913,
     921,   930,  1037,   909,   507,   885,   886,   944,  1038,  1044,
     906,   923,     0,   513,   463,   509,   916,   850,   907,   908,
     937,   911,   927,   936,  1042,  1045,   856,   898,   939,   508,
     926,   932,   831,   169,     0,     0,     0,   410,   924,   935,
     940,   938,   917,   903,   992,   901,   902,  1039,     0,   409,
       0,  1040,  1046,   933,   887,   912,  1051,  1041,   212,   441,
       0,   473,   934,   915,   922,   931,   925,   993,   878,   883,
     882,   881,   880,   879,   840,   900,     0,   849,  1052,   947,
     871,  1048,  1049,  1050,   983,   982,   984,   855,   854,   874,
     981,   833,   978,   986,   980,   979,   877,   920,   976,   988,
     987,   975,   875,   985,   997,   998,   995,   996,   994,   841,
     999,  1000,  1001,   967,   966,   953,   869,   862,   960,   956,
     872,   868,   968,     0,   858,   859,   852,   861,   965,   964,
     961,   957,   973,   974,   972,   963,   959,   952,   860,   971,
     970,   864,   866,   865,   958,   962,   954,   867,   955,   863,
     969,  1023,  1024,  1025,  1026,  1027,  1028,  1004,  1005,  1003,
    1009,  1010,  1011,  1006,  1007,  1008,   876,  1029,  1030,  1031,
    1032,  1033,  1034,  1035,     0,     0,   842,   977,  1047,   844,
     843,   845,   846,   166,   166,     0,     2,   166,     9,    11,
      23,     0,    28,    31,    36,    37,    38,    39,    40,    41,
      42,    32,    58,    59,    33,    34,     0,    76,    80,    81,
      35,    82,    83,     0,   119,    84,    85,    86,    87,    18,
     163,   164,   165,   172,   175,   500,   176,     0,   177,   178,
     179,   180,   181,   182,   183,   186,   187,     0,   213,   221,
     216,   250,   256,     0,   248,     0,   249,   223,   217,   185,
     218,   184,   219,   222,   359,   361,   363,   365,   375,   377,
     381,   383,   386,   391,   394,   397,   399,   401,   403,     0,
     407,   414,   413,   415,     0,   432,   416,   437,   440,   442,
     445,   447,     0,   452,   449,     0,   460,   470,   472,   446,
     477,   491,   505,   492,   493,   494,   497,   498,   495,   499,
     522,   523,   496,   527,   529,   530,   531,   528,   576,   577,
     578,   579,   580,   581,   582,   469,   624,     0,   615,   623,
     622,   621,   620,   617,   619,   616,   618,   506,    43,   224,
     225,   227,   226,   228,   220,   502,   503,   504,   501,   230,
     233,   229,   231,   232,   471,   824,   847,   941,   950,   948,
    1052,     0,   167,   168,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   127,   129,   130,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   281,   282,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   465,   468,
     455,     0,     0,   827,   857,   830,   888,   951,   832,   899,
     834,   910,   837,   839,   838,   928,  1036,  1043,   826,  1037,
     886,  1038,  1044,   911,  1042,  1045,   935,   992,  1039,  1040,
    1046,  1041,   993,   981,   978,   980,   979,   875,   997,   998,
     995,   841,   999,   889,   859,   977,   461,   824,   453,     0,
     193,   454,   457,   827,   828,   830,   829,   834,   835,   836,
     837,   826,   833,   510,     0,   459,   458,     0,     0,   213,
       0,   471,   832,   839,   838,     0,   169,   802,   999,     0,
       0,   221,     0,   511,     0,   543,     0,     0,   464,     0,
     435,   436,     0,   467,   466,   456,   439,     0,     0,   572,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   364,
     804,     0,   806,     0,     0,   166,     3,     4,     1,    10,
      12,     0,     0,     0,     0,     6,   166,     0,   166,     0,
       0,     0,   120,   173,     0,     0,   191,     0,     0,   200,
       0,     0,     0,     0,     0,     0,   236,   237,     0,     0,
     205,   234,     0,   255,   251,   257,   252,   254,   253,   261,
       0,     0,     0,     0,     0,     0,     0,   426,     0,     0,
     424,     0,   370,     0,   425,   418,   423,   422,   421,   420,
     419,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     412,   411,   408,     0,   169,   433,   438,     0,     0,     0,
     448,   488,   451,   450,   462,     0,     0,     0,   478,     0,
       0,     0,   584,   587,   585,   588,   592,   594,     0,   125,
       0,   823,    47,    44,    45,    48,    49,     0,     0,     0,
       0,    50,    51,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   661,   662,   663,     0,   104,   143,     0,
       0,   113,     0,     0,     0,     0,   128,     0,     0,   632,
       0,     0,     0,   627,     0,     0,     0,   642,     0,     0,
     259,   264,     0,     0,   135,     0,   139,   169,   525,     0,
      60,     0,    69,     0,     0,     0,    61,     0,     0,     0,
       0,   630,     0,     0,   633,     0,     0,     0,     0,   330,
       0,     0,   631,     0,     0,     0,   650,     0,     0,     0,
       0,   636,     0,     0,   195,     0,     0,   189,   188,     0,
       0,   808,     0,     0,     0,     0,   510,     0,   803,     0,
     512,   544,   543,   540,     0,     0,     0,   574,   573,   434,
       0,   571,     0,   674,   675,     0,     0,     0,     0,     0,
     817,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   680,     0,   203,   204,     0,     0,     0,     0,     0,
     208,   209,   260,   805,   807,     0,     5,    24,     0,    25,
       0,     7,    29,     0,    15,     8,    30,     0,    19,   941,
      79,    77,    16,    78,    20,     0,   194,     0,   192,   214,
     215,     0,     0,     0,     0,   299,   206,   235,   238,   239,
     258,     0,     0,   360,   362,   368,   374,   373,   372,     0,
     369,   366,   367,     0,   378,   385,   384,   382,   388,   389,
     390,   387,   392,   393,   396,   395,     0,     0,     0,     0,
     417,     0,   443,   444,     0,   489,     0,   485,   486,   487,
     482,   481,   518,   479,   520,     0,   625,   524,   516,     0,
       0,   122,   124,     0,     0,     0,   103,     0,    93,    95,
      96,    97,    98,   100,   101,   102,    94,    99,    89,    90,
       0,     0,   109,     0,   105,   107,   108,   115,     0,     0,
      88,    46,     0,     0,     0,     0,     0,     0,     0,   752,
     757,     0,     0,   753,   787,   740,   742,   743,   744,   746,
     748,   747,   745,   749,     0,     0,     0,     0,     0,   112,
       0,   145,     0,     0,   591,   583,   628,   629,     0,     0,
     646,     0,   643,     0,     0,     0,     0,     0,     0,     0,
       0,   141,     0,     0,   136,     0,     0,     0,     0,     0,
       0,    62,     0,     0,     0,   280,   286,   283,     0,   635,
     634,   641,   649,     0,     0,     0,     0,   590,     0,     0,
       0,   427,     0,     0,     0,     0,   639,     0,   637,     0,
     196,     0,     0,   810,     0,   471,   809,     0,     0,     0,
     543,   541,     0,   532,     0,     0,   514,   515,     0,    13,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   201,     0,
       0,     0,   688,     0,   210,     0,     0,     0,     0,   166,
       0,   166,     0,   526,   174,   247,     0,   314,   310,   312,
       0,   307,   300,   301,     0,     0,   240,   241,     0,   246,
     371,     0,     0,     0,     0,   718,   379,   691,   695,   697,
     699,   701,   704,   711,   712,   720,   951,   831,     0,   840,
     844,   843,   845,   846,     0,   398,   597,   604,   605,   608,
     653,   654,   609,   610,   613,   400,   402,   405,   614,   404,
     431,   490,   483,     0,     0,   480,   517,   126,    56,    57,
      54,    55,   132,   131,     0,    91,     0,     0,     0,   110,
     111,   116,    74,    75,    52,    53,    73,   758,     0,   761,
     788,     0,   751,   750,   755,   754,   786,     0,     0,   763,
       0,   759,   762,   741,     0,     0,     0,     0,     0,     0,
       0,   626,     0,     0,     0,   144,   147,   149,     0,     0,
       0,   114,   117,     0,     0,     0,   169,     0,     0,   650,
       0,     0,     0,   266,     0,   596,     0,   267,     0,     0,
       0,     0,     0,     0,     0,   142,   137,   140,     0,   190,
       0,     0,    71,    65,    68,    67,     0,    63,   284,     0,
       0,   169,     0,   328,   332,     0,     0,     0,   335,     0,
       0,   341,     0,     0,   348,     0,   352,     0,   429,   428,
     169,     0,     0,   197,     0,   199,   329,     0,     0,     0,
       0,     0,   544,     0,   534,     0,   567,   564,     0,   568,
     569,     0,     0,   563,     0,   538,   566,   565,     0,     0,
       0,     0,   667,   668,   664,     0,     0,   672,   673,   669,
     813,   814,   678,   818,   676,     0,     0,     0,     0,   682,
     832,   839,   838,   999,   202,     0,     0,     0,   683,   684,
       0,     0,   211,   816,    26,     0,    17,    21,    22,   311,
     323,     0,   324,     0,   315,   316,   317,   318,     0,   510,
       0,   308,     0,     0,     0,   242,     0,     0,     0,     0,
     293,     0,   290,   295,   245,     0,     0,   702,   715,     0,
     376,   380,     0,   734,     0,     0,     0,     0,     0,     0,
     690,   692,   693,   729,   730,   731,   733,   732,     0,     0,
     706,   705,     0,   709,   713,   727,   725,   724,   717,   721,
       0,     0,   600,     0,     0,     0,     0,     0,     0,     0,
     655,   656,   601,   603,   602,   598,   406,   484,   519,   521,
     134,   133,    92,   106,   775,   756,     0,   780,     0,   780,
     769,   764,   148,   150,     0,     0,     0,   118,   146,     0,
      27,     0,   647,   648,   651,   644,   645,   265,     0,     0,
     279,   271,     0,   275,     0,   269,     0,     0,     0,   138,
       0,     0,    70,     0,    64,   285,     0,     0,   331,   333,
     338,     0,   336,     0,     0,     0,     0,     0,   342,     0,
       0,   356,     0,     0,   349,     0,     0,   353,   430,     0,
     640,   638,   198,   811,   812,     0,     0,   543,     0,     0,
     575,     0,   543,     0,   539,     0,    14,     0,     0,     0,
       0,     0,     0,     0,   681,     0,     0,     0,     0,     0,
       0,   685,   689,   327,   325,   326,   319,   320,   321,   313,
       0,     0,   309,   302,   288,     0,     0,     0,   244,   291,
     294,     0,   719,   710,   716,     0,     0,   789,   790,   800,
     799,   798,     0,     0,     0,     0,   791,   696,   797,     0,
     694,   698,     0,     0,   703,   707,     0,   728,   723,   726,
     722,     0,     0,   611,     0,   606,   660,   599,   822,   821,
     820,   607,   776,     0,     0,   774,   781,   782,   778,     0,
     773,     0,   771,     0,     0,     0,     0,     0,     0,   586,
     268,     0,   277,     0,     0,   273,     0,   276,     0,     0,
       0,    66,   287,   593,     0,     0,   339,   343,   337,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   589,     0,     0,   543,     0,   570,     0,   543,   595,
     665,   666,   670,   671,     0,   677,   819,   679,     0,     0,
       0,     0,   686,   322,   303,     0,   289,   297,   298,   292,
     243,   714,   801,     0,     0,   793,     0,   739,   738,   737,
     736,   735,   700,     0,   792,     0,   657,     0,     0,   785,
     784,   783,     0,   777,   770,   768,     0,   765,   766,   760,
     151,   153,   155,     0,     0,     0,     0,     0,   272,   270,
       0,   278,   207,   358,    72,   334,   340,     0,   357,   354,
     350,     0,     0,     0,   344,     0,     0,   346,     0,   553,
     547,   542,     0,   543,   533,     0,   815,     0,     0,     0,
       0,   306,   304,     0,   795,   794,     0,     0,   658,   612,
       0,   779,   772,     0,     0,   157,   156,     0,     0,     0,
       0,   152,   274,     0,     0,     0,     0,     0,   561,   555,
       0,   554,   556,   562,   559,   549,     0,   548,   550,   560,
     535,     0,   536,     0,     0,   687,   305,     0,   796,   708,
     659,   767,   154,   158,     0,     0,     0,     0,     0,     0,
     347,   345,     0,     0,   546,   557,   558,   545,   551,   552,
     537,     0,   296,   159,     0,     0,     0,   355,   351,     0,
       0,     0,     0,     0,     0,     0,     0,   161,     0,     0,
     160,     0,     0,     0,     0,     0,     0,     0,     0,   162
  };

  const short
  jsoniq_parser::yypgoto_[] =
  {
   -1527, -1527,  -218,  -185, -1527,  1237,  1239, -1527,  1240,  -540,
   -1527, -1527, -1527, -1527, -1527, -1527, -1527, -1527,  -610, -1527,
   -1040, -1527, -1527, -1527, -1527,  -234,  -572, -1527,   742,   -36,
   -1527, -1527, -1527, -1527, -1527,   281,   509, -1527, -1527,     0,
    -280,  1071, -1527,  1060, -1527, -1527,  -577, -1527,   451, -1527,
     242, -1527,  -241,  -287, -1527,  -546, -1527,    43,   209,    50,
    -226,  -188, -1527,  -861, -1527, -1527,   228, -1527, -1527, -1527,
   -1527, -1527, -1527, -1527, -1527, -1527, -1527,   628, -1527,    97,
    2289,    17, -1527, -1527,  -322, -1527, -1527,   350, -1527, -1527,
    -311, -1527,    34, -1527, -1527,  -107,  -952,  -781,  -727, -1527,
   -1527,   712, -1527, -1527,   -76,   104, -1527, -1527, -1527,   114,
   -1240, -1527,   369,   120, -1527, -1527,   126, -1334, -1527,   965,
     221, -1527, -1527,   215, -1051, -1527, -1527,   214, -1527, -1527,
   -1280, -1278, -1527,   212, -1447, -1527, -1527,   866,  -158, -1527,
   -1527,  -522,   847, -1527, -1527,  -694,   328,  -680,   333,   349,
   -1527, -1527, -1527,   579, -1527, -1527,  1200, -1527, -1527, -1527,
   -1527, -1527,  -879,  -339,  -684, -1527,   -66, -1527, -1527, -1527,
   -1527, -1527, -1527, -1527,     6,  -854, -1527,  -570,   661,   244,
   -1527,  -371, -1527,   818, -1527,   819, -1527, -1527, -1527,   815,
   -1527, -1527, -1527, -1527, -1037, -1527,   202, -1527,   703,  -838,
   -1527, -1527, -1527, -1527, -1527,  -279,  -267, -1249,  -962, -1527,
   -1527, -1527, -1527, -1527, -1527, -1527, -1527, -1527, -1527, -1527,
    -745,  -926,  -221,  -898, -1527, -1527, -1527,  -767, -1527, -1527,
   -1527, -1527, -1527, -1527, -1527, -1527, -1527,  1088,  1090,  -198,
     507,  -721, -1527,   362,   367, -1527, -1527, -1527, -1527, -1527,
   -1527, -1527, -1527,   186, -1527, -1527,   176, -1527,   178, -1087,
   -1527, -1527, -1527,   129,   122,   -54,   398, -1527, -1527, -1527,
   -1527, -1527, -1527, -1527, -1527, -1527, -1527, -1527, -1527, -1527,
     134, -1527, -1527, -1527,   -47,   391,   540, -1527, -1527, -1527,
   -1527, -1527,   326, -1527, -1527, -1526, -1527, -1527, -1527,  -603,
   -1527,    88, -1527,   -68, -1527, -1527, -1527, -1527, -1348, -1527,
     142, -1527, -1527, -1527, -1527, -1527,   975, -1527, -1527, -1527,
   -1527, -1527,  -949,  -396,  1623,   987, -1527
  };

  const short
  jsoniq_parser::yydefgoto_[] =
  {
       0,   265,   648,   266,   267,   268,   269,   270,   271,   272,
     273,   274,   275,   276,   277,   278,   279,   280,   281,   282,
    1302,   805,   283,   284,   285,   286,   287,   288,   998,   999,
    1000,   289,   290,   291,  1004,  1005,  1006,   292,   460,   579,
     294,   738,   295,   462,   463,   464,   474,   795,   796,   296,
    1265,   297,  1750,  1751,   298,   299,   300,   567,   301,   302,
     303,   304,   305,   798,   306,   307,   550,   308,   309,   310,
     311,   312,   313,   660,   314,   315,   890,   891,   316,   317,
     318,   581,   320,   661,  1687,   673,  1168,  1169,   321,   663,
     322,   665,   582,   324,   325,   790,   791,  1420,   481,   326,
     482,   483,   932,  1421,  1422,  1423,   666,   667,  1162,  1163,
    1411,   668,  1158,  1159,  1404,  1405,  1406,  1407,   327,   818,
     819,   328,  1317,  1318,  1533,   329,  1320,  1321,   330,   331,
    1323,  1324,  1325,  1326,  1540,   332,   333,   334,   335,   336,
     939,   337,   338,  1430,   339,   340,   341,   342,   343,   344,
     345,   346,   347,  1206,   348,   349,   350,   351,   691,   692,
     352,   353,   354,   355,   356,   357,   358,   359,   360,   361,
     362,   363,   364,   365,   366,   367,   368,   369,   728,   721,
     370,   371,   372,   373,   374,   375,   376,   377,   378,   975,
     379,   380,   381,   382,   383,   384,  1354,   852,   853,   854,
    1781,  1826,  1827,  1820,  1821,  1828,  1822,  1355,  1356,   385,
     386,  1357,   387,   388,   389,   390,   391,   392,   393,   394,
    1060,  1541,  1475,  1196,  1634,  1197,  1207,   395,   396,   397,
     398,   399,   400,   401,   402,   403,   404,   405,   406,   825,
    1278,   407,  1199,  1200,  1201,  1202,   408,   409,   410,   411,
     412,   413,   880,   881,   414,  1388,  1389,  1712,  1143,  1176,
    1440,  1441,  1177,  1178,  1179,  1180,  1181,  1450,  1624,  1625,
    1182,  1453,  1183,  1605,  1184,  1185,  1458,  1459,  1630,  1628,
    1442,  1443,  1444,  1445,  1730,   767,  1025,  1026,  1027,  1028,
    1029,  1030,  1251,  1654,  1747,  1252,  1652,  1745,  1031,  1489,
    1649,  1645,  1646,  1647,  1032,  1033,  1446,  1454,  1615,  1447,
    1611,  1431,   415,   416,   417,   418,   570,   419,   420,   421,
     422,   423,  1203,  1303,   424,   425,   426
  };

  const short
  jsoniq_parser::yytable_[] =
  {
     293,   662,   617,   947,   969,   293,   293,   293,   293,   293,
     293,  1058,   664,   642,  1112,   715,   461,   319,   948,   949,
     950,   951,   319,   319,   319,   319,   319,   319,   910,   960,
    1195,  1205,  1307,   870,   323,  1142,   634,  1319,   877,   323,
     323,   323,   323,   323,   323,  1544,   742,   619,   431,   434,
     435,   436,   437,   637,  1153,   433,   433,   433,   433,   433,
     439,  1556,  1074,   915,  1174,  1059,   911,   913,   639,   739,
     739,  1587,  1588,   635,   596,  1086,   643,  1353,   626,   627,
    1291,   802,   806,   635,   546,  1426,  1267,    13,    14,  1100,
     904,   908,   912,   914,  1254,  1685,   902,   906,  1689,  1255,
     781,  1283,    15,  1228,  1626,  1564,   607,   446,  1256,  1165,
     479,   635,   635,   293,   635,  1257,  1054,  1258,   782,  1288,
    1753,   741,   635,   695,  1223,   635,  1259,  1748,   635,  1285,
     319,  1612,   635,   635,   840,   449,  1296,   635,    13,    14,
     696,  1056,   601,  1779,   602,   635,  1055,   323,   980,   635,
     635,  1118,  1516,  1260,   935,  1133,   936,   937,   446,   938,
    1014,   940,  1015,   568,   645,   635,   635,   980,   447,   941,
     942,   839,   985,   773,   440,  1055,   635,  1056,  1455,  1456,
     714,   642,   647,   635,   442,   443,   449,   444,   445,  1198,
    1198,   450,   647,  1292,   635,   608,   598,   635,  1242,  1416,
    1085,  1457,   468,   130,  1417,   448,  1016,   643,  1309,  1243,
    1119,   580,   584,   643,   432,   432,   432,   432,   432,   438,
     647,   857,  1175,   451,   452,  1152,   635,  1171,   981,  1057,
    1172,   603,  1754,  1780,  1198,   635,   635,   635,  1727,   635,
    -170,   635,  1056,  1135,  1134,  1728,   647,   982,   646,   480,
     130,   635,  1713,   801,   647,  1729,  -171,   635,   647,   647,
     849,  1395,  1231,   293,   293,  1129,  1524,   293,  1266,  1537,
     480,   664,  1343,  1166,   647,   647,   635,  1841,  1544,  1293,
     319,   319,   480,  1432,   319,   647,  1261,  1128,  1198,   910,
    1463,   716,   647,  1174,  1174,  1198,  1613,   323,   323,   642,
     850,   323,   293,   647,  1746,   636,   647,   638,  1353,   454,
    1564,  1346,  1347,  1167,  1755,   897,  1295,  1353,   899,   319,
    1813,  1149,   432,   480,  1267,  1151,  1394,   911,   913,  1463,
    1507,  1289,  1432,  1294,  1066,   647,   323,  1173,  1088,  1089,
     635,  1604,   643,   480,   647,   647,   647,  1310,   647,   635,
     647,   621,   623,   774,  1139,  1339,  1433,   902,   906,   480,
     647,   775,  1017,  1481,  1011,  1340,   647,  1849,   722,  1852,
     848,   724,  1519,   455,  1316,  1018,  1136,  1019,  1262,  1263,
    1264,   599,  1612,  1341,  1083,   647,  1477,  1531,  1020,  1021,
    1022,   858,  1023,   894,  1024,   635,   741,  1049,  1534,   644,
     494,   893,   903,   907,   457,  1433,   635,  1083,  1348,  1071,
    1224,   635,   458,  1211,   456,   898,   900,  1170,  1805,  1414,
     459,  1198,  1113,  1048,  1787,  1788,   917,  -170,  1789,  1349,
     781,  1350,    15,   293,   635,   635,   926,  1078,  1055,   293,
     896,  1051,   441,  -171,  1434,  1435,   840,  1436,   782,   647,
     319,  1175,  1175,   604,  1437,   605,   319,  1120,   647,  1351,
    1198,  1269,  1094,   116,  1438,  1084,   293,   323,   293,   695,
    1270,  1344,  1843,   323,  1791,  1723,  1439,  1050,   495,   128,
     803,  1063,  1095,   319,   656,   319,   696,   657,  1102,  1114,
     293,  1097,  1413,  1434,  1435,   293,  1436,   489,  1643,  1124,
     323,  1352,   323,  1509,   647,  1482,  1266,   319,   757,  1834,
     758,   892,   319,  1438,   793,   647,   439,   934,   439,   156,
     647,  1052,  1353,   841,   323,  1439,  1115,  1497,   804,   323,
     644,  1198,   465,  1142,  1633,  1116,   644,  1582,  1345,   910,
     439,   910,   606,   647,   647,   439,  1486,  1613,  1614,  1174,
     496,   497,  1836,    15,  1724,  1198,  1544,  1171,  1174,   725,
    1172,  1064,  1096,  1644,  1376,   930,  1517,   786,   293,  1174,
     760,  1098,   799,   783,   669,  1056,   293,   911,   913,   911,
     130,  1117,   812,   794,  1494,   319,  1121,  1122,   498,   823,
     824,   650,  1141,   319,   726,   651,   727,  1602,   830,   743,
     652,  1123,   323,  1396,  1397,  1398,   757,  1488,   758,  1217,
     323,  1055,  1232,  1233,   744,  1214,   293,  1236,  1818,   653,
    1835,  1839,  1346,   717,   718,   293,   433,  1508,  1125,  1126,
    1764,   461,   761,   319,    84,  1214,   293,   293,   293,   293,
     670,   759,   319,  1127,  1818,   671,  1737,  1275,  1346,    95,
     323,   962,   963,   319,   319,   319,   319,  1056,  1742,   323,
    1073,   643,   293,  1146,  1591,   644,   887,  1360,   760,  1361,
     323,   323,   323,   323,   109,   778,   751,   780,  1686,   319,
    1248,  1573,   747,   855,   856,  1150,  1346,  1347,  1248,   900,
    1778,  1154,   672,  1198,   860,  1215,   323,  1249,   748,   821,
     905,  1346,    13,    14,   827,  1249,   472,  1175,  -262,  1663,
     752,  1486,   886,   888,   293,  1216,  1175,   698,   654,  1693,
    1390,   655,   640,  1824,  1697,  1711,  1738,  1175,  1487,  1131,
     761,   319,  1346,  1664,  1148,   469,   656,  1537,  1743,   657,
    1349,  1250,  1350,  1174,  1055,  1619,   656,   699,   323,   657,
    1819,  1055,   473,   905,  1824,   725,  1768,   961,   656,  1401,
     700,   657,  1416,   658,   433,   745,  1349,  1417,  1350,   618,
    1351,  -263,  1488,  1607,  -262,  1198,  1855,  1391,  1198,  -262,
     746,  1403,  1391,  1348,   910,   432,  1418,  1147,   641,  1472,
     726,   695,   727,  1099,   631,   701,  1351,   293,   702,  1001,
    1590,  1428,   475,   632,  1349,  1056,  1350,   703,   696,  1798,
     632,  1799,   659,  1608,   319,  1658,   964,  1823,  1829,  1349,
    1473,  1350,   911,   964,   130,   730,  -262,  1419,   293,   293,
    1825,   323,   697,   725,  1351,   715,   731,  -263,   116,  1474,
    1065,   695,  -263,  1002,   484,   319,   319,   433,  1003,  1351,
    1349,  1277,  1350,  1280,   128,  1148,  1782,   476,   696,  1823,
    1785,  1858,   323,   323,   500,  1829,   477,   704,   726,   916,
     727,   632,   771,   918,  1840,  1400,  1558,   772,   439,   439,
    1351,   705,  1796,    13,    14,   656,  1401,   293,   657,  -263,
    1222,  1346,  1347,   478,   156,   501,  1402,  1331,   502,  1332,
    1218,  1175,  1220,   640,   319,  1522,   797,   609,  1403,   461,
    1682,  1480,   610,  1384,   642,  1198,  1219,   293,  1221,  1725,
    1726,   323,   917,   432,  1346,  1347,  1722,   797,   900,    13,
      14,  1609,  1234,  1235,   319,  1316,  1727,  1542,  1610,  1545,
    1107,  1108,  1109,  1728,   888,  1831,  1542,  1542,  1536,  1469,
    1543,   323,  1546,  1729,   487,  1392,  1194,  1194,  1244,  1692,
    1861,  1055,  1225,   488,   965,    13,    14,   965,  1198,  1245,
    1198,  1752,   965,  1401,  1225,   442,   443,   492,   444,   445,
    1237,  1238,  1239,   493,  1140,   446,  1240,  1002,  1348,  1416,
     466,   499,  1003,   467,  1417,   447,   448,  1535,   548,   470,
    1198,  1194,   471,  1584,  1585,  1583,   432,  1806,   549,  1349,
     551,  1350,   485,   449,  1592,   486,   490,   656,   450,   491,
     657,  1348,   720,   945,   946,   723,   763,   764,   765,  1794,
    1795,   552,  1365,  1198,  1366,   952,   953,  1092,  1093,  1351,
    1739,  1740,  1349,   116,  1350,  1492,  1493,  1198,   564,  1198,
     451,   452,   453,   954,   955,  1194,  1575,   565,   566,   128,
     586,   587,  1194,  1213,  1579,   547,   588,   593,   594,   595,
     597,   600,  1351,  1752,  1520,   664,   116,  -626,   611,   547,
     612,  1563,   615,   616,   613,   624,   628,   614,   649,   674,
     293,   675,   128,   694,   706,   693,   707,   708,   713,   156,
     454,   732,  1838,   547,   709,   719,   729,   319,   733,   741,
     734,   735,   736,   753,  1695,   547,   737,   547,   755,   756,
    1752,   766,   770,   797,   323,   779,   800,  1671,   808,   809,
     810,   828,   156,   829,   811,   822,   835,   467,   471,   293,
     439,   486,   491,   837,   502,   843,   839,   842,   461,   293,
     293,   293,   844,   851,   845,   859,  1385,   847,   862,   879,
     889,   883,   884,   901,   455,   921,   319,   319,   319,   861,
     923,   928,   922,  1386,   931,   924,   943,   929,   956,   957,
     958,   959,   977,   323,   323,   323,   983,   986,  1194,   984,
    1008,  1009,  1880,   976,  1881,   617,  1012,  1013,  1035,  1040,
    1036,  1038,  1034,  1043,  1889,   456,  1044,  1046,  1047,  1674,
    1045,  1675,  1053,  1068,  1679,  1680,  1062,  1069,  1683,  1072,
    1079,  1080,  1081,  1082,  1690,  1087,  1090,  1194,  1091,   466,
     470,   457,   676,  1130,  1132,  1137,  1145,   485,   490,   458,
     677,   678,   501,   679,  1138,  1210,  1226,   459,  1156,  1055,
    1003,   680,  1246,  1247,  1273,   681,  1274,   682,  1284,  1268,
    1286,   793,   683,  1276,  1290,  1298,  1301,  1305,  1425,  1304,
    1300,  1306,  1311,  1315,  1299,  1312,   293,  1316,   684,   988,
     989,   990,   991,   992,   993,   994,   995,   996,   997,  1322,
    1330,  1334,  1335,   319,  1328,  1329,  1337,  1342,  1194,  1327,
     293,  1338,  1359,   685,   686,   687,   688,   689,   690,  1677,
     323,   293,  1377,  1358,   759,  1387,  1408,   319,  1410,  1501,
    1412,  1415,  1194,  1448,  1449,  1460,   433,  1429,   319,  1452,
     293,  1476,    15,  1461,   323,  1464,  1485,  1465,  1466,  1467,
    1468,  1484,  1490,  1495,  1499,   323,  1504,   319,  1502,   547,
    1496,   293,   547,   926,  1527,  1500,  1503,  1505,   293,  1506,
    1514,   433,  1512,  1772,   323,  1518,  1523,  1776,   319,  1526,
    1542,  1548,  1550,  1549,  1557,   319,  1560,  1551,  1567,  1566,
     433,  1571,  1568,  1555,  1569,   323,  1576,  1570,  1577,  1572,
     293,  1578,   323,  1390,  1401,  1595,  1596,  1597,  1601,  1622,
    1606,   439,  1603,  1629,  1642,  1616,  1627,  1385,   439,  1635,
    1648,  1623,  1636,  1651,  1637,  1638,  1653,  1639,  1655,  1661,
    1640,  1659,  1666,  1641,  1386,  1668,  1669,  1657,  1670,  1673,
    1691,  1733,  1715,   749,   547,  1704,  1696,   547,  1699,  1721,
     887,   547,  1762,  1735,   547,   547,  1744,  1734,  1736,  1248,
    1757,  1815,   547,  1766,  1677,  1749,  1760,  1769,   547,   547,
    1194,  1767,  1784,  1793,  1777,  1800,   547,  1801,  1802,  1803,
    1804,  1814,  1808,   547,   547,   547,   547,  1816,  1837,  1809,
    1830,   547,  1832,  1756,  1846,   432,  1833,  1847,  1854,   547,
    1860,  1848,  1866,  1857,  1871,  1853,  1882,  1883,  1872,  1885,
    1886,  1010,  1844,  1864,   629,  1873,   630,  1876,  1878,  1483,
     633,  1229,   740,  1888,  1297,  1498,  1811,  1842,  1144,  1424,
     432,   776,  1077,  1719,  1600,  1399,  1593,  1869,  1589,   836,
    1870,  1586,  1532,  1528,  1538,   293,   547,  1547,  1209,   432,
     933,   944,  1194,   967,   968,  1194,   979,  1559,  1859,   712,
    1769,   547,   319,  1879,  1856,  1111,  1470,   784,  1281,   785,
    1561,  1471,   547,  1574,  1581,  1580,  1617,  1565,  1732,   323,
    1621,  1427,  1731,  1451,  1253,  1620,  1491,  1650,  1741,   293,
    1618,   875,     0,     0,     0,     0,     0,     0,     0,  1850,
     871,     0,     0,     0,     0,   871,  1385,     0,   547,     0,
       0,     0,     0,     0,     0,   547,     0,  1520,     0,     0,
       0,     0,     0,  1386,     0,     0,     0,     0,     0,     0,
       0,     0,  1867,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1867,     0,     0,     0,     0,     0,
       0,     0,     0,  1850,     0,     0,     0,     0,     0,   547,
       0,     0,     0,     0,     0,     0,     0,     0,   293,     0,
       0,     0,     0,  1708,  1709,  1710,     0,     0,     0,   293,
     293,     0,  1194,   293,     0,  1385,     0,     0,     0,     0,
     293,     0,     0,     0,     0,     0,  1385,  1385,     0,     0,
     319,     0,  1386,     0,     0,     0,     0,  1385,     0,     0,
       0,     0,   547,  1386,  1386,   563,     0,   323,     0,     0,
       0,     0,     0,     0,  1386,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1194,   571,  1194,     0,   585,
     987,     0,     0,   547,     0,     0,     0,     0,     0,     0,
       0,   589,     0,   592,     0,     0,     0,   547,     0,     0,
       0,   547,   547,     0,     0,     0,     0,  1194,     0,     0,
       0,     0,     0,     0,     0,     0,   293,     0,     0,     0,
     547,     0,     0,     0,     0,     0,     0,     0,   547,     0,
       0,   547,     0,  1385,     0,   547,     0,     0,     0,     0,
    1194,     0,     0,     0,     0,     0,     0,     0,  1790,     0,
    1386,     0,     0,     0,  1194,   293,  1194,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1797,     0,   319,   293,     0,     0,     0,     0,   547,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   293,   323,
    1385,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   319,     0,  1386,     0,     0,
     293,     0,     0,     0,     0,     0,   547,     0,     0,   293,
       0,     0,   323,     0,     0,     0,     0,  1385,     0,     0,
       0,     0,     0,     0,     0,     0,  1385,     0,     0,     0,
       0,     0,     0,     0,  1386,     0,     0,     0,   547,     0,
       0,     0,     0,  1386,     0,     0,     0,     0,   547,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   547,   547,   547,   547,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   547,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   547,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   547,     0,     0,
       0,     0,     0,     0,     0,     0,   547,     0,   547,     0,
     547,     0,   547,     0,     0,     0,     0,     0,     0,   547,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     750,     0,     0,   754,     0,     0,     0,   762,     0,     0,
     768,   769,   547,     0,   547,     0,     0,     0,   777,     0,
       0,     0,     0,     0,   789,   792,     0,     0,     0,     0,
       0,     0,   807,     0,     0,     0,     0,     0,     0,   815,
     816,   817,   820,     0,     0,     0,     0,   826,     0,     0,
       0,   871,     0,     0,     0,   833,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   547,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   834,     0,     0,   547,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   820,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   846,   571,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   547,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   571,
       0,     0,     0,     0,   882,     0,     0,     0,     0,     0,
       0,   792,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   547,
       0,     0,     0,   547,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   547,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   792,     0,     0,     0,   547,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   547,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   547,
       0,     0,     0,     0,     0,     0,     0,     0,   970,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   547,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   547,     0,     0,     0,   547,  1007,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1037,     0,     0,     0,  1041,  1042,     0,
       0,     0,   569,     0,     0,     0,   547,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1061,     0,     0,     0,
       0,     0,     0,     0,  1067,     0,     0,  1070,     0,     0,
       0,  1075,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   547,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1105,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1110,     0,     0,     0,     0,     0,
       0,     0,   547,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   547,     0,     0,   547,
       0,     0,   547,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1155,   547,     0,     0,     0,     0,
     547,     0,     0,     0,  1164,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1204,
    1204,  1208,  1208,   547,   547,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1204,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1241,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1272,     0,     0,     0,     0,     0,   547,
       0,   547,  1279,   547,  1279,   547,   792,     0,  1204,     0,
       0,     0,   547,     0,     0,  1204,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   826,     0,
     826,     0,   547,     0,   547,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     547,     0,     0,   547,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   547,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   547,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1409,     0,     0,     0,   547,     0,     0,
       0,     0,     0,     0,     0,   547,     0,     0,     0,     0,
       0,   547,     0,   547,     0,     0,     0,     0,     0,     0,
       0,  1204,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   547,     0,     0,     0,     0,     0,
       0,     0,     0,   547,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1204,     0,     0,   547,     0,     0,     0,     0,   547,     0,
       0,     0,     0,     0,     0,   569,     0,     0,     0,     0,
     547,     0,     0,   547,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     863,   864,     0,   872,   873,   874,   876,     0,   878,     0,
       0,     0,     0,   885,     0,   792,     0,     0,     0,  1510,
       0,     0,     0,   895,     0,     0,     0,     0,     0,     0,
       0,  1204,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   820,   919,   920,     0,     0,
       0,     0,     0,     0,     0,  1204,     0,   925,     0,     0,
       0,   927,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1562,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     882,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   974,     0,     0,     0,
     974,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1599,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1039,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1204,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1076,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1656,     0,
       0,     0,     0,     0,  1101,     0,     0,     0,  1103,  1104,
       0,  1106,     0,     0,     0,     0,     0,     0,   927,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1681,     0,     0,  1204,     0,     0,  1204,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1694,     0,     0,     0,     0,  1698,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   920,     0,     0,     0,
       0,     0,  1157,  1161,     0,     0,     0,     0,     0,  1717,
    1718,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1230,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1204,     0,  1771,     0,  1773,
       0,  1775,     0,     0,     0,     0,     0,     0,  1783,     0,
       0,     0,     0,     0,     0,     0,     0,  1271,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1287,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1204,     0,
    1204,     0,  1308,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1313,  1314,     0,  1807,     0,     0,  1810,
       0,     0,     0,     0,     0,     0,     0,     0,  1333,     0,
    1204,  1336,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1817,     0,     0,     0,     0,     0,     0,     0,     0,
    1362,  1363,  1364,     0,  1367,  1368,  1369,  1370,  1371,  1372,
    1373,  1374,  1375,  1204,     0,  1378,  1379,     0,     0,     0,
       0,  1845,     0,     0,  1393,     0,     0,  1204,     0,  1204,
       0,     0,     0,     0,     0,  1157,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1862,     0,     0,     0,     0,     0,     0,     0,     0,  1865,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1874,
       0,     0,     0,     0,  1875,     0,     0,     0,     0,     0,
       0,     0,     0,  1479,     0,     0,  1884,     0,     0,  1887,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1511,     0,  1513,
       0,  1515,     0,     0,  -888,   472,     0,  -262,     0,  1521,
       0,     0,     0,     0,     0,     0,     0,     0,  1525,     0,
       0,     0,     0,     0,  1529,  1530,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1552,     0,     0,  1553,  1554,  -888,     0,
       0,     0,  -888,     0,     0,     0,  -888,  -888,     0,     0,
       0,     0,  -888,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  -262,     0,     0,  -888,     0,  -262,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  -888,
       0,  -888,  -888,  -888,     0,     0,  -888,  -888,  -888,  -888,
       0,     0,     0,  -888,  -888,     0,     0,  1157,     0,     0,
    -888,  1161,  1594,  -888,  -888,  -262,     0,  1598,     0,  -888,
       0,     0,     0,     0,  -888,  -888,     0,     0,     0,     0,
    -888,     0,     0,     0,  -888,     0,     0,     0,  -888,  -888,
       0,  -888,     0,  -888,  -888,     0,     0,     0,  -888,  -888,
       0,     0,  -888,  -888,  -888,  -888,  -888,  -888,     0,     0,
    -888,     0,     0,     0,  -888,  -888,     0,     0,  -888,     0,
       0,     0,     0,  -888,     0,     0,  -888,     0,     0,     0,
       0,  -888,  -888,  -888,  -888,  -888,  -888,     0,  -888,  -888,
    -888,  -888,  -888,     0,     0,     0,     0,  -888,  -888,  -888,
       0,  -888,  -888,  -888,  -888,  -888,  -888,  1660,  -888,     0,
    -888,  1662,     0,     0,     0,  1665,  -888,  1667,     0,     0,
    -888,  -888,     0,     0,     0,  1672,     0,     0,     0,     0,
       0,     0,     0,  1676,  1678,  -888,     0,  -888,     0,  -888,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  -888,     0,     0,  1521,     0,  -888,     0,     0,  -888,
       0,     0,     0,     0,     0,     0,  1700,  1701,  1702,  1703,
       0,  1705,  1706,     0,  1707,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1714,
       0,     0,     0,     0,  1716,     0,     0,     0,     0,     0,
    1720,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  -888,     0,  -888,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1758,     0,  1759,     0,     0,  1761,     0,     0,  1763,     0,
       0,     0,     0,  1765,  1678,     0,     0,     0,     0,     0,
       0,     0,  1770,     0,     0,     0,  1774,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1786,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1792,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1812,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1770,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1851,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1863,     0,     0,     0,  1868,     0,     0,
       0,     0,  1851,  -166,   905,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     1,     2,     0,     0,  1868,     3,
       0,     0,  1877,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,     0,    15,     0,     0,    16,   428,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
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
     253,   254,     0,   255,     0,   256,   257,   258,   259,   260,
     261,   262,  -166,   905,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     1,     2,     0,     0,     0,     3,     0,
       0,     0,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,     0,    15,     0,     0,   909,   428,     0,
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
     254,   905,   255,     0,   256,   257,   258,   259,   260,   261,
     262,     1,     2,     0,     0,     0,     3,     0,     0,     0,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,     0,    15,     0,     0,   427,   428,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,   429,    32,    33,    34,    35,    36,     0,    37,    38,
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
     246,   247,   248,   249,   250,   251,   252,   253,   254,   905,
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
     256,   257,   258,   259,   260,   261,   262,  -889,   618,     0,
    -263,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  -889,     0,     0,     0,  -889,     0,     0,     0,  -889,
    -889,     0,     0,     0,     0,  -889,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  -263,     0,     0,  -889,
       0,  -263,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  -889,     0,  -889,  -889,  -889,     0,     0,  -889,
    -889,  -889,  -889,     0,     0,     0,  -889,  -889,     0,     0,
       0,     0,     0,  -889,     0,     0,  -889,  -889,  -263,     0,
       0,     0,  -889,     0,     0,     0,     0,  -889,  -889,     0,
       0,     0,     0,  -889,     0,     0,     0,  -889,     0,     0,
       0,  -889,  -889,     0,  -889,     0,  -889,  -889,     0,     0,
       0,  -889,  -889,     0,     0,  -889,  -889,  -889,  -889,  -889,
    -889,     0,     0,  -889,     0,     0,     0,  -889,  -889,     0,
       0,  -889,     0,     0,     0,     0,  -889,     0,     0,  -889,
       0,     0,     0,     0,  -889,  -889,  -889,  -889,  -889,  -889,
       0,  -889,  -889,  -889,  -889,  -889,     0,     0,     0,     0,
    -889,  -889,  -889,     0,  -889,  -889,  -889,  -889,  -889,  -889,
       0,  -889,     0,  -889,     0,     0,     0,     0,     0,  -889,
       0,     0,     0,  -889,  -889,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  -889,     0,
    -889,     0,  -889,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  -889,     0,     0,     0,     0,  -889,
       0,     0,  -889,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     1,     2,     0,     0,
       0,     3,     0,     0,     0,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,  -889,    15,  -889,     0,
      16,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
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
     154,   155,   156,     0,   157,   158,   159,   160,   161,   162,
     163,   164,   165,   166,   167,   168,   169,   170,   171,   172,
     173,   174,   175,   176,   177,   178,   179,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,   197,   198,   199,   200,   201,   202,
     203,     0,   204,   205,   206,     0,   207,   208,   209,   210,
     211,   212,   213,   214,   215,   216,   217,   218,   219,   220,
     221,   222,   223,   224,   225,   226,   227,   228,   229,   230,
     231,   232,   233,   234,   235,   236,   237,   238,   239,   240,
     241,   242,   243,   244,   245,   246,   247,   248,   249,   250,
     251,   252,   253,   254,     0,   255,     0,   256,   257,   258,
     259,   260,   261,   262,   263,   264,     1,     2,     0,     0,
       0,     3,     0,     0,     0,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,     0,    15,     0,     0,
      16,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
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
     154,   155,   156,     0,   157,   158,   159,   160,   161,   162,
     163,   164,   165,   166,   167,   168,   169,   170,   171,   172,
     173,   174,   175,   176,   177,   178,   179,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,   197,   198,   199,   200,   201,   202,
     203,     0,   204,   205,   206,     0,   207,   208,   209,   210,
     211,   212,   213,   214,   215,   216,   217,   218,   219,   220,
     221,   222,   223,   224,   225,   226,   227,   228,   229,   230,
     231,   232,   233,   234,   235,   236,   237,   238,   239,   240,
     241,   242,   243,   244,   245,   246,   247,   248,   249,   250,
     251,   252,   253,   254,     0,   255,     0,   256,   257,   258,
     259,   260,   261,   262,     0,   625,     1,     2,     0,     0,
       0,     3,     0,     0,     0,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,     0,    15,     0,     0,
     427,   428,     0,    18,    19,    20,    21,    22,    23,    24,
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
     133,     0,   838,     0,   134,     0,   135,   136,   137,   138,
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
     251,   252,   253,   254,     0,   255,     0,   256,   257,   258,
     259,   260,   261,   262,     1,     2,     0,     0,     0,     3,
       0,     0,     0,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,     0,    15,     0,     0,   427,   428,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,   572,   429,    32,    33,    34,    35,    36,
       0,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,     0,    50,    51,    52,   573,   574,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,     0,
      74,    75,    76,    77,    78,     0,    79,    80,    81,     0,
       0,    82,    83,     0,     0,     0,    84,    85,   520,    87,
      88,    89,    90,    91,   575,    93,     0,    94,     0,     0,
       0,    95,     0,    96,    97,    98,    99,     0,   100,     0,
       0,     0,   101,   102,   103,   104,   105,     0,     0,     0,
       0,   972,     0,   106,   107,   108,   109,   110,   111,   112,
     576,   114,     0,   115,   116,   117,   118,   119,     0,     0,
       0,   120,   121,   122,   123,   124,   125,   126,   127,     0,
     128,     0,   129,   130,     0,     0,   131,   132,   133,     0,
       0,     0,   134,   973,   135,   136,   137,     0,   139,   140,
       0,   141,     0,   142,     0,   143,   144,   145,   146,   147,
     148,   149,   150,   151,   152,   153,     0,     0,   154,   155,
     156,     0,   157,   430,   159,   160,   161,   162,   163,   164,
     165,   166,   167,   168,   169,   170,   171,   172,   173,   174,
     175,   176,   177,   178,   179,   180,   181,   182,   183,   538,
     539,   540,   187,   188,   541,   578,   191,   192,   193,   194,
     195,   196,   197,   198,   199,   200,   201,   202,   203,     0,
     204,   205,   206,     0,   207,   208,   209,   210,   211,   212,
     213,   214,   215,   216,   217,   218,   219,   220,   221,   222,
     223,   224,   225,   226,   227,   228,   229,   230,   231,   232,
     233,   234,   235,   236,   237,   238,   239,   240,   241,   242,
     243,   244,   245,   246,   247,   248,   249,   250,   251,   252,
     253,   254,     0,   255,     0,   256,   257,   258,   259,   260,
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
     101,   102,   103,   104,   105,     0,     0,     0,     0,   972,
       0,   106,   107,   108,   109,   110,   111,   112,   576,   114,
       0,   115,   116,   117,   118,   119,     0,     0,     0,   120,
     121,   122,   123,   124,   125,   126,   127,     0,   128,     0,
     129,   130,     0,     0,   131,   132,   133,     0,     0,     0,
     134,   978,   135,   136,   137,     0,   139,   140,     0,   141,
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
       1,     2,     0,     0,     0,     3,     0,     0,     0,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
       0,    15,     0,     0,   427,   428,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
     429,    32,    33,    34,    35,    36,     0,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
       0,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,     0,    74,    75,    76,    77,
      78,     0,    79,    80,    81,     0,     0,    82,    83,     0,
       0,     0,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,     0,    94,     0,     0,     0,    95,     0,    96,
      97,    98,    99,     0,   100,     0,     0,     0,   101,   102,
     103,   104,   105,     0,     0,     0,     0,     0,     0,   106,
     107,   108,   109,   110,   111,   112,   113,   114,     0,   115,
     116,   117,   118,   119,     0,     0,     0,   120,   121,   122,
     123,   124,   125,   126,   127,     0,   128,     0,   129,   130,
       0,     0,   131,   132,   133,     0,     0,     0,   134,     0,
     135,   136,   137,   138,   139,   140,     0,   141,     0,   142,
       0,   143,   144,   145,   146,   147,   148,   149,   150,   151,
     152,   153,     0,     0,   154,   155,   156,     0,   157,   430,
     159,   160,   161,   162,   163,   164,   165,   166,   167,   168,
     169,   170,   171,   172,   173,   174,   175,   176,   177,   178,
     179,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,   197,   198,
     199,   200,   201,   202,   203,     0,   204,   205,   206,     0,
     207,   208,   209,   210,   211,   212,   213,   214,   215,   216,
     217,   218,   219,   220,   221,   222,   223,   224,   225,   226,
     227,   228,   229,   230,   231,   232,   233,   234,   235,   236,
     237,   238,   239,   240,   241,   242,   243,   244,   245,   246,
     247,   248,   249,   250,   251,   252,   253,   254,     0,   255,
       0,   256,   257,   258,   259,   260,   261,   262,     1,     2,
       0,     0,     0,     3,     0,     0,     0,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,     0,    15,
       0,     0,   427,   428,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,   572,   429,    32,
      33,    34,    35,    36,     0,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,     0,    50,
      51,    52,   573,   574,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,     0,    74,    75,    76,    77,    78,     0,
      79,    80,    81,     0,     0,    82,    83,     0,     0,     0,
      84,    85,   520,    87,    88,    89,    90,    91,   575,    93,
       0,    94,     0,     0,     0,    95,     0,    96,    97,    98,
      99,     0,   100,     0,     0,     0,   101,   102,   103,   104,
     105,     0,     0,     0,     0,     0,     0,   106,   107,   108,
     109,   110,   111,   112,   576,   114,     0,   115,   116,   117,
     118,   119,     0,     0,     0,   120,   121,   122,   123,   124,
     125,   126,   127,     0,   128,     0,   129,   130,     0,     0,
     131,   132,   133,     0,     0,   577,   134,     0,   135,   136,
     137,     0,   139,   140,     0,   141,     0,   142,     0,   143,
     144,   145,   146,   147,   148,   149,   150,   151,   152,   153,
       0,     0,   154,   155,   156,     0,   157,   430,   159,   160,
     161,   162,   163,   164,   165,   166,   167,   168,   169,   170,
     171,   172,   173,   174,   175,   176,   177,   178,   179,   180,
     181,   182,   183,   538,   539,   540,   187,   188,   541,   578,
     191,   192,   193,   194,   195,   196,   197,   198,   199,   200,
     201,   202,   203,     0,   204,   205,   206,     0,   207,   208,
     209,   210,   211,   212,   213,   214,   215,   216,   217,   218,
     219,   220,   221,   222,   223,   224,   225,   226,   227,   228,
     229,   230,   231,   232,   233,   234,   235,   236,   237,   238,
     239,   240,   241,   242,   243,   244,   245,   246,   247,   248,
     249,   250,   251,   252,   253,   254,     0,   255,     0,   256,
     257,   258,   259,   260,   261,   262,     1,     2,     0,     0,
       0,     3,     0,     0,     0,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,     0,    15,     0,     0,
     427,   428,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,   572,   429,    32,    33,    34,
      35,    36,     0,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,     0,    50,    51,    52,
     573,   574,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,     0,    74,    75,    76,    77,    78,     0,    79,    80,
      81,     0,     0,    82,    83,     0,     0,     0,    84,    85,
     520,    87,    88,    89,    90,    91,   575,    93,     0,    94,
       0,     0,     0,    95,     0,    96,    97,    98,    99,     0,
     100,     0,     0,     0,   101,   102,   103,   104,   105,     0,
       0,     0,     0,     0,     0,   106,   107,   108,   109,   110,
     111,   112,   576,   114,     0,   115,   116,   117,   118,   119,
       0,     0,     0,   120,   121,   122,   123,   124,   125,   126,
     127,     0,   128,     0,   129,   130,     0,     0,   131,   132,
     133,     0,     0,     0,   134,   583,   135,   136,   137,     0,
     139,   140,     0,   141,     0,   142,     0,   143,   144,   145,
     146,   147,   148,   149,   150,   151,   152,   153,     0,     0,
     154,   155,   156,     0,   157,   430,   159,   160,   161,   162,
     163,   164,   165,   166,   167,   168,   169,   170,   171,   172,
     173,   174,   175,   176,   177,   178,   179,   180,   181,   182,
     183,   538,   539,   540,   187,   188,   541,   578,   191,   192,
     193,   194,   195,   196,   197,   198,   199,   200,   201,   202,
     203,     0,   204,   205,   206,     0,   207,   208,   209,   210,
     211,   212,   213,   214,   215,   216,   217,   218,   219,   220,
     221,   222,   223,   224,   225,   226,   227,   228,   229,   230,
     231,   232,   233,   234,   235,   236,   237,   238,   239,   240,
     241,   242,   243,   244,   245,   246,   247,   248,   249,   250,
     251,   252,   253,   254,     0,   255,     0,   256,   257,   258,
     259,   260,   261,   262,     1,     2,     0,     0,     0,     3,
       0,     0,     0,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,     0,    15,     0,     0,   427,   428,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,   572,   429,    32,    33,    34,    35,    36,
       0,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,     0,    50,    51,    52,   573,   574,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,     0,
      74,    75,    76,    77,    78,     0,    79,    80,    81,     0,
       0,    82,    83,     0,     0,     0,    84,    85,   520,    87,
      88,    89,    90,    91,   575,    93,     0,    94,     0,     0,
       0,    95,     0,    96,    97,    98,    99,     0,   100,     0,
       0,     0,   101,   102,   103,   104,   105,     0,     0,     0,
       0,     0,     0,   106,   107,   108,   109,   110,   111,   112,
     576,   114,     0,   115,   116,   117,   118,   119,     0,     0,
       0,   120,   121,   122,   123,   124,   125,   126,   127,     0,
     128,     0,   129,   130,     0,     0,   131,   132,   133,     0,
       0,     0,   134,     0,   135,   136,   137,     0,   139,   140,
       0,   141,     0,   142,     0,   143,   144,   145,   146,   147,
     148,   149,   150,   151,   152,   153,     0,     0,   154,   155,
     156,     0,   157,   430,   159,   160,   161,   162,   163,   164,
     165,   166,   167,   168,   169,   170,   171,   172,   173,   174,
     175,   176,   177,   178,   179,   180,   181,   182,   183,   538,
     539,   540,   187,   188,   541,   578,   191,   192,   193,   194,
     195,   196,   197,   198,   199,   200,   201,   202,   203,     0,
     204,   205,   206,     0,   207,   208,   209,   210,   211,   212,
     213,   214,   215,   216,   217,   218,   219,   220,   221,   222,
     223,   224,   225,   226,   227,   228,   229,   230,   231,   232,
     233,   234,   235,   236,   237,   238,   239,   240,   241,   242,
     243,   244,   245,   246,   247,   248,   249,   250,   251,   252,
     253,   254,   620,   255,     0,   256,   257,   258,   259,   260,
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
       0,   255,   622,   256,   257,   258,   259,   260,   261,   262,
       1,     2,     0,     0,     0,     3,     0,     0,     0,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
       0,    15,     0,     0,    16,    17,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,     0,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
       0,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,     0,    74,    75,    76,    77,
      78,     0,    79,    80,    81,     0,     0,    82,    83,     0,
       0,     0,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,     0,    94,     0,     0,     0,    95,     0,    96,
      97,    98,    99,     0,   100,     0,     0,     0,   101,   102,
     103,   104,   105,     0,     0,     0,     0,     0,     0,   106,
     107,   108,   109,   110,   111,   112,   113,   114,     0,   115,
     116,   117,   118,   119,     0,     0,     0,   120,   121,   122,
     123,   124,   125,   126,   127,     0,   128,     0,   129,   130,
       0,     0,   131,   132,   133,     0,     0,     0,   134,     0,
     135,   136,   137,   138,   139,   140,     0,   141,     0,   142,
       0,   143,   144,   145,   146,   147,   148,   149,   150,   151,
     152,   153,     0,     0,   154,   155,   156,     0,   157,   158,
     159,   160,   161,   162,   163,   164,   165,   166,   167,   168,
     169,   170,   171,   172,   173,   174,   175,   176,   177,   178,
     179,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,   197,   198,
     199,   200,   201,   202,   203,     0,   204,   205,   206,     0,
     207,   208,   209,   210,   211,   212,   213,   214,   215,   216,
     217,   218,   219,   220,   221,   222,   223,   224,   225,   226,
     227,   228,   229,   230,   231,   232,   233,   234,   235,   236,
     237,   238,   239,   240,   241,   242,   243,   244,   245,   246,
     247,   248,   249,   250,   251,   252,   253,   254,     0,   255,
       0,   256,   257,   258,   259,   260,   261,   262,     1,     2,
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
       0,    94,     0,     0,     0,    95,     0,    96,    97,    98,
      99,     0,   100,     0,     0,     0,   101,   102,   103,   104,
     105,     0,     0,     0,     0,     0,     0,   106,   107,   108,
     109,   110,   111,   112,   113,   114,     0,   115,   116,   117,
     118,   119,     0,     0,     0,   120,   121,   122,   123,   124,
     125,   126,   127,     0,   128,     0,   129,   130,     0,     0,
     131,   132,   133,     0,     0,     0,   134,     0,   135,   136,
     137,   138,   139,   140,     0,   141,     0,   142,     0,   143,
     144,   145,   146,   147,   148,   149,   150,   151,   152,   153,
       0,     0,   154,   155,   156,     0,   157,   430,   159,   160,
     161,   162,   163,   164,   165,   166,   167,   168,   169,   170,
     171,   172,   173,   174,   175,   176,   177,   178,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,   197,   198,   199,   200,
     201,   202,   203,     0,   204,   205,   206,     0,   207,   208,
     209,   210,   211,   212,   213,   214,   215,   216,   217,   218,
     219,   220,   221,   222,   223,   224,   225,   226,   227,   228,
     229,   230,   231,   232,   233,   234,   235,   236,   237,   238,
     239,   240,   241,   242,   243,   244,   245,   246,   247,   248,
     249,   250,   251,   252,   253,   254,     0,   255,     0,   256,
     257,   258,   259,   260,   261,   262,     1,     2,     0,     0,
       0,     3,     0,     0,     0,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,     0,    15,     0,     0,
      16,   428,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
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
     251,   252,   253,   254,     0,   255,     0,   256,   257,   258,
     259,   260,   261,   262,     1,     2,     0,     0,     0,     3,
       0,     0,     0,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,     0,    15,     0,     0,   909,   428,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
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
     253,   254,     0,   255,     0,   256,   257,   258,   259,   260,
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
     129,   130,     0,     0,   131,   132,   133,     0,     0,   971,
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
       1,     2,     0,     0,     0,     3,     0,     0,     0,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
       0,    15,     0,     0,   427,   428,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,   572,
     429,    32,    33,    34,    35,    36,     0,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
       0,    50,    51,    52,   573,   574,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,     0,    74,    75,    76,    77,
      78,     0,    79,    80,    81,     0,     0,    82,    83,     0,
       0,     0,    84,    85,   520,    87,    88,    89,    90,    91,
     575,    93,     0,    94,     0,     0,     0,    95,     0,    96,
      97,    98,    99,     0,   100,     0,     0,     0,   101,   102,
     103,   104,   105,     0,     0,     0,     0,     0,     0,   106,
     107,   108,   109,   110,   111,   112,   576,   114,     0,   115,
     116,   117,   118,   119,     0,     0,     0,   120,   121,   122,
     123,   124,   125,   126,   127,     0,   128,     0,   129,   130,
       0,     0,   131,   132,   133,     0,     0,     0,   134,  1212,
     135,   136,   137,     0,   139,   140,     0,   141,     0,   142,
       0,   143,   144,   145,   146,   147,   148,   149,   150,   151,
     152,   153,     0,     0,   154,   155,   156,     0,   157,   430,
     159,   160,   161,   162,   163,   164,   165,   166,   167,   168,
     169,   170,   171,   172,   173,   174,   175,   176,   177,   178,
     179,   180,   181,   182,   183,   538,   539,   540,   187,   188,
     541,   578,   191,   192,   193,   194,   195,   196,   197,   198,
     199,   200,   201,   202,   203,     0,   204,   205,   206,     0,
     207,   208,   209,   210,   211,   212,   213,   214,   215,   216,
     217,   218,   219,   220,   221,   222,   223,   224,   225,   226,
     227,   228,   229,   230,   231,   232,   233,   234,   235,   236,
     237,   238,   239,   240,   241,   242,   243,   244,   245,   246,
     247,   248,   249,   250,   251,   252,   253,   254,     0,   255,
       0,   256,   257,   258,   259,   260,   261,   262,     1,     2,
       0,     0,     0,     3,     0,     0,     0,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,     0,    15,
       0,     0,   427,   428,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,  1380,   429,    32,
      33,    34,    35,    36,     0,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,     0,    50,
      51,    52,  1381,  1382,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,     0,    74,    75,    76,    77,    78,     0,
      79,    80,    81,     0,     0,    82,    83,     0,     0,     0,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
       0,    94,     0,     0,     0,    95,     0,    96,    97,    98,
      99,     0,   100,     0,     0,     0,   101,   102,   103,   104,
     105,     0,     0,     0,     0,     0,     0,   106,   107,   108,
     109,   110,   111,   112,   113,   114,     0,   115,   116,   117,
     118,   119,     0,     0,     0,   120,   121,   122,   123,   124,
     125,   126,   127,     0,   128,     0,   129,   130,     0,     0,
     131,   132,   133,     0,     0,     0,   134,     0,   135,   136,
     137,   138,   139,   140,     0,   141,     0,   142,     0,   143,
     144,   145,   146,   147,   148,   149,   150,   151,   152,   153,
       0,     0,   154,   155,   156,     0,   157,   430,   159,   160,
     161,   162,   163,   164,   165,   166,   167,   168,   169,   170,
     171,   172,   173,   174,   175,   176,   177,   178,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,  1383,
     191,   192,   193,   194,   195,   196,   197,   198,   199,   200,
     201,   202,   203,     0,   204,   205,   206,     0,   207,   208,
     209,   210,   211,   212,   213,   214,   215,   216,   217,   218,
     219,   220,   221,   222,   223,   224,   225,   226,   227,   228,
     229,   230,   231,   232,   233,   234,   235,   236,   237,   238,
     239,   240,   241,   242,   243,   244,   245,   246,   247,   248,
     249,   250,   251,   252,   253,   254,     0,   255,     0,   256,
     257,   258,   259,   260,   261,   262,     1,     2,     0,     0,
       0,     3,     0,     0,     0,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,     0,    15,     0,     0,
     427,   428,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,   572,   429,    32,    33,    34,
      35,    36,     0,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,     0,    50,    51,    52,
     573,   574,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,     0,    74,    75,    76,    77,    78,     0,    79,    80,
      81,     0,     0,    82,    83,     0,     0,     0,    84,    85,
     520,    87,    88,    89,    90,    91,   575,    93,     0,    94,
       0,     0,     0,    95,     0,    96,    97,    98,    99,     0,
     100,     0,     0,     0,   101,   102,   103,   104,   105,     0,
       0,     0,     0,  1478,     0,   106,   107,   108,   109,   110,
     111,   112,   576,   114,     0,   115,   116,   117,   118,   119,
       0,     0,     0,   120,   121,   122,   123,   124,   125,   126,
     127,     0,   128,     0,   129,   130,     0,     0,   131,   132,
     133,     0,     0,     0,   134,     0,   135,   136,   137,     0,
     139,   140,     0,   141,     0,   142,     0,   143,   144,   145,
     146,   147,   148,   149,   150,   151,   152,   153,     0,     0,
     154,   155,   156,     0,   157,   430,   159,   160,   161,   162,
     163,   164,   165,   166,   167,   168,   169,   170,   171,   172,
     173,   174,   175,   176,   177,   178,   179,   180,   181,   182,
     183,   538,   539,   540,   187,   188,   541,   578,   191,   192,
     193,   194,   195,   196,   197,   198,   199,   200,   201,   202,
     203,     0,   204,   205,   206,     0,   207,   208,   209,   210,
     211,   212,   213,   214,   215,   216,   217,   218,   219,   220,
     221,   222,   223,   224,   225,   226,   227,   228,   229,   230,
     231,   232,   233,   234,   235,   236,   237,   238,   239,   240,
     241,   242,   243,   244,   245,   246,   247,   248,   249,   250,
     251,   252,   253,   254,     0,   255,     0,   256,   257,   258,
     259,   260,   261,   262,     1,     2,     0,     0,     0,     3,
       0,     0,     0,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,     0,    15,     0,     0,   427,   428,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,   506,    29,   508,   429,    32,   509,    34,    35,    36,
       0,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,   511,     0,    50,    51,    52,   513,   514,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,     0,
      74,    75,    76,    77,    78,     0,    79,    80,    81,     0,
       0,    82,    83,     0,     0,     0,    84,    85,   520,    87,
      88,    89,    90,    91,   575,    93,     0,    94,     0,     0,
       0,    95,     0,    96,    97,    98,    99,     0,   100,     0,
       0,     0,   523,   102,   103,   104,   105,     0,     0,     0,
       0,     0,     0,   106,   107,   108,   109,   110,   111,   112,
     576,   114,     0,   115,   116,   710,   118,   119,     0,     0,
       0,   120,   121,   122,   123,   124,   125,   126,   127,     0,
     128,     0,   711,   130,     0,     0,   131,   132,   133,     0,
       0,     0,   134,     0,   135,   136,   137,     0,   139,   140,
       0,   141,     0,   142,     0,   143,   144,   145,   146,   147,
     148,   149,   150,   151,   152,   153,     0,     0,   154,   155,
     156,     0,   157,   430,   159,   160,   161,   162,   163,   164,
     165,   166,   167,   168,   169,   533,   171,   534,   173,   535,
     536,   176,   177,   178,   179,   180,   181,   537,   183,   538,
     539,   540,   187,   188,   541,   542,   191,   192,   193,   194,
     195,   196,   197,   198,   199,   200,   201,   202,   543,     0,
     204,   544,   206,     0,   207,   208,   209,   210,   211,   212,
     213,   214,   215,   216,   217,   218,   219,   220,   221,   222,
     223,   224,   225,   226,   227,   228,   229,   230,   231,   232,
     233,   234,   235,   236,   237,   238,   239,   240,   241,   242,
     243,   244,   245,   246,   247,   248,   249,   250,   251,   252,
     253,   254,     0,   255,     0,   256,   545,   258,   259,   260,
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
       1,     2,     0,     0,     0,     3,     0,     0,     0,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
       0,    15,     0,     0,   427,   428,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,   506,    29,   508,
     429,    32,   509,    34,    35,    36,     0,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,   511,
       0,    50,    51,    52,   513,   514,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,     0,    74,    75,    76,    77,
      78,     0,    79,    80,    81,     0,     0,    82,    83,     0,
       0,     0,    84,    85,   520,    87,    88,    89,    90,    91,
     575,    93,     0,    94,     0,     0,     0,    95,     0,    96,
      97,    98,    99,     0,   100,     0,     0,     0,   523,   102,
     103,   104,   105,     0,     0,     0,     0,     0,     0,   106,
     107,   108,   109,   110,   111,   112,   576,   114,     0,   115,
     116,   117,   118,   119,     0,     0,     0,   120,   121,   122,
     123,   124,   125,   126,   127,     0,   128,     0,   129,   130,
       0,     0,   131,   132,   133,     0,     0,     0,   134,     0,
     135,   136,   137,     0,   139,   140,     0,   141,     0,   142,
       0,   143,   144,   145,   146,   147,   148,   149,   150,   151,
     152,   153,     0,     0,   154,   155,   156,     0,   157,   430,
     159,   160,   161,   162,   163,   164,   165,   166,   167,   168,
     169,   533,   171,   534,   173,   535,   536,   176,   177,   178,
     179,   180,   181,   537,   183,   538,   539,   540,   187,   188,
     541,   542,   191,   192,   193,   194,   195,   196,   197,   198,
     199,   200,   201,   202,   543,     0,   204,   205,   206,     0,
     207,   208,   209,   210,   211,   212,   213,   214,   215,   216,
     217,   218,   219,   220,   221,   222,   223,   224,   225,   226,
     227,   228,   229,   230,   231,   232,   233,   234,   235,   236,
     237,   238,   239,   240,   241,   242,   243,   244,   245,   246,
     247,   248,   249,   250,   251,   252,   253,   254,     0,   255,
       0,   256,   545,   258,   259,   260,   261,   262,     1,     2,
       0,     0,     0,     3,     0,     0,     0,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,     0,    15,
       0,     0,   427,   428,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,   506,    29,   508,   429,    32,
     509,    34,    35,    36,     0,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,   511,     0,    50,
      51,    52,   513,   514,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,     0,    74,    75,    76,    77,    78,     0,
      79,    80,    81,     0,     0,    82,    83,     0,     0,     0,
      84,    85,   520,    87,    88,    89,    90,    91,   575,    93,
       0,    94,     0,     0,     0,    95,     0,    96,    97,    98,
      99,     0,   100,     0,     0,     0,   523,   102,   103,   104,
     105,     0,     0,     0,     0,     0,     0,   106,   107,   108,
     109,   110,   111,   112,   576,   114,     0,   115,   116,   117,
     118,   119,     0,     0,     0,   120,   121,   122,   123,   124,
     125,   126,   127,     0,   128,     0,   129,   130,     0,     0,
     131,   132,   133,     0,     0,     0,   134,     0,   135,   136,
     137,     0,   139,   140,     0,   141,     0,   142,     0,   143,
     144,   145,   146,   147,   148,   149,   150,   151,   152,   153,
       0,     0,   154,   155,   156,     0,   157,   430,   159,   160,
     161,   162,   163,   164,   165,   166,   167,   168,   169,   533,
     171,   534,   173,   535,   536,   176,   177,   178,   179,   180,
     181,   537,   183,   538,   539,   540,   187,   188,   541,   542,
     191,   192,   193,   194,   195,   196,   197,   198,   199,   200,
     201,   202,   543,     0,   204,   544,   206,     0,   207,   208,
     209,   210,   211,   212,   213,   214,   215,   216,   217,   218,
     219,   220,   221,   222,   223,   224,   225,   226,   227,   228,
     229,   230,   231,   232,   233,   234,   235,   236,   237,   238,
     239,   240,   241,   242,   243,   244,   245,   246,   247,   248,
     249,   250,   251,   252,   253,   254,     0,   255,     0,   256,
     545,   258,   259,   260,   261,   262,     1,     2,     0,     0,
       0,     3,     0,     0,     0,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,     0,    15,     0,     0,
     427,   428,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,   572,   429,    32,    33,    34,
      35,    36,     0,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,     0,    50,    51,    52,
     573,   574,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,     0,    74,    75,    76,    77,    78,     0,    79,    80,
      81,     0,     0,    82,    83,     0,     0,     0,    84,    85,
     520,    87,    88,    89,    90,    91,  1160,    93,     0,    94,
       0,     0,     0,    95,     0,    96,    97,    98,    99,     0,
     100,     0,     0,     0,   101,   102,   103,   104,   105,     0,
       0,     0,     0,     0,     0,   106,   107,   108,   109,   110,
     111,   112,   576,   114,     0,   115,   116,   117,   118,   119,
       0,     0,     0,   120,   121,   122,   123,   124,   125,   126,
     127,     0,   128,     0,   129,   130,     0,     0,   131,   132,
     133,     0,     0,     0,   134,     0,   135,   136,   137,     0,
     139,   140,     0,   141,     0,   142,     0,   143,   144,   145,
     146,   147,   148,   149,   150,   151,   152,   153,     0,     0,
     154,   155,   156,     0,   157,   430,   159,   160,   161,   162,
     163,   164,   165,   166,   167,   168,   169,   170,   171,   172,
     173,   174,   175,   176,   177,   178,   179,   180,   181,   182,
     183,   538,   539,   540,   187,   188,   541,   578,   191,   192,
     193,   194,   195,   196,   197,   198,   199,   200,   201,   202,
     203,     0,   204,   205,   206,     0,   207,   208,   209,   210,
     211,   212,   213,   214,   215,   216,   217,   218,   219,   220,
     221,   222,   223,   224,   225,   226,   227,   228,   229,   230,
     231,   232,   233,   234,   235,   236,   237,   238,   239,   240,
     241,   242,   243,   244,   245,   246,   247,   248,   249,   250,
     251,   252,   253,   254,     0,   255,     0,   256,   257,   258,
     259,   260,   261,   262,     1,     2,     0,     0,     0,     3,
       0,     0,     0,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,     0,    15,     0,     0,   427,   428,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,   506,    29,   508,   429,    32,   509,    34,    35,    36,
       0,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,   511,     0,    50,    51,    52,   513,   514,
      55,   515,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,     0,
      74,    75,    76,    77,    78,     0,    79,    80,    81,     0,
       0,    82,    83,     0,     0,     0,    84,    85,   520,    87,
      88,    89,    90,    91,   575,    93,     0,    94,     0,     0,
       0,    95,     0,    96,    97,    98,    99,     0,   100,     0,
       0,     0,   523,   102,   103,   104,   105,     0,     0,     0,
       0,     0,     0,   106,   107,   108,   109,   110,   111,   112,
     576,   114,     0,   115,   116,     0,   118,   119,     0,     0,
       0,   120,   121,   122,   123,   124,   125,   126,   127,     0,
     128,     0,     0,     0,     0,     0,   131,   132,   133,     0,
       0,     0,   134,     0,   135,   136,   137,     0,   139,   140,
       0,   141,     0,   142,     0,   143,   144,   145,   146,   147,
     148,   149,   150,   151,   152,   153,     0,     0,   154,   155,
     156,     0,   157,   430,   159,   160,   161,   162,   163,   164,
     165,   166,   167,   168,   169,   533,   171,   534,   173,   535,
     536,   176,   177,   178,   179,   180,   181,   537,   183,   538,
     539,   540,   187,   188,   541,   542,   191,   192,   193,   194,
     195,   196,   197,   198,   199,   200,   201,   202,   543,     0,
     204,   544,   206,     0,   207,   208,   209,   210,   211,   212,
     213,   214,   215,   216,   217,   218,   219,   220,   221,   222,
     223,   224,   225,   226,   227,   228,   229,   230,   231,   232,
     233,   234,   235,   236,   237,   238,   239,   240,   241,   242,
     243,   244,   245,   246,   247,   248,   249,   250,   251,   252,
     253,   254,     0,   255,     0,   256,   545,   258,   259,   260,
     261,   262,     1,     2,     0,     0,     0,     3,     0,     0,
       0,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,     0,    15,     0,     0,   427,   428,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,   506,
      29,   508,   429,    32,   509,    34,    35,    36,     0,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,   511,     0,    50,    51,    52,   513,   514,    55,   515,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,     0,    74,    75,
      76,    77,    78,     0,    79,    80,    81,     0,     0,    82,
      83,     0,     0,     0,    84,    85,   520,    87,    88,    89,
      90,    91,   575,    93,     0,    94,     0,     0,     0,    95,
       0,    96,    97,    98,    99,     0,   100,     0,     0,     0,
     523,   102,   103,   104,   105,     0,     0,     0,     0,     0,
       0,   106,   107,   108,   109,   110,   111,   112,   576,   114,
       0,   115,   116,     0,   118,   119,     0,     0,     0,   120,
     121,   122,   123,   124,   125,   126,   127,     0,   128,     0,
       0,     0,     0,     0,   131,   132,   133,     0,     0,     0,
     134,     0,   135,   136,   137,     0,     0,     0,     0,   141,
       0,   142,     0,   143,   144,   145,   146,   147,   148,   149,
     150,   151,   152,   153,     0,     0,   154,   155,   156,     0,
     157,   430,   159,   160,   161,   162,   163,   164,   165,   166,
     167,   168,   169,   533,   171,   534,   173,   535,   536,   176,
     177,   178,   179,   180,   181,   537,   183,   538,   539,   540,
     187,   188,   541,   542,   191,   192,   193,   194,   195,   196,
     197,   198,   199,   200,   201,   202,   543,     0,   204,   544,
     206,     0,   207,   208,   209,   210,   211,   212,   213,   214,
     215,   216,   217,   218,   219,   220,   221,   222,   223,   224,
     225,   226,   227,   228,   229,   230,   231,   232,   233,   234,
     235,   236,   237,   238,   239,   240,   241,   242,   243,   244,
     245,   246,   247,   248,   249,   250,   251,   252,   253,   254,
       0,   255,     0,   256,   545,   258,   259,   260,   261,   262,
       1,     2,     0,     0,     0,     3,     0,     0,     0,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
       0,    15,     0,     0,   427,   428,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,   506,    29,   508,
     429,    32,   509,    34,    35,    36,     0,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,   511,
       0,    50,    51,    52,   513,   514,    55,   515,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,     0,    74,    75,    76,    77,
      78,     0,    79,    80,    81,     0,     0,    82,    83,     0,
       0,     0,    84,    85,   520,    87,    88,    89,    90,    91,
     575,    93,     0,    94,     0,     0,     0,    95,     0,    96,
      97,    98,    99,     0,   100,     0,     0,     0,   523,     0,
     103,   104,   105,     0,     0,     0,     0,     0,     0,   106,
     107,   108,   109,     0,   111,   112,   576,   114,     0,   115,
     116,     0,   118,   119,     0,     0,     0,   120,   121,   122,
     123,   124,   125,   126,   127,     0,   128,     0,     0,     0,
       0,     0,   131,   132,   133,     0,     0,     0,   134,     0,
     135,   136,   137,     0,     0,     0,     0,   141,     0,   142,
       0,   143,     0,   145,     0,   147,   148,   149,   150,   151,
     152,   153,     0,     0,   154,   155,   156,     0,   157,   430,
     159,   160,   161,   162,   163,   164,   165,   166,   167,   168,
     169,   533,   171,   534,   173,   535,   536,   176,   177,   178,
     179,   180,   181,   537,   183,   538,   539,   540,   187,   188,
     541,   542,   191,   192,   193,   194,   195,   196,   197,   198,
     199,   200,   201,   202,   543,     0,   204,   544,   206,     0,
     207,   208,   209,   210,   211,   212,   213,   214,   215,   216,
     217,   218,   219,   220,   221,   222,   223,   224,   225,   226,
     227,   228,   229,   230,   231,   232,   233,   234,   235,   236,
     237,   238,   239,   240,   241,   242,   243,   244,   245,   246,
     247,   248,   249,   250,   251,   252,   253,   254,     0,   255,
       0,   256,   545,   258,   259,   260,   261,   262,     1,     2,
       0,     0,     0,     3,     0,     0,     0,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,     0,    15,
       0,     0,   427,   428,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,   506,    29,   508,   429,    32,
     509,    34,    35,    36,     0,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,   511,     0,    50,
      51,    52,   513,   514,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,     0,    74,    75,    76,    77,    78,     0,
      79,    80,    81,     0,     0,    82,    83,     0,     0,     0,
      84,    85,   520,    87,    88,    89,    90,     0,   575,    93,
       0,    94,     0,     0,     0,    95,     0,    96,    97,    98,
      99,     0,   100,     0,     0,     0,   523,     0,   103,   104,
     105,     0,     0,     0,     0,     0,     0,     0,   107,   108,
     109,     0,   111,   112,   576,     0,     0,     0,   116,     0,
       0,   119,     0,     0,     0,   120,   121,   122,   123,   124,
     125,   126,   127,     0,   128,     0,     0,   130,     0,     0,
     131,   132,   133,     0,     0,     0,   134,     0,   135,   136,
     137,     0,     0,     0,     0,     0,     0,   142,     0,   143,
       0,   145,     0,   147,   148,   149,   150,   151,   152,   153,
       0,     0,   154,   155,   156,     0,   157,   430,   159,   160,
     161,   162,   163,   164,   165,   166,   167,   168,   169,   533,
     171,   534,   173,   535,   536,   176,   177,   178,   179,   180,
     181,   537,   183,   538,   539,   540,   187,   188,   541,   542,
     191,   192,   193,   194,   195,   196,   197,   198,   199,   200,
     201,   202,   543,     0,   204,   205,   206,     0,   207,   208,
     209,   210,   211,   212,   213,   214,   215,   216,   217,   218,
     219,   220,   221,   222,   223,   224,   225,   226,   227,   228,
     229,   230,   231,   232,   233,   234,   235,   236,   237,   238,
     239,   240,   241,   242,   243,   244,   245,   246,   247,   248,
     249,   250,   251,   252,   253,   254,     0,   255,     0,   256,
     545,   258,   259,   260,   261,   262,     1,     2,     0,     0,
       0,     3,     0,     0,     0,     4,     0,     0,     0,     0,
       0,     0,    11,    12,     0,     0,     0,     0,     0,     0,
     427,   428,     0,    18,    19,    20,   503,    22,    23,    24,
     504,    26,   505,   506,   507,   508,   429,    32,   509,    34,
      35,    36,     0,    37,    38,    39,    40,   510,    42,    43,
      44,    45,    46,    47,    48,   511,     0,    50,   512,    52,
     513,   514,    55,   515,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,   516,   517,
      73,     0,    74,    75,    76,   518,     0,     0,    79,    80,
      81,     0,     0,   519,    83,     0,     0,     0,     0,    85,
     520,    87,   521,   522,    90,    91,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    96,    97,    98,    99,     0,
     100,     0,     0,     0,   523,   102,   103,   524,   525,     0,
       0,     0,     0,     0,     0,   106,   107,   108,     0,   110,
     111,   112,     0,     0,     0,     0,     0,     0,   118,   526,
       0,     0,     0,   120,   121,   122,   123,   527,   125,   126,
     528,     0,     0,     0,     0,     0,     0,     0,   529,   530,
     133,     0,     0,     0,   134,     0,   135,   136,   531,     0,
       0,     0,     0,   141,     0,   142,     0,   143,   144,   145,
     146,   532,   148,   149,   150,   151,   152,   153,     0,     0,
     154,   155,     0,     0,   157,   430,   159,   160,   161,   162,
     163,   164,   165,   166,   167,   168,   169,   533,   171,   534,
     173,   535,   536,   176,   177,   178,   179,   180,   181,   537,
     183,   538,   539,   540,   187,   188,   541,   542,   191,   192,
     193,   194,   195,   196,   197,   198,   199,   200,   201,   202,
     543,     0,   204,   544,   206,     0,   207,   208,   209,   210,
     211,   212,   213,   214,   215,   216,   217,   218,   219,   220,
     221,   222,   223,   224,   225,   226,   227,   228,   229,   230,
     231,   232,   233,   234,   235,   236,   237,   238,   239,   240,
     241,   242,   243,   244,   245,   246,   247,   248,   249,   250,
     251,   252,   253,     0,     0,     0,     0,   256,   545,   258,
     259,   260,   261,   262,     1,     2,     0,     0,     0,     3,
       0,     0,     0,     4,     0,     0,     0,     0,     0,     0,
      11,    12,     0,     0,     0,     0,     0,     0,   427,   428,
       0,    18,    19,    20,   553,    22,    23,    24,   504,   554,
     555,   506,   507,   508,   429,    32,   509,    34,   556,    36,
       0,    37,    38,    39,    40,   557,    42,   558,   559,    45,
      46,    47,    48,   511,     0,    50,   560,    52,   513,   514,
      55,   515,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,   516,   517,    73,     0,
      74,    75,    76,   561,     0,     0,    79,    80,    81,     0,
       0,   519,    83,     0,     0,     0,     0,    85,   520,    87,
     521,   522,    90,    91,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    96,    97,    98,    99,     0,   100,     0,
       0,     0,   523,   102,   103,   524,   525,     0,     0,     0,
       0,     0,     0,   106,   107,   108,     0,   110,   111,   112,
       0,     0,     0,     0,     0,     0,   118,   526,     0,     0,
       0,   120,   121,   122,   123,   527,   125,   126,   528,     0,
       0,     0,     0,     0,     0,     0,   529,   530,   133,     0,
       0,     0,   134,     0,   135,   136,   531,     0,     0,     0,
       0,   141,     0,   142,     0,   143,   144,   145,   146,   532,
     148,   149,   150,   151,   152,   153,     0,     0,   154,   155,
       0,     0,   157,   430,   159,   160,   161,   162,   163,   164,
     165,   166,   167,   168,   169,   533,   562,   534,   173,   535,
     536,   176,   177,   178,   179,   180,   181,   537,   183,   538,
     539,   540,   187,   188,   541,   542,   191,   192,   193,   194,
     195,   196,   197,   198,   199,   200,   201,   202,   543,     0,
     204,   544,   206,     0,   207,   208,   209,   210,   211,   212,
     213,   214,   215,   216,   217,   218,   219,   220,   221,   222,
     223,   224,   225,   226,   227,   228,   229,   230,   231,   232,
     233,   234,   235,   236,   237,   238,   239,   240,   241,   242,
     243,   244,   245,   246,   247,   248,   249,   250,   251,   252,
     253,     3,     0,     0,     0,   256,   545,   258,   259,   260,
     261,   262,    11,    12,    13,    14,     0,     0,     0,     0,
     427,   428,     0,    18,    19,    20,   503,    22,    23,    24,
     504,    26,   505,   506,  1186,   508,   429,    32,   509,    34,
      35,    36,     0,    37,    38,    39,    40,   510,    42,    43,
      44,    45,    46,    47,    48,   511,     0,    50,   512,    52,
     513,   514,    55,   515,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,   516,   517,
      73,     0,    74,    75,    76,   518,     0,     0,    79,    80,
      81,     0,     0,   519,    83,     0,     0,     0,     0,    85,
     520,    87,   521,   522,    90,    91,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    96,    97,    98,    99,     0,
     100,     0,     0,     0,   523,   102,   103,   524,   525,     0,
       0,     0,     0,     0,     0,   106,   107,   108,     0,   110,
     111,  1187,     0,     0,     0,  1188,     0,     0,   118,   526,
       0,     0,     0,   120,   121,   122,   123,   527,   125,   126,
     528,     0,     0,     0,     0,     0,     0,     0,   529,   530,
     133,     0,     0,     0,   134,  1631,   135,   136,   531,     0,
       0,     0,     0,  1632,     0,   142,     0,   143,   144,   145,
     146,   532,   148,   149,   150,   151,   152,   153,     0,     0,
    1189,   155,     0,     0,   157,   430,   159,   160,   161,   162,
     163,   164,   165,   166,   167,   168,   169,   533,   171,   534,
     173,   535,   536,   176,   177,   178,   179,   180,   181,   537,
     183,   538,   539,   540,   187,   188,   541,   542,   191,   192,
     193,   194,   195,   196,   197,   198,   199,   200,   201,   202,
     543,     0,   204,   544,   206,     0,   207,   208,   209,   210,
     211,   212,   213,   214,   215,   216,   217,   218,   219,   220,
     221,   222,   223,   224,   225,   226,   227,   228,   229,   230,
     231,   232,   233,   234,   235,   236,   237,   238,   239,   240,
     241,   242,   243,   244,   245,   246,   247,   248,   249,   250,
     251,   252,   253,     3,     0,     0,     0,   256,   545,   258,
    1190,  1191,  1192,  1193,    11,    12,     0,     0,     0,    15,
       0,     0,   427,   428,     0,    18,    19,    20,   553,    22,
      23,    24,   504,   554,   555,   506,   507,   508,   429,    32,
     509,    34,   556,    36,     0,    37,    38,    39,    40,   557,
      42,   558,   559,    45,    46,    47,    48,   511,     0,    50,
     560,    52,   513,   514,    55,   515,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
     516,   517,    73,     0,    74,    75,    76,   561,     0,     0,
      79,    80,    81,     0,     0,   519,    83,     0,     0,     0,
       0,    85,   520,    87,   521,   522,    90,    91,   575,    93,
       0,     0,     0,     0,     0,     0,     0,    96,    97,    98,
      99,     0,   100,     0,     0,     0,   523,   102,   103,   524,
     525,     0,     0,     0,     0,     0,     0,   106,   107,   108,
       0,   110,   111,   112,     0,     0,     0,   966,     0,     0,
     118,   526,     0,     0,     0,   120,   121,   122,   123,   527,
     125,   126,   528,     0,     0,     0,     0,     0,     0,     0,
     529,   530,   133,     0,     0,     0,   134,     0,   135,   136,
     531,     0,     0,     0,     0,     0,     0,   142,     0,   143,
     144,   145,   146,   532,   148,   149,   150,   151,   152,   153,
       0,     0,   154,   155,     0,     0,   157,   430,   159,   160,
     161,   162,   163,   164,   165,   166,   167,   168,   169,   533,
     562,   534,   173,   535,   536,   176,   177,   178,   179,   180,
     181,   537,   183,   538,   539,   540,   187,   188,   541,   542,
     191,   192,   193,   194,   195,   196,   197,   198,   199,   200,
     201,   202,   543,     0,   204,   544,   206,     0,   207,   208,
     209,   210,   211,   212,   213,   214,   215,   216,   217,   218,
     219,   220,   221,   222,   223,   224,   225,   226,   227,   228,
     229,   230,   231,   232,   233,   234,   235,   236,   237,   238,
     239,   240,   241,   242,   243,   244,   245,   246,   247,   248,
     249,   250,   251,   252,   253,     3,     0,     0,     0,   256,
     545,   258,   259,   260,   261,   262,    11,    12,    13,    14,
       0,     0,     0,     0,   427,   428,     0,    18,    19,    20,
     503,    22,    23,    24,   504,    26,   505,   506,  1186,   508,
     429,    32,   509,    34,    35,    36,     0,    37,    38,    39,
      40,   510,    42,    43,    44,    45,    46,    47,    48,   511,
       0,    50,   512,    52,   513,   514,    55,   515,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,   516,   517,    73,     0,    74,    75,    76,   518,
       0,     0,    79,    80,    81,     0,     0,   519,    83,     0,
       0,     0,     0,    85,   520,    87,   521,   522,    90,    91,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    96,
      97,    98,    99,     0,   100,     0,     0,     0,   523,   102,
     103,   524,   525,     0,     0,     0,     0,     0,     0,   106,
     107,   108,     0,   110,   111,  1187,     0,     0,     0,  1227,
       0,     0,   118,   526,     0,     0,     0,   120,   121,   122,
     123,   527,   125,   126,   528,     0,     0,     0,     0,     0,
       0,     0,   529,   530,   133,     0,     0,     0,   134,  1462,
     135,   136,   531,     0,     0,     0,     0,     0,     0,   142,
       0,   143,   144,   145,   146,   532,   148,   149,   150,   151,
     152,   153,     0,     0,   154,   155,     0,     0,   157,   430,
     159,   160,   161,   162,   163,   164,   165,   166,   167,   168,
     169,   533,   171,   534,   173,   535,   536,   176,   177,   178,
     179,   180,   181,   537,   183,   538,   539,   540,   187,   188,
     541,   542,   191,   192,   193,   194,   195,   196,   197,   198,
     199,   200,   201,   202,   543,     0,   204,   544,   206,     0,
     207,   208,   209,   210,   211,   212,   213,   214,   215,   216,
     217,   218,   219,   220,   221,   222,   223,   224,   225,   226,
     227,   228,   229,   230,   231,   232,   233,   234,   235,   236,
     237,   238,   239,   240,   241,   242,   243,   244,   245,   246,
     247,   248,   249,   250,   251,   252,   253,     3,     0,     0,
       0,   256,   545,   258,  1190,  1191,  1192,  1193,    11,    12,
      13,    14,     0,     0,     0,     0,   427,   428,     0,    18,
      19,    20,   503,    22,    23,    24,   504,    26,   505,   506,
    1186,   508,   429,    32,   509,    34,    35,    36,     0,    37,
      38,    39,    40,   510,    42,    43,    44,    45,    46,    47,
      48,   511,     0,    50,   512,    52,   513,   514,    55,   515,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,   516,   517,    73,     0,    74,    75,
      76,   518,     0,     0,    79,    80,    81,     0,     0,   519,
      83,     0,     0,     0,     0,    85,   520,    87,   521,   522,
      90,    91,  1539,     0,     0,     0,     0,     0,     0,     0,
       0,    96,    97,    98,    99,     0,   100,     0,     0,     0,
     523,   102,   103,   524,   525,     0,     0,     0,     0,     0,
       0,   106,   107,   108,     0,   110,   111,  1187,     0,     0,
       0,  1188,     0,     0,   118,   526,     0,     0,     0,   120,
     121,   122,   123,   527,   125,   126,   528,     0,     0,     0,
       0,     0,     0,     0,   529,   530,   133,     0,     0,     0,
     134,     0,   135,   136,   531,     0,     0,     0,     0,     0,
       0,   142,     0,   143,   144,   145,   146,   532,   148,   149,
     150,   151,   152,   153,     0,     0,  1189,   155,     0,     0,
     157,   430,   159,   160,   161,   162,   163,   164,   165,   166,
     167,   168,   169,   533,   171,   534,   173,   535,   536,   176,
     177,   178,   179,   180,   181,   537,   183,   538,   539,   540,
     187,   188,   541,   542,   191,   192,   193,   194,   195,   196,
     197,   198,   199,   200,   201,   202,   543,     0,   204,   544,
     206,     0,   207,   208,   209,   210,   211,   212,   213,   214,
     215,   216,   217,   218,   219,   220,   221,   222,   223,   224,
     225,   226,   227,   228,   229,   230,   231,   232,   233,   234,
     235,   236,   237,   238,   239,   240,   241,   242,   243,   244,
     245,   246,   247,   248,   249,   250,   251,   252,   253,     3,
       0,     0,     0,   256,   545,   258,  1190,  1191,  1192,  1193,
      11,    12,    13,    14,     0,     0,     0,     0,   427,   428,
       0,    18,    19,    20,   503,    22,    23,    24,   504,    26,
     505,   506,  1186,   508,   429,    32,   509,    34,    35,    36,
       0,    37,    38,    39,    40,   510,    42,    43,    44,    45,
      46,    47,    48,   511,     0,    50,   512,    52,   513,   514,
      55,   515,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,   516,   517,    73,     0,
      74,    75,    76,   518,     0,     0,    79,    80,    81,     0,
       0,   519,    83,     0,     0,     0,     0,    85,   520,    87,
     521,   522,    90,    91,  1684,     0,     0,     0,     0,     0,
       0,     0,     0,    96,    97,    98,    99,     0,   100,     0,
       0,     0,   523,   102,   103,   524,   525,     0,     0,     0,
       0,     0,     0,   106,   107,   108,     0,   110,   111,  1187,
       0,     0,     0,  1188,     0,     0,   118,   526,     0,     0,
       0,   120,   121,   122,   123,   527,   125,   126,   528,     0,
       0,     0,     0,     0,     0,     0,   529,   530,   133,     0,
       0,     0,   134,     0,   135,   136,   531,     0,     0,     0,
       0,     0,     0,   142,     0,   143,   144,   145,   146,   532,
     148,   149,   150,   151,   152,   153,     0,     0,  1189,   155,
       0,     0,   157,   430,   159,   160,   161,   162,   163,   164,
     165,   166,   167,   168,   169,   533,   171,   534,   173,   535,
     536,   176,   177,   178,   179,   180,   181,   537,   183,   538,
     539,   540,   187,   188,   541,   542,   191,   192,   193,   194,
     195,   196,   197,   198,   199,   200,   201,   202,   543,     0,
     204,   544,   206,     0,   207,   208,   209,   210,   211,   212,
     213,   214,   215,   216,   217,   218,   219,   220,   221,   222,
     223,   224,   225,   226,   227,   228,   229,   230,   231,   232,
     233,   234,   235,   236,   237,   238,   239,   240,   241,   242,
     243,   244,   245,   246,   247,   248,   249,   250,   251,   252,
     253,     3,     0,     0,     0,   256,   545,   258,  1190,  1191,
    1192,  1193,    11,    12,    13,    14,     0,     0,     0,     0,
     427,   428,     0,    18,    19,    20,   503,    22,    23,    24,
     504,    26,   505,   506,  1186,   508,   429,    32,   509,    34,
      35,    36,     0,    37,    38,    39,    40,   510,    42,    43,
      44,    45,    46,    47,    48,   511,     0,    50,   512,    52,
     513,   514,    55,   515,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,   516,   517,
      73,     0,    74,    75,    76,   518,     0,     0,    79,    80,
      81,     0,     0,   519,    83,     0,     0,     0,     0,    85,
     520,    87,   521,   522,    90,    91,  1688,     0,     0,     0,
       0,     0,     0,     0,     0,    96,    97,    98,    99,     0,
     100,     0,     0,     0,   523,   102,   103,   524,   525,     0,
       0,     0,     0,     0,     0,   106,   107,   108,     0,   110,
     111,  1187,     0,     0,     0,  1188,     0,     0,   118,   526,
       0,     0,     0,   120,   121,   122,   123,   527,   125,   126,
     528,     0,     0,     0,     0,     0,     0,     0,   529,   530,
     133,     0,     0,     0,   134,     0,   135,   136,   531,     0,
       0,     0,     0,     0,     0,   142,     0,   143,   144,   145,
     146,   532,   148,   149,   150,   151,   152,   153,     0,     0,
    1189,   155,     0,     0,   157,   430,   159,   160,   161,   162,
     163,   164,   165,   166,   167,   168,   169,   533,   171,   534,
     173,   535,   536,   176,   177,   178,   179,   180,   181,   537,
     183,   538,   539,   540,   187,   188,   541,   542,   191,   192,
     193,   194,   195,   196,   197,   198,   199,   200,   201,   202,
     543,     0,   204,   544,   206,     0,   207,   208,   209,   210,
     211,   212,   213,   214,   215,   216,   217,   218,   219,   220,
     221,   222,   223,   224,   225,   226,   227,   228,   229,   230,
     231,   232,   233,   234,   235,   236,   237,   238,   239,   240,
     241,   242,   243,   244,   245,   246,   247,   248,   249,   250,
     251,   252,   253,     3,     0,     0,     0,   256,   545,   258,
    1190,  1191,  1192,  1193,    11,    12,    13,    14,     0,     0,
       0,     0,   427,   428,     0,    18,    19,    20,   503,    22,
      23,    24,   504,    26,   505,   506,  1186,   508,   429,    32,
     509,    34,    35,    36,     0,    37,    38,    39,    40,   510,
      42,    43,    44,    45,    46,    47,    48,   511,     0,    50,
     512,    52,   513,   514,    55,   515,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
     516,   517,    73,     0,    74,    75,    76,   518,     0,     0,
      79,    80,    81,     0,     0,   519,    83,     0,     0,     0,
       0,    85,   520,    87,   521,   522,    90,    91,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    96,    97,    98,
      99,     0,   100,     0,     0,     0,   523,   102,   103,   524,
     525,     0,     0,     0,     0,     0,     0,   106,   107,   108,
       0,   110,   111,  1187,     0,     0,     0,  1188,     0,     0,
     118,   526,     0,     0,     0,   120,   121,   122,   123,   527,
     125,   126,   528,     0,     0,     0,     0,     0,     0,     0,
     529,   530,   133,     0,     0,     0,   134,     0,   135,   136,
     531,     0,     0,     0,     0,     0,     0,   142,     0,   143,
     144,   145,   146,   532,   148,   149,   150,   151,   152,   153,
       0,     0,  1189,   155,     0,     0,   157,   430,   159,   160,
     161,   162,   163,   164,   165,   166,   167,   168,   169,   533,
     171,   534,   173,   535,   536,   176,   177,   178,   179,   180,
     181,   537,   183,   538,   539,   540,   187,   188,   541,   542,
     191,   192,   193,   194,   195,   196,   197,   198,   199,   200,
     201,   202,   543,     0,   204,   544,   206,     0,   207,   208,
     209,   210,   211,   212,   213,   214,   215,   216,   217,   218,
     219,   220,   221,   222,   223,   224,   225,   226,   227,   228,
     229,   230,   231,   232,   233,   234,   235,   236,   237,   238,
     239,   240,   241,   242,   243,   244,   245,   246,   247,   248,
     249,   250,   251,   252,   253,     3,     0,     0,     0,   256,
     545,   258,  1190,  1191,  1192,  1193,    11,    12,    13,    14,
       0,     0,     0,     0,   427,   428,     0,    18,    19,    20,
     503,    22,    23,    24,   504,    26,   505,   506,  1186,   508,
     429,    32,   509,    34,    35,    36,     0,    37,    38,    39,
      40,   510,    42,    43,    44,    45,    46,    47,    48,   511,
       0,    50,   512,    52,   513,   514,    55,   515,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,   516,   517,    73,     0,    74,    75,    76,   518,
       0,     0,    79,    80,    81,     0,     0,   519,    83,     0,
       0,     0,     0,    85,   520,    87,   521,   522,    90,    91,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    96,
      97,    98,    99,     0,   100,     0,     0,     0,   523,   102,
     103,   524,   525,     0,     0,     0,     0,     0,     0,   106,
     107,   108,     0,   110,   111,  1187,     0,     0,     0,  1227,
       0,     0,   118,   526,     0,     0,     0,   120,   121,   122,
     123,   527,   125,   126,   528,     0,     0,     0,     0,     0,
       0,     0,   529,   530,   133,     0,     0,     0,   134,     0,
     135,   136,   531,     0,     0,     0,     0,     0,     0,   142,
       0,   143,   144,   145,   146,   532,   148,   149,   150,   151,
     152,   153,     0,     0,   154,   155,     0,     0,   157,   430,
     159,   160,   161,   162,   163,   164,   165,   166,   167,   168,
     169,   533,   171,   534,   173,   535,   536,   176,   177,   178,
     179,   180,   181,   537,   183,   538,   539,   540,   187,   188,
     541,   542,   191,   192,   193,   194,   195,   196,   197,   198,
     199,   200,   201,   202,   543,     0,   204,   544,   206,     0,
     207,   208,   209,   210,   211,   212,   213,   214,   215,   216,
     217,   218,   219,   220,   221,   222,   223,   224,   225,   226,
     227,   228,   229,   230,   231,   232,   233,   234,   235,   236,
     237,   238,   239,   240,   241,   242,   243,   244,   245,   246,
     247,   248,   249,   250,   251,   252,   253,     3,     0,   590,
     591,   256,   545,   258,  1190,  1191,  1192,  1193,    11,    12,
       0,     0,     0,     0,     0,     0,   427,   428,     0,    18,
      19,    20,   553,    22,    23,    24,   504,   554,   555,   506,
     507,   508,   429,    32,   509,    34,   556,    36,     0,    37,
      38,    39,    40,   557,    42,   558,   559,    45,    46,    47,
      48,   511,     0,    50,   560,    52,   513,   514,    55,   515,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,   516,   517,    73,     0,    74,    75,
      76,   561,     0,     0,    79,    80,    81,     0,     0,   519,
      83,     0,     0,     0,     0,    85,   520,    87,   521,   522,
      90,    91,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    96,    97,    98,    99,     0,   100,     0,     0,     0,
     523,   102,   103,   524,   525,     0,     0,     0,     0,     0,
       0,   106,   107,   108,     0,   110,   111,   112,     0,     0,
       0,     0,     0,     0,   118,   526,     0,     0,     0,   120,
     121,   122,   123,   527,   125,   126,   528,     0,     0,     0,
       0,     0,     0,     0,   529,   530,   133,     0,     0,     0,
     134,     0,   135,   136,   531,     0,     0,     0,     0,     0,
       0,   142,     0,   143,   144,   145,   146,   532,   148,   149,
     150,   151,   152,   153,     0,     0,   154,   155,     0,     0,
     157,   430,   159,   160,   161,   162,   163,   164,   165,   166,
     167,   168,   169,   533,   562,   534,   173,   535,   536,   176,
     177,   178,   179,   180,   181,   537,   183,   538,   539,   540,
     187,   188,   541,   542,   191,   192,   193,   194,   195,   196,
     197,   198,   199,   200,   201,   202,   543,     0,   204,   544,
     206,     0,   207,   208,   209,   210,   211,   212,   213,   214,
     215,   216,   217,   218,   219,   220,   221,   222,   223,   224,
     225,   226,   227,   228,   229,   230,   231,   232,   233,   234,
     235,   236,   237,   238,   239,   240,   241,   242,   243,   244,
     245,   246,   247,   248,   249,   250,   251,   252,   253,     3,
       0,     0,     0,   256,   545,   258,   259,   260,   261,   262,
      11,    12,     0,     0,     0,     0,     0,     0,   427,   428,
       0,    18,    19,    20,   553,    22,    23,    24,   504,   554,
     555,   506,   507,   508,   429,    32,   509,    34,   556,    36,
       0,    37,    38,    39,    40,   557,    42,   558,   559,    45,
      46,    47,    48,   511,     0,    50,   560,    52,   513,   514,
      55,   515,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,   516,   517,    73,     0,
      74,    75,    76,   561,     0,     0,    79,    80,    81,     0,
       0,   519,    83,     0,     0,     0,     0,    85,   520,    87,
     521,   522,    90,    91,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    96,    97,    98,    99,     0,   100,     0,
       0,     0,   523,   102,   103,   524,   525,     0,     0,     0,
       0,     0,     0,   106,   107,   108,     0,   110,   111,   112,
       0,     0,     0,     0,     0,     0,   118,   526,     0,     0,
       0,   120,   121,   122,   123,   527,   125,   126,   528,     0,
       0,     0,     0,     0,     0,     0,   529,   530,   133,     0,
       0,     0,   134,   787,   135,   136,   531,     0,     0,     0,
       0,   788,     0,   142,     0,   143,   144,   145,   146,   532,
     148,   149,   150,   151,   152,   153,     0,     0,   154,   155,
       0,     0,   157,   430,   159,   160,   161,   162,   163,   164,
     165,   166,   167,   168,   169,   533,   562,   534,   173,   535,
     536,   176,   177,   178,   179,   180,   181,   537,   183,   538,
     539,   540,   187,   188,   541,   542,   191,   192,   193,   194,
     195,   196,   197,   198,   199,   200,   201,   202,   543,     0,
     204,   544,   206,     0,   207,   208,   209,   210,   211,   212,
     213,   214,   215,   216,   217,   218,   219,   220,   221,   222,
     223,   224,   225,   226,   227,   228,   229,   230,   231,   232,
     233,   234,   235,   236,   237,   238,   239,   240,   241,   242,
     243,   244,   245,   246,   247,   248,   249,   250,   251,   252,
     253,     3,     0,     0,     0,   256,   545,   258,   259,   260,
     261,   262,    11,    12,     0,     0,     0,   813,     0,     0,
     427,   428,     0,    18,    19,    20,   553,    22,    23,    24,
     504,   554,   555,   506,   507,   508,   429,    32,   509,    34,
     556,    36,     0,    37,    38,    39,    40,   557,    42,   558,
     559,    45,    46,    47,    48,   511,     0,    50,   560,    52,
     513,   514,    55,   515,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,   516,   517,
      73,     0,    74,    75,    76,   561,     0,     0,    79,    80,
      81,     0,     0,   519,    83,     0,     0,     0,     0,    85,
     520,    87,   521,   522,    90,    91,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    96,    97,    98,    99,     0,
     100,     0,     0,     0,   523,   102,   103,   524,   525,     0,
       0,     0,     0,     0,     0,   106,   107,   108,     0,   110,
     111,   112,     0,     0,     0,     0,     0,     0,   118,   526,
       0,     0,     0,   120,   121,   122,   123,   527,   125,   126,
     528,     0,     0,     0,     0,     0,     0,     0,   529,   530,
     133,     0,     0,     0,   134,   814,   135,   136,   531,     0,
       0,     0,     0,     0,     0,   142,     0,   143,   144,   145,
     146,   532,   148,   149,   150,   151,   152,   153,     0,     0,
     154,   155,     0,     0,   157,   430,   159,   160,   161,   162,
     163,   164,   165,   166,   167,   168,   169,   533,   562,   534,
     173,   535,   536,   176,   177,   178,   179,   180,   181,   537,
     183,   538,   539,   540,   187,   188,   541,   542,   191,   192,
     193,   194,   195,   196,   197,   198,   199,   200,   201,   202,
     543,     0,   204,   544,   206,     0,   207,   208,   209,   210,
     211,   212,   213,   214,   215,   216,   217,   218,   219,   220,
     221,   222,   223,   224,   225,   226,   227,   228,   229,   230,
     231,   232,   233,   234,   235,   236,   237,   238,   239,   240,
     241,   242,   243,   244,   245,   246,   247,   248,   249,   250,
     251,   252,   253,     3,     0,     0,     0,   256,   545,   258,
     259,   260,   261,   262,    11,    12,     0,     0,     0,     0,
       0,     0,   427,   428,     0,    18,    19,    20,   553,    22,
      23,    24,   504,   554,   555,   506,   507,   508,   429,    32,
     509,    34,   556,    36,     0,    37,    38,    39,    40,   557,
      42,   558,   559,    45,    46,    47,    48,   511,     0,    50,
     560,    52,   513,   514,    55,   515,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
     516,   517,    73,     0,    74,    75,    76,   561,     0,     0,
      79,    80,    81,     0,     0,   519,    83,     0,     0,     0,
       0,    85,   520,    87,   521,   522,    90,    91,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    96,    97,    98,
      99,     0,   100,     0,     0,     0,   523,   102,   103,   524,
     525,     0,     0,     0,     0,     0,     0,   106,   107,   108,
       0,   110,   111,   112,     0,     0,     0,     0,     0,     0,
     118,   526,     0,     0,     0,   120,   121,   122,   123,   527,
     125,   126,   528,     0,     0,     0,     0,     0,     0,     0,
     529,   530,   133,     0,     0,     0,   134,   831,   135,   136,
     531,     0,     0,     0,     0,   832,     0,   142,     0,   143,
     144,   145,   146,   532,   148,   149,   150,   151,   152,   153,
       0,     0,   154,   155,     0,     0,   157,   430,   159,   160,
     161,   162,   163,   164,   165,   166,   167,   168,   169,   533,
     562,   534,   173,   535,   536,   176,   177,   178,   179,   180,
     181,   537,   183,   538,   539,   540,   187,   188,   541,   542,
     191,   192,   193,   194,   195,   196,   197,   198,   199,   200,
     201,   202,   543,     0,   204,   544,   206,     0,   207,   208,
     209,   210,   211,   212,   213,   214,   215,   216,   217,   218,
     219,   220,   221,   222,   223,   224,   225,   226,   227,   228,
     229,   230,   231,   232,   233,   234,   235,   236,   237,   238,
     239,   240,   241,   242,   243,   244,   245,   246,   247,   248,
     249,   250,   251,   252,   253,     3,     0,     0,     0,   256,
     545,   258,   259,   260,   261,   262,    11,    12,     0,     0,
       0,     0,     0,     0,   427,   428,     0,    18,    19,    20,
     553,    22,    23,    24,   504,   554,   555,   506,   507,   508,
     429,    32,   509,    34,   556,    36,     0,    37,    38,    39,
      40,   557,    42,   558,   559,    45,    46,    47,    48,   511,
       0,    50,   560,    52,   513,   514,    55,   515,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,   516,   517,    73,     0,    74,    75,    76,   561,
       0,     0,    79,    80,    81,     0,     0,   519,    83,     0,
       0,     0,     0,    85,   520,    87,   521,   522,    90,    91,
    1282,     0,     0,     0,     0,     0,     0,     0,     0,    96,
      97,    98,    99,     0,   100,     0,     0,     0,   523,   102,
     103,   524,   525,     0,     0,     0,     0,     0,     0,   106,
     107,   108,     0,   110,   111,   112,     0,     0,     0,     0,
       0,     0,   118,   526,     0,     0,     0,   120,   121,   122,
     123,   527,   125,   126,   528,     0,     0,     0,     0,     0,
       0,     0,   529,   530,   133,     0,     0,     0,   134,     0,
     135,   136,   531,     0,     0,     0,     0,     0,     0,   142,
       0,   143,   144,   145,   146,   532,   148,   149,   150,   151,
     152,   153,     0,     0,   154,   155,     0,     0,   157,   430,
     159,   160,   161,   162,   163,   164,   165,   166,   167,   168,
     169,   533,   562,   534,   173,   535,   536,   176,   177,   178,
     179,   180,   181,   537,   183,   538,   539,   540,   187,   188,
     541,   542,   191,   192,   193,   194,   195,   196,   197,   198,
     199,   200,   201,   202,   543,     0,   204,   544,   206,     0,
     207,   208,   209,   210,   211,   212,   213,   214,   215,   216,
     217,   218,   219,   220,   221,   222,   223,   224,   225,   226,
     227,   228,   229,   230,   231,   232,   233,   234,   235,   236,
     237,   238,   239,   240,   241,   242,   243,   244,   245,   246,
     247,   248,   249,   250,   251,   252,   253,     3,     0,     0,
       0,   256,   545,   258,   259,   260,   261,   262,    11,    12,
       0,     0,     0,     0,     0,     0,   427,   428,     0,    18,
      19,    20,   553,    22,    23,    24,   504,   554,   555,   506,
     507,   508,   429,    32,   509,    34,   556,    36,     0,    37,
      38,    39,    40,   557,    42,   558,   559,    45,    46,    47,
      48,   511,     0,    50,   560,    52,   513,   514,    55,   515,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,   516,   517,    73,     0,    74,    75,
      76,   561,     0,     0,    79,    80,    81,     0,     0,   519,
      83,     0,     0,     0,     0,    85,   520,    87,   521,   522,
      90,    91,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    96,    97,    98,    99,     0,   100,     0,     0,     0,
     523,   102,   103,   524,   525,     0,     0,     0,     0,     0,
       0,   106,   107,   108,     0,   110,   111,   112,     0,     0,
       0,     0,     0,     0,   118,   526,     0,     0,     0,   120,
     121,   122,   123,   527,   125,   126,   528,     0,     0,     0,
       0,     0,     0,     0,   529,   530,   133,     0,     0,     0,
     134,     0,   135,   136,   531,     0,     0,     0,     0,     0,
       0,   142,     0,   143,   144,   145,   146,   532,   148,   149,
     150,   151,   152,   153,     0,     0,   154,   155,     0,     0,
     157,   430,   159,   160,   161,   162,   163,   164,   165,   166,
     167,   168,   169,   533,   562,   534,   173,   535,   536,   176,
     177,   178,   179,   180,   181,   537,   183,   538,   539,   540,
     187,   188,   541,   542,   191,   192,   193,   194,   195,   196,
     197,   198,   199,   200,   201,   202,   543,     0,   204,   544,
     206,     0,   207,   208,   209,   210,   211,   212,   213,   214,
     215,   216,   217,   218,   219,   220,   221,   222,   223,   224,
     225,   226,   227,   228,   229,   230,   231,   232,   233,   234,
     235,   236,   237,   238,   239,   240,   241,   242,   243,   244,
     245,   246,   247,   248,   249,   250,   251,   252,   253,     0,
       0,     0,     0,   256,   545,   258,   259,   260,   261,   262,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
       0,    15,     0,     0,   427,   428,     0,    18,    19,    20,
     865,    22,    23,    24,    25,     0,   866,   506,    29,     0,
     429,    32,   509,    34,     0,    36,     0,    37,    38,    39,
      40,   867,    42,     0,     0,    45,    46,    47,    48,   511,
       0,    50,   868,    52,     0,     0,    55,   515,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,   516,   517,    73,     0,    74,    75,    76,   869,
       0,     0,    79,    80,    81,     0,     0,   519,    83,     0,
       0,     0,    84,    85,   520,    87,   521,   522,    90,    91,
     575,    93,     0,     0,     0,     0,     0,    95,     0,    96,
      97,    98,    99,     0,   100,     0,     0,     0,   523,   102,
     103,   524,   525,     0,     0,     0,     0,     0,     0,   106,
     107,   108,   109,   110,   111,     0,   576,   114,     0,   115,
     116,     0,   118,   119,     0,     0,     0,   120,   121,   122,
     123,   124,   125,   126,   528,     0,   128,     0,     0,     0,
       0,     0,   529,   530,   133,     0,     0,     0,   134,     0,
     135,   136,   531,     0,     0,     0,     0,     0,     0,   142,
       0,   143,   144,   145,   146,   147,   148,   149,   150,   151,
     152,   153,     0,     0,     0,   155,   156,     0,   157,   430,
     159,   160,   161,   162,   163,   164,   165,   166,   167,   168,
     169,   533,     0,   534,   173,   535,   536,   176,   177,   178,
     179,   180,   181,   537,   183,   538,   539,   540,   187,   188,
       0,   542,   191,   192,   193,   194,   195,   196,   197,   198,
     199,   200,   201,   202,   543,     0,   204,   544,   206,     0,
     207,   208,   209,   210,   211,   212,   213,   214,   215,   216,
     217,   218,   219,   220,   221,   222,   223,   224,   225,   226,
     227,   228,   229,   230,   231,   232,   233,   234,   235,   236,
     237,   238,   239,   240,   241,   242,   243,   244,   245,   246,
     247,   248,   249,   250,   251,   252,   253,   254,     0,   255,
      11,    12,   545,   258,     0,     0,     0,     0,   427,   428,
       0,    18,    19,    20,     0,    22,    23,    24,   504,     0,
       0,   506,   507,     0,   429,    32,   509,    34,     0,    36,
       0,    37,    38,    39,    40,     0,    42,     0,     0,    45,
      46,    47,    48,   511,     0,    50,     0,    52,     0,     0,
      55,   515,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,   516,   517,    73,     0,
      74,    75,    76,     0,     0,     0,    79,    80,    81,     0,
       0,   519,    83,     0,     0,     0,     0,    85,   520,    87,
     521,   522,    90,    91,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    96,    97,    98,    99,     0,   100,     0,
       0,     0,   523,   102,   103,   524,   525,     0,     0,     0,
       0,     0,     0,   106,   107,   108,     0,   110,   111,     0,
       0,     0,     0,     0,     0,     0,   118,   526,     0,     0,
       0,   120,   121,   122,   123,   527,   125,   126,   528,     0,
       0,     0,     0,     0,     0,     0,   529,   530,   133,     0,
       0,     0,   134,     0,   135,   136,   531,     0,     0,     0,
       0,     0,     0,   142,     0,   143,   144,   145,   146,   532,
     148,   149,   150,   151,   152,   153,     0,     0,     0,   155,
       0,     0,   157,   430,   159,   160,   161,   162,   163,   164,
     165,   166,   167,   168,   169,   533,     0,   534,   173,   535,
     536,   176,   177,   178,   179,   180,   181,   537,   183,   538,
     539,   540,   187,   188,     0,   542,   191,   192,   193,   194,
     195,   196,   197,   198,   199,   200,   201,   202,   543,     0,
     204,   544,   206,     0,   207,   208,   209,   210,   211,   212,
     213,   214,   215,   216,   217,   218,   219,   220,   221,   222,
     223,   224,   225,   226,   227,   228,   229,   230,   231,   232,
     233,   234,   235,   236,   237,   238,   239,   240,   241,   242,
     243,   244,   245,   246,   247,   248,   249,   250,   251,   252,
     253,     0,     0,     0,     0,     0,   545,   258
  };

  const short
  jsoniq_parser::yycheck_[] =
  {
       0,   323,   190,   697,   725,     5,     6,     7,     8,     9,
      10,   792,   323,   293,   852,   354,    16,     0,   698,   699,
     700,   701,     5,     6,     7,     8,     9,    10,   638,   713,
     956,   957,  1072,   603,     0,   889,   270,  1088,   608,     5,
       6,     7,     8,     9,    10,  1323,   442,   205,     5,     6,
       7,     8,     9,   271,   915,     5,     6,     7,     8,     9,
      10,  1341,   807,   640,   943,   792,   638,   639,   286,   440,
     441,  1405,  1406,     1,   140,   820,   302,  1114,   263,   264,
      38,   477,   478,     1,    78,  1172,  1035,    29,    30,   834,
     636,   637,   638,   639,    41,  1542,   636,   637,  1545,    46,
      47,  1053,    32,  1001,  1452,  1354,   233,    49,    55,    58,
     121,     1,     1,   113,     1,    62,    38,    64,    65,   151,
     121,    32,     1,   162,   985,     1,    73,  1653,     1,  1055,
     113,    99,     1,     1,   110,    77,  1062,     1,    29,    30,
     179,    99,   233,    96,   235,     1,    97,   113,   110,     1,
       1,   135,   151,   100,   676,   233,   678,   679,    49,   681,
      57,   683,    59,   113,   110,     1,     1,   110,    59,   691,
     692,   125,   749,   115,   160,    97,     1,    99,   255,   256,
     157,   461,   110,     1,    39,    40,    77,    42,    43,   956,
     957,    82,   110,   151,     1,   322,    33,     1,   271,    56,
     151,   278,   157,   180,    61,    60,   103,   433,   146,   282,
     194,   114,   115,   439,     5,     6,     7,     8,     9,    10,
     110,     8,   943,   114,   115,   116,     1,   157,   190,   151,
     160,   322,   233,   186,  1001,     1,     1,     1,   277,     1,
       0,     1,    99,   110,   322,   284,   110,   190,   194,   281,
     180,     1,  1586,   164,   110,   294,     0,     1,   110,   110,
     582,   116,  1007,   263,   264,   241,  1306,   267,  1035,  1320,
     281,   582,  1110,   222,   110,   110,     1,  1803,  1556,  1060,
     263,   264,   281,    99,   267,   110,   233,   241,  1055,   899,
    1188,   357,   110,  1172,  1173,  1062,   264,   263,   264,   579,
     190,   267,   302,   110,  1652,   194,   110,   194,  1345,   164,
    1559,     9,    10,   262,   315,   194,  1061,  1354,   194,   302,
    1767,   194,   113,   281,  1273,   194,   194,   899,   900,  1227,
    1282,  1058,    99,  1060,   190,   110,   302,   267,   190,   190,
       1,  1428,   568,   281,   110,   110,   110,  1074,   110,     1,
     110,   254,   255,   295,   190,   190,   172,   897,   898,   281,
     110,   303,   259,  1224,   760,   190,   110,  1814,   362,  1816,
     188,   365,  1298,   228,   103,   272,   243,   274,   325,   326,
     327,   218,    99,   190,   110,   110,   190,   116,   285,   286,
     287,   178,   289,   321,   291,     1,    32,   110,   103,   302,
      79,   319,   636,   637,   295,   172,     1,   110,   106,   805,
     987,     1,   303,   188,   269,   633,   634,   939,  1752,  1164,
     311,  1188,   134,   187,   190,   190,   644,   187,   190,   127,
      47,   129,    32,   433,     1,     1,   662,   187,    97,   439,
     625,   110,   160,   187,   260,   261,   110,   263,    65,   110,
     433,  1172,  1173,   233,   270,   235,   439,    97,   110,   157,
    1227,   307,   187,   161,   280,   191,   466,   433,   468,   162,
     316,   134,  1806,   439,  1714,    54,   292,   190,   157,   177,
     116,   110,   110,   466,   189,   468,   179,   192,   191,   201,
     490,   110,   151,   260,   261,   495,   263,   121,   140,    97,
     466,   199,   468,  1284,   110,  1226,  1273,   490,    47,  1789,
      49,   618,   495,   280,   121,   110,   466,   675,   468,   217,
     110,   190,  1559,   187,   490,   292,   187,  1272,   164,   495,
     433,  1298,   164,  1387,  1460,   187,   439,  1391,   201,  1149,
     490,  1151,   322,   110,   110,   495,    99,   264,   265,  1428,
     229,   230,  1792,    32,   133,  1322,  1834,   157,  1437,   123,
     160,   190,   190,   205,  1134,   672,  1293,   470,   568,  1448,
     109,   190,   475,   190,     3,    99,   576,  1149,  1150,  1151,
     180,   187,   485,   190,  1268,   568,   226,   227,   126,   492,
     493,    44,   187,   576,   158,    48,   160,   187,   501,   185,
      53,   241,   568,  1149,  1150,  1151,    47,   160,    49,   980,
     576,    97,  1008,  1009,   200,   110,   616,  1013,     5,    72,
     187,   187,     9,   195,   196,   625,   576,   151,   226,   227,
    1670,   631,   171,   616,   113,   110,   636,   637,   638,   639,
      69,    82,   625,   241,     5,    74,   110,  1043,     9,   128,
     616,   717,   718,   636,   637,   638,   639,    99,   110,   625,
     146,   887,   662,   897,  1409,   568,   616,   240,   109,   242,
     636,   637,   638,   639,   153,   466,   172,   468,   121,   662,
      99,   245,   169,   586,   587,   903,     9,    10,    99,   907,
     121,   917,   121,  1460,   597,   190,   662,   116,   185,   490,
       1,     9,    29,    30,   495,   116,     1,  1428,     3,   151,
     206,    99,   615,   616,   714,   190,  1437,   120,   171,  1557,
     157,   174,    49,    31,  1562,   157,   190,  1448,   116,    97,
     171,   714,     9,  1514,    35,   160,   189,  1788,   190,   192,
     127,   160,   129,  1622,    97,  1439,   189,   150,   714,   192,
     137,    97,   160,     1,    31,   123,  1682,   714,   189,   109,
     163,   192,    56,   216,   714,   185,   127,    61,   129,     1,
     157,     3,   160,   222,    69,  1542,   137,   214,  1545,    74,
     200,   131,   214,   106,  1394,   576,    80,    35,   115,   148,
     158,   162,   160,   146,    35,   198,   157,   797,   205,    97,
     146,   157,   160,    51,   127,    99,   129,   214,   179,  1735,
      51,  1737,   265,   262,   797,  1499,   719,  1779,  1780,   127,
     179,   129,  1394,   726,   180,   149,   121,   121,   828,   829,
     138,   797,   203,   123,   157,  1174,   160,    69,   161,   198,
     797,   162,    74,   141,   160,   828,   829,   797,   146,   157,
     127,  1049,   129,  1051,   177,    35,  1694,    27,   179,  1821,
    1698,   138,   828,   829,   126,  1827,    36,   140,   158,   641,
     160,    51,   141,   645,  1800,    98,   199,   146,   828,   829,
     157,   154,   203,    29,    30,   189,   109,   887,   192,   121,
     141,     9,    10,    63,   217,   157,   119,  1095,   160,  1097,
     152,  1622,   152,    49,   887,  1301,   157,   233,   131,   909,
     214,   141,   238,  1139,  1194,  1682,   168,   917,   168,  1613,
    1614,   887,  1140,   714,     9,    10,  1606,   157,  1146,    29,
      30,   276,   132,   133,   917,   103,   277,   103,   283,   103,
     843,   844,   845,   284,   847,  1783,   103,   103,   116,    49,
     116,   917,   116,   294,   160,  1143,   956,   957,   271,   116,
     116,    97,   998,   160,   720,    29,    30,   723,  1735,   282,
    1737,  1655,   728,   109,  1010,    39,    40,   160,    42,    43,
     285,   286,   287,   160,   887,    49,   291,   141,   106,    56,
     157,   126,   146,   160,    61,    59,    60,  1319,   126,   157,
    1767,  1001,   160,   132,   133,  1401,   797,  1752,   121,   127,
     126,   129,   157,    77,  1410,   160,   157,   189,    82,   160,
     192,   106,   361,   695,   696,   364,   229,   230,   231,  1723,
    1724,   126,   240,  1800,   242,   702,   703,   828,   829,   157,
    1643,  1644,   127,   161,   129,  1266,  1267,  1814,   121,  1816,
     114,   115,   116,   704,   705,  1055,  1378,   126,   126,   177,
     157,   157,  1062,   966,  1386,    78,   126,   126,   126,   126,
     157,   221,   157,  1757,  1300,  1386,   161,   160,   121,    92,
     249,   199,   160,   157,   249,   322,     0,   250,   194,   170,
    1090,    95,   177,   102,    52,   257,   204,   104,   197,   217,
     164,   187,  1796,   116,   105,   158,   160,  1090,   187,    32,
     187,   187,   187,    49,   199,   128,   187,   130,   156,   121,
    1804,   254,   312,   157,  1090,   190,     1,  1523,   121,   146,
     110,   157,   217,   157,   190,   190,   146,   160,   160,  1139,
    1090,   160,   160,   187,   160,   160,   125,   125,  1148,  1149,
    1150,  1151,   160,    37,   160,    17,  1139,   157,    32,   239,
     253,   194,   194,     3,   228,   121,  1149,  1150,  1151,   218,
     173,   292,   171,  1139,   121,   173,    73,   292,   239,    97,
      97,    97,   153,  1149,  1150,  1151,   110,    32,  1188,   110,
     164,   164,  1876,   190,  1878,  1383,   131,   136,    97,   121,
     307,   146,   254,   136,  1888,   269,   187,   190,   190,  1531,
     187,  1533,   110,    99,  1536,  1537,    97,    47,  1540,    99,
     190,   190,   190,   190,  1546,   187,   157,  1227,   187,   157,
     157,   295,   136,    97,   245,   146,   241,   157,   157,   303,
     144,   145,   157,   147,   194,   187,   136,   311,   173,    97,
     146,   155,    32,   294,    97,   159,   307,   161,   131,   235,
     121,   121,   166,   157,   151,    97,   136,   136,  1171,   164,
     202,    99,   157,   151,   187,   121,  1276,   103,   182,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,   103,
     157,   146,   194,  1276,   187,   187,   125,    37,  1298,  1090,
    1300,   125,    32,   207,   208,   209,   210,   211,   212,  1535,
    1276,  1311,   121,   157,    82,   253,   110,  1300,   109,  1276,
     110,   262,  1322,   266,   268,   160,  1276,   293,  1311,   275,
    1330,   148,    32,   160,  1300,   160,    32,   160,   160,   160,
     160,   294,    32,   295,   235,  1311,   148,  1330,   190,   362,
     314,  1351,   365,  1579,  1311,   194,   190,   190,  1358,   190,
     131,  1311,   151,  1685,  1330,   151,   110,  1689,  1351,   146,
     103,   187,   190,  1330,   136,  1358,     7,   190,   241,   194,
    1330,    99,   241,   202,   241,  1351,   160,   241,   160,   245,
    1390,   160,  1358,   157,   109,   151,   121,   121,    80,   151,
     258,  1351,   190,    81,    32,   258,   294,  1390,  1358,   190,
      32,   290,   190,   279,   190,   190,   110,   190,   173,   151,
     190,   187,   151,   190,  1390,   130,   130,   265,    99,   187,
     187,   157,   146,   446,   447,   324,   187,   450,   187,   187,
    1390,   454,  1668,    97,   457,   458,    32,   288,   190,    99,
     173,  1773,   465,  1679,  1680,   190,   151,  1683,   471,   472,
    1460,    97,   201,    56,  1690,    97,   479,    32,   273,   110,
     110,    97,   121,   486,   487,   488,   489,    97,   121,   233,
     201,   494,   201,   295,   121,  1276,   202,   233,    96,   502,
     201,  1813,   121,   186,   121,  1817,   203,   295,   301,   233,
     121,   759,   313,   313,   267,   313,   267,   314,   314,  1228,
     270,  1002,   441,   314,  1063,  1273,  1757,  1804,   890,  1169,
    1311,   461,   810,  1599,  1420,  1156,  1412,  1849,  1408,   564,
    1852,  1405,  1317,  1312,  1320,  1535,   549,  1325,   959,  1330,
     674,   694,  1542,   725,   725,  1545,   731,  1345,  1827,   349,
    1776,   564,  1535,  1875,  1821,   852,  1194,   469,  1051,   469,
    1351,  1194,   575,  1377,  1388,  1387,  1437,  1358,  1622,  1535,
    1448,  1173,  1619,  1182,  1034,  1441,  1250,  1489,  1646,  1579,
    1438,   606,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1815,
     603,    -1,    -1,    -1,    -1,   608,  1579,    -1,   611,    -1,
      -1,    -1,    -1,    -1,    -1,   618,    -1,  1833,    -1,    -1,
      -1,    -1,    -1,  1579,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1848,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1870,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1879,    -1,    -1,    -1,    -1,    -1,   672,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1668,    -1,
      -1,    -1,    -1,  1576,  1577,  1578,    -1,    -1,    -1,  1679,
    1680,    -1,  1682,  1683,    -1,  1668,    -1,    -1,    -1,    -1,
    1690,    -1,    -1,    -1,    -1,    -1,  1679,  1680,    -1,    -1,
    1683,    -1,  1668,    -1,    -1,    -1,    -1,  1690,    -1,    -1,
      -1,    -1,   725,  1679,  1680,    92,    -1,  1683,    -1,    -1,
      -1,    -1,    -1,    -1,  1690,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1735,   113,  1737,    -1,   116,
     753,    -1,    -1,   756,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   128,    -1,   130,    -1,    -1,    -1,   770,    -1,    -1,
      -1,   774,   775,    -1,    -1,    -1,    -1,  1767,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1776,    -1,    -1,    -1,
     793,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   801,    -1,
      -1,   804,    -1,  1776,    -1,   808,    -1,    -1,    -1,    -1,
    1800,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1711,    -1,
    1776,    -1,    -1,    -1,  1814,  1815,  1816,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1733,    -1,  1815,  1833,    -1,    -1,    -1,    -1,   851,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1848,  1815,
    1833,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1848,    -1,  1833,    -1,    -1,
    1870,    -1,    -1,    -1,    -1,    -1,   889,    -1,    -1,  1879,
      -1,    -1,  1848,    -1,    -1,    -1,    -1,  1870,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1879,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1870,    -1,    -1,    -1,   921,    -1,
      -1,    -1,    -1,  1879,    -1,    -1,    -1,    -1,   931,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   956,   957,   958,   959,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1001,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1015,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1040,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1049,    -1,  1051,    -1,
    1053,    -1,  1055,    -1,    -1,    -1,    -1,    -1,    -1,  1062,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     447,    -1,    -1,   450,    -1,    -1,    -1,   454,    -1,    -1,
     457,   458,  1095,    -1,  1097,    -1,    -1,    -1,   465,    -1,
      -1,    -1,    -1,    -1,   471,   472,    -1,    -1,    -1,    -1,
      -1,    -1,   479,    -1,    -1,    -1,    -1,    -1,    -1,   486,
     487,   488,   489,    -1,    -1,    -1,    -1,   494,    -1,    -1,
      -1,  1134,    -1,    -1,    -1,   502,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1160,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   549,    -1,    -1,  1188,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   564,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   575,   576,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1227,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   606,
      -1,    -1,    -1,    -1,   611,    -1,    -1,    -1,    -1,    -1,
      -1,   618,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1282,
      -1,    -1,    -1,  1286,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1298,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   672,    -1,    -1,    -1,  1312,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1322,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1352,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   725,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1377,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1387,    -1,    -1,    -1,  1391,   756,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   770,    -1,    -1,    -1,   774,   775,    -1,
      -1,    -1,   113,    -1,    -1,    -1,  1419,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   793,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   801,    -1,    -1,   804,    -1,    -1,
      -1,   808,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1460,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   840,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   851,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1495,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1539,    -1,    -1,  1542,
      -1,    -1,  1545,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   921,  1558,    -1,    -1,    -1,    -1,
    1563,    -1,    -1,    -1,   931,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   956,
     957,   958,   959,  1596,  1597,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1001,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1015,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1040,    -1,    -1,    -1,    -1,    -1,  1682,
      -1,  1684,  1049,  1686,  1051,  1688,  1053,    -1,  1055,    -1,
      -1,    -1,  1695,    -1,    -1,  1062,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1095,    -1,
    1097,    -1,  1735,    -1,  1737,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1753,    -1,    -1,  1756,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1767,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1778,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1160,    -1,    -1,    -1,  1800,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1808,    -1,    -1,    -1,    -1,
      -1,  1814,    -1,  1816,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1188,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1837,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1846,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1227,    -1,    -1,  1866,    -1,    -1,    -1,    -1,  1871,    -1,
      -1,    -1,    -1,    -1,    -1,   576,    -1,    -1,    -1,    -1,
    1883,    -1,    -1,  1886,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     601,   602,    -1,   604,   605,   606,   607,    -1,   609,    -1,
      -1,    -1,    -1,   614,    -1,  1282,    -1,    -1,    -1,  1286,
      -1,    -1,    -1,   624,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1298,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1312,   647,   648,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1322,    -1,   658,    -1,    -1,
      -1,   662,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1352,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1377,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   727,    -1,    -1,    -1,
     731,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1419,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   772,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1460,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   809,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1495,    -1,
      -1,    -1,    -1,    -1,   835,    -1,    -1,    -1,   839,   840,
      -1,   842,    -1,    -1,    -1,    -1,    -1,    -1,   849,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1539,    -1,    -1,  1542,    -1,    -1,  1545,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1558,    -1,    -1,    -1,    -1,  1563,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   917,    -1,    -1,    -1,
      -1,    -1,   923,   924,    -1,    -1,    -1,    -1,    -1,  1596,
    1597,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1003,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1682,    -1,  1684,    -1,  1686,
      -1,  1688,    -1,    -1,    -1,    -1,    -1,    -1,  1695,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1038,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1057,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1735,    -1,
    1737,    -1,  1073,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1084,  1085,    -1,  1753,    -1,    -1,  1756,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1099,    -1,
    1767,  1102,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1778,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1121,  1122,  1123,    -1,  1125,  1126,  1127,  1128,  1129,  1130,
    1131,  1132,  1133,  1800,    -1,  1136,  1137,    -1,    -1,    -1,
      -1,  1808,    -1,    -1,  1145,    -1,    -1,  1814,    -1,  1816,
      -1,    -1,    -1,    -1,    -1,  1156,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1837,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1846,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1866,
      -1,    -1,    -1,    -1,  1871,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1214,    -1,    -1,  1883,    -1,    -1,  1886,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1288,    -1,  1290,
      -1,  1292,    -1,    -1,     0,     1,    -1,     3,    -1,  1300,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1309,    -1,
      -1,    -1,    -1,    -1,  1315,  1316,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1334,    -1,    -1,  1337,  1338,    44,    -1,
      -1,    -1,    48,    -1,    -1,    -1,    52,    53,    -1,    -1,
      -1,    -1,    58,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    72,    -1,    74,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    95,
      -1,    97,    98,    99,    -1,    -1,   102,   103,   104,   105,
      -1,    -1,    -1,   109,   110,    -1,    -1,  1408,    -1,    -1,
     116,  1412,  1413,   119,   120,   121,    -1,  1418,    -1,   125,
      -1,    -1,    -1,    -1,   130,   131,    -1,    -1,    -1,    -1,
     136,    -1,    -1,    -1,   140,    -1,    -1,    -1,   144,   145,
      -1,   147,    -1,   149,   150,    -1,    -1,    -1,   154,   155,
      -1,    -1,   158,   159,   160,   161,   162,   163,    -1,    -1,
     166,    -1,    -1,    -1,   170,   171,    -1,    -1,   174,    -1,
      -1,    -1,    -1,   179,    -1,    -1,   182,    -1,    -1,    -1,
      -1,   187,   188,   189,   190,   191,   192,    -1,   194,   195,
     196,   197,   198,    -1,    -1,    -1,    -1,   203,   204,   205,
      -1,   207,   208,   209,   210,   211,   212,  1508,   214,    -1,
     216,  1512,    -1,    -1,    -1,  1516,   222,  1518,    -1,    -1,
     226,   227,    -1,    -1,    -1,  1526,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1534,  1535,   241,    -1,   243,    -1,   245,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   257,    -1,    -1,  1555,    -1,   262,    -1,    -1,   265,
      -1,    -1,    -1,    -1,    -1,    -1,  1567,  1568,  1569,  1570,
      -1,  1572,  1573,    -1,  1575,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1590,
      -1,    -1,    -1,    -1,  1595,    -1,    -1,    -1,    -1,    -1,
    1601,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   319,    -1,   321,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1661,    -1,  1663,    -1,    -1,  1666,    -1,    -1,  1669,    -1,
      -1,    -1,    -1,  1674,  1675,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1683,    -1,    -1,    -1,  1687,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1704,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1715,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1760,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1772,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1815,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1844,    -1,    -1,    -1,  1848,    -1,    -1,
      -1,    -1,  1853,     0,     1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    11,    12,    -1,    -1,  1869,    16,
      -1,    -1,  1873,    20,    21,    22,    23,    24,    25,    26,
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
     317,   318,    -1,   320,    -1,   322,   323,   324,   325,   326,
     327,   328,     0,     1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    11,    12,    -1,    -1,    -1,    16,    -1,
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
     318,     1,   320,    -1,   322,   323,   324,   325,   326,   327,
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
     310,   311,   312,   313,   314,   315,   316,   317,   318,     1,
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
     322,   323,   324,   325,   326,   327,   328,     0,     1,    -1,
       3,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    44,    -1,    -1,    -1,    48,    -1,    -1,    -1,    52,
      53,    -1,    -1,    -1,    -1,    58,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    72,
      -1,    74,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    95,    -1,    97,    98,    99,    -1,    -1,   102,
     103,   104,   105,    -1,    -1,    -1,   109,   110,    -1,    -1,
      -1,    -1,    -1,   116,    -1,    -1,   119,   120,   121,    -1,
      -1,    -1,   125,    -1,    -1,    -1,    -1,   130,   131,    -1,
      -1,    -1,    -1,   136,    -1,    -1,    -1,   140,    -1,    -1,
      -1,   144,   145,    -1,   147,    -1,   149,   150,    -1,    -1,
      -1,   154,   155,    -1,    -1,   158,   159,   160,   161,   162,
     163,    -1,    -1,   166,    -1,    -1,    -1,   170,   171,    -1,
      -1,   174,    -1,    -1,    -1,    -1,   179,    -1,    -1,   182,
      -1,    -1,    -1,    -1,   187,   188,   189,   190,   191,   192,
      -1,   194,   195,   196,   197,   198,    -1,    -1,    -1,    -1,
     203,   204,   205,    -1,   207,   208,   209,   210,   211,   212,
      -1,   214,    -1,   216,    -1,    -1,    -1,    -1,    -1,   222,
      -1,    -1,    -1,   226,   227,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   241,    -1,
     243,    -1,   245,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   257,    -1,    -1,    -1,    -1,   262,
      -1,    -1,   265,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    11,    12,    -1,    -1,
      -1,    16,    -1,    -1,    -1,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,   319,    32,   321,    -1,
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
     315,   316,   317,   318,    -1,   320,    -1,   322,   323,   324,
     325,   326,   327,   328,   329,   330,    11,    12,    -1,    -1,
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
     315,   316,   317,   318,    -1,   320,    -1,   322,   323,   324,
     325,   326,   327,   328,    -1,   330,    11,    12,    -1,    -1,
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
     185,    -1,   187,    -1,   189,    -1,   191,   192,   193,   194,
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
     315,   316,   317,   318,    -1,   320,    -1,   322,   323,   324,
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
      -1,   148,    -1,   150,   151,   152,   153,   154,   155,   156,
     157,   158,    -1,   160,   161,   162,   163,   164,    -1,    -1,
      -1,   168,   169,   170,   171,   172,   173,   174,   175,    -1,
     177,    -1,   179,   180,    -1,    -1,   183,   184,   185,    -1,
      -1,    -1,   189,   190,   191,   192,   193,    -1,   195,   196,
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
     317,   318,    -1,   320,    -1,   322,   323,   324,   325,   326,
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
     139,   140,   141,   142,   143,    -1,    -1,    -1,    -1,   148,
      -1,   150,   151,   152,   153,   154,   155,   156,   157,   158,
      -1,   160,   161,   162,   163,   164,    -1,    -1,    -1,   168,
     169,   170,   171,   172,   173,   174,   175,    -1,   177,    -1,
     179,   180,    -1,    -1,   183,   184,   185,    -1,    -1,    -1,
     189,   190,   191,   192,   193,    -1,   195,   196,    -1,   198,
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
     121,   122,    -1,   124,    -1,    -1,    -1,   128,    -1,   130,
     131,   132,   133,    -1,   135,    -1,    -1,    -1,   139,   140,
     141,   142,   143,    -1,    -1,    -1,    -1,    -1,    -1,   150,
     151,   152,   153,   154,   155,   156,   157,   158,    -1,   160,
     161,   162,   163,   164,    -1,    -1,    -1,   168,   169,   170,
     171,   172,   173,   174,   175,    -1,   177,    -1,   179,   180,
      -1,    -1,   183,   184,   185,    -1,    -1,    -1,   189,    -1,
     191,   192,   193,   194,   195,   196,    -1,   198,    -1,   200,
      -1,   202,   203,   204,   205,   206,   207,   208,   209,   210,
     211,   212,    -1,    -1,   215,   216,   217,    -1,   219,   220,
     221,   222,   223,   224,   225,   226,   227,   228,   229,   230,
     231,   232,   233,   234,   235,   236,   237,   238,   239,   240,
     241,   242,   243,   244,   245,   246,   247,   248,   249,   250,
     251,   252,   253,   254,   255,   256,   257,   258,   259,   260,
     261,   262,   263,   264,   265,    -1,   267,   268,   269,    -1,
     271,   272,   273,   274,   275,   276,   277,   278,   279,   280,
     281,   282,   283,   284,   285,   286,   287,   288,   289,   290,
     291,   292,   293,   294,   295,   296,   297,   298,   299,   300,
     301,   302,   303,   304,   305,   306,   307,   308,   309,   310,
     311,   312,   313,   314,   315,   316,   317,   318,    -1,   320,
      -1,   322,   323,   324,   325,   326,   327,   328,    11,    12,
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
      -1,   124,    -1,    -1,    -1,   128,    -1,   130,   131,   132,
     133,    -1,   135,    -1,    -1,    -1,   139,   140,   141,   142,
     143,    -1,    -1,    -1,    -1,    -1,    -1,   150,   151,   152,
     153,   154,   155,   156,   157,   158,    -1,   160,   161,   162,
     163,   164,    -1,    -1,    -1,   168,   169,   170,   171,   172,
     173,   174,   175,    -1,   177,    -1,   179,   180,    -1,    -1,
     183,   184,   185,    -1,    -1,   188,   189,    -1,   191,   192,
     193,    -1,   195,   196,    -1,   198,    -1,   200,    -1,   202,
     203,   204,   205,   206,   207,   208,   209,   210,   211,   212,
      -1,    -1,   215,   216,   217,    -1,   219,   220,   221,   222,
     223,   224,   225,   226,   227,   228,   229,   230,   231,   232,
     233,   234,   235,   236,   237,   238,   239,   240,   241,   242,
     243,   244,   245,   246,   247,   248,   249,   250,   251,   252,
     253,   254,   255,   256,   257,   258,   259,   260,   261,   262,
     263,   264,   265,    -1,   267,   268,   269,    -1,   271,   272,
     273,   274,   275,   276,   277,   278,   279,   280,   281,   282,
     283,   284,   285,   286,   287,   288,   289,   290,   291,   292,
     293,   294,   295,   296,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,   311,   312,
     313,   314,   315,   316,   317,   318,    -1,   320,    -1,   322,
     323,   324,   325,   326,   327,   328,    11,    12,    -1,    -1,
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
     185,    -1,    -1,    -1,   189,   190,   191,   192,   193,    -1,
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
     315,   316,   317,   318,    -1,   320,    -1,   322,   323,   324,
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
      -1,    -1,   189,    -1,   191,   192,   193,    -1,   195,   196,
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
     317,   318,   319,   320,    -1,   322,   323,   324,   325,   326,
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
      -1,   320,   321,   322,   323,   324,   325,   326,   327,   328,
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
     121,   122,    -1,   124,    -1,    -1,    -1,   128,    -1,   130,
     131,   132,   133,    -1,   135,    -1,    -1,    -1,   139,   140,
     141,   142,   143,    -1,    -1,    -1,    -1,    -1,    -1,   150,
     151,   152,   153,   154,   155,   156,   157,   158,    -1,   160,
     161,   162,   163,   164,    -1,    -1,    -1,   168,   169,   170,
     171,   172,   173,   174,   175,    -1,   177,    -1,   179,   180,
      -1,    -1,   183,   184,   185,    -1,    -1,    -1,   189,    -1,
     191,   192,   193,   194,   195,   196,    -1,   198,    -1,   200,
      -1,   202,   203,   204,   205,   206,   207,   208,   209,   210,
     211,   212,    -1,    -1,   215,   216,   217,    -1,   219,   220,
     221,   222,   223,   224,   225,   226,   227,   228,   229,   230,
     231,   232,   233,   234,   235,   236,   237,   238,   239,   240,
     241,   242,   243,   244,   245,   246,   247,   248,   249,   250,
     251,   252,   253,   254,   255,   256,   257,   258,   259,   260,
     261,   262,   263,   264,   265,    -1,   267,   268,   269,    -1,
     271,   272,   273,   274,   275,   276,   277,   278,   279,   280,
     281,   282,   283,   284,   285,   286,   287,   288,   289,   290,
     291,   292,   293,   294,   295,   296,   297,   298,   299,   300,
     301,   302,   303,   304,   305,   306,   307,   308,   309,   310,
     311,   312,   313,   314,   315,   316,   317,   318,    -1,   320,
      -1,   322,   323,   324,   325,   326,   327,   328,    11,    12,
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
      -1,   124,    -1,    -1,    -1,   128,    -1,   130,   131,   132,
     133,    -1,   135,    -1,    -1,    -1,   139,   140,   141,   142,
     143,    -1,    -1,    -1,    -1,    -1,    -1,   150,   151,   152,
     153,   154,   155,   156,   157,   158,    -1,   160,   161,   162,
     163,   164,    -1,    -1,    -1,   168,   169,   170,   171,   172,
     173,   174,   175,    -1,   177,    -1,   179,   180,    -1,    -1,
     183,   184,   185,    -1,    -1,    -1,   189,    -1,   191,   192,
     193,   194,   195,   196,    -1,   198,    -1,   200,    -1,   202,
     203,   204,   205,   206,   207,   208,   209,   210,   211,   212,
      -1,    -1,   215,   216,   217,    -1,   219,   220,   221,   222,
     223,   224,   225,   226,   227,   228,   229,   230,   231,   232,
     233,   234,   235,   236,   237,   238,   239,   240,   241,   242,
     243,   244,   245,   246,   247,   248,   249,   250,   251,   252,
     253,   254,   255,   256,   257,   258,   259,   260,   261,   262,
     263,   264,   265,    -1,   267,   268,   269,    -1,   271,   272,
     273,   274,   275,   276,   277,   278,   279,   280,   281,   282,
     283,   284,   285,   286,   287,   288,   289,   290,   291,   292,
     293,   294,   295,   296,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,   311,   312,
     313,   314,   315,   316,   317,   318,    -1,   320,    -1,   322,
     323,   324,   325,   326,   327,   328,    11,    12,    -1,    -1,
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
     315,   316,   317,   318,    -1,   320,    -1,   322,   323,   324,
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
     317,   318,    -1,   320,    -1,   322,   323,   324,   325,   326,
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
     179,   180,    -1,    -1,   183,   184,   185,    -1,    -1,   188,
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
     121,   122,    -1,   124,    -1,    -1,    -1,   128,    -1,   130,
     131,   132,   133,    -1,   135,    -1,    -1,    -1,   139,   140,
     141,   142,   143,    -1,    -1,    -1,    -1,    -1,    -1,   150,
     151,   152,   153,   154,   155,   156,   157,   158,    -1,   160,
     161,   162,   163,   164,    -1,    -1,    -1,   168,   169,   170,
     171,   172,   173,   174,   175,    -1,   177,    -1,   179,   180,
      -1,    -1,   183,   184,   185,    -1,    -1,    -1,   189,   190,
     191,   192,   193,    -1,   195,   196,    -1,   198,    -1,   200,
      -1,   202,   203,   204,   205,   206,   207,   208,   209,   210,
     211,   212,    -1,    -1,   215,   216,   217,    -1,   219,   220,
     221,   222,   223,   224,   225,   226,   227,   228,   229,   230,
     231,   232,   233,   234,   235,   236,   237,   238,   239,   240,
     241,   242,   243,   244,   245,   246,   247,   248,   249,   250,
     251,   252,   253,   254,   255,   256,   257,   258,   259,   260,
     261,   262,   263,   264,   265,    -1,   267,   268,   269,    -1,
     271,   272,   273,   274,   275,   276,   277,   278,   279,   280,
     281,   282,   283,   284,   285,   286,   287,   288,   289,   290,
     291,   292,   293,   294,   295,   296,   297,   298,   299,   300,
     301,   302,   303,   304,   305,   306,   307,   308,   309,   310,
     311,   312,   313,   314,   315,   316,   317,   318,    -1,   320,
      -1,   322,   323,   324,   325,   326,   327,   328,    11,    12,
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
      -1,   124,    -1,    -1,    -1,   128,    -1,   130,   131,   132,
     133,    -1,   135,    -1,    -1,    -1,   139,   140,   141,   142,
     143,    -1,    -1,    -1,    -1,    -1,    -1,   150,   151,   152,
     153,   154,   155,   156,   157,   158,    -1,   160,   161,   162,
     163,   164,    -1,    -1,    -1,   168,   169,   170,   171,   172,
     173,   174,   175,    -1,   177,    -1,   179,   180,    -1,    -1,
     183,   184,   185,    -1,    -1,    -1,   189,    -1,   191,   192,
     193,   194,   195,   196,    -1,   198,    -1,   200,    -1,   202,
     203,   204,   205,   206,   207,   208,   209,   210,   211,   212,
      -1,    -1,   215,   216,   217,    -1,   219,   220,   221,   222,
     223,   224,   225,   226,   227,   228,   229,   230,   231,   232,
     233,   234,   235,   236,   237,   238,   239,   240,   241,   242,
     243,   244,   245,   246,   247,   248,   249,   250,   251,   252,
     253,   254,   255,   256,   257,   258,   259,   260,   261,   262,
     263,   264,   265,    -1,   267,   268,   269,    -1,   271,   272,
     273,   274,   275,   276,   277,   278,   279,   280,   281,   282,
     283,   284,   285,   286,   287,   288,   289,   290,   291,   292,
     293,   294,   295,   296,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,   311,   312,
     313,   314,   315,   316,   317,   318,    -1,   320,    -1,   322,
     323,   324,   325,   326,   327,   328,    11,    12,    -1,    -1,
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
      -1,    -1,    -1,   148,    -1,   150,   151,   152,   153,   154,
     155,   156,   157,   158,    -1,   160,   161,   162,   163,   164,
      -1,    -1,    -1,   168,   169,   170,   171,   172,   173,   174,
     175,    -1,   177,    -1,   179,   180,    -1,    -1,   183,   184,
     185,    -1,    -1,    -1,   189,    -1,   191,   192,   193,    -1,
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
     315,   316,   317,   318,    -1,   320,    -1,   322,   323,   324,
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
      -1,    -1,   189,    -1,   191,   192,   193,    -1,   195,   196,
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
     317,   318,    -1,   320,    -1,   322,   323,   324,   325,   326,
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
     121,   122,    -1,   124,    -1,    -1,    -1,   128,    -1,   130,
     131,   132,   133,    -1,   135,    -1,    -1,    -1,   139,   140,
     141,   142,   143,    -1,    -1,    -1,    -1,    -1,    -1,   150,
     151,   152,   153,   154,   155,   156,   157,   158,    -1,   160,
     161,   162,   163,   164,    -1,    -1,    -1,   168,   169,   170,
     171,   172,   173,   174,   175,    -1,   177,    -1,   179,   180,
      -1,    -1,   183,   184,   185,    -1,    -1,    -1,   189,    -1,
     191,   192,   193,    -1,   195,   196,    -1,   198,    -1,   200,
      -1,   202,   203,   204,   205,   206,   207,   208,   209,   210,
     211,   212,    -1,    -1,   215,   216,   217,    -1,   219,   220,
     221,   222,   223,   224,   225,   226,   227,   228,   229,   230,
     231,   232,   233,   234,   235,   236,   237,   238,   239,   240,
     241,   242,   243,   244,   245,   246,   247,   248,   249,   250,
     251,   252,   253,   254,   255,   256,   257,   258,   259,   260,
     261,   262,   263,   264,   265,    -1,   267,   268,   269,    -1,
     271,   272,   273,   274,   275,   276,   277,   278,   279,   280,
     281,   282,   283,   284,   285,   286,   287,   288,   289,   290,
     291,   292,   293,   294,   295,   296,   297,   298,   299,   300,
     301,   302,   303,   304,   305,   306,   307,   308,   309,   310,
     311,   312,   313,   314,   315,   316,   317,   318,    -1,   320,
      -1,   322,   323,   324,   325,   326,   327,   328,    11,    12,
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
      -1,   124,    -1,    -1,    -1,   128,    -1,   130,   131,   132,
     133,    -1,   135,    -1,    -1,    -1,   139,   140,   141,   142,
     143,    -1,    -1,    -1,    -1,    -1,    -1,   150,   151,   152,
     153,   154,   155,   156,   157,   158,    -1,   160,   161,   162,
     163,   164,    -1,    -1,    -1,   168,   169,   170,   171,   172,
     173,   174,   175,    -1,   177,    -1,   179,   180,    -1,    -1,
     183,   184,   185,    -1,    -1,    -1,   189,    -1,   191,   192,
     193,    -1,   195,   196,    -1,   198,    -1,   200,    -1,   202,
     203,   204,   205,   206,   207,   208,   209,   210,   211,   212,
      -1,    -1,   215,   216,   217,    -1,   219,   220,   221,   222,
     223,   224,   225,   226,   227,   228,   229,   230,   231,   232,
     233,   234,   235,   236,   237,   238,   239,   240,   241,   242,
     243,   244,   245,   246,   247,   248,   249,   250,   251,   252,
     253,   254,   255,   256,   257,   258,   259,   260,   261,   262,
     263,   264,   265,    -1,   267,   268,   269,    -1,   271,   272,
     273,   274,   275,   276,   277,   278,   279,   280,   281,   282,
     283,   284,   285,   286,   287,   288,   289,   290,   291,   292,
     293,   294,   295,   296,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,   311,   312,
     313,   314,   315,   316,   317,   318,    -1,   320,    -1,   322,
     323,   324,   325,   326,   327,   328,    11,    12,    -1,    -1,
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
     185,    -1,    -1,    -1,   189,    -1,   191,   192,   193,    -1,
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
     315,   316,   317,   318,    -1,   320,    -1,   322,   323,   324,
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
     157,   158,    -1,   160,   161,    -1,   163,   164,    -1,    -1,
      -1,   168,   169,   170,   171,   172,   173,   174,   175,    -1,
     177,    -1,    -1,    -1,    -1,    -1,   183,   184,   185,    -1,
      -1,    -1,   189,    -1,   191,   192,   193,    -1,   195,   196,
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
     317,   318,    -1,   320,    -1,   322,   323,   324,   325,   326,
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
      -1,   160,   161,    -1,   163,   164,    -1,    -1,    -1,   168,
     169,   170,   171,   172,   173,   174,   175,    -1,   177,    -1,
      -1,    -1,    -1,    -1,   183,   184,   185,    -1,    -1,    -1,
     189,    -1,   191,   192,   193,    -1,    -1,    -1,    -1,   198,
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
     121,   122,    -1,   124,    -1,    -1,    -1,   128,    -1,   130,
     131,   132,   133,    -1,   135,    -1,    -1,    -1,   139,    -1,
     141,   142,   143,    -1,    -1,    -1,    -1,    -1,    -1,   150,
     151,   152,   153,    -1,   155,   156,   157,   158,    -1,   160,
     161,    -1,   163,   164,    -1,    -1,    -1,   168,   169,   170,
     171,   172,   173,   174,   175,    -1,   177,    -1,    -1,    -1,
      -1,    -1,   183,   184,   185,    -1,    -1,    -1,   189,    -1,
     191,   192,   193,    -1,    -1,    -1,    -1,   198,    -1,   200,
      -1,   202,    -1,   204,    -1,   206,   207,   208,   209,   210,
     211,   212,    -1,    -1,   215,   216,   217,    -1,   219,   220,
     221,   222,   223,   224,   225,   226,   227,   228,   229,   230,
     231,   232,   233,   234,   235,   236,   237,   238,   239,   240,
     241,   242,   243,   244,   245,   246,   247,   248,   249,   250,
     251,   252,   253,   254,   255,   256,   257,   258,   259,   260,
     261,   262,   263,   264,   265,    -1,   267,   268,   269,    -1,
     271,   272,   273,   274,   275,   276,   277,   278,   279,   280,
     281,   282,   283,   284,   285,   286,   287,   288,   289,   290,
     291,   292,   293,   294,   295,   296,   297,   298,   299,   300,
     301,   302,   303,   304,   305,   306,   307,   308,   309,   310,
     311,   312,   313,   314,   315,   316,   317,   318,    -1,   320,
      -1,   322,   323,   324,   325,   326,   327,   328,    11,    12,
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
     113,   114,   115,   116,   117,   118,   119,    -1,   121,   122,
      -1,   124,    -1,    -1,    -1,   128,    -1,   130,   131,   132,
     133,    -1,   135,    -1,    -1,    -1,   139,    -1,   141,   142,
     143,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   151,   152,
     153,    -1,   155,   156,   157,    -1,    -1,    -1,   161,    -1,
      -1,   164,    -1,    -1,    -1,   168,   169,   170,   171,   172,
     173,   174,   175,    -1,   177,    -1,    -1,   180,    -1,    -1,
     183,   184,   185,    -1,    -1,    -1,   189,    -1,   191,   192,
     193,    -1,    -1,    -1,    -1,    -1,    -1,   200,    -1,   202,
      -1,   204,    -1,   206,   207,   208,   209,   210,   211,   212,
      -1,    -1,   215,   216,   217,    -1,   219,   220,   221,   222,
     223,   224,   225,   226,   227,   228,   229,   230,   231,   232,
     233,   234,   235,   236,   237,   238,   239,   240,   241,   242,
     243,   244,   245,   246,   247,   248,   249,   250,   251,   252,
     253,   254,   255,   256,   257,   258,   259,   260,   261,   262,
     263,   264,   265,    -1,   267,   268,   269,    -1,   271,   272,
     273,   274,   275,   276,   277,   278,   279,   280,   281,   282,
     283,   284,   285,   286,   287,   288,   289,   290,   291,   292,
     293,   294,   295,   296,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,   311,   312,
     313,   314,   315,   316,   317,   318,    -1,   320,    -1,   322,
     323,   324,   325,   326,   327,   328,    11,    12,    -1,    -1,
      -1,    16,    -1,    -1,    -1,    20,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    28,    -1,    -1,    -1,    -1,    -1,    -1,
      35,    36,    -1,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    -1,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    -1,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    -1,    97,    98,    99,   100,    -1,    -1,   103,   104,
     105,    -1,    -1,   108,   109,    -1,    -1,    -1,    -1,   114,
     115,   116,   117,   118,   119,   120,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   130,   131,   132,   133,    -1,
     135,    -1,    -1,    -1,   139,   140,   141,   142,   143,    -1,
      -1,    -1,    -1,    -1,    -1,   150,   151,   152,    -1,   154,
     155,   156,    -1,    -1,    -1,    -1,    -1,    -1,   163,   164,
      -1,    -1,    -1,   168,   169,   170,   171,   172,   173,   174,
     175,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   183,   184,
     185,    -1,    -1,    -1,   189,    -1,   191,   192,   193,    -1,
      -1,    -1,    -1,   198,    -1,   200,    -1,   202,   203,   204,
     205,   206,   207,   208,   209,   210,   211,   212,    -1,    -1,
     215,   216,    -1,    -1,   219,   220,   221,   222,   223,   224,
     225,   226,   227,   228,   229,   230,   231,   232,   233,   234,
     235,   236,   237,   238,   239,   240,   241,   242,   243,   244,
     245,   246,   247,   248,   249,   250,   251,   252,   253,   254,
     255,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,    -1,   267,   268,   269,    -1,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,    -1,    -1,    -1,    -1,   322,   323,   324,
     325,   326,   327,   328,    11,    12,    -1,    -1,    -1,    16,
      -1,    -1,    -1,    20,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    28,    -1,    -1,    -1,    -1,    -1,    -1,    35,    36,
      -1,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      -1,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    -1,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    -1,
      97,    98,    99,   100,    -1,    -1,   103,   104,   105,    -1,
      -1,   108,   109,    -1,    -1,    -1,    -1,   114,   115,   116,
     117,   118,   119,   120,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   130,   131,   132,   133,    -1,   135,    -1,
      -1,    -1,   139,   140,   141,   142,   143,    -1,    -1,    -1,
      -1,    -1,    -1,   150,   151,   152,    -1,   154,   155,   156,
      -1,    -1,    -1,    -1,    -1,    -1,   163,   164,    -1,    -1,
      -1,   168,   169,   170,   171,   172,   173,   174,   175,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   183,   184,   185,    -1,
      -1,    -1,   189,    -1,   191,   192,   193,    -1,    -1,    -1,
      -1,   198,    -1,   200,    -1,   202,   203,   204,   205,   206,
     207,   208,   209,   210,   211,   212,    -1,    -1,   215,   216,
      -1,    -1,   219,   220,   221,   222,   223,   224,   225,   226,
     227,   228,   229,   230,   231,   232,   233,   234,   235,   236,
     237,   238,   239,   240,   241,   242,   243,   244,   245,   246,
     247,   248,   249,   250,   251,   252,   253,   254,   255,   256,
     257,   258,   259,   260,   261,   262,   263,   264,   265,    -1,
     267,   268,   269,    -1,   271,   272,   273,   274,   275,   276,
     277,   278,   279,   280,   281,   282,   283,   284,   285,   286,
     287,   288,   289,   290,   291,   292,   293,   294,   295,   296,
     297,   298,   299,   300,   301,   302,   303,   304,   305,   306,
     307,   308,   309,   310,   311,   312,   313,   314,   315,   316,
     317,    16,    -1,    -1,    -1,   322,   323,   324,   325,   326,
     327,   328,    27,    28,    29,    30,    -1,    -1,    -1,    -1,
      35,    36,    -1,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    -1,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    -1,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    -1,    97,    98,    99,   100,    -1,    -1,   103,   104,
     105,    -1,    -1,   108,   109,    -1,    -1,    -1,    -1,   114,
     115,   116,   117,   118,   119,   120,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   130,   131,   132,   133,    -1,
     135,    -1,    -1,    -1,   139,   140,   141,   142,   143,    -1,
      -1,    -1,    -1,    -1,    -1,   150,   151,   152,    -1,   154,
     155,   156,    -1,    -1,    -1,   160,    -1,    -1,   163,   164,
      -1,    -1,    -1,   168,   169,   170,   171,   172,   173,   174,
     175,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   183,   184,
     185,    -1,    -1,    -1,   189,   190,   191,   192,   193,    -1,
      -1,    -1,    -1,   198,    -1,   200,    -1,   202,   203,   204,
     205,   206,   207,   208,   209,   210,   211,   212,    -1,    -1,
     215,   216,    -1,    -1,   219,   220,   221,   222,   223,   224,
     225,   226,   227,   228,   229,   230,   231,   232,   233,   234,
     235,   236,   237,   238,   239,   240,   241,   242,   243,   244,
     245,   246,   247,   248,   249,   250,   251,   252,   253,   254,
     255,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,    -1,   267,   268,   269,    -1,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,    16,    -1,    -1,    -1,   322,   323,   324,
     325,   326,   327,   328,    27,    28,    -1,    -1,    -1,    32,
      -1,    -1,    35,    36,    -1,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    -1,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    -1,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    -1,    97,    98,    99,   100,    -1,    -1,
     103,   104,   105,    -1,    -1,   108,   109,    -1,    -1,    -1,
      -1,   114,   115,   116,   117,   118,   119,   120,   121,   122,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   130,   131,   132,
     133,    -1,   135,    -1,    -1,    -1,   139,   140,   141,   142,
     143,    -1,    -1,    -1,    -1,    -1,    -1,   150,   151,   152,
      -1,   154,   155,   156,    -1,    -1,    -1,   160,    -1,    -1,
     163,   164,    -1,    -1,    -1,   168,   169,   170,   171,   172,
     173,   174,   175,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     183,   184,   185,    -1,    -1,    -1,   189,    -1,   191,   192,
     193,    -1,    -1,    -1,    -1,    -1,    -1,   200,    -1,   202,
     203,   204,   205,   206,   207,   208,   209,   210,   211,   212,
      -1,    -1,   215,   216,    -1,    -1,   219,   220,   221,   222,
     223,   224,   225,   226,   227,   228,   229,   230,   231,   232,
     233,   234,   235,   236,   237,   238,   239,   240,   241,   242,
     243,   244,   245,   246,   247,   248,   249,   250,   251,   252,
     253,   254,   255,   256,   257,   258,   259,   260,   261,   262,
     263,   264,   265,    -1,   267,   268,   269,    -1,   271,   272,
     273,   274,   275,   276,   277,   278,   279,   280,   281,   282,
     283,   284,   285,   286,   287,   288,   289,   290,   291,   292,
     293,   294,   295,   296,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,   311,   312,
     313,   314,   315,   316,   317,    16,    -1,    -1,    -1,   322,
     323,   324,   325,   326,   327,   328,    27,    28,    29,    30,
      -1,    -1,    -1,    -1,    35,    36,    -1,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    -1,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      -1,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    -1,    97,    98,    99,   100,
      -1,    -1,   103,   104,   105,    -1,    -1,   108,   109,    -1,
      -1,    -1,    -1,   114,   115,   116,   117,   118,   119,   120,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   130,
     131,   132,   133,    -1,   135,    -1,    -1,    -1,   139,   140,
     141,   142,   143,    -1,    -1,    -1,    -1,    -1,    -1,   150,
     151,   152,    -1,   154,   155,   156,    -1,    -1,    -1,   160,
      -1,    -1,   163,   164,    -1,    -1,    -1,   168,   169,   170,
     171,   172,   173,   174,   175,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   183,   184,   185,    -1,    -1,    -1,   189,   190,
     191,   192,   193,    -1,    -1,    -1,    -1,    -1,    -1,   200,
      -1,   202,   203,   204,   205,   206,   207,   208,   209,   210,
     211,   212,    -1,    -1,   215,   216,    -1,    -1,   219,   220,
     221,   222,   223,   224,   225,   226,   227,   228,   229,   230,
     231,   232,   233,   234,   235,   236,   237,   238,   239,   240,
     241,   242,   243,   244,   245,   246,   247,   248,   249,   250,
     251,   252,   253,   254,   255,   256,   257,   258,   259,   260,
     261,   262,   263,   264,   265,    -1,   267,   268,   269,    -1,
     271,   272,   273,   274,   275,   276,   277,   278,   279,   280,
     281,   282,   283,   284,   285,   286,   287,   288,   289,   290,
     291,   292,   293,   294,   295,   296,   297,   298,   299,   300,
     301,   302,   303,   304,   305,   306,   307,   308,   309,   310,
     311,   312,   313,   314,   315,   316,   317,    16,    -1,    -1,
      -1,   322,   323,   324,   325,   326,   327,   328,    27,    28,
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
      -1,   130,   131,   132,   133,    -1,   135,    -1,    -1,    -1,
     139,   140,   141,   142,   143,    -1,    -1,    -1,    -1,    -1,
      -1,   150,   151,   152,    -1,   154,   155,   156,    -1,    -1,
      -1,   160,    -1,    -1,   163,   164,    -1,    -1,    -1,   168,
     169,   170,   171,   172,   173,   174,   175,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   183,   184,   185,    -1,    -1,    -1,
     189,    -1,   191,   192,   193,    -1,    -1,    -1,    -1,    -1,
      -1,   200,    -1,   202,   203,   204,   205,   206,   207,   208,
     209,   210,   211,   212,    -1,    -1,   215,   216,    -1,    -1,
     219,   220,   221,   222,   223,   224,   225,   226,   227,   228,
     229,   230,   231,   232,   233,   234,   235,   236,   237,   238,
     239,   240,   241,   242,   243,   244,   245,   246,   247,   248,
     249,   250,   251,   252,   253,   254,   255,   256,   257,   258,
     259,   260,   261,   262,   263,   264,   265,    -1,   267,   268,
     269,    -1,   271,   272,   273,   274,   275,   276,   277,   278,
     279,   280,   281,   282,   283,   284,   285,   286,   287,   288,
     289,   290,   291,   292,   293,   294,   295,   296,   297,   298,
     299,   300,   301,   302,   303,   304,   305,   306,   307,   308,
     309,   310,   311,   312,   313,   314,   315,   316,   317,    16,
      -1,    -1,    -1,   322,   323,   324,   325,   326,   327,   328,
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
      -1,    -1,    -1,   130,   131,   132,   133,    -1,   135,    -1,
      -1,    -1,   139,   140,   141,   142,   143,    -1,    -1,    -1,
      -1,    -1,    -1,   150,   151,   152,    -1,   154,   155,   156,
      -1,    -1,    -1,   160,    -1,    -1,   163,   164,    -1,    -1,
      -1,   168,   169,   170,   171,   172,   173,   174,   175,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   183,   184,   185,    -1,
      -1,    -1,   189,    -1,   191,   192,   193,    -1,    -1,    -1,
      -1,    -1,    -1,   200,    -1,   202,   203,   204,   205,   206,
     207,   208,   209,   210,   211,   212,    -1,    -1,   215,   216,
      -1,    -1,   219,   220,   221,   222,   223,   224,   225,   226,
     227,   228,   229,   230,   231,   232,   233,   234,   235,   236,
     237,   238,   239,   240,   241,   242,   243,   244,   245,   246,
     247,   248,   249,   250,   251,   252,   253,   254,   255,   256,
     257,   258,   259,   260,   261,   262,   263,   264,   265,    -1,
     267,   268,   269,    -1,   271,   272,   273,   274,   275,   276,
     277,   278,   279,   280,   281,   282,   283,   284,   285,   286,
     287,   288,   289,   290,   291,   292,   293,   294,   295,   296,
     297,   298,   299,   300,   301,   302,   303,   304,   305,   306,
     307,   308,   309,   310,   311,   312,   313,   314,   315,   316,
     317,    16,    -1,    -1,    -1,   322,   323,   324,   325,   326,
     327,   328,    27,    28,    29,    30,    -1,    -1,    -1,    -1,
      35,    36,    -1,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    -1,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    -1,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    -1,    97,    98,    99,   100,    -1,    -1,   103,   104,
     105,    -1,    -1,   108,   109,    -1,    -1,    -1,    -1,   114,
     115,   116,   117,   118,   119,   120,   121,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   130,   131,   132,   133,    -1,
     135,    -1,    -1,    -1,   139,   140,   141,   142,   143,    -1,
      -1,    -1,    -1,    -1,    -1,   150,   151,   152,    -1,   154,
     155,   156,    -1,    -1,    -1,   160,    -1,    -1,   163,   164,
      -1,    -1,    -1,   168,   169,   170,   171,   172,   173,   174,
     175,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   183,   184,
     185,    -1,    -1,    -1,   189,    -1,   191,   192,   193,    -1,
      -1,    -1,    -1,    -1,    -1,   200,    -1,   202,   203,   204,
     205,   206,   207,   208,   209,   210,   211,   212,    -1,    -1,
     215,   216,    -1,    -1,   219,   220,   221,   222,   223,   224,
     225,   226,   227,   228,   229,   230,   231,   232,   233,   234,
     235,   236,   237,   238,   239,   240,   241,   242,   243,   244,
     245,   246,   247,   248,   249,   250,   251,   252,   253,   254,
     255,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,    -1,   267,   268,   269,    -1,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,    16,    -1,    -1,    -1,   322,   323,   324,
     325,   326,   327,   328,    27,    28,    29,    30,    -1,    -1,
      -1,    -1,    35,    36,    -1,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    -1,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    -1,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    -1,    97,    98,    99,   100,    -1,    -1,
     103,   104,   105,    -1,    -1,   108,   109,    -1,    -1,    -1,
      -1,   114,   115,   116,   117,   118,   119,   120,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   130,   131,   132,
     133,    -1,   135,    -1,    -1,    -1,   139,   140,   141,   142,
     143,    -1,    -1,    -1,    -1,    -1,    -1,   150,   151,   152,
      -1,   154,   155,   156,    -1,    -1,    -1,   160,    -1,    -1,
     163,   164,    -1,    -1,    -1,   168,   169,   170,   171,   172,
     173,   174,   175,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     183,   184,   185,    -1,    -1,    -1,   189,    -1,   191,   192,
     193,    -1,    -1,    -1,    -1,    -1,    -1,   200,    -1,   202,
     203,   204,   205,   206,   207,   208,   209,   210,   211,   212,
      -1,    -1,   215,   216,    -1,    -1,   219,   220,   221,   222,
     223,   224,   225,   226,   227,   228,   229,   230,   231,   232,
     233,   234,   235,   236,   237,   238,   239,   240,   241,   242,
     243,   244,   245,   246,   247,   248,   249,   250,   251,   252,
     253,   254,   255,   256,   257,   258,   259,   260,   261,   262,
     263,   264,   265,    -1,   267,   268,   269,    -1,   271,   272,
     273,   274,   275,   276,   277,   278,   279,   280,   281,   282,
     283,   284,   285,   286,   287,   288,   289,   290,   291,   292,
     293,   294,   295,   296,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,   311,   312,
     313,   314,   315,   316,   317,    16,    -1,    -1,    -1,   322,
     323,   324,   325,   326,   327,   328,    27,    28,    29,    30,
      -1,    -1,    -1,    -1,    35,    36,    -1,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    -1,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      -1,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    -1,    97,    98,    99,   100,
      -1,    -1,   103,   104,   105,    -1,    -1,   108,   109,    -1,
      -1,    -1,    -1,   114,   115,   116,   117,   118,   119,   120,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   130,
     131,   132,   133,    -1,   135,    -1,    -1,    -1,   139,   140,
     141,   142,   143,    -1,    -1,    -1,    -1,    -1,    -1,   150,
     151,   152,    -1,   154,   155,   156,    -1,    -1,    -1,   160,
      -1,    -1,   163,   164,    -1,    -1,    -1,   168,   169,   170,
     171,   172,   173,   174,   175,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   183,   184,   185,    -1,    -1,    -1,   189,    -1,
     191,   192,   193,    -1,    -1,    -1,    -1,    -1,    -1,   200,
      -1,   202,   203,   204,   205,   206,   207,   208,   209,   210,
     211,   212,    -1,    -1,   215,   216,    -1,    -1,   219,   220,
     221,   222,   223,   224,   225,   226,   227,   228,   229,   230,
     231,   232,   233,   234,   235,   236,   237,   238,   239,   240,
     241,   242,   243,   244,   245,   246,   247,   248,   249,   250,
     251,   252,   253,   254,   255,   256,   257,   258,   259,   260,
     261,   262,   263,   264,   265,    -1,   267,   268,   269,    -1,
     271,   272,   273,   274,   275,   276,   277,   278,   279,   280,
     281,   282,   283,   284,   285,   286,   287,   288,   289,   290,
     291,   292,   293,   294,   295,   296,   297,   298,   299,   300,
     301,   302,   303,   304,   305,   306,   307,   308,   309,   310,
     311,   312,   313,   314,   315,   316,   317,    16,    -1,    18,
      19,   322,   323,   324,   325,   326,   327,   328,    27,    28,
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
      -1,   130,   131,   132,   133,    -1,   135,    -1,    -1,    -1,
     139,   140,   141,   142,   143,    -1,    -1,    -1,    -1,    -1,
      -1,   150,   151,   152,    -1,   154,   155,   156,    -1,    -1,
      -1,    -1,    -1,    -1,   163,   164,    -1,    -1,    -1,   168,
     169,   170,   171,   172,   173,   174,   175,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   183,   184,   185,    -1,    -1,    -1,
     189,    -1,   191,   192,   193,    -1,    -1,    -1,    -1,    -1,
      -1,   200,    -1,   202,   203,   204,   205,   206,   207,   208,
     209,   210,   211,   212,    -1,    -1,   215,   216,    -1,    -1,
     219,   220,   221,   222,   223,   224,   225,   226,   227,   228,
     229,   230,   231,   232,   233,   234,   235,   236,   237,   238,
     239,   240,   241,   242,   243,   244,   245,   246,   247,   248,
     249,   250,   251,   252,   253,   254,   255,   256,   257,   258,
     259,   260,   261,   262,   263,   264,   265,    -1,   267,   268,
     269,    -1,   271,   272,   273,   274,   275,   276,   277,   278,
     279,   280,   281,   282,   283,   284,   285,   286,   287,   288,
     289,   290,   291,   292,   293,   294,   295,   296,   297,   298,
     299,   300,   301,   302,   303,   304,   305,   306,   307,   308,
     309,   310,   311,   312,   313,   314,   315,   316,   317,    16,
      -1,    -1,    -1,   322,   323,   324,   325,   326,   327,   328,
      27,    28,    -1,    -1,    -1,    -1,    -1,    -1,    35,    36,
      -1,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      -1,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    -1,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    -1,
      97,    98,    99,   100,    -1,    -1,   103,   104,   105,    -1,
      -1,   108,   109,    -1,    -1,    -1,    -1,   114,   115,   116,
     117,   118,   119,   120,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   130,   131,   132,   133,    -1,   135,    -1,
      -1,    -1,   139,   140,   141,   142,   143,    -1,    -1,    -1,
      -1,    -1,    -1,   150,   151,   152,    -1,   154,   155,   156,
      -1,    -1,    -1,    -1,    -1,    -1,   163,   164,    -1,    -1,
      -1,   168,   169,   170,   171,   172,   173,   174,   175,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   183,   184,   185,    -1,
      -1,    -1,   189,   190,   191,   192,   193,    -1,    -1,    -1,
      -1,   198,    -1,   200,    -1,   202,   203,   204,   205,   206,
     207,   208,   209,   210,   211,   212,    -1,    -1,   215,   216,
      -1,    -1,   219,   220,   221,   222,   223,   224,   225,   226,
     227,   228,   229,   230,   231,   232,   233,   234,   235,   236,
     237,   238,   239,   240,   241,   242,   243,   244,   245,   246,
     247,   248,   249,   250,   251,   252,   253,   254,   255,   256,
     257,   258,   259,   260,   261,   262,   263,   264,   265,    -1,
     267,   268,   269,    -1,   271,   272,   273,   274,   275,   276,
     277,   278,   279,   280,   281,   282,   283,   284,   285,   286,
     287,   288,   289,   290,   291,   292,   293,   294,   295,   296,
     297,   298,   299,   300,   301,   302,   303,   304,   305,   306,
     307,   308,   309,   310,   311,   312,   313,   314,   315,   316,
     317,    16,    -1,    -1,    -1,   322,   323,   324,   325,   326,
     327,   328,    27,    28,    -1,    -1,    -1,    32,    -1,    -1,
      35,    36,    -1,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    -1,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    -1,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    -1,    97,    98,    99,   100,    -1,    -1,   103,   104,
     105,    -1,    -1,   108,   109,    -1,    -1,    -1,    -1,   114,
     115,   116,   117,   118,   119,   120,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   130,   131,   132,   133,    -1,
     135,    -1,    -1,    -1,   139,   140,   141,   142,   143,    -1,
      -1,    -1,    -1,    -1,    -1,   150,   151,   152,    -1,   154,
     155,   156,    -1,    -1,    -1,    -1,    -1,    -1,   163,   164,
      -1,    -1,    -1,   168,   169,   170,   171,   172,   173,   174,
     175,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   183,   184,
     185,    -1,    -1,    -1,   189,   190,   191,   192,   193,    -1,
      -1,    -1,    -1,    -1,    -1,   200,    -1,   202,   203,   204,
     205,   206,   207,   208,   209,   210,   211,   212,    -1,    -1,
     215,   216,    -1,    -1,   219,   220,   221,   222,   223,   224,
     225,   226,   227,   228,   229,   230,   231,   232,   233,   234,
     235,   236,   237,   238,   239,   240,   241,   242,   243,   244,
     245,   246,   247,   248,   249,   250,   251,   252,   253,   254,
     255,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,    -1,   267,   268,   269,    -1,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,    16,    -1,    -1,    -1,   322,   323,   324,
     325,   326,   327,   328,    27,    28,    -1,    -1,    -1,    -1,
      -1,    -1,    35,    36,    -1,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    -1,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    -1,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    -1,    97,    98,    99,   100,    -1,    -1,
     103,   104,   105,    -1,    -1,   108,   109,    -1,    -1,    -1,
      -1,   114,   115,   116,   117,   118,   119,   120,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   130,   131,   132,
     133,    -1,   135,    -1,    -1,    -1,   139,   140,   141,   142,
     143,    -1,    -1,    -1,    -1,    -1,    -1,   150,   151,   152,
      -1,   154,   155,   156,    -1,    -1,    -1,    -1,    -1,    -1,
     163,   164,    -1,    -1,    -1,   168,   169,   170,   171,   172,
     173,   174,   175,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     183,   184,   185,    -1,    -1,    -1,   189,   190,   191,   192,
     193,    -1,    -1,    -1,    -1,   198,    -1,   200,    -1,   202,
     203,   204,   205,   206,   207,   208,   209,   210,   211,   212,
      -1,    -1,   215,   216,    -1,    -1,   219,   220,   221,   222,
     223,   224,   225,   226,   227,   228,   229,   230,   231,   232,
     233,   234,   235,   236,   237,   238,   239,   240,   241,   242,
     243,   244,   245,   246,   247,   248,   249,   250,   251,   252,
     253,   254,   255,   256,   257,   258,   259,   260,   261,   262,
     263,   264,   265,    -1,   267,   268,   269,    -1,   271,   272,
     273,   274,   275,   276,   277,   278,   279,   280,   281,   282,
     283,   284,   285,   286,   287,   288,   289,   290,   291,   292,
     293,   294,   295,   296,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,   311,   312,
     313,   314,   315,   316,   317,    16,    -1,    -1,    -1,   322,
     323,   324,   325,   326,   327,   328,    27,    28,    -1,    -1,
      -1,    -1,    -1,    -1,    35,    36,    -1,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    -1,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      -1,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    -1,    97,    98,    99,   100,
      -1,    -1,   103,   104,   105,    -1,    -1,   108,   109,    -1,
      -1,    -1,    -1,   114,   115,   116,   117,   118,   119,   120,
     121,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   130,
     131,   132,   133,    -1,   135,    -1,    -1,    -1,   139,   140,
     141,   142,   143,    -1,    -1,    -1,    -1,    -1,    -1,   150,
     151,   152,    -1,   154,   155,   156,    -1,    -1,    -1,    -1,
      -1,    -1,   163,   164,    -1,    -1,    -1,   168,   169,   170,
     171,   172,   173,   174,   175,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   183,   184,   185,    -1,    -1,    -1,   189,    -1,
     191,   192,   193,    -1,    -1,    -1,    -1,    -1,    -1,   200,
      -1,   202,   203,   204,   205,   206,   207,   208,   209,   210,
     211,   212,    -1,    -1,   215,   216,    -1,    -1,   219,   220,
     221,   222,   223,   224,   225,   226,   227,   228,   229,   230,
     231,   232,   233,   234,   235,   236,   237,   238,   239,   240,
     241,   242,   243,   244,   245,   246,   247,   248,   249,   250,
     251,   252,   253,   254,   255,   256,   257,   258,   259,   260,
     261,   262,   263,   264,   265,    -1,   267,   268,   269,    -1,
     271,   272,   273,   274,   275,   276,   277,   278,   279,   280,
     281,   282,   283,   284,   285,   286,   287,   288,   289,   290,
     291,   292,   293,   294,   295,   296,   297,   298,   299,   300,
     301,   302,   303,   304,   305,   306,   307,   308,   309,   310,
     311,   312,   313,   314,   315,   316,   317,    16,    -1,    -1,
      -1,   322,   323,   324,   325,   326,   327,   328,    27,    28,
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
      -1,   130,   131,   132,   133,    -1,   135,    -1,    -1,    -1,
     139,   140,   141,   142,   143,    -1,    -1,    -1,    -1,    -1,
      -1,   150,   151,   152,    -1,   154,   155,   156,    -1,    -1,
      -1,    -1,    -1,    -1,   163,   164,    -1,    -1,    -1,   168,
     169,   170,   171,   172,   173,   174,   175,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   183,   184,   185,    -1,    -1,    -1,
     189,    -1,   191,   192,   193,    -1,    -1,    -1,    -1,    -1,
      -1,   200,    -1,   202,   203,   204,   205,   206,   207,   208,
     209,   210,   211,   212,    -1,    -1,   215,   216,    -1,    -1,
     219,   220,   221,   222,   223,   224,   225,   226,   227,   228,
     229,   230,   231,   232,   233,   234,   235,   236,   237,   238,
     239,   240,   241,   242,   243,   244,   245,   246,   247,   248,
     249,   250,   251,   252,   253,   254,   255,   256,   257,   258,
     259,   260,   261,   262,   263,   264,   265,    -1,   267,   268,
     269,    -1,   271,   272,   273,   274,   275,   276,   277,   278,
     279,   280,   281,   282,   283,   284,   285,   286,   287,   288,
     289,   290,   291,   292,   293,   294,   295,   296,   297,   298,
     299,   300,   301,   302,   303,   304,   305,   306,   307,   308,
     309,   310,   311,   312,   313,   314,   315,   316,   317,    -1,
      -1,    -1,    -1,   322,   323,   324,   325,   326,   327,   328,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      -1,    32,    -1,    -1,    35,    36,    -1,    38,    39,    40,
      41,    42,    43,    44,    45,    -1,    47,    48,    49,    -1,
      51,    52,    53,    54,    -1,    56,    -1,    58,    59,    60,
      61,    62,    63,    -1,    -1,    66,    67,    68,    69,    70,
      -1,    72,    73,    74,    -1,    -1,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    -1,    97,    98,    99,   100,
      -1,    -1,   103,   104,   105,    -1,    -1,   108,   109,    -1,
      -1,    -1,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,    -1,    -1,    -1,    -1,    -1,   128,    -1,   130,
     131,   132,   133,    -1,   135,    -1,    -1,    -1,   139,   140,
     141,   142,   143,    -1,    -1,    -1,    -1,    -1,    -1,   150,
     151,   152,   153,   154,   155,    -1,   157,   158,    -1,   160,
     161,    -1,   163,   164,    -1,    -1,    -1,   168,   169,   170,
     171,   172,   173,   174,   175,    -1,   177,    -1,    -1,    -1,
      -1,    -1,   183,   184,   185,    -1,    -1,    -1,   189,    -1,
     191,   192,   193,    -1,    -1,    -1,    -1,    -1,    -1,   200,
      -1,   202,   203,   204,   205,   206,   207,   208,   209,   210,
     211,   212,    -1,    -1,    -1,   216,   217,    -1,   219,   220,
     221,   222,   223,   224,   225,   226,   227,   228,   229,   230,
     231,   232,    -1,   234,   235,   236,   237,   238,   239,   240,
     241,   242,   243,   244,   245,   246,   247,   248,   249,   250,
      -1,   252,   253,   254,   255,   256,   257,   258,   259,   260,
     261,   262,   263,   264,   265,    -1,   267,   268,   269,    -1,
     271,   272,   273,   274,   275,   276,   277,   278,   279,   280,
     281,   282,   283,   284,   285,   286,   287,   288,   289,   290,
     291,   292,   293,   294,   295,   296,   297,   298,   299,   300,
     301,   302,   303,   304,   305,   306,   307,   308,   309,   310,
     311,   312,   313,   314,   315,   316,   317,   318,    -1,   320,
      27,    28,   323,   324,    -1,    -1,    -1,    -1,    35,    36,
      -1,    38,    39,    40,    -1,    42,    43,    44,    45,    -1,
      -1,    48,    49,    -1,    51,    52,    53,    54,    -1,    56,
      -1,    58,    59,    60,    61,    -1,    63,    -1,    -1,    66,
      67,    68,    69,    70,    -1,    72,    -1,    74,    -1,    -1,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    -1,
      97,    98,    99,    -1,    -1,    -1,   103,   104,   105,    -1,
      -1,   108,   109,    -1,    -1,    -1,    -1,   114,   115,   116,
     117,   118,   119,   120,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   130,   131,   132,   133,    -1,   135,    -1,
      -1,    -1,   139,   140,   141,   142,   143,    -1,    -1,    -1,
      -1,    -1,    -1,   150,   151,   152,    -1,   154,   155,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   163,   164,    -1,    -1,
      -1,   168,   169,   170,   171,   172,   173,   174,   175,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   183,   184,   185,    -1,
      -1,    -1,   189,    -1,   191,   192,   193,    -1,    -1,    -1,
      -1,    -1,    -1,   200,    -1,   202,   203,   204,   205,   206,
     207,   208,   209,   210,   211,   212,    -1,    -1,    -1,   216,
      -1,    -1,   219,   220,   221,   222,   223,   224,   225,   226,
     227,   228,   229,   230,   231,   232,    -1,   234,   235,   236,
     237,   238,   239,   240,   241,   242,   243,   244,   245,   246,
     247,   248,   249,   250,    -1,   252,   253,   254,   255,   256,
     257,   258,   259,   260,   261,   262,   263,   264,   265,    -1,
     267,   268,   269,    -1,   271,   272,   273,   274,   275,   276,
     277,   278,   279,   280,   281,   282,   283,   284,   285,   286,
     287,   288,   289,   290,   291,   292,   293,   294,   295,   296,
     297,   298,   299,   300,   301,   302,   303,   304,   305,   306,
     307,   308,   309,   310,   311,   312,   313,   314,   315,   316,
     317,    -1,    -1,    -1,    -1,    -1,   323,   324
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
     360,   368,   397,   368,   397,   388,   408,   344,   408,   422,
     422,   121,   171,   173,   173,   422,   402,   422,   292,   292,
     437,   121,   444,   479,   480,   483,   483,   483,   483,   482,
     483,   483,   483,    73,   484,   488,   488,   487,   489,   489,
     489,   489,   490,   490,   491,   491,   239,    97,    97,    97,
     506,   399,   508,   508,   421,   521,   160,   525,   527,   583,
     666,   188,   148,   190,   422,   531,   190,   153,   190,   531,
     110,   190,   190,   110,   110,   388,    32,   667,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,   370,   371,
     372,    97,   141,   146,   376,   377,   378,   666,   164,   164,
     370,   665,   131,   136,    57,    59,   103,   259,   272,   274,
     285,   286,   287,   289,   291,   628,   629,   630,   631,   632,
     633,   640,   646,   647,   254,    97,   307,   666,   146,   422,
     121,   666,   666,   136,   187,   187,   190,   190,   187,   110,
     190,   110,   190,   110,    38,    97,    99,   151,   439,   440,
     562,   666,    97,   110,   190,   399,   190,   666,    99,    47,
     666,   665,    99,   146,   562,   666,   422,   443,   187,   190,
     190,   190,   190,   110,   191,   151,   562,   187,   190,   190,
     157,   187,   400,   400,   187,   110,   190,   110,   190,   146,
     562,   422,   191,   422,   422,   666,   422,   421,   421,   421,
     666,   540,   541,   134,   201,   187,   187,   187,   135,   194,
      97,   226,   227,   241,    97,   226,   227,   241,   241,   241,
      97,    97,   245,   233,   322,   110,   243,   146,   194,   190,
     421,   187,   517,   600,   419,   241,   367,    35,    35,   194,
     344,   194,   116,   405,   402,   666,   173,   422,   454,   455,
     121,   422,   450,   451,   666,    58,   222,   262,   428,   429,
     483,   157,   160,   267,   504,   583,   601,   604,   605,   606,
     607,   608,   612,   614,   616,   617,    49,   156,   160,   215,
     325,   326,   327,   328,   381,   563,   565,   567,   569,   584,
     585,   586,   587,   664,   666,   563,   495,   568,   666,   495,
     187,   188,   190,   421,   110,   190,   190,   523,   152,   168,
     152,   168,   141,   405,   388,   371,   136,   160,   565,   378,
     422,   562,   665,   665,   132,   133,   665,   285,   286,   287,
     291,   666,   271,   282,   271,   282,    32,   294,    99,   116,
     160,   634,   637,   628,    41,    46,    55,    62,    64,    73,
     100,   233,   325,   326,   327,   392,   569,   664,   235,   307,
     316,   422,   666,    97,   307,   665,   157,   581,   582,   666,
     581,   582,   121,   438,   131,   563,   121,   422,   151,   440,
     151,    38,   151,   439,   440,   562,   563,   390,    97,   187,
     202,   136,   362,   665,   164,   136,    99,   362,   422,   146,
     440,   157,   121,   422,   422,   151,   103,   464,   465,   466,
     468,   469,   103,   472,   473,   474,   475,   400,   187,   187,
     157,   581,   581,   422,   146,   194,   422,   125,   125,   190,
     190,   190,    37,   541,   134,   201,     9,    10,   106,   127,
     129,   157,   199,   536,   538,   549,   550,   553,   157,    32,
     240,   242,   422,   422,   422,   240,   242,   422,   422,   422,
     422,   422,   422,   422,   422,   422,   519,   121,   422,   422,
      50,    75,    76,   252,   402,   423,   434,   253,   597,   598,
     157,   214,   403,   422,   194,   116,   397,   397,   397,   454,
      98,   109,   119,   131,   456,   457,   458,   459,   110,   666,
     109,   452,   110,   151,   562,   262,    56,    61,    80,   121,
     439,   445,   446,   447,   429,   421,   601,   608,   157,   293,
     485,   653,    99,   172,   260,   261,   263,   270,   280,   292,
     602,   603,   622,   623,   624,   625,   648,   651,   266,   268,
     609,   627,   275,   613,   649,   255,   256,   278,   618,   619,
     160,   160,   190,   565,   160,   160,   160,   160,   160,    49,
     585,   586,   148,   179,   198,   564,   148,   190,   148,   422,
     141,   405,   583,   377,   294,    32,    99,   116,   160,   641,
      32,   634,   564,   564,   506,   295,   314,   562,   392,   235,
     194,   399,   190,   190,   148,   190,   190,   438,   151,   439,
     666,   422,   151,   422,   131,   422,   151,   440,   151,   563,
     402,   422,   665,   110,   362,   422,   146,   399,   462,   422,
     422,   116,   465,   466,   103,   426,   116,   466,   469,   121,
     476,   563,   103,   116,   473,   103,   116,   475,   187,   399,
     190,   190,   422,   422,   422,   202,   472,   136,   199,   538,
       7,   400,   666,   199,   549,   400,   194,   241,   241,   241,
     241,    99,   245,   245,   595,   426,   160,   160,   160,   426,
     600,   598,   517,   665,   132,   133,   458,   459,   459,   455,
     146,   562,   665,   451,   422,   151,   121,   121,   422,   666,
     447,    80,   187,   190,   601,   615,   258,   222,   262,   276,
     283,   652,    99,   264,   265,   650,   258,   605,   652,   487,
     622,   606,   151,   290,   610,   611,   650,   294,   621,    81,
     620,   190,   198,   563,   566,   190,   190,   190,   190,   190,
     190,   190,    32,   140,   205,   643,   644,   645,    32,   642,
     643,   279,   638,   110,   635,   173,   666,   265,   506,   187,
     422,   151,   422,   151,   439,   422,   151,   422,   130,   130,
      99,   665,   422,   187,   426,   426,   422,   402,   422,   426,
     426,   666,   214,   426,   121,   476,   121,   426,   121,   476,
     426,   187,   116,   541,   666,   199,   187,   541,   666,   187,
     422,   422,   422,   422,   324,   422,   422,   422,   421,   421,
     421,   157,   599,   459,   422,   146,   422,   666,   666,   446,
     422,   187,   489,    54,   133,   487,   487,   277,   284,   294,
     626,   626,   607,   157,   288,    97,   190,   110,   190,   641,
     641,   645,   110,   190,    32,   639,   650,   636,   637,   190,
     394,   395,   506,   121,   233,   315,   295,   173,   422,   422,
     151,   422,   402,   422,   362,   422,   402,    97,   563,   402,
     422,   666,   426,   666,   422,   666,   426,   402,   121,    96,
     186,   542,   541,   666,   201,   541,   422,   190,   190,   190,
     421,   452,   422,    56,   487,   487,   203,   421,   563,   563,
      97,    32,   273,   110,   110,   459,   562,   666,   121,   233,
     666,   394,   422,   476,    97,   426,    97,   666,     5,   137,
     545,   546,   548,   550,    31,   138,   543,   544,   547,   550,
     201,   541,   201,   202,   472,   187,   452,   121,   487,   187,
     563,   637,   395,   459,   313,   666,   121,   233,   426,   476,
     402,   422,   476,   426,    96,   137,   548,   186,   138,   547,
     201,   116,   666,   422,   313,   666,   121,   402,   422,   426,
     426,   121,   301,   313,   666,   666,   314,   422,   314,   426,
     506,   506,   203,   295,   666,   233,   121,   666,   314,   506
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
     364,   364,   364,   365,   366,   366,   367,   367,   367,   367,
     368,   368,   368,   368,   368,   368,   368,   368,   369,   369,
     370,   370,   371,   372,   372,   372,   372,   372,   372,   372,
     372,   372,   372,   373,   374,   375,   376,   376,   377,   377,
     377,   378,   379,   379,   379,   380,   380,   380,   380,   381,
     381,   382,   382,   382,   382,   383,   383,   384,   384,   385,
     385,   386,   386,   387,   387,   388,   388,   388,   388,   389,
     389,   390,   390,   391,   391,   391,   391,   392,   392,   392,
     392,   393,   393,   394,   394,   395,   395,   395,   395,   396,
     396,   396,   396,   397,   398,   398,   398,   399,   399,   399,
     400,   400,   401,   401,   401,   402,   402,   402,   402,   402,
     402,   402,   402,   402,   402,   402,   402,   402,   403,   404,
     405,   406,   407,   407,   407,   408,   408,   408,   408,   409,
     410,   411,   412,   413,   413,   414,   415,   416,   417,   418,
     418,   419,   420,   421,   421,   421,   422,   422,   422,   422,
     422,   422,   423,   423,   423,   423,   423,   423,   423,   423,
     423,   423,   423,   423,   424,   425,   426,   426,   427,   427,
     428,   428,   428,   429,   429,   430,   430,   431,   432,   432,
     432,   433,   433,   433,   433,   433,   434,   434,   435,   435,
     435,   435,   436,   436,   437,   437,   437,   438,   438,   438,
     438,   438,   438,   438,   438,   438,   438,   438,   438,   439,
     440,   441,   442,   442,   443,   443,   443,   443,   444,   444,
     445,   445,   445,   446,   446,   446,   447,   447,   447,   448,
     449,   450,   450,   451,   451,   451,   451,   451,   451,   452,
     453,   453,   454,   454,   455,   455,   456,   456,   456,   456,
     456,   456,   456,   457,   457,   458,   458,   459,   460,   460,
     461,   461,   462,   462,   463,   464,   464,   465,   466,   466,
     467,   468,   468,   469,   470,   470,   471,   471,   472,   472,
     473,   473,   474,   474,   475,   475,   476,   476,   477,   478,
     478,   479,   479,   480,   480,   481,   481,   481,   481,   481,
     482,   481,   481,   481,   481,   483,   483,   484,   484,   485,
     485,   486,   486,   487,   487,   487,   488,   488,   488,   488,
     488,   489,   489,   489,   490,   490,   490,   491,   491,   492,
     492,   493,   493,   494,   494,   495,   495,   496,   496,   497,
     497,   497,   497,   498,   498,   498,   499,   499,   500,   500,
     500,   500,   500,   500,   501,   501,   501,   502,   502,   502,
     502,   503,   504,   504,   505,   505,   505,   506,   506,   506,
     506,   507,   508,   508,   508,   509,   509,   510,   510,   510,
     510,   511,   511,   512,   512,   512,   512,   512,   512,   512,
     513,   513,   514,   514,   515,   515,   515,   515,   515,   516,
     516,   517,   517,   518,   518,   518,   518,   519,   519,   519,
     519,   519,   519,   519,   519,   519,   519,   519,   520,   520,
     521,   522,   522,   522,   522,   522,   522,   522,   522,   522,
     522,   522,   522,   522,   522,   523,   523,   524,   524,   524,
     525,   526,   526,   527,   528,   529,   530,   530,   531,   531,
     531,   531,   532,   532,   533,   534,   534,   535,   535,   536,
     536,   536,   537,   537,   537,   537,   537,   537,   538,   538,
     539,   539,   540,   541,   541,   542,   542,   543,   543,   544,
     544,   544,   544,   545,   545,   546,   546,   546,   546,   547,
     547,   548,   548,   549,   549,   549,   549,   550,   550,   550,
     550,   551,   551,   552,   552,   553,   554,   554,   554,   554,
     554,   554,   554,   555,   556,   556,   556,   557,   557,   557,
     558,   559,   560,   560,   561,   561,   562,   563,   563,   563,
     563,   564,   564,   564,   565,   565,   565,   565,   565,   565,
     565,   566,   566,   567,   568,   569,   569,   569,   569,   569,
     569,   569,   569,   569,   569,   570,   571,   572,   572,   572,
     573,   574,   575,   576,   576,   576,   577,   577,   577,   577,
     577,   578,   579,   579,   579,   579,   579,   579,   579,   580,
     581,   582,   583,   584,   584,   584,   584,   585,   586,   586,
     587,   588,   588,   588,   589,   589,   589,   589,   589,   589,
     589,   589,   589,   589,   590,   590,   591,   591,   592,   593,
     594,   594,   595,   596,   597,   597,   598,   599,   600,   600,
     601,   602,   602,   603,   603,   604,   604,   605,   605,   606,
     606,   607,   607,   608,   609,   609,   610,   610,   611,   612,
     612,   612,   613,   613,   614,   615,   615,   616,   617,   617,
     618,   618,   619,   619,   619,   620,   620,   621,   621,   622,
     622,   622,   622,   622,   623,   624,   625,   626,   626,   626,
     627,   627,   628,   628,   628,   628,   628,   628,   628,   628,
     629,   629,   629,   629,   630,   630,   631,   632,   632,   633,
     633,   633,   634,   634,   635,   635,   636,   636,   637,   638,
     638,   639,   639,   640,   640,   640,   641,   641,   642,   642,
     643,   643,   644,   644,   645,   645,   646,   647,   647,   648,
     648,   648,   649,   650,   650,   650,   650,   651,   651,   652,
     652,   653,   654,   654,   655,   655,   656,   656,   657,   658,
     658,   658,   658,   659,   659,   659,   660,   661,   662,   663,
     664,   664,   664,   665,   666,   666,   666,   666,   666,   666,
     666,   666,   666,   666,   666,   666,   666,   666,   666,   666,
     666,   666,   666,   666,   666,   666,   666,   667,   667,   667,
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
     667,   667,   667,   668
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
       1,     1,     1,     1,     1,     1,     1,     1,     3,     3,
       3,     2,     3,     2,     3,     2,     3,     4,     5,     5,
       2,     4,     5,     3,     3,     2,     2,     8,     3,     1,
       2,     3,     1,     1,     3,     3,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     2,     2,     1,     1,     2,     2,
       1,     1,     2,     4,     3,     5,     4,     3,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     2,     3,     3,
       3,     2,     1,     1,     1,     4,     3,     3,     5,     4,
       6,     4,     6,     5,     7,     4,     5,     5,     6,     3,
       3,     2,     1,     3,     4,     5,     3,     6,     4,     5,
       1,     2,     3,     1,     2,     1,     6,     3,     3,     2,
       3,     1,     3,     4,     5,     6,     5,     1,     2,     2,
       3,     4,     1,     3,     1,     2,     1,     1,     1,     2,
       2,     2,     3,     1,     1,     2,     2,     2,     5,     5,
       1,     4,     3,     4,     8,     1,     2,     3,     2,     3,
       8,     1,     2,     3,     8,    10,     8,    10,     1,     2,
       4,     7,     1,     2,     4,     7,     1,     3,     8,     1,
       3,     1,     3,     1,     2,     1,     3,     3,     3,     3,
       0,     4,     3,     3,     3,     1,     5,     1,     3,     0,
       1,     1,     3,     1,     3,     3,     1,     3,     3,     3,
       3,     1,     3,     3,     1,     3,     3,     1,     4,     1,
       4,     1,     4,     1,     4,     1,     2,     1,     2,     1,
       1,     2,     2,     1,     1,     1,     1,     3,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     4,     5,     5,
       6,     4,     1,     2,     3,     2,     2,     1,     2,     2,
       1,     1,     1,     3,     3,     1,     1,     1,     2,     1,
       2,     2,     1,     2,     2,     2,     2,     2,     2,     2,
       1,     2,     2,     1,     2,     2,     2,     2,     2,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     2,     3,
       4,     3,     3,     4,     5,     3,     3,     3,     1,     2,
       3,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       2,     2,     3,     1,     4,     4,     3,     4,     1,     3,
       1,     3,     1,     1,     3,     3,     4,     1,     1,     1,
       1,     1,     4,     8,     5,     9,     9,    10,     1,     2,
       1,     2,     6,     0,     1,     3,     3,     0,     1,     1,
       1,     2,     2,     0,     1,     1,     1,     2,     2,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       3,     3,     2,     3,     3,     2,     1,     1,     1,     1,
       1,     1,     1,     4,     3,     3,     7,     3,     3,     7,
       4,     4,     3,     7,     3,     7,     2,     1,     2,     3,
       2,     1,     1,     1,     1,     1,     3,     3,     1,     1,
       1,     1,     3,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     3,     1,     3,     4,     4,
       3,     3,     3,     3,     4,     4,     3,     4,     6,     4,
       6,     4,     3,     4,     6,     6,     4,     6,     6,     4,
       1,     2,     1,     1,     1,     2,     2,     4,     5,     6,
       3,     3,     3,     3,     5,     7,     7,     5,     5,     5,
       7,     7,     5,     5,     3,     3,     5,     7,     5,     7,
       1,     4,     3,     5,     1,     2,     3,     3,     1,     3,
       2,     0,     1,     1,     2,     1,     3,     1,     3,     1,
       4,     1,     2,     3,     0,     1,     0,     1,     4,     2,
       3,     1,     0,     1,     4,     0,     1,     2,     1,     3,
       0,     1,     2,     2,     1,     0,     1,     0,     1,     1,
       1,     1,     1,     1,     1,     3,     3,     1,     1,     1,
       2,     3,     1,     1,     1,     1,     1,     1,     1,     1,
       2,     2,     1,     1,     2,     2,     3,     1,     2,     2,
       5,     2,     1,     1,     0,     2,     1,     3,     4,     0,
       2,     0,     2,     4,     4,     3,     2,     3,     1,     3,
       0,     1,     1,     2,     2,     2,     2,     1,     2,     2,
       2,     2,     3,     2,     3,     3,     4,     2,     2,     1,
       1,     3,     2,     3,     2,     3,     2,     3,     3,     3,
       3,     5,     5,     5,     5,     8,     5,     3,     5,     7,
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
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1
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
       0,   941,   941,   942,   946,   950,   956,   960,   964,   970,
     976,   984,   990,   999,  1004,  1010,  1016,  1022,  1030,  1035,
    1045,  1055,  1066,  1078,  1083,  1089,  1095,  1104,  1112,  1119,
    1125,  1136,  1137,  1138,  1139,  1140,  1143,  1144,  1145,  1146,
    1147,  1148,  1149,  1150,  1153,  1158,  1164,  1170,  1176,  1181,
    1187,  1192,  1198,  1203,  1209,  1214,  1219,  1224,  1230,  1232,
    1234,  1242,  1247,  1255,  1263,  1272,  1279,  1287,  1293,  1299,
    1305,  1312,  1320,  1331,  1338,  1345,  1353,  1360,  1366,  1376,
    1386,  1387,  1388,  1389,  1390,  1391,  1392,  1393,  1396,  1402,
    1409,  1416,  1424,  1432,  1433,  1434,  1435,  1436,  1437,  1438,
    1439,  1440,  1441,  1444,  1458,  1464,  1470,  1477,  1483,  1490,
    1495,  1501,  1507,  1519,  1531,  1544,  1549,  1557,  1565,  1574,
    1579,  1586,  1591,  1598,  1603,  1611,  1616,  1623,  1629,  1638,
    1639,  1642,  1656,  1669,  1684,  1697,  1702,  1707,  1712,  1718,
    1725,  1733,  1738,  1746,  1754,  1762,  1770,  1780,  1784,  1790,
    1794,  1801,  1810,  1821,  1827,  1834,  1839,  1846,  1853,  1862,
    1872,  1882,  1892,  1907,  1923,  1927,  1932,  1937,  1941,  1946,
    1951,  1956,  1964,  1971,  1978,  1991,  1992,  1993,  1994,  1995,
    1996,  1997,  1998,  1999,  2000,  2001,  2002,  2003,  2006,  2012,
    2038,  2044,  2050,  2057,  2064,  2073,  2082,  2091,  2100,  2111,
    2117,  2123,  2129,  2141,  2146,  2152,  2164,  2177,  2197,  2203,
    2210,  2219,  2226,  2232,  2237,  2249,  2261,  2262,  2263,  2264,
    2265,  2266,  2269,  2270,  2271,  2272,  2273,  2274,  2275,  2276,
    2277,  2278,  2279,  2280,  2283,  2295,  2301,  2302,  2307,  2312,
    2318,  2323,  2328,  2334,  2343,  2353,  2363,  2374,  2380,  2381,
    2382,  2385,  2386,  2387,  2388,  2389,  2392,  2399,  2407,  2411,
    2419,  2427,  2435,  2436,  2441,  2448,  2455,  2466,  2476,  2486,
    2496,  2506,  2516,  2526,  2536,  2546,  2555,  2565,  2575,  2587,
    2593,  2599,  2605,  2611,  2619,  2627,  2635,  2643,  2653,  2659,
    2668,  2669,  2673,  2680,  2684,  2689,  2692,  2696,  2700,  2706,
    2712,  2718,  2724,  2732,  2736,  2744,  2752,  2760,  2768,  2778,
    2784,  2790,  2798,  2804,  2812,  2816,  2824,  2830,  2836,  2842,
    2851,  2860,  2869,  2880,  2884,  2890,  2896,  2904,  2910,  2919,
    2930,  2936,  2944,  2948,  2957,  2963,  2969,  2977,  2983,  2989,
    2997,  3003,  3009,  3017,  3023,  3030,  3040,  3047,  3057,  3063,
    3071,  3077,  3086,  3092,  3100,  3106,  3115,  3121,  3129,  3135,
    3139,  3145,  3150,  3156,  3160,  3171,  3175,  3184,  3190,  3199,
    3209,  3208,  3221,  3230,  3239,  3250,  3254,  3265,  3269,  3275,
    3278,  3284,  3288,  3294,  3298,  3302,  3308,  3312,  3318,  3324,
    3330,  3338,  3342,  3346,  3352,  3356,  3362,  3370,  3374,  3382,
    3386,  3394,  3398,  3406,  3410,  3418,  3422,  3428,  3432,  3438,
    3442,  3446,  3450,  3458,  3459,  3460,  3463,  3468,  3474,  3478,
    3482,  3486,  3490,  3494,  3500,  3504,  3508,  3514,  3518,  3522,
    3526,  3535,  3543,  3549,  3557,  3561,  3565,  3571,  3575,  3583,
    3591,  3629,  3635,  3645,  3649,  3655,  3656,  3659,  3665,  3673,
    3679,  3689,  3695,  3703,  3707,  3711,  3715,  3719,  3725,  3731,
    3737,  3741,  3747,  3751,  3760,  3764,  3768,  3774,  3778,  3786,
    3787,  3790,  3794,  3800,  3804,  3808,  3812,  3818,  3822,  3826,
    3830,  3834,  3838,  3844,  3850,  3856,  3860,  3864,  3870,  3876,
    3884,  3890,  3891,  3892,  3893,  3894,  3895,  3896,  3897,  3898,
    3899,  3900,  3901,  3902,  3903,  3906,  3910,  3916,  3923,  3930,
    3939,  3945,  3949,  3955,  3962,  3968,  3974,  3978,  3996,  4002,
    4008,  4014,  4022,  4026,  4032,  4038,  4047,  4059,  4060,  4063,
    4064,  4065,  4068,  4076,  4095,  4103,  4122,  4143,  4166,  4172,
    4181,  4187,  4196,  4203,  4204,  4206,  4211,  4218,  4221,  4227,
    4233,  4239,  4246,  4255,  4258,  4264,  4270,  4276,  4283,  4292,
    4296,  4302,  4306,  4312,  4316,  4320,  4325,  4332,  4336,  4340,
    4344,  4350,  4355,  4361,  4367,  4374,  4380,  4385,  4390,  4395,
    4400,  4405,  4410,  4413,  4419,  4423,  4427,  4433,  4437,  4441,
    4447,  4453,  4459,  4463,  4469,  4473,  4479,  4485,  4489,  4493,
    4497,  4504,  4510,  4516,  4524,  4563,  4567,  4571,  4575,  4579,
    4583,  4589,  4595,  4603,  4609,  4615,  4619,  4623,  4627,  4631,
    4635,  4639,  4643,  4647,  4651,  4657,  4663,  4669,  4673,  4677,
    4683,  4688,  4694,  4700,  4704,  4709,  4715,  4719,  4725,  4731,
    4735,  4743,  4749,  4753,  4759,  4768,  4777,  4783,  4789,  4797,
    4803,  4809,  4815,  4821,  4825,  4829,  4835,  4843,  4849,  4853,
    4861,  4867,  4873,  4879,  4887,  4891,  4897,  4903,  4907,  4913,
    4917,  4923,  4929,  4936,  4944,  4949,  4955,  4961,  4969,  4975,
    4982,  4988,  4997,  5003,  5009,  5015,  5024,  5031,  5037,  5043,
    5052,  5059,  5063,  5069,  5074,  5081,  5085,  5091,  5095,  5101,
    5105,  5111,  5115,  5123,  5134,  5137,  5143,  5146,  5152,  5158,
    5166,  5170,  5176,  5179,  5185,  5195,  5198,  5204,  5214,  5220,
    5228,  5231,  5237,  5241,  5245,  5251,  5254,  5260,  5263,  5269,
    5273,  5277,  5281,  5285,  5291,  5297,  5307,  5317,  5321,  5325,
    5331,  5337,  5345,  5349,  5353,  5357,  5361,  5365,  5369,  5373,
    5379,  5383,  5387,  5391,  5397,  5403,  5411,  5419,  5423,  5429,
    5439,  5450,  5456,  5460,  5466,  5469,  5475,  5480,  5487,  5495,
    5498,  5504,  5508,  5514,  5523,  5530,  5538,  5542,  5548,  5554,
    5562,  5565,  5571,  5576,  5586,  5594,  5604,  5610,  5614,  5620,
    5624,  5628,  5634,  5640,  5644,  5648,  5652,  5658,  5666,  5676,
    5680,  5686,  5692,  5696,  5702,  5706,  5712,  5716,  5722,  5729,
    5739,  5745,  5752,  5763,  5769,  5779,  5785,  5791,  5809,  5834,
    5854,  5858,  5862,  5868,  5871,  5872,  5873,  5874,  5875,  5876,
    5877,  5878,  5879,  5880,  5881,  5882,  5883,  5884,  5885,  5886,
    5887,  5888,  5889,  5890,  5891,  5892,  5893,  5896,  5897,  5898,
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
    6049,  6050,  6051,  6052,  6053,  6054,  6055,  6056,  6057,  6058,
    6059,  6060,  6061,  6062,  6063,  6064,  6065,  6066,  6067,  6068,
    6069,  6070,  6071,  6072,  6073,  6074,  6075,  6076,  6077,  6078,
    6079,  6080,  6081,  6082,  6083,  6084,  6085,  6086,  6087,  6088,
    6089,  6090,  6091,  6092,  6093,  6094,  6095,  6096,  6097,  6098,
    6099,  6100,  6101,  6104
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
