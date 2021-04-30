#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/fcntl.h>
#include <sys/wait.h>
#include <unistd.h>
#include <dirent.h>
#include <string>
#include <cstring>
#include <fstream>

#include "monitor.h"
#include "DataStructures/stringList/stringList.h"
#include "DataStructures/bloomFilter/bloomFilter.h"
#include "DataStructures/skipList/skipList.h"
#include "DataStructures/binaryAvlTree/tree.h"
#include "citizenRecords/citizen.h"
#include "util.h"

using namespace std;

Monitor::Monitor(string r, string w) : readFifo(r), writeFifo(w) {
    readFD = open(this->readFifo.c_str(), O_RDONLY);
    writeFD = open(this->writeFifo.c_str(), O_WRONLY);

    this->tree = NULL;
    this->countries = new stringList();
    checkNew(this->countries);

    this->viruses = new stringList();
    checkNew(this->viruses);

    this->skipLists = new skipList_List();
    checkNew(this->skipLists);
}

Monitor::~Monitor() {
    close(readFD);
    close(writeFD);
}

void Monitor::receiveCredentials() {
    if (read(readFD, &this->id, sizeof(int)) == -1)
        cout << "Monitor error in reading id with errno=" << errno << endl;
    if (read(readFD, &this->bufferSize, sizeof(int)) == -1)
        cout << "Monitor " << this->id << " error in reading bufferSize with errno=" << errno << endl;
    if (read(readFD, &this->bloomSize, sizeof(int)) == -1)
        cout << "Monitor " << this->id << " error in reading bloomSize with errno=" << errno << endl;
    this->blooms = new bloomFilterList(this->bloomSize);
    checkNew(this->blooms);
    cout << "Monitor " << this->id << ", bufferSize=" << this->bufferSize << ", bloomSize=" << this->bloomSize << endl;


    int sizeOfDir;
    if (read(readFD, &sizeOfDir, sizeof(int)) == -1)
        cout << "Monitor " << this->id << " error in reading sizeOfDir with errno=" << errno << endl;

    string dir = "";
    for (int i = 0;i <= sizeOfDir / this->bufferSize;i++) {
        char buff[this->bufferSize];
        if (read(readFD, &buff, this->bufferSize) == -1)
            cout << "Monitor " << this->id << " error in reading buff in receiveCredentials with errno=" << errno << endl;
        buff[this->bufferSize] = '\0';
        dir.append(buff);

    }
    this->generalDirectory = dir;

}

void Monitor::receiveCountries() {
    while (1) {
        int sizeOfCountry;
        if (read(readFD, &sizeOfCountry, sizeof(int)) == -1)
            cout << "Monitor " << this->id << " error in reading sizeOfCountry with errno=" << errno << endl;

        if (sizeOfCountry == -1)
            break;

        string country = "";
        for (int i = 0;i <= sizeOfCountry / this->bufferSize;i++) {
            char buff[this->bufferSize];
            if (read(readFD, &buff, this->bufferSize) == -1)
                cout << "Monitor " << this->id << " error in reading buff in receiveCountries with errno=" << errno << endl;
            buff[this->bufferSize] = '\0';
            country.append(buff);
            // cout << "Monitor " << this->id << " readed=" << buff << endl;
        }
        this->addNewCountry(country);
        cout << "Monitor " << this->id << " got country=" << country << endl;
    }
}

void Monitor::readFilesAndCreateStructures() {
    stringList* country = this->countries;

    while (country != NULL) {
        DIR* input;
        struct dirent* dir;
        string in = this->generalDirectory;
        in.append(country->getString());
        char* in2 = &in[0];
        input = opendir(in2);

        // cout << "Monitor " << this->id << " " << in2 << endl;
        if (input)
        {
            while ((dir = readdir(input)) != NULL)
            {
                string FILE = dir->d_name;
                if (FILE.compare("..") == 0 || FILE.compare(".") == 0)
                    continue;
                // cout << "Monitor " << this->id << " " << FILE << endl;
                string fullpath = in;
                fullpath.append("/");
                fullpath.append(FILE);
                this->addFromFile(fullpath);
            }
        }
        country = country->getNext();
    }
}

