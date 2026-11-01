(: Returns the name of the customer with the given id (from customers.csv). :)
import module namespace dir = "http://zorba.io/modules/directory";

declare variable $id external;

for $row in dir:open($dir:root, "customers.csv")("content")()
where $row("id") eq $id
return $row("name")
