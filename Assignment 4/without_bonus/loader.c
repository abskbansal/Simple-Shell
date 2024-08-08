#include "loader.h"
#include <elf.h>
#include <stdint.h>

Elf32_Ehdr *ehdr;
Elf32_Phdr *phdr;
int fd;
struct sigaction sa;
Elf32_Phdr unloaded_segments[MAX_SIZE];
Segment *segments[MAX_SIZE];
PageData page_data = {0, 0, 0};
int no_of_segments_loaded = 0 ;


Elf32_Phdr searchSegment(uintptr_t fault_address){
    for (int i = 0 ; i<MAX_SIZE ;i++){
        if (phdr[i].p_vaddr <= fault_address && fault_address <= phdr[i].p_vaddr + phdr[i].p_memsz ){
            return phdr[i] ;
        }
    }
}
void segfault_handler(int signo, siginfo_t *si, void *context) {

    unsigned long fault_address = (unsigned long)si->si_addr;

    // printf("fault address: %lu\n", fault_address);
    // printf("Segmentation fault occured. I am here now.....\n");

    Elf32_Phdr segment_to_use = searchSegment(fault_address);

    uintptr_t page_start = ((uintptr_t)segment_to_use.p_vaddr) & ~(PAGE_SIZE - 1);
    
    

    // printf("%d\n" ,(int)page_start);
    double size = segment_to_use.p_memsz;
    // printf("%f\n" , size);

    int count = 0;


    if ((int)(size / PAGE_SIZE) * (PAGE_SIZE) == size){
        count = (size / PAGE_SIZE) ;
    }
    else{
        count = (size / PAGE_SIZE) + 1;
    }
    
    // printf("count : %d\n" , count);

    void *segment1 = mmap((void *)page_start, PAGE_SIZE * count, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED  , fd, 0);
    
    if (segment1 == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    segments[no_of_segments_loaded] = (Segment*) malloc(sizeof(Segment));
    if (segments[no_of_segments_loaded] == NULL) {
        perror("malloc in segment array");
        exit(EXIT_FAILURE);
    }

    segments[no_of_segments_loaded]->segment = segment1;
    segments[no_of_segments_loaded]->size = size;
    segments[no_of_segments_loaded]->no_of_pages = count;
    
    
    page_data.page_faults++;
    page_data.page_allocations += count;

    // printf("%d\n" ,count) ;
    // printf("the number of page faults in the handler is: %d\n", page_data.page_faults); 
    lseek(fd, segment_to_use.p_offset, SEEK_SET);
    if (read(fd, segment1, PAGE_SIZE * count) < 0) {
        perror("read");
        exit(EXIT_FAILURE);
    }

    
    page_data.internal_fragmentation +=  PAGE_SIZE * count - size ;
    // printf("this is internal fragmentation: %d\n" , page_data.internal_fragmentation );


    
    

    no_of_segments_loaded++;
    

    // // i am continuing the normal process from here .
    // signal(SIGSEGV, SIG_DFL); // Reset the handler

    //void *segment1 = mmap((void *)(uintptr_t)found_phdr.p_vaddr, found_phdr.p_memsz, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_FIXED, fd, found_phdr.p_offset);
    // loader_cleanup();
    // exit(EXIT_FAILURE);

}
void loader_cleanup() {
    if (ehdr != NULL)
    {
        free(ehdr);
        ehdr = NULL;
    }
    if (phdr != NULL)
    {
        free(phdr);
        ehdr = NULL;
    }

    for (int i=0; i<no_of_segments_loaded; i++) {
        if (munmap(segments[i]->segment, PAGE_SIZE * segments[i]->no_of_pages) < 0) {
            perror("munmap failed");
            exit(EXIT_FAILURE);
        }
        
        if (segments[i])
            free(segments[i]);
    }
}

/*
 * Load and run the ELF executable file
 */
void load_and_run_elf(char **exe) {
    fd = open(exe[1], O_RDONLY);

    if (fd == -1) {
        printf("Error occured in opening the file\nExiting...\n");
        exit(EXIT_FAILURE);
    }


    ehdr = (Elf32_Ehdr *)malloc(sizeof(Elf32_Ehdr));

    

    int s = read(fd, ehdr, sizeof(Elf32_Ehdr));
    if (s != sizeof(Elf32_Ehdr)) {
        printf("Error occured while reading the file\nExiting...\n");
        exit(EXIT_FAILURE);
    }
   

    if (ehdr->e_phnum == 0) {
        printf("Missing program header\nExiting...\n");
        exit(EXIT_FAILURE);
    }

    // phdr=(Elf32_Phdr *) malloc(sizeof(Elf32_Phdr));
    phdr = (Elf32_Phdr *)malloc(ehdr->e_phentsize * ehdr->e_phnum);
    // printf("g");

    lseek(fd, ehdr->e_phoff, SEEK_SET);
    if (read(fd, phdr, (ehdr->e_phnum) * (ehdr->e_phentsize)) < 0) {
        perror("read");
        printf("Error occurred while reading program headers\nExiting...\n");
        exit(EXIT_FAILURE);
    }

    int t = 0 ;
    for (int i = 0; i < ehdr->e_phnum; i++) {
            // printf("%d\n", phdr[i].p_vaddr);

            // printf("This is the phdr[i].p_memsz :  %d\n",phdr[i].p_memsz);
            // printf("This is the offset of the entry:  %d\n", ehdr->e_entry - phdr[i].p_memsz);
            unloaded_segments[t++] = phdr[i];

            // void *segment1 = mmap((void *)(uintptr_t)phdr[i].p_vaddr, phdr[i].p_memsz, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_FIXED, fd, phdr[i].p_offset);
                

                
                

            // if (segment1 == MAP_FAILED) {
            //     printf("Error mapping memory\nExiting...\n");
            //     exit(EXIT_FAILURE);
            // }
            //loader_cleanup();
                
        
        
    }

  
    // printf(" hello this before the seg fault comes!\n");

    int (*_start)(void) = (int (*)(void))(ehdr->e_entry);
    
    int result = _start();


    printf("User main return value = %d\n", result);

    printf("Total page faults: %d\n", page_data.page_faults);

    printf("Total page allocations: %d\n", page_data.page_allocations);

    printf("Total internal fragmentation: %f Kilobytes(in KB)\n", (double)page_data.internal_fragmentation/1024);
    close(fd);


}

void setup_segfault_handler() {
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = segfault_handler;
    // sigemptyset(&sa.sa_mask);

    if (sigaction(SIGSEGV, &sa, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char **argv) {
    Elf32_Ehdr ehdr1;
    int fd1;
    fd1 = open((&argv[1])[0], O_RDONLY);

    read(fd1, &ehdr1, sizeof(Elf32_Ehdr));

    
    setup_segfault_handler();
    // printf("he\n"); 
    if (argc != 2) {
        printf("Usage: %s <ELF Executable> \n", argv[0]);
        exit(EXIT_FAILURE);
    }


    if (memcmp(ehdr1.e_ident, ELFMAG, SELFMAG) == 0) {

        load_and_run_elf(argv);
   
        loader_cleanup();
    } else {
        printf("This is not a valid ELF file\nExiting....\n");
        exit(EXIT_FAILURE);
    }

    return 0;
}

