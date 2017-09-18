rm -f readfiles.txt;
echo $2 >> readfiles.txt;
echo `basename $2` >> readfiles.txt;
for i in `ls $1`; do echo "$1$i" >> readfiles.txt; echo "$i" >> readfiles.txt; done
./S2
