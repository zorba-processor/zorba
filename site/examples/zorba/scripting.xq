(: Zorba's scripting extension: mutable variables, assignment, loops. :)
variable $total := 0;
variable $i := 1;
while ($i le 5) {
  $total := $total + $i * $i;
  $i := $i + 1;
}
$total
