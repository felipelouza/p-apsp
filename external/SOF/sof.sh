#! /bin/sh

#sources
A=(10000 50000 100000 150000 200000 250000 300000)
#A=(10000000 50000000 100000000 150000000 200000000 250000000 300000000)

D=("all_ests" "reads") 
ext=(".fasta" ".fastq") 

L=(1 10 20 30 40 50)
T=(1 2 4 8 16 32)

host="jau"
dir="/mnt/data/bio/projects/gpt/sof/"
src="/mnt/data/bio/projects/gpt/"
exp="sof"
out="0"

mkdir -p tests
make clean 
make

########################################################################
#
#mkdir -p ${dir}
#
#for d in 0 
#do
#       for a in {0..6}
#       do
#       
#               echo converter ${src}${D[$d]}${ext[$d]} ${dir}${D[$d]}.${A[$a]}${ext[$d]} ${A[$a]}
#               ./converter ${src}${D[$d]}${ext[$d]} ${dir}${D[$d]}.${A[$a]}${ext[$d]} ${A[$a]}
#       
#       done
#done   
#
#######################################################################

	
for d in 0 
do
	
	test=${D[$d]}
	echo "################"	
	echo "${test}"	

	for a in {0..6}
	do
		echo "########"	
		echo "K=${A[$a]}"	
	
		for l in 1
		do
		
			echo "####"	
			echo "L=${L[$l]}"	
		
			for t in {0..5}
			do
	
				mkdir -p tests/${test}/
				date >> tests/${test}/${host}.db.${A[$a]}.${exp}.txt
	
				echo " " >> tests/${test}/${host}.db.${A[$a]}.${exp}.txt
				./Apsp ${dir}${D[$d]}.${A[$a]}${ext[$d]} -p ${T[$t]} -m ${L[$l]} >> tests/${test}/${host}.db.${A[$a]}.${exp}.txt

			done
		done
	done
	
done
#########################################################################
