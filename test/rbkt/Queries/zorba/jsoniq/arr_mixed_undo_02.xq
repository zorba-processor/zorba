variable $doc := <foo a="1"/>;

variable $a := [ 1, 2, 3, 4 ];

try
{
(
delete json $a(2),
replace value of json $a(2) with 20,

insert node $doc/@a into $doc
);
}
catch *
{
}

$a
