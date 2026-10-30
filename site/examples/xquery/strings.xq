(
  <tokens>{ tokenize("a, b,c", ",\s*") }</tokens>,
  <reordered>{ replace("2026-10-29", "(\d+)-(\d+)-(\d+)", "$3/$2/$1") }</reordered>,
  <matches>{ matches("Neuromancer", "^neuro", "i") }</matches>,
  <number>{ format-number(1234567.891, "#,##0.00") }</number>,
  <date>{ format-date(xs:date("2026-10-29"), "[D1] [MNn] [Y]") }</date>
)
