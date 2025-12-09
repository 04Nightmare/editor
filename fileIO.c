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
extern char *setPrompt(char *prompt);

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

int RxToCxConvert(editorRow *row, int rx){
    int cursor_rx = 0;
    int cx;
    for(cx = 0; cx < row->size; cx++){
        if(row->chars[cx] == '\t'){
            cursor_rx = cursor_rx + (TAB_STOP_LENGTH - 1) - (cursor_rx % TAB_STOP_LENGTH);
        }
        cursor_rx++;
        if(cursor_rx > rx) return cx;
    }
    return cx;
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

void editorAppendRow(int index, char *s, size_t len){
    if(index < 0 || index > E.numrows) return;
    E.row = realloc(E.row, sizeof(editorRow) * (E.numrows + 1));
    memmove(&E.row[index + 1], &E.row[index], sizeof(editorRow) * (E.numrows - index));

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
        editorAppendRow(E.numrows, line, linelength);
    }
    free(line);
    fclose(fd);
    E.changed=0;
}


//Write in file..

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
        editorAppendRow(E.numrows, "", 0);
    }
    insertCharInRow(&E.row[E.cy], E.cx, c);
    E.cx++;
}

void editorInsertNewLine(){
    if(E.cx == 0){
        editorAppendRow(E.cy, "", 0);
    }else{
        editorRow *row = &E.row[E.cy];
        editorAppendRow(E.cy + 1, &row->chars[E.cx], row->size - E.cx);
        row = &E.row[E.cy];
        row->size = E.cx;
        row->chars[row->size] = '\0';
        editorUpdateRow(row);
    }
    E.cy++;
    E.cx=0;
}

//Delete from file.

void delCharFromRow(editorRow *row, int delAt){
    if(delAt < 0 || delAt >= row->size) return;
    memmove(&row->chars[delAt], &row->chars[delAt+1], row->size-delAt);
    row->size--;
    editorUpdateRow(row);
    E.changed++;
}

void freeRow(editorRow *row){
    free(row->render);
    free(row->chars);
}

void editorDelRow(int delAt){
    if(delAt < 0 || delAt >= E.numrows) return;
    freeRow(&E.row[delAt]);
    memmove(&E.row[delAt], &E.row[delAt+1], sizeof(editorRow)*(E.numrows-delAt-1));
    E.numrows--;
    E.changed++;
}

void editorAppendStringInRow(editorRow *row, char *s, size_t len){
    row->chars = realloc(row->chars, row->size + len + 1);
    memcpy(&row->chars[row->size], s, len);
    row->size += len;
    row->chars[row->size] = '\0';
    editorUpdateRow(row);
    E.changed++;
}

void editorDelChar(){
    if (E.cy == E.numrows) return;
    if (E.cx == 0 && E.cy == 0) return;
    editorRow *row = &E.row[E.cy];
    if(E.cx > 0){
        delCharFromRow(row, E.cx-1);
        E.cx--;
    }else{
        E.cx = E.row[E.cy-1].size;
        editorAppendStringInRow(&E.row[E.cy-1], row->chars, row->size);
        editorDelRow(E.cy);
        E.cy--;
    }
}

//Save to file, rename file.

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
    if(E.filename == NULL) {
        E.filename = setPrompt("Save as: %s (ESC to cancel)");
        if(E.filename == NULL){
            setStatusMessage("Save aborted");
            return;
        }
    }
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

void editorRename(){
    char *oldname = strdup(E.filename);
    char *newname = setPrompt("Rename file: %s (ESC to cancel)");
    if(newname == NULL){
        setStatusMessage("Rename aborted");
        return;
    }
    free(E.filename);
    E.filename = newname;
    editorSave();
    setStatusMessage("File renamed to %s", newname);
    remove(oldname);
    free(oldname);
}

void editorFind(){
    char *query = setPrompt("Search: %s (ESC to cancel)");
    if (query == NULL) return;
    int i;
    for(i = 0; i < E.numrows; i++){
        editorRow *row = &E.row[i];
        char *match = strstr(row->render, query);
        if (match){
            E.cy = i;
            E.cx = RxToCxConvert(row, match - row->render);
            E.rowoffset = E.numrows;
            break;
        }
    }
    free(query);
}