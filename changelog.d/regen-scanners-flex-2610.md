### Changed

- The committed XQuery and JSONiq scanners and `FlexLexer.h` are generated
  with flex 2.6.10. The scanner reads its input in 8 KiB chunks into a
  16 KiB buffer on every platform, as before; it still grows for longer
  tokens.
