#!/usr/bin/env bash

# computes the speedups for different solutions

echo "Usage: ./speedup.sh [population size (default: 100)] [number of generations (default: 1)]";
echo "";

declare -i ncores;
nc=10; # number of cities
n=1; # number of generations
npop=100; # number of population
ncores=$(echo | nproc);
echo "Number of cores = $((ncores))";
# grep ^cpu\\scores /proc/cpuinfo | uniq |  awk '{print $4}'

if [ $1 ]; then
    npop=$1;
fi
if [ $2 ]; then
    n=$2;
fi

echo "Population size = $npop";
echo "Number of generations = $n";
echo "Number of cities = $nc";
echo "";

nws=(); # number of workers list
par_times=(); # parallel execution times
par_v2_times=(); # parallel v2 execution times
ff_times=(); # fastflow execution times

echo "Cleaning . . .";
make cleanall;
if [ -f tmp_seq.txt ]; then
    rm tmp_seq.txt;
fi
if [ -f tmp_par.txt ]; then
    rm tmp_par.txt;
fi
if [ -f tmp_par_v2.txt ]; then
    rm tmp_par_v2.txt;
fi
if [ -f tmp_ff.txt ]; then
    rm tmp_ff.txt;
fi
echo "Making . . .";
make;
echo "";

# compute sequential average time
t_seq=0;
for run in {1..100};
do
    cmd="./tsp_seq $nc $npop $n >> tmp_seq.txt";
    eval $cmd;
done
t_seq=$(echo | awk '($1 == "Program") {S += $4; N += 1} END {print int(S/N)}' tmp_seq.txt);
echo "Sequential time = $t_seq";
rm tmp_seq.txt;


for (( nw=1; nw<=ncores; nw=nw*2 ))
do
    nws+=($nw);
    
    # compute parallel average time
    for run in {1..100};
    do
        cmd="./tsp_par $nc $npop $n $nw >> tmp_par.txt";
        eval $cmd;
    done
    t_par=$(echo | awk '($1 == "Program") {S += $4; N += 1} END {print int(S/N)}' tmp_par.txt);
    rm tmp_par.txt;
    par_times+=($t_par);

    # compute parallel v2 average time
    for run in {1..100};
    do
        cmd="./tsp_par_v2 $nc $npop $n $nw >> tmp_par_v2.txt";
        eval $cmd;
    done
    t_par_v2=$(echo | awk '($1 == "Program") {S += $4; N += 1} END {print int(S/N)}' tmp_par_v2.txt);
    rm tmp_par_v2.txt;
    par_v2_times+=($t_par_v2);

    # compute fastflow average time
    for run in {1..100};
    do
        cmd="./tsp_ff $nc $npop $n $nw >> tmp_ff.txt";
        eval $cmd;
    done
    t_ff=$(echo | awk '($1 == "Program") {S += $4; N += 1} END {print int(S/N)}' tmp_ff.txt);
    rm tmp_ff.txt;
    ff_times+=($t_ff);

done

len=${#nws[@]};
f=$(echo | awk "BEGIN {print (46 / (46 + $n * 1714))}");
echo "Serial fraction = $f";
echo "Speedup:";
printf "%15s\t%15s\t%15s\t%15s\t%15s\n" "Pardegree" "C++ Threads" "C++ Threads v2" "FastFlow" "Ideal";
for(( i=0; i<$len; i++));
do
    nw=${nws[i]};
    t_par=${par_times[i]};
    t_par_v2=${par_v2_times[i]};
    t_ff=${ff_times[i]};
    
    ideal_sp=$(echo | awk "BEGIN {print ($t_seq / ($f * $t_seq + ((1 - $f) * $t_seq) / $nw))}"); # ideal speedup
    par_sp=$(echo | awk "BEGIN {print ($t_seq / $t_par)}"); # parallel speedup
    par_v2_sp=$(echo | awk "BEGIN {print ($t_seq / $t_par_v2)}"); # parallel v2 speedup
    ff_sp=$(echo | awk "BEGIN {print ($t_seq / $t_ff)}"); # fastflow speedup
    printf "%15s\t%15s\t%15s\t%15s\t%15s\n" $nw $par_sp $par_v2_sp $ff_sp $ideal_sp;
done
echo "";
