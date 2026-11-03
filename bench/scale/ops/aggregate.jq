(: The average of the numeric values of v. :)
avg(for $r in @INPUT@ return local:num($r.v))
