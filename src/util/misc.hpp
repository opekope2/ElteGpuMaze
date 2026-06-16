#pragma once

#define STR_(v) #v
#define STR(v) STR_(v)

#define CHECK(v, e) \
    if (!(v))       \
        throw std::runtime_error(__FILE__ ":" STR(__LINE__) ": " #e " failed");

#define XXD_STRING(var) \
    std::string(reinterpret_cast<char *>(var), var##_len)

// Space Engineers
#define CTRL_AMOUNT(mods) ((mods & GLFW_MOD_CONTROL) ? 10 : 1)
#define SHIFT_AMOUNT(mods) ((mods & GLFW_MOD_SHIFT) ? 100 : 1)
#define AMOUNT(mods) CTRL_AMOUNT(mods) * SHIFT_AMOUNT(mods)
