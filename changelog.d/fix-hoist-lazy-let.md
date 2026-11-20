### Fixed

- At O1 and O2 an expression the optimizer moves out of a for loop is evaluated
  where its value is first read, still once per binding of the variables it
  depends on: under an empty for domain it is no longer evaluated, as at O0, so
  one that does not end no longer keeps the query from ending.
