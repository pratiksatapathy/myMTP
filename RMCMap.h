#ifndef RMCMAP_H
#define RMCMAP_H

#include <stdio.h>
#include <chrono>
#include <iostream>
#include <cstring>
#include <string>
#include <sstream>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/array.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/hash_map.hpp>
#include <boost/serialization/hash_set.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/slist.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/bitset.hpp>
#include <boost/serialization/string.hpp>
#include <boost/shared_ptr.hpp>

#include "ramcloud/RamCloud.h"

using namespace std;
using namespace RAMCloud;
using namespace std::chrono;

/*
//Generic function to convert any data type (classes must implement boot serialization) to string.
template <typename T>
string toString(T const &obj);
*/


template<typename T>
string toString(T const &obj){
	stringstream ofs;
	boost::archive::text_oarchive oa(ofs);
	oa << obj;
	return ofs.str();
}



template<typename ValType>
class RMCData{
  public:
  ValType *value;
  bool valid;
};

template<typename KeyType, typename ValType>
class RMCMap {
public:
RamCloud *cluster=NULL;
uint64_t table;
string t_name="test";

//RMCMap();
RMCMap(char *conf);
RMCMap(char *conf,char *tablename);
~RMCMap();
string currentTimeMillis();
RMCData<ValType> get(KeyType const& key);
void put(KeyType const& key,ValType const& val);
void remove(KeyType const& key);
void clear();
};



template<typename KeyType, typename ValType>
void RMCMap<KeyType, ValType>::remove(KeyType const& key) {
  string ks=toString(key);
  cluster->remove(table, ks.c_str(), ks.size());
}



template<typename KeyType, typename ValType>
RMCMap<KeyType, ValType>::RMCMap(char *conf){
    cluster = new RamCloud(conf,"__unnamed__");
    t_name="test"+currentTimeMillis();
    table = cluster->createTable(t_name.c_str());
    //printf("Table created : %s  TID:%lu\n",t_name.c_str(),table);
}

template<typename KeyType, typename ValType>
RMCMap<KeyType, ValType>::RMCMap(char *conf,char *tablename){
    cluster = new RamCloud(conf,"__unnamed__");
    t_name=*(new string(tablename));
    table = cluster->createTable(tablename);
    //printf("Table created : %s  TID:%lu\n",t_name.c_str(),table);
}


template<typename KeyType, typename ValType>
RMCMap<KeyType, ValType>::~RMCMap(){
    cluster->dropTable(t_name.c_str());
    printf("Table Droped : %s\n",t_name.c_str());
}

template<typename KeyType, typename ValType>
void RMCMap<KeyType, ValType>::clear(){
try {
    /*cluster->dropTable(t_name.c_str());
    t_name="test"+currentTimeMillis();
    table = cluster->createTable(t_name.c_str());
*/
} catch(Exception e) {}
}


template<typename KeyType, typename ValType>
string RMCMap<KeyType, ValType>::currentTimeMillis(){
   auto epoch = high_resolution_clock::from_time_t(0);
   auto now = high_resolution_clock::now();
   long l = (long)duration_cast<milliseconds>(now-epoch).count();
   return to_string(l);
}

template<typename KeyType, typename ValType>
RMCData<ValType> RMCMap<KeyType, ValType>::get(KeyType const& key) {
   RMCData<ValType> rd;
try {
  ValType *val = new(ValType);

  Buffer buffer;
  string ks=toString(key);
  cluster->read(table, ks.c_str(), ks.size(), &buffer);
  char *str=new char[buffer.size()+1];
  buffer.copy(0,buffer.size(),str);
  string s(str);
  stringstream ifs;
  ifs<<s;
  boost::archive::text_iarchive ia(ifs);
  ia >> (*val);
  rd.value = val;
  rd.valid = true;
 } catch (ClientException e) {
   rd.value = NULL;
   rd.valid = false;
 }
  return rd;
}

template<typename KeyType, typename ValType>
void RMCMap<KeyType, ValType>::put(KeyType const& key,ValType const& val) {
  string k=toString(key);
  string v=toString(val);
  cluster->write(table, k.c_str(), (unsigned int)k.size(), v.c_str(), (unsigned int)(v.size()+1));
}


#endif
