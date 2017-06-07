# development version. Compacted version used in CPX-preproces_hash_compile.sh

TMP_PP_WORK_PATH="h2c3.txt"
tpwp2=CPX-test-varvresearch.cpp


HASH=$(
    gawk -v HashProces="sha1sum | cut -c -40" -v tmp_pp_work_path="$TMP_PP_WORK_PATH" '

    #---------- disable P for non-locals injected by cpp
        $1=="#" && $2==1 { #1
            P=!($3~/"<.*>"|"\/usr\/include\/.*"/)
        } #1

    #---------- copy to file if P is enabled
                    { #2

            if(P) {
                print $0 > tmp_pp_work_path
             }
        } #2

    #---------- if it is not a directive, pass to hash proces if P is enabled
        ($1 !~ "^#.*")||($1 ~ "^#.*" && $2 !~ "^[[:digit:]]+$")  { #3

            if(P) { 
                print $0 | HashProces
            }
        } #3

    #----------
    ' "$tpwp2"
)
echo "HASH: $HASH"

 