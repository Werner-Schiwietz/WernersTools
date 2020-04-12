#pragma once
//WS::auto_ptr by Werner Schiwietz 
//WS::auto_ptr ist eine abwandlung vom std::unique_ptr
//im konstruktor kann entschieden werden, ob die resource übernommen wird oder nicht. also das NICHT ist der unterschied zum std::unique_ptr
//z.B. auto_ptr<CZotZeile>( pZeile, false ) das auto_ptr-Objekt benutzt pZeile gibt das objekt aber nicht frei. oder auto_ptr<CZotZeile>( pZeile )
//z.B. auto_ptr<CZotZeile>( pZeile, true ) das auto_ptr-Objekt benutzt pZeile und ruft delete pZeile im destruktor auf
//z.B. auto_ptr<CZotZeile>( std::move(stdunique_ptr) ) das auto_ptr-Objekt übernimmt den std::unique_ptr inhalt std::unique_ptr nur mit default_delete
//z.B. auto_ptr<CZotZeile>( stdshared_ptr) ) das auto_ptr-Objekt übernimmt eine strong referenz  des std::shared_ptr. es gibt keinen owner der letzte shared_ptr gibt objekt frei
//z.B. auto_ptr<char[]>( pString, true ) das auto_ptr-Objekt pString ist mit new char[x] angelegt worden und wird im dtor mit delete [] pString freigegeben
//tests in BasisUnitTests\UT_dtor_call.cpp TEST_CLASS(UT_auto_ptr)
//
//wird konsequent WS::auto_ptr verwendet (also ein owner und 0-n nichtowner bzw  shared_ptr und weak_ptr) wird nicht mehr auf freigegebene, dangle pointer zugegriffen. diese sind statt dessen ggf. nullptr.
//in neuen code evtl. besser reine std::shared_pointer verwenden
//Kopien eines auto_ptr 
//  mit transfer(), neues objekt wird owner wenn ursprungsobjekt owner war
// oder ownerless(), kopie wird nie owner
//
//wenn ihr mit ableitungen und pointer auf basis/abgeleitete klassen arbeitet, sollte ihr immer einen virtuellen destruktor in der basisklasse haben (virtual ~T();)
//
//ausserdem gibt es enable_auto_ptr_from_this siehe std::enable_shared_from_this
//		struct A : public enable_auto_ptr_from_this<A>{};
//		A a;
//		WS::auto_ptr<A> ptr = a.auto_ptr_from_this();//ptr wird nullptr, wenn a zerstoert wird
//
//als parameter in funktionen kann WS::auto_ptr_owner_parameter<T> verwendetet werden
//die klasse muss im konstruktror owner werden, sonst wirft deren ctor eine std::exception

//managed_auto_ptr
//zuweisung und ctor nur von managed auto_ptr oder von ableitungen von enable_auto_ptr_from_this

#include <memory>
#include <atomic>

//#define LINE_STRING2(x) #x				//macht aus der zahl einen sting
//#define LINE_STRING1(x) LINE_STRING2(x)	//nötig, damit __LINE__ zur Zahl wird
//#define _LINE_ LINE_STRING1(__LINE__)		//in pragma message kann _LINE_ verwendet wwerden

