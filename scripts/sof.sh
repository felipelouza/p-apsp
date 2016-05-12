#! /bin/sh

#sources
A=(10000 50000 100000 150000 200000 250000 300000)
#A=(10000000 50000000 100000000 150000000 200000000 250000000 300000000)
#A=(1000 5000 10000 50000 100000 150000 200000)

D=("all_ests" "reads" "citrus_ests")
ext=(".fasta" ".fastq" ".fasta")

L=(5 10 15 20 30 40 50)
T=(1 2 4 8 16 32)

host="jau"
dir="/mnt/data/bio/projects/gpt/sof/"
src="/mnt/data/bio/projects/gpt/"
exp="sof"
out="0"

mkdir -p tests
make clean
make

#########################################################################
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
########################################################################


for d in 0
do

        test=${D[$d]}
        echo "################" 
        echo "${test}"  

        for l in {0..3}
        do
                echo "####"     
                echo "L=${L[$l]}"       

                for a in {0..6}
                do
                        echo "########" 
                        echo "K=${A[$a]}"       



                        for t in {0..5}
                        do

                                mkdir -p tests/${test}/${L[$l]}/
                                date >> tests/${test}/${L[$l]}/${host}.db.${A[$a]}.${exp}.txt

                                echo " " >> tests/${test}/${L[$l]}/${host}.db.${A[$a]}.${exp}.txt
                                ./Apsp ${dir}${D[$d]}.${A[$a]}${ext[$d]} -p ${T[$t]} -m ${L[$l]} >> tests/${test}/${L[$l]}/${host}.db.${A[$a]}.${exp}.txt

                        done
                done
                cp nohup.out tests/${test}/${exp}.${L[$l]}.txt
        done

done
#########################################################################

