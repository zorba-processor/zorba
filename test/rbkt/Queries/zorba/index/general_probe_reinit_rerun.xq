import module namespace m = "http://www.example.com/general-probe-reinit" at
                             "general_probe_reinit.xqlib";

m:init();

fn:string-join((
  m:range-each("lower per bound", (3, 8, 0.5), ()),
  m:range-each("lower per bound, upper 8", (3, 8, 0.5), 8)),
  "&#10;")
