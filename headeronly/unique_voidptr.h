#pragma once

#include <memory>

namespace WS
{
	struct freevoidptr
	{
		void operator()( void* ptr )
		{
			free( ptr );
		}
	};

	//verwalteter speicher sollte mit der methode alloc, s.u., angelegt werden. z.zt wird speicher mit malloc/free angesprochen.
	class unique_voidptr : public std::unique_ptr<void, WS::freevoidptr>
	{
		typedef std::unique_ptr<void, WS::freevoidptr> BaseClass;
	public:
		unique_voidptr() = default;
		unique_voidptr( void* r ) : BaseClass( std::move(r) )
		{
		}
		unique_voidptr( unique_voidptr && r ) : BaseClass( std::move(r) )
		{
		}
		unique_voidptr& operator=( unique_voidptr const & r ) = delete;
		unique_voidptr& operator=( unique_voidptr && r )
		{
			BaseClass::operator=( std::move(r) );
			return *this;
		}
		void* alloc( size_t bytes )
		{
			reset( malloc( bytes ) );
			return get();
		}
		void* get() const
		{
			return BaseClass::get();
		}
	};
}

