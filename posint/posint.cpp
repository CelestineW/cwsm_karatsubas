#include <cctype>
#include <cstdlib>
#include <string>
#include <sstream>
#include <math.h>
#include <ctime>
#include "posint.h"

using namespace std;

/******************** BASE ********************/

int PosInt::B = 0x8000;
int PosInt::Bbase = 2;
int PosInt::Bpow = 15;

void PosInt::setBase(int base, int pow) {
  Bbase = base;
  Bpow = pow;
  B = base;
  while (pow > 1) {
    B *= Bbase;
    --pow;
  }
}

/******************** I/O ********************/

void PosInt::read (const char* s) {
  string str(s);
  istringstream sin (str);
  read(sin);
}

void PosInt::set(int x) {
  digits.clear();

  if (x < 0)
    throw MPError("Can't set PosInt to negative value");

  while (x > 0) {
    digits.push_back(x % B);
    x /= B;
  }
}

void PosInt::set (const PosInt& rhs) {
  if (this != &rhs)
    digits.assign (rhs.digits.begin(), rhs.digits.end());
}

void PosInt::print_array(ostream& out) const {
  out << "[ls";
  for (int i=0; i<digits.size(); ++i)
    out << ' ' << digits[i];
  out << " ms]";
}

void PosInt::print(ostream& out) const {
  if (digits.empty()) out << 0;
  else {
    int i = digits.size()-1;
    int pow = B/Bbase;
    int digit = digits[i];
    for (; digit < pow; pow /= Bbase);
    while (true) {
      for (; pow>0; pow /= Bbase) {
        int subdigit = digit / pow;
        if (subdigit < 10) out << subdigit;
        else out << (char)('A' + (subdigit - 10));
        digit -= subdigit*pow;
      }
      if (--i < 0) break;
      digit = digits[i];
      pow = B/Bbase;
    }
  }
}

void PosInt::read (istream& in) {
  vector<int> digstack;
  while (isspace(in.peek())) in.get();
  int pow = B/Bbase;
  int digit = 0;
  int subdigit;
  while (true) {
    int next = in.peek();
    if (isdigit(next)) subdigit = next-'0';
    else if (islower(next)) subdigit = next - 'a' + 10;
    else if (isupper(next)) subdigit = next - 'A' + 10;
    else subdigit = Bbase;
    if (subdigit >= Bbase) break;
    digit += pow*subdigit;
    in.get();
    if (pow == 1) {
      digstack.push_back (digit);
      pow = B/Bbase;
      digit = 0;
    }
    else pow /= Bbase;
  }
  pow *= Bbase;
  if (pow == B && !digstack.empty()) {
    pow = 1;
    digit = digstack.back();
    digstack.pop_back();
  }
  int pmul = B/pow;
  digits.assign (1, digit/pow);
  for (int i=digstack.size()-1; i >= 0; --i) {
    digits.back() += (digstack[i] % pow) * pmul;
    digits.push_back (digstack[i] / pow);
  }
  normalize();
}

int PosInt::convert () const {
  int val = 0;
  int pow = 1;
  for (int i = 0; i < digits.size(); ++i) {
    val += pow * digits[i];
    pow *= B;
  }
  return val;
}

ostream& operator<< (ostream& out, const PosInt& x) { 
  x.print(out); 
  return out;
}

istream& operator>> (istream& in, PosInt& x) { 
  x.read(in);
  return in;
}

/******************** RANDOM NUMBERS ********************/

// Produces a random number between 0 and n-1
static int randomInt (int n) {
  int max = RAND_MAX - ((RAND_MAX-n+1) % n);
  int r;
  do { r = rand(); }
  while (r > max);
  return r % n;
}

// Sets this PosInt to a random number between 0 and x-1
void PosInt::rand (const PosInt& x) {
  if (this == &x) {
    PosInt xcopy(x);
    rand(xcopy);
  }
  else {
    PosInt max;
    max.digits.assign (x.digits.size(), 0);
    max.digits.push_back(1);
    PosInt rem (max);
    rem.mod(x);
    max.sub(rem);
    do {
      digits.resize(x.digits.size());
      for (int i=0; i<digits.size(); ++i)
        digits[i] = randomInt(B);
      normalize();
    } while (compare(max) >= 0);
    mod(x);
  }
}

