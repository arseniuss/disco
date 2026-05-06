#!/bin/false

ison() { [[ "$1" =~ ^[Yy]([Ee][Ss])?$|^1$|^[Tt]([Rr][Uu][Ee])?$|^[Oo][Nn]$ ]] }

echo_sleep() {
    local timeout=$1

    echo -n "Sleeping for $timeout s "

    while (( $timeout > 0 )); do
        sleep 1
        timeout=$(( $timeout - 1))
        echo -n "."
    done

    echo " done"
}
