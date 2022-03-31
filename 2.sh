#!/bin/bash

s=$(date +%N)

s=$(( s / 1000000))


date +%H:%M:%S:$s




