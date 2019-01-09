#pragma once

#include <string>

class DllHandle
{
private:
	void* m_handle;

	void* getProcAddressInternal(const std::string& name);

public:
	DllHandle(const DllHandle&) = delete;

	DllHandle(std::string filename);
	~DllHandle();

	template<typename T>
	T* getProcAddress(const std::string& name)
	{
		return (T*)getProcAddressInternal(name);
	}
};