import module namespace map = "http://zorba.io/modules/unordered-maps";

(: XPath 3.0 / XQuery 3.0 B.2: "If an operator in the operator mapping tables
   expects an operand of type ET, that operator can be applied to an operand of
   type AT if type AT can be converted to type ET by a combination of type
   promotion and subtype substitution."
   B.1: "A value of type xs:decimal (or any type derived by restriction from
   xs:decimal) can be promoted to either of the types xs:float or xs:double. The
   result of this promotion is created by casting the original value to the
   required type. This kind of promotion may cause loss of precision."
   B.1: "A function that expects a parameter $p of type xs:decimal can be invoked
   with a value of type xs:integer. This is an example of subtype substitution."
   F&O 3.0 4.2: "If the two operands are not of the same type, subtype
   substitution and numeric type promotion are used to obtain two operands of the
   same type."
   F&O 3.0 op:numeric-equal: "Returns true if and only if the value of $arg1 is
   equal to the value of $arg2."
   So xs:integer against xs:decimal compares exactly, as xs:decimal; any
   xs:integer or xs:decimal against xs:double compares as xs:double, after
   rounding. :)

(: unordered-maps.xq: "getting the entries is based on the "eq" comparison". :)
variable $r := ();
map:create("idc-decimal", "decimal");
map:insert("idc-decimal", 9223372036854775807, "i");
map:insert("idc-decimal", 9223372036854775808.0, "d");
map:insert("idc-decimal", 9223372036854775807.5, "h");
map:insert("idc-decimal", 1, "one");
map:insert("idc-decimal", 1.5, "frac");
$r := (
  map:size("idc-decimal"),
  map:get("idc-decimal", 9223372036854775807),
  map:get("idc-decimal", xs:long("9223372036854775807")),
  map:get("idc-decimal", 9223372036854775807.0),
  map:get("idc-decimal", 9223372036854775808.0),
  map:get("idc-decimal", 9223372036854775807.5),
  map:get("idc-decimal", 1),
  map:get("idc-decimal", 1.0),
  map:get("idc-decimal", 1.5));
map:drop("idc-decimal");
map:create("idc-any", "anyAtomicType");
map:insert("idc-any", 9223372036854775807, "i");
map:insert("idc-any", 9223372036854775808.0, "d");
$r := ($r,
  "|",
  map:size("idc-any"),
  map:get("idc-any", 9223372036854775807),
  map:get("idc-any", xs:long("9223372036854775807")),
  map:get("idc-any", 9223372036854775808.0),
  map:get("idc-any", 9223372036854775807.0));
map:drop("idc-any");
map:create("idc-any-double", "anyAtomicType");
map:insert("idc-any-double", 9223372036854775807, "i");
map:insert("idc-any-double", 9.223372036854775808E18, "x");
$r := ($r,
  "|",
  map:size("idc-any-double"),
  map:get("idc-any-double", 9.223372036854775808E18),
  map:get("idc-any-double", 9223372036854775807));
map:drop("idc-any-double");
$r
