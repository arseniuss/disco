#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../" && pwd)"

source $SCRIPT_DIR/test.sh

###################################################################

export SERVER_RUN_VALGRIND=yes

$SCRIPT_DIR/server/start.sh

echo_sleep 5

killall -9 disco_server