namespace WS
{
	template<typename pointer_t> struct ReferenzCounter
	{
		using pointer_type = pointer_t;
		using Managed=bool;
		struct ReferenzCounterShare // gemeinsamer datenpool fuer owner und nicht owner, damit ggf. des nichtowners pointer zum nullptr werden kann
		{
			mutable std::atomic<unsigned short>	counter = 1;
			struct tagValid
			{
				bool	valid	: 1;
				Managed managed	: 7;//belegt erstmal den rest der bits

				tagValid(bool valid) : valid{valid}, managed{false}{}
				tagValid(bool valid,Managed managed) : valid{valid},managed{managed}{}
				operator bool() { return valid; }
			} valid;
		private:
			~ReferenzCounterShare() {}
			ReferenzCounterShare()=delete;
			ReferenzCounterShare( pointer_type const & p, Managed managed ) noexcept : valid{p!=nullptr,managed}{}
			ReferenzCounterShare( ReferenzCounterShare const & ) = delete;
		public:
			ReferenzCounterShare* Release() noexcept
			{
				if( --counter == 0 )
					delete this;
				return nullptr;
			}
			static ReferenzCounterShare* Create( pointer_type const & p, Managed managed)
			{
#				ifdef AUTOPTR_MEMLEAKDETECTION
					DumpAllocStack dumpit;
#				endif
				return new ReferenzCounterShare( p, managed );
			}
			ReferenzCounterShare* AddRef( ) const noexcept
			{
				if(this)
					++this->counter;
				return const_cast<ReferenzCounterShare*>(this);
			}

			void swap( ReferenzCounterShare & r ) noexcept
			{
				std::swap( this->valid, r.valid );
				std::swap( this->counter, r.counter );
			}
		};

		ReferenzCounterShare*	share	= nullptr;
		pointer_type			pointer = nullptr;//nötig, weil dynamic_cast ggf. auch veraenderte adressen liefert, das sharedobjekt sich den pointer nicht merken kann
												  //man könnte das ganze um template data_pointer_type erweitern, dann waeren auto_ptr<A,A*> etwas anderes als auto_ptr<A,B*>. hatte ich schon, war auch seltsam
		ReferenzCounter& operator=( ReferenzCounter const & r ) noexcept
		{
			ReferenzCounter temp( r );
			swap( temp );
			return *this;
		}
		ReferenzCounter& operator=( ReferenzCounter && r ) noexcept
		{
			ReferenzCounter temp( std::move(r) );
			swap( temp );
			return *this;
		}
		~ReferenzCounter()
		{
			if( share )
				share = share->Release();
		}
		ReferenzCounter() noexcept {}
		ReferenzCounter( ReferenzCounter const & r ) noexcept : share(r.share->AddRef()), pointer(r.pointer ) {}
		ReferenzCounter( ReferenzCounter && r ) noexcept : ReferenzCounter()
		{
			swap( r );
		}
		ReferenzCounter( pointer_type p, bool managed ) 
			: share(p ? ReferenzCounterShare::Create(p,managed) : nullptr)
			, pointer(p) {}

		template< typename U> ReferenzCounter( ReferenzCounter<U> const & r  ) noexcept
		{
			//reinterpret_cast nötig, weil nested und ReferenzCounter<T>::ReferenzCounterShare* fuer den compiler etwas anders als ReferenzCounter<U>::ReferenzCounterShare* ist
			this->share = reinterpret_cast<ReferenzCounterShare*>(r.share ? r.share->AddRef() : nullptr);
			if(valid())
				//je nach ableitung kann schon mal eine andere adresse heraus kommen
				this->pointer = dynamic_cast<pointer_type>( r.get() );
		}

		bool valid() const noexcept
		{
			return this->share && this->share->valid;
		}

		void swap( ReferenzCounter & r ) noexcept
		{
			std::swap( this->share, r.share );
			std::swap( this->pointer, r.pointer);
		}
		void SetNullptr() 
		{
			if( this->share )
				this->share->valid = {false};
			this->pointer = nullptr;
		}

		pointer_type get() const 
		{
			if(valid())
				return this->pointer;
			return nullptr;
		}
		//explicit
		operator pointer_type() const 
		{
			return get();
		}
		
	};

