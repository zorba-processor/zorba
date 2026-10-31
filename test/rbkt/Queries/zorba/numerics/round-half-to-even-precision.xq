(: A precision beyond the significant digits of an xs:double or xs:float
   leaves the value unchanged (F&O 3.0 4.4.5). :)
(round-half-to-even(4.9e-324, 400) eq 4.9e-324,
 round-half-to-even(1.5e-300, 320) eq 1.5e-300,
 round-half-to-even(0.1e0, 20) eq 0.1e0,
 round-half-to-even(xs:float("1.4E-45"), 100) eq xs:float("1.4E-45"),
 round-half-to-even(2.5e0) eq 2e0,
 round-half-to-even(1.25e0, 1) eq 1.2e0)
