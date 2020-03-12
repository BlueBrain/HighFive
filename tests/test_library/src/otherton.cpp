#include <stdexcept>

#include "simpleton.hpp"

void noop(const HighFive::Object& obj) {
    if (!obj.isValid()) {
        throw std::exception();
    }
}
