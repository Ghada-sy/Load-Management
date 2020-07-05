#! /bin/bash
for i in {1..250}
do
	./waf --run "scratch/RealSce/RealSce --RunNum=$(($i))"
done
