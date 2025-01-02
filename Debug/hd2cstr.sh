#!/bin/sh

hexdump -v -e '/1 "%02X "' $1 | awk '{ for (i = 1; i < NF; i++) \
	print "0x"$i", ";}'
