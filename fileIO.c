#define _DEFAULT_SOURCE
#define _GNU_SOURCE

#include<sys/types.h>
#include<string.h>
#include<stdlib.h>
#include<stdio.h>

#include "editorConfig.h"
#include "errorHandle.h"

void editorAppendRow(char *s, size_t len){
    E.row = realloc(E.row, sizeof(editorRow) * (E.numrows + 1));

    int index = E.numrows;
    E.row[index].size = len;
    E.row[index].chars = malloc(len + 1);
    memcpy(E.row[index].chars, s, len);
    E.row[index].chars[len] = '\0';
    E.numrows++;
}

void editorOpen(char *filename) {
    FILE *fd = fopen(filename, "r");
    if (!fd) err_handle("fopen");

    char *line = NULL;
    size_t linecap = 0;
    ssize_t linelength;
    // linelength = getline(&line, &linecap, fd);
    while ((linelength = getline(&line, &linecap, fd)) != -1) {
        while (linelength > 0 && (line[linelength -1] == '\n' || line[linelength -1] == '\r'))
            linelength--;
        editorAppendRow(line, linelength);
    }
    free(line);
    fclose(fd);
}
