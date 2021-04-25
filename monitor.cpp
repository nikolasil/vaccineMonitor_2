#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/fcntl.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string>

using namespace std;

int main(int argc, char* argv[])
{
    if (argc != 3)
        cout << "Arguments must be 2" << endl;

    string pipe0 = argv[1];
    string pipe1 = argv[2];

    cout << "Monitor opening " << pipe0 << endl;
    int fd = open(pipe0.c_str(), O_RDONLY);
    cout << "Monitor opened " << pipe0 << endl;
    char num[3];
    if (read(fd, &num, 3) == -1) {
        cout << "Error in reading" << endl;
    }
    cout << "Monitor " << getpid() << " got the char " << num << endl;
    close(fd);
    return 0;
}