#!/bin/bash

numLines=$3
FILE=inputFile
allLetters=abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ

#returns a random number from 0+$2 to $1+$2
random () {

    randomNumber=$RANDOM
    if [ $randomNumber -lt 0 ]
        then let "randomNumber = -randomNumber"
    fi
    let "randomNumber %= $1"
    let "randomNumber += $2"
    echo $randomNumber
}

#checking arguments
if [ $# -ne 4 ]
    then echo "Error: Wrong Number of arguments"
elif [ ! -e $1 -o ! -r $1 ]
    then echo "Error: file $1 does not exist or can't be read"
elif [ ! -e $2 -o ! -r $2 ]
    then echo "Error: file $2 does not exist or can't be read"
elif [ $3 -le 0 ]
    then echo "Error: Wrong Number of numLines"
elif [ $4 -ne 0 -a $4 -ne 1 ]
    then echo "Error: argument duplicatesAllowed must be 0 or 1"
fi

#in case of duplicatesAllowed=0 and numLines>10000
if [ $4 -eq 0 -a $3 -gt 10000 ]; then
    echo "Only 10000 unique records can be created."
    numLines=10000
fi

$(> $FILE)

randomNumber=$(random 10000 0)

while [ $numLines -gt 0 ]
do

    if [ $4 -eq 0 ]; then

        randomNumber=$(random 10000 0)

        #if no duplicates check for existance in previous citizenIDs
        if [ ! -z "$(cut -d " " -f 1 $FILE | grep $randomNumber)" ]; then
            continue
        fi

        #else every 5 lines create a duplicate
    elif [ $(( $numLines % 5 )) -ne 0 ]; then

        randomNumber=$(random 10000 0)
    fi

    echo -n "$randomNumber " >> $FILE
    
    #2 loops for name and surname
    for ((i=1 ; i<=2 ; i++)) do

        randomNumber=$(random 10 3)
        while [ $randomNumber -gt 0 ]
        do

            #random string with length [3,12]
            chosenChar=$(random 52 0)
            echo -n ${allLetters:chosenChar:1} >> $FILE
            randomNumber=$(($randomNumber-1))
        done
        echo -n " " >> $FILE
    done

    #random number from 1 to numLines of countries file
    randomNumber=$(random $(($(wc $2 | cut -d " " -f 2)+1)) 1)

    #pick the country
    echo -n `head -n $randomNumber < $2 | tail -n 1`" " >> $FILE
    
    randomNumber=$(random 120 1)
    echo -n "$randomNumber " >> $FILE

    #same as countries but for virus
    randomNumber=$(random $(($(wc $1 | cut -d " " -f 2)+1)) 1)
    echo -n `head -n $randomNumber < $1 | tail -n 1`" " >> $FILE

    randomNumber=$(random 2 0)

    if [ $randomNumber -eq 0 ]

        then echo -n "NO" >> $FILE

    else

        echo -n "YES " >> $FILE

        randomNumber=$(random 30 1)
        echo -n "$randomNumber-" >> $FILE

        randomNumber=$(random 12 1)
        echo -n "$randomNumber-" >> $FILE

        randomNumber=$(random 30 1990)
        echo -n "$randomNumber" >> $FILE
    fi

    echo >> $FILE
    numLines=$(($numLines-1))
done