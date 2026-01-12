#pragma once

#include <vector>
#include <cstring>
#include <cassert>

template<typename T> class rbuffer {

public:

    rbuffer(){
        pointer = offset = bsize = 0;
    }

    rbuffer(int size){
        pointer = offset = bsize = 0;
        resize(size, (T)0);
    }

    rbuffer(int size, T val){
        pointer = offset = bsize = 0;
        resize(size, val);
    }
    
    void push(T val){
        if(pointer + bsize >= (int)buffer.size()){
            std::memcpy(buffer.data(), buffer.data() + pointer, sizeof(T) * bsize);
            pointer = 0;
        }
        buffer[pointer + bsize] = val;
        pointer++;
    }

    T& operator[](int i){
        return buffer[pointer + offset + i];
    }
    
    std::vector<T> slice(int left, int right) const {
        std::vector<T> s(right-left);
        int po = pointer + offset;
        for(int i=left; i<right; i++) s[i - left] = buffer[po + i];
        return s;
    }

    void set_offset(int o){ offset = o; }
    
    void resize(int size, T val){
        if(4 * size > (int)buffer.size()) buffer.resize(4 * size);
        if(pointer + bsize >= (int)buffer.size()){
            std::memcpy(buffer.data(), buffer.data() + pointer, sizeof(T) * bsize);
            pointer = 0;
        }
        for(int i=bsize; i<size; i++) buffer[pointer + i] = val;
        bsize = size;
    }

    int size() const { return bsize; }
    int left() const { return offset; }
    int right() const { return bsize - offset; }

private:

    int bsize, pointer, offset;
    std::vector<T> buffer;

};
