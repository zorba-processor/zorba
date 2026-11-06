(: A quantifier upper bound just past 2147483647 is rejected, not wrapped. :)
fn:matches("a", "a{1,2147483648}")
