#!/usr/local/bin/cpx

  REPEAT_MAKE(Horizontal,xa|xb|xc|xd|xe|xf|xg|xh)
  REPEAT_MAKE(Vertical,ya|yb|yc|yd|ye|yf|yg|yh)
  REPEAT_MAKE(Depth,za|zb|zc|zd|ze|zf|zg|zh)


#!
  REPEAT_RUNTIME_RANGE(Horizontal,_literals)
  REPEAT_RUNTIME_RANGE(Vertical,_literals)
  REPEAT_RUNTIME_RANGE(Depth,_literals)

  INFO(VARVALS( Horizontal_literals , Vertical_literals , Depth_literals ));

  // nested compiletime loop
  REPEAT_FOREACH(Horizontal,X)
   REPEAT_FOREACH(Vertical,Y)
    REPEAT_FOREACH(Vertical,Z)
     cout<< REPEAT_ITEM_STRING(X) " : " REPEAT_ITEM_STRING(Y) " : " REPEAT_ITEM_STRING(Z) << endl;
    REPEAT_END
   REPEAT_END
  REPEAT_END
