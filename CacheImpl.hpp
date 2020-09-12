#ifndef CACHES_CACHEIMPL_HPP
#define CACHES_CACHEIMPL_HPP

#include <list>
#include <stdexcept>
#include <unordered_map>

namespace CacheImpl {
template <typename K, typename V> class Cache {
private:
  std::size_t capacity;

public:
  explicit Cache(std::size_t capacity) : capacity(capacity) {}

  size_t getCapacity() const { return capacity; }

  void setCapacity(size_t newCapacity) { capacity = newCapacity; }

  virtual V get(const K &key) = 0;

  virtual void put(const K &key, const V &value) = 0;

  virtual void clear() = 0;
};

template <typename K, typename V, typename H = std::hash<K>>
class FIFOCache : public Cache<K, V> {
private:
  std::list<std::pair<K, V>> L;
  std::unordered_map<K, typename std::list<std::pair<K, V>>::iterator, H>
      keyHashmap;

public:
  explicit FIFOCache(std::size_t size) : Cache<K, V>(size) {}

  V get(const K &key) override {
    // We check if 'key' is in the hash map
    auto iter = keyHashmap.find(key);
    if (iter == keyHashmap.end()) {
      throw std::invalid_argument(
          "Key is not found!"); // throw an exception that indicates 'not found'
    }
    // Return 'value' from the pair
    return iter->second->second;
  }

  void put(const K &key, const V &value) override {
    // Corner case:
    if (Cache<K, V>::getCapacity() == 0) {
      return;
    }
    // We check if 'key' is in the hash map
    auto iter = keyHashmap.find(key);
    if (iter == keyHashmap.end()) {
      if (keyHashmap.size() == Cache<K, V>::getCapacity()) {
        // The cache is full, we need to erase the front item from 'L' and
        // update the hash map (First In First Out)
        keyHashmap.erase(L.front().first);
        L.pop_front();
      }
      L.emplace_back(std::make_pair(key, value));
      keyHashmap[key] = (--L.end());
    } else {
      iter->second->second = value;
    }
  }

  void clear() override {
    L.clear();
    keyHashmap.clear();
  }
};

template <typename K, typename V, typename H_Key = std::hash<K>,
          typename H_Freq = std::hash<int>>
class LFUCache : public Cache<K, V> {
private:
  // inner node
  struct Node {
    K key;
    V value;
    int freq;

    explicit Node(K key, V value, int freq)
        : key(key), value(value), freq(freq) {}
  };

  int minimalFreq;
  std::unordered_map<K, typename std::list<Node>::iterator, H_Key> keyHashmap;
  std::unordered_map<int, std::list<Node>, H_Freq> freqHashmap;

public:
  explicit LFUCache(std::size_t size) : Cache<K, V>(size), minimalFreq(0) {}

  V get(const K &key) override {
    auto iter = keyHashmap.find(key);
    if (iter == keyHashmap.end()) {
      throw std::invalid_argument(
          "Key is not found!"); // throw an exception that indicates 'not found'
    }
    auto iter_in_list = iter->second;
    V value = iter_in_list->value;
    int freq = iter_in_list->freq;
    // Update 'freqHashmap'
    freqHashmap[freq].erase(iter_in_list);
    if (freqHashmap[freq].empty()) {
      freqHashmap.erase(freq);
      if (minimalFreq == freq) {
        ++minimalFreq;
      }
    }
    freqHashmap[freq + 1].emplace_front(Node(key, value, freq + 1));
    // Update 'keyHashmap'
    iter->second = freqHashmap[freq + 1].begin();
    // Return 'value'
    return value;
  }

  void put(const K &key, const V &value) override {
    // Corner case:
    if (Cache<K, V>::getCapacity() == 0) {
      return;
    }
    auto iter = keyHashmap.find(key);
    if (iter == keyHashmap.end()) {
      // Delete the least frequently used item in both hashmaps
      if (keyHashmap.size() == Cache<K, V>::getCapacity()) {
        auto iter_to_lfu_item = freqHashmap[minimalFreq].back();
        keyHashmap.erase(iter_to_lfu_item.key);
        freqHashmap[minimalFreq].pop_back();
        if (freqHashmap[minimalFreq].empty()) {
          freqHashmap.erase(minimalFreq);
        }
      }
      // Update 'minimalFreq'
      minimalFreq = 1;
      // Update 'freqHashmap'
      freqHashmap[minimalFreq].emplace_front(Node(key, value, minimalFreq));
      // Update 'keyHashmap'
      keyHashmap[key] = freqHashmap[minimalFreq].begin();
    } else {
      auto iter_in_list = iter->second;
      int freq = iter_in_list->freq;
      // Update 'freqHashmap'
      freqHashmap[freq].erase(iter_in_list);
      if (freqHashmap[freq].empty()) {
        freqHashmap.erase(freq);
        if (minimalFreq == freq) {
          ++minimalFreq;
        }
      }
      freqHashmap[freq + 1].emplace_front(Node(key, value, freq + 1));
      // Update 'keyHashmap'
      iter->second = freqHashmap[freq + 1].begin();
    }
  }

  void clear() override {
    minimalFreq = 0;
    keyHashmap.clear();
    freqHashmap.clear();
  }
};

template <typename K, typename V, typename H_Key = std::hash<K>>
class LRUCache : public Cache<K, V> {
private:
  std::list<std::pair<K, V>> L;
  std::unordered_map<K, typename std::list<std::pair<K, V>>::iterator, H_Key>
      keyHashmap;

public:
  explicit LRUCache(std::size_t size) : Cache<K, V>(size) {}

  V get(const K &key) override {
    // We check if 'key' is in the hash map
    auto iter = keyHashmap.find(key);
    if (iter == keyHashmap.end()) {
      throw std::invalid_argument(
          "Key is not found!"); // throw an exception that indicates 'not found'
    }
    // Otherwise, push the (key, value) pair to the front of 'L' and update the
    // hash map
    L.emplace_front(std::make_pair(key, iter->second->second));
    L.erase(iter->second);
    iter->second = L.begin();
    // Return 'value' from the pair
    return L.begin()->second;
  }

  void put(const K &key, const V &value) override {
    // Corner case:
    if (Cache<K, V>::getCapacity() == 0) {
      return;
    }
    // We check if 'key' is in the hash map
    auto iter = keyHashmap.find(key);
    if (iter == keyHashmap.end()) {
      if (keyHashmap.size() == Cache<K, V>::getCapacity()) {
        // The cache is full, we need to erase the least recently used item from
        // 'L' and update the hash map
        keyHashmap.erase(L.back().first);
        L.pop_back();
      }
      L.emplace_front(std::make_pair(key, value));
      keyHashmap[key] = L.begin();
    } else {
      L.erase(iter->second);
      L.emplace_front(std::make_pair(key, value));
      iter->second = L.begin();
    }
  }

  void clear() override {
    L.clear();
    keyHashmap.clear();
  }
};
} // namespace CacheImpl

#endif // CACHES_CACHEIMPL_HPP
