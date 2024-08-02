#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include "mbr.h"

void usage() {
    fprintf(stdout, "syntax: ./filename1 filename2\n");
    fprintf(stdout, "sample: ./mbr.dd\n");
}

uint32_t swap_endian(uint32_t value) {
    return ((value >> 24) & 0xFF) |
           ((value >> 8) & 0xFF00) |
           ((value << 8) & 0xFF0000) |
           ((value << 24) & 0xFF000000);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "How to use: %s file1\n", argv[0]);
        usage();
        return -1;
    }

    FILE *fb1 = fopen(argv[1], "rb");
    if (fb1 == NULL) {
        perror("file open error");
        return -1;
    }

    uint8_t byte[512];
    if (fread(byte, 1, sizeof(byte), fb1) != sizeof(byte)) {
        perror("file read error");
        fclose(fb1);
        return -1;
    }

    struct BOOT_SEC *BS = (struct BOOT_SEC *)byte;
    struct Partition_Table_Entry *PTE = (struct Partition_Table_Entry *)(byte + sizeof(struct BOOT_SEC));

    fprintf(stdout, "<type>\t<addr> <size>\n");
    int i = 0;

    uint32_t start_sector;
    uint32_t size;
    uint32_t ebr_adr;
    uint32_t move_adr;
    int first_ebr = 0;
    int j = 0;
    while (true) {
        if (PTE[i].type == 0x07) {
            // printf("%x ", BS[0].jcbc[0]);
            // printf("%x ", BS[0].jcbc[1]);
            // printf("%x\n", BS[0].jcbc[2]);
            if (BS[0].jcbc[0] == 0x33 && BS[0].jcbc[1] == 0xc0 && BS[0].jcbc[2] == 0x8e) {
                fprintf(stdout, "NTFS\t");

                start_sector = (PTE[i].add[0] << 24) | (PTE[i].add[1] << 16) | (PTE[i].add[2] << 8) | PTE[i].add[3];
                start_sector = swap_endian(start_sector);
                fprintf(stdout, "%u\t", start_sector);

                size = (PTE[i].size[0] << 24) | (PTE[i].size[1] << 16) | (PTE[i].size[2] << 8) | PTE[i].size[3];
                size = swap_endian(size);
                fprintf(stdout, "%u\n", size);
            }

        } else if (PTE[i].type == 0x05 || PTE[j + 1].type == 0x05) {
            if (BS[0].jcbc[0] == 0x33 && BS[0].jcbc[1] == 0xc0 && BS[0].jcbc[2] == 0x8e) {
                
            }
            fprintf(stdout, "NTFS\t");

            if (first_ebr == 0) {
                start_sector = (PTE[i].add[0] << 24) | (PTE[i].add[1] << 16) | (PTE[i].add[2] << 8) | PTE[i].add[3];
                start_sector = swap_endian(start_sector);
                ebr_adr = start_sector;

                fprintf(stdout, "%u\t", (start_sector + PTE[i].add[0]));

                first_ebr += 1;

                start_sector = (PTE[j + 1].add[0] << 24) | (PTE[j + 1].add[1] << 16) | (PTE[j + 1].add[2] << 8) | PTE[j + 1].add[3];
                start_sector = swap_endian(start_sector);
                move_adr = start_sector;

                fseek(fb1, start_sector * 512, SEEK_SET);
            } else {
                fprintf(stdout, "%u\t", (move_adr + ebr_adr + PTE[j].add[0]));
                start_sector = (PTE[j + 1].add[0] << 24) | (PTE[j + 1].add[1] << 16) | (PTE[j + 1].add[2] << 8) | PTE[j + 1].add[3];
                start_sector = swap_endian(start_sector);
                move_adr = start_sector;

                fseek(fb1, (move_adr + ebr_adr) * 512, SEEK_SET);
            }

            size = (PTE[j].size[0] << 24) | (PTE[j].size[1] << 16) | (PTE[j].size[2] << 8) | PTE[j].size[3];
            size = swap_endian(size);
            fprintf(stdout, "%u\n", size);
            j += 1;

            start_sector = (PTE[j].add[0] << 24) | (PTE[j].add[1] << 16) | (PTE[j].add[2] << 8) | PTE[j].add[3];
            start_sector = swap_endian(start_sector);

            fseek(fb1, (move_adr + ebr_adr) * 512, SEEK_SET);
            j -= 1;

            fread(byte, 1, sizeof(byte), fb1) != sizeof(byte);
        } else {
            fprintf(stdout, "NTFS\t");
            fprintf(stdout, "%u\t", (move_adr + ebr_adr + PTE[j].add[0]));
            start_sector = (PTE[j + 1].add[0] << 24) | (PTE[j + 1].add[1] << 16) | (PTE[j + 1].add[2] << 8) | PTE[j + 1].add[3];
            start_sector = swap_endian(start_sector);

            size = (PTE[j].size[0] << 24) | (PTE[j].size[1] << 16) | (PTE[j].size[2] << 8) | PTE[j].size[3];
            size = swap_endian(size);
            fprintf(stdout, "%u\n", size);
            break;
        }
        i += 1;
    }

    fclose(fb1);
    return 0;
}
