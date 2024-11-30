#include <vector>

class BloomFilter {
    std::vector<bool> bits;
    std::uint32_t size;
    std::uint32_t hash_count;

    std::uint32_t hash(const std::string& key, std::uint32_t i) {
        std::hash<std::string> hash_fn;
         return hash_fn(key + std::to_string(i)) % this->size;
    }


public:

    BloomFilter(std::uint32_t size, std::uint32_t hash_count) : size(size), hash_count(hash_count) {
        this->bits = std::vector<bool>(size, false);
    }

    void add(const std::string& key) {
        for (std::uint32_t i = 0; i < this->hash_count; i++) {
            this->bits[hash(key, i)] = true;
        }
    }

    bool contains(const std::string& key) {
        for (std::uint32_t i = 0; i < this->hash_count; i++) {
            if (!this->bits[hash(key, i)]) {
                return false;
            }
        }

        return true;
    }

    void serialize(const std::string& file_path);
    void deserialize(const std::string& file_path);
};