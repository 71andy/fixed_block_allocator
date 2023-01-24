#ifndef FIXED_SIZE_BLOCK_ALLOCATOR_H
#define FIXED_SIZE_BLOCK_ALLOCATOR_H

// default critical section policy for allocator
// (separate access from different tasks is not supported)
class NoCriticalSectionPolicy {
  public:
    NoCriticalSectionPolicy() {}
    ~NoCriticalSectionPolicy() {}
};

// Critical section policy example for CortexM and IAR compiler
/*
class CriticalSectionPolicy_CortexM_IAR {
  public:
    CriticalSectionPolicy_CortexM_IAR() : StatusReg(__get_interrupt_state())
    {
        __disable_interrupt();
    }
    ~CriticalSectionPolicy_CortexM_IAR()
    {
        __set_interrupt_state(StatusReg);
    }

  private:
    __istate_t StatusReg;
};
*/

// * T - most efficient data type for the architecture (probably uint32_t or
// uint16_t)
// * BLOCK_NUM - number of blocks in pool, type T.
// * BLOCK_SIZE - block size (bytes), type T.
// * CS_POLICY - critical section policy to run allocator in RTOS environment,
// class with:
//   constructor - should save state and disable interrupt (or can use OS
//   service to lock resourse)
// destructor - should restore interrupt state (or unlock resourse)
template <typename T, T BLOCK_NUM, T BLOCK_SIZE,
          class CS_POLICY = NoCriticalSectionPolicy>
class FixedSizeBlockAllocator {
    // pool item buffer and item memory alignment
    union pool_item_t {
        char buf[BLOCK_SIZE];
        pool_item_t *p_next;
        long int memory_alignment;
    };

  public:
    FixedSizeBlockAllocator() {
        CS_POLICY cs;
        for (T i = 0; i < BLOCK_NUM - 1; ++i) {
            pool_buffer[i].p_next = &pool_buffer[i + 1];
        }
        pool_buffer[BLOCK_NUM - 1].p_next = 0;
        p_first = pool_buffer;
        available_blocks = BLOCK_NUM;
    }

    // return pointer to block or 0, if not free blocks available
    void *get() {
        CS_POLICY cs;
        if (available_blocks) {
            pool_item_t *p_blk = p_first;
            p_first = p_first->p_next;
            available_blocks--;
            return p_blk;
        }
        return 0; // or can use nullptr for c++11 and above
    }

    // Put block back to memory pool
    // Complete error check is not realized.
    // For reliability reasons, the following checks can be implemented with the
    // return error codes:
    // - attempt to return a nullptr to the full pool
    // - attempt to return a block to the full pool
    // - attempt to return a pointer that does not point to the start of any
    // block in the pool
    // returns: true - on success, false - on error
    bool put(void *p_blk) {
        if (0 == p_blk) { // no critical section needed for this check
          return false;
        }
        CS_POLICY cs;
        if (available_blocks >= BLOCK_NUM) {
          return false;
        }
        static_cast<pool_item_t *>(p_blk)->p_next = p_first;
        p_first = static_cast<pool_item_t *>(p_blk);
        available_blocks++;
        return true;
    }

    T avail() { return available_blocks; }

  private:
    pool_item_t pool_buffer[BLOCK_NUM]; // memory pool buffer
    pool_item_t *p_first; // pointer to first block in free blocks linked list
    T available_blocks;   // number of free blocks in buffer
};

#endif
