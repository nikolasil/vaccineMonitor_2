#ifndef VACCINEMONITOR_H
#define VACCINEMONITOR_H

#include <iostream>
#include <string>

#include "DataStructures/monitorCountryPairList/monitorCountryPairList.h"
#include "DataStructures/bloomFilter/bloomFilter.h"
#include "DataStructures/stringList/stringList.h"
#include "DataStructures/monitorList/monitorList.h"

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
    void sendDone();
    void startMenu();

    void sendStr(int monitor, string str);
    string receiveStr(int monitor);
    string receiveManyStr(int monitor, int* end);

    void addCountryToMonitor(string c, int m);
    void addMonitor(int pid, int id);
    void addFdToMonitor(int m, int r, int w);

    void addNewVirus(string virusName);
    void addNewCountry(string countryName);

    void printAllViruses();

    void printCountryToMonitor();
private:
    int numMonitors;
    int bufferSize;
    int sizeOfBloom;
    string input_dir;

    monitorList* monitors;
    monitorCountryPairList* countryToMonitor;

    stringList* viruses;
    stringList* countries;
    bloomFilterList* blooms;
};

#endif