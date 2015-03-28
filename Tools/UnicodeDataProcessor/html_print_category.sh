#! /bin/sh

cat="$1"

echo "<html>"
echo "<body>"

cat ./UnicodeData.txt | grep ";$1;" | sed -n -e "s/\(.*\);\(.*\);\(.*\);\(.*\);\(.*\);\(.*\);\(.*\);\(.*\);\(.*\);\(.*\);\(.*\);\(.*\);\(.*\);\(.*\);\(.*\)/\1 \&#x\1;<br>/p"

echo "</body>"
echo "</html>"