/******************** UTILITY ********************/

// Removes leading 0 digits
void PosInt::normalize () {
  int i;
  for (i = digits.size()-1; i >= 0 && digits[i] == 0; --i);
  if (i+1 < digits.size()) digits.resize(i+1);
}

bool PosInt::isEven() const {
  if (B % 2 == 0) return digits.empty() || (digits[0] % 2 == 0);
  int sum = 0;
  for (int i = 0; i < digits.size(); ++i)
    sum += digits[i] % 2;
  return sum % 2 == 0;
}

// Result is -1, 0, or 1 if a is <, =, or > than b,
// up to the specified length.
int PosInt::compareDigits (const int* a, int alen, const int* b, int blen) {
  int i = max(alen, blen)-1;
  for (; i >= blen; --i) {
    if (a[i] > 0) return 1;
  }
  for (; i >= alen; --i) {
    if (b[i] > 0) return -1;
  }
  for (; i >= 0; --i) {
    if (a[i] < b[i]) return -1;
    else if (a[i] > b[i]) return 1;
  }
  return 0;
}

// Result is -1, 0, or 1 if this is <, =, or > than rhs.
int PosInt::compare (const PosInt& x) const {
  if (digits.size() < x.digits.size()) return -1;
  else if (digits.size() > x.digits.size()) return 1;
  else if (digits.size() == 0) return 0;
  else return compareDigits
    (&digits[0], digits.size(), &x.digits[0], x.digits.size());
}

/******************** ADDITION ********************/

// Computes dest += x, digit-wise
// REQUIREMENT: dest has enough space to hold the complete sum.
void PosInt::addArray (int* dest, const int* x, int len) {
  int i;
  for (i=0; i < len; ++i)
    dest[i] += x[i];

  for (i=0; i+1 < len; ++i) {
    if (dest[i] >= B) {
      dest[i] -= B;
      ++dest[i+1];
    }
  }

  for ( ; dest[i] >= B; ++i) {
    dest[i] -= B;
    ++dest[i+1];
  }
}

// this = this + x
void PosInt::add (const PosInt& x) {
  digits.resize(max(digits.size(), x.digits.size())+1, 0);
  addArray (&digits[0], &x.digits[0], x.digits.size());
  normalize();
}

/******************** SUBTRACTION ********************/

// Computes dest -= x, digit-wise
// REQUIREMENT: dest >= x, so the difference is non-negative
void PosInt::subArray (int* dest, const int* x, int len) {

  int i = 0;
  for ( ; i < len; ++i)
    dest[i] -= x[i];

  for (i=0; i+1 < len; ++i) {
    if (dest[i] < 0) {
      dest[i] += B;
      --dest[i+1];
    }
  }

  for ( ; dest[i] < 0; ++i) {
    dest[i] += B;
    --dest[i+1];
  }

 // cout << "sub- Dest : " << *dest << endl;
}

// this = this - x
void PosInt::sub (const PosInt& x) {
  if (compare(x) < 0)
    throw MPError("Subtraction would result in negative number");
  else if (x.digits.size() > 0) {
    subArray (&digits[0], &x.digits[0], x.digits.size());
    normalize();
  }
}

/******************** MULTIPLICATION ********************/

// Computes dest = x * y, digit-wise.
// x has length xlen and y has length ylen.
// dest must have size (xlen+ylen) to store the result.
// Uses standard O(n^2)-time multiplication.
void PosInt::mulArray 
  (int* dest, const int* x, int xlen, const int* y, int ylen) 
{
  for (int i=0; i<xlen+ylen; ++i) dest[i] = 0;
  for (int i=0; i<xlen; ++i) {
    for (int j=0; j<ylen; ++j) {
      dest[i+j] += x[i] * y[j];
      dest[i+j+1] += dest[i+j] / B;
      dest[i+j] %= B;
    }
  }
}

