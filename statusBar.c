#define _DEFAULT_SOURCE
#define _GNU_SOURCE

#include<stdio.h>
#include<stdlib.h>
#include<ctype.h>
#include<stdarg.h>
#include<string.h>

#include "editorConfig.h"
#include "appendBuffer.h"

extern void editorRefreshScreen();
extern int editorReadKey();

extern int lastKeyPressed;

void drawStatusBar(struct abuffer *ab){
    appendBuffer(ab, "\x1b[7m", 4);
    char status[80], rstatus[80];
    int len = snprintf(status, sizeof(status), "%.20s - %d lines %s", E.filename ? E.filename : "[no name]", E.numrows, E.changed ? "(modified)" : "");
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


char *setPrompt(char *prompt){
    size_t bufsize = 128, buflen;
    char *buf;
    if(E.filename == NULL || lastKeyPressed == CTRL_KEY('f')){
        buflen = 0;
        buf = malloc(bufsize);
        buf[0] = '\0';
    }else{
        buflen = strlen(E.filename);
        buf = strdup(E.filename);
    }
    while(1){
        setStatusMessage(prompt, buf);
        editorRefreshScreen();
        int c = editorReadKey();
        if(!iscntrl(c) && c < 128){
            if (buflen == bufsize-1){
                bufsize *= 2;
                buf = realloc(buf, bufsize);
            }
            buf[buflen++] = c;
            buf[buflen] = '\0';
        }else if (c == DEL_KEY || c == CTRL_KEY('h') || c == BACKSPACE){
            if(buflen != 0) buf[--buflen] = '\0';
        }else if (c == '\x1b'){
            setStatusMessage("");
            free(buf);
            return NULL;
        }else if (c == '\r'){
            if(buflen != 0){
                setStatusMessage("");
                return buf;
            }
        }
    }
    free(buf);
}

