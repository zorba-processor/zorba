### Security

- A `wordnet:` thesaurus URI other than the built-in default reads a data file from the library path and loads no native code. It now needs `Capability::FileRead` with no read-path list instead of `Capability::NativeModules`, so under `zorba --directory` and `zorba-mcp` it is denied with `err:FTST0018`. The built-in thesaurus stays allowed without a capability.
