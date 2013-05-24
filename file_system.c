#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include "file_system.h"
#include "mydisk.h"

#define SUPERBLOCK 0
#define ROOTBLOCK 1
#define FREEBLOCK 2


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

/* Copies a string of arbitrary length into a fixed length databuffer.
 * Pass in a buffer that has already been malloc'd to block size:
 *    unsigned char *databuf = malloc(disk->block_size);
 * The final parameter represents how many bytes into the buffer you
 * wish to put your string.
 * Only for test purposes.
*/
void copy2buf(unsigned char *databuf, unsigned char *string, int loc){
   int i;
   for(i=0;string[i]!='\0';++i,++loc) databuf[loc] = string[i];
}

void write_super_block(disk_t disk){
   printf("Writing super block...");
   unsigned char *databuf = malloc(disk->block_size);
   unsigned char *string = "\nSuper Block Starts Here:\nps512\n";
   copy2buf(databuf,string,0);
   writeblock(disk,SUPERBLOCK,databuf);
   printf("\n");
}

void write_root_dir(disk_t disk){

}

void write_block_map(disk_t disk){

}
