#include <iostream>
#include "posint.h"
using namespace std;

int main() {

  // Set base to 16^3
  // This causes numbers to display in hex.  
  // However, base must be no greater than 2^15 when
  // using 32-bit integers, so we can only fit 3 hex 
  // digits (or 12 bits) per 32-bit int.  It is more
  // space-efficient to use the default, but then the
  // numbers will print in binary.
  
  PosInt::setBase(16,3);
  
  // PosInt::setBase(10, 4);

  // x = 2^128
  PosInt x(2);
  PosInt y(128);
  x.pow(y);

  // z = random number between 0 and 2^128 - 1
  //  srand(0);
  srand(time(NULL));
  PosInt z;
  z.rand(x);

  cout << "x = 2^128 = " << x << endl;
  cout << "x         = ";
  x.print_array(cout);
  cout << endl << endl;

  cout << "z = rand(x) = " << z << endl;
  cout << "z           = ";
  z.print_array(cout);
  cout << endl << endl;

  // z = z^2
  z.mul(z);

  cout << "z^2 = " << z << endl;

  // z = z mod x
  z.mod(x);

  cout << "z^2 mod x = " << z << endl;
}
  
