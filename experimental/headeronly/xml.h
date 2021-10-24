#pragma once

#include "..\..\headeronly\parse_helper.h"
#include "..\..\headeronly\is_in.h"

//https://www.w3.org/TR/xml/

namespace WS { namespace XML 
{
	template<typename char_t> constexpr char_t _open(){return '<';}
	template<typename char_t> constexpr char_t _close(){return '>';}
	template<typename char_t> constexpr char_t _space(){return ' ';}
	template<typename char_t> constexpr char_t _assign(){return '=';}
	template<typename char_t> constexpr char_t _ampersand(){return '&';}
	template<typename char_t> constexpr char_t _semicolon(){return ';';}
	template<typename char_t> constexpr char_t _doppelteshochkomma(){return '"';}
	template<typename char_t> constexpr char_t _hochkomma(){return '\'';}
	template<typename char_t> constexpr char_t _doppelkreuz(){return '#';}

	template<typename char_t> constexpr char_t _lt(){return '<';}
	template<typename iterator_access_t> constexpr iterator_access_t _lt_ref(){return iterator_access("lt"); }
	template<> _iterator_access<wchar_t const *> _lt_ref<_iterator_access<wchar_t const *>>(){return iterator_access(L"lt"); }
	template<typename char_t> constexpr char_t _gt(){return '>';}
	template<typename iterator_access_t> constexpr iterator_access_t _gt_ref(){return iterator_access("gt"); }
	template<> _iterator_access<wchar_t const *> _gt_ref<_iterator_access<wchar_t const *>>(){return iterator_access(L"gt"); }
	template<typename char_t> constexpr char_t _amp(){return '&';}
	template<typename iterator_access_t> constexpr iterator_access_t _amp_ref(){return iterator_access("amp"); }
	template<> _iterator_access<wchar_t const *> _amp_ref<_iterator_access<wchar_t const *>>(){return iterator_access(L"amp"); }
	template<typename char_t> constexpr char_t _quot(){return '"';}
	template<typename iterator_access_t> constexpr iterator_access_t _quot_ref(){return iterator_access("quot"); }
	template<> _iterator_access<wchar_t const *> _quot_ref<_iterator_access<wchar_t const *>>(){return iterator_access(L"quot"); }
	template<typename char_t> constexpr char_t _apos(){return '\'';}
	template<typename iterator_access_t> constexpr iterator_access_t _apos_ref(){return iterator_access("apos"); }
	template<> _iterator_access<wchar_t const *> _apos_ref<_iterator_access<wchar_t const *>>(){return iterator_access(L"apos"); }


	template<typename char_t> constexpr auto const & whitespace()
	{ 
		static const char_t chars[]={char_t(' '),char_t('\x9'),char_t('\xa'),char_t('\xd')};
		static const auto retvalue = array_iterator_access(chars);
		return retvalue;
	}

	template<typename char_t> constexpr auto comment_begin_tag()
	{ 
		static const char_t chars[]={char_t('<'),char_t('!'),char_t('-'),char_t('-')};
		static const auto retvalue = iterator_access(chars,_countof(chars));
		return retvalue;
	}
	template<typename char_t> constexpr auto minus_minus()
	{ 
		static const char_t chars[]={char_t('-'),char_t('-')};
		static const auto retvalue = array_iterator_access(chars);
		return retvalue;
	}
	template<typename char_t> constexpr auto comment_end_tag()
	{ 
		static const char_t chars[]={char_t('-'),char_t('-'),char_t('>')};
		static const auto retvalue = array_iterator_access(chars);
		return retvalue;
	}
	template<typename char_t> constexpr auto prolog_begin_tag()
	{ 
		static const char_t chars[]={char_t('<'),char_t('?')};
		static const auto retvalue = iterator_access(chars,_countof(chars));
		return retvalue;
	}
	template<typename char_t> constexpr auto prolog_end_tag()
	{ 
		static const char_t chars[]={char_t('?'),char_t('>')};
		static const auto retvalue = array_iterator_access(chars);
		return retvalue;
	}

	template<typename char_t> bool _is_char( char_t ch ) = delete;
	bool _is_char( wchar_t ch )
	{
		return is_in( ch
					  , L'\x09', L'\x0a', L'\x0d'
					  , bereich(L'\x0021',L'\xD7FF') 
					  , bereich(L'\xE000',L'\xFFFE') 
		);
	}

