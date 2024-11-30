#include <vector>

class LSMTree{
    //MemTable Manager
    //WAL Manager
    //SSTable Manager
    //Compaction Manager

public:
    LSMTree(std::string dir_path);
    ~LSMTree();

    void insert(int key, std::vector<char>& value);
    std::vector<char> get(int key);
    void remove(int key);
};