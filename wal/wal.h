#include <vector>

class WriteAheadLog{
public:
    WriteAheadLog(std::string filename);
    ~WriteAheadLog();

    void write(std::vector<char>& data);
    bool read(std::vector<char>& data);
};