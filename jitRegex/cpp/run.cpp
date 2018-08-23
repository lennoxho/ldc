#include <cstdint>
#include <iostream>
#include <string>

#include "driver/tool.h"

namespace jitRegex {
    extern "C" {
        void init();
        void term();
        std::uint32_t makeJitRegex(const char* pattern);
    }
}

struct jit_regex_manager {
    using handle = std::uint32_t;

    jit_regex_manager() { 
        cppmain_simple()
        jitRegex::init(); 
    }
    ~jit_regex_manager() { jitRegex::term(); }

    jit_regex_manager(const jit_regex_manager&) = delete;
    jit_regex_manager &operator=(const jit_regex_manager&) = delete;

    handle make(const std::string &pattern) const {
        return jitRegex::makeJitRegex(pattern.c_str());
    }
};

int main() {
    jit_regex_manager jit_regex;
    std::cout << jit_regex.make("a.*") << "\n";
}