#ifndef STATUSBAR_H
#define STATUSBAR_H

void drawStatusBar(struct abuffer *ab);
void setStatusMessage(const char *fmt, ...);
void drawMessageInStatbar(struct abuffer *ab);

#endif