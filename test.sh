#!/bin/bash
SHELL=./mini_shell
PASS=0
FAIL=0

check(){
    if [ "$1" = "$2" ]; then
        echo "PASS: $3"
        PASS=$((PASS+1))
    else
        echo "FAIL: $3 (expected '$2' got '$1')"
        FAIL=$((FAIL+1))
    fi
}

# test echo
result=$(echo "echo hello" | ./mini_shell | grep hello)
check "$result" "hello" "echo command"

# test pipe
result=$(echo "ls | grep .c" | ./mini_shell | grep main)
check "$result" "main.c" "pipe ls grep"

# test redirect
echo "echo world > out.txt" | ./mini_shell
check "$(cat out.txt)" "world" "output redirect"

# test append
echo "echo appended >> out.txt" | ./mini_shell
check "$(cat out.txt)" "$(printf 'world\nappended')" "append redirect"

# test input redirect
echo "cat < out.txt" | ./mini_shell | grep world
check "$?" "0" "input redirect"

# cleanup
rm -f out.txt

echo "Passed: $PASS  Failed: $FAIL"