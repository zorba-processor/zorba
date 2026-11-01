#!/bin/bash
# Smoke test of the runtime image's MCP server:
#   docker/runtime/smoke-test.sh <image>
# Starts `zorba-mcp --root /data` in a container over a small fixture and
# checks the JSON-RPC responses to initialize, server/discover, tools/list
# and tools/call (run_query, list_documents and a predefined query).
set -euo pipefail

image=${1:?usage: smoke-test.sh <image>}
here=$(cd "$(dirname "$0")" && pwd)
out=$(mktemp)
trap 'rm -f "$out"' EXIT

query='import module namespace dir = "http://zorba.io/modules/directory"; for $o in dir:open($dir:root, "orders/q1.xml")("content")/orders/order let $p := (for $d in dir:open($dir:root, "payments.json")("content")() where $d("order") eq string($o/@id) return $d)[1] order by string($o/@id) return { "order": string($o/@id), "total": xs:integer($o/total), "status": $p("status") }'

meta='{"io.modelcontextprotocol/protocolVersion":"2026-07-28","io.modelcontextprotocol/clientCapabilities":{}}'

{
  # 2025-11-25 and earlier: the initialize handshake.
  jq -cn '{jsonrpc:"2.0",id:1,method:"initialize",params:{protocolVersion:"2025-11-25",capabilities:{},clientInfo:{name:"smoke",version:"0"}}}'
  jq -cn '{jsonrpc:"2.0",method:"notifications/initialized"}'
  jq -cn '{jsonrpc:"2.0",id:2,method:"tools/list"}'
  jq -cn --arg q "$query" '{jsonrpc:"2.0",id:3,method:"tools/call",params:{name:"run_query",arguments:{query:$q}}}'
  jq -cn '{jsonrpc:"2.0",id:4,method:"tools/call",params:{name:"list_documents"}}'
  jq -cn '{jsonrpc:"2.0",id:5,method:"tools/call",params:{name:"customer_name",arguments:{id:"globex"}}}'
  jq -cn '{jsonrpc:"2.0",id:6,method:"tools/call",params:{name:"run_query",arguments:{query:"doc(\"http://example.com/\")"}}}'
  # 2026-07-28: stateless, every request carries its protocol version.
  jq -cn --argjson m "$meta" '{jsonrpc:"2.0",id:7,method:"server/discover",params:{_meta:$m}}'
  jq -cn --argjson m "$meta" '{jsonrpc:"2.0",id:8,method:"tools/list",params:{_meta:$m}}'
} | docker run -i --rm -v "$here/smoke/data:/data:ro" \
      -v "$here/smoke/queries:/queries:ro" "$image" \
      zorba-mcp --root /data --queries /queries > "$out"

cat "$out"

fail=0
check() { # <description> <request id> <jq filter that must be true>
  if jq -e -s --argjson id "$2" \
       'map(select(.id == $id)) | length == 1 and (.[0] | '"$3"')' \
       "$out" > /dev/null; then
    echo "ok   $1"
  else
    echo "FAIL $1"
    fail=1
  fi
}

check "initialize"          1 '.result.serverInfo.name == "zorba-mcp"'
check "tools/list (legacy)" 2 '[.result.tools[].name] | contains(["list_documents","run_query","customer_name"])'
check "run_query"           3 '.result.isError == false and (.result.content[0].text | split("\n") | map(select(. != "") | fromjson) == [{"order":"o1","total":100,"status":"paid"},{"order":"o2","total":250,"status":"pending"}])'
check "list_documents"      4 '.result.isError == false and (.result.content[0].text | split("\n") | map(select(. != "") | fromjson.path) == ["customers.csv","orders/q1.xml","payments.json"])'
check "predefined query"    5 '.result.isError == false and (.result.content[0].text | fromjson) == "Globex"'
check "network denied"      6 '.result.isError == true and (.result.content[0].text | contains("access denied by security policy"))'
check "server/discover"     7 '.result.resultType == "complete" and (.result.supportedVersions | index("2026-07-28") != null) and (.result.capabilities | has("tools"))'
check "tools/list"          8 '[.result.tools[].name] | contains(["list_documents","run_query","customer_name"])'

exit $fail
