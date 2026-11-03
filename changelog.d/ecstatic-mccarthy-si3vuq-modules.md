### Changed

- Reword the module and import error messages to say what to fix: XQST0059 for a module that is not a library module
  or has another target namespace, XQST0048 (use the prefix of the module declaration), XQST0045 for a function
  name in a reserved namespace (use another prefix, `local:` in a main module), XQST0036 and the matching XPTY0004
  forms (import the schema that defines the type), XQST0047, XQST0070 and ZXQP0008. Error codes do not change.
