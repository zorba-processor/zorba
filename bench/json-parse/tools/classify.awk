# classify.awk: charges each sample of `perf script` output (perf record -g with
# frame pointers) to one part of the JSON parse or of the query, by the innermost
# frame that names one. Generic helpers (strings, reference counts, hash tables,
# malloc/free, the C++ library) are charged to their first such caller.
#   perf script -i perf.data | awk -f classify.awk
# Prints one line per part: name, samples, share of all samples.

function part_of(f) {
  if (f ~ /underflow|xsgetn|filebuf|\[kernel|_IO_|__libc_read|__read|read_text|ReadText|transcode/) return "read"
  if (f ~ /istream::get|istream::peek|istream::sentry/) return "tokenize"
  if (f !~ /zorba::/) return ""
  if (f ~ /zorba::rstring|zorba::equals|zorba::ztd::|zorba::atomic|ItemHandle|zorba::store::Item::(add|remove)Reference|rchandle|zorba::operator==|std::/ &&
      f !~ /zorba::(json|simplestore|serializer|[A-Z][A-Za-z]*Iterator)/) return ""
  if (f ~ /JSONObject::~|JSONArray::~|StringItem::~|DecimalItem::~|AtomicItem::~|::free\(\)|Item::destroy/) return "free"
  if (f ~ /json::loader::make_key|json::loader::record_key/) return "keys"
  if (f ~ /createDecimal|Decimal::|MAPM|createInteger|createDouble|Integer::|Float</) return "numbers"
  if (f ~ /createString/) return "strings"
  if (f ~ /createJSONObject\(zorba::store::ItemHandle<zorba::store::Item>&, std::vector<zorba::store::ItemHandle|SimpleJSONObject::(add|findKey|reserve|SimpleJSONObject)|createJSONArray|SimpleJSONArray::push_back|loader::add_value|stack_element|createJSONNull|createBoolean|addHeightOf/) return "containers"
  if (f ~ /json::lexer::/) return "tokenize"
  if (f ~ /json::parser::/) return "grammar"
  if (f ~ /json::loader::/) return "loader"
  if (f ~ /serializ|emitter/) return "q_serialize"
  if (f ~ /getObjectValue|ObjectLookup|JSONItemAccessor/) return "q_navigate"
  if (f ~ /GroupBy|groupby|GroupTuple|Sort|OrderBy|sortTuples/) return "q_groupby_sort"
  if (f ~ /Compare|compare|ValueComp|GenericComp/) return "q_compare"
  if (f ~ /JSONObjectIterator|ObjectConstructor|createJSONObject/) return "q_construct"
  if (f ~ /FLWOR|Flwor|ForIterator|LetIterator|TupleStream/) return "q_flwor"
  if (f ~ /JSONParseIterator|CountIterator|UDFunctionCall|PlanIterator/) return "iterators"
  return ""
}

function flush() {
  if (part == "") part = "other"
  samples[part]++
  total++
  in_sample = 0
}

/^[^ \t]/ { if (in_sample) flush(); in_sample = 1; part = ""; next }
/^[ \t]+[0-9a-f]+ / {
  if (part == "") { sub(/^[ \t]+[0-9a-f]+ /, ""); part = part_of($0) }
  next
}
/^$/ { if (in_sample) flush() }

END {
  if (in_sample) flush()
  for (p in samples) printf "%-16s %8d %6.2f%%\n", p, samples[p], 100 * samples[p] / total
  printf "%-16s %8d\n", "total", total
}
