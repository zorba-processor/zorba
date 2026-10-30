#!/usr/bin/env python3
# Copyright 2006-2016 zorba.io
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
"""Build libFuzzer seed corpora (zip files) from the test suite.

Usage: make_seed_corpora.py <zorba source dir> <output dir>

Writes <target>_seed_corpus.zip for each fuzz target, the naming that
OSS-Fuzz / ClusterFuzzLite pick up automatically. The selection is
deterministic: small files only, evenly sampled over the sorted list.
"""

import json
import os
import sys
import zipfile

MAX_FILES = 400


def collect(root, exts, max_size):
    found = []
    for dirpath, dirnames, filenames in os.walk(root):
        dirnames.sort()
        for name in sorted(filenames):
            if not name.endswith(exts):
                continue
            path = os.path.join(dirpath, name)
            if 0 < os.path.getsize(path) <= max_size:
                found.append(path)
    return found


def sample(paths, n=MAX_FILES):
    if len(paths) <= n:
        return paths
    step = len(paths) / n
    return [paths[int(i * step)] for i in range(n)]


def is_json(path):
    try:
        with open(path, encoding="utf-8") as f:
            text = f.read().strip()
        if not text or text[0] not in "{[":
            return False
        json.loads(text)
        return True
    except (ValueError, UnicodeDecodeError):
        return False


def write_zip(out_dir, target, paths, src):
    zpath = os.path.join(out_dir, target + "_seed_corpus.zip")
    with zipfile.ZipFile(zpath, "w", zipfile.ZIP_DEFLATED) as z:
        for p in paths:
            z.write(p, os.path.relpath(p, src).replace(os.sep, "_"))
    print("%s: %d seeds" % (zpath, len(paths)))


def main():
    src, out_dir = sys.argv[1], sys.argv[2]
    queries = os.path.join(src, "test", "rbkt", "Queries")
    test = os.path.join(src, "test")

    write_zip(out_dir, "xquery_compile",
              sample(collect(queries, (".xq",), 2048)), src)
    write_zip(out_dir, "jsoniq_compile",
              sample(collect(queries, (".jq",), 2048)), src)
    write_zip(out_dir, "xml_parse",
              sample(collect(test, (".xml",), 8192)), src)

    jsons = collect(test, (".json",), 8192)
    results = os.path.join(src, "test", "rbkt", "ExpQueryResults")
    jsons += [p for p in collect(results, (".res",), 4096) if is_json(p)]
    write_zip(out_dir, "json_parse", sample(jsons), src)


if __name__ == "__main__":
    main()