void Monitor::addFromFile(string filepath)
{
    // cout << filepath << endl;
    ifstream file(filepath);
    string line;
    string* words = new string[8];
    int length = 0;
    while (getline(file, line))
    {
        // cout << line << endl;
        splitString(&words, line, &length);
        addRecord(length, words, line);
        length = 0;
    }
    delete[] words;
    file.close();
}

void Monitor::addRecord(int length, string* words, string line)
{

    citizenRecord* citizen;

    if (this->checkSyntaxRecord("ERROR IN RECORD ", length, words, line)) // the record had syntax errors
        return;

    addNewVirus(words[5]);
    stringList* virus = this->viruses->search(words[5]);
    addNewCountry(words[3]);
    stringList* country = this->countries->search(words[3]);

    char status = 'n';
    string date = "";
    if (words[6].compare("YES") == 0)
    {
        status = 'y';
        date = words[7];
    }

    citizen = new citizenRecord(stoi(words[0]), words[1], words[2], country, stoi(words[4]), virus, status, date);
    checkNew(citizen);

    string result = "";
    citizenRecord* alreadyInTree = NULL;

    tree = this->tree->insert(tree, citizen, &alreadyInTree, &result, false); // insert in tree

    if (result.compare("NEW CITIZEN") == 0)
    {
        if (status == 'y')
        {
            this->blooms->getBloom(virus)->add(citizen->getID());
            this->skipLists->getVaccinated(virus)->add(citizen->getID(), citizen);
        }
        else if (status == 'n')
            this->skipLists->getNotVaccinated(virus)->add(citizen->getID(), citizen);
    }
    else if (result.compare("VIRUS ADDED TO CITIZEN") == 0)
    {
        if (status == 'y')
        {
            this->blooms->getBloom(virus)->add(alreadyInTree->getID());
            this->skipLists->getVaccinated(virus)->add(alreadyInTree->getID(), alreadyInTree);
        }
        else if (status == 'n')
            this->skipLists->getNotVaccinated(virus)->add(alreadyInTree->getID(), alreadyInTree);
    }
    else if (result.compare("WRONG CREDENTIALS") == 0)
    {
        if (length == 8)
            cout << "ERROR IN RECORD " << words[0] << " " << words[1] << " " << words[2] << " " << words[3] << " " << words[4] << " " << words[5] << " " << words[6] << " " << words[7] << endl;
        else
            cout << "ERROR IN RECORD " << words[0] << " " << words[1] << " " << words[2] << " " << words[3] << " " << words[4] << " " << words[5] << " " << words[6] << endl;

        cout << "ERROR= " << result << endl;
    }
    else if (result.compare("VIRUS DUPLICATE") == 0)
    {
        if (length == 8)
            cout << "ERROR IN RECORD " << words[0] << " " << words[1] << " " << words[2] << " " << words[3] << " " << words[4] << " " << words[5] << " " << words[6] << " " << words[7] << endl;
        else
            cout << "ERROR IN RECORD " << words[0] << " " << words[1] << " " << words[2] << " " << words[3] << " " << words[4] << " " << words[5] << " " << words[6] << endl;

        cout << "ERROR= " << result << endl;
    }
}

