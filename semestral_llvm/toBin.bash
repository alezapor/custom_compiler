#!/usr/bin/env bash

for i in samples/*.p 
do
	./pjp $i
done

cd obj
for i in *.o 
do
	clang $i -o "../bin/${i%.o}.out"
done
