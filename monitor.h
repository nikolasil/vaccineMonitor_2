#ifndef MONITOR_H
#define MONITOR_H

#include <iostream>
#include <string>
#include <signal.h>

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
    void sendBlooms();
    void receiveDone();

    void addFromFile(string filePath);
    void addRecord(int length, string* words, string line);
    // void signal_handler_2(int signo);
    void addNewVirus(string virusName);
    void addNewCountry(string countryName);
    int addNewFile(string file);

    int checkSyntaxRecord(string errorMessage, int length, string* words, string input);

    void sendStr(string str);
    string receiveStr();
    string receiveManyStr(int* end);

    void printAllCountries();
    void printAllViruses();

    void waitForSignals();
private:
    int id;
    struct sigaction handler;

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
    stringList* filesReaded;
};

#endif