#include <iostream>

using namespace std;


class A {
public:
    A(int a = 1) { cout << "A constructor: " << a << endl; }
    A(char c) { cout << "A constructor: " << c << endl; }
    /*virtual*/ ~A() { cout << "A destructor" << endl; }
};

class B : public A{
public:
    B() { cout << "B constructor" << endl; }
    B(char c) { cout << "B constructor: " << c << endl; }
    ~B() { cout << "B destructor" << endl; }
};

int main(int argc, char *argv[]) {
    // A *a = new B('b');
    // delete a;
    /* result - without A virtual constructor
     A constructor: 1
     B constructor: b
     A destructor

       result - with A virtual destructor
     A constructor: 1
     B constructor: b
     B destructor
     A destructor

     */


    B *a = new B('b');
    /*
       result - destructor of parent will be called?

       A constructor: 1
       B constructor: b
       B destructor
       A destructor

    */
    delete a;
    return 0;
}
