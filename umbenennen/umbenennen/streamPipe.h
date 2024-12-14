#pragma once

#include "traits.h"

#include <atomic>

#include "..\..\headeronly\pipe.h"


struct pipedata
{
	size_t					zaehlerwert;
	string_t				output;
	ostream_t&				ostream;
	bool					skipable = false;

	pipedata() = delete;
	pipedata( pipedata const & ) = delete;
	pipedata( pipedata && );
	pipedata( string_t output, ostream_t& ostream, bool skipable );
	pipedata( string_t output, ostream_t& ostream );
};

void pipeworker( pipedata && data );


