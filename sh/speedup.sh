#!/usr/bin/env bash

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
ff_times=(); # fastflow execution times

#if [[ ! -f ./tsp_seq || ! -f ./tsp_par || ! -f ./tsp_ff ]]; then
echo "Cleaning . . .";
make cleanall;
echo "Making . . .";
make;
echo "";
#fi

# compute sequential average time
t_seq=0;
for run in {1..100};
do
    cmd="./tsp_seq $nc $npop $n >> sh/tmp_seq.txt";
    eval $cmd;
done
t_seq=$(echo | awk '($1 == "Program") {S += $4; N += 1} END {print int(S/N)}' sh/tmp_seq.txt);
echo "Sequential time = $t_seq";
rm sh/tmp_seq.txt;


for (( nw=1; nw<=ncores; nw=nw*2 ))
do
    nws+=($nw);
    
    # compute parallel average time
    for run in {1..100};
    do
        cmd="./tsp_par $nc $npop $n $nw >> sh/tmp_par.txt";
        eval $cmd;
    done
    t_par=$(echo | awk '($1 == "Program") {S += $4; N += 1} END {print int(S/N)}' sh/tmp_par.txt);
    rm sh/tmp_par.txt;
    par_times+=($t_par);

    # compute fastflow average time
    for run in {1..100};
    do
        cmd="./tsp_ff $nc $npop $n $nw >> sh/tmp_ff.txt";
        eval $cmd;
    done
    t_ff=$(echo | awk '($1 == "Program") {S += $4; N += 1} END {print int(S/N)}' sh/tmp_ff.txt);
    rm sh/tmp_ff.txt;
    ff_times+=($t_ff);

done

len=${#nws[@]};
#f=$(echo | awk "BEGIN {print (46+($n*86))/(46+($n*86)+($n*1628))}");
f=$(echo | awk "BEGIN {print (46 / (46 + $n * 1714))}");
echo "Serial fraction = $f";
echo "Speedup:";
printf "%12s\t%12s\t%12s\t%12s\n" "Pardegree" "C++ Threads" "FastFlow" "Ideal";
for(( i=0; i<$len; i++));
do
    nw=${nws[i]};
    t_par=${par_times[i]};
    t_ff=${ff_times[i]};
    
    ideal_sp=$(echo | awk "BEGIN {print ($t_seq / ($f * $t_seq + ((1 - $f) * $t_seq) / $nw))}"); # ideal speedup
    par_sp=$(echo | awk "BEGIN {print ($t_seq / $t_par)}"); # parallel speedup
    ff_sp=$(echo | awk "BEGIN {print ($t_seq / $t_ff)}"); # fastflow speedup
    printf "%12s\t%12s\t%12s\t%12s\n" $nw $par_sp $ff_sp $ideal_sp;
done
echo "";
