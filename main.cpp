#include <iostream>
#include <string>

#include "vaccineMonitor.h"
#include "util.h"

using namespace std;

int main(int argc, char* argv[])
{
    string filepath;
    int bloomSize;
    if (!checkArguments(argc, argv, filepath, bloomSize))
        return 1;

    vaccineMonitor monitor = vaccineMonitor(bloomSize);
    monitor.addFromFile(filepath);
    monitor.startMenu();
    return 0;
}