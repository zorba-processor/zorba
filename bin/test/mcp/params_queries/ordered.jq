(: Eight parameters declared in a deliberately non-alphabetical order. :)
declare variable $zeta external;
declare variable $alpha external := "a";
declare variable $mid external;
declare variable $beta external := "b";
declare variable $omega external;
declare variable $kappa external := "k";
declare variable $delta external;
declare variable $sigma external := "s";
string-join(($zeta, $alpha, $mid, $beta, $omega, $kappa, $delta, $sigma), ",")
