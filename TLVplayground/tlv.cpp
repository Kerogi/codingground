#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <assert.h>  
#include <stdexcept>  
#include <limits>
#include <iomanip>


namespace tlv {
using byte = unsigned char;
using int16 = short;
using int32 = int;
//using size_t = int;
const size_t TLV_TAG_SIZE = 1;
const size_t TLV_LENGTH_SIZE = 2;
const size_t TLV_LENGTH_OFF = TLV_TAG_SIZE;
const size_t TLV_VALUES_OFF = TLV_TAG_SIZE+TLV_LENGTH_SIZE;
const size_t TLV_MIN_SIZE = TLV_TAG_SIZE+TLV_LENGTH_SIZE;


struct buffer
{
    buffer(const byte* data, const size_t size)
        : size_(size)
        , data_(data)
    {}
    
    const size_t size_;
    const byte*  data_;
	size_t      size() const {return size_;}
	const byte* data() const {return data_;}
};

class iterator;

class cursor
{
	const byte* ptr;
	cursor(const buffer& buf, size_t offset)
		: ptr(buf.data() + offset)
		{
			assert(buf.size()-TLV_MIN_SIZE >= offset);
		}
public:
	friend class iterator;

	int tag() const
	{
		return (int)(*ptr);	
	}

	size_t length() const
	{
        const byte* p = ptr+TLV_LENGTH_OFF;
        const short size1 = *(p++);
        const short size2 = *(p);
            
		return  size2<<8 | size1;	
	}

	const byte* value() const
	{
        if(length()>0){
		    return ptr+TLV_VALUES_OFF;	
        } else {
            return nullptr;
        }
	}
};
class iterator
{
	static buffer nullbuff;
	const buffer& buf;
	size_t offset;
	bool valid = true;	
public:
    iterator()
    	: buf(nullbuff)
    	, offset(0)
    	, valid (false)
    {}

    iterator(const buffer& buf, size_t offset)
    	: buf(buf)
    	, offset(offset)
    {

    }
    bool operator!=(const iterator& other) const
    {
        bool result = (valid && other.valid)? offset != other.offset : valid != other.valid;
    	return result;
    }
    cursor operator*() const
    {
    	if(!valid) throw std::runtime_error("dereferencing invalid iterator");
    	return cursor(buf, offset);
    }
    iterator& operator++()
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

};

buffer iterator::nullbuff(nullptr, 0);

class chain
{
	const buffer buf;
	mutable std::vector<cursor> cache;
	mutable bool cache_valid = false;
public:
    chain(const cursor &cur)
        :buf(cur.value(), cur.length())
	{
        assert(nullptr != cur.value() && cur.length()>=3);
    }
	chain(const byte* buffer, size_t size)
        :buf(buffer, size)
	{}

	iterator begin() const
	{
		return iterator(buf, 0);
	}

	iterator end() const
	{
		return iterator();
	}
	
	size_t size() const
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

	cursor operator[](size_t index) const
	{
		if(index >= size()) {
			std::string msg(std::to_string(index));
			msg += " index too big for size ";
			msg += std::to_string(size());
			throw std::out_of_range(msg);
		}
		return cache[index];
	}

};

}