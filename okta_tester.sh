#/bin/bash

N=5
T=2
M=12

for P in {0..N-2}; do
    ./bin/frontend.exe -n $N -t $T -m $M -p $P & 
done
./bin/frontend.exe -n $N -t $T -m $M -p $N-1