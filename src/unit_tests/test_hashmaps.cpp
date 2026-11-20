
#include <cstdlib>
#include <stdio.h>
#include <iostream>
#include <new>
#include <utility>

#include "zorbautils/hashmap.h"
#include "zorbautils/hashset.h"
#include "util/hashmap32.h"
#include "util/hashmap.h"
#include "util/unordered_map.h"
#include "util/hash/hash.h"

namespace zorba {

namespace UnitTests {


class IntCompFunc
{
public:
  static bool equal(uint64_t k1, uint64_t k2) 
  {
    return k1 == k2;
  }

  static uint32_t hash(uint64_t key)
  {
#if 1
    return static_cast<uint32_t>(key);
#else
    char buf[9];
    buf[0] = (unsigned char)(key>>56);
    buf[1] = (unsigned char)(key>>48);
    buf[2] = (unsigned char)(key>>40);
    buf[3] = (unsigned char)(key>>32);
    buf[4] = (unsigned char)(key>>24);
    buf[5] = (unsigned char)(key>>16);
    buf[6] = (unsigned char)(key>>8 );
    buf[7] = (unsigned char)(key    );
    buf[8] = 0;
    return (uint32_t)hashfun::h64((void*)buf,8,FNV_64_INIT);
#endif
  }
};


class StrCompFunc
{
public:
  static bool equal(const std::string& k1, const std::string& k2) 
  {
    return k1 == k2;
  }

