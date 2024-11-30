#include <vector>
#include <map>
#include "key_value_pair.h"

class MemTable{

    //TODO: Replace it with concurrent skip list
    std::mutex lock;
    std::map<Key, std::shared_ptr<Value>> table;


public:
    MemTable();
    ~MemTable();

    void insert(Key key, std::shared_ptr<Value> value);
    std::shared_ptr<Value> get(Key key);
    void remove(Key key);

    uint32_t size()  const;
    uint32_t lowKey() const;
    uint32_t highKey() const;

    //TODO: Implement iterator
    std::vector<std::shared_ptr<Key>> keys() const;
};