	template<typename char_t> bool _is_basechar( char_t ch ) = delete;
	bool _is_basechar( wchar_t ch ) 
	{
		return is_in( ch
					  , bereich(L'\x0041',L'\x005A') 
					  , bereich(L'\x0061',L'\x007A') 
					  , bereich(L'\x00C0',L'\x00D6') 
					  , bereich(L'\x00D8',L'\x00F6') 
					  , bereich(L'\x00F8',L'\x00FF') 
					  , bereich(L'\x0100',L'\x0131') 
					  , bereich(L'\x0134',L'\x013E') 
					  , bereich(L'\x0141',L'\x0148') 
					  , bereich(L'\x014A',L'\x017E') 
					  , bereich(L'\x0180',L'\x01C3') 
					  , bereich(L'\x01CD',L'\x01F0') 
					  , bereich(L'\x01F4',L'\x01F5') 
					  , bereich(L'\x01FA',L'\x0217') 
					  , bereich(L'\x0250',L'\x02A8') 
					  , bereich(L'\x02BB',L'\x02C1') 
					  , L'\x0386' 
					  , bereich(L'\x0388',L'\x038A') 
					  , L'\x038C' 
					  , bereich(L'\x038E',L'\x03A1') 
					  , bereich(L'\x03A3',L'\x03CE') 
					  , bereich(L'\x03D0',L'\x03D6') 
					  , L'\x03DA' 
					  , L'\x03DC' 
					  , L'\x03DE' 
					  , L'\x03E0' 
					  , bereich(L'\x03E2',L'\x03F3') 
					  , bereich(L'\x0401',L'\x040C') 
					  , bereich(L'\x040E',L'\x044F') 
					  , bereich(L'\x0451',L'\x045C') 
					  , bereich(L'\x045E',L'\x0481') 
					  , bereich(L'\x0490',L'\x04C4') 
					  , bereich(L'\x04C7',L'\x04C8') 
					  , bereich(L'\x04CB',L'\x04CC') 
					  , bereich(L'\x04D0',L'\x04EB') 
					  , bereich(L'\x04EE',L'\x04F5') 
					  , bereich(L'\x04F8',L'\x04F9') 
					  , bereich(L'\x0531',L'\x0556') 
					  , L'\x0559' 
					  , bereich(L'\x0561',L'\x0586') 
					  , bereich(L'\x05D0',L'\x05EA') 
					  , bereich(L'\x05F0',L'\x05F2') 
					  , bereich(L'\x0621',L'\x063A') 
					  , bereich(L'\x0641',L'\x064A') 
					  , bereich(L'\x0671',L'\x06B7') 
					  , bereich(L'\x06BA',L'\x06BE') 
					  , bereich(L'\x06C0',L'\x06CE') 
					  , bereich(L'\x06D0',L'\x06D3') 
					  , L'\x06D5' 
					  , bereich(L'\x06E5',L'\x06E6') 
					  , bereich(L'\x0905',L'\x0939') 
					  , L'\x093D' 
					  , bereich(L'\x0958',L'\x0961') 
					  , bereich(L'\x0985',L'\x098C') 
					  , bereich(L'\x098F',L'\x0990') 
					  , bereich(L'\x0993',L'\x09A8') 
					  , bereich(L'\x09AA',L'\x09B0') 
					  , L'\x09B2' 
					  , bereich(L'\x09B6',L'\x09B9') 
					  , bereich(L'\x09DC',L'\x09DD') 
					  , bereich(L'\x09DF',L'\x09E1') 
					  , bereich(L'\x09F0',L'\x09F1') 
					  , bereich(L'\x0A05',L'\x0A0A') 
					  , bereich(L'\x0A0F',L'\x0A10') 
					  , bereich(L'\x0A13',L'\x0A28') 
					  , bereich(L'\x0A2A',L'\x0A30') 
					  , bereich(L'\x0A32',L'\x0A33') 
					  , bereich(L'\x0A35',L'\x0A36') 
					  , bereich(L'\x0A38',L'\x0A39') 
					  , bereich(L'\x0A59',L'\x0A5C') 
					  , L'\x0A5E' 
					  , bereich(L'\x0A72',L'\x0A74') 
					  , bereich(L'\x0A85',L'\x0A8B') 
					  , L'\x0A8D' 
					  , bereich(L'\x0A8F',L'\x0A91') 
					  , bereich(L'\x0A93',L'\x0AA8') 
					  , bereich(L'\x0AAA',L'\x0AB0') 
					  , bereich(L'\x0AB2',L'\x0AB3') 
					  , bereich(L'\x0AB5',L'\x0AB9') 
					  , L'\x0ABD' 
					  , L'\x0AE0' 
					  , bereich(L'\x0B05',L'\x0B0C') 
					  , bereich(L'\x0B0F',L'\x0B10') 
					  , bereich(L'\x0B13',L'\x0B28') 
					  , bereich(L'\x0B2A',L'\x0B30') 
					  , bereich(L'\x0B32',L'\x0B33') 
					  , bereich(L'\x0B36',L'\x0B39') 
					  , L'\x0B3D' 
					  , bereich(L'\x0B5C',L'\x0B5D') 
					  , bereich(L'\x0B5F',L'\x0B61') 
					  , bereich(L'\x0B85',L'\x0B8A') 
					  , bereich(L'\x0B8E',L'\x0B90') 
					  , bereich(L'\x0B92',L'\x0B95') 
					  , bereich(L'\x0B99',L'\x0B9A') 
					  , L'\x0B9C' 
					  , bereich(L'\x0B9E',L'\x0B9F') 
					  , bereich(L'\x0BA3',L'\x0BA4') 
					  , bereich(L'\x0BA8',L'\x0BAA') 
					  , bereich(L'\x0BAE',L'\x0BB5') 
					  , bereich(L'\x0BB7',L'\x0BB9') 
					  , bereich(L'\x0C05',L'\x0C0C') 
					  , bereich(L'\x0C0E',L'\x0C10') 
					  , bereich(L'\x0C12',L'\x0C28') 
					  , bereich(L'\x0C2A',L'\x0C33') 
					  , bereich(L'\x0C35',L'\x0C39') 
					  , bereich(L'\x0C60',L'\x0C61') 
					  , bereich(L'\x0C85',L'\x0C8C') 
					  , bereich(L'\x0C8E',L'\x0C90') 
					  , bereich(L'\x0C92',L'\x0CA8') 
					  , bereich(L'\x0CAA',L'\x0CB3') 
					  , bereich(L'\x0CB5',L'\x0CB9') 
					  , L'\x0CDE' 
					  , bereich(L'\x0CE0',L'\x0CE1') 
					  , bereich(L'\x0D05',L'\x0D0C') 
					  , bereich(L'\x0D0E',L'\x0D10') 
					  , bereich(L'\x0D12',L'\x0D28') 
					  , bereich(L'\x0D2A',L'\x0D39') 
					  , bereich(L'\x0D60',L'\x0D61') 
					  , bereich(L'\x0E01',L'\x0E2E') 
					  , L'\x0E30' 
					  , bereich(L'\x0E32',L'\x0E33') 
					  , bereich(L'\x0E40',L'\x0E45') 
					  , bereich(L'\x0E81',L'\x0E82') 
					  , L'\x0E84' 
					  , bereich(L'\x0E87',L'\x0E88') 
					  , L'\x0E8A' 
					  , L'\x0E8D' 
					  , bereich(L'\x0E94',L'\x0E97') 
					  , bereich(L'\x0E99',L'\x0E9F') 
					  , bereich(L'\x0EA1',L'\x0EA3') 
					  , L'\x0EA5' 
					  , L'\x0EA7' 
					  , bereich(L'\x0EAA',L'\x0EAB') 
					  , bereich(L'\x0EAD',L'\x0EAE') 
					  , L'\x0EB0' 
					  , bereich(L'\x0EB2',L'\x0EB3') 
					  , L'\x0EBD' 
					  , bereich(L'\x0EC0',L'\x0EC4') 
					  , bereich(L'\x0F40',L'\x0F47') 
					  , bereich(L'\x0F49',L'\x0F69') 
					  , bereich(L'\x10A0',L'\x10C5') 
					  , bereich(L'\x10D0',L'\x10F6') 
					  , L'\x1100' 
					  , bereich(L'\x1102',L'\x1103') 
					  , bereich(L'\x1105',L'\x1107') 
					  , L'\x1109' 
					  , bereich(L'\x110B',L'\x110C') 
					  , bereich(L'\x110E',L'\x1112') 
					  , L'\x113C' 
					  , L'\x113E' 
					  , L'\x1140' 
					  , L'\x114C' 
					  , L'\x114E' 
					  , L'\x1150' 
					  , bereich(L'\x1154',L'\x1155') 
					  , L'\x1159' 
					  , bereich(L'\x115F',L'\x1161') 
					  , L'\x1163' 
					  , L'\x1165' 
					  , L'\x1167' 
					  , L'\x1169' 
					  , bereich(L'\x116D',L'\x116E') 
					  , bereich(L'\x1172',L'\x1173') 
					  , L'\x1175' 
					  , L'\x119E' 
					  , L'\x11A8' 
					  , L'\x11AB' 
					  , bereich(L'\x11AE',L'\x11AF') 
					  , bereich(L'\x11B7',L'\x11B8') 
					  , L'\x11BA' 
					  , bereich(L'\x11BC',L'\x11C2') 
					  , L'\x11EB' 
					  , L'\x11F0' 
					  , L'\x11F9' 
					  , bereich(L'\x1E00',L'\x1E9B') 
					  , bereich(L'\x1EA0',L'\x1EF9') 
					  , bereich(L'\x1F00',L'\x1F15') 
					  , bereich(L'\x1F18',L'\x1F1D') 
					  , bereich(L'\x1F21',L'\x1F45') 
					  , bereich(L'\x1F48',L'\x1F4D') 
					  , bereich(L'\x1F50',L'\x1F57') 
					  , L'\x1F59' 
					  , L'\x1F5B' 
					  , L'\x1F5D' 
					  , bereich(L'\x1F5F',L'\x1F7D') 
					  , bereich(L'\x1F80',L'\x1FB4') 
					  , bereich(L'\x1FB6',L'\x1FBC') 
					  , L'\x1FBE' 
					  , bereich(L'\x1FC2',L'\x1FC4') 
					  , bereich(L'\x1FC6',L'\x1FCC') 
					  , bereich(L'\x1FD0',L'\x1FD3') 
					  , bereich(L'\x1FD6',L'\x1FDB') 
					  , bereich(L'\x1FE0',L'\x1FEC') 
					  , bereich(L'\x1FF2',L'\x1FF4') 
					  , bereich(L'\x1FF6',L'\x1FFC') 
					  , L'\x2126' 
					  , bereich(L'\x212A',L'\x212B') 
					  , L'\x212E' 
					  , bereich(L'\x2180',L'\x2182') 
					  , bereich(L'\x3041',L'\x3094') 
					  , bereich(L'\x30A1',L'\x30FA') 
					  , bereich(L'\x3105',L'\x312C') 
					  , bereich(L'\xAC00',L'\xD7A3') 
				  );

	}

