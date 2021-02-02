#!/usr/bin/env bash

# define some colors
RED='\033[0;31m'
NC='\033[0m' # No Color

# loop through all files in test directory
for file in $(find tests -name "*.tinyL")
#for file in tests/*.tinyL
do
    # if we have a file
    if [ -f "$file" ]; then
        # execute solution and store output
        ./solution/compile $file > /dev/null 2> /dev/null
        mv tinyL.out _tc_s.out

        # execute manual answer and store output
        ./bin/compile $file > /dev/null 2> /dev/null
        mv tinyL.out _tc_t.out

        # compare the output
        if cmp -s _tc_s.out _tc_t.out; then
            # if the compilation passes, test out the optimization
            ./solution/optimize < _tc_s.out > _tc_s_opt.out 2> /dev/null
            ./bin/optimize < _tc_t.out > _tc_t_opt.out 2> /dev/null

            # compare the output
            if cmp -s _tc_s_opt.out _tc_t_opt.out; then
                ERROR_COUNT=$(valgrind --leak-check=full --show-reachable=yes --track-origins=yes ./bin/optimize < _tc_t.out 2>&1 | grep ERROR | awk  '{ print $4 }')
                if [[ $ERROR_COUNT -eq "0" ]]; then
                    echo $file passed
                else
                    echo $file passed, with $ERROR_COUNT memory leaks
                fi
            else
                echo -e $file$RED failed optimization $NC
            fi
        else
            echo $file$RED failed compile $NC
        fi
    fi
done

# remove testing artifacts
rm _tc_t.out
rm _tc_s.out
rm _tc_t_opt.out
rm _tc_s_opt.out

if test -f "vgcore.*"; then
    rm vgcore.*
fi
