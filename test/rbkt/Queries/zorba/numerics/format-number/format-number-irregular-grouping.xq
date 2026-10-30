(: Grouping positions that are not all multiples of the smallest one, or
   miss one of its multiples, are used as given (F&O bug 29534). :)
string-join((
  fn:format-number(123456789, "###,##,00"),
  fn:format-number(1234567, "#,##,###"),
  fn:format-number(1234567, "#,###")
), "|") eq "12345,67,89|12,34,567|1,234,567"
