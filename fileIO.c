#define _DEFAULT_SOURCE
#define _GNU_SOURCE

#define TAB_STOP_LENGTH 8

#include<sys/types.h>
#include<string.h>
#include<stdlib.h>
#include<stdio.h>

#include "editorConfig.h"
#include "errorHandle.h"

int CxToRxConvert(editorRow *row, int cx){
    int rx = 0;
    for(int j = 0; j < cx; j++){
        if(row->chars[j] == '\t'){
            rx = rx + (TAB_STOP_LENGTH - 1) - (rx % TAB_STOP_LENGTH);
        }
        rx++;
    }
    return rx;
}

void editorUpdateRow(editorRow *row) {
    int tabs = 0;
    for(int j = 0; j < row->size; j++){
        if (row->chars[j] == '\t') tabs++;
    }
    free(row->render);
    row->render = malloc(row->size + tabs*(TAB_STOP_LENGTH-1) + 1);
    int index = 0;
    for(int j = 0; j < row -> size; j++){
        if(row->chars[j] == '\t'){
            row->render[index++] = ' ';
            while(index % TAB_STOP_LENGTH != 0) row->render[index++] = ' ';
        }else{
            row->render[index++] = row->chars[j];
        }
    }
    row->render[index] = '\0';
    row->rsize = index;
}


void editorAppendRow(char *s, size_t len){
    E.row = realloc(E.row, sizeof(editorRow) * (E.numrows + 1));

    int index = E.numrows;
    E.row[index].size = len;
    E.row[index].chars = malloc(len + 1);
    memcpy(E.row[index].chars, s, len);
    E.row[index].chars[len] = '\0';
    E.numrows++;

    E.row[index].rsize = 0;
    E.row[index].render = NULL;
    editorUpdateRow(&E.row[index]);
}

void editorOpen(char *filename) {
    free(E.filename);
    E.filename = strdup(filename);
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
