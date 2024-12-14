#include "streamPipe.h"
#include <tchar.h>

namespace
{
	std::atomic<size_t>	aktzaehler{};
	size_t skipped = 0;
	size_t last_out_len = 0;
}

pipedata::pipedata( pipedata && r )
	: zaehlerwert( r.zaehlerwert )
	, output( r.output )
	, ostream(r.ostream)
	, skipable(r.skipable)
{
}
pipedata::pipedata( string_t output, ostream_t& ostream, bool skipable )
	: zaehlerwert(++aktzaehler)
	, output(output)
	, ostream(ostream)
	, skipable(skipable)
{
}
pipedata::pipedata( string_t output, ostream_t& ostream )
	: pipedata(output, ostream, false)
{
}


void pipeworker( pipedata && data )
{
	bool skipoutput = static_cast<size_t>(aktzaehler) != data.zaehlerwert;
	if( data.skipable && skipoutput )
	{
		++skipped;
		return;
	}

	if( &data.ostream != &errstream )
	{
		auto len = data.output.length();
		if( last_out_len > len )
			data.ostream << string_t(last_out_len,_T(' ')) << _T('\r');//alten text löschen
	}

	data.ostream << data.output;
	if( data.skipable )
		last_out_len = data.output.length();
	else
		last_out_len = 0;
}

