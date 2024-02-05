#include <list>
#include <unordered_map>
#include <ctime>
#include <set>
using namespace std;

class CacheEntry {
private:
    int key;
    int value;
    time_t expiration_time;
    int piority;

public:
    CacheEntry * prev;
    CacheEntry * next;

    CacheEntry(int key, int value, time_t expiration_time, int piority) {
        this->key = key;
        this->value = value;
        this-> expiration_time = expiration_time;
        this->piority = piority;
        this->prev = nullptr;
        this->next = nullptr;
    }
    int getKey() {
        return this->key;
    }
    int getValue() {
        return this->value;
    }
    void setValue(int value) {
        this->value = value;
    }
    time_t getExpirationTime() {
        return this->expiration_time;
    }
    int getPiority() {
        return this->piority;
    }
    void setKey(int key) {
        this->key = key;
    }
};

class CustomCache {
private:
    list<CacheEntry*> double_linked_list; // for lru
    set<pair<int, int>> expired_time; // for expiration time
    set<pair<int, int>> piority; // for piority order
    unordered_map<int, CacheEntry*> cache;
    int capacity;
    int size;
    CacheEntry * begin;
    CacheEntry * end;

    void moveToEnd(CacheEntry * entry) {
        // move the entry to the end of the list
        entry->prev->next = entry->next;
        entry->next->prev = entry->prev;
        entry->prev = this->end->prev;
        entry->next = this->end;
        this->end->prev->next = entry;
        this->end->prev = entry;
    }

    void removeEntry(CacheEntry * entry) {
        this->cache.erase(entry->getKey());
        this->expired_time.erase(make_pair(entry->getExpirationTime(), entry->getKey()));
        this->piority.erase(make_pair(entry->getPiority(), entry->getKey()));

        // remove the entry from the list
        entry->prev->next = entry->next;
        entry->next->prev = entry->prev;
        delete entry;
        this->size--;
    }

    void addEntry(CacheEntry * entry) {
        this->cache[entry->getKey()] = entry;
        this->expired_time.insert(make_pair(entry->getExpirationTime(), entry->getKey()));
        this->piority.insert(make_pair(entry->getPiority(), entry->getKey()));

        // add the entry to the end of the list
        entry->prev = this->end->prev;
        entry->next = this->end;
        this->end->prev->next = entry;
        this->end->prev = entry;
        this->size++;
    }

    CacheEntry * getEntryNode(int key) {
        return this->cache[key];
    }

public:
    CustomCache(int capacity) {
        this->capacity = capacity;
        this->size = 0;
        this->begin = new CacheEntry(-1, -1, 0, 0);
        this->end = new CacheEntry(-1, -1, 0, 0);
        this->begin->next = this->end;
        this->end->prev = this->begin;
    }

    void set(int key, int value, time_t expiration_time = 0, int piority = 0) {
        // entry already exists
        if (this->has(key)) {
            CacheEntry * entry = this->cache[key];
            entry->setValue(value);
            this->moveToEnd(entry);
        }else{
            // entry does not exist
            CacheEntry * new_entry = new CacheEntry(key, value, expiration_time, piority);
            this->addEntry(new_entry);

            // if the cache is full
            if (this->size > this->capacity) {
                // is there any expired entry?
                if (this->expired_time.begin()->first < time(nullptr)) {
                    int expired_key = this->expired_time.begin()->second;
                    CacheEntry * expired_entry = this->getEntryNode(expired_key);
                    this->removeEntry(expired_entry);
                }else{
                    // no expired entry
                    if (this->piority.begin()->first < (this->piority.begin()++)->first) {
                        int least_piority_key = this->piority.begin()->second;
                        CacheEntry * least_piority_entry = this->getEntryNode(least_piority_key);
                        this->removeEntry(least_piority_entry);
                    }else{
                        // least recently used
                        CacheEntry * lru_entry = this->begin->next;
                        this->removeEntry(lru_entry);
                    }
                }
            }
        }
    }

    bool has(int key) {
        return this->cache.find(key) != this->cache.end();
    }
    
    int get(int key) {
        // entry does not exist
        if (this->has(key) == false) {
            throw std::invalid_argument("Key does not exist.");
        }
        // entry exists
        CacheEntry * entry = this->cache[key];
        this->moveToEnd(entry);
        return entry->getValue();    
    }

    // size getter
    int getSize() {
        return this->size;
    }

    // destructor
    ~CustomCache() {
        for (auto it = this->cache.begin(); it != this->cache.end(); it++) {
            delete it->second;
        }
    }
};

