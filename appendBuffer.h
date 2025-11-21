#ifndef APPENDBUFFER_H
#define APPENDBUFFER_H

typedef struct abuffer {
	char *b;
	int len;
};
#define ABUFFER_INIT {NULL, 0}

void appendBuffer(struct abuffer *ab, const char *s, int len);
void freeBuff(struct abuffer *ab);

#endif
