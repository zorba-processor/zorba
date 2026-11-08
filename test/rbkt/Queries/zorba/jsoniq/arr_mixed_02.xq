(: Positions of all array updates in one pending update list refer to the
   array before any of them is applied, whatever their order in the list. :)
variable $a := [ 1, 2, 3, 4 ];

(
append json 7 into $a,
insert json 9 into $a at position 2,
replace value of json $a(2) with 8,
insert json 6 into $a at position 4,
delete json $a(4)
);

$a