	using take_ownership=bool;
	template<typename T> class auto_ptr
	{
		using Managed=bool;
		template<typename> friend class auto_ptr;
	public:
		typedef typename std::remove_all_extents_t<T> * pointer_type;//ggf die [] aus T entfernen
	private:
		mutable ReferenzCounter<pointer_type> share;
		mutable std::unique_ptr<T> Ptr;
		mutable std::shared_ptr<T> SharedPtr;

	public:
		auto_ptr() noexcept {}
		auto_ptr( auto_ptr const & r ) noexcept
			: share( r.share )
			, SharedPtr(r.SharedPtr)
		{
		}
		auto_ptr( auto_ptr && r ) noexcept : auto_ptr()
		{
			swap( r );
		}
	public:
		template<typename T> friend class enable_auto_ptr_from_this;
		~auto_ptr()
		{
			if( Ptr || SharedPtr.use_count()==1 )
				share.SetNullptr();
		}
		auto_ptr( pointer_type p, take_ownership autodelete, Managed managed )
			: share(p,managed)
			, Ptr(autodelete ? p : nullptr){}

		auto_ptr( pointer_type p, take_ownership autodelete ) : auto_ptr( p, autodelete, Managed(autodelete) ){}

		//explicit
		auto_ptr( pointer_type p ) : auto_ptr( p, take_ownership(false) ){}

		auto_ptr( enable_auto_ptr_from_this<T> & r ) : auto_ptr( r.auto_ptr_from_this() ) {}

		//explicit 
		auto_ptr( std::unique_ptr<T> && Ptr ) 
			: share(Ptr.get(),Managed(true))
			, Ptr(std::move(Ptr))
		{}
		explicit auto_ptr( std::shared_ptr<T> sharedptr ) //bei sharedpointer muss sich z.zt ggf. der aufrufer um den cast kümmern, dass kann ich sonst nicht mehr testen
			: share(sharedptr.get(),Managed(true))
			, SharedPtr(sharedptr)
		{}
		auto_ptr& Set( std::shared_ptr<T> sharedptr ) & //operator= gibt mit dem anderen operatoren aerger
		{
			auto_ptr temp( sharedptr );
			swap( temp );
			return *this;
		}
		//auto_ptr(auto_ptr const& r) : auto_ptr(r.ownerless()){}
		template<typename U> 
		//explicit 
		auto_ptr(auto_ptr<U> const & r) noexcept
			: share( nullptr,Managed(false) )
		{
			static_assert( std::is_convertible<auto_ptr<U>::pointer_type,pointer_type>::value 
						   || std::is_convertible<pointer_type,auto_ptr<U>::pointer_type>::value 
							, __FUNCTION__ " pointer sind nicht zuweisbar");

			share = ReferenzCounter<pointer_type>( r.share );
		}
		template<typename U> 
		//explicit 
		auto_ptr(auto_ptr<U> && r) noexcept//dient der konvertierung  const T = U oder T bzw U ist abgeleitet von vom Anderen, bzw. T = const U
		{	//this muss neuer owner werden, da muessen einige bedingungen erfüllt sein

			//nicht nötig aber, wg is_convertible aber macht die problemfindung einfacher
			static_assert( std::is_const<U>::value==false 
						|| std::is_const<T>::value==true
						, "wenn U const ist muss T auch const sein");

			static_assert( std::is_convertible<auto_ptr<U>::pointer_type,pointer_type>::value 
						   || std::is_convertible<pointer_type,auto_ptr<U>::pointer_type>::value 
							, __FUNCTION__ " pointer sind nicht zuweisbar");

			using Tpt = WS::auto_ptr<T>::pointer_type;
			using Upt = WS::auto_ptr<U>::pointer_type;
			using T_t = std::remove_pointer_t<Tpt>;
			using U_t = std::remove_pointer_t<Upt>;
			using Tpt1 = std::remove_const_t<T_t>;
			using Upt1 = std::remove_const_t<U_t>;

			static_assert(std::is_array<T>::value==false && std::is_array<U>::value==false 
						|| std::is_same<Tpt1,Upt1>::value//U[] kann zu const T[] werden
						, "arrays koennen nicht gecastet werden");
			//static_assert(std::is_array<T>::value==std::is_array<U>::value, "beides arrays oder einzelwerte");
			static_assert( std::is_same<std::remove_const_t<T>,std::remove_const_t<U>>::value		//U kann zu const T werden
							|| std::is_base_of<Tpt1,Upt1>::value				//oder T ist basisklasse zu U
							|| std::is_base_of<Upt1,Tpt1>::value				//oder U ist basisklasse zu T 
							, "U kann nicht zu T mit owneruebername werden" );

			auto_ptr<U> tempU;
			tempU.swap( r );

			this->share = ReferenzCounter<pointer_type>( tempU.share );

			if( tempU.Ptr )
			{
				if( dynamic_cast<pointer_type>(tempU.Ptr.get()) )
					this->Ptr = std::unique_ptr<T>( dynamic_cast<pointer_type>(tempU.Ptr.release()) );
				//else
				//	ASSERT( !"cast von U auf T klappt nicht");
			}
		}

		auto_ptr & operator=(auto_ptr && r) & noexcept
		{
			auto_ptr temp{ std::move(r) };
			this->swap( temp );
			return *this;
		}
		auto_ptr & operator=(auto_ptr const & r) & noexcept
		{
			if( this==&r)return *this;
			return operator=(r.ownerless());
		}
		template<typename U>auto_ptr & operator=(auto_ptr<U> && r) & noexcept
		{
			static_assert( std::is_base_of<U, T>::value
						|| std::is_base_of<T, U>::value
						, __FUNCTION__ " pointer sind nicht zuweisbar");

			auto_ptr temp{ r.transfer() };
			swap( temp );

			return *this;
		}
		template<typename U>auto_ptr & operator=(auto_ptr<U> const & r) & noexcept
		{
			static_assert( std::is_base_of<U, T>::value
						   || std::is_base_of<T, U>::value
						, __FUNCTION__ " pointer sind nicht zuweisbar");

			auto_ptr<T> temp{ r };

			swap( temp );
			return *this;
		}
		template<typename U> auto_ptr & operator=(std::unique_ptr<U> && uniqueptr) & noexcept
		{
			return operator=( auto_ptr<U>(std::move(uniqueptr)) );
		}
		auto_ptr & operator=(std::shared_ptr<T> sharedptr ) &
		{
			return operator=( auto_ptr(sharedptr) );
		}		
		auto_ptr & operator=(pointer_type ptr) &
		{
			if (this->get() == ptr)
				return *this;
			return operator=( auto_ptr(ptr) );
		}

		template<typename U> bool operator==( auto_ptr<U> const & r ) const { return this->get() == r.get(); }
		template<typename U> bool operator!=( auto_ptr<U> const & r ) const { return !operator==(r); }
		//mit den folgenden 4 vergleichs-op geht kein vergleich mit NULL mehr
		//bool operator==( pointer_type r ) const noexcept { return get()==r; }//vergleicht die per get() gelieferten pointer
		//bool operator!=( pointer_type r ) const noexcept { return get()!=r; }//vergleicht die per get() gelieferten pointer
		//friend bool operator==( pointer_type l, auto_ptr const & r ) noexcept { return l==r.get(); }//vergleicht die per get() gelieferten pointer
		//friend bool operator!=( pointer_type l, auto_ptr const & r ) noexcept { return l==r.get(); }//vergleicht die per get() gelieferten pointer

		template<typename U> bool operator< ( auto_ptr<U> const & r ) const noexcept // vergleicht raw-pointer (auch invalid), nicht den inhalt
		{
			//nicht get() da der < operator evtl. z.b. als Key in einer std::map verwendet wird. dort soll er zumindest bei der reihenfolgenpflege immer den gleichen wert liefern 
			//und nicht nullptr, wenn das objekt nicht mehr existiert. (verwendung in CAbschnitt::_WieDuplexDrucken)
			return this->share.pointer < r.share.pointer;
		}

		void swap( auto_ptr & r ) noexcept
		{
			std::swap(this->share,r.share);
			std::swap(this->Ptr,r.Ptr);
			std::swap(this->SharedPtr,r.SharedPtr);
		}
		operator pointer_type() const
		{
			return share.get();
		}

		//std::add_lvalue_reference_t<std::remove_pointer_t<pointer_type>>& operator *() const //ohne implementierung kommt genau das gleiche raus
		std::remove_pointer_t<pointer_type> & operator *() const //ohne implementierung kommt genau das gleiche raus, man kann aber keinen breakpoint setzen
		{
			return *share.get();
		}
		pointer_type operator->() const //liefert pointer
		{
			return share.get();
		}
		pointer_type get() const //liefert pointer
		{ 
			return share.get();
		}
		operator bool() const
		{
			return get()!=nullptr;
		}

		//liefert pointer, wenn owner war, sonst nullptr. ist danach nicht mehr owner, behaelt aber immer seinen pointer. der aufrufer uebernimmt damit die verantwortung. 
		//exterm gefährlich weil nicht klar ist, wie lange interner pointer gueltig ist. dangling pointer
		pointer_type  Ptr_release() noexcept 
		{
			if( this->share.share && is_owner() )
				this->share.share->valid.managed = false;
			return this->Ptr.release();
		}
		//liefert std::unique_ptr<T>, wenn owner war, sonst nullptr. ist danach nicht mehr owner, behaelt aber immer seinen pointer. der aufrufer uebernimmt damit die verantwortung. 
		//exterm gefährlich weil nicht klar ist, wie lange interner pointer gueltig ist. dangling pointer
		std::unique_ptr<T> release_as_unique_ptr() noexcept
		{	
			if( this->share.share && is_owner() )
				this->share.share->valid.managed = false;
			return std::unique_ptr<T>(this->Ptr.release());
		}
		//liefert immer pointer, gibt ggf. ownership ab, bzw. this behaelt aber immer seinen pointer. der aufrufer uebernimmt damit ggf die verantwortung fuer die resource. 
		//exterm gefährlich weil nicht klar ist, wie lange interner pointer gueltig ist. dangling pointer
		pointer_type release() noexcept
		{	
			if( this->share.share && is_owner() )
				this->share.share->valid.managed = false;
			this->Ptr.release();
			return share.get();
		}
		private:
		//trytransfer ownership wechselt, wenn dynamic_cast funktioniert und beides arrays oder eben keine arrays sind
		template<typename,bool> friend struct _trytransfer;
		//Partial Specialization
		template<typename U,bool oneisarray=(std::is_array<T>::value || std::is_array<U>::value)> struct _trytransfer{};
		template<typename U> struct _trytransfer<U,false>
		{
			auto_ptr<T> & ap;
			_trytransfer( auto_ptr<T> & ap ) : ap(ap){}

			operator auto_ptr<U>()
			{
				auto_ptr<U> retvalue( ap );
				if( ap.owner() && retvalue.get() )
				{
					ap.Ptr.release();
					retvalue.Ptr = std::unique_ptr<U>( retvalue.get() );
				}
				return retvalue;
			}
		};
		template<typename U> struct _trytransfer<U,true>
		{	
			using T_t = std::remove_pointer_t<typename auto_ptr<T>::pointer_type>;
			using U_t = std::remove_pointer_t<typename auto_ptr<U>::pointer_type>;

			static_assert( std::is_array<T>::value && std::is_array<U>::value, "es muessen schon beides arrays sein" );
			static_assert( !(std::is_const<U_t>::value==false && std::is_const<T_t>::value), "wenn T const ist muss U auch const sein" );
			static_assert( std::is_same<std::remove_const_t<T_t>,std::remove_const_t<U_t>>::value, "der datatyp der array muss identisch sein" );

			auto_ptr<T> & ap;
			_trytransfer( auto_ptr<T> & ap ) : ap(ap){}

			operator auto_ptr<U>()
			{
				auto_ptr<U> retvalue( ap );
				retvalue.Ptr = std::move(ap.Ptr);
				return retvalue;
			}
		};
		public:
		//diese funktion wird wohl nie jemand benutzen
		template<typename U> auto_ptr<U> trytransfer()  //Versucht den transfer auch von basisklasse zur ableitung. 
														//wenn es klappt verliert this ownership
														//wenn ein dynamic_cast klappt wird auf jeden fall der pointer gesetzt aber ownership bleibt ggf. bei this
		{
			return _trytransfer<U>( *this );
		}

		//weak, this behaelt pointer, bei SharedPtr wird die strong referenz NICHT erhöht
		auto_ptr weak() const
		{
			auto_ptr retvalue( *this );
			retvalue.SharedPtr.reset();//
			return retvalue;
		}
		//transfer, this behaelt pointer, verliert ggf. aber ownership. bei SharedPtr wird die strong referenz erhöht
		auto_ptr transfer()
		{
			auto_ptr retvalue( *this );
			retvalue.Ptr = std::move(this->Ptr);
			return retvalue;
		}
		//ownerless wie transfer, this behaelt pointer und ggf. ownership. bei SharedPtr wird die strong referenz erhöht. zuweisung macht genau das gleiche
		auto_ptr ownerless() const
		{
			auto_ptr retvalue( *this );
			return retvalue;
		}

		bool is_managed() const //liefert true, wenn der pointer verwaltet ist, also jeder auto_ptr zu nullptr wird, wenn das objekt zerstört wurde. z.I. release() und seine derivate setzt managed auf false.
		{
			return  operator bool() && this->share.share->valid.managed;
		}
		bool owner() const//same as is_owner() //alleiniger eigentümer
		{
			return this->Ptr.get()!=nullptr;
		}
		bool is_owner() const//same as owner() //alleiniger eigentümer
		{
			return this->Ptr.get()!=nullptr;
		}
		bool is_shared_ptr() const 
		{
			return this->SharedPtr.operator bool();
		}
		bool is_manager() const				//alleiniger eigentümer oder shared_pointer
		{
			return is_owner() || is_shared_ptr();
		}
		bool is_owner_or_shared() const		//alleiniger eigentümer oder shared_pointer
		{
			return is_owner() || is_shared_ptr();
		}
	};

