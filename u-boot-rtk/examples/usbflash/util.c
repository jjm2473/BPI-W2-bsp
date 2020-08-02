
#include "util.h"

UINT32 min(UINT32 a, UINT32 b) {
	return a>b?b:a;
}

UINT32 max(UINT32 a, UINT32 b) {
	return a>b?a:b;
}

const char *getenv(const char *name) {
	return NULL;
}

void hang() {
	while(1);
}