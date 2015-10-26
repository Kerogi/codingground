
#pragma once 
#ifndef __TLV__
#define __TLV__

#include <vector>
#include <cstddef>

namespace tlv 
{

using byte = unsigned char;
using int16 = short;
using int32 = int;

const size_t TLV_TAG_SIZE = 1;
const size_t TLV_LENGTH_SIZE = 2;
const size_t TLV_LENGTH_OFF = TLV_TAG_SIZE;
const size_t TLV_VALUES_OFF = TLV_TAG_SIZE+TLV_LENGTH_SIZE;
const size_t TLV_MIN_SIZE = TLV_TAG_SIZE+TLV_LENGTH_SIZE;


struct buffer
{
    buffer(const byte* data, const size_t size);    
    const size_t size_;
    const byte*  data_;
	size_t      size() const;
	const byte* data() const;
};

class iterator;

class cursor
{
	const byte* ptr;
	cursor(const buffer& buf, ptrdiff_t offset);
public:
	friend class iterator;

	int tag() const;

	size_t length() const;

	const byte* value() const;
};
class iterator
{
	static buffer nullbuff;
	const buffer& buf;
	ptrdiff_t offset;
	bool valid = true;	
public:
    iterator();
	operator bool() const {return valid;}
    iterator(const buffer& buf, ptrdiff_t offset);
    bool operator!=(const iterator& other) const;
    cursor operator*() const;
    iterator& operator++();
    iterator operator++(int);
};

class chain
{
	const buffer buf;
	mutable std::vector<cursor> cache;
	mutable bool cache_valid = false;
public:
    chain(const cursor &cur);
	chain(const byte* buffer, size_t size);

	iterator begin() const;
	iterator end() const;
	size_t size() const;

	cursor operator[](size_t index) const;

};

}
#endif // __TLV__
