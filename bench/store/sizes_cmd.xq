(: Prints the command that compiles bench/store/sizes.cpp to assembly with
 : the flags the build uses for src/store/naive/node_items.cpp (store memory
 : benchmark, bench/store/run.sh).
 :
 :   zorba --serialize-text -f -q sizes_cmd.xq -e db:=compile_commands.json
 :         -e src:=sizes.cpp -e out:=sizes.s
 :
 : The precompiled header and the dependency-file options are dropped. :)
import module namespace file = "http://expath.org/ns/file";
import module namespace jn = "http://jsoniq.org/functions";

declare variable $db as xs:string external;
declare variable $src as xs:string external;
declare variable $out as xs:string external;

let $entry := jn:parse-json( file:read-text( $db ) )()
                [ ends-with( .( "file" ), "/store/naive/node_items.cpp" ) ]
let $cmd := $entry[1]( "command" )
return
  if ( empty( $cmd ) ) then
    error( xs:QName( "local:NO_ENTRY" ),
           "node_items.cpp is not in the compilation database" )
  else
    let $cmd := replace( $cmd, " -include \S*cmake_pch[^ ]*", "" )
    let $cmd := replace( $cmd, " -Winvalid-pch", "" )
    let $cmd := replace( $cmd, " -M[TF] \S+", "" )
    let $cmd := replace( $cmd, " -MD( |$)", "$1" )
    let $cmd := replace( $cmd, " -o \S+", concat( " -S -o ", $out ) )
    let $cmd := replace( $cmd, " -c \S+", concat( " -c ", $src ) )
    return concat( $cmd, "&#10;" )
