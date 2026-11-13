(: A node bound by a let or for of a non-sequential function is no longer
   referenced once the call has returned: ddl:delete of its collection in the
   caller does not raise ZDDY0015, whether the function is inlined or called. :)
import module namespace m = "http://www.example.com/collection-refs" at
                             "refs.xqlib";

m:fill();
variable $let := m:let();
variable $letOutcome := m:delete();

m:fill();
variable $for := m:for();
variable $forOutcome := m:delete();

m:fill();
variable $letCalling := m:let-calling();
variable $letCallingOutcome := m:delete();

m:fill();
variable $forCalling := m:for-calling();
variable $forCallingOutcome := m:delete();

variable $fromFunction := m:from-function();

fn:string-join((
  fn:concat("main, let: ", $let, " ", $letOutcome),
  fn:concat("main, for: ", $for, " ", $forOutcome),
  fn:concat("main, let, body with a call: ", $letCalling, " ",
            $letCallingOutcome),
  fn:concat("main, for, body with a call: ", $forCalling, " ",
            $forCallingOutcome),
  $fromFunction),
  "&#10;")
