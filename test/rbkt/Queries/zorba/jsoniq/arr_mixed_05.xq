(: Two replaces at one array position are an error even when a delete at
   that position omits them. :)
variable $a := [ 1, 2, 3 ];

(
delete json $a(2),
replace value of json $a(2) with 4,
replace value of json $a(2) with 5
);

$a
