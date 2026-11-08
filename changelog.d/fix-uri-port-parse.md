### Fixed

- A URI port is now read only when it is all digits and at most 65535. Before, it was read with `atoi`, so a port beyond the `int` range could wrap to another value (`http://h:4294967376/` was read as port 80) and trailing text was ignored (`http://h:80abc/` was read as port 80). Such authorities are now kept as written, the same as ports from 65536 up already were.
