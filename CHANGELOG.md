# Changelog

All notable changes to Zorba are documented in this file.

The format is based on [Keep a Changelog 1.1.0](https://keepachangelog.com/en/1.1.0/), and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [5.0.0] - Unreleased

### At a glance: 3.1 → 5.0

| Area | Before (3.1, 2016) | Now (5.0) |
|---|---|---|
| CI | None in the repository | Every change builds and tests on Linux amd64, Linux arm64, macOS and Clang |
| Memory and undefined behavior | Valgrind memcheck configured in CTest, not run in any CI | ASan, LSan and UBSan in CI; ASan failing tests 450 → 1, UBSan sites 21 → 0 |
| Security | No policy, XXE active | CodeQL, hardening flags, libFuzzer and ClusterFuzzLite, security policy API; XXE closed (#51); secure by default: the library default policy is `SecurityPolicy::secure()` (#103) |
| Compiler warnings | 1,897 to 5,949 per platform | Dozens or fewer |
| Build options tested | Full build only; other options had rotted | `embedded`, `embedded-noicu`, `full-nopch`, debugger in CI; XQueryX and big-integer profiles |
| Test timeouts | Never enforced (`NORMAL_TIMEOUT_VALUE` macro bug) | Enforced |
| W3C conformance | QT3 harness present but inactive | Weekly QT3 run in CI: 26,367 of 27,055 applicable test cases pass (97.5%) |
| Performance | Not measured | XMark and JSONiq benchmarks with A/B comparison; cold build 795 s → 376 s; hash join example 22 s → 0.5 s |
| Toolchain | `auto_ptr`, TR1, CMake 2.8-era checks, Flex 2.5.35 | C++23, CMake 3.28, Bison 3.8, Flex 2.6.4, ICU 78, libxml2 2.15, Xerces-C 3.3 |

### Breaking changes

- Drop Windows support: the Windows-only code paths (StackWalker, WIN32 packaging) are no longer maintained ([#27](https://github.com/zorba-processor/zorba/pull/27)).
- Require a C++23 compiler (GCC 13+, Clang, Apple Clang) and CMake 3.28 or newer; CMake presets use schema v6 ([#25](https://github.com/zorba-processor/zorba/pull/25), [#31](https://github.com/zorba-processor/zorba/pull/31)).
- Add the security policy API (`include/zorba/security.h`, `Zorba::setSecurityPolicy`, `StaticContext::setSecurityPolicy`); the library default later became `secure()` (see the entry below) ([#44](https://github.com/zorba-processor/zorba/pull/44)).
- Bump the serialized execution plan format from version 25 to 26; plans saved by older builds no longer load ([#44](https://github.com/zorba-processor/zorba/pull/44)).
- The `http-uri-resolution` feature is now the `RemoteImport` capability: disabling it applies to the context it is declared on, not the whole process, and remote module or schema imports then fail with `ZXQP0029` instead of `XQST0059` ([#66](https://github.com/zorba-processor/zorba/pull/66)).
- Raise `err:FOAR0002` on `xs:integer` overflow instead of silently wrapping around (64-bit builds); out-of-range string casts to `xs:integer` raise `FOCA0003`, and JSON integers beyond 64 bits parse as `xs:decimal` ([#53](https://github.com/zorba-processor/zorba/pull/53)).
- Remove the SWIG language bindings (Java, XQJ, Python, Ruby, PHP, C#) and the `ZORBA_SUPPRESS_SWIG` option. A stable C API with native bindings is planned for a later release; the existing C API (`include/zorba/zorbac.h`) is unchanged ([#61](https://github.com/zorba-processor/zorba/pull/61)).
- Recursion deeper than the call depth limit raises `zerr:ZXQP0069` instead of `zerr:ZXQP0003`; the limit comes from `SecurityPolicy::setMaxCallDepth()` (default 1024) and also applies to `eval` (formerly a fixed 256). `Properties::setMaxUDFCallDepth()` is no longer read by the engine. ([#75](https://github.com/zorba-processor/zorba/pull/75))
- `XQuery::setTimeout()` overrides the security policy timeout for one query (seconds; 0 = none; -1 = policy). It used to have no effect. ([#75](https://github.com/zorba-processor/zorba/pull/75))
- Regular expressions: patterns that are invalid in XSD/XPath but that ICU accepted now raise FORX0002: a literal `-` that is neither first nor last in a character group (`[a-h-o-z]`, `[0-9-.]`), a character class subtraction that is not last in its group (`[a-[b] ]`), and block escapes whose name cannot be an XSD block name (`\p{IsBasic-Latin}`). ([#76](https://github.com/zorba-processor/zorba/pull/76))
- `fn:parse-xml-fragment` (and the xml module's parse-external-parsed-entity option): a text declaration without an encoding declaration (`<?xml version='1.0'?>`) is FODC0006, as required for an external parsed entity. ([#76](https://github.com/zorba-processor/zorba/pull/76))
- `group by` requires a grouping variable in XQuery and JSONiq: `group by $x mod 10` is XPST0003; write `group by $k := $x mod 10`. ([#76](https://github.com/zorba-processor/zorba/pull/76))
- `function(*)` without an occurrence indicator matches exactly one function item, as in XQuery 3.0: `() instance of function(*)` is false and a sequence of function items no longer matches it. ([#76](https://github.com/zorba-processor/zorba/pull/76))
- A resource that the security policy denies is reported with the error the specification prescribes for a resource that is not available, instead of `zerr:ZXQP0029`: `err:XQST0059` for module and schema import, `err:FODC0002` for `fn:doc`, `err:FOUT1170` for `fn:unparsed-text` and `fn:unparsed-text-lines`, `err:FTST0018` for thesauri, `err:FTST0008` for stop-word lists, `jse:SCHEMA_NOT_FOUND` for JSound schema import and `uri:URI_UNRESOLVED_OR_NOSTREAM` for the fetch module. `fn:doc-available` and `fn:unparsed-text-available` return false for a denied resource instead of raising an error. The message still says that the security policy denied the access and why, e.g. `access denied by security policy (requires capability RemoteImport)`. ([#80](https://github.com/zorba-processor/zorba/pull/80))
- The file, http-client and ftp-client modules report a denied access with their documented error codes: `file:FOFL9999`, `http:HTTP` (`error:HC001` through the EXPath interface) and `ftp:FTP_ERROR`, with the same message. ([#80](https://github.com/zorba-processor/zorba/pull/80))
- `zerr:ZXQP0029` remains only for external entities and DTDs refused while a document is parsed and for URIs refused by a `URIMapper` (`URIMapper::DENY_ACCESS`). ([#80](https://github.com/zorba-processor/zorba/pull/80))
- Regular expressions: `.` matches any character except `\n` and `\r` (it no longer refuses U+0085, U+2028 and U+2029), with the `s` flag it matches exactly one character (`"\r\n"` is two), and `\s` is `[#x20\t\n\r]` (it no longer matches U+00A0, U+0085, U+2028 and the other Unicode spaces), as XSD and F&O 3.0 define them. ([#86](https://github.com/zorba-processor/zorba/pull/86))
- Regular expressions that ICU accepted but XSD does not raise FORX0002: a quantifier after `{n,m}` (`a{1,5}+`) and `\p{...}`/`\P{...}` with a name that is not an XSD general category or block (`\p{L_}`, `\p{Alphabetic}`, `\p{Cs}`). ([#86](https://github.com/zorba-processor/zorba/pull/86))
- `fn:format-number` raises XPTY0004 for a `$value` that is not numeric (an `xs:string` gave NaN); an `xs:untypedAtomic` value is cast to `xs:double` (it gave NaN). ([#86](https://github.com/zorba-processor/zorba/pull/86))
- A character reference to a character that is not allowed in XML, such as `&#x0;`, in direct element content or an attribute value raises XQST0090. ([#86](https://github.com/zorba-processor/zorba/pull/86))
- A direct element constructor whose end tag does not match its start tag raises XQST0118 instead of XPST0003. ([#86](https://github.com/zorba-processor/zorba/pull/86))
- `fn:format-date`, `fn:format-dateTime` and `fn:format-time` with an Olson timezone name as `$place` adjust the value to that timezone (12:00-10:00 with `America/New_York` formats as 17:00 EST). ([#86](https://github.com/zorba-processor/zorba/pull/86))
- The library default security policy is now `SecurityPolicy::secure()`. A program that embeds Zorba and does not call `Zorba::setSecurityPolicy()` runs queries that cannot read or write files, use the network, import modules or schemas over the network, expand external entities, load native modules or read environment variables: `fn:doc` and `fn:unparsed-text` on files or URLs fail with the error the specification prescribes for an unavailable resource (for example `err:FODC0002`), importing the file, http-client, ftp-client or another native module fails with `zerr:ZXQP0067`, the environment functions return the empty sequence, and `ZORBA_URI_PATH` / `ZORBA_LIB_PATH` are ignored. Allow what the application needs with `SecurityPolicy::secure().allow(...)`, or call `zorba->setSecurityPolicy(SecurityPolicy::unrestricted())` to keep the behavior of earlier versions. The `zorba` command line still runs queries unrestricted unless `--secure` is given. See the new "Security Policy" documentation page. ([#103](https://github.com/zorba-processor/zorba/pull/103))
- Windows is no longer a supported platform. Supported platforms are Linux (amd64, arm64) and macOS (arm64). Configuring on a non-UNIX system now stops with an error. The .NET (C#) bindings are not affected. ([#109](https://github.com/zorba-processor/zorba/pull/109))

### Security

- Fix XML external entity (XXE) processing: `fn:doc`, `fn:parse-xml`, `x:parse` and `XmlDataManager::parseXML` read `SYSTEM` entities from any file and bypassed URI mappers and resolvers. External resources are now loaded only if the security policy allows `ExternalEntities`, `FileRead` or `Network`; `XML_PARSE_NONET` is always on, entity amplification is limited, and refused resources raise `ZXQP0029` ([#51](https://github.com/zorba-processor/zorba/pull/51)).
- Harden XQueryX and Xerces schema loading: no global libxml2 state, libxslt security preferences forbid file and network access, Xerces gets an entity resolver and a `SecurityManager`; remove the unused `zorbautils/SAXParser` and the `ZORBA_WITH_LIBXML2_SAX` option ([#51](https://github.com/zorba-processor/zorba/pull/51)).
- Fix parameter re-substitution in diagnostics: user text quoted in an error message (for example from the JSON parser) could inject `$N` markers and expand other parameters or raise an internal error ([#45](https://github.com/zorba-processor/zorba/pull/45)).
- Enable compiler and linker hardening by default on GCC and Clang (`ZORBA_HARDENING`): PIE, stack protector, stack-clash protection, `_FORTIFY_SOURCE`, RELRO/BIND_NOW, libstdc++/libc++ assertions; CI verifies the binaries ([#33](https://github.com/zorba-processor/zorba/pull/33)).
- Add CodeQL scanning (security-extended) on pushes, pull requests and weekly; of 79 initial alerts, 12 were real and fixed, the rest are triaged as false positives or by design ([#33](https://github.com/zorba-processor/zorba/pull/33), [#35](https://github.com/zorba-processor/zorba/pull/35)).
- Fix a buffer overflow in `icu_streambuf` for writes over 4096 UTF-16 units, and a dropped tail of output in `xsputn()` ([#35](https://github.com/zorba-processor/zorba/pull/35)).
- Fix a heap overflow in `utf8::to_string(char16_t)` and use-after-free and delete-through-base-without-virtual-destructor bugs found by ASan ([#28](https://github.com/zorba-processor/zorba/pull/28)).
- Fix a time-of-check/time-of-use race in `mmap_file`, uninitialized reads in `zorbacmd` options, and replace `localtime`/`gmtime` with `localtime_r`/`gmtime_r` ([#35](https://github.com/zorba-processor/zorba/pull/35)).
- Add libFuzzer targets for the XQuery and JSONiq compilers and the XML and JSON parsers, with ClusterFuzzLite on pull requests and daily batch runs ([#37](https://github.com/zorba-processor/zorba/pull/37)).
- Fix crashes, hangs and aborts found by fuzzing: endless loop and out-of-memory on unterminated comments or tags at end of input, `fn:analyze-string` on an empty input, infinite loop in `URI::toPathNotation`, abort on `xml:base=""`, null dereferences in JSON update expressions, and out-of-range reads in QName parsing ([#37](https://github.com/zorba-processor/zorba/pull/37)).
- Fix undefined behavior reported by UBSan: 21 sites and 445 reports reduced to 0 ([#34](https://github.com/zorba-processor/zorba/pull/34)).
- Check `pthread` return values and treat failures as fatal ([#27](https://github.com/zorba-processor/zorba/pull/27), [#50](https://github.com/zorba-processor/zorba/pull/50)).
- Enforce the security policy at URL resolution: `fn:doc`, `fn:unparsed-text`, module and schema import, thesauri, stop words, JSound and `fetch` are checked before any built-in resolver runs. File paths are compared after resolving `..` and symlinks, URLs by scheme, host, port and path segment, and with `Network` allowed, hosts resolving to loopback, private or link-local addresses are refused unless explicitly allowed. Refusals raise `ZXQP0029`. `ZORBA_URI_PATH` and `ZORBA_LIB_PATH` are honored only with the `Environment` capability. Redirects and DNS rebinding are not covered yet ([#56](https://github.com/zorba-processor/zorba/pull/56)).
- Enforce the security policy in modules: importing the file module needs `FileRead` or `FileWrite`, the http-client and ftp-client modules need `Network` (`ZXQP0067`); native libraries load only with `NativeModules`. The file module checks every path (canonical, so `..` and symlinks cannot escape) and opens files race-free; the HTTP and FTP clients allow only http/https/ftp/ftps and check every redirect target and the address of every connection, closing the redirect and DNS-rebinding gaps. `fn:environment-variable` and `fn:available-environment-variables` return nothing without `Environment`. HTTP resolution, zorba-query queries and the file resolver run under the caller's policy; `zq:load-from-query-plan` needs an unrestricted caller; WordNet thesauri need `NativeModules` ([#66](https://github.com/zorba-processor/zorba/pull/66)).
- Enforce the security policy timeout during evaluation: a deadline checked every 1024 iterator calls aborts the query with `zerr:ZXQP0068`, which `try`/`catch` cannot catch; nested plans (`eval`, function bodies) share the deadline. No timeout by default. Replaces the unused timeout thread. ([#75](https://github.com/zorba-processor/zorba/pull/75))
- Deep recursion raises `zerr:ZXQP0069` instead of crashing: besides the call depth limit, a function call is refused when less than a third of the thread's stack is left (closing the plan afterwards recurses through all call levels again). ([#75](https://github.com/zorba-processor/zorba/pull/75))
- `zorba` command line: new options restrict what a query may do. `--secure` starts from a policy that allows nothing; `--allow` and `--deny` take a comma-separated list of capabilities (`external-entities`, `file-read`, `file-write`, `network`, `remote-import`, `environment`, `native-modules`, `all`) and are applied in command-line order; `--read-path`, `--write-path` and `--network-prefix` (repeatable) limit file and network access to directories and URL prefixes; `--private-addresses allow|deny` controls access to loopback, private and link-local addresses. With `--secure` the current directory is no longer added to the module and library paths. Without these options `zorba` still runs queries unrestricted. ([#79](https://github.com/zorba-processor/zorba/pull/79))
- CI runs `scripts/check-os-access.sh`, which fails when code in `src/` or `modules/` opens files, loads libraries, reads the environment, connects to the network or parses XML without an entry in the reviewed allow-list `scripts/check-os-access.allow`. ([#79](https://github.com/zorba-processor/zorba/pull/79))
- Deeply nested queries fail to compile with `zerr:ZXQP0070` instead of overflowing the stack. The limit is the nesting depth of the query's syntax tree (FLWOR clauses, predicates, typeswitch/switch cases and catch clauses count one level each), set with `SecurityPolicy::setMaxExprDepth()` (default 1500; 0 = no limit other than the stack); it is also capped by the stack left on the compiling thread, so threads with small stacks and eval from deep recursion fail cleanly too. Parse trees are destroyed without recursion, so even queries far beyond the limit (or with a syntax error deep inside) no longer crash. ([#84](https://github.com/zorba-processor/zorba/pull/84))
- JSound schema imports and `$constraints` (`jsd:validate`, `jsd:annotate`) now run under the security policy of the calling query instead of the instance policy, so a query restricted with `StaticContext::setSecurityPolicy()` can no longer read files or reach the network through them. ([#106](https://github.com/zorba-processor/zorba/pull/106))
- `zorba --directory <dir>` runs the query under the secure policy with only `file-read` (restricted to `<dir>`) and `native-modules` allowed; options that would widen it (`--allow`, `--read-path`, `--write-path`, `--network-prefix`, `--private-addresses allow`) are rejected. The directory module rejects paths with `..` segments, absolute paths and paths through symbolic links (`dir:INVALID_PATH`), and its listing does not follow symbolic links. ([#111](https://github.com/zorba-processor/zorba/pull/111))
- `zorba-mcp` runs every query under the same policy as `zorba --directory`: the secure policy with only `file-read` (restricted to the root) and `native-modules` allowed. ([#112](https://github.com/zorba-processor/zorba/pull/112))

### Added

- Add differential testing against Saxon-HE (`.github/workflows/differential.yml`, `test/differential/`): weekly and on demand, the plain XQuery 3.0 rbkt queries run on Zorba and Saxon-HE and disagreements are classified in an informational report ([#65](https://github.com/zorba-processor/zorba/pull/65)).
- Add the security policy API: `SecurityPolicy` with capabilities (`ExternalEntities`, `FileRead`, `FileWrite`, `Network`, `RemoteImport`, `Environment`, `NativeModules`), allow-lists, limits and `secure()`/`unrestricted()` presets; queries can never widen the policy of their context; new error codes `ZXQP0067`, `ZXQP0068`, `ZXQP0069` ([#44](https://github.com/zorba-processor/zorba/pull/44)).
- Add `err:ZXQP0029` for refused external resources during XML loading ([#51](https://github.com/zorba-processor/zorba/pull/51)).
- Add a documentation site (XQuery, JSONiq and Zorba tutorials, cheat sheet) on GitHub Pages; all 59 code examples are run in CI and the shown output is verified ([#30](https://github.com/zorba-processor/zorba/pull/30)).
- Add a new logo and color palette to the site and README ([#46](https://github.com/zorba-processor/zorba/pull/46)).
- Add `CMakePresets.json` with configure, build, test and workflow presets (`core-release`, `full-release`, `full-debug`, `full-release-offline`) ([#25](https://github.com/zorba-processor/zorba/pull/25), [#31](https://github.com/zorba-processor/zorba/pull/31)).
- Add build profile presets `embedded`, `embedded-noicu` and `full-nopch`, each built and tested in CI with a library size report ([#42](https://github.com/zorba-processor/zorba/pull/42)).
- Add CI on GitHub Actions: linux-amd64 (GCC 16), linux-arm64 (GCC 13), macOS arm64 (Apple Clang) and a Clang job; HTTP tests run against a containerized test server ([#25](https://github.com/zorba-processor/zorba/pull/25), [#39](https://github.com/zorba-processor/zorba/pull/39)).
- Publish builder and command-line `zorba` container images to GHCR ([#25](https://github.com/zorba-processor/zorba/pull/25)).
- Add a `bigint` preset and CI profile for the optional big-integer `xs:integer` build ([#53](https://github.com/zorba-processor/zorba/pull/53)).
- Add an ASan, LSan and UBSan preset with a weekly CI run and a summary of memory errors, leaks and undefined behavior ([#28](https://github.com/zorba-processor/zorba/pull/28), [#34](https://github.com/zorba-processor/zorba/pull/34)).
- Every CI build (and the Sanitizers workflow, under ASan/UBSan/LSan) replays the fuzz regression inputs in `test/fuzz/regressions/` through standalone replay drivers that need no libFuzzer ([#67](https://github.com/zorba-processor/zorba/pull/67)).
- Add XMark and JSONiq benchmark suites with an A/B comparison workflow ([#38](https://github.com/zorba-processor/zorba/pull/38)).
- Add a weekly W3C QT3 conformance run (pinned qt3tests commit, sharded, per-case timeout) with a committed pass baseline; any regression fails the run. First run: 26,367 of 27,055 applicable test cases pass (97.5%); fix FOTS driver bugs in version matching, XSD 1.0/1.1 dependencies, single-case selection and environment parameter collisions ([#59](https://github.com/zorba-processor/zorba/pull/59)).
- Add an on-demand build-time measurement workflow ([#29](https://github.com/zorba-processor/zorba/pull/29)).
- Add a pull request template with a changelog line.
- Add `SecurityPolicy::checkURI()` to test a URI against a policy ([#56](https://github.com/zorba-processor/zorba/pull/56)).
- Add `SecurityPolicy::checkConnection()` and `SecurityPolicy::openFile()`, a `checkURI()` overload that reports the reason, and `curl::restrict_to_policy()` in util-curl ([#66](https://github.com/zorba-processor/zorba/pull/66)).
- Add a release workflow for `v*` tags: tests on linux-amd64, then publishes a source tarball, SPDX SBOMs, `SHA256SUMS` and signed build provenance attestations on a draft GitHub Release with notes from this changelog, and tags the multi-arch `zorba` image as `<version>` (plus `<major>.<minor>`, `<major>` and `latest` for stable releases only); master builds now publish `zorba:edge` instead of moving `latest`. See `RELEASING.md` and `docker/README.md`.
- Add changelog fragments: each pull request adds `changelog.d/<branch>.md` instead of editing `CHANGELOG.md`, and `scripts/changelog_assemble.py` merges the fragments (with pull request links) into the release section, so open pull requests no longer conflict on the changelog. ([#70](https://github.com/zorba-processor/zorba/pull/70))
- `test/libunit/mapm_threads`, a multithreaded decimal/integer stress test, and
  a ThreadSanitizer job (`tsan`, `tsan-bigint` presets) in the Sanitizers
  workflow. ([#72](https://github.com/zorba-processor/zorba/pull/72))
- `j8_decimal` benchmark query (decimal arithmetic). ([#72](https://github.com/zorba-processor/zorba/pull/72))
- Add generated boundary-value matrices and property tests to the differential testing workflow (`test/differential/boundary.py`, `property.py`; `set=rbkt|boundary|property`): about 38,000 deterministic boundary cases per atomic type (operators, comparisons, casts, string, numeric, date/time and aggregate functions) run on Zorba and Saxon-HE, and about 5,900 seeded random property cases (serialization and cast round trips, inverses, idempotence, sort stability, Python oracles) run on Zorba alone; results are informational. ([#73](https://github.com/zorba-processor/zorba/pull/73))
- `zorba --timeout <seconds>` aborts queries that run longer; `--max-udf-call-depth` sets the policy's call depth limit. ([#75](https://github.com/zorba-processor/zorba/pull/75))
- `zorba --max-call-depth`, an alias for `--max-udf-call-depth`. ([#79](https://github.com/zorba-processor/zorba/pull/79))
- Add `CONTRIBUTING.md` with build, CI, changelog and review guidelines. ([#82](https://github.com/zorba-processor/zorba/pull/82))
- `zorba --max-expr-depth <depth>` sets the policy's expression nesting limit. ([#84](https://github.com/zorba-processor/zorba/pull/84))
- Debugger server: DBGP `property_value` returns the full value of a variable in the top stack frame (`-m` limits the bytes sent, `-p` selects a page of 32 sequence items). ([#88](https://github.com/zorba-processor/zorba/pull/88))
- Debugger server: DBGP `property_set` assigns the result of an XQuery expression to an assignable prolog or block-local variable of the suspended query; the value must match the declared type as in an assignment. An existing variable that can not take the value answers `success="0"`, an unknown variable error 300, and an expression that fails to evaluate error 206. ([#88](https://github.com/zorba-processor/zorba/pull/88))
- Debugger server: DBGP `property_get` honours `-p` (pages of 32 children, with `page`, `pagesize` and `numchildren`) and `-m` (truncated value, with `size`). ([#88](https://github.com/zorba-processor/zorba/pull/88))
- Debugger client API: `DebuggerClient::property_set` overload that sends the value; xqdb command `set -n <name> -v <expression>`. ([#88](https://github.com/zorba-processor/zorba/pull/88))
- `scripts/ast2dot.xq` converts the XML parse tree printed by `zorba --print-ast` into a Graphviz DOT graph; it replaces the removed `parsenode_print_dot_visitor`. A ctest (`scripts/ast2dot`) covers it. ([#90](https://github.com/zorba-processor/zorba/pull/90))
- Unit test `test/libunit/ordpath` for `OrdPath` (compress/decompress round trip, document order, `insertBefore`/`insertAfter`/`insertInto`, relative position); it replaces the unbuilt `test/test_ordpath.cpp` command-line tool. ([#102](https://github.com/zorba-processor/zorba/pull/102))
- Add `x:parse-html()` and `x:html-doc()` to the `http://zorba.io/modules/xml` module: parse HTML as found in practice (omitted end tags, void elements, HTML entities, raw script/style text, missing html/head/body) into an XDM document with libxml2's HTML parser in recovery mode. Element and attribute names are lowercase and in no namespace. `x:html-doc()` resolves and checks its URI like `fn:doc()` (security policy, `err:FODC0002`) and honors the meta charset; the parser never accesses the network. ([#110](https://github.com/zorba-processor/zorba/pull/110))
- Directory module (`http://zorba.io/modules/directory`): query a directory tree of XML, JSON and CSV files. `dir:list($root)` lists the files (relative path, format, size, modification time); `dir:open($root, $path)` and `dir:documents($root, $formats)` return document records `{ "path", "format", "content" }` (a document node for XML, the parsed item for JSON, an array of row objects for CSV) for joins across files and formats. A file that cannot be parsed yields `{ "path", "format", "error" : { "code", "namespace", "message" } }` instead of stopping the query. HTML files are listed but not parsed yet. `$dir:root` holds a default root bound by the embedder or the command line. ([#111](https://github.com/zorba-processor/zorba/pull/111))
- `zorba --directory <dir>`: runs the query with the directory module's `$dir:root` bound to `<dir>` and writes the result as JSON, one item per line. ([#111](https://github.com/zorba-processor/zorba/pull/111))
- `zorba-mcp --root <dir> [--queries <dir>] [--timeout <seconds>] [--max-output <bytes>]`: an MCP server (JSON-RPC 2.0 over stdio) that lets an AI agent query a directory of XML, JSON and CSV documents. It implements MCP 2026-07-28 (per-request `_meta`, `server/discover`) and, after an `initialize` request, the handshake revisions 2025-11-25, 2025-06-18 and 2025-03-26. Tools: `list_documents`, `run_query` (a JSONiq/XQuery query with the directory module's `$dir:root` bound; the result is JSON, one item per line) and one tool per `<name>.jq` file in `--queries` (its leading comment is the description, its external variables are string parameters). A failed query returns the Zorba error code, message and the line and column of the failing expression. Output longer than `--max-output` (default 128 KiB) is cut with a marker; each call is limited to `--timeout` seconds (default 30). ([#112](https://github.com/zorba-processor/zorba/pull/112))
- Add an opt-in to run the W3C QT3 suite under the secure policy (`test/fots/qt3_run.py --policy secure`, QT3 workflow input `policy`): the driver and the test-case queries get only file-read under the suite, driver, manifest and expected-failures directories plus native modules; the default run is unchanged. ([#113](https://github.com/zorba-processor/zorba/pull/113))

### Changed

- Set the version to 5.0.0; versioned install paths change accordingly (`share/zorba-5.0.0`, core module directories, `share/cmake/zorba-5.0.0`) ([#63](https://github.com/zorba-processor/zorba/pull/63)).
- Speed up hash joins: the index-join rule now applies to comparisons on untyped XML and JSON operands; a JSONiq join over 10 MB drops from 22 s to 0.5 s, and over 100 MB from a 300 s timeout to 8.2 s ([#41](https://github.com/zorba-processor/zorba/pull/41)).
- Cut the cold build from 795 s to 376 s with a precompiled header (`ZORBA_USE_PCH`, default on) and header cleanup ([#29](https://github.com/zorba-processor/zorba/pull/29)).
- Port the grammar to Bison 3.8 and Flex 2.6.4 and regenerate the parsers; parser and scanner files compile at `-O1` (parsers from about 225 s to about 55 s each), and the cold build drops from 658 s to 427 s. Regeneration is opt-in (`ZORBA_REGENERATE_PARSER`) and CI checks the checked-in files for drift ([#36](https://github.com/zorba-processor/zorba/pull/36)).
- Port to C++23: remove `auto_ptr`, `binary_function`, `mem_fun`, `std::iterator` inheritance, TR1 headers and per-OS atomics; build with GCC 16, Clang, Apple Clang, ICU 78, libxml2 2.15, Xerces-C 3.3 and libcurl 8 ([#25](https://github.com/zorba-processor/zorba/pull/25), [#26](https://github.com/zorba-processor/zorba/pull/26)).
- Reduce compiler warnings from 1897 to 31 (GCC 16), 2885 to 53 (GCC 13) and 5949 to 8 (Apple Clang); with the later migrations and parser regeneration the counts are lower still ([#26](https://github.com/zorba-processor/zorba/pull/26), [#32](https://github.com/zorba-processor/zorba/pull/32), [#36](https://github.com/zorba-processor/zorba/pull/36)).
- Migrate deprecated libcurl (form API to MIME API) and libxml2 APIs (`xmlSubstituteEntitiesDefault`, `xmlSAXUserParseMemory`, `lastError`) ([#32](https://github.com/zorba-processor/zorba/pull/32)).
- Build with upstream Clang and libstdc++ (4333 of 4333 tests pass) ([#39](https://github.com/zorba-processor/zorba/pull/39)).
- Update all GitHub Actions to their latest versions on Node 24 ([#43](https://github.com/zorba-processor/zorba/pull/43)).
- Run CI builds and security scans only when code changes ([#48](https://github.com/zorba-processor/zorba/pull/48)).
- Build the debugger (`xqdb`) in the `full-nopch` CI profile; `XqdbClient` members are `std::unique_ptr`; remove redundant null checks before `delete` ([#27](https://github.com/zorba-processor/zorba/pull/27), [#50](https://github.com/zorba-processor/zorba/pull/50)).
- Build XQueryX and run its tests in the `full-nopch` CI profile ([#51](https://github.com/zorba-processor/zorba/pull/51)).
- Remove references to the former company's infrastructure (test hosts, CDash, CircleCI, Docker Hub) and replace two test modules carrying its copyright notice ([#25](https://github.com/zorba-processor/zorba/pull/25)).
- Document the 5.0 threading model: one engine per process (`Zorba::getInstance` is a singleton over a single store), used from one thread at a time or behind the application's own mutex; separate processes are safe. `ZORBA_FOR_ONE_THREAD_ONLY=OFF` remains unsupported. Thread-safety is planned for a 5.x release. ([#77](https://github.com/zorba-processor/zorba/pull/77))
- Reorganized the repository root: CMake support files moved to `cmake/` (`cmake_modules/` is now `cmake/modules/`, `config/` is now `cmake/config/`, plus the CMake helper `.txt`/`.cmake`/`.in` files); `NOTICE.xml` moved to `doc/legal/`, `KNOWN_ISSUES.txt` to `doc/`, `import_tests.sh` to `scripts/`. Installed CMake files (`ZorbaConfig.cmake`, `ZorbaUse.cmake`, `ZorbaModule.cmake`, `ExternalModuleConfig.cmake.in`) keep their install locations. ([#78](https://github.com/zorba-processor/zorba/pull/78))
- `README.txt` and `Readme.md` merged into `README.md`. ([#78](https://github.com/zorba-processor/zorba/pull/78))
- `zorba --disable-http-resolution` is now the same as `--deny remote-import`. ([#79](https://github.com/zorba-processor/zorba/pull/79))
- Replace the vendored Loki singleton library (`src/util/singleton.h`,
  `src/util/threads.h`) with the C++ standard library. The API item factory is
  still destroyed by `Zorba::shutdown()` and re-created after a new
  initialization; concurrent first calls to `Zorba::getItemFactory()` are now
  synchronized. ([#82](https://github.com/zorba-processor/zorba/pull/82))
- The plan serialization format version is now 27 (the security policy in a serialized static context includes the expression nesting limit); plans saved by earlier versions must be recompiled. ([#84](https://github.com/zorba-processor/zorba/pull/84))
- The Sanitizers workflow's asan job now fails when any test fails under ASan/LSan/UBSan instead of only summarizing sanitizer logs. ([#94](https://github.com/zorba-processor/zorba/pull/94))
- CI: the embedded, embedded-noicu, bigint and full-nopch profile jobs now run on a pull request only when files in their area change (path filters in `.github/ci-filters.yml`); the required jobs still run for any code change and master pushes run everything. `scripts/check-profile-filters.sh` fails if a file containing a profile's guard macro is not covered by that profile's filter. ([#96](https://github.com/zorba-processor/zorba/pull/96))
- CI: the Linux jobs pull the builder image published to GHCR (tag `df-<hash of the Dockerfile>`, built by `images.yml` on master) instead of building it, and build it locally only when the Dockerfile changed. ([#96](https://github.com/zorba-processor/zorba/pull/96))
- The JSON serialization method writes the `item-separator` serialization parameter, when set, between top-level items (it was ignored). ([#111](https://github.com/zorba-processor/zorba/pull/111))

### Fixed

- Fix a memory leak of parser error objects when a query has several lexical errors ([#67](https://github.com/zorba-processor/zorba/pull/67)).
- Fix a memory leak when a function containing a full-text `contains text` expression is inlined, and a double delete of its positional filters (`ordered`, `window`, ...) that the leak was hiding ([#67](https://github.com/zorba-processor/zorba/pull/67)).
- Fix a heap buffer over-read in `rstring::find_last_of` when the search starts at index 0 (reached by declaring an external function in a namespace whose URI has a one-character path), found by fuzzing ([#60](https://github.com/zorba-processor/zorba/pull/60)).
- Fix an internal error (`ZXQP0002`) in static typing of `namespace-node()` steps on the child, descendant, sibling and ancestor axes, found by fuzzing ([#60](https://github.com/zorba-processor/zorba/pull/60)).
- Fix the full-text grammar accepting any literal where XQFT requires a string literal (`contains text 2 "x"` now raises `XPST0003` instead of an invalid downcast) ([#60](https://github.com/zorba-processor/zorba/pull/60)).
- Fix misaligned and overrunning blocks in the expression arena allocator: blocks are aligned to `alignof(std::max_align_t)` and requests larger than a page get their own block ([#60](https://github.com/zorba-processor/zorba/pull/60)).
- Fix `mmap_file` read-write and write-only mappings, which opened the file write-only and failed with `EACCES` ([#60](https://github.com/zorba-processor/zorba/pull/60)).
- Fix a crash at startup when `ZORBA_URI_PATH` or `ZORBA_LIB_PATH` is unset, and a use-after-free in locale lookup that crashed `format-date` with a language ([#25](https://github.com/zorba-processor/zorba/pull/25)).
- Fix `fn:replace` with the `q` flag returning an empty string on ICU 78 ([#25](https://github.com/zorba-processor/zorba/pull/25)).
- Fix a hang in `fn:parse-xml-fragment` and external parsed entities on libxml2 2.12 and later by rewriting the fragment loader on the public push-parser API ([#25](https://github.com/zorba-processor/zorba/pull/25)).
- Fix Xerces error messages that printed a pointer instead of text, and a parallel `ctest` race where `foo.xq` and `foo.jq` shared a result file ([#25](https://github.com/zorba-processor/zorba/pull/25)).
- Fix a double delete in the `OrdPath` copy constructor, missing reference counting in `SequenceType::operator=`, and `zstring == NULL` calling `strlen(NULL)` ([#26](https://github.com/zorba-processor/zorba/pull/26)).
- Fix ASan failures: 450 failing tests reduced to 3 (then 1 after the parser port), memory errors from 12 to 0, leak sites from 101 to 12 ([#28](https://github.com/zorba-processor/zorba/pull/28), [#36](https://github.com/zorba-processor/zorba/pull/36)).
- Fix leaks and double frees on parse-error paths (Bison `YYERROR` rules, `NumericLiteral`) ([#36](https://github.com/zorba-processor/zorba/pull/36)).
- Fix an assertion failure (`"i >= 0"`) when nested FLWOR expressions are flattened and an index key refers to an out-of-scope variable ([#41](https://github.com/zorba-processor/zorba/pull/41)).
- Fix `XmlDataManager` and `JsonDataManager` failing at start (`XQST0009`) and a crash in `static_context::validate` in builds without XML Schema or ICU ([#42](https://github.com/zorba-processor/zorba/pull/42)).
- Fix `jn:parse-json` raising an internal error on out-of-range numbers; it now raises `FOAR0002`, and integers beyond 64 bits parse as `xs:decimal` ([#37](https://github.com/zorba-processor/zorba/pull/37), [#53](https://github.com/zorba-processor/zorba/pull/53)).
- Fix per-test CTest timeouts, which were never applied because `ZORBA_ADD_TEST` read a removed variable ([#53](https://github.com/zorba-processor/zorba/pull/53)).
- Fix destruction order of expressions and FLWOR clauses, which was undefined behavior in every FLWOR query ([#37](https://github.com/zorba-processor/zorba/pull/37)).
- Fix crashes found by the W3C QT3 suite: `xs:base64Binary(xs:hexBinary(""))`, `fn:unparsed-text-available(())`, an inline function with an empty body (now returns the empty sequence), and the http-client module on status lines without a reason phrase (HTTP/2) or empty header values, which aborted `fn:unparsed-text` on missing `http:` resources ([#64](https://github.com/zorba-processor/zorba/pull/64)).
- Fix `fn:serialize` error codes for bad `$params`: `XPTY0004` for a wrong element, `SEPM0017` for invalid parameters, `SEPM0019` for duplicates; parameters in other namespaces are ignored and `item-separator` is accepted ([#64](https://github.com/zorba-processor/zorba/pull/64)).
- Fix `fn:format-number` with a zero integer part when the picture has no mandatory integer digit (`.2`, `.0`), irregular grouping separators, adjacent or trailing grouping separators (`FODF1310`), overflow to infinity and negative zero ([#64](https://github.com/zorba-processor/zorba/pull/64)).
- Fix fractional seconds (`[f]`) in `format-dateTime`/`format-time`: digit family, truncation, optional digits, grouping separators and width modifiers follow F&O 3.1; invalid width modifiers raise `FOFD1340` ([#64](https://github.com/zorba-processor/zorba/pull/64)).
- Fix regular expressions that never finished in ICU (e.g. `(?:r?)*?r`): matching now has a time limit and raises `ZXQP0001` when it is exceeded ([#64](https://github.com/zorba-processor/zorba/pull/64)).
- Fix schema import rejecting valid XSD 1.0 schemas that restrict a union-typed element to a restriction of the union (`XQST0059`, Xerces-C bug): the Linux CI, builder and `zorba` runtime images build Xerces-C from the patched fork [zorba-processor/xerces-c](https://github.com/zorba-processor/xerces-c) (`v3.3.0-zorba.2`), and the 161 QT3 test-cases importing `unionListDefined.xsd` now run; macOS and other stock Xerces-C builds still have the bug. ([#69](https://github.com/zorba-processor/zorba/pull/69))
- xs:decimal arithmetic (and xs:integer arithmetic in `ZORBA_WITH_BIG_INTEGER`
  builds) is now thread-safe without a global lock: MAPM's internal work
  buffers, caches and constants are per-thread, and the reference count of
  shared MAPM values is atomic. Concurrent queries in one process no longer
  serialize on, or race in, the arbitrary-precision library. ([#72](https://github.com/zorba-processor/zorba/pull/72))
- `xs:dateTime`/`xs:time` with more than 6 fractional-second digits are truncated to microseconds; `"23:59:59.999999999"` no longer prints as `59.1` with `seconds-from-dateTime` returning 60. ([#74](https://github.com/zorba-processor/zorba/pull/74))
- A negative `xs:dayTimeDuration` keeps its sign in `*` and `div` (`-PT1S * 1.5` gave `PT1.5S`, `-PT1S div PT1S` gave 1). ([#74](https://github.com/zorba-processor/zorba/pull/74))
- Multiplying or dividing a duration into a value too large to represent raises FODT0002 instead of the internal error ZXQP0003. ([#74](https://github.com/zorba-processor/zorba/pull/74))
- Casting to `xs:token` collapses whitespace instead of failing with ZXQP0003 on leading or trailing whitespace; casts to `xs:language`, `xs:NMTOKEN`, `xs:Name`, `xs:NCName`, `xs:ID`, `xs:IDREF` and `xs:ENTITY` collapse whitespace before validating (`" true " cast as xs:language`). ([#74](https://github.com/zorba-processor/zorba/pull/74))
- `fn:translate`: when a character occurs more than once in `$mapString`, the first occurrence wins (`translate('aaa', 'aa', 'bc')` is `bbb`). ([#74](https://github.com/zorba-processor/zorba/pull/74))
- `fn:upper-case` and `fn:lower-case` use the full Unicode case mappings (`upper-case('ß')` is `SS`; titlecase letters such as U+01C5 are mapped). ([#74](https://github.com/zorba-processor/zorba/pull/74))
- `fn:escape-html-uri` escapes all non-ASCII characters. ([#74](https://github.com/zorba-processor/zorba/pull/74))
- `fn:resolve-uri("a", "http://a")` returns `http://a/a` (RFC 3986 merge with an empty base path). ([#74](https://github.com/zorba-processor/zorba/pull/74))
- `fn:namespace-uri-for-prefix` and `fn:resolve-QName` know the `xml` prefix; `fn:namespace-uri-for-prefix` returns `xs:anyURI`. ([#74](https://github.com/zorba-processor/zorba/pull/74))
- `fn:round-half-to-even` on `xs:double`/`xs:float` with a precision beyond the value's last digit returns the value (`round-half-to-even(4.9e-324, 400)` gave 0). ([#74](https://github.com/zorba-processor/zorba/pull/74))
- `castable as xs:QName` returns false for an invalid string literal instead of raising FORG0001. ([#74](https://github.com/zorba-processor/zorba/pull/74))
- Casting a string without digits (whitespace only, `"+"`, `"."`) to `xs:decimal` raises FORG0001 instead of returning 0. ([#74](https://github.com/zorba-processor/zorba/pull/74))
- Casting to `xs:double`/`xs:float` rejects trailing characters and hexadecimal forms (`"0x1A"` gave 30). ([#74](https://github.com/zorba-processor/zorba/pull/74))
- `lt`, `gt`, `le`, `ge` and `<`, `>`, `<=`, `>=` on `xs:gYear`, `xs:gYearMonth`, `xs:gMonth`, `xs:gMonthDay` and `xs:gDay` raise XPTY0004. ([#74](https://github.com/zorba-processor/zorba/pull/74))
- `fn:max`/`fn:min` return a NaN of the promoted type (`max((0e0, xs:float('NaN')))` is an `xs:double`). ([#74](https://github.com/zorba-processor/zorba/pull/74))
- `fn:format-integer` formats -9223372036854775808, and ordinals of 21, 22, 23 (and so on) end in st, nd, rd. ([#74](https://github.com/zorba-processor/zorba/pull/74))
- Casting a negative decimal, float or double at or below -1 to an unsigned integer type raises FORG0001 instead of FOCA0002. ([#74](https://github.com/zorba-processor/zorba/pull/74))
- The `zorba` command applies serialization parameters declared in the query prolog (`declare option output:method "text"`); `-z` and the `--serialize-*` options still override them. ([#74](https://github.com/zorba-processor/zorba/pull/74))
- xs:float and xs:double are cast to strings with the shortest digit sequence that round-trips: `xs:float(1.999)` gave `2`, `xs:float(1.0000001)` gave `1`, and doubles lost or gained digits (`3.1415926535897931`). ([#76](https://github.com/zorba-processor/zorba/pull/76))
- `math:acos(-0)` returns +π/2 instead of -π/2; the math module's `asinh` and `atanh` keep the sign of zero. ([#76](https://github.com/zorba-processor/zorba/pull/76))
- Regular expressions: `^` outside a character class is always an anchor (`fn:matches("a", "^^")` is true); without the `m` flag `$` matches only at the end of the string wherever it appears; `[\s-]` and `[^-a]` are accepted. ([#76](https://github.com/zorba-processor/zorba/pull/76))
- Serialization no longer inserts a space between a text node and an adjacent atomic value. ([#76](https://github.com/zorba-processor/zorba/pull/76))
- `fn:unparsed-text` and `fn:unparsed-text-lines` raise FOUT1170 instead of FODC0005 for an invalid `$href`. ([#76](https://github.com/zorba-processor/zorba/pull/76))
- zq:delete-query no longer frees a query whose function items are still in use by the outer query (crash in the QT3 FOTS driver, inline-fn-032/-033). ([#76](https://github.com/zorba-processor/zorba/pull/76))
- Annotations on inline function expressions and function tests are parsed (XQuery 3.0); `%public`/`%private` on an inline function raise XQST0125. ([#76](https://github.com/zorba-processor/zorba/pull/76))
- URIQualifiedNames (`Q{uri}local`) are accepted in computed element and attribute constructors, as literal names and as computed name strings. ([#76](https://github.com/zorba-processor/zorba/pull/76))
- `fn:format-number` accepts a URIQualifiedName (`Q{uri}local`) as the decimal format name. ([#76](https://github.com/zorba-processor/zorba/pull/76))
- Regular expressions with the `q` flag: the whole pattern is literal, so a lone `\` is accepted, and `fn:replace` no longer raises FORX0003 for a literal pattern such as `a?` that would match the empty string as a regular expression. ([#76](https://github.com/zorba-processor/zorba/pull/76))
- Debugger client destructor deleted its input stream twice and leaked the output stream. ([#83](https://github.com/zorba-processor/zorba/pull/83))
- Regular expressions: a reluctant quantifier (`*?`, `+?`) on a group or back-reference that can match the empty string no longer loops until the time limit (QT3 fn-matches-50, `"(?:r?)*?r"`), also without ICU. `{`, `}`, `(` and `)` inside a character class are ordinary characters, `#` is an ordinary character with the `x` flag, whitespace inside `\p{...}` is ignored with the `x` flag, and `\NN` after fewer than NN groups is the back-reference `\N` followed by a digit. ([#86](https://github.com/zorba-processor/zorba/pull/86))
- `fn:format-date`, `fn:format-dateTime` and `fn:format-time`: `[Z]`, `[z]` and `[ZN]` output nothing for a value without a timezone (`[ZZ]` still gives `J`); `[ZN]` uses the timezone abbreviation for an Olson name or country code in `$place` (CDT for -05:00 in a US summer) and knows HST and IST. ([#86](https://github.com/zorba-processor/zorba/pull/86))
- `fn:format-number` formats `xs:decimal` and `xs:integer` values exactly instead of through `xs:double` (`format-number(xs:decimal("79228162514264337593543950335"), "0%")` gave `7922816251426434000000000000000%`), and an `xs:float` by its value (`format-number(xs:float("0.1"), "0.0#########")` is `0.1000000015`). ([#86](https://github.com/zorba-processor/zorba/pull/86))
- `fn:lower-case` maps a word-final capital sigma to the final sigma U+03C2 (Unicode Final_Sigma; needs ICU). ([#86](https://github.com/zorba-processor/zorba/pull/86))
- `StaticContext::invoke()` now initializes the global variables of modules imported with `StaticContext::loadProlog()` before calling the function, as a query compiled against the same static context does. Previously a function reading a module global failed or read an uninitialized value. ([#87](https://github.com/zorba-processor/zorba/pull/87))
- Debugger: breakpoints, stepping and eval never fired because the debug-mode wrapping of expressions was discarded by the translator. ([#88](https://github.com/zorba-processor/zorba/pull/88))
- Debugger: every eval of a variable failed with XPDY0002 because the context item was passed to the debugger eval under its old internal name. ([#88](https://github.com/zorba-processor/zorba/pull/88))
- Eval queries run inside a debugged query are no longer compiled in debug mode. ([#88](https://github.com/zorba-processor/zorba/pull/88))
- Executing a query compiled in debug mode without a debugger session no longer crashes. ([#88](https://github.com/zorba-processor/zorba/pull/88))
- Debugger client: `property_value` sent a `property_get` command. ([#88](https://github.com/zorba-processor/zorba/pull/88))
- Debugger server: error messages are XML-escaped in DBGP error responses. ([#88](https://github.com/zorba-processor/zorba/pull/88))
- The `debug_iter_serialization` unit test was never built or run. ([#88](https://github.com/zorba-processor/zorba/pull/88))
- Schema validation with the patched Xerces-C fork (`v3.3.0-zorba.4`, used by the Linux CI, builder and `zorba` runtime images): `NaN` no longer passes `minInclusive`/`maxInclusive` facets on `xs:double`/`xs:float` (XERCESC-2263), and `length`, `minLength` and `maxLength` count characters instead of UTF-16 code units, so a character outside the Basic Multilingual Plane counts as one (XERCESC-2063). Stock Xerces-C builds still have both bugs. ([#89](https://github.com/zorba-processor/zorba/pull/89))
- `zorba --print-ast` printed a global variable declaration as `<VarDecl>...</GlobalVarDecl>`, which is not well-formed XML; the opening tag is now `<GlobalVarDecl>`. ([#90](https://github.com/zorba-processor/zorba/pull/90))
- Schema import and validation no longer depend on the process locale: strings coming from Xerces-C (element, attribute and type names, facet values, schema locations, error messages) are converted to UTF-8 explicitly instead of through the locale code page, so non-ASCII text is no longer mangled under `LANG=C`/`LC_ALL=C`. ([#91](https://github.com/zorba-processor/zorba/pull/91))
- Fix a memory leak when streaming an `XMLChArray` and a write through a null buffer in the length-limited `StrX` constructor. ([#91](https://github.com/zorba-processor/zorba/pull/91))
- Regular expressions: a back-reference to a capturing group that did not participate in the match matches the empty string, and a group inside a repeated group participates only if it matched in the current iteration, as F&O 3.0 5.6.1 defines it (`matches("x", "(a)|\1")` and `matches("babadad", "^((.)?a\2)+$")` are true; QT3 fn-matches-50). This applies to `fn:matches`, `fn:replace` (`$N` for such a group is the empty string), `fn:tokenize` and `fn:analyze-string`. With ICU, patterns with a back-reference are matched by Zorba's own regular expression engine, whose matcher was rewritten as a backtracking matcher with these semantics; other patterns still use ICU. With the `i` flag, that engine matches characters by Unicode case folding (KELVIN SIGN matches `k`, LONG S matches `s`); without ICU, a character range matches only the character's simple case mappings. ([#92](https://github.com/zorba-processor/zorba/pull/92))
- Regular expressions: a back-reference to a group that contains other groups, such as `\1` in `^((a)(b))\1$`, no longer raises FORX0002. ([#92](https://github.com/zorba-processor/zorba/pull/92))
- Regular expressions without ICU: `^` in `fn:tokenize` and `fn:analyze-string` matches only at the start of the input (or of a line with the `m` flag), not at the start of each search; with the `x` flag, whitespace is removed everywhere outside character classes (also before quantifiers and inside `\p{...}`); `"\"` in the replacement string of `fn:replace` escapes the next character, so `$10` with one group is group 1 followed by `0`; a one-digit back-reference followed by a non-digit in a pattern with ten or more groups no longer hangs. ([#92](https://github.com/zorba-processor/zorba/pull/92))
- JSound: the `$pattern` facet of a type reports the XQuery regular expression as written (it gave the converted ICU pattern). ([#92](https://github.com/zorba-processor/zorba/pull/92))
- C++ API `fn::tokenize()` takes an XQuery regular expression, as documented (the pattern went to ICU unconverted). ([#92](https://github.com/zorba-processor/zorba/pull/92))
- Full-text stop-word sets are no longer leaked when a `using stop words at` URI cannot be resolved (FTST0008). ([#94](https://github.com/zorba-processor/zorba/pull/94))
- Resetting a user-defined function call reset each argument subplan twice: directly, and through the parameters of the callee. Calls nested in arguments do the same with their own arguments, so the work doubled at every level of nesting and recursion. The QT3 test app-Demos/raytracer took 105-158 s (over the 60 s per-test timeout) and now takes about 2 s. ([#95](https://github.com/zorba-processor/zorba/pull/95))
- xs:double and xs:float values hashed to their integer part, so every value in (-1, 1) had the same hash code and hash tables keyed on fractional numbers (the result cache of recursive functions, fn:distinct-values, grouping) degraded to linear lists. Non-integral numbers now hash on their value; values equal after numeric promotion still hash equal. ([#95](https://github.com/zorba-processor/zorba/pull/95))
- Hash xs:integer and xs:decimal values beyond 2^53 on the xs:double they are promoted to, so that values equal under value comparison (e.g. 9007199254740993 and 9.007199254740992E15) are recognized as duplicates by `fn:distinct-values`, `group by` and the hash tables built on item hashes. ([#100](https://github.com/zorba-processor/zorba/pull/100))
- `group by` with several grouping keys no longer treats two tuples as equal because their first keys are both NaN. ([#100](https://github.com/zorba-processor/zorba/pull/100))
- QT3 workflow: the report job no longer fails on single-shard runs, where the shard artifact is extracted without a per-shard directory. ([#101](https://github.com/zorba-processor/zorba/pull/101))
- Node ids whose last byte holds several short components (for example `1.2.0.0.0.1`) lost their trailing components when decoded, so repeated insertion of nodes at the same position by XQuery Update produced invalid ids after a few rounds. The stored id format is unchanged. ([#104](https://github.com/zorba-processor/zorba/pull/104))
- C++ API: a function item taken from a query result stays usable after the query is closed or destroyed. Invoking it with `StaticContext::invoke()` or passing it to another query used freed memory; the creating query's compiled functions and contexts are now released when the last such function item goes away (cycles through prolog variables holding function items, directly or inside JSON objects and arrays, are collected). The same applies to function items created by `reflection:eval` (which were freed when the eval was re-run), to function items referring to functions declared by `StaticContext::loadProlog()`, and to partially applied function items, whose closure could be freed with the original function item. ([#93](https://github.com/zorba-processor/zorba/pull/93))
- A function item reads the prolog variables of the query that created it, as XQuery 3.0 requires, also when it is invoked by another query; it read the invoking query's variables (wrong values or XPDY0002, including when the invoking query declared a variable with the same name). ([#93](https://github.com/zorba-processor/zorba/pull/93))
- `%an:cache` and `%an:strictlydeterministic` functions no longer cache calls whose arguments or results contain function items. ([#93](https://github.com/zorba-processor/zorba/pull/93))
- A clone of a query (`XQuery::clone()`) no longer uses freed memory when the original query is closed first. ([#93](https://github.com/zorba-processor/zorba/pull/93))
- The functions declared by `StaticContext::loadProlog()` no longer report compile-time warnings to freed memory. ([#93](https://github.com/zorba-processor/zorba/pull/93))
- Schema validation with the patched Xerces-C fork (`v3.3.0-zorba.5`, used by the Linux CI, builder and `zorba` runtime images): an `xs:choice` with an empty `xs:sequence` branch accepts empty content and each of the other branches (XERCESC-2243), and a union type no longer collapses whitespace in values of a whitespace-preserving member type; each member applies its own `whiteSpace` facet (XERCESC-2115). Stock Xerces-C builds still have both bugs. ([#105](https://github.com/zorba-processor/zorba/pull/105))
- The built-in thesaurus (`using thesaurus default`, WordNet data in the library path) no longer requires `Capability::NativeModules`; other `wordnet:` URIs still do, and a thesaurus at a file path still needs `FileRead`. ([#106](https://github.com/zorba-processor/zorba/pull/106))
- A JSound schema import denied by the security policy names the capability its `$location` needs, not the one for the namespace URI tried after it. ([#106](https://github.com/zorba-processor/zorba/pull/106))
- Fetch module errors (`zuri:URI_UNRESOLVED_OR_NOSTREAM`) are no longer wrapped in a second copy of the same error message. ([#106](https://github.com/zorba-processor/zorba/pull/106))

### Removed

- Remove the SWIG language bindings: the `swig/` directory, the `ZORBA_SUPPRESS_SWIG` option, SWIG/PHP/Python lookups in CMake, the per-language Doxygen projects and examples, and the matching CPack components and NOTICE entries ([#61](https://github.com/zorba-processor/zorba/pull/61)).
- Remove Windows support code paths ([#27](https://github.com/zorba-processor/zorba/pull/27)).
- Remove the `zorbautils/SAXParser` class and the `ZORBA_WITH_LIBXML2_SAX` option ([#51](https://github.com/zorba-processor/zorba/pull/51)).
- Remove 91 stale `EXPECTED_FAILURE` test entries ([#25](https://github.com/zorba-processor/zorba/pull/25)).
- Unused `WIN32.cpack`, `zorba_install.nsi`, the empty `CPack.cmake` stub and the stale `README.txt`. ([#78](https://github.com/zorba-processor/zorba/pull/78))
- Incomplete copy of the Node.js binding (`binding.gyp`, `package.json`, `example.js`, `include/QueryManager.h`, `include/NodeDiagnosticHandler.h`, `include/helpers.h`); its sources were never committed here and the full project lives at github.com/Rod-O/zorba-node. A Node.js binding over the new C API is planned for 5.x. ([#78](https://github.com/zorba-processor/zorba/pull/78))
- Dead text-validation-info member and accessors in ValidationEventHandler/EventSchemaValidator (never assigned or called since 2008). ([#85](https://github.com/zorba-processor/zorba/pull/85))
- Dead source files that were not part of any build target (old interactive `*_test.cpp` programs, the unfinished parse-tree DOT visitor, the pre-DBGP debugger client and event listener, `TupleItem`, `ResultIteratorChainer`, the unused MAPM mutex wrapper, the Windows Mobile project, 2007 design documents, obsolete scripts and the broken `check_links` target). ([#102](https://github.com/zorba-processor/zorba/pull/102))
- Windows-only build files and helpers: `cmake/modules/Windows/` (proxy find modules, DLL version resource), the Visual Studio project template, `scripts/win_exec.bat.in`, NSIS CPack options, the `.chm` documentation target, the bundled `modules/util-curl/cacert.pem`, and the MSVC/WinCE/Cygwin/MinGW branches of the CMake files. ([#109](https://github.com/zorba-processor/zorba/pull/109))
- Windows-only sources and branches: `src/diagnostics/StackWalker.{h,cpp}`, `src/zorbamisc/config/{win32,cygwin,compatib_defs,stdint}.h`, and every `WIN32`/`_WIN32`/`WINCE`/`CYGWIN`/`_MSC_VER` code path in `src/`, `include/`, `bin/`, `modules/` and the tests, including the Win32 `Latch`/`Condition`, `LoadLibrary` module loading and the Windows `fs_util` functions. ([#109](https://github.com/zorba-processor/zorba/pull/109))
- The `stdafx.h` includes (576 files) and the unused MSVC precompiled-header option `ZORBA_PRECOMPILED_HEADERS`; the real precompiled header (`ZORBA_USE_PCH`) is unchanged. ([#109](https://github.com/zorba-processor/zorba/pull/109))
- The Windows build page `doc/zorba/build_windows.dox` and the Windows notes in the manual. ([#109](https://github.com/zorba-processor/zorba/pull/109))

## Before 5.0

The entries below come from the former `ChangeLog` file and are kept verbatim.

Zorba - The XQuery Processor

version 3.1

New Features:
  * Added support for query profiling.
  * The json-xml module now supports the "object form" of JsonML.
  * Fixed bugs in the json-xml module.
  * Added public introspection API for external variable annotations.
  * Added non deterministic functions to the JDBC module.
  * Added introspection API for external variable annotations.
  * setVariable() now supports casting sequence elements.
  * zorba command now supports converting multiple values for the same variable into a sequence, e.g.: zorba -e x:=1 -e x:=2 -q 'declare variable $x as xs:integer+ external; $x'
  * The function caching mechanism of Zorba has been completely revamped. New annotations have been introduced for function caching.
    - `%an:nondeterministic`: Declares that the function is non-deterministic. Several calls with the same paramaters may
      return different results. The optimizer is not allowed to cache anything.
    - `%an:deterministic` (default): Declares that the function is deterministic and that several calls with the same paramaters always
      return the same results in the same snapshot, except that newly constructed nodes will have a different identity on each
      call. This is the default behavior from the W3C specifications. The optimizer is allowed to cache function results.
    - `%an:strictlydeterministic`: Declares and enforces that a function is deterministic. Several calls with the same paramaters always
      return the exact same results (also same identity) within a snapshot. The optimizer guarantees this by caching within a snapshot.
    - For a function that only has atomics in its signature, `%an:strictlydeterministic` and `%an:deterministic` are semantically
      equivalent. These functions are by default automatically cached by the optimizer.
    - `%an:cache` (use with care): forces caching across snapshots.
    - `%an:exclude-from-cache-key(n)` specifies that a function argument has to be ignored when computing the cache key.
    - `%an:compare-with-deep-equal(n)` specifies that a function argument has to be compared with deep-equal semantics.
  * New sleep module.
  * Substantial improvements to the http client module.
  * Added JSON Data Manager to expose JSON parsing through the C++ api.
  * Iterator tree printing as XML or JSON.
  * New FTP client.
  * New JSound validator.
  
Optimizations:
  * Improved JSON serialization performance.
  * Improvements in the lexer and parser.

Bug Fixes/Other Changes:
  * Fixed permission of files in the archive (better compatibility with archive extractors (exp. mac).
  * Fixed CSV parser bugs.
  * Fixed transform iterator.
  * Fixed many to many equality.
  * Fixed time comparison.
  * Fixed internal snapshot when using eval.
  * Fixed ref-counting assertions.
  * Fixed debug mode compilation and behaviour.
  * Fixed libxml error handling.
  * Fixed pattern validation.
  * Fixed castable implementation.
  * Fixed null validation in jsound.
  * Fixed function item partial application.
  * Fixed -DZORBA_NO_FULL_TEXT=ON -DZORBA_NO_XMLSCHEMA=ON build flags.
  * Fixed external function and udf profiling.
  * Fixed function cache hashing.
  * Fixed references in query plans.
  * Fixed db:edit and fn:parse-xml segfaults.
  * Fixed simple type cast.
  
version 3.0

New Naming:
  * All modules, annotations, options, collations, pragmas, and other
    namespace URIs changed from http://www.zorba-xquery.com/
    to http://zorba.io/ .

New Features:
  * The grouping variable in a group by clause is now optional.
  * JSONiq extension: New syntax for JSONiq array lookup: expr1[[expr2]].
  * JSONiq extension: jn:trim() and libjn:descendant-arrays().
  * JSONiq extension: Object/array navigation allows item()* as the type of the
    input sequence (doing implicit iteration over the input sequence and skipping
    items that are not objects/arrays).
  * JSONiq extension: Several of the jsoniq functions now allow item()* as the
    type of the input sequence (doing implicit iteration over the input sequence
    and skipping or simply propagating items that are not objects/arrays).
  * JSONiq extension: implicit iteration is also done for jsoniq delete, renames,
    and value replacements.
  * JSONiq extension: EBV on jsoniq items now returns true (instead of raising
    an error).
  * JSONiq extension: jn:analyze-string and jn:serializeFixed (#1192246).
  * New CSV-to-JSON converter module (#1189840).
  * Support XQuery 3.0 item-separator serialization parameter (#1217140).
  * Beta implementation of the JSound schema language (module http://jsound.io/modules/validate)

Optimizations:
  * Implemented hoisting optimization for general FLWOR.
  * Implemented flwor-merge optimization for general FLWOR.
  * Implemented document-ordering elimination for general FLWOR.
  * Applied count optimization to the return clause of general FLWORs.
  * Optimized implementation of fn:deep-equal.
  * No need to apply document ordering on the domain expression of a FOR clause
    if the FOR clause is followed by an orderby or groupby clause.
  * Replaced use of create_value_type() in runtime with getTypeCode().
  * Optimized switch expression.
  * Optimized implementation of function caching and removed the restriction
    on the return type.
  * No node ordering/distinct required for self axis.
  * More aggressive elimination of unused LET variables.
  * Improved performance of castable as expression (#1194720).

Bug Fixes/Other Changes:
  * Fixed bug #1117952 (Improve XML error output format).
  * Fixed bug #1188084 (fn-replace-42 failing).
  * Fixed bug #1189636 (transcoding hexBinary streambuf).
  * Fixed bug #1162631 (format-integer 'w' format of negative numbers).
  * Fixed bug #942171 (file module to allow for arbitrary encodings).
  * Fixed bug #1192285 (Have JSON token know number subtype).
  * Fixed bug #1210628 (file:last-modified returns wrong month).
  * Fixed bug #1190261 (relative paths bug in file module).
  * Fixed bug #1189798 (Update core module "errors").
  * Fixed bug #1187537 (Eliminate (or at least reduce) use of MAX_PATH).
  * Fixed bug #1058398 (set-based sequence operations).
  * Fixed bug #1180220 (Consolidate redundant path/file public APIs).
  * Fixed bug #1158052 (createBase64Binary() API too subtle).
  * Fixed bug #1103115 (Timezone units as hours are wrong).
  * Fixed bug #1188280 (casting xs:id to xs:ncname).
  * Fixed bug #1188100 (regex issues with 'q' flag in fn:tokenize).
  * Fixed invalid memory access error occuring during sequence type matching
    for user-defined types.
  * Fixed bug #1200090 (Convert json-xml module to use JSONiq items).
  * Fixed bug #938574 (changed the names and semantics of the NsScoping enum).
  * Fixed bug #1190710 (fn-format-date failures).
  * Fixed bug #1190407 (wrong rewrite of if-then-else return clause in case
    of general flwor).
  * Fixed bug #1188281 (casting to xs:anySimpleType not allowed).
  * Fixed bug #1189792 (Update core module "sctx").
  * Fixed bug #1188095 (do correct normalization for fn:string-length())).
  * Fixed bug #1189996 (Relocate some public API headers to util).
  * Fixed bug #1189790 (Update core module "full-text").
  * Fixed bug in hoisting through try-catch expr.
  * Fixed bug in casting to xs:NCName.
  * Replaced Base64 C++ API with a better one.
  * Replaced HexBinary C++ API with a better one.
  * Check that the namespace URI used in default namespace declaration is valid
  * Fixed bug in error conditions for computed PI constructor.
  * Fixed implementation of fn:deep-equal according to latest W3C spec.
  * Must apply document ordering on the domain expression of a FOR clause, if
    the FOR clause is followed by a sequential clause.
  * Selector value in object/array navigation is always cast to
    string/integer.
  * The function jn:is-null() has been removed.
  * Removed from libjn module the functions that existed in the jn module as well.
  * Renamed xqxq module to zorba-query module (zq module).

version 2.9

New Features:
  * A new common-language mode has been added to help write code that is 
    compatible with both Zorba's extended XQuery and JSONiq languages. It will 
    raise warnings for code that is not conformant to the common subset.
  * Typeswitch expression extended to allow union of types in each case clause,
    as specified by XQuery v3.0
  * In C++ API, added optional XML formatting of exceptions.
  * Bind zerr:line-number-end and zerr:column-number-end variables in the body
    of a catch clause.
  * New C++ and XQuery function (see module http://www.zorba-xquery.com/modules/item)
    to estimate the size an item allocates in memory.
  * Extended sequence types to include unions, as specified by XQuery v3.0
  * Added XQuery::parse(std::istream& aQuery, ModuleInfo_t& aResult) function.
  * In store API, added ability to specify a stream's originating URI (file)
    for streamable strings and base64Binary.
  * Added millis-to-dateTime() function in datetime module.
  * Added setCalendar(), getCalendar(), setLocale(), and getLocale() to
    DynamicContext class in C++ API.
  * fn:trace outputs "empty-sequence()" if the input is the empty-sequence.
  * Allow multiple default function namespaces.
  * Implemented computed namespace constructors and namespace-node() kindTest.
  * $array() => jn:members($array), $object() => jn:keys($object)
  * Added xqxq:variable-value function.
  * Added canonicalize function to modules/xml.
  * Added support for xs:dateTimeStamp type from XMLSchema 1.1, bug #924754.
  * Implemented resolution of module-import cycles according to the W3C XQuery 3.0 spec.
  * Added uri:parse() and uri:serialize() functions to URI module for
    parsing URI into components and forming URIs from components.
  * Added support for the standardized require-feature and prohibit-feature
    options.
  * Implemented higher order functions (function items, partial and dynamic
    function invocation, and related fn library functions).

Optimizations:
  * Extended optimization rules for subsequence function (pushing $startingLoc
    parameter into collection skip if $sourceSeq parameter is from a
    collection).
  * Introduced generic framework for optimizing fn:count() during runtime:
    Virtual PlanIterator::count() method provides the base (unoptimized)
    implementation of fn:count(). This base method is redefined by other plan
    iterators who have a way of returning the cardinality of their result
    sequence without actually computing the full result sequence.
  * Optimized fn:index-of function
  * Extented index join  rule to general flwor expressions. 

Bug Fixes/Other Changes:
  * Fixed bug #1176038 (memory error in parser revealed by Bison 2.7)
  * Fixed bug #1102997 (Swapping referenced nodes)
  * Fixed bug #1169908 (Zorba hangs with invalid utf-8 input)
  * Fixed bug #1111487 (Swapping nodes that are in collections)
  * Fixed bug #1083006 (subsequence: large input values)
  * Fixed bug #1123836 (overflows in date/time casts now return FODT0001 and
    in durations return FODT0002)
  * Fixed bug #1147518 (fn:round-half-to-even (at least 11 failures))
  * Fixed bug #1167704 (Implement [w] for ISO calendars for format-date/time functions)
  * Fixed bug #1114228 (unrecognized options in the XQuery namespace now raise an error)
  * Fixed bug #1124273 (xqdoc crash because of annotation literals)
  * Fixed bug #1175752 ("format-date-en152" and "format-dateTime-en152" failing)
  * Fixed bug #1085408 (xs:date(): casting large year values)
  * Fixed bug #867027 (XQST0059 error messages inconsistent)
  * Fixed fn:nilled function
  * Fixed bug concerning nillability during sequence type matching.
  * Fixed bug in handling the type declaration of a FOR variable that allows empty.
  * Fixed bug in casting to normalizedString.
  * Fixed bug #1095889 (Improve error message for xml-parsing error).
  * Fixed bug #1131984 (apparently invalid regex in queries)
  * Fixed bug #1123163 (fn:format-integer failures)
  * Fixed bug in index join rule (no index join if inner clause has positional var).
  * Fixed bug in index join rule (copy var ids after cloning index domain expr).
  * Fixed bug #1131988 (re00732 fails)
  * Fixed bug in handling the type declaration of a window variable.
  * Fixed bug #1131990 (re00737 fails)
  * Fixed bug #1123162 (FOTS: formatting dates and times failures)
  * Added missing wrapper expressions around some variable references.
  * Fixed optimizer bug: elimination of positional variable sometimes caused
    elimination of whole FOR clause.
  * Fixed bug in the throwing of error XQTY0086 during node construction.
  * Fixed bug #1148335 (where-clause expression was not always reset when it should be)
  * Fixed bug #1122396 (Associate origin/destination URI to I/O stream)
  * Fixed bug #1111786 (xml/json parse error location in catch clause).
  * Fixed bug #1153800 (construct hexBinary without encoding the data)
  * Fixed bug #1076402 (bug in CastIterator when target is QName)
  * NaN items are considered equal to each other during grouping
  * Fixed bug #855481 (Too small time types on Windows).
  * Fixed bug #1154984 (windows build crashes if a vector iterator is reused
    after it is used to erase an element of the vector)
  * Fixed bug #1132032 (Certain regexes involving ^ should be legal)
  * Fixed bug #1023168 (Non-single-char-escapes in regex character ranges not
    caught)
  * Throw XQST0048 for groupby clause, as specified by XQuery 3.0.
  * Fixed bug #1064289 (file module cannot delete broken symbolic links)
  * Fixed bug in raising XQTY0086 (copying of namespace-sensitive values during
    node construction).
  * Fixed bug #866874 (regex "range subtraction" not supported for ICU)
  * Fixed bug in computing the static type of an allowing-empty FOR variable.
  * Fixed bug #1099648 and #1088886 (XML parsing failures on Red Hat)
  * Fixed bug #1131985 (\p{IsPrivateUse} in regex may not be working)
  * Fixed bug #1099535  (xml:parse endless loop)
  * Fixed bug in optimizing fn:count over general-index probes
  * Fixed bug #866958  (Parsing error not explicit enough)
  * Fixed bug #1123164 (fn:format-number failures (at least 19))
  * Fixed memory leak in temp sequence
  * Fixed bug in computing the static type of a self-axis path step.
  * Fixed bug #867068: (Incorrect usage of XQDY0027)
  * Fixed bug: jsoniq constructor should not be const-folded
  * Fixed bug #866958 (Parsing error not explicit enough)
  * Fixed bug #1152834 (bug in computing the scripting kind of array and object
    constructor exprs)
  * Fixed bug #1125444 (input group-by exprs were not treated by index-flwor_vars()
    function, leading to erroneous loop hoisting).
  * Raise XQST0046, if needed, from namespace declaration attribute inside
    direct element constructor.
  * Fixed bug #1023362 (xsi:type attribute ignored during validation)
  * Fixed bug #1025564 (Deprecate -f argument to zorbacmd)
  * Fixed bug #1082740 (support for INF and -INF values in fn:subsequence function).
  * Fixed bug #1015468 (wrong result from fn:resolve-uri())


version 2.8

New Features:
  * New external module for interfacing with Couchbase Server.
  * New external module for interfacing with SQLite Database.
  * New external module for interfacing with Oracle NoSQL Database.
  * New external module for interfacing with relational databases via JDBC.
  * New external info-extraction module for querying concepts and entities in
    natural language text.
  * New built-in module to (de)reference XML nodes or JSON objects and
    arrays that are stored in a collection
  * Implemented JSONiq's json-doc() function.
  * http-uri-resolution feature to disable HTTP when resolving URIs.
  * Option to specify that parse-json() strips the top-level array.
  * Extended cast and castable expression to allow any simple target type
    (as specified by XQuery 3.0).
  * The XQXQ (XQuery-for-XQuery) module became part of the Zorba core.
  * The datetime module became part of the Zorba core.
  * In the datetime module, added parse-date(), parse-dateTime(), parse-time(),
    and utc-offset().
  * Includes test driver for W3C XQuery 3.0 Test Suite (aka FOTS).

Optimizations:
  * Various optimizations in the implementation of the optimizer rules.
  * Better computation of the static type for global variables.
  * Optimization of comparison operators when untypedAtomic items are involved.
  * Improved computation of static type for fn:data function.
  * Fixed iteration over the components of a window clause; this improves static
    type computation of expressions referencing window vars.
  * No need to cast xs:untypedAtomic to xs:string in order-by expression.
  * Convert LET variables whose domain sequence has exactly one item to FOR
    vars.

Bug Fixes/Other Changes:
  * Fixed bug #949064  (faulty QName should be printed in the error message)
  * Fixed bug #1072644 (broken parser error location)
  * Fixed bug #868640 (Node lifetime is shorter than expected due to let
    variable inlining)
  * Fixed bugs #1081626 (Segmentation fault in a side-effecting FLWOR),
    #1079488 (streaming segfault)
  * Fixed bug #1062093 (bug in var reference iterators with an associated
    position that is negative)
  * Fixed bug #1073978 (FOTS: option default ns not supported)
  * Fixed bug #1061222 (bad message for errors in index key type declaration)
  * Fixed bug #1065175 (preceding::node()[1] returns wrong results)
  * Fixed bug #1065321 (following:text() doesn't return results in doc order)
  * Fixed bug #1067706 (wrong const folding in mutually recursive udfs)
  * Fixed bug #1021492 (while computing the "sources" of a prolog var, skip any
    var-setting exprs that appear in non-used (and non-optimized) functions).
  * Fixed bug #1070551 (zerr:ZOSE0003 stream read failure)
  * Fixed bug #992304 (Compiling library modules could load data from wrong
    source).
  * Fixed bug #1043294 (XQST0036 not used in XQuery 3.0).
  * Fixed bug in castable expr when the target type is xs:QName.
  * Fixed bug in testing if a sequence type is subtype of node(xs:untyped)
  * Fixed mustCopyInputNodes() method of no-copy, and jsoniq functions.
  * Fixed bugs related to casting to user-defined types.


version 2.7

New Features:
  * XQuery 3.0 simple map operator http://www.w3.org/TR/xquery-30/#id-map-operator
  * Copy function added to the node module
  * Allow prolog variables to be referenced before they are declared (XQuery 3.0 feature)
  * Module http://www.jsoniq.org/modules/function-library with handy JSONiq functions
  * jn:parse-json can parse multiple top-level JSON items
  * New semantics of null for comparison and arithmetics operations
  * xml:parse (fragment parsing) now allows for a DOCTYPE declaration at the
    beginning of the XML document (requested in bug #1016606).
  * Roundtripping of JSON items has been moved from the serializer to encoding functions.
  * Added uuid to public API.
  * Best-effort serialization of atomic values in JSON items.
  * Positional pagination support for index probes
  * Recognize the {http://www.zorba-xquery.com/extensions}no-copy pragma to avoid
    copying nodes before insertion into a collection.
  * Added createUntypedAtomic to API's ItemFactory.
  * new xqDoc feature; reporting collections and indexes
  * new xqDoc function parameter to enable/disable xqDoc generation of certain
    components
  * Adding new external module read-pdf, it converts PDF documents to text or rendered images.

Optimizations:
  * Improved memory management for compiler expressions (no more ref counting)
  * Extended optimization rules (for variable inlining/elimination and positional
    predicate rewrite) to general flwor expressions.
  * Early optimization of positional predicates to subsequence expressions: now
    done at translation time, if possible.
  * For non-mutable global variables, assign a type based on their initializing expr.
  * Use char* instead of zstring as the hashmap key in json objects
  * Removed document-order sorting after the index value probe functions.
  * Optimize positional predicates involving inequality (convert them to subsequence)
  * Optimize positional predicate that is in CNF with other predicates.
  * Optimized runtime group iterator based on latest group-by semantics.
  * Removed two unnecessary steps from optimizer.

Bug Fixes/Other Changes:
  * Fixed bug #1006960 (taking construction mode into account during no-copy rule)
  * Fixed bug #950621 (Removed two-arg version of fn:parse-xml(); use XML module
    function xml:parse() instead)
  * Fixed bug #867227 (Improved error message for missing commas)
  * Fixed memory leak during runtime group-by"
  * Fixed bug #1024033 and #1023170 (segfaults in parse-xml:parse())
  * Fixed bug #898792 (Dynamically computed strings can now be cast to xs:QName)
  * Fixed bug in determining the IsId property of constructed attribute nodes
  * Fixed bug #1034990 (text serialization with jsoniq fails)
  * Fixed bugs #899364 and 899363 (throw XQST0103 in case of non-distinct window
    variables)
  * Fixed bug #923686 (implicitly declare global context size and context position
    variables in main module and provide api function to assign values to these
    variables)
  * Fixed bug #899366 (enforce the type declaration of a window variable)
  * Fixed bug #1034942 (improper variable inlining inside general flwor expr)
  * Fixed bug #1024892 (index declaration references udf declared after the index)
  * Fixed bug #1038410 (Memory leaks in parser, trace iterator, and general index)
  * Fixed bug #1046411 (automatic boxing of multi-item sequence appearing as the
    value of an object pair)
  * Fixed bug #1046410 (automatic conversion to null of empty sequence appearing
    as the value of an object pair)
  * Fixed bug #1039488 (inserting more than one pair at once in a JSON object)
  * Fixed bug #1042840 (QName pool free-list corruption)
  * Fixed bug #866984 (better error message for an eval error)
  * Fixed bug #1047538 (SourceFinder::findLocalNodeSources missing json expressions)
  * Fixed bug #932374 (FOTS fn:matches failing tests)
  * Fixed #1052383 (accessing context item in eval expr)
  * Fixed bug #1046559 (dynamic resolution between function invocation and jsoniq
    navigation)
  * Fixed bug #932884 (HTML and XHTML serialization of empty elements)
  * Fixed bug #1046415 (pair name cast to string)
  * Fixed bug #1045902 (file:last-modified returns current date time)
  * Fixed bug #1043976 (compiler warning in serializer code)
  * Fixed bug #866994 (a better error message when collection iterator not open)
  * Fixed bug #1039254 (allow empty sequence as position value in positional LET
    var iterator)


version 2.6

New Features:
  * Include first-class support for JSON data by implementing the JSONiq spec
  - see http://jsoniq.org/ for more information. This feature is enabled by default.
  * Implemented the new EQName syntax (use Q{namespace}local instead of "namespace":local)
  * New function Item::isSeekable for streamable content (xs:string and xs:base64Binary)
  * Implemented the latest W3C specification for the group by clause (backwards incompatible)
  * New ft:tokenize-nodes() function to full-text module
  * New XQuery 3.0 function fn:parse-xml-fragment#1
  * New ItemFactory function that allows creating dateTime items without timezone
  * Added support for transient maps to the
    http://www.zorba-xquery.com/modules/store/data-structures/unordered-map module.
  * Added support for fragments to fn:path
  * Positional pagination support for collections
  * Pagination by reference support for collections
  * http://www.zorba-xquery.com/modules/archive module for creating,
    reading, and updating various (compressed) archives (e.g. zip, tar.gz,
    or tar.bz2)
  * Incremental maintenance for general indexes
  * New function in the http://www.zorba-xquery.com/modules/store/static/indexes/dml
    module which returns the value of all keys contained in an index

Optimizations:
  * Optimization of comparison operations
  * Tighter hoisting of expressions (also fixes bug #967428)
  * Optimized hash sets used by fn:distinct-values and nodes-distinct
  * Optimized hash function used for nodes (also fixes bug #1010051)
  * Rewrite of plan serializer internals, resulting in big performance improvement
  * Avoid treat expr for checking that the value of a non-external
    global variable conforms to the type declaration of the variable
  * Streaming execution for tumbling windows (also fixes bug #1010051)

Bug Fixes/Other Changes:
  * Fixed bug #1025622 (Incorrect JSON serialization of supplementary plane code points)
  * Fixed bug #1016606 (DOCTYPE in the input of the parse-fragment function)
  * Fixed bug #1002993 (bug during revalidation after update; improper condition
    for calling TypeOps::get_atomic_type_code() from
    SchemaValidatorImpl::isPossibleSimpleContentRevalImpl())
  * Fixed bug #867357 (Improved parser error messages)
  * Fixed bug #1026379 ("q" flag for fn:replace() is ignored)
  * Fixed bug #932314 (non-comparable values must be treated as distinct by
    fn:distinct-values)
  * Fixed bug #1015580 (Add base64_streambuf / replace inefficient base64 code)
  * Fixed bug #1022557 (subsequence function applied on window variable)
  * Fixed bug #9910884 (raise XUST0001 in trycatch with mixed updating and simple clauses)
  * Fixed bug #854506 (ugly type error messages) and partial fix for bug #867008
  * Fixed bug #1008082 (bug in transform expr when a copy var is not used anywhere)
  * Fixed bug #898066 (Stringstream & fn:trace)
  * Fixed numbering of items in fn:trace starts with one
  * Fixed bug #1003023$ (optimizer problems due to common subexpression after
    var folding into if-then-else)
  * Fixed bug #1006166 (disabling 2 functions with the same qname)
  * Fixed bug #960083 (improper error handling of NaN comparisons)
  * Fixed bug #878508 (JsonML serialization not escaping characters)
  * The XML plan serialization format is not supported anymore


version 2.5

New Features:
  * New XQuery 3.0 functions
    - fn:available-environment-variables
    - fn:environment-variables
    - fn:uri-collection
    - fn:unparsed-text
    - fn:unparsed-text-available
    - fn:has-children#0
    - fn:nilled#0
    - fn:path
  * Added base64:decode#2 function which also does transcoding
  * Extended API for Python, Java, PHP and Ruby
  * Added JVM classpath to zorbacmd and to Zorba API
  * Added full-text module
  * Added support for NO_ICU (to not use ICU for unicode processing)
  * Added XQJ support
  * Added CollectionManager and DocumentManager support for XQJ
  * Added new API EntityKind SOME_CONTENT
  * Added new API StreamResource which is able to specify whether a
    stream is seekable or not.
  * Added new functions in the fetch module and the StaticContext to fetch
    resources as binary
  * Added non-core schema-tools module for getting sample XMLSchema from
    XML instances and sample XML instances from XMLSchema.

Optimizations:
  * optimized insertion into a collection (don't copy it if the node was
    created by an element constructor and is not used anywhere else in the query)
  * Fixed performance problem with the findNodeSources function of the no-copy rule

Bug Fixes/Other Changes:
  * Fixed bug #867170 (Variables declared in Prolog are Overwritten inside the query)
  * Fixed bug #1001463 (type not available during computation of function caching)
  * Fixed bug #1002867 (resulting base64 in http-client is wrong)
  * Fixed bugs #905028 (Allow to set base URI to undefined)
  * Fixed bugs #931501 and #866987 (improved error messages for fn:format-number()
  * fn:format-number raises FODF1310 instead of XTDE1310
  * Fixed bug 955170 (Catch clause with URILiteral-based wilcard NameTest)
  * Fixed bug #862971 (no error upon duplicate function declarations)
  * Fixed bug 955135 (err:XQDY0044 not caught by try-catch expressions)
  * Fixed bug #986075 (encountering flwor expr with no clauses)
  * Fixed bug #967864 (var substitution did not update theFreeVars property)
  * Fixed bug #891650 (context size var not always declared within path expr)
  * Fixed bug #948879 (--uri-path doesn't work with fetch:content())
  * Fixed bug in MarkNodeCopyProps rule (static cast to replace_expr without chaing the expr kind first)
  * Fixed bug in window iterator (binding the end vars in the output tuple stream)
  * Fixed bug #866547 (protect index-join rule from general flwor)
  * Fixed bug #867253 (cdml:delete-nodes should only remove root nodes)
  * Fixed bug #967428 (do not hoist index creation outside a try-catch)
  * Fixed bug #872234 (prevent a rewritting to take place in case of sequential expr)
  * Fixed bug #966706 (key uniqueness of index not enforced during incremental refresh)
  * Fixed bug #906494 (default compile with D_FILE_OFFSET_BITS=64)
  * Fixed bug #988412 (date:current-dateTime daylight saving)
  * Fixed bug #912586, #912593 and #912722 (assertion failures with lax validation)
  * Fixed bug #986377 (do not apply any updates on collection if it is to be truncated)
  * Fixed bug #921458 (file:read-text-lines() blocking)
  * Fixed bug #981405 (do not hoist expr containing try-catch variables out of the associated try-catch expression)
  * Fixed bug #996084 (crash in Streamable*Item with file module)
  * Fixed bug #947627 (throw XQST0099 if more than one declarations of context item type in same module)
  * Fixed bug #980526 (no-copy rule bug due to global var being set in "distant" udf)
  * Fixed bug #949910 (has-children may be invoked on all nodes)
  * Fixed Bug #933490 (Error ItemFactoryImpl::createBase64Binary with istream)
  * Fixed bug #867112 (Diagnostic Handler was not working on external APIs)
  * Fixed bug #857842 (Assertion failed with simple content element with comments)
  * Fixed bug #928626 (no setting classpath in xsl-fo module)

version 2.2

New Features:
  * Add --compile-plan (-c) and --execute-plan (-e) options to zorba command line.
  * Add XQuery 3.0 concat operator (http://www.w3.org/TR/xquery-30/#id-string-concat-expr)
  * New URI module providing percent-decoding functions.
  * The core parse-xml module is now able to parse input files in a streaming mode. A number of new options have also been added.
  * LibXml2 version 2.7.0 or higher is now required in order to build Zorba.
  * Added index management function to the C++ api's StaticCollectionManager.
  * Added createDayTimeDuration, createYearMonthDuration, createDocumentNode, createCommentNode, createPiNode to api's ItemFactory.
  * Fixed bug #905050 (setting and getting the context item type via the c++ api).
  * Added split function to the string module that allows for streamable tokenization but doesn't have regular expression
  * Add new XQuery interface for the PHP bindings.
  * Added two API methods StaticContext::getFunctions to introspect the functions of a static context
  * Added API method Item::getNamespaceBindings().
  * Added a transcoding streambuffer to the API which allows transcoding arbitrary encodings
    from and to UTF-8
  * file:read-text is able to handle arbitrary encodings (fixes bug #867159)
  * http:send-request is able to handle arbitrary encodings
  * Added API method StaticContext::getNamespaceBindings() (see bug #905035)
  * New version of the JSON converter module in the core of Zorba (no external dependencies anymore)
  * Added truncate function to the collection modules for efficient deletion of all nodes in a collection.


Optimization:
  * No-copy optimization: avoids copying nodes during node-constructor expressions.
  * Added external function annotations %ann:propagates-input-nodes and
    %ann:must-copy-input-nodes to be used by the no-copy optimization.
  * Caching of results for recursive functions with atomic parameter and return types.
  * Added %ann:cache and %ann:no-cache to enable or disable caching of results of functions with atomic parameter and return types.
  * Types-related optimizations for runtime operators (comparisons, FnBoolean, Or, And,
    Compare, instance-of, cast, castable, treat, promote).
  * More efficient implementation for base64Binary items
  * Preallocate and reuse temp sequence iterator for LetVarITerator
    and CtxVarIterator.
  * Enhanced push-down of positional predicate into LetVarITerator
    and CtxVarIterator.
  * Change the implementation of the free-vars annotation and got rid
    of the annotations map in expressions.

Bug Fixes/Other Changes:
  * Fixed bug 923015 (clone() not implemented for full-text expressions)
  * Fixed bug 917923 (bug in copying outer var values into the eval dynamic context)
  * Fixed bug 867509 (Can not handle largest xs:unsignedLong values)
  * Fixed bug 924063 (sentence is incorrectly incremented when token characters end without sentence terminator)
  * Fixed bug 909126 (bug in cloning of var_expr)
  * Fixed bug 928631 (external builtin function were not executed in the module they
    were declared)
  * Fixed bug in destruction of exit_catcher_expr
  * Fixed bug #867024 (error messages)
  * Fixed bug #957580 (stream read failure in StringToCodepointsIteartor)
  * Fixed bug #911585 (management of variables during eval)
  * Fixed bug #866423 (fn:empty and fn:exists iterators must reset their input in
    case of early-out)
  * Fixed bug #955574 (http-client "hangs" if following a POST request)
  * Fixed bug #945241 (StaticCollectionManager::declaredIndexes() and temporary indexes)
  * Fixed bug #872288 (reset recursive flag during node rename)
  * Fixed bug #905041 (allow for the default element and function namespaces to be
    set multiple times via the c++ api).
  * Fixed bug #907872 (segfault when returning an input ItemSequence from an external function).
  * Fixed bug in codegen: do not create a materialize clause if one exists already
    support.
  * Fixed bug involving positional var and groupby
  * Fixed bug #867266 (do not hoist updating expressions)
  * Fixed bug #867022 (added location and params to some XPTY0004 errors)
  * zerr is not predeclared anymore to be http://www.zorba-xquery.com/errors
  * Fixed bug #917981 (disallow declaring same module twice).
  * Deprecated StaticContext:getNamespaceURIByPrefix()
  * Fixed bug #943788 (streamable string is not seekable)
  * Fixed bug #921624 (slow queries with bogus example.com schema URIs)
  * Fixed bug #918211 (xqueryx fulltext w3c conformance generation)
  * Fixed bug #918157 (Add XQFTTS to validation queue)
  * Fixed bug with unversioned modules with C++ external functions
  * Fixed bug #912579 (validate-in-place on non-root elements)
  * Fixed bug #891209 (validation with xsi:type returns untyped)
  * Fixed bug #952829 (Nondeterministic annotation doesn't seem to be checked)
  * Fixed bug #951772 (Comment node crashes serialization)
  * Fixed bug #945241 (StaticCollectionManager::declaredIndexes() and temporary indexes)


version 2.1

New Features:
  * New node-position module. This module allows to obtain a representation of a node position, which
    can be used to assess structural relationships with other nodes.
  * New node-reference module. References can be obtained for any node, and
       different nodes cannot have the same identifier.
  * Custom Full-text thesaurus using Zorba URI resolver mechanism.
  * Modified C++ API to add isSequential, getExternalVariables,
       isBoundContextItem and isBoundExternalVariable Functions
  * Collection allows to retrieve information about the type
  and the annotations of a static collection.
  * Implemented the probe-index-range-value for general indexes
  * Added ExternalFunctionParameter class to be registered in the DynamicContext
  * New module for compiling and executing XQueries from XQuery (xqxq)
  * External functions are allowed to be updating (i.e. return a pending update list)

Optimization:
  * Optimization for positioned sequence access (e.g. $x[$x[1]] where $x is a
  sequence of integers)
  * Optimization for count(collection()) expressions
  * Optimization of the fn:substring function in the case when its $start and $length arguments
  are integers

Bug Fixes/Other Changes:
  * Fixed bug #3403291 (build for Fedora 15)
  * Fixed bug #3397293 (Incorrect XQueryX error message)
  * Fixed bug #3398637 (Nested element constructors cause loss of namespace bindings)
  * Fixed bug #3401592 (Undo of Create Collection doesn't work)
  * Fixed bug #3404689 (assignment problem in flwor)
  * Fix for bug #3404450 (testdriver_mt problem: caching and validation of docs)
  * Fixed installation bug to not install the .so for test modules
  * Fixed bug #3390410 (Adding a node to a collection causes out of bound in vector)
  * Fixed bug #3406200 (node types were not being moved to new tree during node detach)
  * Fixed bug #3406191 (Namespace bindings lost after update due to bug in node detach)
  * Fixed bug #3290122 (ZDST0003 needs collection name)
  * Fixed bug #3406272 (merging of adjacent text nodes must be done after all
    updates have been applied).
  * Don't raise warnings for unknown annotations.
  * Fixed bug #3409344 (during detach, if the node being detached is the root of
    its tree, it should be detached from the tree as well; otherwise memory
    corruption will occur)
  * Fixed bug # (Should not destroy the parent of a node that is being detached
    before the detach is done).
  * Added const qualifier to StaticContext::getAuditEvent()
  * Fixed bug #3408181 (available-collection() returns undeclared collections)
  * Fixed bug #859465 (Fatal error if a PUL contains two deactivate IC primitives)
  * Fixed bug #859467 (Fatal error if a PUL contains two activate Foreign Key primitives)
  * Fixed bug #859468 (Fatal error if a PUL contains two activate Simple IC primitives)
  * Fixed bug #859522 (Fatal error if a PUL contains two delete document primitives)
  * Fixed bug #859968 (Using put on an attribute produces an invalid XDM instance and may cause segmentation faults)
  * Fixed bug #855314 (should not fold a constant expr that returns an error item)
  * Fixed bug when parsing a document with a base-uri attribute.
  * Fixed bug #863320 (Sentence is incorrectly incremented when token characters end without sentence terminator)
  * Fixed bug #863730 (static delete-node* functions don't raise ZDDY0012)
  * Implemented the probe-index-range-value for general indexes
  * Removed ZSTR0005 and ZSTR0006 error codes
  * Fixed bug #867662 ("nullptr" warning)
  * Fixed bug #868258 (Assertion failure with two delete collection)
  * Fixed bug #871623 and #871629 (assertion failures with insertions in dynamic collections)
  * Fixed bug #867262 (allow reuse of iterator over ExtFuncArgItemSequence)
  * Fixed bug #869024 (segmentation fault with node-reference)
  * Fixed bug #869025 (segmentation fault with node-reference)
  * New node-reference module. References can be obtained for any node, and
      different nodes cannot have the same identifier.
  * Fixed bug #872697  (segmentation fault with validation of NMTOKENS)
  * General index cannot be declared as unique if the type of its key is
    xs:anyAtomicType or xs:untypedAtomic.
  * Added undo for node revalidation
  * Optimization for count(collection()) expressions
  * Fixed bug #872796  (validate-in-place can interfere with other update primitives)
  * Fixed bug #872799 (validate-in-place can set incorrect types)
  * Fixed bug #855715 (Invalid escaped characters in regex not caught)
  * Fixed bug #868325 (fn:analyze-string fails with some recursive subgroups)
  * Fixed bug #872697 (segmentation fault with validation of NMTOKENS)
  * General index cannot be declared as unique if the type of its key is
  xs:anyAtomicType or xs:untypedAtomic.
  * Added undo for node revalidation
  * Optimization for count(collection()) expressions
  * Fixed bug #867133 (SWIG PHP build failure on Mac OSX)
  * Fixed bug #872796 (validate-in-place can interfere with other update primitives)
  * Fixed bug #872799 (validate-in-place can set incorrect types)
  * Fixed bug #872850 (Serialization of Tumbling Window For fails)
  * Fixed bug #855715 (Invalid escaped characters in regex not caught)
  * Fixed bug #862089 (Split binary/xq install directories for modules) by
  splitting "module path" into separate URI and Library paths
  * New node-position module. This module allows to obtain a representation of a node position, which
    can be used to assess structural relationships with other nodes.
  * Fixed bug #872502 (validation of the JSON module xqdoc fails)
  * Fixed bug #897619 (testdriver_mt can not run the XQueryX tests)
  * Fixed bug #867107 (xqdoc dependency to zorba is wrong)
  * Fixed bug #897616 (Fix the generation of the XQFTTS report results for
  W3C)

version 2.0.3
  * Fixed bug #867662 ("nullptr" warning)
  * documentation improvements

version 2.0.2
  * Fixed bug #3401971 (node-by-reference on dynamic collections)
  * documentation improvements
  * installer improvements
  * Fixed bug #3403291 (build for Fedora 15)
  * Don't install the .so for test modules
  * Fix for running ctest on Windows

version 2.0.1

  * Custom Full-text stemming.
  * Custom Full-text tokenization.
  * Fixed scripting bug where apply statement was considered a sequential expr even if the source expr was non-updating and non-sequential.
  * Fixed bug #3394400 (Element replacement, really unexpected result)
  * Fixed bug #3394588 (Unexpected error during pul undo)
  * Fixed bugs #3398511 and #3398552 with "make install"
  * Fixed bug #3398521 for compiling with ZORBA_NO_XMLSCHEMA

version 2.0
  * Zorba 2.0 is a major rewrite of previous versions. It doesn't claim to
    preserve backwards compatibility with any 1.X version.
    Host language API changes have been made to the following components:
    - External functions API
    - XmlDataManager
    - Error reporting
    - Debugger API
    XQuery language and API changes have been made to the following components:
    - XQuery Scripting Extension
    - Function APIs
    - XQuery Data Definition Facility
    - XQuery modules and module versioning
    - Changes to conform with the latest XQuery 3.0
    - Namespaces of annotations, errors, collations.
  * In addition to host language API and XQuery language improvements,
    plenty of bug fixes and optimizations have been made.

version 1.4
  * added a datetime library module with nondeterministic functions
  * new nondeterministic read functions in the http-client module
  * the http-client's send-request function accepts a non-validated request parameter
  * fixed an issue with a wrong static context being used in eval queries (see bug item #2979317)
  * added XQuery 1.1 Switch expressions (only available in 1.1 processing mode)
  * added the fn:parse function from XQuery Functions and Operators 1.1
  * improved error reporting for parser errors
  * improved error reporting for errors regarding sequential expressions
  * bug fixes
    - accept relative URIs in xs:anyURI constructors (#2988209)
    - direct element/attribute construction (#2999765)
    - deep-equal (#2999772)
    - schema-attribute(*) is parsed as a function call instead of a node test (#2993286)
    - handling xml:base attributes during doc loading (#3013096)
    - fixed build with CMake 2.6.0 on Debian Lenny
    - try-catch error if XQuery version < 1.1 (#3005344)
    - W3C test suite bugs CastableAs649 and CastableAs650 (#2999777)
    - assertion in StatelessExtFunctionCallIterator (#3012651)
    - http-client return type handling for (*+xml and text/html)
  * optimization of fn:subsequence
  * new xqDoc function (xqdoc-content) taking a documented module as string
  * new xqDoc feature; reporting functions invoked from within a particular function
  * xqDoc schema change; name element for functions got an arity attribute
  * added a tidy library module that provides functions to tidy an HTML string
    and returns a valid XHTML document node (http://www.zorba-xquery.com/modules/tidy)
  * added a schema library module that provides XML schema related functions (e.g. schema-type)
    (http://www.zorba-xquery.com/modules/schema)
  * added a node reference library module that provides functions to retrieve the URI
    of a node and vice versa (http://www.zorba-xquery.com/modules/reference)
  * added a hmac module (http://www.zorba-xquery.com/modules/security/hmac)
    that provides an sha1 function (e.g. used in OAuth)
  * added a random module (http://www.zorba-xquery.com/modules/random)
    that provides fuctions for generating (pseudo-)random integers and UUIDs.

version 1.2
  * more introspection functions (e.g. in-scope-element and in-scope-attribute)
  * first prototype for higher order functions from XQuery 1.1
  * JSON module in (com/zorba-xquery/www/modules/json)
  * huge performance improvements
  * added an Atom schema to the build-in schemas
  * register a callback function in the static context that is called
    if a collection or index declaration is processed (compiled)
  * nondeterministic (instead of sequential) functions in the file module
  * reference counting the store (de)initialization
  * many bug fixes; for example
    - compile with Xerces 3.1
    - loading external functions (prefix problem)
    - plan serializer fixes
    - serialization fixes
    - base64 decoder
    - loading documents with general entities
    - EXPATH http-client fixes
    - not hoisting nondeterministic functions
    - scripting related fixes
    - unicode related fixes (e.g. for parameters to the doc function)
    - StaticContext::loadProlog fix
    - SAX serialization fix

version 1.0
  * Changes in the C++ API
    * Removed ResultIterator class. All public API methods use simply
      Iterator.
    * Added a new class named ExternalModule.
      This class must be used for registering external functions.
      This change makes the external function API incompatible with
      previous releases.
    * Loading pre-compiled queries (using loadExecutionPlan) requires to pass
      a SerializationCallback which is used for
      retrieving registered uri resolvers or trace streams.
      Moreover, the callback is used for retrieving ExternalModules
      which have been used when compiling the query.
    * Changed the append function in zorba::String to const. Now, the
      function does not change the String itself, but returns a new
      String. Furgthermore zorba::String::append can now handle other
      zorba::String objects - not only const char*. Also the operators
      + and += are defined on zorba::String.
  * Other language bindings (Java, PHP, python, ruby) also now use
    simply Iterator; ResultIterator removed.
  * XQC (C language binding) brought up-to-date with latest revision
    of that spec (https://xqc.sourceforge.net).
  * Resolving of modules is now enhanced by the so called "Zorba
    Module Path".  Moreover, multiple ModuleURIResolvers can be
    registered in one StaticContext.  See the documentation for more
    information.
  * Added static and dynamic context introspection modules.
  * Added the excel library to the built-in modules.
    Implemented the most used excel functions.
  * Added the math library to the built-in modules (the namespace has
    changed to http://www.zorba-xquery.com/modules/math)
  * Pass the Static- and DynamicContext as parameter to the evaluate function
    of external functions.
  * The XQuery class got a setTimeout option that allows the user to set a
    maximum running time of a particular query execution.
  * The StatelessExternalFunction is now non-pure by default. There is
    a new subclass called PureStatelessExternalFunction which should
    be used instead. PureStatelessExternalFunctions don't have access
    to the Static- and DynamicContext.
  * Renamed XQuery::serialize to XQuery::execute. The new method also
    implements the functionality of the XQuery::applyUpdates method
    (which was removed).
  * Added the serialize library to the built-in modules. The function
    util:serialize-to-string has been removed.
  * The DynamicContext can be used to pass arbitrary name-value pairs
    to the evaluate method of external functions.
  * Implemented CSV to XML converters. See csv2xml.xq module.
    You can convert back and forth between CSV files and XML
    or formated text files and XML.
  * Changes in the email functionnality
    * implemented smtp:send functions as external functions
    * added support for MIME messages (Text in character sets other
      than ASCII, Non-text attachments, Message bodies with multiple
      parts, Header information in non-ASCII character sets)
  * most of the runtime iterators and functions are generated
    (see https://sourceforge.net/apps/mediawiki/zorba/index.php?title=Creating_New_Runtime_Iterators_and_Functions)
  * improved exception messages raised by the serializer
  * Json is now able to parse Unicode messages
  * improved functionality for java, php, python and ruby bindings.

version 0.9.8-rev1 (2009.09.22)
  * bug fixes
    * fixed a crash caused by converting long integers (> 1024 digits)
      to string
    * the compile-only check reports the URI of the encapsulating
      entity in it's error report (used for XQDT semantic checking)
    * fixed 'for $x in () return $x/test' to return the empty sequence
      instead of raising a static error
    * fixed build using Xerces-C 3.0.1
    * fixed build for Mac OS X 10.6 (Snow Leopard)
    * improved error reporting in the command line utility
    * improved detection of the installed version of Xerces-C (raise
      an error if a wrong version is installed)

version 0.9.8 (2009.08.26)
  * More XQuery Scripting Features
    * default initialization for external variables
    * block local variables
  * XQuery 1.1
    * Support for the "validation as TypeName" feature
    * Changed the try/catch syntax as specified in XQuery 1.1 (except
      the try- and catch expressions are ExprSingle and not Expr).
  * Examples for the Java API
  * Function library
  * creating PDFs using XSL Formatting Objects (requires Java)
  * util:parse function
  * converter functions to/from JSON or JsonML and XML
  * function for sending simple text emails
  * the util:tidy function returns a node instead of a string.
  * XQDoc module http://www.zorba-xquery.com/zorba/xqdoc-functions
  * Added a loadProlog function to the static context.
  * Added support for XQueryX and XQueryX Update
  * Added compiled query persistence to the C++ API
    (XQuery::loadExecutionPlan() and XQuery::saveExecutionPlan())
  * Added a printPlan function to the XQuery class of the API.
  * Added new option in the Zorba command line utility: compile-only,
    parse-only, lib-module, print-errors-as-xml.
  * Added two plan printing function (printPlanAsXML and
    printPlanAsDOT) to the SWIG generated APIs.
  * Implemented 'declare option': options are set in the static context.
  * fn:trace can be disabled by setting the option
    QName("http://www.zorba-xquery.org/option", "trace") to "disable".
  * Changed the scripting syntax of the exit expression from 'exit
    with' to 'exit returning'.This is due to an ambiguity in the
    grammar (see http://www.w3.org/Bugs/Public/show_bug.cgi?id=6852
    for more information).
  * Added accessor functions for node items to the Item class in the
    API (i.e. getChildren, getAttributes, and getNodeName).
  * Added a UserException to the API which is raised if the user makes
    a call to the fn:error function.
  * UTF8 support for the lexer
  * bug fixes and performance improvements

version 0.9.5 (2008.02.17)
  * Unified the Tidy and Rest API's
  * Added new sinatures to zorba-util:tidy and zorba-util:tdoc that
    allow setting of tidy functions
  * Added Schema imports using target namespace or location hints
  * Added StaticContext::findFunctions and StaticContext::disableFunciton
  * Added Functions for collections
  * Windows packages include a zorba_simplestore.dll
  * Improved error messages for regular expression processing
  * Added functions to return an item instead of xs:string
  * Added a new signature to zorba-util:random in order to be able to call
    random within a loop and get different numbers each time.
  * Corrected the types of some functions/operators (YM_DURATION or
    DT_DURATION instead of DURATION).
  * Now possible to pass empty sequence as collection URI.
  * Now cURL is set to follow an infinite number of redirects.
  * Added validate after update
  * Fixed schema files without target namespace.
  * Fixed notation cast.
  * Added list and union item types to type manager cache
  * Added validation tests for schema substitution groups
  * Added message when error during update test driver.
  * Added partial support for scripting (XQSE)
  * Added support "default element namespace" schema imports
  * Added support variable initializers that refer to forward variable
    declarations.
  * Various bug fixes and performance enhancements.

version 0.9.4 (2008.10.06)
  * added General FLWOR (GFLWOR)
    * added Windowing functionality
    * added ability to perform "outer" joins in the FLWOR with outer
      for clauses
    * output numbering added to FLWOR
  * Added HTTP support to module import
  * fn:parse() is implemented as per the XQuery proposal
  * Extended api to register user-defined URI resolvers for documents,
    collections, schemas, and modules
  * boost dependencies removed
  * Added xquery debugging support
  * Added a store implementation based on Windows DOM.
  * Added functions to make calls to REST based web services through XQuery
  * Added PHP bindings to zorba
  * Added tidy support to convert web pages into xhtml.
  * Added XML Schema support for validation
  * Added the ability to revalidate XML data with respect to a Schema
    after applying updates

version 0.9.21-4303 (2008.06.26):
  * fixed installation problem for python and ruby language bindings
  * fixed some valgrind reported memory problems
  * other minor bug fixes and performance improvements

version 0.9.2-4251 (2008.06.19):
  * Module support (as in the XQuery 1.0 W3C recommendation)
  * Removed boost shared pointers from the API
  * Performance improvements and bug fixes
  * C API (zorbac.h)
  * Rudimentary SWIG interface for generating other language bindings
    (i.e. Ruby and Python)
  * Added support for running multiple queries in the command line utility
  * group by (as in the evolving W3C XQuery 1.1 recommendation)
  * try-catch (as in the evolving W3C recommendation)
  * eval facility to dynamically evaluate a string and execute it as a query
  * some math functions (e.g. sqrt, sin, cos, etc)
  * Basic schema validation (without post schema validation and
    substitution groups)
  * Stabilized the Store API & build the simplestore as a separate library

version 0.9.1 (2008.04.30):
  * Initial release supporting
    * XQuery 1.0 and XPath 2.0 (as in the W3C recommendation)
      except modules and only limited support for schema validation
    * XQuery Update Facility 1.0 (as in the evolving W3C recommendation)
    * An In-Memory (replaceable) XML store
      (implements the XQuery Data Model (XDM) W3C recommendation)
  * This release 0.9 passes 97.6% of the W3C tests of XQuery 1.0 test suite
    and 99.4% of the XQuery Update Facility test suite.
