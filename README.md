![CI](https://github.com/H-Shen/Caches/workflows/Caches%20CI/badge.svg)

#### The toy implementations of caches in C++

The project includes the implementations of four cache replacement policies using single thread:

*   First in last out (FILO)
*   First in first out (FIFO)
*   Least recently used (LRU)
*   Least frequently used (LFU)

#### Usage

The implementations are easy to use and are included within a single namespace in a single header file *CacheImpl.hpp*. The user can provide the type of data to store in the cache since the implementation is generic, also the user is able to provide custom hash function for inner hash maps in the cache for better efficiency. The project used [Catch2](https://github.com/catchorg/Catch2) for unit testing. Any requests about any issues or updates are welcome.

Example:

```cpp
#include "CacheImpl.hpp"
#include <chrono>
#include <iostream>
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
```

#### Requirements

A C++ compiler that supports C++11 standard.

The project has been tested with:

*   clang++ 11.0.3
*   g++ 8.3.1
*   cmake 3.14.4

#### References
*   [Cache replacement policies](https://en.wikipedia.org/wiki/Cache_replacement_policies)
*   [LFU Cache](https://leetcode.com/problems/lfu-cache/)
*   [LRU Cache](https://leetcode.com/problems/lru-cache/)
*   [Leetcode 146](https://leetcode.com/problems/lru-cache/)
*   [Leetcode 460](https://leetcode.com/problems/lfu-cache/)
*   [Blowing up unordered_map, and how to stop getting hacked on it](https://codeforces.com/blog/entry/62393)
