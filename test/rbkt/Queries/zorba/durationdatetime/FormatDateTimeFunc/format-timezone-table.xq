(: The timezone formatting table of F&O 3.0 9.8.4.2, with xs:dateTime
   values in winter and in summer. :)
let $offsets := ('-10:00', '-05:00', 'Z', '+05:30', '+13:00')
for $date in ('2018-01-01', '2018-07-01')
let $times := $offsets ! xs:dateTime($date || 'T12:00:00' || .)
return (
  for $f in ('[Z]', '[Z0]', '[Z0:00]', '[Z00:00]', '[Z0000]', '[Z00:00t]',
             '[z]', '[ZZ]', '[ZN]')
  return $f || ' ' || string-join($times ! format-dateTime(., $f), ' '),
  '[ZN] us ' ||
    string-join($times ! format-dateTime(., '[ZN]', (), (), 'us'), ' '),
  '[ZN] America/New_York ' ||
    string-join($times ! format-dateTime(., '[H00]:[m00] [ZN]', (), (),
                                         'America/New_York'), ' '),
  '[ZN] unknown place ' ||
    string-join($times ! format-dateTime(., '[H00]:[m00] [ZN]', (), (),
                                         'Nowhere/Land'), ' ')
)
