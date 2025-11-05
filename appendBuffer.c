#include<stdlib.h>
#include<string.h>

#include "appendBuffer.h"


void appendBuffer(struct abuffer *ab, const char *s, int len) {
	char *new = realloc(ab->b, ab->len + len);
	
	if (new == NULL) return;
	memcpy(&new[ab->len], s, len);
	ab -> b = new;
	ab -> len += len;
}


void freeBuff(struct abuffer *ab) {
	free(ab->b);
	ab->b = NULL;
	ab->len = 0;
}