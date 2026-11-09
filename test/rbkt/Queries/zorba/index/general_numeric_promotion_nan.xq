import module namespace m = "http://www.example.com/general-numeric-promotion" at
                             "general_numeric_promotion.xqlib";

m:init();

(: NaN is equal to, less than and greater than no number. :)
fn:string-join((
  m:point("Range NaN", xs:QName("m:c"), xs:QName("m:Range"), xs:double("NaN")),
  m:point("Equality NaN", xs:QName("m:c"), xs:QName("m:Equality"),
          xs:float("NaN")),
  m:point("Range untyped NaN", xs:QName("m:c"), xs:QName("m:Range"),
          xs:untypedAtomic("NaN")),
  m:range("NaN lower", xs:QName("m:c"), xs:QName("m:Range"), xs:double("NaN"),
          (), true())),
  "&#10;")
