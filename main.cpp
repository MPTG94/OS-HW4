
#include "smalloc.h"
#include <assert.h>
#include <iostream>

void malloc2_test_01() {

    // malloc
    int *ten = (int *) smalloc(sizeof(int) * 10);
    assert(ten);
    for (int i = 0; i < 10; i++) {
        ten[i] = 10;
    }
    int *five = (int *) smalloc(sizeof(int) * 5);
    assert(five);
    for (int i = 0; i < 5; i++) {
        five[i] = 5;
    }

    for (int i = 0; i < 10; i++) {
        assert(ten[i] == 10);
    }
    for (int i = 0; i < 5; i++) {
        assert(five[i] == 5);
    }

    // calloc
    int *three = (int *) scalloc(3, sizeof(int));
    assert(three);
    for (int i = 0; i < 3; i++) {
        assert(three[i] == 0);
    }

    // helpers
    assert(_num_free_blocks() == 0);
    assert(_num_free_bytes() == 0);
    assert(_num_allocated_blocks() == 3);
    assert(_num_allocated_bytes() == sizeof(int) * 18);
    assert(_num_meta_data_bytes() == _size_meta_data() * 3);

    // realloc
    int *ninety = (int *) srealloc(ten, sizeof(int) * 90);
    for (int i = 0; i < 90; i++) {
        ninety[i] = 90;
    }
    assert(ninety);
    assert(_num_free_blocks() == 1);
    assert(_num_free_bytes() == sizeof(int) * 10);
    assert(_num_allocated_blocks() == 4);
    assert(_num_allocated_bytes() == sizeof(int) * 108);
    assert(_num_meta_data_bytes() == _size_meta_data() * 4);

    int *sixty = (int *) srealloc(NULL, sizeof(int) * 60);
    assert(sixty);
    assert(_num_free_blocks() == 1);
    assert(_num_free_bytes() == sizeof(int) * 10);
    assert(_num_allocated_blocks() == 5);
    assert(_num_allocated_bytes() == sizeof(int) * 168);
    assert(_num_meta_data_bytes() == _size_meta_data() * 5);

    // order so far: ten(freed), five, three, ninety, sixty
    // free & malloc
    sfree(ninety);
    int *eleven = (int *) smalloc(sizeof(int) * 11);
    assert(eleven == ninety);
    for (int i = 0; i < 11; i++) {
        eleven[i] = 11;
    }
    for (int i = 11; i < 90; i++) {
        assert(ninety[i] == 90);
    }

    // order so far: ten(freed), five, three, ninety(eleven), sixty
    assert(_num_free_blocks() == 1);
    assert(_num_free_bytes() == sizeof(int) * 10);
    assert(_num_allocated_blocks() == 5);
    assert(_num_allocated_bytes() == sizeof(int) * 168);
    assert(_num_meta_data_bytes() == _size_meta_data() * 5);

}

