let $level := 2
let $name := "release-notes"
return element { concat("h", $level) } {
  attribute id { $name },
  attribute class { "title" },
  text { "Release notes" },
  comment { " generated " }
}
