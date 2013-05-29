#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include "file_system.h"
//#include "mydisk.h"

#define BITMASK 255

struct {
   unsigned int rootblock,freeblock,datablock,size;
} superblock;

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
   if(loc < sizeof(databuf)/sizeof(unsigned char))
		databuf[loc] = '\0';
}

/* Place a larger number up to 32 bits into buffer at location loc where e
 * is the value.
*/
void emplace_buf(unsigned char *databuf, unsigned int e, unsigned int bytes, unsigned int loc){
   int i;
   //for(i=8*(--bytes);i>=0;i-=8,++loc) databuf[loc] = (e&(BITMASK<<i))>>i; //error: âBITMASKâ undeclared (first use in this function)
}

unsigned int read_buf(unsigned char *databuf, unsigned int bytes, unsigned int loc){
   unsigned int e=0;
   for(;bytes>0;--bytes,++loc,e<<=8) e+=databuf[loc];
   return e>>8;
}

void read_super_block(disk_t disk){
   printf("Reading super block...");
   unsigned char *databuf = malloc(disk->block_size);
   readblock(disk,SUPERBLOCK,databuf);
   superblock.size = read_buf(databuf,4,0);
   superblock.rootblock = databuf[4];
   superblock.freeblock = databuf[5];
   superblock.datablock = databuf[6];
   printf("%d %d %d %d",superblock.size,superblock.rootblock,
                        superblock.freeblock,superblock.datablock);
   printf("\n");
}

void write_super_block(disk_t disk){
   printf("Writing super block...");
   unsigned char *databuf = malloc(disk->block_size);
   //Info 0-3:SIZE 4:RDIR 5:FBM 6:DATA
   emplace_buf(databuf,disk->size,4,0);
   databuf[4] = ROOTBLOCK;
   databuf[5] = FREEBLOCK;
   databuf[6] = '?';
   writeblock(disk,SUPERBLOCK,databuf);
   free(databuf);
   printf("\n");
}

// Inodes which represent directories should be size = 0, pointers will be to other inodes
// the max # of pointers is (BLOCK_SIZE-3-(ceil(log10(disk_size))*2)/(ceil(log10(disk_size))+1) 
// if you have more pointers than that, then it is your job to create an Inode with the remaining
// pointers, write the inode, and then give the block location of that inode to the previous Inode.
// Additional inodes should have size = to the number of blocks in them
Inode* createInode(int size, int * pointers,bool directory, int block){
	Inode* node = malloc(sizeof(Inode));
	node->size = size;
	node->pointers = pointers;
	node->isDirectory = directory;
	node->block = block;
	return node;
}

//turns a list of ints into a char array of numbers separated by commas. Will end in a comma.
//needs to be freed
char * intArray2charArray(int * numbers){
	char * str = malloc(sizeof(char) * 512);
	int num = numbers[0];
	int i = 0;
	while(num != '\0'){
		char * num2str = int2str(num);
		strcat(str,int2str(num));
		free(num2str);
		strcat(str,",");
		num = numbers[i+1];
		i +=1;
	}
	char *s = malloc(sizeof(char) * length(str));
	for(i = 0; str[i] != '\0';i+=1){
		s[i] = str[i];
	}
	
	s[i] = '\0'; //cpy '\0';
	free(str);
	return  s;
}

Inode* writeInode(disk_t disk, int block, int * gpointers,bool directory){
	int size = sizeof(gpointers)/sizeof(int);
	int * pointers = malloc(sizeof(int) * size);
	int i;
	for(i = 0; i < size; i+=1){
		pointers[i] = gpointers[i];
	}
	if(size < (disk->block_size-2-(ceil(log10(disk->size))*2)/(ceil(log10(disk->size))+1))){
		unsigned char *strings[4];
		strings[0] = (directory)? "0" : int2str(size);
		char * list = intArray2charArray(pointers);
		char * pntrs = malloc(sizeof(char) * (length(list)+1));
		pntrs[0] = '\n';
		pntrs[1] = '\0';
		strcat(pntrs,list);
		strings[1] = pntrs;
		strings[2] = "\n";
		strings[3] = NULL;
		unsigned char *databuf = malloc(disk->block_size);
		copy2buf(databuf,strings,0);
		writeblock(disk,block,databuf);
		free(databuf);
		free(list);
		free(pntrs);
	}else{
		//need to read from free block map to find more room to put extra Inode
	}
	return createInode(size,pointers,directory,block);
}

void freeInode(Inode * inode){
	free(inode->pointers);
	free(inode);
}

void write_root_dir(disk_t disk){
   printf("Writing root directory...");
   printf("\n");
}

void write_block_map(disk_t disk, int * bmap){
   printf("Writing free block map...");
   printf("\n");
   int i;
   char *disk_name;
   unsigned char *databuf;
   /* disk with n blocks requires a bitmap with n bits
   bitmap size is enough blocks to hold n bits, starts at block 2 */
   int *bitmap;
   
   disk = opendisk(disk_name);
   
   
   databuf = malloc(sizeof(char) * disk->block_size);

   
   /*Initially every entry in bitmap is 0(unsused) 
   Chase: This is not true: the superblock, rootdir and bitmap blocks should all be filled with 1s. 
   To make this function work in more than one case it should just write the bmap given, 
   and the user can fill it with 0s and 1s to start*/
	//i = block number
   int bitmapindex = 0;
   for(i = 2; bmap[bitmapindex] != '\0';i+=1){
	   int bufindex = 0;
	   while(bufindex < disk->block_size && bmap[bitmapindex] != '\0'){
			databuf[bufindex] = (bmap[bitmapindex] == 0)? '0' : '1';
			bufindex +=1;
			bitmapindex +=1;
	   }
	   if(bmap[bitmapindex] == '\0' && bufindex < disk->block_size){
			databuf[bufindex] = '\0'; //terminate
	   }
	   writeblock(disk,i,databuf);
   }
   free(databuf);
   
/* include the superblock, root directory, and bitmap blocks in the bitmap. 
That will make the block numbers correspond to the blocks on your "disk".*/

/*mark superblock and root dir as used*/



}