void malloc3_test_01() {

    char *first = (char *) smalloc(150);
    //std::cout << "addr of first: " << (void *) first << std::endl;
    assert(first);
    char *second = (char *) smalloc(150);
    //std::cout << "addr of second: " << (void *) second << std::endl;
    assert(second);
    char *third = (char *) smalloc(150);
    //std::cout << "addr of third: " << (void *) third << std::endl;
    assert(third);
    assert(_num_free_blocks() == 0);
    assert(_num_free_bytes() == 0);
    assert(_num_allocated_blocks() == 3);
    assert(_num_allocated_bytes() == 450);
    sfree(first);
    assert(_num_free_blocks() == 1);
    assert(_num_free_bytes() == 150);
    sfree(third);
    assert(_num_free_blocks() == 2);
    assert(_num_free_bytes() == 300);
    assert(_num_allocated_blocks() == 3);
    sfree(second);
    assert(_num_free_blocks() == 1);
    assert(_num_free_bytes() == 450 + (2 * _size_meta_data()));
    assert(_num_allocated_blocks() == 1);
    first = (char *) smalloc(200);
    assert(first);
    assert(_num_free_blocks() == 1);
    assert(_num_allocated_blocks() == 2);
    assert(_num_free_bytes() == 250 + _size_meta_data());
    third = (char *) smalloc(400);                //wilder
    assert(third);
    assert(_num_free_blocks() == 0);
    assert(_num_allocated_blocks() == 2);
    assert(_num_free_bytes() == 0);
    sfree(third);
    assert(_num_free_blocks() == 1);
    assert(_num_allocated_blocks() == 2);
    assert(_num_free_bytes() == 400);
    second = (char *) smalloc(250);
    assert(second);
    assert(_num_free_blocks() == 0);
    assert(_num_allocated_blocks() == 2);
    assert(_num_free_bytes() == 0);
    third = (char *) smalloc(320);
    assert(third);
    assert(_num_free_blocks() == 0);
    assert(_num_allocated_blocks() == 3);
    assert(_num_allocated_bytes() == 920);
    sfree(second);
    sfree(third);
    assert(_num_free_blocks() == 1);
    assert(_num_allocated_blocks() == 2);
    assert(_num_free_bytes() == 720 + _size_meta_data());
    assert(_num_allocated_bytes() == 920 + _size_meta_data());
    sfree(first);
    assert(_num_free_blocks() == 1);
    assert(_num_allocated_blocks() == 1);
    assert(_num_free_bytes() == 920 + 2 * _size_meta_data());
    assert(_num_allocated_bytes() == 920 + 2 * _size_meta_data());
    //////////////////////////////////////////////////  mmap time!!///////////////////////////////
    char *first_mmap = (char *) smalloc(150000);
    assert(first_mmap);
    assert(_num_free_blocks() == 1);
    assert(_num_allocated_blocks() == 2);
    assert(_num_allocated_bytes() == 920 + 2 * _size_meta_data() + 150000);
    sfree(first_mmap);
    assert(_num_free_blocks() == 1);
    assert(_num_allocated_blocks() == 1);
    assert(_num_allocated_bytes() == 920 + 2 * _size_meta_data());
    first_mmap = (char *) smalloc(150000);
    assert(first_mmap);
    assert(_num_free_blocks() == 1);
    assert(_num_allocated_blocks() == 2);
    assert(_num_allocated_bytes() == 920 + 2 * _size_meta_data() + 150000);
    char *second_mmap = (char *) smalloc(150000);
    assert(second_mmap);
    assert(_num_free_blocks() == 1);
    assert(_num_allocated_blocks() == 3);
    assert(_num_allocated_bytes() == 920 + 2 * _size_meta_data() + 300000);
    char *third_mmap = (char *) smalloc(150000);
    assert(third_mmap);
    assert(_num_free_blocks() == 1);
    assert(_num_allocated_blocks() == 4);
    assert(_num_allocated_bytes() == 920 + 2 * _size_meta_data() + 450000);
    sfree(third_mmap);
    assert(_num_free_blocks() == 1);
    assert(_num_allocated_blocks() == 3);
    assert(_num_allocated_bytes() == 920 + 2 * _size_meta_data() + 300000);
    char *forth_mmap = (char *) smalloc(200000);
    assert(forth_mmap);
    assert(_num_free_blocks() == 1);
    assert(_num_allocated_blocks() == 4);
    assert(_num_allocated_bytes() == 920 + 2 * _size_meta_data() + 500000);
    sfree(first_mmap);
    sfree(second_mmap);
    sfree(forth_mmap);
    assert(_num_allocated_bytes() == 920 + 2 * _size_meta_data());

}

