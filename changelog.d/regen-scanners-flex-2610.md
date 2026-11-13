### Changed

- The committed XQuery and JSONiq scanners and `FlexLexer.h` are generated
  with flex 2.6.10. The scanner's default input buffer is now twice the C
  library's `BUFSIZ` (16 KiB on glibc, as before; 2 KiB on macOS); it still
  grows for longer tokens.
