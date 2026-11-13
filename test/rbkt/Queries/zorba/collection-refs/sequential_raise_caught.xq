(: A node bound to a local variable of a sequential function that raised an
   error is no longer referenced once the caller caught the error: ddl:delete
   does not raise ZDDY0015. :)
import module namespace m = "http://www.example.com/collection-refs" at
                             "refs.xqlib";

declare namespace err = "http://www.w3.org/2005/xqt-errors";

m:fill();
variable $caught :=
  try { m:raise() }
  catch * { fn:concat(fn:local-name-from-QName($err:code), " ",
                      $err:description) };
variable $outcome := m:delete();

fn:concat($caught, " ", $outcome)