// Computes dest = x * y, digit-wise, using Karatsuba's method.
// x and y have the same length (len)
// dest must have size (2*len) to store the result.
void PosInt::fastMulArray (int* dest, const int* x, const int* y, int len) {


  // Karatsuba's method goes here

  // Base Case - when lengths are = 1
  if ( len == 1 ){
      
        // Calls mullArray with single digit numbers
        mulArray( dest , x , len , y , len);
       
  }

  // Recursive Call
  else{
        
        // Splitting terms in half
        int halves = len / 2;
        
        // Creating int arrays with len * 2
        // for space to add the two terms later
        int * x_high = new int[len * 2];
        int * x_low = new int[len * 2];
       
        int high_i = 0;
        int low_i = 0;
        
        // Copying over values of x into respective
        // high and lows
        for (int i  = 0 ; i < len ; i++ ){
                if (i < halves) {
                        x_low[low_i] = x[i];
                        low_i ++;
                }
                else{
                        x_high[high_i] = x[i];
                        high_i ++;
                }
        }


        // Padding x's with 0s
        for (int i = high_i; i < len; i++){
                x_high[high_i] = 0;
        }
        for (int i = low_i; i < len; i++){
                x_low[low_i] = 0 ;
        }

        int * y_high = new int[len * 2];
        int * y_low = new int[len * 2];
         
        high_i = 0;
        low_i = 0;
        
        // Copying over y values to high and lows
        for (int i  = 0 ; i < len ; i++ ){

                if (i < halves) {
                        y_low[low_i] = y[i];
                        low_i ++;
                }
                else{
                        y_high[high_i] = y[i];
                        high_i ++;
                }
        }
  
        // Padding for zeros
        for (int i = high_i; i < len; i ++){
                y_high[high_i] = 0;
        }
        for (int i = low_i; i < len; i ++){
                y_low[low_i] = 0; 
        }


        // First recursive call - Highs
        
        // When a and e are multiplied 
        // to the base, it can be at most
        // len * 2 + 1 
        int * a = new int[len * 3];
        int * d = new int[len * 3];
        int * e = new int[len * 3];

        // Setting to zeros, and padding
        for (int i = 0 ; i < len * 2; i++){
                a[i] = 0;
                d[i] = 0;
                e[i] = 0;
        }

        // Recursive calls for a and d
        fastMulArray(a, x_high, y_high, halves);
        fastMulArray(d, x_low, y_low, halves);

        // Preparing values for recursive call for e
        addArray(x_low, x_high, len * 2);
        addArray(y_low, y_high, len * 2);

        // When e can be two numbers of different
        // digit sizes, we split and call according to
        // the nearest power or 2 number
	int e_len = len-1;
	while( x_low[e_len] == 0 && y_low[e_len] == 0){
		e_len--;
	}
	e_len = (e_len+1 > (len/2) )? len: halves;

        // recursive call for e
        fastMulArray(e, x_low, y_low, e_len); 


        // Base nultiplication for
        // B ^ len and B ^ halves
        int base_len = B;        
        for (int i = 1; i < len ; i ++ ){
                base_len *= B; 
        }
        
        int base_half = B;
        for (int i = 1; i < halves ; i ++ ){
                base_half *= B;
        }

        // Math for prod=
        // (a * B ^len) + ((e-a-d) * B ^ halves)+ d
        
        // (e-a-d) 
        subArray(e, a, len * 2);
        subArray(e, d, len * 2);        

        // a and e multiplied to its base
        mulDigit(a, base_len, len * 2);
        mulDigit(e, base_half, len * 2);
        
        // adding all to dest ptr
        addArray(dest, a, len * 2);
        addArray(dest, e, len * 2);
        addArray(dest, d, len * 2);

	/* clean up */
        delete[] a;
        delete[] e;
        delete[] d;
        delete[] x_high;
        delete[] x_low;
        delete[] y_high;
        delete[] y_low;

  }
  


}

