### Security
- JSound schema imports and `$constraints` (`jsd:validate`, `jsd:annotate`) now run under the security policy of the calling query instead of the instance policy, so a query restricted with `StaticContext::setSecurityPolicy()` can no longer read files or reach the network through them.

### Fixed
- A JSound schema import denied by the security policy names the capability its `$location` needs, not the one for the namespace URI tried after it.
- Fetch module errors (`zuri:URI_UNRESOLVED_OR_NOSTREAM`) are no longer wrapped in a second copy of the same error message.
