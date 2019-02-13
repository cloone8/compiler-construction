#include <stdlib.h>

size_t countNewlines(char* string, size_t length) {
    size_t linecount = 0;
    for(int i = 0; i < length; i++) {
        if(string[i] == '\n' && (i == 0 || string[i - 1] != '\r')) {
            linecount++;
        } else if(string[i] == '\r') {
            linecount++;
        }
    }

    return linecount;
}
