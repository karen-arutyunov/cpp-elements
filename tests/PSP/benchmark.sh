#! /bin/sh

port=$1
requests=10000
concurrency=100

#ab -c $concurrency -n $requests http://vms.ocslab.com:$port/static/eng/contact
ab -c $concurrency -n $requests http://vms.ocslab.com:$port/static/xml/eng/countries.xml
