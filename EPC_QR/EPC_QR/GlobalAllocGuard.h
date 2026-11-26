#pragma once

#include <Windows.h>

class GlobalAllocGuard
{
public:
	GlobalAllocGuard(SIZE_T bytes, UINT flags) noexcept;
	~GlobalAllocGuard();

	GlobalAllocGuard(const GlobalAllocGuard&) = delete;
	GlobalAllocGuard& operator=(const GlobalAllocGuard&) = delete;
	GlobalAllocGuard(GlobalAllocGuard&& other) noexcept;
	GlobalAllocGuard& operator=(GlobalAllocGuard&& other) noexcept;

	explicit operator bool() const noexcept { return this->memory != nullptr; }
	operator HGLOBAL() const noexcept { return this->memory; }
	HGLOBAL release() noexcept;
	void	free() noexcept;

private:
	HGLOBAL memory;
};

class GlobalLockGuard
{
public:
	explicit GlobalLockGuard(HGLOBAL memory) noexcept;
	~GlobalLockGuard();

	GlobalLockGuard(const GlobalLockGuard&) = delete;
	GlobalLockGuard& operator=(const GlobalLockGuard&) = delete;
	GlobalLockGuard(GlobalLockGuard&& other) noexcept;
	GlobalLockGuard& operator=(GlobalLockGuard&& other) noexcept;

	explicit operator bool() const noexcept			{ return this->lockedMemory != nullptr; }
	operator void*() const noexcept					{ return this->lockedMemory; }
	template<typename T> T* get() const noexcept	{ return static_cast<T*>(this->lockedMemory); }

	void unlock();

private:
	HGLOBAL	memory;
	void*	lockedMemory;
};
