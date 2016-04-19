#! /bin/sh

#sources
A=(10000 50000 100000 150000 200000 250000 300000)
L=(1 10 20 30 40 50)
T=(1 2 4 8 16 32)

D=("all_ests.fasta" "reads.fastq") 

host="bug"
dir="/mnt/disk1/database/"
exp="time"

#######################################################################

mkdir -p tests
make clean 
make
	
for i in {0..8}
do
	
	test=${D[$i]}
	echo "${test}"	

	for j in 1
	do
	
#		echo "MODE ${j}"
		
		mkdir -p pizzachili/${test}/
		date >> pizzachili/${test}/${host}.db.${exp}.txt

		echo " " >> pizzachili/${test}/${host}.db.${exp}.txt
		make run DIR=${dir} INPUT=${test}.txt K=${A[$i]} CHECK=$c MODE=$j >> pizzachili/${test}/${host}.db.${exp}.txt
	done
	
done


#########################################################################
