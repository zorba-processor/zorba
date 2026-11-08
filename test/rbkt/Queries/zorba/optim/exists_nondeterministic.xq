(: fn:exists keeps the call of a nondeterministic function although its result type has exactly one item. :)
import module namespace datetime = "http://zorba.io/modules/datetime";
exists(datetime:current-dateTime())
