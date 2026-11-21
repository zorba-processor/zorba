### Changed

- The committed XQuery and JSONiq scanners and `FlexLexer.h` are generated
  with flex 2.7.1. The scanner reads its input in 8 KiB chunks into a
  16 KiB buffer on every platform, as before; it still grows for longer
  tokens. flex 2.7.1 keeps the position of the character it replaces to end
  `yytext`, so input read after a token is restored in place, and a scanner
  stops the build if its `FlexLexer.h` comes from another flex version.
