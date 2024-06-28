#pragma once

#include <cstdint>

struct Labels{
    std::uint64_t if_labels = 0;
    std::uint64_t label_labels = 0;
    std::uint64_t temp_labels = 0;
    std::uint64_t main_labels = 0;
    std::uint64_t bool_labels = 0;
};