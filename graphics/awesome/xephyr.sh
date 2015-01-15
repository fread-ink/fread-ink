#!/bin/sh

APP="http://localhost/"

Xephyr -ac -br -noreset -grayscale -screen 600x800 :1 &
XEPH_PID=$!

export DISPLAY=:1

sleep 1

xsetroot -solid '#FFFFFF'

awesome -c ./rc.lua & 

sleep 1

midori -c `readlink -f ./config` --app=$APP

kill $XEPH_PID
