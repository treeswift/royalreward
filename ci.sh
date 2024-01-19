#!/bin/bash
cp ag.txt ag.bak
AG=`pwd`/ag.txt
if meson build && cd build && ninja
then
	time ./gen > "$AG"
	time ./mod
fi