  static uint32_t hash(const std::string& key)
  { 
    return hashfun::h32(key.c_str(), FNV_32_INIT);
  }
};


int test_hashmaps(int argc, char* argv[])
{
  if (argc < 4)
    return 1;

  int test_id = atoi(argv[1]);

  double load_factor = atof(argv[2]);

  int num = atoi(argv[3]);

  uint64_t* int_buf = new uint64_t[num];

  for (int i = 0; i < num; ++i)
  {
    int_buf[i] = rand() % num;
  }

  std::string* str_buf = new std::string[num];

  for (int i = 0; i < num; ++i)
  {
    char tmp[20];
    (void)snprintf(tmp, sizeof(tmp), "%d", rand() % num); // an int fits in tmp
    str_buf[i] = tmp;

    //std::cout << str_buf[i] << std::endl;
  }


  HashMap<uint64_t, int, IntCompFunc> map1(1024, false);
  HashMap<std::string, int, StrCompFunc> map2(1024, false);

  std::unordered_map<uint64_t, int> map3(1024);
  std::unordered_map<std::string, int> map4(1024);

  hash64map<int> map5(1024, static_cast<float>(load_factor));
  hashmap<std::string, int> map6(1024, static_cast<float>(load_factor));

  map1.set_load_factor(load_factor);
  map2.set_load_factor(load_factor);

  if (test_id == 1)
  {
    zorba::time::walltime startTime;
    zorba::time::get_current_walltime(startTime);

    for (int i = 0; i < num; ++i)
    {
      uint64_t key = int_buf[i];
      int value = 1;
      (void)map1.insert(key, value);
    }
    
    zorba::time::walltime stopTime;
    time::get_current_walltime(stopTime);

    double time = time::get_walltime_elapsed(startTime, stopTime);  

    std::cout << "Load factor = " << load_factor << std::endl
              << "Num numeric insertions = " << num << std::endl
              << "HashMap entries = " << map1.size() << std::endl
              << "HashMap capacity = " << map1.capacity() << std::endl
              << "HashMap colisions = " << map1.collisions() << std::endl
              << "Time = " << time << std::endl;
  }
  else if (test_id == 2)
  {
    zorba::time::walltime startTime;
    zorba::time::get_current_walltime(startTime);

    for (int i = 0; i < num; ++i)
    {
      int value = 1;
      (void)map2.insert(str_buf[i], value);
    }

    zorba::time::walltime stopTime;
    time::get_current_walltime(stopTime);

    double time = time::get_walltime_elapsed(startTime, stopTime);  

    std::cout << "Load factor = " << load_factor << std::endl
              << "Num string insertions = " << num << std::endl
              << "HashMap entries = " << map2.size() << std::endl
              << "HashMap buckets = " << map2.bucket_count() << std::endl
              << "HashMap capacity = " << map2.capacity() << std::endl
              << "Time = " << time << std::endl;
  }
  else if (test_id == 3)
  {
    zorba::time::walltime startTime;
    zorba::time::get_current_walltime(startTime);

    for (int i = 0; i < num; ++i)
    {
      uint64_t key = int_buf[i];
      int value = 1;
      (void)map3.insert(std::pair<uint64_t, int>(key, value));
    }
    
    zorba::time::walltime stopTime;
    time::get_current_walltime(stopTime);

    double time = time::get_walltime_elapsed(startTime, stopTime);  

    std::cout << "Load factor = " << load_factor << std::endl
              << "Num numeric insertions = " << num << std::endl
              << "UnorderedMap entries = " << map3.size() << std::endl
              << "UnorderedMap buckets = " << map3.bucket_count() << std::endl
              << "Time = " << time << std::endl;
  }
  else if (test_id == 4)
  {
    zorba::time::walltime startTime;
    zorba::time::get_current_walltime(startTime);

    for (int i = 0; i < num; ++i)
    {
      int value = 1;
      (void)map4.insert(std::pair<std::string, int>(str_buf[i], value));
    }

    zorba::time::walltime stopTime;
    time::get_current_walltime(stopTime);

    double time = time::get_walltime_elapsed(startTime, stopTime);  

    std::cout << "Load factor = " << load_factor << std::endl
              << "Num string insertions = " << num << std::endl
              << "UnorderedMap entries = " << map4.size() << std::endl
              << "UnorderedMap buckets = " << map4.bucket_count() << std::endl
              << "Time = " << time << std::endl;
  }
  else if (test_id == 5)
  {
    zorba::time::walltime startTime;
    zorba::time::get_current_walltime(startTime);

    for (int i = 0; i < num; ++i)
    {
      uint64_t key = int_buf[i];
      int value = 1;
      (void)map5.put_unsync(key, value);
    }

    zorba::time::walltime stopTime;
    time::get_current_walltime(stopTime);

    double time = time::get_walltime_elapsed(startTime, stopTime);  

    std::cout << "Load factor = " << load_factor << std::endl
              << "Num numeric insertions = " << num << std::endl
              << "hashmap entries = " << map5.size() << std::endl
              << "Time = " << time << std::endl;
  }
  else if (test_id == 6)
  {
    zorba::time::walltime startTime;
    zorba::time::get_current_walltime(startTime);

    for (int i = 0; i < num; ++i)
    {
      int value = 1;
      (void)map6.put(str_buf[i], value, false);
    }

    zorba::time::walltime stopTime;
    time::get_current_walltime(stopTime);

    double time = time::get_walltime_elapsed(startTime, stopTime);  

    std::cout << "Load factor = " << load_factor << std::endl
              << "Num string insertions = " << num << std::endl
              << "hashmap entries = " << map6.size() << std::endl
              << "Time = " << time << std::endl;
  }
  else
  {
    std::cout << "Invalid test id" << std::endl;
    return 2;
  }

  delete [] int_buf;
  delete [] str_buf;

  return 0;
}


// A key whose next copy assignment fails, as a zstring copy can when it
// allocates.
struct FailingKey
{
  static bool theFailNextCopy;

  int theId;

  FailingKey() : theId(0) {}
  explicit FailingKey(int id) : theId(id) {}
  FailingKey(const FailingKey&) = default;

  FailingKey& operator=(const FailingKey& other)
  {
    if (theFailNextCopy)
    {
      theFailNextCopy = false;
      throw std::bad_alloc();
    }
    theId = other.theId;
    return *this;
  }
};

bool FailingKey::theFailNextCopy = false;


// One bucket for all keys: the second key inserted is a collision entry.
class FailingKeyCompFunc
{
public:
  static bool equal(const FailingKey& k1, const FailingKey& k2)
  {
    return k1.theId == k2.theId;
  }

