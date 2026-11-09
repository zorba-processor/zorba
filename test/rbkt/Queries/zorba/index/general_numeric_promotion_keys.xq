import module namespace m = "http://www.example.com/general-numeric-promotion" at
                             "general_numeric_promotion.xqlib";

(: The keys of a general equality index, without the xs:double and xs:float
   images of xs:decimal keys. :)
m:keys()
