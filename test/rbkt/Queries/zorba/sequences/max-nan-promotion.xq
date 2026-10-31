(: fn:max/fn:min return NaN of the type the whole sequence is promoted to
   (F&O 3.0 15.4.3, 15.4.4). :)
(max((0e0, xs:float("NaN"))) instance of xs:double,
 string(max((0e0, xs:float("NaN")))),
 min((xs:float("NaN"), 1e0)) instance of xs:double,
 max((1, xs:float("NaN"))) instance of xs:float,
 min((0e0, xs:float("NaN"), 1)) instance of xs:double)
