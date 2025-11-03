#ifndef APPENDBUFFER_H
#define APPENDBUFFER_H

void appendBuffer(struct abuffer *ab, const char *s, int len);
void freeBuff(struct abuffer *ab);

#endif