	template<typename T> class managed_auto_ptr : public auto_ptr<T>
	{
	public:
		~managed_auto_ptr()
		{
		}
		managed_auto_ptr( pointer_type ) = delete;
		managed_auto_ptr() noexcept : auto_ptr() {}
		managed_auto_ptr(std::nullptr_t) noexcept : auto_ptr() {}
		managed_auto_ptr( managed_auto_ptr const & r ) noexcept : auto_ptr(r) 
		{
		}
		managed_auto_ptr( managed_auto_ptr && r ) noexcept : auto_ptr(std::move(r)) 
		{
		}
		managed_auto_ptr( auto_ptr const & r ) : auto_ptr(r) 
		{
			if( *this && is_managed()==false )
				throw std::invalid_argument( __FUNCTION__ " managed auto_ptr erwartet");
		}
		managed_auto_ptr( auto_ptr && r ) : auto_ptr(std::move(r))
		{
			if( *this && is_managed()==false )
				throw std::invalid_argument( __FUNCTION__ " managed auto_ptr erwartet");
		}
		template<typename U>
		managed_auto_ptr( auto_ptr<U> const & r ) : auto_ptr(r) 
		{
			if( *this && is_managed()==false )
				throw std::invalid_argument( __FUNCTION__ " managed auto_ptr erwartet");
		}
		template<typename U>
		managed_auto_ptr( auto_ptr<U> && r ) : auto_ptr(std::move(r))
		{
			if( *this && is_managed()==false )
				throw std::invalid_argument( __FUNCTION__ " managed auto_ptr erwartet");
		}
		template<typename U>
		managed_auto_ptr( enable_auto_ptr_from_this<U> & r ) : auto_ptr( r.auto_ptr_from_this() ) {}
		template<typename U>
		managed_auto_ptr( enable_auto_ptr_from_this<U> * r ) : auto_ptr( r?r->auto_ptr_from_this():nullptr ) {}
		managed_auto_ptr( std::unique_ptr<T> && Ptr ) noexcept : auto_ptr( std::move(Ptr) ) {}

		explicit managed_auto_ptr( std::shared_ptr<T> sharedptr ) noexcept : auto_ptr( std::move(sharedptr) ) {}//bei sharedpointer muss sich z.zt ggf. der aufrufer um den cast kümmern, dass kann ich sonst nicht mehr testen
		//auto_ptr(auto_ptr const& r) : auto_ptr(r.ownerless()){}
		template<typename U> 
		//explicit 
		managed_auto_ptr(managed_auto_ptr<U> const & r) noexcept : auto_ptr( r ) {}
		template<typename U> 
		//explicit 
		managed_auto_ptr(managed_auto_ptr<U> && r) noexcept : auto_ptr( std::move(r) ) {}//dient der konvertierung  const T = U oder T bzw U ist abgeleitet von vom Anderen, bzw. T = const U

		managed_auto_ptr & operator=( managed_auto_ptr const & r) & noexcept { auto_ptr::operator=(r); return *this; }
		managed_auto_ptr & operator=( managed_auto_ptr && r) & noexcept { auto_ptr::operator=(std::move(r)); return *this; }

		template<typename U> 
		managed_auto_ptr & operator=( enable_auto_ptr_from_this<U> * r ) & noexcept { return operator=(managed_auto_ptr(r)); }
		template<typename U> 
		managed_auto_ptr & operator=( enable_auto_ptr_from_this<U> & r ) & noexcept { return operator=(managed_auto_ptr(r)); }

	};

