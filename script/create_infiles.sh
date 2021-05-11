#! /bin/bash

 rm -R -- */
#setting arguments 
inputFile=$1
input_dir=$2
numFilesPerDirectory=$3

#checking if i have 3 arguments 
if [ $# -ne 3 ];
then
    echo "Give 3 arguments"
    exit
fi
#checking if the inputFile exists and is not empty
if [ ! -s $inputFile ];
then
    echo "InputFile does not exist or has no data"
    exit
fi
#checking if the directory already exists
# if [ -d $input_dir ];
# then 
#     echo "input_dir already exists"
#     exit
# fi   

#checking if the last arguments is a valid arguments, it must be a number and positive

if   [[ ! $numFilesPerDirectory =~ ^-?[0-9]+$ ]] || [ $numFilesPerDirectory -lt 0 ];
then
    echo "numFilesPerDirectory must be an integer and a positive number "
    exit
fi

#create directory

mkdir -p $input_dir
declare -a countries

cd $input_dir
 #taking the countries from inputFile 
 
#creatind directories and .txt inside
 while IFS= read -r line;
 do
    country=$(echo $line | awk '{print $4}')
    if [ ! -d $country ]; then
        countries+=("$country")
        mkdir $country 
        cd $country
        for (( i=0; i<$numFilesPerDirectory; i++ ));
        do
            touch "${country}-$((${i}+1)).txt"
        done
        
        # echo $country

        cd ../
        
    fi

done < ../$inputFile
declare -a tokens;

for(( i=0; i<${#countries[@]}; i++ ));
do
    rrIndex=0
    country=${countries[$i]}
    cd $country
    echo $country
    while IFS= read -r inner;
    do 
        
        # innerCountry=$(echo $inner | awk '{print $4}')
        IFS=' ' read -r -a tokens <<< $inner
        innerCountry=${tokens[3]}
        if [ $innerCountry == $country ];
        then
            if [ $(($rrIndex % $numFilesPerDirectory)) -eq 0 ];
            then
               
                rrIndex=0
            fi
            rrIndex=$((rrIndex+1))
            echo $inner >> "${country}-${rrIndex}.txt"
        fi

    done < ../../$inputFile
     cd ../
done



# 