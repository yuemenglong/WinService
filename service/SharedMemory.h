#pragma once

#include "StdAfx.h"

class SharedMemory
{
	char _name[256];
	HANDLE _handle;
	void* _area;
public:
	SharedMemory(const char* name){
		strcpy_s(_name, name);
	}
	bool create(int size){
		if(!_handle){
			_handle = CreateFileMapping((HANDLE)-1, NULL, PAGE_READWRITE, 0, size, _name);
		}
		if(_handle <= 0){
			return false;
		}
		if(!_area){
			_area = MapViewOfFile(_handle, FILE_MAP_READ|FILE_MAP_WRITE, 0, 0, 0);
		}
		if(!_area){
			return false;
		}
		return true;
	}
	bool open(){
		if(!_handle){
			_handle = OpenFileMapping(FILE_MAP_WRITE, false, _name);
		}
		if(_handle <= 0){
			return false;
		}
		if(!_area){
			_area = MapViewOfFile(_handle, FILE_MAP_READ|FILE_MAP_WRITE, 0, 0, 0);
		}
		if(!_area){
			return false;
		}
		return true;
	}
	bool destroy(){
		bool ret1 = true;
		bool ret2 = true;
		if(_area)
			ret1 = UnmapViewOfFile(_area);
		if(_handle)
			ret2 = CloseHandle(_handle);
		return ret1 && ret2;
	}
	int read_int(int pos = 0){
		int* area = (int*)((char*)_area + pos);
		return *area;
	}
	void write_int(int val, int pos = 0){
		int* area = (int*)((char*)_area + pos);
		*area = val;
	}

	void write_mem(const void* mem, int from, int length){
		memcpy((char*)_area + from, mem, length);
	}

	void read_mem(void* mem, int from, int length){
		memcpy(mem, (char*)_area + from, length);
	}
};