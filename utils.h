#ifndef UTILS_H_
#define UTILS_H_

#include <assert.h>

char *substring(char *string, int start, int end){
	char *buffer = (char *)malloc(end - start);
	assert(buffer != NULL);

	int j = 0;
	for (int i = start; i < end; i++){
		buffer[j] = string[i];
		j++;
	}
	buffer[j] = '\0';

	return buffer;
}

#endif // UTILS_H_