	template<typename char_t> bool _is_ideographic( char_t ch ) = delete;
	bool _is_ideographic( wchar_t ch )
	{
		return is_in( ch
						  , bereich(L'\x4E00',L'\x9FA5')
						  , L'\x3007'
						  , bereich(L'\x3021',L'\x3029') ); 
	}

	template<typename char_t> bool _is_combiningchar( char_t ch ) = delete;
	bool _is_combiningchar( wchar_t ch )
	{
		return is_in( ch
					  , bereich(L'\x0300',L'\x0345') , bereich(L'\x0360',L'\x0361') , bereich(L'\x0483',L'\x0486') , bereich(L'\x0591',L'\x05A1') , bereich(L'\x05A3',L'\x05B9') , bereich(L'\x05BB',L'\x05BD') , L'\x05BF' , bereich(L'\x05C1',L'\x05C2') , L'\x05C4' , bereich(L'\x064B',L'\x0652') , L'\x0670' , bereich(L'\x06D6',L'\x06DC') , bereich(L'\x06DD',L'\x06DF') , bereich(L'\x06E0',L'\x06E4') , bereich(L'\x06E7',L'\x06E8') , bereich(L'\x06EA',L'\x06ED') , bereich(L'\x0901',L'\x0903') , L'\x093C' , bereich(L'\x093E',L'\x094C') , L'\x094D' , bereich(L'\x0951',L'\x0954') , bereich(L'\x0962',L'\x0963') , bereich(L'\x0981',L'\x0983') , L'\x09BC' , L'\x09BE' , L'\x09BF' , bereich(L'\x09C0',L'\x09C4') , bereich(L'\x09C7',L'\x09C8') , bereich(L'\x09CB',L'\x09CD') , L'\x09D7' , bereich(L'\x09E2',L'\x09E3') , L'\x0A02' , L'\x0A3C' , L'\x0A3E' , L'\x0A3F' , bereich(L'\x0A40',L'\x0A42') , bereich(L'\x0A47',L'\x0A48') , bereich(L'\x0A4B',L'\x0A4D') , bereich(L'\x0A70',L'\x0A71') , bereich(L'\x0A81',L'\x0A83') , L'\x0ABC' , bereich(L'\x0ABE',L'\x0AC5') , bereich(L'\x0AC7',L'\x0AC9') , bereich(L'\x0ACB',L'\x0ACD') , bereich(L'\x0B01',L'\x0B03') , L'\x0B3C' , bereich(L'\x0B3E',L'\x0B43') , bereich(L'\x0B47',L'\x0B48') , bereich(L'\x0B4B',L'\x0B4D') , bereich(L'\x0B56',L'\x0B57') , bereich(L'\x0B82',L'\x0B83') , bereich(L'\x0BBE',L'\x0BC2') , bereich(L'\x0BC6',L'\x0BC8') , bereich(L'\x0BCA',L'\x0BCD') , L'\x0BD7' , bereich(L'\x0C01',L'\x0C03') , bereich(L'\x0C3E',L'\x0C44') , bereich(L'\x0C46',L'\x0C48') , bereich(L'\x0C4A',L'\x0C4D') , bereich(L'\x0C55',L'\x0C56') , bereich(L'\x0C82',L'\x0C83') , bereich(L'\x0CBE',L'\x0CC4') , bereich(L'\x0CC6',L'\x0CC8') , bereich(L'\x0CCA',L'\x0CCD') , bereich(L'\x0CD5',L'\x0CD6') , bereich(L'\x0D02',L'\x0D03') , bereich(L'\x0D3E',L'\x0D43') , bereich(L'\x0D46',L'\x0D48') , bereich(L'\x0D4A',L'\x0D4D') , L'\x0D57' , L'\x0E31' , bereich(L'\x0E34',L'\x0E3A') , bereich(L'\x0E47',L'\x0E4E') , L'\x0EB1' , bereich(L'\x0EB4',L'\x0EB9') , bereich(L'\x0EBB',L'\x0EBC') , bereich(L'\x0EC8',L'\x0ECD') , bereich(L'\x0F18',L'\x0F19') , L'\x0F35' , L'\x0F37' , L'\x0F39' , L'\x0F3E' , L'\x0F3F' , bereich(L'\x0F71',L'\x0F84') , bereich(L'\x0F86',L'\x0F8B') , bereich(L'\x0F90',L'\x0F95') , L'\x0F97' , bereich(L'\x0F99',L'\x0FAD') , bereich(L'\x0FB1',L'\x0FB7') , L'\x0FB9' , bereich(L'\x20D0',L'\x20DC') , L'\x20E1' , bereich(L'\x302A',L'\x302F') , L'\x3099' , L'\x309A' 
					); 
	}