// this = this * x
void PosInt::mul(const PosInt& x) {
  if (this == &x) {
    PosInt xcopy(x);
    mul(xcopy);
    return;
  }

  int mylen = digits.size();
  int xlen = x.digits.size();
  if (mylen == 0 || xlen == 0) {
    set(0);
    return;
  }

  int* mycopy = new int[mylen];
  for (int i=0; i<mylen; ++i) mycopy[i] = digits[i];
  digits.resize(mylen + xlen);
  mulArray(&digits[0], mycopy, mylen, &x.digits[0], xlen);

  normalize();
  delete [] mycopy;
}

// this = this * x, using Karatsuba's method
void PosInt::fastMul(const PosInt& x) {	

	clock_t start = clock();	

	cout << "clock start " << start << endl;


        // For numbers with different digit amounts
        int length = max(digits.size(), x.digits.size());

        // reserving space for large multiplication
        int* dest = new int[length * 2]; 
       
        // Padding
        for ( int i = 0; i < length * 2; i ++){
                dest[i] = 0;
        }

        // For powers of 2, finding and padding
        // so each split will be an even number
        int digit_length = 2;

        for (int i = 0; i < length/2 ; i ++){
            digit_length *=2;
        }
        
        int * x_iarray = new int[digit_length];
        int * y_iarray = new int[digit_length];

        // Copying over values and padding
        for (int i = 0 ; i < digit_length ; i++){
            if (i < x.digits.size() ){
                x_iarray[i] = x.digits[i];
            }
            else{
                x_iarray[i] = 0;
            }
        }

        // Copying over values and padding
        for (int i = 0 ; i < digit_length ; i++){
            if (i < digits.size()) {
                y_iarray[i] = this->digits[i];
            }
            else{
                y_iarray[i] = 0;
            }
        }
 
        // Call to recursive function
        fastMulArray(dest, x_iarray, y_iarray, digit_length);
 
        cout << "Final Answer: " ;
       	int i = (length*2) - 1;
	for ( ; i >= 0; i-- ) {
                cout << dest[i];
        }

	clock_t stop = clock();
	
	cout << "clock stop " << stop << endl;

	cout << endl;

	double exec_time = (stop-start)/(CLOCKS_PER_SEC);
	cout << "Execution time: " << exec_time << endl;
	
        cout << endl;
	cout << endl;
 	       
}

/******************** DIVISION ********************/

// Computes dest = dest * d, digit-wise
// REQUIREMENT: dest has enough space to hold any overflow.
void PosInt::mulDigit (int* dest, int d, int len) {
  int i;
  for (i=0; i<len; ++i)
    dest[i] *= d;
  for (i=0; i+1<len; ++i) {
    dest[i+1] += dest[i] / B;
    dest[i] %= B;
  }
  for (; dest[i] >= B; ++i) {
    dest[i+1] += dest[i] / B;
    dest[i] %= B;
  }
}

// Computes dest = dest / d, digit-wise, and returns dest % d
int PosInt::divDigit (int* dest, int d, int len) {
  int r = 0;
  for (int i = len-1; i >= 0; --i) {
    dest[i] += B*r;
    r = dest[i] % d;
    dest[i] /= d;
  }
  return r;
}

// Computes division with remainder, digit-wise.
// REQUIREMENTS: 
//   - length of q is at least xlen-ylen+1
//   - length of r is at least xlen
//   - q and r are distinct from all other arrays
//   - most significant digit of divisor (y) is at least B/2
void PosInt::divremArray 
  (int* q, int* r, const int* x, int xlen, const int* y, int ylen)
{
  // Copy x into r
  for (int i=0; i<xlen; ++i) r[i] = x[i];

  // Create temporary array to hold a digit-multiple of y
  int* temp = new int[ylen+1];

  int qind = xlen - ylen;
  int rind = xlen - 1;

  q[qind] = 0;
  while (true) {
    // Do "correction" if the quotient digit is off by a few
    while (compareDigits (y, ylen, r + qind, xlen-qind) <= 0) {
      subArray (r + qind, y, ylen);
      ++q[qind];
    }

    // Test if we're done, otherwise move to the next digit
    if (qind == 0) break;
    --qind;
    --rind;

    // (Under)-estimate the next digit, and subtract out the multiple.
    int quoest = (r[rind] + B*r[rind+1]) / y[ylen-1] - 2;
    if (quoest <= 0) q[qind] = 0;
    else {
      q[qind] = quoest;
      for (int i=0; i<ylen; ++i) temp[i] = y[i];
      temp[ylen] = 0;
      mulDigit (temp, quoest, ylen+1);
      subArray (r+qind, temp, ylen+1);
    }
  }

  delete [] temp;
}

