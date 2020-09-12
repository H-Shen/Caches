#define UNIT_TESTING

#ifdef UNIT_TESTING
#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#endif

#include "CacheImpl.hpp"
#include <chrono>
#include <memory>

struct custom_hash {
  static uint64_t splitmix64(uint64_t x) {
    // http://xorshift.di.unimi.it/splitmix64.c
    x += 0x9e3779b97f4a7c15;
    x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9;
    x = (x ^ (x >> 27)) * 0x94d049bb133111eb;
    return x ^ (x >> 31);
  }

  std::size_t operator()(uint64_t x) const {
    static const uint64_t FIXED_RANDOM =
        std::chrono::steady_clock::now().time_since_epoch().count();
    return splitmix64(x + FIXED_RANDOM);
  }
};

#ifdef UNIT_TESTING
TEST_CASE("LFU Test 1 with integers as key") {
  constexpr std::size_t CAPACITY = 2;
  auto cache =
      std::make_shared<CacheImpl::LFUCache<int, int, custom_hash, custom_hash>>(
          CAPACITY);
  REQUIRE(cache->getCapacity() == CAPACITY);
  cache->put(1, 1);
  cache->put(2, 2);
  REQUIRE(cache->get(1) == 1);
  cache->put(3, 3);
  REQUIRE_THROWS_AS(cache->get(2), std::invalid_argument);
  REQUIRE(cache->get(3) == 3);
  cache->put(4, 4);
  REQUIRE_THROWS_AS(cache->get(1), std::invalid_argument);
  REQUIRE(cache->get(3) == 3);
  REQUIRE(cache->get(4) == 4);
}

TEST_CASE("LFU Test 2 with integers as keys and zero capacity") {
  constexpr std::size_t CAPACITY = 0;
  auto cache = CacheImpl::LFUCache<int, int>(CAPACITY);
  cache.put(0, 0);
  REQUIRE_THROWS_AS(cache.get(0), std::invalid_argument);
}

TEST_CASE("FIFO Test 1 with integers as keys") {
  constexpr std::size_t CAPACITY = 3;
  auto cache =
      std::make_shared<CacheImpl::FIFOCache<int, int, custom_hash>>(CAPACITY);
  cache->put(1, 1);
  cache->put(2, 2);
  cache->put(1, 15);
  REQUIRE(cache->get(1) == 15);
  cache->put(3, 3);
  cache->put(4, 4);
  REQUIRE(cache->get(2) == 2);
  cache->put(5, 5);
  REQUIRE_THROWS_AS(cache->get(2), std::invalid_argument);
  cache->put(5, 0);
  REQUIRE(cache->get(5) == 0);
  cache->clear();
  REQUIRE_THROWS_AS(cache->get(5), std::invalid_argument);
}

TEST_CASE("FIFO Test 2 with std::strings as keys") {
  constexpr std::size_t CAPACITY = 3;
  auto cache =
      std::make_shared<CacheImpl::FIFOCache<std::string, int>>(CAPACITY);
  cache->put("first_item", 1);
  cache->put("second_item", 2);
  cache->put("first_item", 15);
  REQUIRE(cache->get("first_item") == 15);
  cache->put("third_item", 3);
  cache->put("fourth_item", 4);
  REQUIRE(cache->get("second_item") == 2);
  cache->put("fifth_item", 5);
  REQUIRE_THROWS_AS(cache->get("second_item"), std::invalid_argument);
  cache->put("fifth_item", 0);
  REQUIRE(cache->get("fifth_item") == 0);
}

TEST_CASE("LRU Test 1 with integers as keys") {
  constexpr std::size_t CAPACITY = 2;
  auto cache = CacheImpl::LRUCache<int, int>(CAPACITY);
  cache.put(1, 1);
  cache.put(2, 2);
  REQUIRE(cache.get(1) == 1);
  cache.put(3, 3); // evicts key 2
  REQUIRE_THROWS_AS(cache.get(2), std::invalid_argument);
  cache.put(4, 4); // evicts key 1
  REQUIRE_THROWS_AS(cache.get(1), std::invalid_argument);
  REQUIRE(cache.get(3) == 3);
  REQUIRE(cache.get(4) == 4);
}
#else

#include <iostream>

// Type aliases
template <typename Key, typename Value, typename Hash>
using lfu_cache_t = CacheImpl::LRUCache<Key, Value, Hash>;

template <typename Key, typename Value>
using fifo_cache_t = CacheImpl::FIFOCache<Key, Value>;

int main(int argc, char **argv) {

  // Define the capacity of the cache
  constexpr std::size_t CACHE_CAPACITY = 10;

  // Define a LFU Cache with custom hash function of the key
  lfu_cache_t<int, int, custom_hash> lfu_cache(CACHE_CAPACITY);
  lfu_cache.put(3, 5);
  lfu_cache.put(4, -1);
  std::cout << lfu_cache.get(4) << std::endl;
  lfu_cache.put(4, 0);
  std::cout << lfu_cache.get(4) << std::endl;

  // Define a FIFO Cache with std::string as keys and values using smart
  // pointers
  auto ptr_fifo_cache =
      std::make_shared<fifo_cache_t<std::string, std::string>>(CACHE_CAPACITY);
  std::cout << ptr_fifo_cache->getCapacity() << std::endl;
  ptr_fifo_cache->put("key0", "value0");
  ptr_fifo_cache->put("key1", "value1");
  std::cout << ptr_fifo_cache->get("key0") << std::endl;
  ptr_fifo_cache->clear();
  return 0;
}
#endif