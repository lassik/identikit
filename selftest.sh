#!/bin/sh
exec tcpclient -R 127.0.0.1 auth sh -c 'echo $TCPREMOTEPORT,$TCPLOCALPORT >&7 && cat <&6'
