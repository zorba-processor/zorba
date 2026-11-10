### Fixed

- At O1 and O2, a `for` clause whose domain has the empty sequence type no
  longer drops the domain, or without `allowing empty` the clauses before it,
  when their evaluation can raise an error: the query raises the error that it
  raises at O0. Before, such a FLWOR was replaced by the empty sequence (with
  `allowing empty`, its variable by the empty sequence) and the error was lost.
