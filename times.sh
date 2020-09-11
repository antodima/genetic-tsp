#!/usr/bin/env bash

# computes the times of the operations ot TSP with GA

if [ ! -f ./tsp_seq ]; then
    echo "File tsp_seq does not exists. Making . . .";
    make;
fi

declare -i Ct; # creation time
# declare -i Bt; # breeding time
declare -i Rt; # ranking time
declare -i Xt; # crossover time
declare -i At; # adding time
declare -i Mt; # mutation time

for run in {1..100};
do
    ./tsp_seq 10 100 1 > tmp.txt;

    ci=$(echo | awk '($1 == "Creation") {S += $4; N += 1} END {print int(S/N)}' tmp.txt);
    bi=$(echo | awk '($1 == "Breeding") {S += $4; N += 1} END {print int(S/N)}' tmp.txt);
    ri=$(echo | awk '($1 == "Ranking") {S += $4; N += 1} END {print int(S/N)}' tmp.txt);
    xi=$(echo | awk '($1 == "Crossover") {S += $4; N += 1} END {print int(S/N)}' tmp.txt);
    ai=$(echo | awk '($1 == "Adding") {S += $4; N += 1} END {print int(S/N)}' tmp.txt);
    mi=$(echo | awk '($1 == "Mutation") {S += $4; N += 1} END {print int(S/N)}' tmp.txt);
    
    Ct=$((Ct+ci));
    Bt=$((Bt+bi));
    Rt=$((Rt+ri));
    Xt=$((Xt+xi));
    At=$((At+ai));
    Mt=$((Mt+mi));

    ################################################################
    # TODO: prendere in considerazione ambdhal e gustafson speedup # <-------------
    ################################################################

    rm tmp.txt;
done

echo "Creation average time = $((Ct/100))";
echo "Breeding average time = $((Bt/100))";
echo "  - Crossover average time = $((Xt/100))";
echo "  - Mutation average time = $((Mt/100))";
echo "  - Adding average time = $((At/100))";
echo "Ranking average time = $((Rt/100))";