	template<typename char_t> bool _is_extender( char_t ch ) = delete;
	bool _is_extender( wchar_t ch )
	{
		return is_in( ch
					  , L'\x00B7' , L'\x02D0' , L'\x02D1' , L'\x0387' , L'\x0640' , L'\x0E46' , L'\x0EC6' , L'\x3005' , bereich(L'\x3031',L'\x3035') , bereich(L'\x309D',L'\x309E') , bereich(L'\x30FC',L'\x30FE')
		); 
	}

	template<typename char_t> bool _is_digit( char_t ch ) = delete;
	bool _is_digit( wchar_t ch )
	{
		return is_in( ch
					, bereich(L'\x0030',L'\x0039') 
					, bereich(L'\x0660',L'\x0669') 
					, bereich(L'\x06F0',L'\x06F9') 
					, bereich(L'\x0966',L'\x096F') 
					, bereich(L'\x09E6',L'\x09EF') 
					, bereich(L'\x0A66',L'\x0A6F') 
					, bereich(L'\x0AE6',L'\x0AEF') 
					, bereich(L'\x0B66',L'\x0B6F') 
					, bereich(L'\x0BE7',L'\x0BEF') 
					, bereich(L'\x0C66',L'\x0C6F') 
					, bereich(L'\x0CE6',L'\x0CEF') 
					, bereich(L'\x0D66',L'\x0D6F') 
					, bereich(L'\x0E50',L'\x0E59') 
					, bereich(L'\x0ED0',L'\x0ED9') 
					, bereich(L'\x0F21',L'\x0F29') );
	}

