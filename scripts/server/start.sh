#!/bin/bash

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../../" && pwd)"

$ROOT_DIR/bin/disco_server -c $ROOT_DIR/etc/disco_server.conf


