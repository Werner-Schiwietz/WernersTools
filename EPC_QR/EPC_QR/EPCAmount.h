#pragma once
#include <array>

class CEuroAmount
{
public:
    static constexpr __int64 InvalidCents = -1;
private:
    __int64 cents{ InvalidCents }; // -1 = ungültig, 0 = leer (erlaubt), >0 = gültiger Betrag

public:
    // Bewusst leerer Betrag (für Blanko-Überweisungen erlaubt)
    constexpr CEuroAmount() noexcept : cents{ 0 } {}

    // Direkt aus Cent: bei ungültigem Wert bleibt cents automatisch auf InvalidCents (-1)
    explicit constexpr CEuroAmount( __int64 centsValue ) noexcept
	{
		if( IsValid( centsValue ) )
			this->cents = centsValue;
	}


    // Aus lokaler Eingabe: bei jedem frühen return bleibt cents automatisch auf InvalidCents (-1)
    explicit CEuroAmount( CString rawText );

    static constexpr bool	IsValid( __int64 centsValue ) noexcept	{ return centsValue >= 0 && centsValue <= 99999999999LL; }
    constexpr bool			IsValid() const noexcept				{ return IsValid( this->cents ); }

    explicit constexpr operator bool() const noexcept { return this->IsValid(); }
    constexpr bool IsEmpty() const noexcept { return this->cents == 0 || not IsValid(); }
    constexpr __int64 GetCents() const noexcept { if(IsValid()) return this->cents; return 0; }//0 oder invalid, ich liefere 0

    CString get() const;//amount für qrgencode

    operator CString() const { return this->get(); }

    CString FormatForDisplay() const;
};


