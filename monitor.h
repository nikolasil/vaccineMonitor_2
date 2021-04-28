#ifndef MONITOR_H
#define MONITOR_H

#include <iostream>
#include <string>

#include "DataStructures/bloomFilter/bloomFilter.h"
#include "DataStructures/binaryAvlTree/tree.h"
#include "DataStructures/skipList/skipList.h"
#include "DataStructures/stringList/stringList.h"

using namespace std;

class Monitor {
public:
    Monitor(string r, string w);
    ~Monitor();

    void receiveCredentials();
    void receiveCountries();
    void readFilesAndCreateStructures();

    void addFromFile(string filePath);
    void addRecord(int length, string* words, string line);

    // void addCountry(string c);
    // void addVirus(string v);

    void addNewVirus(string virusName);
    void addNewCountry(string countryName);

    int checkSyntaxRecord(string errorMessage, int length, string* words, string input);

    void printAllCountries();
    void printAllViruses();
private:
    int id;
    string generalDirectory;
    string readFifo;
    string writeFifo;
    int readFD;
    int writeFD;

    int bufferSize;
    int bloomSize;
    treeNode* tree;
    bloomFilterList* blooms;
    skipList_List* skipLists;
    stringList* viruses;
    stringList* countries;
};

#endif