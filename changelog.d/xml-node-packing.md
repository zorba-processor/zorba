### Changed

- Reduce the memory of parsed XML by 15% (a 106 MiB benchmark file goes from
  1603 MiB to 1367 MiB, 15.1x to 12.9x the file): element nodes are 8 bytes
  smaller and keep up to two attributes and children inline instead of in a
  separate allocation.
