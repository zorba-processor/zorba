### Fixed

- Parsing UTF-16 or UTF-32 input as an external parsed entity (fragment),
  for example a byte stream passed to `XmlDataManager::parseXML()` with
  external entity processing, failed at 1:1. The input, recognized by its byte
  order mark or by its first characters `<?` (UTF-16) or `<` (UTF-32), is now
  transcoded to UTF-8 before parsing, and errors are located in characters of
  the input. An unpaired surrogate or a truncated code unit is reported as
  "input is not valid UTF-16LE" (or the encoding in question) where it is.
- Bytes that are not valid in the encoding the input is read in were
  reported as "document labelled UTF-16 but has UTF-8 content". The message
  is now "input is not valid UTF-8" for a fragment read as UTF-8 (all string
  inputs, whose encoding declaration does not apply), and "input is not valid
  in its character encoding" for a document, with the location in both cases.
