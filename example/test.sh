#!/bin/bash
for file in `find ./*.ice`
do
    ice $file
done