// Computes division with remainder. After the call, we have
// x = q*y + r, and 0 <= r < y.
void PosInt::divrem (PosInt& q, PosInt& r, const PosInt& x, const PosInt& y) {
  if (y.digits.empty()) throw MPError("Divide by zero");
  else if (&q == &r) throw MPError("Quotient and remainder can't be the same");
  else if (x.compare(y) < 0) {
    r.set(x);
    q.set(0);
    return;
  }
  else if (y.digits.size() == 1) {
    int divdig = y.digits[0];
    q.set(x);
    r.set (divDigit (&q.digits[0], divdig, q.digits.size()));
  }
  else if (2*y.digits.back() < B) {
    int ylen = y.digits.size();
    int fac = 1;
    int* scaley = new int[ylen];
    for (int i=0; i<ylen; ++i) scaley[i] = y.digits[i];
    do {
      mulDigit (scaley, 2, ylen);
      fac *= 2;
    } while (2*scaley[ylen-1] < B);

    int xlen = x.digits.size()+1;
    int* scalex = new int[xlen];
    for (int i=0; i<xlen-1; ++i) scalex[i] = x.digits[i];
    scalex[xlen-1] = 0;
    mulDigit (scalex, fac, xlen);
    q.digits.resize(xlen - ylen + 1);
    r.digits.resize(xlen);
    divremArray (&q.digits[0], &r.digits[0], scalex, xlen, scaley, ylen);
    divDigit (&r.digits[0], fac, xlen);
    delete [] scaley;
    delete [] scalex;
  }
  else {
    int xlen = x.digits.size();
    int ylen = y.digits.size();
    int* xarr = NULL;
    int* yarr = NULL;
    if (&x == &q || &x == &r) {
      xarr = new int[xlen];
      for (int i=0; i<xlen; ++i) xarr[i] = x.digits[i];
    }
    if (&y == &q || &y == &r) {
      yarr = new int[ylen];
      for (int i=0; i<ylen; ++i) yarr[i] = y.digits[i];
    }
    q.digits.resize(xlen - ylen + 1);
    r.digits.resize(xlen);
    divremArray (&q.digits[0], &r.digits[0], 
      (xarr == NULL ? (&x.digits[0]) : xarr), xlen, 
      (yarr == NULL ? (&y.digits[0]) : yarr), ylen);
    if (xarr != NULL) delete [] xarr;
    if (yarr != NULL) delete [] yarr;
  }
  q.normalize();
  r.normalize();
}

/******************** EXPONENTIATION ********************/

// this = this ^ x
void PosInt::pow (const PosInt& x) {
  static const PosInt one(1);

  if (this == &x) {
    PosInt xcopy(x);
    pow(xcopy);
    return;
  }

  PosInt mycopy(*this);
  set(1);
  for (PosInt i(0); i.compare(x) < 0; i.add(one)) {
    mul(mycopy);
  }
}

// result = a^b mod n
void powmod (PosInt& result, const PosInt& a, const PosInt& b, const PosInt& n) {

}

/******************** GCDs ********************/

// this = gcd(x,y)
void PosInt::gcd (const PosInt& x, const PosInt& y) {
  PosInt b(y);
  set(x);
  while (!b.isZero()) {
    PosInt r (*this);
    r.mod(b);
    set(b);
    b.set(r);
  }
}

// this = gcd(x,y) = s*x - t*y
// NOTE THE MINUS SIGN! This is required so that both s and t are
// always non-negative.
void PosInt::xgcd (PosInt& s, PosInt& t, const PosInt& x, const PosInt& y) {
  return;
}

/******************** Primality Testing ********************/

// returns true if this is PROBABLY prime
bool PosInt::MillerRabin () const {
  return false;
}

