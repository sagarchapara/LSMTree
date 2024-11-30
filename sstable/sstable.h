#include <vector>
#include <math.h>

#include "memtable/memtable.h"
#include "key_value_pair.h"
#include "bloom_filter/bloom_filter.h"

struct SSTableHeader
{
    std::uint32_t size;
    std::uint32_t key_count;
};

struct SSTableIndex
{
    std::uint32_t size; //size of the key
    std::uint32_t value_offset; //offset in data file
    bool is_leaf; //is leaf node
    std::uint32_t left; //left child
    std::uint32_t right; //right child
};


//TODO: Implement using BTree
class SSTable{
    const std::string sstable_prefix = "sstable_";
    const std::string index_file_extension = ".idx";
    const std::string data_file_extension = ".dat";
    const std::string bloom_filter_file_extension = ".bloom";

    std::string filepath;

    std::unique_ptr<SSTableHeader> header;
    std::unique_ptr<BloomFilter> bloom_filter;

    std::string get_file_path(std::string dir_path, int level, int index);

    std::string get_index_file_path() {
        return this->filepath + index_file_extension;
    }

    std::string get_data_file_path() {
        return this->filepath + data_file_extension;
    }

    std::string get_bloom_filter_file_path() {
        return this->filepath + bloom_filter_file_extension;
    }

    std::string SSTable::get_file_path(std::string dir_path, int level, int index){
        return dir_path + "/" + this->sstable_prefix + std::to_string(level) + "_" + std::to_string(index);
    }

public:
    ~SSTable();

    SSTable(std::string dirname, int level, int index);

    void write(const MemTable& memtable);

    //Find the value for the given key
    bool get(const Key& key, std::shared_ptr<Value> value);
};