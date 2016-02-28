#!/bin/bash

dir=$(dirname $0)
pid=$$

run_tests()
{
    echo -e "\nTesting $@"
    for t in $(cd $dir/tst; ls test_*.in | awk -F. '{print $1}'); do
        tmpfile=/tmp/$t.out.$pid
        echo -e "\nrunning $t"
        $@ < $dir/tst/$t.in > $tmpfile
        diff $dir/tst/$t.out $tmpfile
        echo $(test $? == 0 && echo passed || echo failed)
        rm -f $tmpfile
    done
}

run_tests python3 src_py/SimpleDatabase.py

sdb=/tmp/simple_database
echo -e "\nBuilding cpp solution into $sdb ..."
(cd src_cpp; g++ -std=gnu++11 simple_database.cpp -o $sdb)
run_tests $sdb

echo -e "\nDone."
exit 0
