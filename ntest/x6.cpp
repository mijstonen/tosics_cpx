#!/usr/bin/env cpx
#|
#^<complex>

void print(auto const & item)
{
    cout<<item<<endl;
}


#(
    using namespace std::complex_literals;
    print("Hello");
    print(4);
    print(complex<double>(3,4));
#)
