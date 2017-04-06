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
  
  PosInt::setBase(10,2);

  PosInt a(4);
  PosInt b(6);
  cout << "Multiplying " << a << " and " << b << endl;
  a.fastMul(b);

  PosInt c(22);
  PosInt d(45);
  cout << "Multiplying " << c << " and " << d << endl;
  c.fastMul(d);
 
  PosInt e(236);
  PosInt f(147);
  cout << "Multiplying " << e << " and " << f << endl;
  e.fastMul(f);

  PosInt g(8365);
  PosInt h(2952);
  cout << "Multiplying " << g << " and " << h << endl;
  g.fastMul(h);

  PosInt i(23452);
  PosInt j(98324);
  cout << "Multiplying " << i << " and " << j << endl;
  i.fastMul(j);

  PosInt k(346953);
  PosInt l(983467);
  cout << "Multiplying " << k << " and " << l << endl;
  k.fastMul(l);

  PosInt m(1802313);
  PosInt n(7532679);
  cout << "Multiplying " << m << " and " << n << endl;
  m.fastMul(n);

  PosInt o(49275630);
  PosInt p(93720571);
  cout << "Multiplying " << o << " and " << p << endl;
  o.fastMul(p);

  PosInt q(235168734);
  PosInt r(203985673);
  cout << "Multiplying " << q << " and " << 2 << endl;
  q.fastMul(r);

  PosInt s(7205629265);
  PosInt t(4238741005);
  cout << "Multiplying " << s << " and " << t << endl;
  s.fastMul(t);


/*
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
*/
}
  
