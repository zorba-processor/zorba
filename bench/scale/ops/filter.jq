(: Records with a temperature above 90.0. :)
count(for $r in @INPUT@
      where $r.@TEMP@[$$ instance of decimal] gt 90
      return $r)
