#! /bin/bash

createNewRecord () {
    while true  # then we choose an new id here that is not the same with the ids previus added
    do
        id=$((1 + $RANDOM % 9999))
        if [[ ! -v "credentials[$id]" ]];
        then
            break
        fi
    done
    arrId+=("$id") # add the id to the array
    lengthOfarrId=$((lengthOfarrId+1))

    lengthOfFirstName=$((3 + $RANDOM % 12)) # random length for the firstname
    lengthOfLastName=$((3 + $RANDOM % 12)) # random length for the lastname

    firstName=$(head /dev/urandom | tr -cd '[:alpha:]' | cut -c -$lengthOfFirstName) # generate the firstname
    lastName=$(head /dev/urandom | tr -cd '[:alpha:]' | cut -c -$lengthOfLastName) # generate the lastname

    indexCountry=$((0 + $RANDOM % $lengthOfCountries)) # random country

    age=$((1 + $RANDOM % 120)) # random age

    credentials[$id]="$id $firstName $lastName ${arrCountries[indexCountry]} $age"
}

START=$(date +%s) # starting time
if [ $# -ne 4 ] # check arguments
then
    echo "1 Argument = virusesFile"
    echo "2 Argument = countryFile"
    echo "3 Argument = numberOfRecords"
    echo "4 Argument = duplicatesAllowed"
    exit
fi
# take arguments to variables
virusesFile=$1
countriesFile=$2
numLines=$3
duplicatesAllowed=$4

if test -f $virusesFile; then # check if virusesFile exists
    echo "- $virusesFile exists."
else
    echo "- $virusesFile doesn't exists."
    exit
fi

if test -f $countriesFile; then # check if countriesFile exists
    echo "- $countriesFile exists."
else
    echo "- $countriesFile doesn't exists."
    exit
fi

echo "- $numLines Records"

if [ $duplicatesAllowed -eq 1 ]
then
    echo - "Duplicates are allowed"
else
    echo "- Duplicates are NOT allowed"
fi

# declare arrays to store the countries,viruses and previus added ids and credentials
declare -a arrCountries; 
declare -a arrViruses;
declare -a arrId;
declare -A credentials;

while IFS= read -r line; do # put the countries from the countriesFile to the array arrCountries
    arrCountries+=("$line")
done < $countriesFile

while IFS= read -r line; do  # put the countries from the virusesFile to the array arrViruses
    arrViruses+=("$line")
done < $virusesFile

lengthOfCountries=${#arrCountries[@]}
lengthOfViruses=${#arrViruses[@]}

lengthOfarrId=0
createNewRecord # create the first record here before the for

for (( i=0; i<$numLines; i++ )) do # do a loop for max records
    id=0
    if [ $duplicatesAllowed -eq 1 ]    # if we want duplicates
    then
        possibility=4
        if [ $((1 + $RANDOM % 10)) -lt $possibility ] # there is 40% change to make a duplicate id 
        then
            index=$((0 + $RANDOM % $lengthOfarrId))
            id=${arrId[index]}
        fi
    fi

    if [ $id -eq 0 ] # if we didnt select an id from the previous if
    then             
        createNewRecord
    fi
    
    indexVirus=$((0 + $RANDOM % $lengthOfViruses))  # random virus
    record=${credentials[$id]}

    if [ $(($RANDOM % 2)) -eq 1 ] # its a 50% change to be NO or YES
    then
        date="$((1 + RANDOM % 30))-$((1 + RANDOM % 12))-$((1950 + RANDOM % 100))" # generate date
        echo $record ${arrViruses[indexVirus]} 'YES' $date >> inputFile.txt
    else
        echo $record ${arrViruses[indexVirus]} 'NO' >> inputFile.txt
    fi
done
echo ""
echo "$i Records generated to inputFile.txt"
END=$(date +%s)
DIFF=$(( $END - $START ))
echo "It took $DIFF seconds"