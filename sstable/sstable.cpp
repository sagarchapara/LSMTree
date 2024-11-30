#include <fstream>
#include <iostream>

#include "sstable.h"

SSTable::SSTable(std::string dirpath, int level, int index)
{
    this->filepath = get_file_path(dirpath, level, index);
    this->header = std::make_unique<SSTableHeader>();

    //if file exists, load header, else create it
    load_header(get_index_file_path(), *this->header);

    this->bloom_filter = std::make_unique<BloomFilter>(this->header->size, 5); //TODO: Implement hash count and add keys

    //load bloom filter
    this->bloom_filter->deserialize(get_bloom_filter_file_path());
}

SSTable::~SSTable()
{
}

bool SSTable::get(const Key& key, std::shared_ptr<Value> value)
{
    if (!this->bloom_filter->contains(std::string(key.data, key.size))) {
        return false;
    }

    //now do binary search in index file
    std::ifstream index_file(get_index_file_path(), std::ios::binary);

    if (!index_file) {
        std::cerr << "Error opening index file" << std::endl;
        return false;
    }

    int offset = sizeof(SSTableHeader);

    SSTableIndex keyIndex;

    while (true) {
        index_file.seekg(offset);
        index_file.read(reinterpret_cast<char*>(&keyIndex), sizeof(SSTableIndex));

        uint32_t key_size = keyIndex.size;
        char* key_data = new char[key_size];

        index_file.read(key_data, key_size);

        Key index_key = {key_size, key_data};

        if (key < index_key) {
            if (keyIndex.left == 0) {
                return false;
            }

            offset = keyIndex.left;
        } else if (index_key < key) {
            if (keyIndex.right == 0) {
                return false;
            }

            offset = keyIndex.right;
        } else {
            break;
        }
    }

    //read value from data file
    std::ifstream data_file(get_data_file_path(), std::ios::binary);

    if (!data_file) {
        std::cerr << "Error opening data file" << std::endl;
        return false;
    }

    data_file.seekg(keyIndex.value_offset);

    uint32_t value_size;
    data_file.read(reinterpret_cast<char*>(&value_size), sizeof(value_size));

    char* value_data = new char[value_size];

    data_file.read(value_data, value_size);

    value->size = value_size;
    value->data = value_data;

    return true;
}

void SSTable::write(const MemTable& memtable)
{
    auto keys = memtable.keys();

    //add the keys to bloom filter
    for (auto key : keys) {
        this->bloom_filter->add(std::string(key->data, key->size));
    }

    //index file and data file pointers
    std::ofstream index_file(get_index_file_path(), std::ios::binary);

    if (!index_file) {
        std::cerr << "Error opening index file" << std::endl;
        return;
    }

    std::ofstream data_file(get_data_file_path(), std::ios::binary);

    if (!data_file) {
        std::cerr << "Error opening data file" << std::endl;
        return;
    }

    std::uint32_t index_offset = 0, data_offset = 0;

    this->header->key_count = keys.size();
    this->header->size = memtable.size(); //TODO: Implement size

    //write header
    write_header(get_index_file_path(), *this->header);

    index_offset += sizeof(SSTableHeader);

    std::vector<std::uint32_t> data_offsets = write_data(data_file, data_offset, memtable, keys);

    //write index
    write_index(index_file, index_offset, memtable, keys, data_offsets, 0, keys.size() - 1);

    //wite bloom filter
    this->bloom_filter->serialize(get_bloom_filter_file_path());
}


/**
 * Loads the header else creates the header
 */
bool load_header(const std::string& file_path, SSTableHeader& header) {
    std::ifstream file(file_path, std::ios::binary);
    if (file) {
        file.read(reinterpret_cast<char*>(&header), sizeof(SSTableHeader));
        if (!file) {
            std::cerr << "Error reading header from file" << std::endl;
            throw std::runtime_error("Error reading header from file");
        }

        return true;
    }
}

void write_header(const std::string& file_path, const SSTableHeader& header) {
    std::ofstream file(file_path, std::ios::binary);

    if (file) {
        file.write(reinterpret_cast<const char*>(&header), sizeof(SSTableHeader));
        if (!file) {
            std::cerr << "Error writing header to file" << std::endl;
            throw std::runtime_error("Error writing header to file");
        }
    } else {
        std::cerr << "Error opening file" << std::endl;
        throw std::runtime_error("Error opening file");
    }
}

void write_index(
    std::ofstream& index_file,
    std::uint32_t& index_offset,
    const MemTable& memtable,
    const std::vector<std::shared_ptr<Key>>& keys,
    const std::vector<std::uint32_t>& data_offsets,
    int start,
    int end) {

    if (start > end) {
        return;
    }

    if(start == end) {
        //write index
        std::unique_ptr<SSTableIndex> keyIndex = std::make_unique<SSTableIndex>();

        keyIndex->value_offset = data_offsets[start];
        keyIndex->is_leaf = true;
        keyIndex->size = keys[start]->size;

        write_index(index_file, index_offset, std::move(keyIndex), keys[start]);

        //write data
        return;
    }

    int mid = start + (end - start) / 2;

    //write left and right child
    write_index(index_file, index_offset, memtable, keys, data_offsets, start, mid-1);

    write_index(index_file, index_offset, memtable, keys, data_offsets, mid + 1, end);

    std::unique_ptr<SSTableIndex> keyIndex = std::make_unique<SSTableIndex>();

    keyIndex->value_offset = data_offsets[mid];
    keyIndex->is_leaf = false;
    keyIndex->size = keys[mid]->size;

    if (mid > 0) {
        keyIndex->left = data_offsets[mid - 1];
    }

    if (mid < keys.size() - 1) {
        keyIndex->right = data_offsets[mid + 1];
    }

    write_index(index_file, index_offset, std::move(keyIndex), keys[mid]);
}

std::vector<uint32_t> write_data(std::ofstream& data_file, std::uint32_t& data_offset, const MemTable& memtable, const std::vector<std::shared_ptr<Key>>& keys) {
    //write value to data file at data_offset
    if (!data_file) {
        throw std::runtime_error("Error opening data file");
    }

    std::vector<std::uint32_t> offsets;

    for (auto key : keys) {
        std::shared_ptr<Value> value = memtable.get(*key);

        //write value to data file at data_offset
        data_file.seekp(data_offset);

        //write size
        data_file.write(reinterpret_cast<const char*>(&value->size), sizeof(value->size));
        data_offset += sizeof(value->size);

        //write data
        data_file.write(reinterpret_cast<const char*>(&value->data), sizeof(value->data));
        data_offset += sizeof(value->data);

        offsets.push_back(data_offset);
    }

    return offsets;
}

void write_index(std::ofstream& index_file, std::uint32_t& index_offset, std::unique_ptr<SSTableIndex>&& keyIndex, std::shared_ptr<Key> key) {
    //write index to index file at index_offset
    if (!index_file) {
        throw std::runtime_error("Error opening index file");
    }

    //write key header to index file at index_offset
    index_file.seekp(index_offset);
    index_file.write(reinterpret_cast<const char*>(&keyIndex), sizeof(SSTableIndex));
    index_offset += sizeof(SSTableIndex);

    //write key to index file at index_offset
    index_file.write(reinterpret_cast<const char*>(key->data), key->size);
    index_offset += sizeof(key->size);
}