	template<typename T,typename ... Arg_ts> auto make_auto_ptr( Arg_ts ... args )
	{
		return auto_ptr<T>{ std::make_unique<T>(std::forward<Arg_ts>(args)...) };//make_unique kümmert sich um [] und sontige unerwartete dinge
	}
	//auto_ptr_owner_parameter hält kurzzeitig einen owner auto_ptr oder einen nullptr
	//nur als parameter fuer funktionen benutzen. member und lokale variablen immer nur als auto_ptr<T> anlegen
	template<typename T> class auto_ptr_owner_parameter 
	{
		auto_ptr<T> data;
	public:
		//usage:	 z.B.	void foo( WS::auto_ptr_owner_parameter<int> );//foo-declaration
							//foo( &int_value ); ATTENTION NEVER call foo with address of stack-object
							//foo( (int*)nullptr ); calling foo with nullptr
							//foo( new int{5} ); calling foo with pointer
							//foo( std::make_unique<int>(5) ); calling foo with unique_ptr
							//foo( std::unique_ptr<int>{} ); calling foo with unique_ptr
							//foo( std::make_shared<int>(5) ); calling foo with shared_ptr
							//foo( std::shared_ptr<int>{} ); calling foo with shared_ptr
							//foo( WS::auto_ptr<int>{} ); calling foo with auto_ptr with nullptr
							//foo( WS::auto_ptr<int>{new int{6},true} ); calling foo with auto_ptr with attrib is_owner()
							//foo( WS::auto_ptr<int>{std::make_shared<int>(7)} ); calling foo with auto_ptr with attrib is_shared_ptr() 
							//foo( WS::auto_ptr<int>{new int{6},false} ); ATTENTION exception throwing when calling foo with auto_ptr without attrib is_managed()

		auto_ptr_owner_parameter( typename auto_ptr<T>::pointer_type p ) : data( p, true ){}														// fn( new int{5} );
		auto_ptr_owner_parameter( auto_ptr<T> const & must_be_owner ) = delete;
		auto_ptr_owner_parameter( auto_ptr<T> && must_be_owner )																					// fn( WS::auto_ptr<int>(new int{5}) );
		{
			if( must_be_owner.is_manager()==false && must_be_owner!=nullptr )
				throw std::invalid_argument( __FUNCTION__ " WS::auto_ptr mit is_manager-attribut erwartet" );
			data = std::move(must_be_owner);
		}
		auto_ptr_owner_parameter( auto_ptr<T> & must_be_owner ) : auto_ptr_owner_parameter(must_be_owner.transfer()){}										// fn( ptr );//wobei ptr ein lvalue vom typ WS::auto_ptr<int> ist
		template<typename U> 
		auto_ptr_owner_parameter( U && r ) : auto_ptr_owner_parameter( auto_ptr<T>( std::move(r) ) ){}									// fn( std::unique_ptr<int>(new int{5}) );

		operator auto_ptr<T>() { return std::move( data ); }//einmaliger aufruf, danach ist data==nullptr
		auto_ptr<T> move() {return std::move( data ); }//einmaliger aufruf, danach ist data==nullptr
	};

