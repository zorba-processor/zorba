### Fixed

- Schema import and validation no longer depend on the process locale: strings coming from Xerces-C (element, attribute and type names, facet values, schema locations, error messages) are converted to UTF-8 explicitly instead of through the locale code page, so non-ASCII text is no longer mangled under `LANG=C`/`LC_ALL=C`.
- Fix a memory leak when streaming an `XMLChArray` and a write through a null buffer in the length-limited `StrX` constructor.
