#include "7z.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int lzma_compress(const char* src, const char* dst){
	const char* args[4];
	args[0] = "";
	args[1] = "e";
	args[2] = src;
	args[3] = dst;
	return main1(sizeof(args)/sizeof(args[0]), args);
}
int lzma_uncompress(const char* src, const char* dst){
	const char* args[4];
	args[0] = "";
	args[1] = "d";
	args[2] = src;
	args[3] = dst;
	return main1(sizeof(args)/sizeof(args[0]), args);
}