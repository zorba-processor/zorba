(: All records in descending order of temperature; the seq of the first. :)
head(for $r in @INPUT@
     order by $r.@TEMP@[$$ instance of decimal] descending empty least, $r.seq descending
     return $r.seq)
