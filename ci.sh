#!/bin/bash
cp ag.txt ag.bak
AG=`pwd`/ag.txt
if meson build --buildtype=debug && cd builddir && ninja
then
	./gen > "$AG"
fi
