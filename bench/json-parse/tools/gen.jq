jsoniq version "1.0";
(: Synthetic NDJSON log of the json-parse benchmark, one event per line.
 :   zorba -f -q gen.jq -e n:=N -e seed:=S -e out:=FILE
 : The same N and S give the same bytes. An event has on average 7.5 objects,
 : 0.24 arrays, 36 keys, 21 strings, 6.8 numbers (4.0 non-integer), 2.1 nulls
 : and 0.26 booleans in about 1,065 bytes (tools/shape.jq measures a file). :)
import module namespace file = "http://expath.org/ns/file";
declare variable $n as string external;
declare variable $seed as string external := "1";
declare variable $out as string external;

declare variable $s as integer := integer($seed);

declare variable $levels := ("info", "info", "info", "info", "debug", "debug", "warn", "error");
declare variable $services := ("checkout", "catalog", "accounts", "search", "billing", "gateway", "inventory", "notifier");
declare variable $methods := ("GET", "GET", "GET", "POST", "PUT", "DELETE");
declare variable $routes := ("/api/v2/items/{id}", "/api/v2/items", "/api/v2/orders/{id}", "/api/v2/orders",
                             "/api/v2/users/{id}/profile", "/api/v2/search", "/healthz", "/api/v2/carts/{id}/lines");
declare variable $statuses := (200, 200, 200, 200, 200, 201, 204, 301, 304, 400, 401, 404, 409, 429, 500, 503);
declare variable $roles := ("viewer", "editor", "admin", "service", "guest");
declare variable $teams := ("platform", "payments", "growth", "data", "mobile", "core");
declare variable $countries := ("DE", "US", "MX", "JP", "BR", "IN", "FR", "KR", "NG", "AU");
declare variable $cities := ("Berlin", "Austin", "Guadalajara", "Osaka", "Recife", "Pune", "Lyon", "Busan", "Lagos", "Perth");
declare variable $agents := (
  "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/141.0.0.0 Safari/537.36",
  "Mozilla/5.0 (Macintosh; Intel Mac OS X 15_6) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/19.0 Safari/605.1.15",
  "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:144.0) Gecko/20100101 Firefox/144.0",
  "okhttp/5.1.0 (synthetic-load-generator; build 4711)");
declare variable $words := ("request", "handled", "upstream", "cache", "lookup", "completed", "retrying", "after",
                            "timeout", "connection", "pool", "reused", "token", "validated", "for", "tenant",
                            "batch", "flushed", "records", "queue", "depth", "below", "threshold", "slow");
declare variable $tags := ("canary", "beta", "internal", "mobile", "bulk", "replay");
declare variable $hex := "0123456789abcdef";

(: A deterministic pseudo-random integer in [0, 2^27) for event $i and draw $k:
 : two rounds of a linear congruential step with a bit rotation in between. :)
declare function local:r($i as integer, $k as integer) as integer
{
  let $h := (($i * 2654435761 + $k * 40503 + $s * 1000003) mod 4294967296 * 1103515245 + 12345) mod 2147483648
  let $h := $h idiv 32768 + $h mod 32768 * 65536
  return ($h * 1103515245 + 12345) mod 2147483648 idiv 16
};

declare function local:pick($items as item*, $i as integer, $k as integer) as item
{
  $items[local:r($i, $k) mod count($items) + 1]
};

declare function local:hex($v as integer, $digits as integer) as string
{
  if ($digits eq 0) then ""
  else local:hex($v idiv 16, $digits - 1) || substring($hex, $v mod 16 + 1, 1)
};

declare function local:hex32($i as integer, $k as integer) as string
{
  local:hex(local:r($i, $k), 7) || local:hex(local:r($i, $k + 1), 7) || local:hex(local:r($i, $k + 2), 7)
  || local:hex(local:r($i, $k + 3), 7) || local:hex(local:r($i, $k + 4), 4)
};

declare function local:pad($v as integer, $digits as integer) as string
{
  let $t := string($v)
  return string-join((for $j in string-length($t) to $digits - 1 return "0", $t))
};

(: $v / 10^$scale written with exactly $scale decimals, e.g. 1234, 2 -> "12.34". :)
declare function local:decimal($v as integer, $scale as integer) as string
{
  let $p := string-join(for $j in 1 to $scale return "0")
  let $d := integer("1" || $p)
  return (if ($v lt 0) then "-" else "") || abs($v) idiv $d || "." || local:pad(abs($v) mod $d, $scale)
};

