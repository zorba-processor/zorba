(: An integer that does not fit xs:integer (64 bits unless Zorba is built with
   ZORBA_WITH_BIG_INTEGER) used to escape as internal error ZXQP0003
   (json_parse fuzzer). :)
jn:parse-json('[ 123456789012345678901234567890 ]')
