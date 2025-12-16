#!/bin/bash

timeout -s ABRT $(echo $2)s "$1"
ret=$?
if [[ $ret == 124 ]]; then
    echo "FAILURE: The test has timed out after $2 seconds!"
    echo "============================"
    exit 1
fi
exit $ret
