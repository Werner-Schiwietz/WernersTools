#pragma once
//WP::auto_ptr by Werner Schiwietz 
//WP::auto_ptr ist eine abwandlung vom std::unique_ptr
//im konstruktor kann entschieden werden, ob die resource übernommen wird oder nicht. also das NICHT ist der unterschied zum std::unique_ptr
//z.B. auto_ptr<CZotZeile>( pZeile, false ) das auto_ptr-Objekt benutzt pZeile gibt das objekt aber nicht frei. oder auto_ptr<CZotZeile>( pZeile )
//z.B. auto_ptr<CZotZeile>( pZeile, true ) das auto_ptr-Objekt benutzt pZeile und ruft delete pZeile im destruktor auf
//z.B. auto_ptr<CZotZeile>( std::move(stdunique_ptr) ) das auto_ptr-Objekt übernimmt den std::unique_ptr inhalt std::unique_ptr nur mit default_delete
//z.B. auto_ptr<CZotZeile>( stdshared_ptr) ) das auto_ptr-Objekt übernimmt eine strong referenz  des std::shared_ptr. es gibt keinen owner der letzte shared_ptr gibt objekt frei
//z.B. auto_ptr<char[]>( pString, true ) das auto_ptr-Objekt pString ist mit new char[x] angelegt worden und wird im dtor mit delete [] pString freigegeben
//tests in BasisUnitTests\UT_dtor_call.cpp TEST_CLASS(UT_auto_ptr)
//
//wird konsequent WP::auto_ptr verwendet (also ein owner und 0-n nichtowner bzw  shared_ptr und weak_ptr) wird nicht mehr auf freigegebene pointer zugegriffen. diese sind statt dessen ggf. nullptr.
//in neuen code besser reine std::shared_pointer verwenden
//Kopien eines auto_ptr 
//  mit transfer(), neues objekt wird owner wenn ursprungsobjekt owner war
// oder ownerless(), kopie wird nie owner
//
//ausserdem gibt es enable_auto_ptr_from_this siehe enable_shared_from_this
//		struct A : public enable_auto_ptr_from_this<A>{};
//		A a;
//		WP::auto_ptr<A> ptr = a.auto_ptr_from_this();//ptr wird nullptr, wenn a zerstoert wird



#include <memory>
#include <atomic>


//#define LINE_STRING2(x) #x				//macht aus der zahl einen sting
//#define LINE_STRING1(x) LINE_STRING2(x)	//nötig, damit __LINE__ zur Zahl wird
//#define _LINE_ LINE_STRING1(__LINE__)		//in pragma message kann _LINE_ verwendet wwerden