	template<typename char_t> bool _is_letter( char_t ch ) 
	{
		return _is_basechar(ch) | _is_ideographic(ch);
	}

	template<typename char_t> bool _is_name_firstchar(char_t ch)
	{
		return _is_letter(ch) | is_in( ch, char_t('_'),char_t(':')) ;
	}
	template<typename char_t> bool _is_name_char(char_t ch)
	{
		return _is_name_firstchar(ch) | _is_digit(ch) | is_in( ch, char_t('.'),char_t('-')) | _is_combiningchar(ch) | _is_extender(ch);
	}
}}

namespace WS { namespace XML
{
	template<typename iterator_t> struct eated_data
	{
		_iterator_access<iterator_t> begin_tag;
		_iterator_access<iterator_t> content;
		_iterator_access<iterator_t> end_tag;
	};
	template<typename iterator_t> struct complex_eated : eated_data<iterator_t>
	{
		operator bool() const {return this->end_tag.empty()==false;}
		bool operator !() const {return !operator bool();}
		bool operator==( bool value ) const {return operator bool() == value;}
		bool error() const { return *this==false && this->begin_tag.empty()==false; }
	};
	template<typename iterator_t> complex_eated<iterator_t> eat_complex( _iterator_access<iterator_t> & container_in, _iterator_access<iterator_t> const & begin_tag, _iterator_access<iterator_t> const & end_tag )
	{
		complex_eated<iterator_t> retvalue;
		using char_t = _iterator_access<iterator_t>::value_t;

		auto container = container_in;

		if( retvalue.begin_tag = eat(container,begin_tag ) )
		{
			while( container.empty()==false && (retvalue.end_tag = eat( container, end_tag )) == false )
				++container.begin();

			if( retvalue )
			{
				container_in = container;
				retvalue.content = _iterator_access<iterator_t>( retvalue.begin_tag.end(), retvalue.end_tag.begin() );
			}
			else
			{	//comment begin without end

			}
		}
		return retvalue;
	}
	template<typename iterator_t> struct comment_eated : eated_data<iterator_t>
	{
		enum class enumError{none,invalidminusminus,missing_end} errorcode = enumError::none;
		using char_t = std::remove_pointer_t<iterator_t>;
		operator bool() const {return this->end_tag == comment_end_tag<char_t>();}
		bool operator !() const {return !operator bool();}
		bool operator==( bool value ) const {return operator bool() == value;}
		bool error() const { return WS::is_in(errorcode,enumError::missing_end); }

	};
	template<typename iterator_t> comment_eated<iterator_t> eat_comment( _iterator_access<iterator_t> & container_in )
	{
		using char_t = _iterator_access<iterator_t>::value_t;
		comment_eated<iterator_t> retvalue;

		auto container = container_in;
		if( retvalue.begin_tag = eat(container, comment_begin_tag<char_t>()) )
		{
			while(container.empty()==false)
			{
				if( (retvalue.end_tag = eat( container, minus_minus<char_t>() )) == false )
					++container.begin();
				else if( eat( container, _close<char_t>() ) )
				{
					++retvalue.end_tag.end();
					break;
				}
				else 
					retvalue.errorcode = decltype(retvalue)::enumError::invalidminusminus;
			}

			if( retvalue && retvalue.error()==false )
			{
				container_in = container;
				retvalue.content = _iterator_access<iterator_t>( retvalue.begin_tag.end(), retvalue.end_tag.begin() );
			}
			else
			{	//comment begin without end
				retvalue.errorcode = decltype(retvalue)::enumError::missing_end;
			}
		}
		return retvalue;
	}
	template<typename iterator_t> comment_eated<iterator_t> eat_comment( _iterator_access<iterator_t> && container_in )//zu testzwecken
	{
		return eat_comment( container_in );
	}
	template<typename iterator_t> complex_eated<iterator_t> eat_prolog( _iterator_access<iterator_t> & container_in )
	{
		using char_t = _iterator_access<iterator_t>::value_t;
		return eat_complex( container_in, prolog_begin_tag<char_t>(), prolog_end_tag<char_t>() );
	}

