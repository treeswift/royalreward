#!/bin/bash
cp ag.txt ag.bak
AG=`pwd`/ag.txt
if meson build && cd build && ninja
then
	time ./gen > "$AG"
	# time ./mod
	# ./dat "$1"
	./mod ~/Downloads/KB/kb_mod -1
	cp ~/Downloads/KB/kb_mod/Nolan/Nolan.DAT ~/Downloads/KB/kb_mod/
fi
