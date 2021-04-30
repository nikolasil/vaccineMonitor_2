#ifndef VACCINEMONITOR_H
#define VACCINEMONITOR_H

#include <iostream>
#include <string>

#include "DataStructures/monitorList/monitorList.h"
#include "DataStructures/bloomFilter/bloomFilter.h"
#include "DataStructures/stringList/stringList.h"
#include "DataStructures/fifoFDList/fifoFDList.h"

using namespace std;

class travelMonitor {
public:
    travelMonitor(int m, int b, int s, string dir);

    void createFIFOs();
    void createMonitors();
    void openFifos();
    void sendCredentials();
    void sendCountries();
    void receiveBlooms();
    void startMenu();

    void addCountryToMonitor(string c, int m);
    void addFD(int r, int w);

    void addNewVirus(string virusName);
    void addNewCountry(string countryName);

    void printAllViruses();

    void printCountryToMonitor();
private:
    int numMonitors;
    int bufferSize;
    int sizeOfBloom;
    string input_dir;

    fifoFDList* fifoFD;
    monitorList* countryToMonitor;

    stringList* viruses;
    stringList* countries;
    bloomFilterList* blooms;
};

#endif