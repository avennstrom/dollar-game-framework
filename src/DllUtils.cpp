#include "DllUtils.hpp"

#if _WIN32
#define NOMINMAX
#include <Windows.h>
#elif __linux__
#include <dlfcn.h>
#endif

DllHandle::DllHandle(std::string filename)
{
#if _WIN32
	m_handle = LoadLibraryA(filename.c_str());
#elif __linux__
	m_handle = dlopen(filename.c_str(), RTLD_LAZY);
#endif
}

DllHandle::~DllHandle()
{
#if _WIN32
	FreeLibrary((HMODULE)m_handle);
#elif __linux__
	dlclose(m_handle);
#endif
}

void* DllHandle::getProcAddressInternal(const std::string& name)
{
#if _WIN32
	return GetProcAddress((HMODULE)m_handle, name.c_str());
#elif __linux__
	return dlsym(m_handle, name.c_str());
#endif
}