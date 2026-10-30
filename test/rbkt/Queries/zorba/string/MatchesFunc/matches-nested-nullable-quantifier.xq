(: "(?:r?)*?r" made ICU loop forever (QT3 fn-matches-50, test p1030). The
   correct answer is true (the second alternative matches); a match that
   hits the regex time limit raises an error instead of hanging. Either
   way the query must terminate. :)
try {
  matches("abcde", "(?:r?)*?r|(.{2,4})")
} catch * {
  true()
}
