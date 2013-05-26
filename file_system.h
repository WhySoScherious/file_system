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

//disk: disk to write to
//block: block to write Inode
//pointers: list of pointers
//directory: whether it's a directory Inode or not.
Inode* writeInode(disk_t disk, int block, int ** pointers);

Inode* reWriteInode(disk_t disk, int block, Inode * inode);

//do not write inode after freeing
void freeInode(Inode * inode);

void write_super_block(disk_t disk);

void read_super_block(disk_t disk);

void write_root_dir(disk_t disk);

void read_root_dir(disk_t disk);

void write_block_map(disk_t disk);

void read_block_map(disk_t disk);

#endif
