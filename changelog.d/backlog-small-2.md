### Fixed

- A document, text, processing-instruction or comment node made without a
  parent no longer stays the root of its tree when its construction fails, as
  element and attribute nodes already did.
- The JSON-XML hybrid serializer no longer leaks the stream of its nested XML
  emitter when that emitter cannot be made, and destroys the emitter before
  its stream.
- A two-argument insert into a hash set (the string pools of the store)
  whose copy into the out item fails leaves the set without the new entry.
- A function item that `fn:function-lookup` makes for a contextual function
  numbers the local variables of its body after the context item, position
  or size it captures.
