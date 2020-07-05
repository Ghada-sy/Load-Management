#! /bin/bash
for i in {1..250}
do
	./waf --run "scratch/POCS/POCS --RunNum=$(($i))"
done
