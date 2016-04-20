#! /bin/sh

#sources
#A=(10000 50000 100000 150000 200000 250000 300000)
#A=(10000000 50000000 100000000 150000000 200000000 250000000 300000000)
A=(781250 1562500 3125000 6250000 12500000 25000000 50000000)

D=("all_ests" "reads") 
ext=(".fasta" ".fastq") 

L=(1 10 20 30 40 50)
T=(1 2 4 8 16 32)

host="bug"
dir="/mnt/disk1/database/"
#dir="~/dataset/"
exp="time"
out="0"

#######################################################################

mkdir -p tests
make clean 
make
	
for i in 1
do
	
	test=${D[$i]}

	echo "################"	
	echo "${test}"	

	for j in {0..6}
	do
	
		echo "########"	
		echo "K=${A[$j]}"	
	
		for k in 1
		do
		
			echo "####"	
			echo "L=${L[$k]}"	
		
			for t in {0..4}
			do
	
				mkdir -p tests/${test}/
				date >> tests/${test}/${host}.db.${A[$j]}.${exp}.txt
	
				echo " " >> tests/${test}/${host}.db.${A[$j]}.${exp}.txt
				make run DIR=${dir} INPUT=${D[$i]}${ext[$i]} K=${A[$j]} OUTPUT=${out} L=${L[$k]} T=${T[$t]} >> tests/${test}/${host}.db.${A[$j]}.${exp}.txt
			done
		done
	done
	
done


#########################################################################
#
#for i in {0..1}
#do
#	
#	test=${D[$i]}
#	echo "${test}"	
#
#	for j in {0..6}
#	do
#	
#		echo "K=${A[$j]}"	
#	
#		for k in {0..5}
#		do
#	
#			mkdir -p tests/${test}/
#			date >> tests/${test}/${host}.db.${A[$j]}.${exp}.txt
#	
#			echo " " >> tests/${test}/${host}.db.${A[$j]}.${exp}.txt
#			make run_apsp DIR=${dir} INPUT=${D}${ext} K=${A[$j]} OUTPUT=${out} L=${L[$k]} >> tests/${test}/${host}.db.${A[$j]}.${exp}.txt
#		done
#	done
#	
#done
#
#
#########################################################################
