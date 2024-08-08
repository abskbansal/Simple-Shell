#include "loader.h"
#include <signal.h>
#include <stdio.h>
#include <sys/types.h>

char** argv1 ;
Elf32_Ehdr *ehdr;
Elf32_Phdr *phdr;
int fd;
struct sigaction sa;
int t = 0 ;

Elf32_Phdr unloaded_segments[MAX_SIZE];
Segment *segments[MAX_SIZE];
Unloaded_Segments segments_info[MAX_SIZE] ;
PageData page_data = {0, 0, 0};
int no_of_segments_loaded = 0 ;

int counter = 0;

void segment_loader(){

    t = 0;
    // printf("fefee\n");
    for (int i = 0; i < ehdr->e_phnum; i++) {
            
            unloaded_segments[t] = phdr[i];
            segments_info[t].unloaded_name = phdr[i];
            segments_info[t].remaining_size = phdr[i].p_memsz;
            segments_info[t].vaddr = phdr[i].p_vaddr;
            segments_info[t].main_offset = phdr[i].p_offset ;
            segments_info[t].var_offset = 0;
            segments_info[t].memsz = phdr[i].p_memsz;
            t++ ;


    }

}
int search_segment(unsigned long fault_address){
    // printf("h1\n");
    for (int i = 0 ; i<t;i++){
        // printf("remaining size: %d\n" ,segments_info[i].remaining_size);
        // printf("starting: %u\n" ,segments_info[i].vaddr);
        // printf("ending: %u\n" ,segments_info[i].vaddr + segments_info[i].memsz);
        if (segments_info[i].remaining_size > 0 && segments_info[i].vaddr <= fault_address && fault_address <= segments_info[i].vaddr + segments_info[i].memsz ){
            return i ;
        }

    }
    
    segment_loader();

    return 0 ;
    

}
void load_segment_in_parts(unsigned long fault_address) {
    // printf("faulty address: %lu\n", fault_address);
    //printf("h3\n");
    int position = search_segment(fault_address) ;
    //printf("h4\n");
    //printf("position: %d\n", position);

    

    //printf("this is for position 3 starting: %u\n", segments_info[3].vaddr );
    //printf("this is for position 3 ending: %u\n", segments_info[3].vaddr +segments_info[3].memsz );
    Unloaded_Segments found_segment = segments_info[position];
    //printf("h5");
    
    //printf("this is the v_addr: %u\n", found_segment.vaddr);
    //printf("this is the page size starting: %u\n", found_segment.vaddr);
    
    // printf("%d\n", found_segment.remaining_size);
    int offset = found_segment.var_offset;
    int remaining_size = found_segment.remaining_size;
    uintptr_t vaddr = found_segment.vaddr;

    
    int chunk_size = (remaining_size > PAGE_SIZE) ? PAGE_SIZE : remaining_size;

    //printf("This is the page ending: %u\n" , found_segment.vaddr+chunk_size);

    lseek(fd, found_segment.main_offset + offset, SEEK_SET);
    
    //printf("hrehre\n");
    
    if ((found_segment.vaddr <= fault_address && fault_address < found_segment.vaddr + chunk_size)){
        // printf("h1\n");
        uintptr_t page_start = vaddr & ~(PAGE_SIZE - 1);

        void *segment_part = mmap((void *)page_start, PAGE_SIZE, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);

        
        if (segment_part == MAP_FAILED) {
            perror("mmap");
            exit(EXIT_FAILURE);
        }

        segments[no_of_segments_loaded] = (Segment*)malloc(sizeof(Segment));
        if (segments[no_of_segments_loaded] == NULL) {
            perror("malloc in segment array");
            exit(EXIT_FAILURE);
        }

        segments[no_of_segments_loaded]->segment = segment_part;
        segments[no_of_segments_loaded]->size = chunk_size;
        segments[no_of_segments_loaded]->no_of_pages = 1;
        
        
        page_data.page_faults++;
        page_data.page_allocations++;
        
        // printf("chunk size: %d\n" , chunk_size) ;
        if (read(fd, segment_part, chunk_size) < 0) {
            perror("read");
            exit(EXIT_FAILURE);
        }
    
        page_data.internal_fragmentation += PAGE_SIZE - chunk_size;
        no_of_segments_loaded++;
    
        
    }
    

    segments_info[position].var_offset += chunk_size;
    segments_info[position].remaining_size -= chunk_size;
    segments_info[position].vaddr += chunk_size;
   

    // printf("This is the internal fragmentation: %d\n", page_data.internal_fragmentation) ;
    

    

    // printf("h2\n");
    
}

void segfault_handler(int signo, siginfo_t *si, void *context){

    unsigned long fault_address = (unsigned long)si->si_addr;
    // printf("In this handler: \n");
    load_segment_in_parts(fault_address);

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
        // printf("h2\n");
        if (munmap(segments[i]->segment, PAGE_SIZE ) < 0) {
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

    t = 0;
    for (int i = 0; i < ehdr->e_phnum; i++) {
        
        
            // printf("%d\n", phdr[i].p_vaddr);
            // printf("This is the phdr[i].p_memsz :  %d\n",phdr[i].p_memsz);
            // printf("This is the offset of the entry:  %d\n", ehdr->e_entry - phdr[i].p_memsz);
            
            unloaded_segments[t] = phdr[i];
            segments_info[t].unloaded_name = phdr[i];
            segments_info[t].remaining_size  = phdr[i].p_filesz ;
            segments_info[t].vaddr = phdr[i].p_vaddr;
            segments_info[t].main_offset = phdr[i].p_offset ;
            segments_info[t].var_offset = 0;
            segments_info[t].memsz = phdr[i].p_memsz;
            t++ ;


    
            // load_segment_in_parts(i);
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
    argv1  = argv;
    int fd1;
    fd1 = open((&argv[1])[0], O_RDONLY);

    read(fd1, &ehdr1, sizeof(Elf32_Ehdr));

    // printf("he\n"); 
    if (argc != 2) {
        printf("Usage: %s <ELF Executable> \n", argv[0]);
        exit(EXIT_FAILURE);
    }

    setup_segfault_handler();

    if (memcmp(ehdr1.e_ident, ELFMAG, SELFMAG) == 0) {

        load_and_run_elf(argv);
   
        loader_cleanup();
    } else {
        printf("This is not a valid ELF file\nExiting....\n");
        exit(EXIT_FAILURE);
    }

    return 0;
}
