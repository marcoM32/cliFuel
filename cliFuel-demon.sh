#!/bin/bash
while true
do
	./cliFuel $@
	i=1
	while [ $i -le 40 ]
	do
	  printf "=";
	  i=$(( $i + 1 ))
	done
	echo ""
	sleep 1d
done