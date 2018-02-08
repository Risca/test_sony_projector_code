#include "serial.h"

int SerialProxy::available()
{
	return g_Instance->available();
}

size_t SerialProxy::print(const char* str)
{
	return g_Instance->print(str);
}

size_t SerialProxy::println(const char* str)
{
	return g_Instance->println(str);
}

size_t SerialProxy::readBytes(char buffer[], int length)
{
	return g_Instance->readBytes(buffer, length);
}

void SerialProxy::setTimeout(long time)
{
	g_Instance->setTimeout(time);
}

size_t SerialProxy::write(char buf[], int len)
{
	return g_Instance->write(buf, len);
}

SerialMock* SerialProxy::g_Instance = 0;
SerialProxy Serial;
SerialProxy Serial0;

