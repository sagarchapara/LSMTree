#include <vector>
#include <map>
#include <queue>

#include "key_value_pair.h"
#include "sstable.h"
#include "memtable/memtable.h"

class sstable_manager
{
private:
    std::string dir_path;
    int level_count;

    //implements using levels
    std::map<int, std::vector<std::unique_ptr<SSTable>>> levels;

    std::queue<std::shared_ptr<MemTable>> flushing_memtables;
    std::mutex flushing_memtables_lock;
public:
    sstable_manager(std::string dir_path);
    ~sstable_manager();

    void write(const MemTable& memtable);
    bool get(const Key& key, std::shared_ptr<Value> value);
    void compact();
};


