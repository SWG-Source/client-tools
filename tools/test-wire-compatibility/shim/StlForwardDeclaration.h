// Test shim: the real header forward-declares VC2013 std internals; use the real containers.
#ifndef INCLUDED_StlForwardDeclaration_H
#define INCLUDED_StlForwardDeclaration_H
#include <vector>
#include <map>
#include <set>
#include <deque>
#include <list>
#include <string>
#include <unordered_map>
template<class T> struct stdvector{ typedef std::vector<T> fwd; };
template<class K,class V,class C=std::less<K> > struct stdmap{ typedef std::map<K,V,C> fwd; };
template<class T,class C=std::less<T> > struct stdset{ typedef std::set<T,C> fwd; };
template<class T> struct stddeque{ typedef std::deque<T> fwd; };
template<class T> struct stdlist{ typedef std::list<T> fwd; };
template<class K,class V,class C=std::less<K> > struct stdmultimap{ typedef std::multimap<K,V,C> fwd; };
template<class T,class C=std::less<T> > struct stdmultiset{ typedef std::multiset<T,C> fwd; };
template<class K,class V,class... R> struct stdhash_map{ typedef std::unordered_map<K,V> fwd; };
#endif
