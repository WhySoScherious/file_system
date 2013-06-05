#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include "file_system.h"
//#include "mydisk.h"



#define SUPERBLOCK 0
#define ROOTBLOCK 1
#define FREEBLOCK 2
#define BITMASK 255

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
   char *string = malloc(sizeof(char)*i+1);
   i -=1;
   for(j=0;i>=0;--i,++j) string[j] = buf[i];
   string[j] = '\0';
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
		databuf[loc] = '\0';
}

/* Place a larger number up to 32 bits into buffer at location loc where e
 * is the value.
*/
void emplace_buf(unsigned char *databuf, unsigned int e, unsigned int bytes, unsigned int loc){
   int i;
   for(i=8*(--bytes);i>=0;i-=8,++loc) databuf[loc] = (e&(BITMASK<<i))>>i; //error: âBITMASKâ undeclared (first use in this function)
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
   free(databuf);
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
   databuf[6] = ceil(double(disk->size)/double(disk->block_size))+1+FREEBLOCK;
   writeblock(disk,SUPERBLOCK,databuf);
   free(databuf);
   printf("\n");
}

// Inodes which represent directories should be size = 0, pointers will be to other inodes
// the max # of pointers is (BLOCK_SIZE-3-(ceil(log10(disk_size))*2)/(ceil(log10(disk_size))+1) 
// if you have more pointers than that, then it is your job to create an Inode with the remaining
// pointers, write the inode, and then give the block location of that inode to the previous Inode.
// Additional inodes should have size = to the number of blocks in them
Inode* createInode(int size, int * pointers,bool directory, int block, char * name){
	Inode* node = malloc(sizeof(Inode));
	node->size = size;
	node->pointers = pointers;
	node->isDirectory = directory;
	node->block = block;
        node->name = name;
	return node;
}

//turns a list of ints into a char array of numbers separated by commas. Will end in a comma.
//needs to be freed
char * intArray2charArray(int * numbers){
	char * str = malloc(sizeof(char) * 512);
	str[0] = '\0';
	int num = numbers[0];
	int i = 0;
	while(num != '\0'){
		char * num2str = int2str(num);
		strcat(str,num2str);
		free(num2str);
		strcat(str,",");
		num = numbers[i+1];
		i +=1;
	}
	
	char *s = malloc(sizeof(char) * (length(str)+1));
	
	for(i = 0; str[i] != '\0';i+=1){
		s[i] = str[i];
	}
	
	s[i] = '\0'; //cpy '\0';
	
	free(str);
	return  s;
}

int arrayLength(int * intarray){
	int i;
	for(i = 0; intarray[i] != '\0';i+=1);
	return i;
}

Inode * rewriteInode(disk_t disk, Inode* inode){
	deleteInode(disk,inode);
	Inode* newinode = writeInode(disk,inode->block,inode->pointers,inode->isDirectory,inode->name);
	freeInode(inode);
	return newinode;
}

void deleteInode(disk_t disk, Inode* inode){
	  char * databuf = malloc(sizeof(char) * disk->block_size);
  readblock(disk,inode->block,databuf);
  int i;
  bool gotsize = false;
  bool gotpointers = false;
  bool gotlink = false;
  int numberbufIndex = 0;
  char * numberbuf = malloc(sizeof(char) * 16);
  for(i = 0; databuf[i] != '\0' && i < disk->block_size;i+=1){
    if(!gotsize){
      if(databuf[i] == '\n'){
        gotsize = true;
      }
    }else if(!gotpointers){
      if(databuf[i] == '\n'){
        gotpointers = true; 
      }
    }else{
      gotlink = true;
      numberbuf[numberbufIndex] = databuf[i];
      numberbufIndex +=1;
    }
  }
  int link;
  if(gotlink){
    numberbuf[numberbufIndex] = '\0';
    link = str2int(numberbuf);
	Inode * newinode = readInode(disk,link);
	deleteInode(disk,newinode);
	freeInode(newinode);
  }
     //TODO: need to write to freeblock map.
}

void write_root_dir(disk){
  int * pointers = {};
  Inode * node = writeInode(disk,1,pointers,true,"root");
  freeInode(node);
}

//max name size = 15
Inode* writeInode(disk_t disk, int block, int * gpointers,bool directory, char * nametemp){
  
	int size = arrayLength(gpointers);
	int * pointers = malloc(sizeof(int) * (size+1));
	int i;
        int strlength = length(nametemp);
        char * name = malloc(sizeof(char)*  16);
        for(i = 0; i < 15 && nametemp[i] != '\0';i+=1){
	  name[i] = nametemp[i];
	}
        name[i] = '\0';
	for(i = 0; i < size; i+=1){
		pointers[i] = gpointers[i];
	}
	pointers[i] = '\0';
	char * potSize;
	if(size < (disk->block_size-20-(ceil(log10(disk->size))*2)/(ceil(log10(disk->size))+1))){ //-4 for 3 \n and 1 \0 - 16 for name length
		unsigned char *strings[6];
		potSize = int2str(size);
		strings[0] = (directory)? "0" : potSize;
		
		char * list = intArray2charArray(pointers);
		char * pntrs = malloc(sizeof(char) * (length(list)+2));
		pntrs[0] = '\n';
		
		pntrs[1] = '\0';
		strcat(pntrs,list);
		strings[1] = pntrs;
		strings[2] = "\n";
                strings[3] = name;
                strings[4] = "\n";
		strings[5] = NULL;
		unsigned char *databuf = calloc(disk->block_size, sizeof(unsigned char) );
		copy2buf(databuf,strings,0);
		writeblock(disk,block,databuf);
		free(databuf);
		free(list);
		free(pntrs);
		free(potSize);
	}else{
		//need to read from free block map to find more room to put extra Inode
	}
	return createInode(size,pointers,directory,block,name);
}

