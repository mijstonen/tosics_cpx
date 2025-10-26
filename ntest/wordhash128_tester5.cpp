#!/usr/local/bin/cpx

#!
    map<uint128_t,vector<string>>
 whmap
 ;
    map<uint64_t,vector<string>>
 wgmap
 ;
    string
 word
 ;
    size_t
 line_cnt=0
 ;
 while (getline(cin, word),cin) {
     uint64_t  ha64=Cstr2uint64 (word.c_str(),word.size());
     uint64_t  hb64=Cstr2uint64v(word.c_str(),word.size());
     if ( ha64!=hb64 ) {
        ++line_cnt;
        INFO(VARVALHEX(ha64),VARVALHEX(hb64));
     }

#if 0
    uint128_t hash(Cstr2uint128(word));

    ++line_cnt;
    if ( ha64!= hv64 ) {
        INFO(HRED "Mismatch" NOCOLOR,VARVALS(hash,ha64,hv64,word));
    }
    whmap[hash].push_back(word);
    // TEST_THE_TEST
    // force error, the check that errors a detected
    if ( (line_cnt%100000 )==0){
        whmap[hash].push_back(word);
    }
#endif
 }
#if 0
 INFO("_________________________________________________________________");
 size_t duplicates=0;
 for(auto [k,v]:whmap) {
     if ( v.size()!=1 ) {
         ++duplicates;
        INFO(VARVALS(k,v));
     }
 }
 ASSERT( line_cnt==whmap.size()); // failing indicates that there are hash collisions
 INFO(VARVALS(duplicates));
#endif
INFO(VARVALS(line_cnt));
