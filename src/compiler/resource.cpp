#include "resource.hpp"

Resource::Resource(std::string name) : name(name), lastWrittenBy(nullptr), currAccesses(std::vector<std::reference_wrapper<Expression>>()) {}