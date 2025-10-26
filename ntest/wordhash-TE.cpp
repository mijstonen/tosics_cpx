#!/usr/local/bin/cpx
#@

#define SYMVAL(word) #word ## _slh
#define SYMBOL(word) word = SYMVAL(word)
    bool
 LineFilter(const char *cs)
 {
     unsigned index=0;
     while(index=static_cast<unsigned char>(*cs), index) {
         if ( *cs<33 )  return false;
         if ( *cs>122 ) return false;
         ++cs;
     }
     return true;
 }

  enum Econmd
 : uint64_t
 {  SYMBOL(comd1)
 ,  SYMBOL(domd2)
 ,  SYMBOL(eomd3)
 ,  SYMBOL(fomd4)
 ,  SYMBOL(gomd5)
 };

    Econmd
 ec=Econmd::gomd5
 ;

________________________________________________________________________________________________________________________
#!

 switch (ec)
 {
     case SYMVAL(comd1):
        INFO("1");
        break;
     case SYMVAL(domd2):
        INFO("2");
        break;
     case SYMVAL(eomd3):
        INFO("3");
        break;
     case SYMVAL(fomd4):
        INFO("4");
        break;
    default:
        INFO(HRED "??????" NOCOLOR);
 }



 map<uint64_t,vector<string>> whmap;

 INFO("wait for results");
    size_t
 line_cnt=0
 ;

// multithreaded examine whmap
     size_t
 numThreads = omp_get_max_threads()
 ;
     size_t
 mapSize = 0
 ;
     size_t
 partitionSize
 ;
     size_t
 remaining = mapSize % numThreads // Handle remainder if map size is not divisible by numThreads
 ;
    auto
 it = whmap.begin()
 ;


||+
||  parallel shared(cin,whmap,numThreads, mapSize, partitionSize, remaining, it)
    {
            size_t
        threadNum = omp_get_thread_num()
        ;
            string
        w
        ;
        do {
||          critical
            {
                getline(cin, w);
            }
            if ( cin && LineFilter(w.c_str())) {
                    uint64_t
                wh= Cstr2uint64(w)
                ;
||              critical
                {
                    ++line_cnt;
                    whmap[wh].push_back(w);
                }
            }
        } while (cin);

||      single
        {
            INFO(ENDL,VARVALS(line_cnt),"______________________________________________________________");
            // multithreaded examine whmap
                int
            numThreads = omp_get_max_threads()
            ;

            mapSize = whmap.size();
            partitionSize = mapSize / numThreads;
            remaining = mapSize % numThreads; // Handle remainder if map size is not divisible by numThreads
            it = whmap.begin();
        }

        auto startIt = it;

        advance(startIt, threadNum * partitionSize + min(static_cast<size_t>(threadNum), remaining));

        size_t myPartitionSize = partitionSize + (threadNum < remaining ? 1 : 0);

        auto endIt = startIt;
        advance(endIt, myPartitionSize);

        for (auto itr = startIt; itr != endIt; ++itr) {
            auto k= itr->first;
            auto v= itr->second;
            if ( v.size()!=1 ) {
||              critical
                {
                    INFO(VARVALS(k,v));
                }
            }
        }
    }
 // parallel

 ASSERT( line_cnt==whmap.size());
