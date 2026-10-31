(: If a character occurs more than once in $mapString, its first occurrence
   determines the action (F&O 3.0 5.4.9). :)
(translate("aaa", "aa", "bc"),
 translate("abc", "aab", "xyz"),
 concat("[", translate("ab", "aa", "x"), "]"),
 concat("[", translate("aa", "aa", ""), "]"))
