#include "CppUTest/TestHarness.h"

#include "fixed_size_block_allocator.h"

TEST_GROUP (block_allocator) {
};

TEST (block_allocator, check_init_state) {
    FixedSizeBlockAllocator<uint32_t, 12, 32> mem_pool;

    CHECK_EQUAL(12, mem_pool.avail());
}

TEST (block_allocator, check_get_and_put) {
    FixedSizeBlockAllocator<uint32_t, 10, 100> mem_pool;

    void *p = mem_pool.get();
    CHECK(p != (void *)0);
    CHECK_EQUAL(9, mem_pool.avail());

    mem_pool.put(p);
    CHECK_EQUAL(10, mem_pool.avail());
}

TEST (block_allocator, check_underflow) {
    FixedSizeBlockAllocator<uint32_t, 2, 32> a2;

    CHECK_EQUAL(2, a2.avail());

    void *p1 = a2.get();
    CHECK(p1 != (void *)0);
    CHECK_EQUAL(1, a2.avail());

    void *p2 = a2.get();
    CHECK_EQUAL(0, a2.avail());
    CHECK(p2 != (void *)0);

    void *p3 = a2.get(); // underflow
    CHECK_EQUAL(0, a2.avail());
    CHECK_EQUAL(p3, (void *)0);
}

TEST (block_allocator, check_block_alignment) {
    FixedSizeBlockAllocator<uint32_t, 4, 11> a4;

    void *p1 = a4.get();
    CHECK_EQUAL_ZERO(reinterpret_cast<std::uintptr_t>(p1) %
                     (sizeof(unsigned long)));
    void *p2 = a4.get();
    CHECK_EQUAL_ZERO(reinterpret_cast<std::uintptr_t>(p2) %
                     (sizeof(unsigned long)));
    void *p3 = a4.get();
    CHECK_EQUAL_ZERO(reinterpret_cast<std::uintptr_t>(p3) %
                     (sizeof(unsigned long)));
    void *p4 = a4.get();
    CHECK_EQUAL_ZERO(reinterpret_cast<std::uintptr_t>(p4) %
                     (sizeof(unsigned long)));
}
