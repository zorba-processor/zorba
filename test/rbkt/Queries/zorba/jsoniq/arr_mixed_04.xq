(: A delete followed by other updates and then a replace at the deleted
   position: the replace is omitted, the other updates are all applied. :)
variable $a := [ 1, 2, 3, 4 ];

(
delete json $a(2),
insert json 9 into $a at position 1,
replace value of json $a(3) with 5,
replace value of json $a(2) with 6
);

$a
