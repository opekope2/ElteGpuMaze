#pragma once

#define STR_(v) #v
#define STR(v) STR_(v)

#define CHECK(v, e) \
    if (!(v))       \
        throw std::runtime_error(__FILE__ ":" STR(__LINE__) ": " #e " failed");
