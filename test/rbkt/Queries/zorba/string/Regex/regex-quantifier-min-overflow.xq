(: A quantifier bound just past 2147483647 is rejected, not wrapped. :)
fn:matches("a", "a{2147483648}")
