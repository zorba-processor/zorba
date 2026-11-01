import module namespace csv = "http://zorba.io/modules/json-csv";

"R1", csv:parse( "a,b,c,d,e,f\nT,Y,F,N,\"T\",true\n",
                 { "cast-unquoted-values" : false } ),
"R2", for $opts in ( { }, { "cast-unquoted-values" : false } )
      return csv:parse( "a,b,c,d,e,f\nT,Y,F,N,\"T\",true\n", $opts ),
"R3", for $opts in ( { }, { "cast-unquoted-values" : false } )
      return csv:parse( "a,b,c,d,f\nT,Y,F,N,true\n", $opts ),
"R4", for $opts in ( { }, { "cast-unquoted-values" : false } )
      return csv:parse( "a,b,c,d,e\nT,Y,F,N,\"T\"\n", $opts ),
"R5", for $opts in ( { }, { "cast-unquoted-values" : false } )
      return csv:parse( "a,b\nT,1\n", $opts ),
"R6", for $opts in ( { }, { "cast-unquoted-values" : false } )
      return csv:parse( "a,b\nT,xy\n", $opts )
