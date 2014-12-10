#!/bin/bash
# Run test consul. usage: test-consul.sh start|stop

case "$1" in
start)
    mkdir -p /tmp/test-consul/data || exit 1
    consul agent -bootstrap-expect=1 -server -dc=ppconsul_test -data-dir=~/tmp/test-consul/data -pid-file=/tmp/test-consul/pid  >/dev/null &
    sleep 3s
    if [ -e "/tmp/test-consul/pid" ]; then
        echo "test consul started, PID=$(</tmp/test-consul/pid)"
    else
        echo "failed to start"
    fi
    ;;
stop)
    if [ -e "/tmp/test-consul/pid" ]; then
        kill -SIGINT $(</tmp/test-consul/pid) || exit 1
        echo "test consul stopped"
    else
        echo "test consul is not running"
    fi
    ;;
*)
    echo "Usage: test-consul.sh start|stop"
    exit 2
    ;;
esac
