# Sandbox of configuration B of the agent benchmark: a bash tool with the
# usual command-line data tools. The harness runs it with the snapshot
# mounted read-only at /data, a read-only root file system, a small writable
# /tmp, and CPU, memory and process limits.
#
# Targets:
#   sandbox         the bash sandbox of the api backend (run with
#                   --network none; the harness runs each command in it)
#   sandbox-claude  the same plus the Claude Code native binary, for the
#                   claude-code backend: Claude Code itself runs inside the
#                   container, so its Bash tool only sees the container; its
#                   only network route is the allowlisting egress proxy
#                   (egress_proxy.py). Build with
#                   --build-arg CLAUDE_CODE_VERSION=<exact version>.
FROM debian:trixie-slim AS sandbox

RUN apt-get update \
 && apt-get install -y --no-install-recommends \
      bash coreutils findutils grep gawk sed python3 jq libxml2-utils \
 && rm -rf /var/lib/apt/lists/* \
 && useradd --create-home --uid 1000 agent

USER agent
WORKDIR /data
CMD ["sleep", "infinity"]

# Claude Code from npm; only its self-contained native binary is copied into
# the sandbox, so Node.js is not one of the tools the agent can use.
FROM node:24-trixie-slim AS claude-code
ARG CLAUDE_CODE_VERSION
# The package's postinstall script puts the native binary in place; recent
# npm releases run it only when allowed explicitly.
RUN test -n "$CLAUDE_CODE_VERSION" \
 && npm install --global --allow-scripts=@anthropic-ai/claude-code \
      "@anthropic-ai/claude-code@${CLAUDE_CODE_VERSION}" \
 && mkdir /opt/claude \
 && cp "$(npm root --global)/@anthropic-ai/claude-code/bin/claude.exe" \
       /opt/claude/claude \
 && /opt/claude/claude --version

FROM sandbox AS sandbox-claude
COPY --from=claude-code /opt/claude/claude /usr/local/bin/claude
