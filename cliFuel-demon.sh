#!/bin/bash

su - clifueluser

HERE="$(dirname "$(test -L "$0" && readlink "$0" || echo "$0")")"
cd "${HERE}"

if [ ! -f "./cliFuel" ]; then
    echo "cliFuel not exist"
    exit -1
fi

while true
do
   OUTPUT=`./cliFuel $@`
   mailsend -to $ACCOUNT_GMAIL_TO -from cliFuel@no-reply.it -ssl -port 465 -auth -smtp smtp.gmail.com -sub "cliFuel report" -cs "us-ascii" -enc-type "7bit" -M "$OUTPUT" +cc +bc -v -user $ACCOUNT_GMAIL_USER -pass $ACCOUNT_GMAIL_PASSWORD
   i=1
   while [ $i -le 40 ]
   do
      printf "=";
      i=$(( $i + 1 ))
   done
   echo ""
   sleep 1d
done