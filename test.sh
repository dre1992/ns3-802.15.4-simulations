#test
clear
#for ((j=1;j<=4;j++))
j=1
do 

for ((i=1;i<=(j*10)-1;i++))
do
num=$((j*10))
/home/dre/work/ns-allinone-3.24.1/ns-3.24.1/waf --run "scratch/test_macservice --t=$i --k=$num" --cwd=/home/dre/Desktop/final-out/macservice_new


#done
done
exit 0
