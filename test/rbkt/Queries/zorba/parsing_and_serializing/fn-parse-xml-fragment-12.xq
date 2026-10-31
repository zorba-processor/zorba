(: A text declaration needs an encoding declaration and may omit the
   version (XML 1.0 4.3.1 TextDecl). :)
fn:parse-xml-fragment("<?xml encoding='UTF-8'?><a/>")
