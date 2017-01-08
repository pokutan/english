#pragma once

#include <random>

#ifndef CLASS_NO_COPY_NO_MOVE
#define CLASS_NO_COPY_NO_MOVE(__class_name__)                   \
    __class_name__(const __class_name__&) = delete;             \
    __class_name__& operator =(const __class_name__&) = delete; \
    __class_name__& operator =(__class_name__&&) = delete;      \
    __class_name__(__class_name__&&) = delete;
#endif // CLASS_NO_COPY_NO_MOVE

template<typename t> class gen_random final {
public:
    gen_random() : _mt(_rd()), _dist(nullptr){
        static_assert(std::is_integral<t>::value, "Integral type required");
        static_assert(std::is_arithmetic<t>::value, "Arithmetic type required");
    }
    gen_random(t min_/*inclusive*/, t max_/*not inclusive*/) : gen_random(){ set_range(min_, max_); }
    void set_range(t min_/*inclusive*/, t max_/*not inclusive*/){
        if(_dist){
            delete _dist;
            _dist = nullptr;
        }
        if(max_ > min_)
            _dist = new std::uniform_real_distribution<double>(min_, max_);
    }
    ~gen_random(){ if(_dist)delete _dist; }
    operator t(){ return _dist ? (t)(*_dist)(_mt) : 0; }
    t rand(){ return (t)*this; }
    // copy / move is prohibited
    CLASS_NO_COPY_NO_MOVE(gen_random)
private:
    std::random_device _rd;
    std::mt19937 _mt;
    std::uniform_real_distribution<double>* _dist;
};
