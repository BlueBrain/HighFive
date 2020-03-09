#include <stdexcept>

#include "simpleton.hpp"

void function(const HighFive::Object& obj) {
    if (!obj.isValid()) {
        throw std::exception();
    }
}
