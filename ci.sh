#!/bin/bash
cp ag.txt ag.bak
AG=`pwd`/ag.txt
if meson build && cd build && ninja
then
	time ./gen > "$AG"
	# time ./mod
	./mod ~/Downloads/KB/kb_mod -1
	./dat ~/Downloads/KB/kb_mod/Nolan/Nolan.DAT > ../Nolan.txt
	cp ~/Downloads/KB/kb_mod/Nolan/Nolan.DAT ~/Downloads/KB/kb_mod/
fi
