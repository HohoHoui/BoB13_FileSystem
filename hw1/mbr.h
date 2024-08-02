#define BOOT_SECTOR 446
typedef struct BOOT_SEC{
    unsigned char jcbc[3];
    unsigned char bootsec[443];
} BOOT_SEC;

typedef struct Partition_Table_Entry{
    unsigned char nonused[4];
    unsigned char type;
    unsigned char nonused2[3];
    unsigned char add[4];
    unsigned char size[4];
} Partition_Table_Entry;
