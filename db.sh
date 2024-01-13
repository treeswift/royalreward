#!/bin/bash
cp ag.txt ag.bak
AG=`pwd`/ag.txt
if meson build --buildtype=debug --reconfigure && cd build && ninja
then
	time ./gen > "$AG"
fi
