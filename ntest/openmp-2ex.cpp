#!/usr/local/bin/cpx
#!//main
||  parallel
    {
        int ID= omp_get_thread_num();
        printf("%d\n", ID);
    }

    const int N = 1000000;
    int a[N];
    long sum = 0;

    // Initialize the array
    for (int i = 0; i < N; ++i) {
       a[i] = i;
    }

||  parallel for reduction(+:sum)
    for (int i = 0; i < N; ++i) {
        sum += a[i];
    }
    cout << "Sum: " << sum << endl;

||  parallel sections
    {

   <?>  for($section__number=1;$section__number<=4;++$section__number)
   <{>
||      section
        {
           auto msg=STREAM2STR("Section <?=$section__number?> executed by thread " << omp_get_thread_num()<< endl);
||         critical
           {
               msg+= "123\n";
               cout << msg<< endl;
           }
        }
   <}>
    } // parallel sections
