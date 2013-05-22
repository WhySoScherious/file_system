#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "mydisk.h"

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

  // Set up a buffer for writing and reading
  databuf = malloc(disk->block_size);

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
  printf("Seeking off the end of the disk\n");
  seekblock(disk, disk->size);
  
}
