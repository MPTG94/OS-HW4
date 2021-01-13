//
// Created by sivan on 1/13/2021.
//


#include <unistd.h>
#include <stdio.h>
#include <cstring>
#include <iostream>
#include <sys/mman.h>

#define MAX_BLOCK 100000000
#define MMAP_MIN_SIZE 128000

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

    friend class MmapList;
};

class MmapList {
    MallocMetadata *head = nullptr;
    MallocMetadata *tail = nullptr;
    size_t num_allocated_blocks = 0;
    size_t num_allocated_bytes = 0;
    size_t num_of_metadata = 0;

public:
    void *MmapInsert(size_t size) {
        // Try to allocate the requested size using mmap
        void *mmapAddr = mmap(NULL, size + get_metadata_size(), PROT_WRITE | PROT_READ, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
        if (mmapAddr == (void *) (-1)) {
            // mmap failed
            return nullptr;
        }
        MallocMetadata *nMeta = (MallocMetadata *) mmapAddr;
        if (head == nullptr) {
            // insert list head
            nMeta->size = size;
            nMeta->is_free = false;
            void *convAddr = (char *) mmapAddr + get_metadata_size();
            nMeta->mem_address = convAddr;
            nMeta->prev = nullptr;
            nMeta->next = nullptr;
            head = nMeta;
            tail = nMeta;
            IncreaseCounter(size);
            return nMeta->mem_address;
        } else {
            // insert at end
            nMeta->size = size;
            nMeta->is_free = false;
            void *convAddr = (char *) mmapAddr + get_metadata_size();
            nMeta->mem_address = convAddr;
            ReplaceTail(nMeta);
            IncreaseCounter(size);
            return nMeta->mem_address;
        }
    }

    void MmapFree(void *p) {
        MallocMetadata *current = head;
        while (current) {
            if (current->mem_address == p) {
                size_t sizeToFree = current->size + get_metadata_size();
                void *addressToFree = (char *) p - get_metadata_size();
                munmap(addressToFree, sizeToFree);
                if (current == head) {
                    // We need to replace the head
                    if (head->next) {
                        head = head->next;
                        head->prev = nullptr;
                    } else {
                        // The head is the only node in the list
                        head = nullptr;
                    }
                } else {
                    // Need to remove a node from the middle/end of the list
                    if (current->next) {
                        // We are removing a node from the middle of the list
                        MallocMetadata *prev = current->prev;
                        MallocMetadata *next = current->next;
                        prev->next = next;
                        next->prev = prev;
                    } else {
                        // We are removing a node from the end of the list
                        MallocMetadata *prev = current->prev;
                        prev->next = nullptr;
                    }
                }
                DecreaseCounter(sizeToFree);
                return;
            }
        }
    }

    void IncreaseCounter(size_t size) {
        num_of_metadata++;
        num_allocated_blocks++;
        num_allocated_bytes += size;
    }

    void DecreaseCounter(size_t size) {
        num_of_metadata--;
        num_allocated_blocks--;
        num_allocated_bytes -= size;
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

    // Assume memory layout of A B C -> D
    // if both A B C are free, then A should be the only metadata block, and should point to D, and D should point to A
    // if both A and B are free, then A should be the metadata block, and should point to C, and C should point to A
    // if both B and C are free, then B should be the metadata block, and should point to D, and D should point to B
    void MarkAsFree(void *addr) {
        if (head == nullptr) {
            return;
        }
        MallocMetadata *current = head;
        while (current) {
            if (current->mem_address == addr && !current->is_free) {
                if (current->prev && current->prev->is_free && current->next && current->next->is_free) {
                    MallocMetadata *prev = current->prev;
                    size_t new_size = prev->size + current->size + current->next->size + 2 * get_metadata_size();
                    prev->size = new_size;
                    // A will now point to D
                    prev->next = current->next->next;
                    // D will not point to A
                    if (current->next->next) {
                        current->next->next->prev = prev;
                    }
                    num_free_blocks--;
                    num_free_bytes += current->size + 2 * get_metadata_size();
                    num_allocated_blocks -= 2;
                    num_allocated_bytes += 2 * get_metadata_size();
                    num_of_metadata -= 2;
                } else if (current->next && current->next->is_free) {
                    current->is_free = true;
                    MallocMetadata *next = current->next;
                    // B will now point to D
                    current->next = next->next;
                    // D will now point to B
                    next->next->prev = current;
                    current->size += next->size + get_metadata_size();
                    num_free_bytes += get_metadata_size();
                    num_allocated_blocks -= 1;
                    num_allocated_bytes += get_metadata_size();
                    num_of_metadata -= 1;
                } else if (current->prev && current->prev->is_free) {
                    MallocMetadata *prev = current->prev;
                    // Now A will point to C
                    prev->next = current->next;
                    // C will now point to A
                    current->next->prev = prev;
                    prev->size += current->size + get_metadata_size();
                    num_free_bytes += get_metadata_size();
                    num_allocated_blocks -= 1;
                    num_allocated_bytes += get_metadata_size();
                    num_of_metadata -= 1;
                } else {
                    // A and C are not free, so only free B alone
                    current->is_free = true;
                    num_free_blocks++;
                    num_free_bytes += current->size;
                }
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

    bool is_large_enough(size_t cur_size, size_t new_size) {
        if (cur_size - new_size - get_metadata_size() >= 128) {
            return true;
        }
        return false;
    }

    void *AllocateToWildernessIfPossible(size_t size) {
        MallocMetadata *current = head;
        bool wildernessState = true;
        while (current) {
            if (current->is_free == true && current != tail) {
                // this is a free block, not at the tail of the list, so wilderness is not satisfied
                return nullptr;
            } else if (current->is_free == true && current == tail) {
                // this is the only free block, and also the tail, so wilderness is satisfied
                // need to increase the size of the tail
                size_t amountToExtend = size - current->size;
                // Increase the size of the tail
                void *addr = sbrk(amountToExtend);
                if (addr == (void *) (-1)) {
                    return nullptr;
                }
                num_free_bytes -= current->size;
                num_free_blocks--;
                current->size += amountToExtend;
                num_allocated_bytes += amountToExtend;
                return current->mem_address;
            }
            current = current->next;
        }
    }

    void *FindLargeEnoughFreeBlockBySize(size_t new_size) {
        if (head == nullptr) {
            // the list is empty, no free blocks (technically everything is free)
            return nullptr;
        }
        MallocMetadata *current = head;
        while (current) {
            if (current->is_free && is_large_enough(current->size, new_size)) {
                // Marking the current block as used by the requestor and returning the address
                current->is_free = false;
                current->size = new_size;
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

    bool IsEmpty() {
        return head == nullptr;
    }

    // Try to merge with previous block (if possible)
    // Try to merge with next block (if possible)
    // Try to merge with both prev and next (if possible)
    void *TryMerge(void *oldp, size_t nSize) {
        MallocMetadata *current = head;
        while (current) {
            if (current->mem_address == oldp) {
                // Found the block that needs to be increased, we previously checked if we already satisfy the new requirement, so no need to try
                // just try and merge according to cases
                MallocMetadata *prev = current->prev;
                MallocMetadata *next = current->next;
                if (prev->is_free && prev->size + current->size + get_metadata_size() >= nSize) {
                    // Prev and current together are enough
                    prev->is_free = false;
                    prev->next = current->next;
                    current->next->prev = prev;
                    // need to update counters
                    num_free_blocks--;
                    num_free_bytes -= (prev->size + get_metadata_size());
                    num_allocated_bytes += get_metadata_size();
                    num_allocated_blocks--;
                    num_of_metadata--;
                    prev->size += current->size + get_metadata_size();
                    // need to do memcopy
                    memcpy(prev->mem_address, oldp, current->size);
                    return prev->mem_address;
                } else if (next->is_free && next->size + current->size + get_metadata_size() >= nSize) {
                    // next and current together are enough
                    current->next = next->next;
                    next->next->prev = current;
                    // need to update counters
                    num_free_blocks--;
                    num_free_bytes -= (next->size + get_metadata_size());
                    num_allocated_bytes += get_num_metadata_bytes();
                    num_allocated_blocks--;
                    num_of_metadata--;
                    current->size += next->size + get_metadata_size();
                    // No need for memcpy, data is already in place
                    return current->mem_address;
                } else if (prev->is_free && next->is_free && prev->size + next->size + current->size + 2 * get_metadata_size() >= nSize) {
                    // all 3 blocks together are enough
                    prev->is_free = false;
                    prev->next = next->next;
                    next->next->prev = prev;
                    // need to update counters
                    num_free_blocks -= 2;
                    num_free_bytes -= (prev->size + next->size + 2 * get_metadata_size());
                    num_allocated_bytes += 2 * get_metadata_size();
                    num_allocated_blocks -= 2;
                    num_of_metadata -= 2;
                    prev->size += current->size + next->size + 2 * get_metadata_size();
                    // need to do memcpy
                    memcpy(prev->mem_address, oldp, current->size);
                    return prev->mem_address;
                }
            }
        }
    }

    void *split(void *addr, size_t curr_size, size_t new_size) {
        // new meta data need to be at addr + size
        void *nMetadAddr = (char *) addr + new_size;
        MallocMetadata *new_meta_data = (MallocMetadata *) nMetadAddr;
        new_meta_data->is_free = true;
        new_meta_data->size = curr_size - new_size - get_metadata_size();
        new_meta_data->mem_address = new_meta_data + get_metadata_size();
        ReplaceTail(new_meta_data);
        num_of_metadata++;
        num_free_bytes -= get_metadata_size();
        return new_meta_data->mem_address;
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
MmapList *mmapList = (MmapList *) sbrk(sizeof(MmapList));

/**
 * Action order:
 * 1. Will try to find a large enough block to both allocate the space, and also split into two blocks
 * 2. Will try to find a free block with the requested size of more
 * 3. Will try to allocate to the wilderness block if it exists (the tail is the only free one)
 * 4. Will try to normally allocate by creating a new tail and increasing sbrk
 * @param size The size of the new block
 * @return The address of the allocated block
 */
void *smalloc(size_t size) {
    if (size == 0 || size > MAX_BLOCK) {
        return nullptr;
    }
    if (size >= MMAP_MIN_SIZE) {
        return mmapList->MmapInsert(size);
    }
    // Searching for a block big enough to both satisfy the requirement, and have enough size left for a size cut
    void *largeEnoughBlock = mallocList->FindLargeEnoughFreeBlockBySize(size);
    if (largeEnoughBlock != nullptr) {
        // We found a large enough block, get the size of the new block, and use it to create a new metadata and free block, with the remaining size
        size_t curr_size = mallocList->GetSizeOfBlockByAddress(largeEnoughBlock);
        // We create a split, meaning now the largeEnough address stores the block the user requested, and split will generate a new free block
        // for later use
        void *nAdrr = mallocList->split(largeEnoughBlock, curr_size, size);
        return largeEnoughBlock;
    }
    // Just try to find a free block with enough space
    void *nAddr = mallocList->FindFreeBlockBySize(size);
    if (nAddr != nullptr) {
        return nAddr;
    }
    // There's no free block that satisfies the size requirement, let's check if a wilderness block exists (the only one that is free + is the tail)
    void *wilAddr = mallocList->AllocateToWildernessIfPossible(size);
    if (wilAddr != nullptr) {
        return wilAddr;
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
    if (mallocList->GetSizeOfBlockByAddress(p) != 0) {
        mallocList->MarkAsFree(p);
    } else {
        // need to try and free from mmap list
        mmapList->MmapFree(p);
    }
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
    // Try to merge with previous block (if possible)
    // Try to merge with next block (if possible)
    // Try to merge with both prev and next (if possible)
    void *mergeRes = mallocList->TryMerge(oldp, size);
    if (mergeRes != nullptr) {
        // should try to split the block if it is large enough (according to challenge #1 definition)
        // nSize is (possibly) very large, but we actually only need size
        size_t nSize = mallocList->GetSizeOfBlockByAddress(mergeRes);
        if (mallocList->is_large_enough(nSize, size)) {
            // here we need to split, so merge res will hold the data that was realloc'd, and after it in memory there will be a free block
            // that is at least 128kb in size
            mallocList->split(mergeRes, nSize, size);
        }
        return mergeRes;
    }

    // Can't use merge, will call smalloc, and smalloc will try to find a free block that will satisfy the requirement, if not,
    // it will check if the wilderness block is an option, and if not it will just resort to a standard allocation
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
    std::cout << "num free blocks is " << mallocList->get_num_free_blocks() << std::endl;
    return mallocList->get_num_free_blocks();
}

// Returns the number of bytes in all allocated blocks in the heap that are currently free,
// excluding the bytes used by the meta-data structs.
size_t _num_free_bytes() {
    std::cout << "num free bytes is " << mallocList->get_num_free_bytes() << std::endl;
    return mallocList->get_num_free_bytes();
}

// Returns the overall (free and used) number of allocated blocks in the heap.
size_t _num_allocated_blocks() {
    std::cout << "num allocated blocks is " << mallocList->get_num_allocated_blocks() + mmapList->get_num_allocated_blocks() << std::endl;
    return mallocList->get_num_allocated_blocks() + mmapList->get_num_allocated_blocks();
}

// Returns the overall number (free and used) of allocated bytes in the heap, excluding
// the bytes used by the meta-data structs.
size_t _num_allocated_bytes() {
    std::cout << "num allocated bytes is " << mallocList->get_num_allocated_bytes() + mmapList->get_num_allocated_bytes() << std::endl;
    return mallocList->get_num_allocated_bytes() + mmapList->get_num_allocated_bytes();
}

// Returns the overall number of meta-data bytes currently in the heap.
size_t _num_meta_data_bytes() {
    std::cout << "num metadata bytes is " << mallocList->get_num_metadata_bytes() + mmapList->get_num_metadata_bytes() << std::endl;
    return mallocList->get_num_metadata_bytes() + mmapList->get_num_metadata_bytes();
}

// Returns the number of bytes of a single meta-data structure in your system.
size_t _size_meta_data() {
    std::cout << "metadata size is " << mallocList->get_metadata_size() << std::endl;
    return mallocList->get_metadata_size();
}