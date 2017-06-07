#! /usr/bin/env cpx
#^ <boost/circular_buffer.hpp>
#|

/*
    Large clusters of in include files make the precompiled header explode.
    Ofcause you can include them locally. But then they tend to slowdown the
    hash phase while possibly never change.
    Instead you inject them prior to all locally include files by use of #^ 
    and skip hashing. Compilation will be (slow) as usual. But it stays away from
    the precompiled header that effects all scripts. And once (even a script using boost)
    is compiled, the hash phase is as fast as in other cases (presuming the actual script code
    is small (as usual).
*/

int main()
{
  typedef boost::circular_buffer<int> circular_buffer;
  circular_buffer cb{3};

  INFO(VARVAL(cb.capacity()));
  INFO(VARVAL( cb.size()));

  cb.push_back(0);
  cb.push_back(1);
  cb.push_back(2);

  INFO(VARVAL(cb.size()));

  cb.push_back(3);
  cb.push_back(4);
  cb.push_back(5);

  INFO(VARVAL(cb.size()));

  for (decltype(cb.size()) i=0;i<cb.size();++i){
    INFO(VARVAL(i),VARVAL(cb[i]));
  }
}

