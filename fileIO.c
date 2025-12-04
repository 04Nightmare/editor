#define _DEFAULT_SOURCE
#define _GNU_SOURCE

#define TAB_STOP_LENGTH 8

#include<sys/types.h>
#include<errno.h>
#include<unistd.h>
#include<string.h>
#include<stdlib.h>
#include<stdio.h>
#include<fcntl.h>

#include "editorConfig.h"
#include "errorHandle.h"

extern void setStatusMessage(const char *fmt, ...);

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
    E.changed++;
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
    E.changed=0;
}


//Write character in file..

void insertCharInRow(editorRow *row, int insertAt, int c){
    if (insertAt < 0 || insertAt > row->size) insertAt = row->size;
    row->chars = realloc(row->chars, row->size+2);
    memmove(&row->chars[insertAt + 1], &row->chars[insertAt], row->size-insertAt+1);
    row->size++;
    row->chars[insertAt] = c;
    editorUpdateRow(row);
    E.changed++;
}

void editorWriteChar(int c) {
    if (E.cy == E.numrows){
        editorAppendRow("", 0);
    }
    insertCharInRow(&E.row[E.cy], E.cx, c);
    E.cx++;
}


char *editorRowsToString(int *bufferlen){
    int totalLen = 0;
    int j;
    for(j = 0; j < E.numrows; j++)
        totalLen += E.row[j].size + 1;
    *bufferlen = totalLen;

    char *buff = malloc(totalLen);
    char *p = buff;
    for (j = 0; j < E.numrows; j++){
        memcpy(p, E.row[j].chars, E.row[j].size);
        p += E.row[j].size;
        *p = '\n';
        p++;
    }
    return buff;
}

void editorSave(){
    if(E.filename == NULL) return;
    int len;
    char *buf = editorRowsToString(&len);
    int fd = open(E.filename, O_RDWR | O_CREAT, 0644);
    if (fd != -1){
        if(ftruncate(fd ,len) != -1){
            if (write(fd, buf, len) == len){
                close(fd);
                free(buf);
                E.changed=0;
                setStatusMessage("%d bytes written on the disk", len);
                return;
            }
        }
        close(fd);
    }
    free(buf);
    setStatusMessage("Can't save! Error: %s", strerror(errno));
}
