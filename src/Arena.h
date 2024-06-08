#pragma once

class ArenaAllocator{
public:
    inline explicit ArenaAllocator(size_t bytes) : _bytes(bytes){
        _buffer = static_cast<std::byte *>(malloc(_bytes));
        _offset = _buffer;
    }

    template<typename T>
    inline T* alloc(){
        void* offset = _offset;
        _offset += sizeof(T);
        return static_cast<T*>(offset);
    }

    inline ArenaAllocator(const ArenaAllocator& other) = delete;
    inline ArenaAllocator operator=(const ArenaAllocator& other) = delete;

    inline void Delete(){
        free(_buffer);
    }

private:

    size_t _bytes;
    std::byte* _buffer;
    std::byte* _offset;
};