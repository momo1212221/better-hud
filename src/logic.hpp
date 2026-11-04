#pragma once
#include <string>
namespace qol {
inline std::string health(int hp){ return "Health: "+std::to_string(hp); }
inline std::string bolts(int n){ return "Bolts: "+std::to_string(n); }
}
