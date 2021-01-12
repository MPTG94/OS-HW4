//
// Created by 1912m on 12/01/2021.
//

#include <unistd.h>
#include <stdio.h>
#include <cstring>
#include <iostream>

#define MAX_BLOCK 100000000

class MallocMetadata {
    size_t size;
    bool is_free;
    void *mem_address;
    MallocMetadata *next;
    MallocMetadata *prev;

    // List head CTOR
    MallocMetadata(size_t size, void *addr) : size(size), is_free(false), mem_address(addr), next(nullptr), prev(nullptr) {}

    // List tail CTOR
    MallocMetadata(size_t size, void *addr, MallocMetadata *prev) : size(size), is_free(false), mem_address(addr), prev(prev), next(nullptr) {}

    friend class List;
};

class List {
    MallocMetadata *head = nullptr;
    MallocMetadata *tail = nullptr;
    size_t num_free_blocks = 0;
    size_t num_free_bytes = 0;
    size_t num_allocated_blocks = 0;
    size_t num_allocated_bytes = 0;
    size_t num_of_metadata = 0;

    void IncreaseCounter(size_t size) {
        num_of_metadata++;
        num_allocated_blocks++;
        num_allocated_bytes += size;
    }

    void ReplaceTail(MallocMetadata *nTail) {
        // mark prev of new tail as the old tail
        nTail->prev = tail;
        // mark new tail as tail
        tail = nTail;
        // mark the next of the previous tail as the current tail
        tail->prev->next = tail;
        // this is the tail, so there is nothing after it
        tail->next = nullptr;
    }

public:
    List() : head(nullptr), tail(nullptr), num_free_blocks(0), num_free_bytes(0), num_allocated_blocks(0), num_allocated_bytes(0),
             num_of_metadata(0) {}

    void *Insert(size_t size) {
        if (head == nullptr) {
            // insert list head
            MallocMetadata *nHead = (MallocMetadata *) sbrk(sizeof(MallocMetadata));
            if (nHead == (void *) (-1)) {
                return nullptr;
            } else {
                void *addr = sbrk(size);
                if (addr == (void *) (-1)) {
                    return nullptr;
                }
                nHead->size = size;
                nHead->is_free = false;
                nHead->mem_address = addr;
                nHead->prev = nullptr;
                nHead->next = nullptr;
                head = nHead;
                tail = nHead;
                IncreaseCounter(size);
                return addr;
            }
        } else {
            // insert at end
            MallocMetadata *nTail = (MallocMetadata *) sbrk(sizeof(MallocMetadata));
            if (nTail == (void *) (-1)) {
                return nullptr;
            } else {
                void *addr = sbrk(size);
                if (addr == (void *) (-1)) {
                    return nullptr;
                }
                nTail->size = size;
                nTail->is_free = false;
                nTail->mem_address = addr;
                ReplaceTail(nTail);
                IncreaseCounter(size);
                return addr;
            }
        }
    }

    void MarkAsFree(void *addr) {
        if (head == nullptr) {
            return;
        }
        MallocMetadata *current = head;
        while (current) {
            if (current->mem_address == addr && !current->is_free) {
                current->is_free = true;
                num_free_blocks++;
                num_free_bytes += current->size;
                return;
            }
            current = current->next;
        }
    }

    void *FindFreeBlockBySize(size_t size) {
        if (head == nullptr) {
            // the list is empty, no free blocks (technically everything is free)
            return nullptr;
        }
        MallocMetadata *current = head;
        while (current) {
            if (current->is_free && current->size >= size) {
                // Marking the current block as used by the requestor and returning the address
                current->is_free = false;
                num_free_blocks--;
                num_free_bytes -= current->size;
                return current->mem_address;
            }
            current = current->next;
        }
        // No free block was found, need to allocate a new one
        return nullptr;
    }

    size_t GetSizeOfBlockByAddress(void *addr) {
        if (head == nullptr) {
            return 0;
        }
        MallocMetadata *current = head;
        while (current) {
            if (current->mem_address == addr) {
                return current->size;
            }
            current = current->next;
        }
        return 0;
    }

    size_t get_num_free_blocks() {
        return num_free_blocks;
    }

    size_t get_num_free_bytes() {
        return num_free_bytes;
    }

    size_t get_num_allocated_blocks() {
        return num_allocated_blocks;
    }

    size_t get_num_allocated_bytes() {
        return num_allocated_bytes;
    }

    size_t get_num_metadata_bytes() {
        return num_of_metadata * get_metadata_size();
    }

    size_t get_metadata_size() {
        return sizeof(MallocMetadata);
    }
};

List *mallocList = (List *) sbrk(sizeof(List));

void *smalloc(size_t size) {
    if (size == 0 || size > MAX_BLOCK) {
        return nullptr;
    }
    void *nAddr = mallocList->FindFreeBlockBySize(size);
    if (nAddr != nullptr) {
        return nAddr;
    }
    // There's no free block that satisfies the size requirement, need to allocate a new one
    void *res = mallocList->Insert(size);
    if (res == nullptr) {
        // allocation of new metadata block/new data block failed.
        return nullptr;
    }
    return res;
}

void *scalloc(size_t num, size_t size) {
    void *addr = smalloc(num * size);
    if (addr == nullptr) {
        // allocation of block failed
        return nullptr;
    }
    // Setting all newly allocated blocks to 0
    memset(addr, 0, num * size);
    return addr;
}

void sfree(void *p) {
    if (p == nullptr) {
        return;
    }
    mallocList->MarkAsFree(p);
}

void *srealloc(void *oldp, size_t size) {
    if (size == 0 || size > MAX_BLOCK) {
        return nullptr;
    }
    if (oldp == nullptr) {
        // No real previous address, just allocate a new one with the requested size
        return smalloc(size);
    }
    size_t currentSize = mallocList->GetSizeOfBlockByAddress(oldp);
    if (currentSize >= size) {
        // The current block is already big enough to hold the requested size
        return oldp;
    }
    void *nAddr = smalloc(size);
    if (nAddr == nullptr) {
        // Allocation of new block failed
        return nullptr;
    }
    // Copying data from old address to new
    memcpy(nAddr, oldp, currentSize);
    // Marking old address as free
    mallocList->MarkAsFree(oldp);
    return nAddr;
}

// Returns the number of allocated blocks in the heap that are currently free
size_t _num_free_blocks() {
    return mallocList->get_num_free_blocks();
}

// Returns the number of bytes in all allocated blocks in the heap that are currently free,
// excluding the bytes used by the meta-data structs.
size_t _num_free_bytes() {
    return mallocList->get_num_free_bytes();
}

// Returns the overall (free and used) number of allocated blocks in the heap.
size_t _num_allocated_blocks() {
    return mallocList->get_num_allocated_blocks();
}

// Returns the overall number (free and used) of allocated bytes in the heap, excluding
// the bytes used by the meta-data structs.
size_t _num_allocated_bytes() {
    return mallocList->get_num_allocated_bytes();
}

// Returns the overall number of meta-data bytes currently in the heap.
size_t _num_meta_data_bytes() {
    return mallocList->get_num_metadata_bytes();
}

// Returns the number of bytes of a single meta-data structure in your system.
size_t _size_meta_data() {
    return mallocList->get_metadata_size();
}