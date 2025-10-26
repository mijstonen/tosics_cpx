#!/usr/local/bin/cpx
#@

#define SYMVAL(word) #word ## _slh
#define SYMBOL(word) word = SYMVAL(word)
    bool
 LineFilter(const char *cs)
 {
     unsigned index=0;
     while(index=static_cast<unsigned char>(*cs), index) {
         if ( *cs<33 )  return true;
         if ( *cs>122 ) return true;
         ++cs;
     }
     return false;
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
 #if XXX
 {
 char ees[33]="eeeeeeeeeeeeeeeeeeeeeeeeeeeeeee";
 for(auto i: repeat(30)){
     char buf[33];
     strcpy(buf,ees);
     buf[i]='\0';
     uint64_t wh=CommandHash(buf);
     ++whmap[wh];
     const char* whbits=Bits2str(wh);
     INFO(VARVALHEX(wh),VARVALS(buf,hex,wh,whbits));
 }
 INFO(VARVALS(whmap));
 }
 {
 char ees[33]="_______________________________";
 for(auto i: repeat(30)){
     char buf[33];
     strcpy(buf,ees);
     buf[i]='\0';
     uint64_t wh=CommandHash(buf);
     ++whmap[wh];
     const char* whbits=Bits2str(wh);
     INFO(VARVALHEX(wh),VARVALS(buf,hex,wh,whbits));
 }
 #endif

 INFO("wait for results");
 size_t line_cnt=0;
    string
 w
 ;
    size_t
 line_skips=0
 ;
 while (getline(cin, w),cin) {
     /*
    if ( LineFilter(w.c_str()) ) {
        ++line_skips;
        continue;
    }
     */
    uint64_t wh= Cstr2uint64(w); // std::hash<std::string>{}(w)  CommandHash(w.c_str());
    ++line_cnt;
    whmap[wh].push_back(w);
    const char* whbits=Bits2str(wh);
//    INFO(VARVALHEX(wh),VARVALS(w,hex,wh,whbits));
//    printf("%-50s %70s\n",w.c_str(),whbits);
 }
 INFO(ENDL,VARVALS(line_cnt,line_skips),"______________________________________________________________");

 //INFO(VARVALS(whmap));

 for(auto [k,v]:whmap) {
     if ( v.size()!=1 ) {
        INFO(VARVALS(k,v));
     }
 }

 ASSERT( line_cnt==whmap.size());
