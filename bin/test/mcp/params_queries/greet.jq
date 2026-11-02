(: Greets a person; the greeting is optional. :)
declare variable $name external;
declare variable $greeting external := "hello";
concat($greeting, ", ", $name)
