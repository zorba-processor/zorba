import module namespace m = "http://www.example.com/general-index-probe-gaps" at
                             "general_index_probe_gaps.xqlib";

declare namespace err = "http://www.w3.org/2005/xqt-errors";

(: An untyped key that violates uniqueness after some of its entries were
   inserted leaves no entry behind once the update is undone. :)
m:create(xs:QName("m:uh"), <e id="1" t="y" v="2001"/>,
         xs:QName("m:UHEquality"));
variable $hashInsert :=
  m:insert("UHEquality, untyped 2001 after gYear 2001", xs:QName("m:uh"),
           (<e id="2" t="d" v="5"/>, <e id="3" v="2001"/>));

m:create(xs:QName("m:ut"), <e id="1" t="date" v="2001-01-01"/>,
         xs:QName("m:UTRange"));
variable $treeInsert :=
  m:insert("UTRange, untyped date after the same date", xs:QName("m:ut"),
           (<e id="2" t="d" v="5"/>, <e id="3" v="2001-01-01"/>));

fn:string-join((
  $hashInsert,
  try
  {
    m:point-value("UHEquality value after undo", xs:QName("m:uh"),
                  xs:QName("m:UHEquality"), xs:gYear("2001"))
  }
  catch *
  {
    fn:concat("UHEquality value after undo: ",
              fn:local-name-from-QName($err:code))
  },
  m:point("UHEquality untyped after undo", xs:QName("m:uh"),
          xs:QName("m:UHEquality"), xs:untypedAtomic("2001")),
  $treeInsert,
  try
  {
    m:range-value("UTRange value after undo", xs:QName("m:ut"),
                  xs:QName("m:UTRange"), xs:date("2001-01-01"),
                  xs:date("2001-01-01"), fn:true(), fn:true())
  }
  catch *
  {
    fn:concat("UTRange value after undo: ",
              fn:local-name-from-QName($err:code))
  },
  m:point("UTRange untyped after undo", xs:QName("m:ut"),
          xs:QName("m:UTRange"), xs:untypedAtomic("2001-01-01"))),
  "&#10;")
