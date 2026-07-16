#include <stdio.h>

typedef struct {
    unsigned start;
    unsigned size;
    int free;
} Block;

static void print_blocks(const Block blocks[], unsigned count, const char *label) {
    printf("%s\n", label);
    for (unsigned i = 0; i < count; ++i) {
        printf("  block%u start=%u size=%u %s\n",
               i,
               blocks[i].start,
               blocks[i].size,
               blocks[i].free ? "free" : "used");
    }
}

int main(void) {
    Block blocks[4] = {
        { 0, 100, 1 },
        { 0, 0, 0 },
        { 0, 0, 0 },
        { 0, 0, 0 }
    };
    unsigned count = 1;

    print_blocks(blocks, count, "initial free list");

    blocks[0] = (Block){ 0, 40, 0 };
    blocks[1] = (Block){ 40, 60, 1 };
    count = 2;
    print_blocks(blocks, count, "after malloc task stack 40");

    blocks[1] = (Block){ 40, 30, 0 };
    blocks[2] = (Block){ 70, 30, 1 };
    count = 3;
    print_blocks(blocks, count, "after malloc queue storage 30");

    blocks[0].free = 1;
    print_blocks(blocks, count, "after free task stack 40");
    puts("malloc 48 fails: total_free=70 largest_free=40");

    blocks[1].free = 1;
    blocks[0] = (Block){ 0, 100, 1 };
    count = 1;
    print_blocks(blocks, count, "after free queue and coalesce adjacent blocks");

    puts("malloc 120 fails: largest free block is 100");
    return 0;
}
