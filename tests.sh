#! /bin/sh

#sources
A=(10000 50000 100000 150000 200000 250000 300000)
#A=(10000000 50000000 100000000 150000000 200000000 250000000 300000000)

D=("all_ests" "reads") 
ext=(".fasta" ".fastq") 

L=(1 10 20 30 40 50)
T=(1 2 4 8 16 32)

host="jau"
dir="/mnt/data/bio/projects/gpt/"
exp="apsp"
out="0"

#######################################################################

mkdir -p tests
make clean 
make
	
for i in 0 
do
	
	test=${D[$i]}

	echo "################"	
	echo "${test}"	

	for j in {0..6}
	do
	
		echo "########"	
		echo "K=${A[$j]}"	
	
		for l in 1
		do
		
			echo "####"	
			echo "L=${L[$l]}"	
		
			for t in {0..5}
			do
	
				mkdir -p tests/${test}/
				date >> tests/${test}/${host}.db.${A[$j]}.${exp}.txt
	
				echo " " >> tests/${test}/${host}.db.${A[$j]}.${exp}.txt
				make run_apsp DIR=${dir} INPUT=${D[$i]}${ext[$i]} K=${A[$j]} OUTPUT=${out} L=${L[$l]} T=${T[$t]} >> tests/${test}/${host}.db.${A[$j]}.${exp}.txt
			done
		done
	done
	
done
#########################################################################
