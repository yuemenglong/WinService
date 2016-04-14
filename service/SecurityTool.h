#pragma once
#include "aes.h"
#include "cbase64.h"
#include <string>
class SecurityTool
{
#define MIN(a,b) ((a)<(b)?(a):(b))
public:	
	static const int BUF_SIZE = 16;
	static const int B64_SIZE = BUF_SIZE * 3 / 2;
	static DWORD KEY[60];

	static void encrypt(std::string& in, std::string& out, DWORD key[60]){
		out.clear();
		for(int i = 0; i < in.length(); i += BUF_SIZE){
			unsigned char in_buf[BUF_SIZE] = {0};
			unsigned char out_buf[BUF_SIZE] = {0};
			std::string b64_buf;

			memcpy(in_buf, in.c_str() + i, MIN(BUF_SIZE, in.length() - i));
			AesEncrypt_Buffer(in_buf, out_buf, BUF_SIZE, key);
			CBase64::Encode(out_buf, BUF_SIZE, b64_buf);
			out.append(b64_buf);
		}
	}

	static void decrypt(std::string& in, std::string& out, DWORD key[60]){
		out.clear();
		unsigned long size = B64_SIZE;
		for(int i = 0; i < in.length(); i += B64_SIZE){
			unsigned char in_buf[BUF_SIZE] = {0};
			unsigned char out_buf[BUF_SIZE] = {0};
			std::string b64_buf = in.substr(i, B64_SIZE);

			CBase64::Decode(b64_buf, in_buf, &size);
			AesDecrypt_Buffer(in_buf, out_buf, BUF_SIZE, key);
			std::string sigment((char*)out_buf, BUF_SIZE);
			out.append(sigment);
		}
	}
};

DWORD SecurityTool::KEY[60] = {0};
