import module namespace shop = "http://example.org/shop" at "lib/shop.xqm";
import module namespace ddl = "http://zorba.io/modules/store/static/collections/ddl";
import module namespace dml = "http://zorba.io/modules/store/static/collections/dml";
import module namespace icddl = "http://zorba.io/modules/store/static/integrity-constraints/ddl";

declare namespace err = "http://www.w3.org/2005/xqt-errors";

ddl:create($shop:products);
icddl:activate($shop:unique-sku);
dml:insert($shop:products, <product sku="A1" category="tea">Green tea</product>);

try {
  {
    dml:insert($shop:products, <product sku="A1" category="tea">Duplicate</product>);
    "inserted"
  }
}
catch * {
  concat("rejected: ", local-name-from-QName($err:code))
},
count(dml:collection($shop:products))