declare function local:q($v as string) as string
{
  "\"" || $v || "\""
};

declare function local:event($i as integer) as string
{
  let $ms := 1772323200000 + $i * 397 + local:r($i, 1) mod 397
  let $ts := xs:dateTime("1970-01-01T00:00:00Z") + xs:dayTimeDuration("PT0.001S") * $ms
  let $msg := string-join(for $j in 1 to 34 + local:r($i, 2) mod 25 return local:pick($words, $i, 100 + $j), " ")
  return string-join((
    "{\"ts\":", local:q(string($ts)),
    ",\"level\":", local:q(local:pick($levels, $i, 3)),
    ",\"service\":", local:q(local:pick($services, $i, 4)),
    ",\"host\":", local:q("node-" || local:pad(local:r($i, 5) mod 64, 2) || ".dc" || local:r($i, 6) mod 4 || ".example.net"),
    ",\"env\":", local:q(if (local:r($i, 7) mod 10 eq 0) then "staging" else "prod"),
    ",\"pid\":", string(1000 + local:r($i, 8) mod 60000),
    ",\"msg\":", local:q($msg),
    ",\"trace\":{\"id\":", local:q(local:hex32($i, 10)),
      ",\"span\":", local:q(local:hex(local:r($i, 20), 7) || local:hex(local:r($i, 21), 7) || local:hex(local:r($i, 22), 2)),
      ",\"parent\":", if ($i mod 3 eq 0) then local:q(local:hex(local:r($i, 23), 7) || local:hex(local:r($i, 24), 7) || local:hex(local:r($i, 25), 2)) else "null",
    "},\"http\":{\"method\":", local:q(local:pick($methods, $i, 30)),
      ",\"path\":", local:q("/api/v2/items/" || local:r($i, 31) mod 1000000 || "?view=" || local:pick(("full", "summary", "ids"), $i, 32)),
      ",\"status\":", string(local:pick($statuses, $i, 33)),
      if ($i mod 5 eq 4) then () else (",\"bytes\":", string(local:r($i, 34) mod 250000)),
      ",\"duration_ms\":", local:decimal(local:r($i, 35) mod 2000000, 3),
      ",\"route\":", local:q(local:pick($routes, $i, 36)),
    "},\"client\":{\"ip\":", local:q("10." || local:r($i, 40) mod 256 || "." || local:r($i, 41) mod 256 || "." || local:r($i, 42) mod 256),
      ",\"agent\":", local:q(local:pick($agents, $i, 43)),
      ",\"session\":", local:q("s-" || local:hex(local:r($i, 44), 7) || local:hex(local:r($i, 45), 5)),
      ",\"geo\":{\"lat\":", local:decimal(local:r($i, 46) mod 1800000 - 900000, 4),
        ",\"lon\":", local:decimal(local:r($i, 47) mod 3600000 - 1800000, 4),
        ",\"country\":", local:q(local:pick($countries, $i, 48)),
        ",\"city\":", local:q(local:pick($cities, $i, 48)),
    "}},\"metrics\":{\"cpu\":", local:decimal(local:r($i, 50) mod 10000, 2),
      ",\"worker\":", local:q("w-" || local:r($i, 51) mod 32),
    "},\"user\":{\"id\":", local:q("u-" || local:pad(local:r($i, 60) mod 1000000, 6)),
      ",\"role\":", local:q(local:pick($roles, $i, 61)),
      ",\"team\":", if ($i mod 5 lt 2) then "null" else local:q(local:pick($teams, $i, 62)),
    "},\"error\":null",
    if ($i mod 2 eq 0) then (",\"cache\":{\"key\":", local:q("k:" || local:hex(local:r($i, 70), 7)), "}") else (),
    if ($i mod 25 lt 6) then (",\"tags\":[", local:q(local:pick($tags, $i, 71)), ",", local:q(local:pick($tags, $i, 72)), "]") else (),
    if ($i mod 50 lt 13) then (",\"sampled\":", if (local:r($i, 73) mod 2 eq 0) then "true" else "false") else (),
    "}"))
};

file:write-text-lines($out, for $i in 1 to integer($n) return local:event($i))
