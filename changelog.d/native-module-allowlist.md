### Security

- `SecurityPolicy::addNativeModule()` restricts `Capability::NativeModules` to the listed module namespace URIs; importing any other native module raises `zerr:ZXQP0067` naming the module. With no entries, `NativeModules` applies to every native module as before. The list is part of the serialized execution plan, so plans saved by earlier versions no longer load (plan archive version 28).
- `zorba --directory` and `zorba-mcp` allow only the native file module (`http://expath.org/ns/file`), which the directory module uses; a query under them can no longer load other native modules in the library path, such as zorba-query or sleep.
