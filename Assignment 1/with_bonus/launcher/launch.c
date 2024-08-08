#include "../loader/loader.h"

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
