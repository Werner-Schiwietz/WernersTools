#include "pch.h"
#include "GlobalAllocGuard.h"

#include <utility>

GlobalAllocGuard::GlobalAllocGuard(SIZE_T bytes, UINT flags) noexcept
	: memory(::GlobalAlloc(flags, bytes))
{}

GlobalAllocGuard::~GlobalAllocGuard()
{
	free();
}

GlobalAllocGuard::GlobalAllocGuard(GlobalAllocGuard&& other) noexcept
	: memory(std::exchange(other.memory, nullptr))
{}

GlobalAllocGuard& GlobalAllocGuard::operator=(GlobalAllocGuard&& other) noexcept
{
	if( this != &other)
	{
		free();
		this->memory = std::exchange(other.memory, nullptr);
	}
	return *this;
}


HGLOBAL GlobalAllocGuard::release() noexcept
{
	return std::exchange(this->memory,nullptr);
}

void	GlobalAllocGuard::free() noexcept
{
	if( this->memory )
		::GlobalFree( release() );
}

GlobalLockGuard::GlobalLockGuard(HGLOBAL memory) noexcept
	: memory(memory)
	, lockedMemory(::GlobalLock(memory))
{}

GlobalLockGuard::~GlobalLockGuard()
{
	unlock();
}

GlobalLockGuard::GlobalLockGuard(GlobalLockGuard&& other) noexcept
	: memory(std::exchange(other.memory, nullptr))
	, lockedMemory(std::exchange(other.lockedMemory, nullptr))
{
}

GlobalLockGuard& GlobalLockGuard::operator=(GlobalLockGuard&& other) noexcept
{
	if( this != &other )
	{
		unlock();

		this->memory = std::exchange(other.memory, nullptr);
		this->lockedMemory = std::exchange(other.lockedMemory, nullptr);
	}
	return *this;
}

void GlobalLockGuard::unlock()
{
	if( std::exchange(this->lockedMemory, nullptr) )
	{
		::GlobalUnlock(this->memory);
	}
}
