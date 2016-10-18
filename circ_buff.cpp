#include <iostream>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

template<typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& vec)
{
    for (auto& el : vec)
    {
        os << el << ' ';
    }
    return os;
}
using uint8_t = unsigned char;

class circ_buff 
{
public:
    struct data_handle {
        size_t      pos;
        size_t      length;
        size_t      age;

        const circ_buff* parent;    
    };
    circ_buff(size_t size): data(size), pos(0), age(0)
    {

    }

    data_handle store(const uint8_t* src, size_t len) {
        size_t new_pos = pos + len;
        size_t old_pos = pos;
        size_t old_age = age;
        size_t size = data.size();
        //assert(len <= size);
        if (new_pos <= size) {
            std::memcpy(&data[0]+pos, src, len);
            if(new_pos==size) age+=1;
        } else {
            size_t first_part = size - pos; //storage_size - storage_pos
            size_t second_part = new_pos - size;
            std::memcpy(&data[0] + pos, src, first_part); //storage_size - storage_pos
            std::memcpy(&data[0], src + first_part, second_part); //storage_size - storage_pos
            age+=1;
        }
        pos = new_pos%size;
        std::string d((const char*)src);
        return {old_pos, len, old_age, this};
    }

    size_t load(const data_handle& h, uint8_t* dst) const{
        size_t size = data.size();
        if (h.parent != this) return 0; // invalid handle 
        if (   ((h.age == age)     && ((h.pos + h.length)%size <= pos))
            || ((age - h.age == 1) && (h.pos                   >= pos))
            )
            {
            if (h.pos + h.length <= size) 
            {
                std::memcpy(dst, &data[0] + h.pos, h.length);
            } else {
                size_t first_part = size - h.pos; 
                size_t second_part = h.pos + h.length - size;
                std::memcpy(dst, &data[0] + h.pos, first_part); 
                std::memcpy(dst + first_part, &data[0],  second_part); 

            }
            return h.length;
        }
        return 0; // invalid handle 
    }

private:
    std::vector<uint8_t> data;
    size_t pos = 0 ;
    size_t age = 0;
};
/*
struct storage {
    uint8_t*    data;
    size_t      size;
    size_t      pos;
    size_t      age;
};

struct data_handle {
    size_t      pos;
    size_t      length;
    size_t      age;

    const storage* stor;    
    std::string debug;
};


data_handle store(storage* stor, const uint8_t* src, size_t len) {
    size_t new_pos = stor->pos + len;
    size_t old_pos = stor->pos;
    size_t old_age = stor->age;
    if (new_pos <=  stor->size) {
        std::memcpy(stor->data+stor->pos, src, len);
        if(new_pos==stor->size) stor->age+=1;
    } else {
        size_t first_part = stor->size - stor->pos; //storage_size - storage_pos
        size_t second_part = new_pos - stor->size;
        std::memcpy(stor->data + stor->pos, src, first_part); //storage_size - storage_pos
        std::memcpy(stor->data, src + first_part, second_part); //storage_size - storage_pos
        stor->age+=1;
    }
    stor->pos = new_pos%stor->size;
    std::string d((const char*)src);
    return {old_pos, len, old_age, stor, d};
}

size_t load(const storage& stor, const data_handle& p, uint8_t* dst) {
    size_t size = stor.size;
    if (((p.age == stor.age) && ((p.pos + p.length)%size <= stor.pos))
        || ((stor.age - p.age == 1) && (p.pos >= stor.pos)))
    {
        if (p.pos + p.length <= size) 
        {
            std::memcpy(dst, stor.data+p.pos, p.length);
        } else {
            size_t first_part = size - p.pos; 
            size_t second_part = p.pos + p.length - size;
            std::memcpy(dst, stor.data + p.pos, first_part); 
            std::memcpy(dst + first_part, stor.data,  second_part); 

        }
        return p.length;
    }
    return 0;
}

std::ostream& operator<<(std::ostream& os, const storage& s)
{
    os<<"[";
    for (size_t i =0; i<s.size;++i)
    {
        os << *(s.data + i) << ' ';
    }
    os<<"] next:"<<s.pos<<" age:"<<s.age;
    return os;
}
*/
std::ostream& operator<<(std::ostream& os, const circ_buff::data_handle& p)
{
    os << "at:" << p.pos <<" len:"<<p.length<<" age:"<<p.age<<" - ";
    uint8_t data[128];
    if(p.parent->load(p, data) > 0) {
        data[p.length] = 0;
        os<<data;
    } else {
        os<<"overwriten";
    }
    return os;
}
int main()
{
    std::vector<std::string> words = {
        "1", "22", "333", "4444", "55555", "666666", "7777777","88888888","999999999",
        "aaaaaaaaaa","bbbbbbbbbbb"
    };

    constexpr size_t storage_size = 21;

    std::vector<circ_buff::data_handle> packets;
    circ_buff stor(storage_size);

    for(const auto& w: words) {
        //std::cout<<stor<<std::endl;
        packets.push_back(stor.store((const uint8_t*)w.c_str(), w.length()));
    }

    //std::cout<<stor<<std::endl;
    for (auto& el : packets)
    {
        std::cout << el << std::endl;;
    }
    //std::cout<<stor<<std::endl;

}
