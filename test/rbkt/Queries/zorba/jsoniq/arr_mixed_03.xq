(: A replace and a delete at the same array position: the replace is omitted
   in the merged pending update list, so the member is deleted. :)
variable $a := [ 1, 2, 3 ];

(
replace value of json $a(3) with 4,
delete json $a(3)
);

$a
