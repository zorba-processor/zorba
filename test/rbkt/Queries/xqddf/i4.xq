import module namespace xqddf-test = "http://www.zorba-xquery.com/modules/xqddf/test" at "xqddf_prolog.xqlib";
import module namespace xqddf = "http://www.zorba-xquery.com/modules/xqddf";


xqddf:create-collection($xqddf-test:white-collection);

xqddf:create-index($xqddf-test:index2);

for $i in fn:doc("auction.xml")//item
return 
    xqddf:insert-nodes($xqddf-test:white-collection, (copy $copyi := $i modify () return $copyi));


xqddf:refresh-index($xqddf-test:index2);
(:xqddf:collection($xqddf-test:white-collection);:)

xqddf:probe-index-range($xqddf-test:index2, 1, 3, fn:true(), fn:true(), fn:true(), fn:false())[1]; 
