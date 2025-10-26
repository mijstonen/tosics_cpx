#!/usr/bin/env cpx
#|

// I don't remember the purpose of this experiment, but solution() finds the first unique item in the container.
// put your globals, templates,class definitions and includes here

int solution(const vector<int> &A) {
    std::size_t array_size{A.size()};
    for (auto& value: A) {
        bool multiple=false;
        for (auto& match_value: A) {
            if ( &match_value == &value ) continue;
            INFO(VARVALS(&match_value,match_value,&value,value,match_value==value));
            if (match_value==value) {
                multiple=true;
                break;
            }
        }
        if (!multiple) {
            return value;
        }
        INFO("-------------------");
    }
    return -1;
}


#!
     vector<int> avu{66,2,77,3,8,4,93,93,93,93,92,3,6,3,66,4,2,6,8};
    INFO(VARVALS(avu,solution(avu)));