Inode* readInode(disk_t disk, int block){
  char * databuf = malloc(sizeof(char) * disk->block_size);
  printf("reading block %d\n",block);
  readblock(disk,block,databuf);
  int size;
  int i,j;
  char * name;
  bool gotsize = false;
  bool gotpointers = false;
  bool gotlink = false;
  bool gotname = false;
  int numberbufIndex = 0;
  char * numberbuf = malloc(sizeof(char) * 16);
  int * pointers = malloc(sizeof(int) * disk->size);
  int pointersIndex = 0;
  for(i = 0; databuf[i] != '\0' && i < disk->block_size;i+=1){
    if(!gotsize){
      if(databuf[i] == '\n'){
        numberbuf[numberbufIndex] = '\0';
        size = str2int(numberbuf);
		numberbufIndex = 0;
        gotsize = true;
      }else{
        numberbuf[numberbufIndex] = databuf[i];
        numberbufIndex +=1;
      }
    }else if(!gotpointers){
      if(databuf[i] == '\n'){
        gotpointers = true; 
        numberbuf[0] = '\0';
        numberbufIndex = 0;
      }else{
        if(databuf[i] == ','){
	  numberbuf[numberbufIndex] = '\0';
          pointers[pointersIndex] = str2int(numberbuf);
	  numberbufIndex = 0;
          pointersIndex +=1;
	}else{
          numberbuf[numberbufIndex] = databuf[i];
          numberbufIndex +=1;
	}
      }
    }else if(!gotname){
      if(databuf[i] == '\n'){
        name = malloc(sizeof(char) * numberbufIndex);
        for(j = 0; j < numberbufIndex; j+=1){
          name[j] = numberbuf[j];
	}
	gotname = true;
        numberbufIndex = 0;
      }else{
        numberbuf[numberbufIndex]= databuf[i];
        numberbufIndex +=1;
      }
    }
    else{
      gotlink = true;
      numberbuf[numberbufIndex] = databuf[i];
      numberbufIndex +=1;
    }
  }
  int link;
  if(gotlink){
    numberbuf[numberbufIndex] = '\0';
    link = str2int(numberbuf);
	Inode * newinode = readInode(disk,link);
	for(i = 0; newinode->pointers[i] != '\0';i+=1){
		pointers[pointersIndex] = newinode->pointers[i];
		pointersIndex+=1;
	}
	freeInode(newinode);
  }
  int * pointersSmall = malloc(sizeof(int) * (pointersIndex+1));
  for(i = 0; i < pointersIndex;i+=1){
	  printf("pointers = %d ",pointers[i]);
    pointersSmall[i] = pointers[i];
  }
  pointersSmall[i] = '\0';
  printf("\n");
  Inode * node = createInode(size,pointersSmall,(size ==0)? true : false, block,name);
  free(numberbuf);
  free(databuf);
  free(pointers);
  //free(pointersSmall);
  return node;
  
}


void freeInode(Inode * inode){
	free(inode->pointers);
        free(inode->name);
	free(inode);
}

void write_root_dir(disk_t disk){
  int pointers[0];
  Inode * node = writeInode(disk,1,pointers,true,"root");
  freeInode(node);
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
   
   
   databuf = malloc(sizeof(char) * disk->block_size);

   
   /*Initially every entry in bitmap is 0(unsused) 
   Chase: This is not true: the superblock, rootdir and bitmap blocks should all be filled with 1s. 
   To make this function work in more than one case it should just write the bmap given, 
   and the user can fill it with 0s and 1s to start*/
	//i = block number
   int bitmapindex = 0;
   for(i = 2; bitmapindex < disk->size;i+=1){
	   int bufindex = 0;
	   while(bufindex < disk->block_size && bitmapindex < disk->size){
			databuf[bufindex] = (bmap[bitmapindex] == 0)? '0' : '1';
			bufindex +=1;
			bitmapindex +=1;
	   }
	   if(bitmapindex == disk->size && bufindex < disk->block_size){
			databuf[bufindex] = '\0'; //terminate
	   }
	   printf("wrote blockmap %s to %d\n",databuf,i);
	   writeblock(disk,i,databuf);

   }
   
   free(databuf);
   

}


/*should read in thsese blocks and return a list of ints representing 
each bit*/
int * read_block_map(disk_t disk)
{

  int i, j, L= 0;
  unsigned char *databuf = malloc(sizeof(char) * disk->block_size);
  int *buf = malloc(sizeof(int) * disk->size);

 
  for(i = 2; L < disk->size; i+=1){
    readblock(disk, i, databuf);
  
    for(j = 0; databuf[j] != '\0' && j < disk->block_size; j+=1){
 
     // buf[L] = str2int(databuf[j]); // instead of databuf[j], we should just have databuf since str2int takes in a char *
      buf[L] = (databuf[j] == '0')? 0 : 1;
	// printf("reading %d \n",str2int(data  
        L +=1;

    }
  }

  free(databuf);

 return buf; 
}
