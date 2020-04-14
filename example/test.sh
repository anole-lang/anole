#!/bin/bash
for file in `find ./*.anole`
do
    anole $file
done
