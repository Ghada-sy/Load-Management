#! /bin/bash
for i in {1..200}
do
	./waf --run "scratch/POCS/POCS --RunNum=$(($i))"
done
