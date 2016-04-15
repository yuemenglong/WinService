#pragma once
#pragma comment(lib,"7z.lib")
#include "../7z/7z.h"
class CompressTool
{
public:
	static int compress(const char* src, const char* dst){
		return lzma_compress(src, dst);
	}
	static int uncompress(const char* src, const char* dst){
		return lzma_uncompress(src, dst);
	}
};

