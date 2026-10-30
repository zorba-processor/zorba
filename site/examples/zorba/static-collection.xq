import module namespace shop = "http://example.org/shop" at "lib/shop.xqm";
import module namespace ddl = "http://zorba.io/modules/store/static/collections/ddl";
import module namespace dml = "http://zorba.io/modules/store/static/collections/dml";
import module namespace iddl = "http://zorba.io/modules/store/static/indexes/ddl";
import module namespace idml = "http://zorba.io/modules/store/static/indexes/dml";

ddl:create($shop:products);
iddl:create($shop:by-category);

dml:insert($shop:products, (
  <product sku="A1" category="tea">Green tea</product>,
  <product sku="B7" category="coffee">Espresso beans</product>,
  <product sku="C3" category="tea">Rooibos</product>));

<tea>{ idml:probe-index-point-value($shop:by-category, "tea")/string() }</tea>
