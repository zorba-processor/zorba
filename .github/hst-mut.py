def sub(p,a,b):
    s=open(p).read()
    assert a in s,(p,a[:50])
    open(p,'w').write(s.replace(a,b,1))
sub('src/compiler/rewriter/rules/index_join_rule.cpp','''  if (expr_tools::calls_unproven_function(theVarDefExprs[0]))
    return false;
''','')
sub('src/compiler/rewriter/rules/index_matching_rule.cpp','!expr_tools::calls_unproven_function(theQueryExpr) && matchIndex()','matchIndex()')
sub('src/compiler/rewriter/rules/flwor_rules.cpp','''  if (! flwor->has_sequential_clauses() &&
      ! expr_tools::calls_unproven_function(flwor))''','''  if (! flwor->has_sequential_clauses())''')