	template<typename dest_t,typename source_t> auto_ptr<dest_t> dynamic_pointer_cast( auto_ptr<source_t> const & r )
	{
		return auto_ptr<dest_t> ( r );
	}
	template<typename dest_t,typename source_t> auto_ptr<dest_t> dynamic_pointer_cast( auto_ptr<source_t> && r )
	{
		return auto_ptr<dest_t>( std::move(r) );
	}

	template<typename T> auto CreateTPtrArray( T* p )
	{
		return auto_ptr<T[]>( p,false );
	}
	template<typename T> auto CreateTPtrArray( T* p, take_ownership autodelete )
	{
		return auto_ptr<T[]>( p,autodelete );
	}
	template<typename T> auto CreateTPtr( T* p )
	{
		return auto_ptr<T>( p,false );
	}
	template<typename T> auto CreateTPtr( T* p, take_ownership autodelete )
	{
		return auto_ptr<T>( p,autodelete );
	}
	template<typename T> auto CreateTPtr( std::unique_ptr<T> && Ptr )
	{
		return auto_ptr<T>( std::move(Ptr) );
	}
	template<typename T> auto CreateTPtr( std::shared_ptr<T> Ptr )
	{
		return auto_ptr<T>( Ptr );
	}

	//von enable_auto_ptr_from_this public ableiten, 
	//dann kann per auto_ptr_from_this ein auto_ptr besorgt werden, der automatisch null wird, wenn dtor des objekts durchlaufen wird (nie mehr ungültige pointer, yippy)
	//das funktioniert übrigens immer, egal wie das objekt erzeugt wurde, im gegensatz zu enable_shared_from_this
	template<typename this_t> class enable_auto_ptr_from_this 
	{
		using this_type = this_t;
		mutable auto_ptr<this_t> auto_this;//wird erst bei der ersten verwendung initialisiert, so werden keine resourcen ungenutzt belegt
	protected:
		enable_auto_ptr_from_this( )
		{}
	public:
		virtual ~enable_auto_ptr_from_this( ) = 0
		{
			//alle noch existierenden auto_ptr_from_this liefern ab jetzt nullptr
			this->auto_this.share.SetNullptr();
		}
	public:
		auto_ptr<this_t> auto_ptr_from_this()
		{
			if( this->auto_this==nullptr )//conditionjump besser als funktionspointerjump, so kann die CPU optimieren, behauptet das internet ohne beiweise. glaube ich aber nicht. ist aber deutlich besser lesbar als mit functionspointern
			{
				this->auto_this = auto_ptr<this_t>(dynamic_cast<this_t*>(this),false,auto_ptr<this_t>::Managed(true));
			}
			return auto_this;
		}
		auto_ptr<this_t const > auto_ptr_from_this() const
		{
			if( this->auto_this==nullptr )//conditionjump besser als funktionspointerjump, so kann die CPU optimieren, behauptet das internet ohne beiweise. glaube ich aber nicht. ist aber deutlich besser lesbar als mit functionspointern
			{
				//const_cast ist noetig
				auto_this = auto_ptr<this_t>(dynamic_cast<this_t*>(const_cast<enable_auto_ptr_from_this*>(this)),false,auto_ptr<this_t>::Managed(true));
			}
			return auto_this;
		}
		//gibt nur aerger
		//auto_ptr<this_t> operator &() const
		//{
		//	return auto_ptr_from_this();
		//}
	};

}

