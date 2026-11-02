(: The string refers to an external parsed entity whose file does not exist
 : (relative to the static base URI). XDM 3.0 section 3.2 requires all
 : external parsed entities to be expanded: FODC0006, instead of a document
 : without the entity. :)
parse-xml("<!DOCTYPE a [<!ENTITY foo SYSTEM 'parse-xml/missing.entity'>]><a>" ||
  codepoints-to-string(38) || "foo;</a>")
