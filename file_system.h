#ifndef __FILE_SYSTEM_H__
#define __FILE_SYSTEM_H__

#include "mydisk.h"

#define SUPERBLOCK 0
#define ROOTBLOCK 1
#define FREEBLOCK 2

typedef struct Inode{
  int size;
  int ** pointers;
  int nextInodePointer;
} Inode;

Inode* readInode(disk_t disk, int block);

void writeInode(disk_t disk, int block, Inode* inode);

// Inodes which represent directories should be size = 0, pointers will be to other inodes
// the max # of pointers is (BLOCK_SIZE-3-(ceil(log10(disk_size))*2)/(ceil(log10(disk_size))+1) 
// if you have more pointers than that, then it is your job to create an Inode with the remaining
// pointers, write the inode, and then give the block location of that inode to the previous Inode.
// Additional inodes should have size = to the number of blocks in them
Inode* createInode(int size, int ** pointers, int * block);

//do not write inode after freeing
void freeInode(Inode * inode);

void write_super_block(disk_t disk);

void read_super_block(disk_t disk);

void write_root_dir(disk_t disk);

void read_root_dir(disk_t disk);

void write_block_map(disk_t disk);

void read_block_map(disk_t disk);

#endif
