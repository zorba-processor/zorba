import module namespace m = "http://www.example.com/truncate-other-collection" at
                             "truncate_other_collection.xqlib";

m:init();

variable $start := m:state();

m:insert-x-truncate-y();

variable $afterY := m:state();

m:insert-y-truncate-x();

fn:string-join(($start, $afterY, m:state()), "&#10;")
