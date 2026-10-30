module namespace shop = "http://example.org/shop";

import module namespace dml = "http://zorba.io/modules/store/static/collections/dml";

declare namespace an = "http://zorba.io/annotations";

(: A statically declared collection of <product> elements :)
declare collection shop:products as element(product)*;
declare variable $shop:products := xs:QName("shop:products");

(: A value index on the product category, kept up to date automatically :)
declare %an:value-equality %an:nonunique %an:automatic index shop:by-category
  on nodes dml:collection(xs:QName("shop:products"))
  by string(@category) as xs:string;
declare variable $shop:by-category := xs:QName("shop:by-category");

(: No two products may share a sku :)
declare integrity constraint shop:unique-sku
  on collection shop:products node $p check unique key $p/@sku;
declare variable $shop:unique-sku := xs:QName("shop:unique-sku");
