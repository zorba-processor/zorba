(: The same in scripting: each statement applies its insert. :)
variable $t := <a>t<b/></a>;
insert nodes (attribute k { 1 }, <x/>, text { "u" }) into $t;
insert nodes (attribute m { 2 }, <y/>) before $t/b;
insert nodes (attribute n { 3 }, <z/>) as first into $t;
serialize($t)
