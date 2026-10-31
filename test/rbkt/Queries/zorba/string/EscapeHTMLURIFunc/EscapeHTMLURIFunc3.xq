(: fn:escape-html-uri escapes every character outside the printable ASCII
   range 32-126 (F&O 3.0 6.4.4). :)
(escape-html-uri("&#x130;"),
 escape-html-uri("http://www.example.com/~b&#xE9;b&#xE9;?a=&#x10000;"),
 escape-html-uri("http://www.example.com/00/Weather/CA/Los Angeles#ocean"))
