struct Key{
    int size;
    char* data;

    //TODO: Somehow give ability to add custom comparator
    bool operator<(const Key& other) const{
        //first compare size
        if (size == other.size){
            //them compare the first byte from start
            for (int i = 0; i < size; i++){
                if (data[i] != other.data[i]){
                    return data[i] < other.data[i];
                }
            }
        }

        return size < other.size;
    }

};

struct Value{
    int size;
    char* data;
    bool tombstone;
    std::time_t timestamp;
};