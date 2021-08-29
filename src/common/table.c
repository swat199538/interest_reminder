//
// Created by wangl on 8/28/2021.
//

#include "table.h"


static int str_len(unsigned char* str){
    int len =0;
    int offset = 2;
    int word_len = strlen(str);

    for (int i = 0; i < word_len; ++i) {
        if (str[i] < 128){
            len +=1;
        } else{
            len +=2;
            i += offset;
        }
    }

    return len;
}



void format_table(table *t, char* *formatted_out){

}