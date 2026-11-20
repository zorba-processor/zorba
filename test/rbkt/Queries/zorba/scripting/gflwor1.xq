import module namespace ddl = "http://zorba.io/modules/store/dynamic/collections/ddl";
import module namespace dml = "http://zorba.io/modules/store/dynamic/collections/dml";

declare namespace ann = "http://zorba.io/annotations";

declare variable $col := xs:QName("myCollection");

declare %ann:sequential function local:test()
{
  ddl:create($col, (<a><c>1</c></a>,<b>2</b>));

  for $i in (1 to 3)
  where $i > 1
  for $j in (1 to 2)
  let $c := count(dml:collection($col)) + $j
  return
    {
      dml:delete(dml:collection($col)[$i]);
      dml:insert-first($col, <x i="{$i}">{$c}</x>);
      dml:insert-last($col, <x i="{$i}">{$c}</x>);
    }

  dml:collection($col)
};

local:test()


(:
Each iteration runs complete before the next: $c counts the collection as the
previous iterations left it.
i , j , c
2 , 1 , 3 --> <x i=2 3> <a> <x i=2 3>
2 , 2 , 5 --> <x i=2 5> <x i=2 3> <x i=2 3> <x i=2 5>
3 , 1 , 5 --> <x i=3 5>   <x i=2 5> <x i=2 3> <x i=2 5>   <x i=3 5>
3 , 2 , 7 --> <x i=3 7>   <x i=3 5> <x i=2 5> <x i=2 5> <x i=3 5>   <x i=3 7>
:)
