import module namespace m = 'xqueryzorba.org/test/xqddf/ic' at 'ic.xqlib';
import module namespace ddl = "http://zorba.io/modules/store/static/collections/ddl";
import module namespace dml = "http://zorba.io/modules/store/static/collections/dml";
import module namespace icdml = "http://zorba.io/modules/store/static/integrity-constraints/dml";

ddl:create($m:empc);

dml:insert($m:empc, <emp><salary>2000</salary></emp>);

icdml:check-integrity-constraint(xs:QName("m:ic_simple"))