void malloc3_test_02() {
    if (_num_allocated_bytes() == 920 + 2 * _size_meta_data()) {
        std::cout << " WHY DONT YOU LISTEN?! RUN THIS FUNCTION ALONE! :) " << std::endl;
    }
    assert(_num_allocated_bytes() == 0);
    char *first_sbrk = (char *) srealloc(nullptr, 100);
    assert(first_sbrk);
    assert(_num_free_blocks() == 0);
    assert(_num_free_bytes() == 0);
    assert(_num_allocated_blocks() == 1);
    assert(_num_allocated_bytes() == 100);
    sfree(first_sbrk);
    assert(_num_free_blocks() == 1);
    assert(_num_free_bytes() == 100);
    first_sbrk = (char *) srealloc(nullptr, 150);
    assert(first_sbrk);
    assert(_num_free_blocks() == 0);
    assert(_num_free_bytes() == 0);
    assert(_num_allocated_blocks() == 1);
    assert(_num_allocated_bytes() == 150);
    sfree(first_sbrk);
    assert(_num_free_blocks() == 1);
    assert(_num_free_bytes() == 150);
    first_sbrk = (char *) smalloc(150);
    assert(first_sbrk);
    assert(_num_free_blocks() == 0);
    assert(_num_free_bytes() == 0);
    assert(_num_allocated_blocks() == 1);
    assert(_num_allocated_bytes() == 150);
    char *second_sbrk = (char *) smalloc(100);
    assert(second_sbrk);
    char *third_sbrk = (char *) smalloc(50);
    assert(third_sbrk);
    assert(_num_free_blocks() == 0);
    assert(_num_free_bytes() == 0);
    assert(_num_allocated_blocks() == 3);
    assert(_num_allocated_bytes() == 300);
    char *forth_sbrk = (char *) srealloc(second_sbrk, 120);
    assert(forth_sbrk);
    assert(_num_free_blocks() == 1);
    assert(_num_free_bytes() == 100);
    assert(_num_allocated_blocks() == 4);
    char *check_sbrk = (char *) srealloc(first_sbrk, 120);
    assert(check_sbrk == first_sbrk);
    second_sbrk = (char *) smalloc(80);
    assert(second_sbrk);
    assert(_num_free_blocks() == 0);
    assert(_num_free_bytes() == 0);
    assert(_num_allocated_blocks() == 4);
    sfree(third_sbrk);
    assert(_num_free_bytes() == 50);
    forth_sbrk = (char *) srealloc(forth_sbrk, 160);
    assert(forth_sbrk);
    assert(_num_free_bytes() == 50);
    // Fail here, some issue with merge sizes and expected free blocks
    assert(_num_allocated_blocks() == 4);
    sfree(first_sbrk);
    assert(_num_free_blocks() == 2);
    assert(_num_free_bytes() == 200);
    first_sbrk = (char *) srealloc(second_sbrk, 290);
    assert(first_sbrk);
    assert(_num_allocated_blocks() == 2);
    assert(_num_free_blocks() == 0);
    third_sbrk = (char *) smalloc(120);
    assert(third_sbrk);
    char *fifth_sbrk = (char *) smalloc(50);
    sfree(fifth_sbrk);
    sfree(forth_sbrk);
    second_sbrk = (char *) srealloc(third_sbrk, 220);
    assert(_num_allocated_blocks() == 3);
    second_sbrk = (char *) srealloc(second_sbrk, 340);
    assert(_num_allocated_blocks() == 2);
    third_sbrk = (char *) smalloc(50);
    forth_sbrk = (char *) smalloc(240);
    sfree(first_sbrk);
    sfree(forth_sbrk);
    third_sbrk = (char *) srealloc(third_sbrk, 90);
    assert(_num_allocated_blocks() == 4);
    third_sbrk = (char *) srealloc(third_sbrk, 350);
    assert(_num_allocated_blocks() == 3);
    //////////////////////////////////////////////////  mmap time!!////////////////////////////


}

void malloc3_test_03() {
    if (_num_allocated_bytes() > 0) {
        std::cout << " WHY DONT YOU LISTEN?! RUN THIS FUNCTION ALONE! :) " << std::endl;
    }
    assert(_num_allocated_bytes() == 0);
    char *first_mmap = (char *) smalloc(150000);
    assert(first_mmap);
    assert(_num_free_blocks() == 0);
    char *second_mmap = (char *) smalloc(150000);
    assert(first_mmap);
    second_mmap = (char *) srealloc(second_mmap, 200000);
    assert(_num_allocated_bytes() == 350000);
    second_mmap = (char *) srealloc(second_mmap, 140000);
    // Should realloc make an existing block smaller? (Piazza)
    assert(_num_allocated_bytes() == 290000);
    sfree(first_mmap);

}


int main() {

    // check it for malloc2 before this test
    //malloc2_test_01();
    //malloc3_test_01();
    //malloc3_test_02();
    malloc3_test_03();


    return 0;
}
