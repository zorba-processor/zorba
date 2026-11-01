### Added

- Add `x:parse-html()` and `x:html-doc()` to the `http://zorba.io/modules/xml`
  module: parse HTML as found in practice (omitted end tags, void elements,
  HTML entities, raw script/style text, missing html/head/body) into an XDM
  document with libxml2's HTML parser in recovery mode. Element and attribute
  names are lowercase and in no namespace. `x:html-doc()` resolves and checks
  its URI like `fn:doc()` (security policy, `err:FODC0002`) and honors the
  meta charset; the parser never accesses the network.