namespace WP
{
	template<typename pointer_t> struct ReferenzCounter
	{
		using pointer_type = pointer_t;
		struct ReferenzCounterShare // gemeinsamer datenpool fuer owner und nicht owner, damit ggf. des nichtowners pointer zum nullptr werden kann
		{
			mutable std::atomic<unsigned short>	counter = 1;
			bool								valid	= false;
		private:
			~ReferenzCounterShare(){}
			ReferenzCounterShare()=delete;
			ReferenzCounterShare( pointer_type const & p ) : valid(p!=nullptr){}
			ReferenzCounterShare( ReferenzCounterShare const & ) = delete;
		public:
			ReferenzCounterShare* Release()
			{
				if( --counter == 0 )
					delete this;
				return nullptr;
			}
			static ReferenzCounterShare* Create( pointer_type const & p)
			{
#				ifdef AUTOPTR_MEMLEAKDETECTION
					DumpAllocStack dumpit;
#				endif
				return new ReferenzCounterShare( p );
			}
			ReferenzCounterShare* AddRef( ) const
			{
				if(this)
					++this->counter;
				return const_cast<ReferenzCounterShare*>(this);
			}

			void swap( ReferenzCounterShare & r )
			{
				std::swap( this->valid, r.valid );
				std::swap( this->counter, r.counter );
			}
		};

		ReferenzCounterShare*	share	= nullptr;
		pointer_type			pointer = nullptr;//nötig, weil dynamic_cast ggf. auch veraenderte adressen liefert, das sharedobjekt sich den pointer nicht merken kann
												  //man könnte das ganze um template data_pointer_type erweitern, dann waeren auto_ptr<A,A*> etwas anderes als auto_ptr<A,B*>. hatte ich schon, war auch seltsam
		~ReferenzCounter()
		{
			if( share )
				share = share->Release();
		}
		ReferenzCounter( ReferenzCounter && r ) : ReferenzCounter()
		{
			swap( r );
		}
		ReferenzCounter& operator=( ReferenzCounter const & r )
		{
			ReferenzCounter temp( r );
			swap( temp );
			return *this;
		}
		ReferenzCounter& operator=( ReferenzCounter && r )
		{
			ReferenzCounter temp( std::move(r) );
			swap( temp );
			return *this;
		}
		ReferenzCounter() {}
		ReferenzCounter( ReferenzCounter const & r ) : share(r.share->AddRef()), pointer(r.pointer ) {}
		ReferenzCounter( pointer_type p ) 
			: share(p ? ReferenzCounterShare::Create(p) : nullptr)
			, pointer(p) {}

		template< typename U> ReferenzCounter( ReferenzCounter<U> const & r  )
		{
			//reinterpret_cast nötig, weil nested und ReferenzCounter<T>::ReferenzCounterShare* fuer den compiler etwas anders als ReferenzCounter<U>::ReferenzCounterShare* ist
			this->share = reinterpret_cast<ReferenzCounterShare*>(r.share ? r.share->AddRef() : nullptr);
			if(valid())
				//je nach ableitung kann schon mal eine andere adresse heraus kommen
				this->pointer = dynamic_cast<pointer_type>( r.get() );
		}

		bool valid() const
		{
			return this->share && this->share->valid;
		}

		void swap( ReferenzCounter & r )
		{
			std::swap( this->share, r.share );
			std::swap( this->pointer, r.pointer);
		}
		void SetNullptr()
		{
			if( this->share )
				this->share->valid = false;
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

	template<typename T> class auto_ptr
	{
		template<typename> friend class auto_ptr;
	public:
		typedef typename std::remove_all_extents_t<T> * pointer_type;//ggf die [] aus T entfernen
	private:
		mutable ReferenzCounter<pointer_type> share;
		mutable std::unique_ptr<T> Ptr;
		mutable std::shared_ptr<T> SharedPtr;

	public:
		auto_ptr( auto_ptr const & r )
			: share( r.share )
			, SharedPtr(r.SharedPtr)
		{
		}

	public:
		template<typename T> friend class enable_auto_ptr_from_this;
		~auto_ptr()
		{
			if( Ptr || SharedPtr.use_count()==1 )
				share.SetNullptr();
		}
		auto_ptr( pointer_type p, bool autodelete ) 
			: share(p)
			, Ptr(autodelete ? p : nullptr){}
		//explicit
		auto_ptr( pointer_type p ) : auto_ptr( p, false ){}
		//explicit 
		auto_ptr( std::unique_ptr<T> && Ptr ) 
			: share(Ptr.get())
			, Ptr(std::move(Ptr))
		{}
		explicit auto_ptr( std::shared_ptr<T> sharedptr ) //bei sharedpointer muss sich z.zt ggf. der aufrufer um den cast kümmern, dass kann ich sonst nicht mehr testen
			: share(sharedptr.get())
			, SharedPtr(sharedptr)
		{}
		auto_ptr& Set( std::shared_ptr<T> sharedptr ) //operator= gibt mit dem anderen operatoren aerger
		{
			auto_ptr temp( sharedptr );
			swap( temp );
			return *this;
		}
		auto_ptr() = default;
		auto_ptr(auto_ptr && r) : auto_ptr()
		{
			swap( r );
		}
		//auto_ptr(auto_ptr const& r) : auto_ptr(r.ownerless()){}
		template<typename U> 
		//explicit 
		auto_ptr(auto_ptr<U> const & r)
			: share( nullptr )
		{
			static_assert( std::is_convertible<auto_ptr<U>::pointer_type,pointer_type>::value 
						   || std::is_convertible<pointer_type,auto_ptr<U>::pointer_type>::value 
							, __FUNCTION__ " pointer sind nicht zuweisbar");

			share = ReferenzCounter<pointer_type>( r.share );
		}
		template<typename U> 
		//explicit 
		auto_ptr(auto_ptr<U> && r)//dient der konvertierung  const T = U oder T bzw U ist abgeleitet von vom Anderen, bzw. T = const U
		{	//this muss neuer owner werden, da muessen einige bedingungen erfüllt sein

			//nicht nötig aber, wg is_convertible aber macht die problemfindung einfacher
			static_assert( std::is_const<U>::value==false 
						|| std::is_const<T>::value==true
						, "wenn U const ist muss T auch const sein");

			static_assert( std::is_convertible<auto_ptr<U>::pointer_type,pointer_type>::value 
						   || std::is_convertible<pointer_type,auto_ptr<U>::pointer_type>::value 
							, __FUNCTION__ " pointer sind nicht zuweisbar");

			using Tpt = WP::auto_ptr<T>::pointer_type;
			using Upt = WP::auto_ptr<U>::pointer_type;
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
							, "U kann nicht zu T mit owneruebername werden" );

			auto_ptr<U> tempU;
			tempU.swap( r );

			this->share = ReferenzCounter<pointer_type>( tempU.share );

			//wenn es unique_ptr nicht kann, geht es halt nicht
			this->Ptr = std::move(tempU.Ptr);
		}

		auto_ptr & operator=(auto_ptr && r)
		{
			auto_ptr temp( std::move(r) );
			this->swap( temp );
			return *this;
		}
		auto_ptr & operator=(auto_ptr const & r) 
		{
			return operator=(r.ownerless());
		}
		template<typename U>auto_ptr & operator=(auto_ptr<U> && r)
		{
			static_assert( std::is_base_of<U, T>::value
						|| std::is_base_of<T, U>::value
						, __FUNCTION__ " pointer sind nicht zuweisbar");

			auto_ptr temp( r.transfer() );
			swap( temp );

			return *this;
		}
		template<typename U>auto_ptr & operator=(auto_ptr<U> const & r)
		{
			static_assert( std::is_base_of<U, T>::value
						   || std::is_base_of<T, U>::value
						, __FUNCTION__ " pointer sind nicht zuweisbar");

			auto_ptr<T> temp( r );

			swap( temp );
			return *this;
		}
		auto_ptr & operator=(std::unique_ptr<T> && ptr)
		{
			return operator=( auto_ptr(std::move(ptr)) );
		}
		auto_ptr & operator=(pointer_type ptr)
		{
			if (this->get() == ptr)
				return *this;
			return operator=( auto_ptr(ptr) );
		}

		template<typename U> bool operator< ( auto_ptr<U> const & r ) const // vergleicht pointer, nicht den unhalt
		{
			//nicht get() da der < operator evtl. z.b. als Key in einer std::map verwendet wird. dort soll er zumindest bei der reihenfolgenpflege immer den gleichen wert liefern 
			//und nicht nullptr, wenn das objekt nicht mehr existiert. (verwendung in CAbschnitt::_WieDuplexDrucken)
			return this->share.pointer < r.share.pointer;
		}

		void swap( auto_ptr & r )
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
		pointer_type operator->() const//liefert pointer
		{
			return share.get();
		}
		pointer_type get() const//liefert pointer
		{ 
			return share.get();
		}
		operator bool() const
		{
			return get()!=nullptr;
		}

		//liefert pointer, wenn owner war, sonst nullptr. ist danach nicht mehr owner, behaelt aber immer seinen pointer. der aufrufer uebernimmt damit die verantwortung. 
		//exterm gefährlich weil nicht klar ist, wie lange interner pointer gueltig ist
		pointer_type  Ptr_release() 
		{
			return this->Ptr.release();
		}
		//liefert immer pointer, gibt ggf. ownership ab, bzw. this behaelt aber immer seinen pointer. der aufrufer uebernimmt damit ggf die verantwortung fuer die resource. 
		//exterm gefährlich weil nicht klar ist, wie lange interner pointer gueltig ist
		pointer_type release()
		{	
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
														//wenn es klappt verliefert this ownership
														//wenn ein dynamic_cast klappt wird auf jeden fall der pointer gesetzt aber ownership bleibt ggf. bei this
		{
			return _trytransfer<U>( *this );
		}

		//weak wie release, this behaelt pointer, bei SharedPtr wird die strong referenz NICHT erhöht
		auto_ptr weak() const
		{
			auto_ptr retvalue( *this );
			retvalue.SharedPtr.reset();//
			return retvalue;
		}
		//transfer wie release, this behaelt pointer, verliert ggf. aber ownership. bei SharedPtr wird die strong referenz erhöht
		auto_ptr transfer() const
		{
			auto_ptr retvalue( *this );
			retvalue.Ptr = std::move(this->Ptr);
			return retvalue;
		}
		//ownerless wie transfer, this behaelt pointer und ggf. ownership. bei SharedPtr wird die strong referenz erhöht
		auto_ptr ownerless() const
		{
			auto_ptr retvalue( *this );
			return retvalue;
		}

		bool owner() const
		{
			return this->Ptr.get()!=nullptr;
		}
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
	template<typename T> auto CreateTPtrArray( T* p, bool autodelete )
	{
		return auto_ptr<T[]>( p,autodelete );
	}
	template<typename T> auto CreateTPtr( T* p )
	{
		return auto_ptr<T>( p,false );
	}
	template<typename T> auto CreateTPtr( T* p, bool autodelete )
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
		auto_ptr<this_t> const & auto_ptr_from_this() const//wichtig das retvalue const & ist. als rvalue wuerden die zuweisung mit transfer versucht, das klappt bei ableitung aber nicht
		{
			if( auto_this==nullptr )//conditionjump besser als funktionspointerjump, so kann die CPU optimieren, behauptet das internet ohne beiweise. glaube ich aber nicht. ist aber deutlich besser lesbar als mit functionspointern
				auto_this = auto_ptr<this_t>(dynamic_cast<this_t*>(const_cast<enable_auto_ptr_from_this*>(this)));
			return auto_this;
		}
	};
}

