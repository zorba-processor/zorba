# Sandbox of configuration B of the agent benchmark: a bash tool with the
# usual command-line data tools. The harness runs it with the snapshot
# mounted read-only at /data, no network, a read-only root file system, a
# small writable /tmp, and CPU, memory and process limits.
FROM debian:trixie-slim

RUN apt-get update \
 && apt-get install -y --no-install-recommends \
      bash coreutils findutils grep gawk sed python3 jq libxml2-utils \
 && rm -rf /var/lib/apt/lists/* \
 && useradd --create-home --uid 1000 agent

USER agent
WORKDIR /data
CMD ["sleep", "infinity"]
