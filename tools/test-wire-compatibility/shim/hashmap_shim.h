// VC2013 std::hash_map spelled via unordered_map (container choice only; no serializer uses it)
#pragma once
#include <unordered_map>
namespace std { template<class K,class V,class... R> using hash_map = std::unordered_map<K,V>; }
