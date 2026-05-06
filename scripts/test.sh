#!/bin/false

source $SCRIPT_DIR/aux.sh

cleanup() {
    echo Killing all background jobs: $(jobs -p)
    kill -9 $(jobs -p)
}

trap cleanup EXIT
