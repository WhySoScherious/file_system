#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
//#include "mydisk.h" //already in file_system.h
#include "file_system.h"

void readdisk(disk_t disk, int blocknum){
   read_super_block(disk);
   Inode *file = readInode(disk,blocknum);
   if(file->size == 0) {
      printf("Is a directory\n");
      return;
   }
   unsigned char *databuf = malloc(disk->block_size * (sizeof(unsigned char)));
   unsigned char *filebuf = malloc(disk->block_size * file->size * sizeof(unsigned char));
   int i,j,k;
   k = 0;
   for(i=0;i<file->size;++i,++blocknum){
      readblock(disk,blocknum,databuf);
      for(j=0;j<(disk->block_size * sizeof(unsigned char));++j,++k){
         filebuf[k] = databuf[j];
      }
   }
   for(i=0;i<disk->block_size * file->size * sizeof(unsigned char)
           || filebuf[i] == '\0';++i){
      putchar(filebuf[i]);
   }
}

void main(int argc, char *argv[])
{
  char *disk_name;
  disk_t disk;
  unsigned char *databuf;
  int i, j;

  // Read the parameters
  if(argc != 2) {
    printf("Usage: testdisk <disk_name>\n");
    exit(-1);
  }

  disk_name = (char *)argv[1];

  // Open the disk
  disk = opendisk(disk_name);
  
  //Read and write super block
  write_super_block(disk);
  read_super_block(disk);

  // Set up a buffer for writing and reading
  databuf = malloc(disk->block_size);

  //test writing block map
  printf("testing read and write block map\n");
  int blocks[6]= {1,1,0,1,0,1};
  write_block_map(disk,blocks);
  int * retublocks = read_block_map(disk);
  for(i = 0; i < 6; i+=1){
    printf("%d, ",retublocks[i]);
  }
  printf("\n");
  free(retublocks);

  //test writing inode
    printf("\nWriting Inode\n");
    int pointers[4] = {12,243,3,'\0'};
  Inode* node = writeInode(disk, 3, pointers,false,"test");

  //read the Inode;
  readblock(disk, 3, databuf);
  printf(databuf);
  freeInode(node);
  node = readInode(disk,3);
  printf("Inode size = %d\n",node->size);
  int * pointers2 = node->pointers;
  for(i = 0; i < pointers2[i] != '\0'; i+=1){
    printf("%d,",pointers2[i]);
  }
  freeInode(node);
  printf("\nEnd Inode\n");

  // Write some blocks
  printf("Writing some blocks...");
  for(i = 0; i < disk->size; i++) {

    // Put some data in the block
    for(j = 0; j < BLOCK_SIZE; j++) 
      databuf[j] = i;

    printf("%d ", i);
    writeblock(disk, i, databuf);
  }

  printf("\nWrote successfully\n");

  // Read some blocks
  printf("Reading some blocks...");
  for(i = 0; i < disk->size; i++) {
    printf("%d ", i);
    readblock(disk, i, databuf);

    // Check the data in the block
    for(j = 0; j < BLOCK_SIZE; j++)
      if(databuf[j] != i) {
	printf("Main: read data different from written data!\n");
	exit(-1);
      }
  }

  printf("\nRead Successfully and data verified\n");

  // Seek back and read them again
  printf("Seeking back to block 0\n");
  seekblock(disk, 0);

  // Read some blocks
  printf("Reading some blocks...");
  for(i = 0; i < disk->size; i++) {
    printf("%d ", i);
    readblock(disk, i, databuf);

    // Check the data in the block
    for(j = 0; j < BLOCK_SIZE; j++)
      if(databuf[j] != i) {
	printf("Main: read data different from written data!\n");
	exit(-1);
      }
  }

  printf("\nRead Successfully and data verified\n");
  printf("Done reading and writing.\n");

  // Try seeking past the end of the disk
  //printf("Seeking off the end of the disk\n");
  //seekblock(disk, disk->size);
  free(disk);
  free(databuf);
  exit(0);
}
