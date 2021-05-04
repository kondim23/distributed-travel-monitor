#!/bin/bash

#checking arguments
if [ $# -ne 3 ]
    then echo "Error: Wrong Number of arguments"
    exit
elif [ ! -e $1 -o ! -r $1 ]
    then echo "Error: file $1 does not exist or can't be read"
    exit
elif [ -e $2 ]
    then echo "Error: directory $2 allready exists"
    exit
elif [ $3 -le 0 ]
    then echo "Error: Wrong Number of numFilesPerDirectory"
    exit
fi

mkdir $2

for i in $(cut -d " " -f 4 $1 | uniq)
do
    mkdir $2/$i
    for ((j=1 ; j<=$3 ; j++))
    do
        touch $2/$i/$i-$j.txt
    done

    touch $2/$i/fileNum
    echo 0 > $2/$i/fileNum

done

while read i
do

    country=$(echo $i | cut -d " " -f 4)
    file=$(cat $2/$country/fileNum)
    let "file += 1"
    echo $i >> $2/$country/$country-$file.txt
    let "file %= $3"
    echo -n $file > $2/$country/fileNum
done < $1

for i in $(cut -d " " -f 4 $1 | uniq)
do
    rm $2/$i/fileNum
done