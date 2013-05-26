#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <math.h>
#include "file_system.h"
//#include "mydisk.h"

#define SUPERBLOCK 0
#define ROOTBLOCK 1
#define FREEBLOCK 2

/*Converts a decimal number into a binary string
 * Takes a number n
*/
char *int2binstr(unsigned int n){
   int w = ceil(log2(n));
   int i;
   char *s = malloc(sizeof(char)*w);
   for(i=0;w>=0;++i,--w) s[i] = ((n&(1<<w))>0)?'1':'0';
   return s;
}

/* Converts a decimal number (max 64 digits long) into a string
*/
char *int2str(int n){
   int i,j;
   char buf[64];
   for(i=0;n>0;++i,n/=10) buf[i] = n%10+48;
   char *string = malloc(sizeof(char)*i--);
   for(j=0;i>=0;--i,++j) string[j] = buf[i];
   return string;
}

/* Converts a character string of decimal numbers to an int
*/
int str2int(char *s){
   int i,n;
   for(i=n=0;s[i]!='\0';++i,n*=10) n+=s[i]-48;
   return n/10;
}

/* Returns the length of a string
*/
int length(char *s){
  int i;
  for(i=0;s[i]!='\0';++i);
  return i;
}

/* Copies a stringa of arbitrary length into a fixed length databuffer.
 * Pass in a buffer that has already been malloc'd to block size:
 *    unsigned char *databuf = malloc(disk->block_size);
 * The second is an array of strings to be copied where
 * THE LAST STRING MUST BE A NULL!!!!!
 * The final parameter represents how many bytes into the buffer you
 * wish to put your string.
 * See write_super_block for an example.
*/
void copy2buf(unsigned char *databuf, unsigned char **strings, int loc){
   int i,j;
   for(i=0;strings[i]!=NULL;++i)
      for(j=0;strings[i][j]!='\0';++j,++loc) databuf[loc] = strings[i][j];
}

void write_super_block(disk_t disk){
   printf("Writing super block...");
   unsigned char *databuf = malloc(disk->block_size);
   int i=0;
   unsigned char *strings[10];
   strings[0] = "\nSuper Block Starts Here:\nSize: ";
   strings[1] = int2str(disk->size);
   strings[2] = "\nLocations:\n   Free Block Map: ";
   strings[3] = int2str(FREEBLOCK);
   strings[4] = "\n   Root Directory: ";
   strings[5] = int2str(ROOTBLOCK);
   strings[6] = "\n   Data Block: ";
   strings[7] = "?";
   strings[8] = "\n";
   strings[9] = NULL;
   copy2buf(databuf,strings,0);
   writeblock(disk,SUPERBLOCK,databuf);
   printf("\n");
}

Inode* createInode(int size, int ** pointers, int * block){
   Inode* node = malloc(sizeof(Inode));
	node->size = size;
	node->pointers = pointers;
	node->nextInodePointer = block;

}

void write_root_dir(disk_t disk){
   printf("Writing root directory...");
   printf("\n");
}

void write_block_map(disk_t disk){
   printf("Writing free block map...");
   printf("\n");
}
