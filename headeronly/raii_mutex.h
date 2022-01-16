#pragma once
/// <summary>
/// WS::raii_mutex<mutex_type> nur RAII-Lockes. lock,unlock sind protected
///	usage:
///		WS::raii_mutex<std::recursive_timed_mutex> m;
///		if( auto locked = [WS::]try_lock_for(m, std::chrono::milliseconds{50} ) ){}//mutex_type muss natürlich try_lock_for unterstützen, z.b. std::timed_mutex 
///		if( auto locked = [WS::]try_lock_until(m, std::chrono::steady_clock::now()+std::chrono::milliseconds{50} ) ){}//mutex_type muss natürlich try_lock_until unterstützen, z.b. std::timed_mutex 
///		if( auto locked = [WS::]try_lock(m) ){}
///		if( auto locked = [WS::]lock(m) ){}
///			[WS::] namespace kann, muss aber nicht angegeben werden. ADL working 
/// 
///		die lock-funktionen liefern einen WS::unique_lock
///		WS::unique_lock wie std::unique_lock,
///		aber 
///			operator bool() const & noexcept; ist nicht explicit und nur auf lvalue aufrufbar
///			release() = delete; nicht aufrufbar, da mutex.unlock() unerreichbar ist
/// </summary>
/// 

#include <mutex>
#include <chrono>

namespace WS
{
	template<class mutex_type> class lock_guard;//forward

	// operator bool() ohne explicit
	template <class mutex_type> class unique_lock : public std::unique_lock<mutex_type>
	{
	public:
		using mutex_t = mutex_type;
		using base_t = std::unique_lock<mutex_type>;
		using base_t::base_t;
		operator bool() & noexcept { return base_t::operator bool();}//	if(auto locked=lock(mutex)){...} geht 
		operator bool() const & noexcept = delete;//					if(lock(mutex)){...} geht nicht. verhindert die prüfung auf true direkt auf den rückgabewert. der lock wäre schnell wieder weg 
		mutex_t* release() noexcept = delete;//bitte nicht benutzen, wozu auch??
	};

	//lock etc verstecken, benutzt die RAII-Klassen siehe friend

	template<class mutex_type> class raii_mutex : protected mutex_type
	{
	public:
		using mutex_t = mutex_type;
		using base_t = mutex_type;
		using base_t::base_t;

		template<class mutex_type>		friend class std::unique_lock;
		template<class ... mutex_types>	friend class std::scoped_lock;
		template<class mutex_type>		friend class std::lock_guard;
		template<class mutex_type>		friend class WS::lock_guard;
	};

	#pragma region Komfortzone locking-funktionen für raii_mutex
	template<class mutex_t>
	inline [[nodiscard]] auto lock( WS::raii_mutex<mutex_t> & m )
	{
		return WS::unique_lock<WS::raii_mutex<mutex_t>>{m};
	}
	template<class mutex_t>
	inline [[nodiscard]] auto try_lock( WS::raii_mutex<mutex_t> & m )
	{
		auto ret_value = WS::unique_lock<WS::raii_mutex<mutex_t>>{m,std::defer_lock};
		(void)ret_value.try_lock();
		return ret_value;
	}
	template<class mutex_t,typename rep_t,typename period_t > 
	[[nodiscard]] auto try_lock_for( WS::raii_mutex<mutex_t> & m, std::chrono::duration<rep_t,period_t> const & duration )
	{
		/// duration_t etwa std::chrono::milliseconds usw. 
		/// oder 
		///			using namespace std::chrono_literals;
		///			20ms 500us 1s 2min 3h ... und was sonst noch geht

		auto ret_value = WS::unique_lock<WS::raii_mutex<mutex_t>>{m,std::defer_lock};
		(void)ret_value.try_lock_for(duration);
		return ret_value;
	}
	template<class mutex_t,typename clock_t,typename duration_t> 
	[[nodiscard]] auto try_lock_until( WS::raii_mutex<mutex_t> & m, std::chrono::time_point<clock_t,duration_t> const & timepoint)
	{
		auto ret_value = WS::unique_lock<WS::raii_mutex<mutex_t>>{m,std::defer_lock};
		(void)ret_value.try_lock_until(timepoint);
		return ret_value;
	}
}
