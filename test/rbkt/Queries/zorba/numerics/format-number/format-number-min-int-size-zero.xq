(: A zero integer part has no digits when the minimum-integer-part-size is
   zero; a lone zero digit goes in the fractional part (F&O bug 29164). :)
string-join((
  fn:format-number(0.2, ".#"),
  fn:format-number(0.2, "this.#"),
  fn:format-number(0, "#.#"),
  fn:format-number(0, "#.00"),
  fn:format-number(0, "###,###.##"),
  fn:format-number(0.4, "#"),
  fn:format-number(0.2, "#.")
), "|") eq ".2|this.2|.0|.00|.0|0|0"
