#!/bin/bash
for file in `find ./*.icec`
do
    ice $file
done
