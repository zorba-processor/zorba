### Fixed

- A copy of an element with an explicit `xml:base` attribute got two hidden
  base-uri attributes: the copy of `xml:base` added one and the copy added
  another. The extra attribute took node id space, so nested copies of such
  elements reached the node id limit at about 257 levels instead of about
  409. The copy now has one; base URIs are unchanged.