	template<typename iterator_t> struct entityReferenz_eated
	{
		using char_t = typename _iterator_access<iterator_t>::value_t;
		char_t							value = 0;
		_iterator_access<iterator_t>	eated;

		operator bool(){return value!=0;}
		bool operator !(){return value==0;}
	};
	template<typename iterator_t> entityReferenz_eated<iterator_t> eat_entityReferenz( _iterator_access<iterator_t> & container_in )
	{
		using char_t = _iterator_access<iterator_t>::value_t;
		char_t return_char = 0;

		auto eat_numeric = [&](_iterator_access<iterator_t> & container_in )->bool
		{
			auto container = container_in;
			if( eat( container, _doppelkreuz<char_t>() ) )
			{
				bool eatonevalue=false;
				while( auto digit = std::isdigit(*container.begin()) )
				{
					return_char = return_char*10 + static_cast<char_t>(digit) - static_cast<char_t>('0');
					eatonevalue = true;
				}
				return eatonevalue; 
			}
			return false;
		};


		auto container = container_in;
		if( eat( container, _ampersand<char_t>() ) )
		{
			if( eat( container, _lt_ref<_iterator_access<char_t const *>>() ) )
				return_char = _lt<char_t>();
			else if( eat( container, _gt_ref<_iterator_access<char_t const *>>() ) )
				return_char = _gt<char_t>();
			else if( eat( container, _amp_ref<_iterator_access<char_t const *>>() ) )
				return_char = _amp<char_t>();
			else if( eat( container, _quot_ref<_iterator_access<char_t const *>>() ) )
				return_char = _quot<char_t>();
			else if( eat( container, _apos_ref<_iterator_access<char_t const *>>() ) )
				return_char = _apos<char_t>();
			else if( eat_numeric(container) )
			{}
			else
				return {};//fehler

			if( eat( container, _semicolon<char_t>() ) )
			{
				entityReferenz_eated<iterator_t> ret_value{ return_char };
				ret_value.eated = _iterator_access<iterator_t>( container_in.begin(), container.begin() );
				container_in = container;
				return ret_value;
			}
		}
		return {};//fehler
	}

