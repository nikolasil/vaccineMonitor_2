#include <iostream>
#include <string>

using namespace std;

#include "monitor.h"
#include "util.h"


int main(int argc, char* argv[])
{
    string pipe0 = argv[0];
    string pipe1 = argv[1];
    // cout << argc << endl;
    if (argc != 2)
        cout << "Arguments must be 2" << endl;
    // check if pipes exists !!!!!!!!!!!!!!!!!!!!!!
    Monitor monitor(pipe0, pipe1);
    monitor.receiveCredentials();
    monitor.receiveCountries();
    monitor.readFilesAndCreateStructures();
    monitor.sendBlooms();

    return 0;
}