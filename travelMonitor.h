#ifndef VACCINEMONITOR_H
#define VACCINEMONITOR_H

#include <iostream>
#include <string>

using namespace std;

class travelMonitor {
public:
    travelMonitor(int m, int b, int s, string dir);

    int createFIFOs();
    void createMonitors();
    void openFifos();
    void sendFilesToMonitors();
    void receiveBlooms();
    void startMenu();
private:
    int numMonitors;
    int bufferSize;
    int sizeOfBloom;
    string input_dir;
};

#endif