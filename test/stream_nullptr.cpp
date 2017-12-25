#!/usr/bin/env cpx
#|

// Demonstrates tosics::util::Is_null() and std::is_null_pointer() and std::is_pointer() .


template <typename Value_T>
  void show(char const* _lab, Value_T _v)
{
   FAKE_USE(_v);
   INFO(_lab,VARVAL(_v));
   INFO(_lab,VARVAL(Is_null(_v)));
   INFO(_lab,VARVAL(is_null_pointer<Value_T>::value));
   INFO(_lab,VARVAL(is_pointer<Value_T>::value));
   INFO();


}


#(
   if ( nullptr )
  {
     INFO("Lying nullptr");
  }
  else
  {
     INFO("Truth nullptr");
  }
  INFO();



  show("int             ",1);
  show("double          ",0.00999);

  show("char const*     ","Michel");
  // Trouble here:
  show("nullptr         ",nullptr);
  show("(void*)nullptr ",static_cast<void*>(nullptr));
  int c=7;
  void* pc=&c;
  FAKE_USE(c,pc);
  show("(void*)&pc ",static_cast<void*>(pc));
#)
