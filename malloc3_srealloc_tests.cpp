#include <assert.h>
#include <iostream>
#include "smalloc.h"

//include your header file here
//compile with g++ -g malloc3_srealloc_tests.cpp malloc_3.cpp -o sreallocTest
//then run
//RUN ONLY ONE TEST AT A TIME!!!!!!!! ¯\_(ツ)_/¯
//goodluck ^_^

void malloc3_realloc_note_a_test(){
    std::cout << "test a" << std::endl;
    assert(_num_allocated_bytes()  == 0);
    assert(_num_free_blocks() == 0);
    assert(_num_allocated_blocks() == 0);

    //realloc note a
    char* first = (char *) smalloc(150);
    char* second = (char *) srealloc(first,149);
    
    assert(first == second);
    
    assert(_num_allocated_bytes()  == 150);
    assert(_num_free_blocks() == 0);
    assert(_num_allocated_blocks() == 1);
    
}

void malloc3_realloc_note_b_test(){
    std::cout << "test b" << std::endl;
    assert(_num_allocated_bytes()  == 0);
    assert(_num_free_blocks() == 0);
    assert(_num_allocated_blocks() == 0);

    //realloc note b
    char* first = (char *) smalloc(150);
    char* second = (char *) smalloc(150);
    sfree(first);
    char* third = (char *) srealloc(second,332);
    
    assert(_num_allocated_bytes()  == 332);
    assert(_num_free_blocks() == 0);
    assert(_num_allocated_blocks() == 1);
    
}

void malloc3_realloc_note_c_test(){
    std::cout << "test c" << std::endl;
    assert(_num_allocated_bytes()  == 0);
    assert(_num_free_blocks() == 0);
    assert(_num_allocated_blocks() == 0);

    //realloc note c
    char* first = (char *) smalloc(150);
    char* second = (char *) smalloc(150);
    sfree(second);
    char* third = (char *) srealloc(first,332);
    
    assert(_num_allocated_bytes()  == 332);
    assert(_num_free_blocks() == 0);
    assert(_num_allocated_blocks() == 1);
    
}

void malloc3_realloc_note_d_test(){
    std::cout << "test d" << std::endl;
    assert(_num_allocated_bytes()  == 0);
    assert(_num_free_blocks() == 0);
    assert(_num_allocated_blocks() == 0);

    //realloc note d
    char* first = (char *) smalloc(150);
    char* second = (char *) smalloc(150);
    char* third = (char *) smalloc(150);
    sfree(first);
    sfree(third);
    srealloc(second,514);
    
    assert(_num_allocated_bytes()  == 514);
    assert(_num_free_blocks() == 0);
    assert(_num_allocated_blocks() == 1);
    
}

void malloc3_realloc_note_e_test(){
    std::cout << "test e" << std::endl;
    assert(_num_allocated_bytes()  == 0);
    assert(_num_free_blocks() == 0);
    assert(_num_allocated_blocks() == 0);

    //realloc note e
    char* first = (char *) smalloc(150);
    char* second = (char *) smalloc(150);
    char* third = (char *) smalloc(150);
    char* fourth = (char *) smalloc(600);

    sfree(first);
    sfree(third);
    sfree(fourth);
    srealloc(second,600);
    _num_allocated_blocks();
    _num_free_blocks();
    assert(_num_allocated_bytes()  == (1082));
    assert(_num_free_blocks() == 2);
    assert(_num_allocated_blocks() == 3);
    
}

void malloc3_realloc_note_d2_test(){
    std::cout << "test d2" << std::endl;
    assert(_num_allocated_bytes()  == 0);
    assert(_num_free_blocks() == 0);
    assert(_num_allocated_blocks() == 0);

    //realloc note d
    char* first = (char *) smalloc(150);
    char* second = (char *) smalloc(150);
    char* third = (char *) smalloc(150);
    char* fourth = (char *) smalloc(600);

    sfree(first);
    sfree(third);
    sfree(fourth);
    srealloc(second,1146);
    
    assert(_num_allocated_bytes()  == 1146);
    assert(_num_free_blocks() == 0);
    assert(_num_allocated_blocks() == 1);
    
}

void malloc3_realloc_note_f_test(){
    std::cout << "test f" << std::endl;
    assert(_num_allocated_bytes()  == 0);
    assert(_num_free_blocks() == 0);
    assert(_num_allocated_blocks() == 0);

    //realloc note d
    char* first = (char *) smalloc(150);
    char* second = (char *) smalloc(150);

    srealloc(first,151);
    
    assert(_num_allocated_bytes()  == 451);
    assert(_num_free_blocks() == 1);
    assert(_num_allocated_blocks() == 3);
    
}

int main(){

	//RUN ONLY ONE TEST AT A TIME!!!!!!!! ^_^
    // PASS
    //malloc3_realloc_note_a_test();
    // PASS
    //malloc3_realloc_note_b_test();
    // PASS
    //malloc3_realloc_note_c_test();
    // PASS
    //malloc3_realloc_note_d_test();
    // PASS
	//malloc3_realloc_note_d2_test();
	// bad num of blocks
    //malloc3_realloc_note_e_test();
    // PASS
    malloc3_realloc_note_f_test();

    return 0;
}