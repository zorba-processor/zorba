#!/bin/sh
# `docker run IMAGE zorba-mcp --root /data` runs the MCP server; any other
# arguments go to the zorba command-line tool as before.
case "${1-}" in
  zorba-mcp|zorba) exec "$@" ;;
esac
exec zorba "$@"