int Monitor::checkSyntaxRecord(string errorMessage, int length, string* words, string input)
{
    if (length < 7 || length > 8) // record must be only 7 or 8 words
    {
        errorMessage.pop_back();
        cout << errorMessage << " " << input << endl;
        cout << "ERROR= ARGUMENT LENGTH ERROR" << endl;
        return 1;
    }
    if (words[0].find_first_not_of("0123456789") != -1)
    {
        cout << errorMessage << input << endl;
        cout << "ERROR= ID ERROR" << endl;
        return 1;
    }
    if (stoi(words[0]) > 9999 && stoi(words[0]) <= 0) // id is 4 digits
    {
        cout << errorMessage << input << endl;
        cout << "ERROR= ID ERROR" << endl;
        return 1;
    }
    if (words[4].find_first_not_of("0123456789") != -1)
    {
        cout << errorMessage << input << endl;
        cout << "ERROR= AGE ERROR" << endl;
        return 1;
    }
    if (stoi(words[4]) > 120 && stoi(words[4]) <= 0) // age is from 1 to 120
    {
        cout << errorMessage << input << endl;
        cout << "ERROR= AGE ERROR" << endl;
        return 1;
    }
    if (words[6].compare("NO") != 0 && words[6].compare("YES") != 0) // the 6 word must only be NO or YES
    {
        cout << errorMessage << input << endl;
        cout << "ERROR= YES/NO ERROR" << endl;
        return 1;
    }
    if (length == 8 && (words[6].compare("NO") == 0)) // when we have 8 words the word must be YES
    {
        cout << errorMessage << words[0] << " " << words[1] << " " << words[2] << " " << words[3] << " " << words[4] << " " << words[5] << " " << words[6] << " " << words[7] << endl;
        cout << "ERROR= NO WITH DATE ERROR" << endl;
        return 1;
    }
    if (length == 7 && (words[6].compare("YES") == 0)) // when we have 7 words the word must be NO
    {
        cout << errorMessage << words[0] << " " << words[1] << " " << words[2] << " " << words[3] << " " << words[4] << " " << words[5] << " " << words[6] << endl;
        cout << "ERROR= YES WITHOUT DATE ERROR" << endl;
        return 1;
    }
    if (length == 8)
    {
        date checker(words[7]);
        if (!checker.isValid())
        {
            cout << errorMessage << words[0] << " " << words[1] << " " << words[2] << " " << words[3] << " " << words[4] << " " << words[5] << " " << words[6] << " " << words[7] << endl;
            cout << "ERROR= DATE FORMAT" << endl;
            return 1;
        }
    }
    return 0;
}

void Monitor::sendBlooms() {
    // send the viruses
    stringList* temp = this->viruses;
    while (temp != NULL) {
        string virus = temp->getString();
        char* to_tranfer = &virus[0];
        int sizeOfVirus = strlen(to_tranfer);

        if (write(writeFD, &sizeOfVirus, sizeof(int)) == -1)
            cout << "Error in writting sizeOfVirus with errno=" << errno << endl;
        int pos = 0;
        for (int i = 0;i <= strlen(to_tranfer) / this->bufferSize;i++) {
            if (write(writeFD, &to_tranfer[pos], this->bufferSize) == -1)
                cout << "Error in writting to_tranfer with errno=" << errno << endl;

            pos += this->bufferSize;
        }
        temp = temp->getNext();
    }
    int end = -1;
    if (write(writeFD, &end, sizeof(int)) == -1)
        cout << "Error in writting end with errno=" << errno << endl;

}

void Monitor::addNewVirus(string virusName)
{
    if (this->viruses->search(virusName) == NULL) // if we dont have that virus add it to the list of viruses
    {                                         // and make the bloom filter and the skiplist for that virus
        this->viruses = this->viruses->add(virusName);
        this->blooms = this->blooms->add(this->viruses);
        this->skipLists = this->skipLists->add(this->viruses);
    }
}

void Monitor::addNewCountry(string countryName)
{
    if (this->countries->search(countryName) == NULL) // if we dont have that country add it to the list of Countries
    {
        this->countries = this->countries->add(countryName);
    }
}

void Monitor::printAllCountries() {
    cout << "Monitor " << this->id << " has the countries" << endl;
    this->countries->print();
}

void Monitor::printAllViruses() {
    cout << "Monitor " << this->id << " has the viruses" << endl;
    this->viruses->print();
}