jsoniq version "1.0";
(: Per-event averages of a log, to compare with the header of gen.jq:
 :   zorba -f -q tools/shape.jq -e log:=FILE
 : It holds every event in memory: measure a sample of 10^4 to 10^5 events. :)
import module namespace file = "http://expath.org/ns/file";
declare variable $log as string external;

declare function local:kinds($v as item) as string*
{
  typeswitch ($v)
  case object return ("objects", for $k in keys($v) return ("keys", local:kinds($v.$k)))
  case array return ("arrays", members($v) ! local:kinds($$))
  case string return "strings"
  case integer return ("numbers", "integers")
  case decimal return ("numbers", "non-integers")
  case double return ("numbers", "non-integers")
  case boolean return "booleans"
  default return "nulls"
};

let $events := parse-json(file:read-text($log), { "jsoniq-multiple-top-level-items" : true })
let $n := count($events)
return {|
  { "events" : $n, "bytes" : round(file:size($log) div $n, 1) },
  for $k in $events ! local:kinds($$)
  group by $kind := $k
  order by $kind
  return { $kind : round(count($k) div $n, 2) }
|}
