/**
 * Do not submit your assignment with a main function in this file.
 * If you submit with a main function in this file, you will get a zero.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "debug.h"
#include "sfmm.h"
#include "errno.h"

double curr_pld = 0;
double max_pld = 0;

void qkl_add(sf_block* block, int i);
void free_list_add(sf_block* free_block);

int get_block_size(sf_block* block) { 
    int size = (block->header ^ MAGIC) & 0xFFFFFFF0;
    return size;
} //retrieve bits 33-60 and store in variable block_size

//initializes headers of blocks, takes the block, payload size, block_size, is_alloc = 0 if free or pro/epilogue
void init_header(sf_block* block, sf_size_t pay_size, int block_size, int is_al, int is_pal, int is_qkl) {
    block->header = 0;                    //initialize payload as 0
    if (pay_size != 0) {                  //if payload size > 0, then the payload is set
        block->header |= pay_size;        //set header = payload
        block->header <<= 32;             //bit shift by 32
    }
    block->header ^= MAGIC;   
    int ret = get_block_size(block);         //retrieve bits 33-60 and store in variable ret
    block->header ^= MAGIC;   
    ret |= block_size;                       //or ret with block size 
    block->header |= ret;                     //set header to ret value
    if (is_al != 0) {block->header |= THIS_BLOCK_ALLOCATED;}   //or with 0x4 to set alloc bit to 1
    if (is_pal != 0) {block->header |= PREV_BLOCK_ALLOCATED;}  //or with 0x2 to set prev_alloc bit to 1
    if (is_qkl != 0) {block->header |= IN_QUICK_LIST;}         //or with 0x1 to set in qklst bit to 1     
    block->header ^= MAGIC;                             //obfuscate header
}

void init_free_list_heads() {
    for (int i = 0; i < NUM_FREE_LISTS; i++) {
        sf_block* head = &sf_free_list_heads[i];
        head->body.links.next = head;
        head->body.links.prev = head;
    }
}

int get_free_list(int block_size) {
    int log = block_size / 32;
    int i;
    for (i = 0; log > 1; i++) { log /= 2; }
    if (i > 9) { i = 9; }
    return i;
}

void init_qkl() { for (int i = 0; i < NUM_QUICK_LISTS; i++) { sf_quick_lists[i].length = 0; } }

int get_qkl(int block_size) { return (block_size-32)/16; }

void qkl_remove(int i) {
    sf_quick_lists[i].first = sf_quick_lists[i].first->body.links.next;
    sf_quick_lists[i].length--;
}

void qkl_add(sf_block* block, int i) {
    int len = sf_quick_lists[i].length;
    if(len == QUICK_LIST_MAX) { 
        for (int j = 0; i < sf_quick_lists[i].length; j++) { 
            free_list_add(sf_quick_lists[i].first);
            qkl_remove(i); 
        } 
    }
    int size = get_block_size(block);
    int pal = (block->header ^ MAGIC) & PREV_BLOCK_ALLOCATED;
    init_header(block, 0, size, 1, pal, 1);
    block->body.links.next = sf_quick_lists[i].first;
    sf_quick_lists[i].first = block;
    sf_quick_lists[i].length++;
}

void free_list_add(sf_block* free_block) {
    int block_size = get_block_size(free_block);      //get the size of the block you want to add
    int qkl = get_qkl(block_size);
    if (qkl < QUICK_LIST_MAX) {
        qkl_add(free_block, qkl);
    } else {
        int index = get_free_list(block_size);            //get the index of the free lists array to add to the correct playlist
        sf_block* curr = free_block;                      //curr = free_block
        sf_block* head = &sf_free_list_heads[index];      //head = head of the appropriate list

        curr->body.links.next = head->body.links.next;   //curr->next = head->next
        head->body.links.next = curr;                    //head->next = curr
        curr->body.links.prev = head;                    //curr->prev = head
        curr->body.links.next->body.links.prev = curr;   //next->prev = curr
    }
}

void remove_free_block(sf_block* curr) {
    curr->body.links.prev->body.links.next = curr->body.links.next;                      //curr->prev = curr->next
    curr->body.links.next->body.links.prev = curr->body.links.prev;     //curr->next->prev = curr->prev
    curr->body.links.next = curr;
    curr->body.links.prev = curr;
}

sf_block* grow_heap() {
    sf_block* free_block = sf_mem_end() - 16;                 //new freeblock starts at where epilogue was
    if (sf_mem_grow() == NULL) {return NULL; }                //request a new page
    else {
        int pal = (free_block->header ^ MAGIC) & PREV_BLOCK_ALLOCATED;      //get the prev alloc bit
        init_header(free_block, 0, PAGE_SZ, 0, pal, 0);           //initialize free block
        sf_block* epilogue = (void*)free_block + PAGE_SZ;         //addr of new epilogue is free block + 1024
        init_header(epilogue, 0, 0, 1, 0, 0);                     //initialize epilogue
        epilogue->prev_footer = free_block->header;               //initiliaze footer of free block   
        return free_block;
    }
}

sf_block* coalesce(sf_block* curr) {
    sf_block* next, * prev;
    int curr_size = get_block_size(curr), prev_size, next_size;
    int pal = (curr->header ^ MAGIC) & PREV_BLOCK_ALLOCATED;       //get the prev alloc bit
    
    next = (void*) curr + curr_size;                     //get addr of next block
    int al = (next->header ^ MAGIC) & THIS_BLOCK_ALLOCATED;       //get the alloc bit
    if(al == 0) {
        next_size = get_block_size(next);               //get next size
        curr_size += next_size;                         //add next size to curr size
        init_header(curr, 0, curr_size, 0, pal, 0);
        sf_block* next_next = (void*)next+next_size;
        next_next->prev_footer = curr->header;               //update footer of curr
        remove_free_block(next);
    }
    if (pal == 0) {  
        prev_size = (curr->prev_footer ^ MAGIC) & 0xFFFFFFF0;     //get prev block size
        prev = (void*)curr - prev_size;                  //prev = prev block addr
        pal = (prev->header ^ MAGIC) & PREV_BLOCK_ALLOCATED;      //get the prev alloc bit
        prev_size += curr_size;                         //add curr size to prev
        init_header(prev, 0, prev_size, 0, pal, 0);
        sf_block* curr_next = (void*)curr + curr_size;
        curr_next->prev_footer = prev->header;               //init prev footer
        curr = prev;                             //curr = prev
    }
    return curr;
}

sf_block* alloc(int size, int pay_size, int index) {
    sf_block* head = &sf_free_list_heads[index];      //get the head at the index
    sf_block* curr = head->body.links.next;           //curr = the block immediately after head       
    while (curr != head) { 
        int block_size = get_block_size(curr);                   //get the size of the current block
        if (size < block_size) { curr = curr->body.links.next; } //if the block in the list has a block size < size, then check the next block
    } 
    if (curr == head) {
        while (curr == head && index < NUM_FREE_LISTS) {    //while curr == head means free list is empty, keep incrementing until you find a non empty list  
            index++;                                        //increment index      
            head = &sf_free_list_heads[index];              //get the head at the index
            curr = head->body.links.next;                   //curr = the block immediately after head
        }
        sf_block* free_block;
        if (index == NUM_FREE_LISTS) {                      //if no free lists have a free block big enough to allocate, grow the heap and coalesce
            int curr_size = 0;
            while (size > curr_size) { 
                curr = grow_heap();                         //grow the heap
                if (curr != NULL) {
                    free_block = coalesce(curr);            //coalesce the newly freed block and save it in curr
                    curr_size = get_block_size(free_block); //get the size of the coalesced block
                } else {
                    sf_errno = ENOMEM;
                    return NULL; 
                }
            }
            curr = free_block;
        }
    }      
    int free_block_size = get_block_size(curr);         //get the size of the free block
    int pal = (curr->header ^ MAGIC) & PREV_BLOCK_ALLOCATED;      //get the prev alloc bit
    if (size == free_block_size || free_block_size - size < 32) { 
        init_header(curr, pay_size, size, 1, pal, 0); 
        remove_free_block(curr);
    } else {
        //allocate block with size less than free_block_size and > 32
        init_header(curr, pay_size, size, 1, pal, 0);   //make the allocated block's header 
        //split
        sf_block* free_block = (void*) curr + size;     //the free block's address is the current block's address + its size
        free_block_size -= size;                        //free block's size decreased since part of it was allocated, need to decrease it by allocated block size
        init_header(free_block, 0, free_block_size, 0, 1, 0);
        free_list_add(free_block);                             //add the new free block to its appropriate free list
        sf_block* next = (void*)free_block + free_block_size;  //next = block after free block
        next->prev_footer = free_block -> header;              //setting the footer of the free block
        remove_free_block(curr);
    }
    return curr;
}

void update_curr_pld() {
    sf_block* block = sf_mem_start() + 32;
    sf_block* end = sf_mem_end() - 16;
    curr_pld = 0;
    while (block != end)  {
        int al = (block->header ^ MAGIC) & THIS_BLOCK_ALLOCATED;
        int qkl = (block->header ^ MAGIC) & IN_QUICK_LIST;
        if (al && !qkl) { curr_pld += (block->header ^ MAGIC) >> 32;}
        block = (void*)block + get_block_size(block);
    }
}

void *sf_malloc(sf_size_t size) {
    if (size == 0) {return NULL;}
    if (sf_mem_start() == sf_mem_end()) {
        sf_mem_grow();                                  //req a page

        sf_block* prologue = sf_mem_start();            //prologue = mem_start addr + 8 bytes
        sf_block* epilogue = sf_mem_end() - 16;         //epilogue = mem_end addr - 16 bytes
        prologue->prev_footer = 0;                      //initialize the unused 8 bytes before prologue as 0

        //initialize the prologue and epilogue of the page
        init_header(prologue, 0, 0x20, 1, 0, 0);     //initialize prologue
        init_header(epilogue, 0, 0, 1, 0, 0);        //initialize epilogue

        //initialize the first free block, payload size = 0, block size = PAGE_SZ-48, not allocated           
        sf_block* free_block = sf_mem_start() + 32;
        init_header(free_block, 0, PAGE_SZ-48, 0, 1, 0);//create a free block spanning the page
        epilogue->prev_footer = free_block->header;     //set epilogue's footer to free block's header
        init_free_list_heads();                         //initialize all the sentinels in the array of segregated free lists
        free_list_add(free_block);                      //add the free_block to the appropriate free list
    }

    int pay_size = size;                                //payload size = size
    //turn the payload size into block size
    if (size+8 <= 32) {size = 32;}
    else if ((size+8) % 16 != 0) {size = ((size+8) + (16-((size+8)%16)));}
    else if ((size+8) % 16 == 0) {size += 8;}
    else size+=8;

    int index = get_qkl(size);
    sf_block* ptr;
    if (index < QUICK_LIST_MAX && sf_quick_lists[index].length != 0) {
        ptr = sf_quick_lists[index].first; 
        int pal =  (ptr->header ^ MAGIC) & PREV_BLOCK_ALLOCATED;
        init_header(ptr, pay_size, size, 1, pal, 0);
        qkl_remove(index);
    } else {
        index = get_free_list(size);                    //get the index of the appropriate free list
        ptr = alloc(size, pay_size, index);       //call alloc with block size, payload size and the ind
    }

    update_curr_pld();
    if (curr_pld > max_pld) {max_pld = curr_pld;}

    if (ptr == NULL) { return NULL; }
    else return ptr->body.payload;
}

int is_valid(void *pp) {
    sf_block* block = pp-16;
    int block_size = get_block_size(block);
    int al = (block->header ^ MAGIC) & THIS_BLOCK_ALLOCATED;
    int pal =  (block->header ^ MAGIC) & PREV_BLOCK_ALLOCATED;
    int al_prev =  (block->prev_footer ^ MAGIC) & THIS_BLOCK_ALLOCATED;

    if (pp == NULL) {return 0;}
    if ((uintptr_t)pp % 16 != 0) {return 0;}
    if (block_size < 32) {return 0;}
    if (block_size % 16 != 0) {return 0;}
    if (al == 0) {return 0;}
    if (pal == 0) { if (al_prev == 1) {return 0;} }
    return 1;
}

void sf_free(void *pp) {
    int val = is_valid(pp);
    if (val == 0) {abort();}
    sf_block* block = pp-16;
    int block_size = get_block_size(block);
    int pal =  (block->header ^ MAGIC) & PREV_BLOCK_ALLOCATED;
    init_header(block, 0, block_size, 0, pal, 0);
    sf_block* next = (void*) block + block_size;
    next->prev_footer = block->header;
    block = coalesce(block);
    get_block_size(block);
    next = (void*) block + block_size;
    next->prev_footer = block->header;
    free_list_add(block);
    update_curr_pld();
    if (curr_pld > max_pld) {max_pld = curr_pld;}
}

void *sf_realloc(void *pp, sf_size_t rsize) {
    int val = is_valid(pp);
    if (val == 0) {abort();}
    sf_block* block = pp-16;
    int block_size = get_block_size(block);
    int pal =  (block->header ^ MAGIC) & PREV_BLOCK_ALLOCATED;
    if (rsize == 0) {
        sf_free(block);
        sf_errno = EINVAL;
        update_curr_pld();
        if (curr_pld > max_pld) {max_pld = curr_pld;}
        return NULL;
    }

    sf_block* new_block;
    if (block_size < rsize) {
        new_block = (void*) sf_malloc(rsize) - 16;
        memcpy((void*)new_block+16, pp, rsize);
        sf_free((void*)block + 16);
    }
    if (block_size > rsize) {
        if (block_size - rsize <= 32) { 
            init_header(block, rsize, block_size, 1, pal, 0); 
            update_curr_pld();
            if (curr_pld > max_pld) {max_pld = curr_pld;}
            return block->body.payload;
        }
        else {
            int size = rsize;
            if (size+8 <= 32) {size = 32;}
            else if ((size+8) % 16 != 0) {size = ((size+8) + (16-((size+8)%16)));}
            else if ((size+8) % 16 == 0) {size += 8;}
            else size+=8;
            debug("%d", size);
            debug("%d", rsize);
            init_header(block, rsize, size, 1, pal, 0);    //make the allocated block's header 
            //split
            sf_block* free_block = (void*)block + size;     //the free block's address is the current block's address + its size
            block_size -= size;          //free block's size decreased since part of it was allocated, need to decrease it by allocated block size
            init_header(free_block, 0, block_size, 0, 1, 0);
            coalesce(free_block);                             //add the new free block to its appropriate free list
            free_list_add(free_block);
            update_curr_pld();
            if (curr_pld > max_pld) {max_pld = curr_pld;}
            return block->body.payload;
        }
    }
    update_curr_pld();
    if (curr_pld > max_pld) {max_pld = curr_pld;}
    return new_block;
}

double sf_internal_fragmentation() {
    double infr = 0.0;
    if (sf_mem_start == sf_mem_end) {return infr;}
    sf_block* block = sf_mem_start() + 32;
    sf_block* end = sf_mem_end() - 16;
    double tot_pld = 0, tot_al = 0;
    while (block != end)  {
        int al = (block->header ^ MAGIC) & THIS_BLOCK_ALLOCATED;
        int qkl = (block->header ^ MAGIC) & IN_QUICK_LIST;
        if (al &&! qkl) {
            tot_pld += (block->header ^ MAGIC) >> 32;
            tot_al += get_block_size(block);
        }
        block = (void*)block + get_block_size(block);
    }
    if(tot_al == 0) {return 0;}
    infr = tot_pld/tot_al;
    return infr;
}

double sf_peak_utilization() {
    double pkut = 0.0;
    if (sf_mem_start == sf_mem_end) {return pkut;}
    double heap_size = sf_mem_end() - sf_mem_start();
    if (max_pld == 0) {return pkut;}
    pkut = max_pld/heap_size;
    return pkut;
}
