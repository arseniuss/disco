#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../" && pwd)"

source $SCRIPT_DIR/test.sh

###################################################################

$SCRIPT_DIR/server/start.sh &

echo_sleep 5