	template<typename iterator_t> bool eat_whitespace( _iterator_access<iterator_t> & container_in )
	{
		using char_t = _iterator_access<iterator_t>::value_t;
		return eat_oneof( container_in, whitespace<char_t>() );
	}
	template<typename iterator_t> bool eat_whitespace( _iterator_access<iterator_t> && container_in )//zu testzwecken
	{
		return eat_whitespace( container_in );
	}
	template<typename iterator_t> _iterator_access<iterator_t>  eat_whitespaces( _iterator_access<iterator_t> & container_in )
	{
		using char_t = _iterator_access<iterator_t>::value_t;
		_iterator_access<iterator_t> retvalue;
		while( auto erg=eat_oneof( container_in, whitespace<char_t>() ) )
			retvalue.append(erg);
		return retvalue;
	}

	template<typename iterator_t> bool _eat_basechar( _iterator_access<iterator_t> & container_in )
	{
	}
	//tag-name
	template<typename iterator_t> _iterator_access<iterator_t> eat_name( _iterator_access<iterator_t> & container_in )
	{
		using char_t = _iterator_access<iterator_t>::value_t;
		auto retvalue = _iterator_access<iterator_t>(container_in.begin(),container_in.begin());

		//im Mom nur mit wchar_t
		if( eat_if(container_in,_is_name_firstchar<char_t> ))
		{
			retvalue.end() = container_in.begin();
			retvalue.append( eat_while(container_in, _is_name_char<char_t>) );
		}
		return retvalue;
	}
	template<typename iterator_t> _iterator_access<iterator_t> eat_name( _iterator_access<iterator_t> && container_in )//zu testzwecken
	{
		return eat_name( container_in );
	}
	//attributname
	template<typename iterator_t> _iterator_access<iterator_t> eat_token( _iterator_access<iterator_t> & container_in )
	{
		using char_t = _iterator_access<iterator_t>::value_t;
		auto retvalue = _iterator_access<iterator_t>(container_in.begin(),container_in.begin());
		retvalue.end() = container_in.begin();

		//im Mom nur mit wchar_t
		retvalue.append( eat_while(container_in, _is_name_char<char_t>) );
		return retvalue;
	}
	//attributvalue
	template<typename iterator_access_t > struct attributvalue_eated
	{
		enum enumError{none,opener_missing,closer_missing,invalid_ampersand};
		using value_t = _iterator_access<typename iterator_access_t::value_t const *>;
		value_t				value;//wegen möglichem umkopieren anderer datentyp

		iterator_access_t	first_close_in_value;//ungewöhnlich aber gültig erstes > im text
		iterator_access_t	error_position;
		enumError			error = enumError::none;
		operator bool() const { return this->error==enumError::none; }
		bool operator !() const { return !operator bool(); }

		operator value_t() const {return value;}
	};
	template<typename iterator_access_t > attributvalue_eated<iterator_access_t> eat_attributvalue( iterator_access_t & container_in )
	{
		using char_t = iterator_access_t::value_t;
		using ret_t = attributvalue_eated<iterator_access_t>;
		auto container = container_in;
		ret_t retvalue;
		retvalue.error = attributvalue_eated<iterator_access_t>::enumError::opener_missing;
		if( auto opening= eat_oneof(container,_doppelteshochkomma<char_t>(), _hochkomma<char_t>() ) )
		{
			WS::appender<iterator_access_t> value;
			retvalue.error = attributvalue_eated<iterator_access_t>::enumError::closer_missing;
			//lesen, bis der opener, '@' als opener einer referenz oder '>' als vermutete fehlerposition,  
			while( auto erg = eat_while(container, [&](char_t ch){return !WS::is_in(ch,*opening.begin(),_open<char_t>(),_ampersand<char_t>(),_close<char_t>()); }) )
			{
				value.append( erg );
				if( eat_oneof(container,opening) )
				{
					container_in=container;
					retvalue.value = value.move();
					retvalue.error = attributvalue_eated<iterator_access_t>::enumError::none;
					return retvalue;
				}
				if( auto erg2 = eat_oneof(container,_close<char_t>()) )
				{	//kein fehler, aber unerwartet
					if(retvalue.first_close_in_value.empty())
						retvalue.first_close_in_value=erg2;
					value.append( erg2 );
				}
				else if( auto referenz = eat_entityReferenz( container ) )
				{
					value.append( referenz.value );
				}
				else
				{
					retvalue.error_position = container;
					retvalue.error = attributvalue_eated<iterator_access_t>::enumError::invalid_ampersand;
					break;//fehler
				}
			}
		}
		return retvalue;
	}
	template<typename iterator_access_t > attributvalue_eated<iterator_access_t> eat_attributvalue( iterator_access_t && container_in )//zu testzwecken
	{
		return eat_attributvalue( container_in );
	}
	template<typename iterator_t> struct attribut
	{
		_iterator_access<iterator_t> name;
		_iterator_access<iterator_t> value;
	};
	template<typename iterator_t> struct attribut_eated : attribut<iterator_t>
	{
		enum class enumError{none,name_missing,assign_missing,value_missing,value_parseerror} errorcode = enumError::none;

		operator bool() const { return errorcode == enumError::none; }//das attibut wurde geladen, oder false es war nichts da
		bool operator !() const { return !operator bool(); }
		bool error() const { return !WS::is_in(this->errorcode, enumError::none, enumError::name_missing); }//attribut konnte nur teilweise geparsed werden
	};
	template<typename iterator_t> attribut_eated<iterator_t> eat_attribut( _iterator_access<iterator_t> & container_in )
	{
		using char_t = _iterator_access<iterator_t>::value_t;
		attribut_eated<iterator_t> retvalue;

		auto container = container_in;
		retvalue.errorcode = attribut_eated<iterator_t>::enumError::name_missing;
		if( retvalue.name = eat_token(container) )
		{
			retvalue.errorcode = attribut_eated<iterator_t>::enumError::assign_missing;
			eat_whitespace(container);
			if( eat( container, _assign<char_t>() ) )
			{
				retvalue.errorcode = attribut_eated<iterator_t>::enumError::value_missing;
				eat_whitespace(container);
				if( auto attribut_value = eat_attributvalue(container) )
				{
					retvalue.errorcode = attribut_eated<iterator_t>::enumError::none;
					retvalue.value = attribut_value;
				}
				else
				{	//fehler beim parsen
					switch(attribut_value.error)
					{
					case decltype(attribut_value)::enumError::opener_missing:
						break;
					default:
						retvalue.errorcode = attribut_eated<iterator_t>::enumError::value_parseerror;
					}
				}
			}
		}
		return retvalue;
	}
}}
