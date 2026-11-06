(: F&O 3.1 fn:parse-json: "If the input starts with a byte order mark, this
 : function ignores it." :)
jn:members(jn:parse-json("&#xFEFF;[1,2]"))
