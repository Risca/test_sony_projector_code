#pragma once

#include "gmock/gmock.h"

#include <stddef.h>

class ISerial
{
public:
	virtual ~ISerial() {}
	virtual int available() = 0;
	virtual size_t print(const char* str) = 0;
	virtual size_t println(const char* str) = 0;
	virtual size_t readBytes(char buffer[], int length) = 0;
	virtual void setTimeout(long time) = 0;
	virtual size_t write(char buf[], int len) = 0;
};

class SerialMock : public ISerial
{
public:
	MOCK_METHOD0(available, int());
	MOCK_METHOD1(print, size_t(const char* str));
	MOCK_METHOD1(println, size_t(const char* str));
	MOCK_METHOD2(readBytes, size_t(char[], int));
	MOCK_METHOD1(setTimeout, void(long));
	MOCK_METHOD2(write, size_t(char[], int));
};

struct SerialProxy : public ISerial
{
	static SerialMock* g_Instance;

	int available();
	size_t print(const char* str);
	size_t println(const char* str);
	size_t readBytes(char buffer[], int length);
	void setTimeout(long time);
	size_t write(char buf[], int len);
};

extern SerialProxy Serial0;

