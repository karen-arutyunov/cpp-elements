#! /bin/sh

url=$1
signature=$2

if test -z "$url" -o -z "$signature"; then
  echo "USAGE: ElProbWEB.sh <url> <signature>"
  exit 2
fi

if test "`wget --no-check-certificate -o /dev/null -O - $url`" = "$signature"; then
 exit 0
else
 exit 1
fi
