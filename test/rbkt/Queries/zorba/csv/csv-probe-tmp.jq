import module namespace csv = "http://zorba.io/modules/json-csv";

"P1", csv:parse( "a,b\nT,Y\n", { "cast-unquoted-values" : false } ),
"P2", for $opts in ( { }, { "cast-unquoted-values" : false } )
      return csv:parse( "a,b,c\nT,Y,F\n", $opts ),
"P3", for $opts in ( { "cast-unquoted-values" : false }, { } )
      return csv:parse( "a,b,c\nT,Y,F\n", $opts ),
"P4", for $opts in ( { }, { "cast-unquoted-values" : false } )
      return csv:parse( "a,b,c\nx,Y,F\n", $opts ),
"P5", for $opts in ( { "cast-unquoted-values" : false },
                     { "cast-unquoted-values" : false } )
      return csv:parse( "a,b,c\nT,Y,F\n", $opts )
