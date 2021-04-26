#ifndef MONITOR_H
#define MONITOR_H

#include <iostream>
#include <string>

// #include "DataStructures/bloomFilter/bloomFilter.h"
#include "DataStructures/stringList/stringList.h"

using namespace std;

class Monitor {
public:
    Monitor(string r, string w);
    ~Monitor();

    void receiveCredentials();
    void receiveCountries();

    void addCountry(string c);
    void addVirus(string v);

    void printAllCountries();
    void printAllViruses();
private:
    int id;

    string readFifo;
    string writeFifo;
    int readFD;
    int writeFD;

    int bufferSize;
    int bloomSize;

    // bloomFilterList* blooms;
    stringList* viruses;
    stringList* countries;
};

#endif