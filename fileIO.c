#include<sys/types.h>
#include<string.h>
#include<stdlib.h>

#include "editorConfig.h"

void editorOpen(void) {
    
    char *line = "Hello World";
    ssize_t linelen = 13;
    
    E.row.size = linelen;
    E.row.chars = malloc(linelen+1);
    memcpy(E.row.chars, line, linelen);
    E.row.chars[linelen] = '\0';
    E.numrows = 1;
}
