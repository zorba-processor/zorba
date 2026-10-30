declare variable $name external;
declare variable $times as xs:integer external := 2;

string-join(for $i in 1 to $times return concat("hi ", $name), ", ")
