#include "loader.h"

Elf32_Ehdr *ehdr;
Elf32_Phdr *phdr;
int fd;

/*
 * release memory and other cleanups
 */
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

    // ehdr = (Elf32_Ehdr *)malloc(sizeof(Elf32_Ehdr));
    ehdr = (Elf32_Ehdr *)malloc(sizeof(Elf32_Ehdr));
    // printf("ge\n");

    // 1. Load entire binary content into the memory from the ELF file.
    int s = read(fd, ehdr, sizeof(Elf32_Ehdr));
    if (s != sizeof(Elf32_Ehdr)) {
        printf("Error occured while reading the file\nExiting...\n");
        exit(EXIT_FAILURE);
    }
    // for (int j = 0 ;j<100;j++){
    //
    //   if (ehdr->e_ident[j]==0X7F){
    //     printf("ELF\n");
    //   }
    //   else{
    //     printf("false\n");
    //   }
    //
    //   printf("%c\n",ehdr->e_ident[j]) ;
    // }
    // printf("tg\n");

    if (ehdr->e_phnum == 0) {
        printf("Missing program header\nExiting...\n");
        exit(EXIT_FAILURE);
    }

    // phdr=(Elf32_Phdr *) malloc(sizeof(Elf32_Phdr));
    phdr = (Elf32_Phdr *)malloc(ehdr->e_phentsize * ehdr->e_phnum);
    // printf("g");

    lseek(fd, ehdr->e_phoff, SEEK_SET);
    read(fd, phdr, (ehdr->e_phnum) * (ehdr->e_phentsize));

    // phdr=(Elf32_Phdr *)((uintptr_t)ehdr->e_phoff);//skip elf header
    //  printf("phdr\n");
    // Elf32_Ehdr phdr_found =(Elf32_Phdr *) malloc(ehdr->e_phentsize * ehdr->e_phnum);
    // printf("%c\n",ehdr->e_ident[2]);
    // printf("%d",(int)ehdr->e_type);
    // printf("LL");

    for (int i = 0; i < ehdr->e_phnum; i++) {
        // printf("%u\n",phdr[i].p_vaddr);
        // printf("%u\n",phdr[i].p_memsz);
        // printf("%u\n",ehdr->e_entry);
        // printf("\n");
        if (phdr[i].p_type == PT_LOAD) {
            // printf("if working\n");
            // printf("hello131\n");

            if ((phdr[i].p_vaddr) <= (ehdr->e_entry)) {
                if ((ehdr->e_entry) <= ((phdr[i].p_vaddr) + (phdr[i].p_memsz))) {
                    // printf("working2\n");
                    // printf("working3\n");
                    // printf("working4\n");

                    void *segment1 = mmap((void *)(uintptr_t)phdr[i].p_vaddr, phdr[i].p_memsz, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_FIXED, fd, phdr[i].p_offset);
                    /*for (int j = 0 ; j<phdr->p_filesz ;i++){

                      if(*((uintptr_t*)(segment+j)))
                    }
                    */

                    if (segment1 == MAP_FAILED) {
                        printf("Error mapping memory\nExiting...\n");
                        exit(EXIT_FAILURE);
                    }
                    break;
                }
            }
        }

        // printf("hl");
    }
    close(fd);

    // printf("working5\n");

    // void *segment = malloc(phdr_found.p_vaddr);
    int (*_start)(void) = (int (*)(void))(ehdr->e_entry);

    // printf("new check\n");
    _start();
    int result = _start();
    // loader_cleanup()

    printf("User main return value = %d\n", result);

    // printf("%ld\n",(uintptr_t)(*_start));
    // printf("****************************\n");

    // printf("working6\n");

    // printf("working7\n");

    // close(fd);

    // printf("working8\n");
    // int(*number_of_phdr)() = (int(*)())(ehdr->e_phnum);
    // void *function_address = (void *)((uintptr_t)phdr_found.p_vaddr + (ehdr.e_entry - phdr_found.p_offset));
    // printf("working9\n");
    // 5. Typecast the address to that of function pointer matching "_start" method in fib.c.
    // int (*_start)() = (int(*)())(function_address);
    // printf("working10\n");

    // printf("working11\n");
    // int result = _start();
    // printf("working12\n");

    // printf("User main return value = %d\n", _start());
    //  1. Load entire binary content into the memory from the ELF file.
}

int main(int argc, char **argv) {
    Elf32_Ehdr ehdr1;
    int fd1;
    fd1 = open((&argv[1])[0], O_RDONLY);

    read(fd1, &ehdr1, sizeof(Elf32_Ehdr));

    // printf("he\n");
    if (argc != 2) {
        printf("Usage: %s <ELF Executable> \n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // 1. carry out necessary checks on the input ELF file
    if (memcmp(ehdr1.e_ident, ELFMAG, SELFMAG) == 0) {

        // printf("this is a valid elf file\n");
        // 2. passing it to the loader for carrying out the loading/execution
        load_and_run_elf(argv);
        // 3. invoke the cleanup routine inside the loade
        loader_cleanup();
    } else {
        printf("This is not a valid ELF file\nExiting....\n");
        exit(EXIT_FAILURE);
    }

    return 0;
}
