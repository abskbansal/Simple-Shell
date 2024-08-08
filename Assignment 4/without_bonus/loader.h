#include <stdio.h>
#include <elf.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <signal.h>
#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#define PAGE_SIZE 4096
#define MAX_SIZE 200

typedef struct {
    void* segment;
    double size;
    size_t no_of_pages;
} Segment;

typedef struct {
    Elf32_Phdr unloaded_name ;
    int remaining_size;
    uintptr_t vaddr;
    int main_offset ;
    int var_offset ;
} Unloaded_Segments;

typedef struct {
    int page_faults;
    int page_allocations;
    int internal_fragmentation;
} PageData;

void load_and_run_elf(char** exe);
void loader_cleanup();
