#include<stdio.h>
#include<stdarg.h>
#include<string.h>

#include "editorConfig.h"
#include "appendBuffer.h"


void drawStatusBar(struct abuffer *ab){
    appendBuffer(ab, "\x1b[7m", 4);
    char status[80], rstatus[80];
    int len = snprintf(status, sizeof(status), "%.20s - %d lines", E.filename ? E.filename : "[no name]", E.numrows);
    int rlen = snprintf(rstatus, sizeof(rstatus), "%d - %d", E.cy+1, E.numrows);
    if (len > E.screencols) len = E.screencols;
    appendBuffer(ab, status, len);    
    while (len < E.screencols){
        if(E.screencols - len == rlen){
            appendBuffer(ab, rstatus, rlen);
            break;
        }else{
            appendBuffer(ab, " ", 1);
            len++;
        }
    }
    appendBuffer(ab, "\x1b[m", 3);
    appendBuffer(ab, "\r\n", 2);
}

void setStatusMessage(const char *fmt, ...){
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(E.statmessage, sizeof(E.statmessage), fmt, ap);
    va_end(ap);
    E.statmessage_time = time(NULL);
}

void drawMessageInStatbar(struct abuffer *ab){
    appendBuffer(ab, "\x1b[K", 3);
    int msglen = strlen(E.statmessage);
    if (msglen > E.screencols) msglen = E.screencols;
    if (msglen && time(NULL) - E.statmessage_time < 5){
        appendBuffer(ab, E.statmessage, msglen);
    }
}
