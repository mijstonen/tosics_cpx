 int fib(int n)
 {
    int x,y;
    if (n<2) return n;

|| task shared(x)
   {
        x=fib(n-1);
   }
|| task shared(y)
   {
        y=fib(n-2);
   }
|| taskwait
    return x+y;
 }
