#pragma once
#ifndef __TLV_SCHEME__
#define __TLV_SCHEME__

#include "tlv.h"
#include <string>

template<int Tag, class WrappedTlv>
class TaggedTlv: public WrappedTlv
{
public:
	static const int tag = Tag;
	inline bool parse(tlv::iterator& it)
	{
		auto atlv = *it;
		if(Tag != atlv.tag()) return false;
		return WrappedTlv::parse(it);
	}
};

template<class TypeTraits>
class TypedTlv: public TypeTraits
{
	inline bool parse(tlv::iterator& it)
	{
		auto atlv = *it;
		TypeTraits::from_byte_array(atlv.value(), atlv.length());
		return true;
	}
	
	inline operator typename TypeTraits::type()
	{
		return TypeTraits::cast();
	}
};

class StringTlv
{
public:
	std::string str;
	inline bool parse(tlv::iterator& it)
	{
		auto atlv = *it;
		str.assign(reinterpret_cast<const char*>(atlv.value()), atlv.length());
		return true;
	}
	
	operator std::string()
	{
		return str;
	}
};

template<class SubTlv>
class NestedTlv: public SubTlv
{
public:
	inline bool parse(tlv::iterator& it)
	{
		auto atlv = *it;
		auto sub_chain = tlv::chain(atlv);
		auto sub_it = sub_chain.begin();
		return SubTlv::parse(sub_it);
	}
};

template <int Tag, typename SubTlv>
using TaggetNestedTlv = TaggedTlv<Tag, NestedTlv <SubTlv> >;


template<class SubTlv>
class SequenceTlv
{
	std::vector<SubTlv> list;
public:

	decltype(list.cbegin()) begin() const { return list.begin(); }
	decltype(list.cend()) end() const { return list.end(); }
	size_t size() const { return list.size(); }

	const SubTlv operator[](size_t index) const { return list[index]; }
	
	inline bool parse(tlv::iterator& it)
	{
		bool cont = true;
		do {
			SubTlv tmp;
			cont = tmp.parse(it); 
			if (cont) {
				list.push_back(std::move(tmp));
				++it;
			}
		} while (cont && it);
		
		return !list.empty();
	}
};

template<class... Args>
class TuppleTlv;

template<typename Current, typename... Rest>
class TuppleTlv<Current, Rest...> : TuppleTlv<Rest...>
{
public:
        typedef TuppleTlv<Rest...>  Next_types;
        
        Next_types& 	next = static_cast<Next_types&>(*this);
        Current       	value;
		
		bool parse(tlv::iterator& it)
		{
			return value.parse(it) && Next_types::parse(++it);
		}
};

template<>
class TuppleTlv<>
{
	public:
	bool parse(tlv::iterator& it){ return true;}
};

#endif //__TLV_SCHEME__
