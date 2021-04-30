#include <iostream>
#include <string>

#include "travelMonitor.h"
#include "util.h"

using namespace std;

int main(int argc, char* argv[])
{
    int numMonitors;
    int bufferSize;
    int sizeOfBloom;
    string input_dir;
    if (!checkArguments(argc, argv, numMonitors, bufferSize, sizeOfBloom, input_dir))
        return 1;

    travelMonitor mainMonitor = travelMonitor(numMonitors, bufferSize, sizeOfBloom, input_dir);
    mainMonitor.createFIFOs();
    mainMonitor.createMonitors();
    mainMonitor.openFifos();
    mainMonitor.sendCredentials();
    mainMonitor.sendCountries();
    mainMonitor.receiveBlooms();
    mainMonitor.printAllViruses();
    return 0;
}