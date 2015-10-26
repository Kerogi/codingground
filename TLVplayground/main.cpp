#include "tlv.h"
#include "tlv_scheme.h"

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <limits>
#include <iomanip>
#include <assert.h>  

template<typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& vec)
{
    for (auto& el : vec)
    {
        os << el << ' ';
    }
    return os;
}

template<>
std::ostream& operator<<<tlv::byte>(std::ostream& os, const std::vector<tlv::byte>& vec)
{
    for (auto& el : vec)
    {
        os <<std::hex<<std::setfill('0')<<std::setw(2)<< (int)el << ' ';
    }
    return os;
}

template<class vec>
void push_int8(vec& v, int i)
{
    assert(i<=std::numeric_limits<tlv::byte>::max());
    tlv::byte b = (unsigned char)i;
    v.push_back(b);
}

template<class vec>
void push_int16(vec& v, int i)
{
    assert(i <= std::numeric_limits<short>::max());
    auto it = (tlv::byte*)&i;
    tlv::byte a = *it++;
    tlv::byte b = *it++;
    v.push_back(a);
    v.push_back(b);
}

template<class vec>
void push_string(vec& v, std::string& str)
{
    //assert(i<=255);
    for(auto c: str) {
        tlv::byte b = c;    
        v.push_back(b);
    }
    v.push_back(0);
}

void print(tlv::chain c, int i= 0)
{
    std::string ind(i*2,' ');
    for(auto tlv:c) {
        
        std::cout <<ind<< "{ T=" << tlv.tag() << ", L=" << tlv.length() << ", V=";
        if (tlv.tag() == 66){ // nested tlv 
            std::cout<<'\n';
            print(tlv::chain(tlv), i+1);
            std::cout<<ind<<"}"<< std::endl;
            
        } else {
            std::cout << std::addressof(*tlv.value()) << "}" << std::endl;
        }
    }
}

class CustomTlv
{
public:
	TaggedTlv<65, StringTlv> first_name;
	TaggedTlv<65, StringTlv> last_name;

	bool parse(tlv::iterator& it)
	{
		return first_name.parse(it) && last_name.parse(++it);
	}
};

using SimpleString = TaggedTlv<65, StringTlv>;
using FullName = TaggedTlv<66, NestedTlv<
	TuppleTlv<
		SimpleString, 
		SimpleString, 
		TaggetNestedTlv<66, CustomTlv > 
	> 
	>>;



int main()
{
    std::vector<std::string> words = {
        "Hello", "from", "GCC", __VERSION__, "!"
    };
    
    std::vector<tlv::byte> nested_tlv;
    std::string nested_s1 = "Kerogi";
    std::string nested_s2 = "San";
    push_int8(nested_tlv,65);
    push_int16(nested_tlv,nested_s1.length()+1);
    push_string(nested_tlv,nested_s1);
    push_int8(nested_tlv,65);
    push_int16(nested_tlv,nested_s2.length()+1);
    push_string(nested_tlv,nested_s2); 
    std::vector<tlv::byte> nested_tlv_copy = nested_tlv;
        
    push_int8(nested_tlv,66);
    push_int16(nested_tlv,nested_tlv_copy.size());
    for(auto b: nested_tlv_copy) push_int8(nested_tlv,b);
    
    int k=0;
    std::vector<tlv::byte> test_tlv;
    for(auto word: words) {
        push_int8(test_tlv,65);
        push_int16(test_tlv,word.length()+1);
        push_string(test_tlv,word);
        if(1 == k++) {
            push_int8(test_tlv,66);
            push_int16(test_tlv,nested_tlv.size());
            for(auto b: nested_tlv) push_int8(test_tlv,b);
        }
    }
    

    
    //test_tlv[34] = 0;
    //push_int8(test_tlv,255);
    //push_int8(test_tlv,65);
    std::cout << words << std::endl;
    std::stringstream ss;
    ss << "tlv array["<<test_tlv.size()<<"]: "<< std::hex << test_tlv << std::endl;
    std::cout << ss.str();
    
    print(tlv::chain(test_tlv.data(), test_tlv.size()), 1);
	auto achain = tlv::chain(test_tlv.data(), test_tlv.size());
	auto ait = achain.begin();
	SequenceTlv<SimpleString> seq;
	FullName cred;
	seq.parse(ait);
	cred.parse(ait);
	std::cout<<"Greetings: "<<std::endl;
	for(auto strt: seq) { 
		std::cout<<"'" << strt.str <<"'"<< std::endl; 
	}
	std::cout<<"From :"<< cred.value.str <<" "<< cred.next.value.str << std::endl;
	std::cout<<"	 :"<< cred.next.next.value.first_name.str <<" "<< cred.next.next.value.last_name.str << std::endl;
    
}
