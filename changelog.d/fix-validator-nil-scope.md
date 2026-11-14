### Fixed

- Validation rejects with `XQDY0027` an element with `xsi:nil="true"` that
  has element children, as XML Schema requires (a nilled element has no
  character or element children), in strict and in lax mode. Before, the
  element was accepted when its children were nillable, nilled themselves,
  or followed text in mixed content, and lax validation accepted a nilled
  element with any content.
