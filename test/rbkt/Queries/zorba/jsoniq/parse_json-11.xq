(: A lone '~' used to be looked up as a diagnostic dictionary key when the
   error message was built, failing a ZORBA_ASSERT (json_parse fuzzer). :)
jn:parse-json('[ 1 ~ ]')
