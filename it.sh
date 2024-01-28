#!/bin/bash

build/ham "$1"
echo -n "Size: "
wc -c "$1" | cut -d ' ' -f 1
echo -n "Hash: "
sha256sum "$1" | cut -d ' ' -f 1