//spielerei auto_ptr_vw. you dont need it
namespace std
{
	template< class _Ty
			, class _Alloc //= allocator<_Ty> 
	> class vector;
}
namespace WS
{
	//usage: auto_ptr_vw<int[,std::vector|,std::deque]>//den alloator bzw zusätzliche template-parameter bekomme ich im moment nicht hin
	template<typename T, class container_type=std::vector<auto_ptr<T>,std::allocator<auto_ptr<T>>>> class auto_ptr_vw
	{
	public:
		using container_t = container_type;
		using pointer_type = typename auto_ptr<T>::pointer_type;
		using pointer_t = pointer_type;

	private:
		container_t container;

	public:
		auto begin() const{return container.begin();}//nur als const implementiert, weil man sonst eine referenz auf den owner bekäme
		auto end() const{return container.end();}
		auto size() const{return container.size();}
		auto_ptr<T> at( size_t index ) { return container.at(index); }//liefert kopie. referenz wäre owner mit evtl. fatalen folgen
		auto_ptr<T> operator[]( size_t index ) { return container[index]; }//liefert kopie. referenz wäre owner mit evtl. fatalen folgen

		auto & push_back( auto_ptr_owner_parameter<T> auto_ptr_owner )//return_value ohne ref, also als kopie, sonst waere es eine refenez auf owner
		{
			return container.emplace_back( auto_ptr_owner.move() );
		}
		void clear()
		{
			container.clear();
		}

		//erase liefert nullptr bei fehler oder owner 
		template<typename U> auto_ptr<T> erase( auto_ptr<U> const & erasevalue )
		{
			auto iter = std::find( container.begin(), container.end(), erasevalue );
			if( iter == container.end() )
				return nullptr;

			auto retvalue = iter->transfer();
			container.erase( iter );
			return retvalue;
		}
		auto_ptr<T> erase( pointer_t p ) { return erase( auto_ptr<T>( p ) ); }
		
		//replace liefert nullptr bei fehler oder owner 
		template<typename U> auto_ptr<T> replace( auto_ptr<T> const & replacevalue, auto_ptr_owner_parameter<U> replacewith )
		{
			auto iter = std::find( container.begin(), container.end(), replacevalue );
			if( iter == container.end() )
				return nullptr;
			
			auto retvalue = iter->transfer();
			*iter = replacewith;
			return retvalue;
		}
		template<typename U, typename V> auto_ptr<T> replace( U replacevalue, V replacewith ) { return replace( auto_ptr<T>{replacevalue}, auto_ptr_owner_parameter<T>{replacewith}); }
	};
}
