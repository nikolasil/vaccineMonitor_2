#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/fcntl.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string>
#include <cstring>
using namespace std;

int main(int argc, char* argv[])
{
    if (argc != 3)
        cout << "Arguments must be 2" << endl;

    string pipe0 = argv[1];
    string pipe1 = argv[2];
    cout << "m " << pipe0.c_str() << " " << pipe1.c_str() << endl;
    int bufferSize;
    int bloomSize;

    int fd0 = open(pipe0.c_str(), O_RDONLY);
    int fd1 = open(pipe1.c_str(), O_WRONLY);

    if (read(fd0, &bufferSize, sizeof(int)) == -1)
        cout << "Error in reading" << endl;
    if (read(fd0, &bloomSize, sizeof(int)) == -1)
        cout << "Error in reading" << endl;


    // cout << bufferSize << " " << bloomSize << endl;
    while (1) {
        fd = open(pipe0.c_str(), O_RDONLY);
        int size;
        if (read(fd0, &size, sizeof(int)) == -1)
            cout << "Error in reading" << endl;

        if (size == -1)
            break;

        string output = "";
        for (int i = 0;i <= size / bufferSize;i++) {
            char buff[bufferSize];
            if (read(fd0, &buff, bufferSize) == -1)
                cout << "Error in reading" << endl;
            buff[bufferSize] = '\0';
            output.append(buff);
            cout << "readed " << buff << endl;
        }
        cout << "Got " << output << endl;

    }
    close(fd0);
    close(fd1);
    cout << "Ended" << endl;
    return 0;
}