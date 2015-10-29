#!/bin/bash
#CMPUT 379 Fall 2015
#Test Case: 3


echo "START 1st Sleep15"
./sleep15 &

sleep 6

echo "START 2nd Sleep15"
./sleep15 &

echo "RUNNING procnanny"
./procnanny test2.config


