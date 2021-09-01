#include "table.h"

#define CORNER_FLAG "+"
#define ROW_LINE "_"
#define COL_PADDING " "
#define COL_LINE "|"

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

static  void make_edge(const unsigned  int *col_width, const unsigned  int col_num, char *edge){
    memset(edge, 0, sizeof (edge));
    strcat(edge, CORNER_FLAG);
    int col_line_counter, row_line_counter;

    for(col_line_counter=0; col_line_counter < col_num; col_line_counter++){
        strcat(edge, ROW_LINE)
    }

}

void format_table(table *t, char* *formatted_out){

}