import module namespace m = "http://www.example.com/general-probe-reinit" at
                             "general_probe_reinit.xqlib";

m:init();

m:range-each("lower per bound", (3, 8, 0.5))
