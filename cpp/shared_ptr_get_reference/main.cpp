#include <iostream>
#include <unistd.h>
#include <memory>
#include <stdlib.h>

using namespace std;

class Struct {
public:
    Struct(int i) {
        c = i;
        cout << "constructor: " << c << endl;
        buf = new char[i];
    }
    ~Struct() {
        delete [] buf;
        cout << "destructor: " << c << endl;
    }
    int c;
    char *buf;
};

template <typename T>
shared_ptr<T> create_object(int i) {
    return shared_ptr<T>(new T(i));
}

void go() {
    shared_ptr<Struct> t1 = create_object<Struct>(10);
    shared_ptr<Struct> t2 = create_object<Struct>(5);

    cout << "1: " << t1->c << endl;
    cout << "2: " << t2->c << endl;
}

int main(int argc, char *argv) {
    go();
    cout << "sleep" << endl;
    usleep(1000000);
    return 0;
}
