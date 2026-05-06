#!/bin/bash

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../../" && pwd)"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../" && pwd)"

source $SCRIPT_DIR/aux.sh

####################################################################

if ison $SERVER_RUN_VALGRIND; then
    valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes \
        --log-file=$ROOT_DIR/var/disco_server_valgrind.log \
        $ROOT_DIR/bin/disco_server -c $ROOT_DIR/etc/disco_server.conf
else
    $ROOT_DIR/bin/disco_server -c $ROOT_DIR/etc/disco_server.conf
fi


