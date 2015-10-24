#include "tlv.h"

#include <string>
#include <sstream>
#include <assert.h>  
#include <stdexcept>  

using namespace tlv;

buffer::buffer(const byte* data, const size_t size)
: size_(size)
, data_(data)
{}
size_t      buffer::size() const {return size_;}
const byte* buffer::data() const {return data_;}

cursor::cursor(const buffer& buf, size_t offset)
    : ptr(buf.data() + offset)
{
	assert(buf.size()-TLV_MIN_SIZE >= offset);
}


int cursor::tag() const
{
	return (int)(*ptr);	
}

size_t cursor::length() const
{
    const byte* p = ptr+TLV_LENGTH_OFF;
    const short size1 = *(p++);
    const short size2 = *(p);
        
	return  size2<<8 | size1;	
}

const byte* cursor::value() const
{
    if(length()>0){
	    return ptr+TLV_VALUES_OFF;	
    } else {
        return nullptr;
    }
}

iterator::iterator()
	: buf(nullbuff)
	, offset(0)
	, valid (false)
{}

iterator::iterator(const buffer& buf, size_t offset)
	: buf(buf)
	, offset(offset)
{

}

bool iterator::operator!=(const iterator& other) const
{
    bool result = (valid && other.valid)? offset != other.offset : valid != other.valid;
	return result;
}

cursor iterator::operator*() const
{
	if(!valid) throw std::runtime_error("dereferencing invalid iterator");
	return cursor(buf, offset);
}

iterator& iterator::operator++()
{

	if(!valid) throw std::runtime_error("incrementing invalid iterator");
	if(offset == buf.size()) {
		valid = false;
	} else {
        const byte* p = buf.data()+offset+TLV_LENGTH_OFF;
        const short size1 = *(p++);
        const short size2 = *(p);
        
    	size_t size = size2<<8 | size1;
	    	size_t hop = TLV_MIN_SIZE+size;
        size_t new_offset = offset + hop;

        if(new_offset == buf.size() ) { // reached the end exacly
             valid = false; 
             return *this;
        }           
    	if(new_offset > buf.size()) { // next tlv tag is unreachble
            valid = false; 
    		std::string msg("corrupted tlv chain missed ");
    		msg += std::to_string(new_offset- buf.size());
    		msg += " bytes";
    		throw std::runtime_error(msg);
    		//return *this;
    	} 

    	if(new_offset + TLV_MIN_SIZE > buf.size()) { // after jump to next tlv we cant not read it
    		valid = false;
    		std::stringstream ss;
    		for(size_t b = new_offset; b < buf.size(); ++b)
        		ss<<"0x"<<std::hex<<std::setfill('0')<<std::setw(2)<< (int)(*(buf.data()+b))<<";";
   			ss<<" bytes left is not enought fot next tlv";
			throw std::runtime_error(ss.str());
			//return *this;
			} 

		offset = new_offset;
    }

	return *this;
}

iterator iterator::operator++(int)
{
	iterator copy = *this;
	++(*this);
	return copy;
}


buffer iterator::nullbuff(nullptr, 0);


chain::chain(const cursor &cur)
    :buf(cur.value(), cur.length())
{
    assert(nullptr != cur.value() && cur.length()>=3);
}

chain::chain(const byte* buffer, size_t size)
    :buf(buffer, size)
{

}

iterator chain::begin() const
{
	return iterator(buf, 0);
}

iterator chain::end() const
{
	return iterator();
}

size_t chain::size() const
{
	if(!cache_valid) {
		for(auto i: *this)
		{
			cache.push_back(std::move(i));
		}
		cache_valid = true;
	}
	return cache.size();
}

cursor chain::operator[](size_t index) const
{
	if(index >= size()) {
		std::string msg(std::to_string(index));
		msg += " index too big for size ";
		msg += std::to_string(size());
		throw std::out_of_range(msg);
	}
	return cache[index];
}

