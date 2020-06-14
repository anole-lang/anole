#!/bin/bash
for file in `find ./*.anole`
do
    echo "<RUN> $file"
    anole $file
    echo ""
done
