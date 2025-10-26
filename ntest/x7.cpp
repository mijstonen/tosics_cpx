#!/usr/bin/env cpx
// Demonstrates displaying containers of all kinds


#!
deque<int> v {  100, 3, 1, 4, 1, 5, 9 };
INFO(VARVALS(type_name(v),v));

{LOCAL_MODIFIED(AppliedInfoSettingsPtr);

 InfoSettings lifs;
 lifs.containerStart="[ ";
 lifs.containerItemSeparation=" | ";
 lifs.containerEnd=" ]";
 AppliedInfoSettingsPtr= &lifs;
 forward_list<double> ld{1.1,2.2,3.3,4.4,5.5};
 INFO(VARVALS(type_name(ld),ld));
}

make_heap(v.begin(),v.end());
INFO(VARVALS(type_name(v),v));
array<unsigned,6> u{6,6,6,6,6,6};
INFO(VARVALS(type_name(u),u));
unordered_set<char> s{'x','Y','q','Z'};
INFO(VARVALS(type_name(s),s));
pair<string,unsigned> psi{"age",26};
INFO(VARVALS(type_name(psi),psi));
unordered_map<char,int>
    mci{
        {'q',12},
        {'b',90},
        {'h',22},
        {'s',15},
        {'f',12},
        {'l',8},
        {'p',0}
    };
    AppliedInfoSettingsPtr->pairItemsSeparation="->";
    INFO(VARVALS(type_name(mci),mci));
