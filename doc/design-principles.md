# Design principles

These principles guide decisions about Zorba's interfaces: the query
language surface, the command line, `zorba-mcp`, error messages, modules and
documentation. Zorba's users are developers and the AI agents that work for
them, so the principles are written for those readers. They are inspired by
the [design principles in Apple's Human Interface
Guidelines](https://developer.apple.com/design/human-interface-guidelines/design-principles)
and adapted to a query engine.

They help choose how something is built. Whether it is built at all is
decided by purpose, the first principle.

## Precedence

**The specification wins.** Where a W3C or JSONiq specification defines a
behavior, Zorba follows it, even when another behavior would look more
familiar or simpler. No principle below justifies a conformance bug. When the
specified behavior surprises people, improve the error message or the
documentation, not the semantics.

Example: `$a2-$a1` is a single name in XQuery. The parser keeps reading it
that way, and the diagnostic is what can help.

## Principles

### Purpose: every feature serves a use case

Add a module, option or tool only when a concrete use case needs it. Remove
what no longer serves one, after recording the idea it carried.

*Test: which use case fails without this?*

### Agency: let people recover by themselves

Do not force one workflow. An error says what went wrong, where, and what to
do next, in a form both a person and a program can act on.

*Test: can the reader fix this from the message alone?*

### Responsibility: safe by default, nothing hidden

Access to the file system, the network and native code is denied unless
granted. Zorba and its tools run on the user's own infrastructure with the
user's own credentials and depend on no hosted service. Documentation states
limitations plainly.

*Test: what can this do that the user did not ask for?*

### Familiarity: build on what people already write

Within the specification, accept the forms that developers and models
already use, follow common conventions for command lines and data formats,
and behave consistently across modules.

*Test: what would someone who has never read our documentation try first?*

### Flexibility: an independent program that fits many contexts

Zorba runs from a container, a local build or an embedded library, with build
profiles for what each context needs. It does not assume a platform, a
provider or a client.

*Test: does this work without our accounts, our machines or one vendor?*

### Simplicity: small surface, short output

Prefer one way to do a thing, sensible defaults and concise output. Output
size matters: what an interface returns is read, and paid for, by the agent
that receives it.

*Test: what happens if we remove this option, field or line?*

### Craft: measure, then claim

Tests come first and are seen to fail. Performance and memory claims come
from measurements that can be repeated, with their limits stated. Warnings
and failures are fixed, not silenced.

*Test: which run shows this is true?*

### First success: the first query works

The path from installation to a first useful answer is short and is tested.
Examples in the documentation run as written.

*Test: can a new user get an answer without help?*
