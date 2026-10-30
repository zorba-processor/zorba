(: A literal '-' after a character range is neither the first nor the last
   character of the group (XSD F.1): invalid, as in QT3 re00086. :)
fn:matches("a", "^(?:[a-c-1-4x-z-7-9]*)$")
