(: The json-xml module converts generically between the two models. :)
import module namespace jx = "http://zorba.io/modules/json-xml";

jx:json-to-xml({ "sku": "A1", "qty": 2, "tags": [ "new" ] })
