#ifndef __FILE_SYSTEM_H__
#define __FILE_SYSTEM_H__
#include "mydisk.h"

#define SUPERBLOCK 0
#define ROOTBLOCK 1
#define FREEBLOCK 2

#define bool int
#define true 1
#define false 0

typedef struct Inode{
  int size;
  int * pointers;
  int nextInodePointer;
  bool isDirectory;
  int block;
  char * name;
} Inode;

struct {
   unsigned int rootblock,freeblock,datablock,size;
} superblock;


//block location on disk of Inode.
Inode* readInode(disk_t disk, int block);

//disk: disk to write to
//block: block to write Inode
//pointers: list of pointers
//directory: whether it's a directory Inode or not.
//Inode must be freed.
//name must be less than 16 chars
Inode* writeInode(disk_t disk, int block, int * pointers,bool directory,char * name);

//will completely remove, and then write the updated Inode in the old location
Inode* rewriteInode(disk_t disk, Inode* inode);
 
//will safely remove Inode
void deleteInode(disk_t disk, Inode* inode);

//Conveniency Method, frees pointers, and then Inode
void freeInode(Inode * inode);

void write_super_block(disk_t disk);

void read_super_block(disk_t disk);

void write_root_dir(disk_t disk);

void read_root_dir(disk_t disk);

void write_block_map(disk_t disk, int * bmap);

int * read_block_map(disk_t disk);

#endif
