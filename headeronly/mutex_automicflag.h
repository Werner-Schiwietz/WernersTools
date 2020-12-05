#pragma once

#include <atomic>
#include <mutex>	//wer WS::mutex_atomicflag benutzt sollte auch std::lock_guard nutzen
					//usage 
					// mutex_atomicflag	locker {};
					// auto auto_release_lock = std::lock_guard<mutex_atomicflag>{locker};//der ctor ruft locker.lock() der dtor locker.unlock()

namespace WS
{
	class mutex_atomicflag
	{
		std::atomic_flag flag{};
	public:
		void lock(){ while (flag.test_and_set());}
		void unlock(){ flag.clear(); }
	};

}