#!/usr/bin/env bash

# computes the times of the operations ot TSP with GA

if [ ! -f ./tsp_seq ]; then
    echo "File tsp_seq does not exists. Making . . .";
    make;
fi

declare -i Cr;
declare -i Bt;
declare -i Rt;

for run in {1..1000};
do
    ./tsp_seq 10 100 1 > tmp.txt;

    ci=$(echo | awk '($1 == "Creation") {S += $4; N += 1} END {print S/N}' tmp.txt);
    bi=$(echo | awk '($1 == "Breeding") {S += $4; N += 1} END {print S/N}' tmp.txt);
    ri=$(echo | awk '($1 == "Ranking") {S += $4; N += 1} END {print S/N}' tmp.txt);

    Ct=$((Ct+ci));
    Bt=$((Bt+bi));
    Rt=$((Rt+ri));

    rm tmp.txt;
done

echo "Creation average time = $((Ct/1000))";
echo "Breeding average time = $((Bt/1000))";
echo "Ranking average time = $((Rt/1000))";