  static uint32_t hash(const FailingKey&)
  {
    return 0;
  }
};


typedef HashMap<FailingKey, int, FailingKeyCompFunc> FailingKeyMap;


// Insert key with a failing key copy through the given overload; the map
// must be left as it was: no entry counted, the key insertable afterwards.
static bool insert_fails_cleanly(FailingKeyMap& map, int key, bool pairOverload)
{
  csize const sizeBefore = map.size();
  int value = key;
  FailingKey::theFailNextCopy = true;
  try
  {
    if (pairOverload)
      (void)map.insert(std::pair<const FailingKey, int>(FailingKey(key), value));
    else
      (void)map.insert(FailingKey(key), value);
    std::cerr << "insert of key " << key << " did not fail" << std::endl;
    return false;
  }
  catch (std::bad_alloc const&)
  {
  }
  FailingKey::theFailNextCopy = false;

  if (map.size() != sizeBefore)
  {
    std::cerr << "failed insert of key " << key << " left " << map.size()
              << " entries instead of " << sizeBefore << std::endl;
    return false;
  }

  if (!map.insert(FailingKey(key), value) || map.size() != sizeBefore + 1)
  {
    std::cerr << "key " << key << " not insertable after a failed insert"
              << std::endl;
    return false;
  }
  return true;
}


int test_hashmap_insert_rollback(int, char*[])
{
  bool ok = true;
  for (int pairOverload = 0; pairOverload < 2; ++pairOverload)
  {
    FailingKeyMap map(8, false);
    // An empty bucket, then a collision entry in the same bucket.
    ok = insert_fails_cleanly(map, 1, pairOverload) && ok;
    ok = insert_fails_cleanly(map, 2, pairOverload) && ok;
  }
  return ok ? 0 : 1;
}


typedef HashSet<FailingKey, FailingKeyCompFunc> FailingKeySet;

enum SetInsertOverload { SET_INSERT, SET_INSERT_OUT, SET_INSERT_CONST_OUT };


// Insert key with a failing key copy through the given overload; the set
// must be left as it was: no entry counted, the key insertable afterwards.
static bool set_insert_fails_cleanly(
    FailingKeySet& set,
    int key,
    SetInsertOverload overload)
{
  csize const sizeBefore = set.size();
  FailingKey item(key);
  FailingKey const constItem(key);
  FailingKey outItem;
  FailingKey::theFailNextCopy = true;
  try
  {
    switch (overload)
    {
    case SET_INSERT:
      (void)set.insert(item);
      break;
    case SET_INSERT_OUT:
      (void)set.insert(item, outItem);
      break;
    case SET_INSERT_CONST_OUT:
      (void)set.insert(constItem, outItem);
      break;
    }
    std::cerr << "set insert of key " << key << " did not fail" << std::endl;
    return false;
  }
  catch (std::bad_alloc const&)
  {
  }
  FailingKey::theFailNextCopy = false;

  if (set.size() != sizeBefore)
  {
    std::cerr << "failed set insert of key " << key << " left " << set.size()
              << " entries instead of " << sizeBefore << std::endl;
    return false;
  }

  if (!set.insert(item) || set.size() != sizeBefore + 1)
  {
    std::cerr << "key " << key << " not insertable in the set after a failed "
              << "insert" << std::endl;
    return false;
  }
  return true;
}


int test_hashset_insert_rollback(int, char*[])
{
  bool ok = true;
  SetInsertOverload const overloads[] =
    { SET_INSERT, SET_INSERT_OUT, SET_INSERT_CONST_OUT };
  for (SetInsertOverload overload : overloads)
  {
    FailingKeySet set(8, false);
    // An empty bucket, then a collision entry in the same bucket.
    ok = set_insert_fails_cleanly(set, 1, overload) && ok;
    ok = set_insert_fails_cleanly(set, 2, overload) && ok;
  }
  return ok ? 0 : 1;
}


}
}
