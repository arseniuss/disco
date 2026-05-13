#!/bin/bash

# List files in dist tarball (strip top-level dir)
tar tzf *.tar.gz | sed 's|^[^/]*/||' | sort > /tmp/dist-files.txt
# List tracked files in repo
git ls-files | sort > /tmp/repo-files.txt
# Show files in repo but missing from dist
echo "=== Files missing from distribution ==="
comm -23 /tmp/repo-files.txt /tmp/dist-files.txt
