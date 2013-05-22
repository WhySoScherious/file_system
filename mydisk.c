#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "mydisk.h"

/*
 * Create a new disk in a file
 * Parameters: 
 * - char *disk_name  --  a string representing the name of the file that will contain the disk
 * - int size -- the size (in blocks) of the "disk"
 */
void createdisk(char *disk_name, int size)
{
  printf("Creating disk %s\n", disk_name);
  int fd;
  disk_t disk = malloc(BLOCK_SIZE);
  ssize_t bytes_written;

  fd = creat(disk_name, S_IRWXU);

  if(fd == -1) {
    printf("createdisk creat() failed\n");
    perror("createdisk");
    exit(-1);
  }

  // Write the partition table at disk block 0
  disk->fd = -1;
  disk->last_block = 0;
  disk->block_size = BLOCK_SIZE;
  disk->size = size;

  bytes_written = write(fd, disk, BLOCK_SIZE);

  if(bytes_written == -1) {
    printf("createdisk: write failed\n");
    perror("createdisk");
    exit(-1);
  }

  if(bytes_written != disk->block_size) {
    printf("createdisk: write failed, too few bytes written\n");
    perror("createdisk");
    exit(-1);
  }

  // Close the file and free up the disk structure
  close(disk->fd);
  free(disk);

  return;
}

/*
 * Open a disk file and read the partition table to get the disk parameters
 * Parameters: 
 * - char *disk_name -- the name of the disk file
 */
disk_t opendisk(char *disk_name)
{
  int fd;
  disk_t disk = malloc(BLOCK_SIZE);

  printf("Opening disk %s\n", disk_name);
  fd = open(disk_name, O_RDWR);

  if(fd == -1) {
    printf("opendisk failed\n");
    perror("opendisk");
    exit(-1);
  }

  // Read the partition table
  read(fd, (void *) disk, BLOCK_SIZE);
  disk->fd = fd;  // Set the fd

  printf("Disk %s opened. size = %d, block size = %d\n", disk_name, disk->size, disk->block_size);

  return(disk);
}


// Seeks to the specified location in the disk partition
// Note that we add 1 to block to avoid the partition table
int seekblock(disk_t disk, int block)
{
  off_t offset;

  // Check the block number against the beginning of the disk
  if(block < 0) {
    printf("seekblock: tried to seek below the beginning of the disk!\n");
    printf("seekblock: seek block %d, disk size is %d.\n", block, disk->size);
    exit(-1);
  }

  // Check the block number against the size of the disk
  if(block >= disk->size) {
    printf("seekblock: tried to seek past the end of the disk!\n");
    printf("seekblock: seek block %d.  Disk size is %d, so last block is %d.\n", block, disk->size, disk->size-1);
    exit(-1);
  }

  // Seek to the specified location
  // Note that we add 1 to avoid the partition table
  offset = lseek(disk->fd, (block+1)*disk->block_size, SEEK_SET);

  // Make sure the seek worked
  if(offset == -1) {
    printf("seekblock: lseek failed\n");
    perror("seekblock");
    exit(-1);
  }

  // Update last_block so the next read or write will work properly
  disk->last_block = block;
}

// Reads and writes one block of data
int readblock(disk_t disk, int block, unsigned char *databuf) 
{
  off_t offset;
  ssize_t bytes_read;
  
  if(block < 0) {
    printf("readblock: There are no blocks < 0!!!\n");
    exit(-1);
  }

  // Check the block number against the size of the disk
  if(block >= disk->size) {
    printf("readblock: tried to read past the end of the disk!\n");
    printf("readblock: read block %d, disk size is %d.\n", block, disk->size);
    exit(-1);
  }

  // Seek to the right location, if non_sequential
  if(block != disk->last_block+1)
    seekblock(disk, block);

  bytes_read = read(disk->fd, (void *)databuf, disk->block_size);

  // First see if the read worked at all
  if(bytes_read == -1) {
    printf("readblock read failed\n");
    perror("readblock");
    exit(-1);
  }

  // Then see if we got the right number of bytes
  if(bytes_read != disk->block_size) {
    printf("readblock: read failed, too few bytes read\n");
    printf("readblock: probably because you read data that was not previously written.\n");
    perror("readblock");
    exit(-1);
  }

  // Change last_block to indicate what we have already read
  disk->last_block = block;

  return 0;

}

int writeblock(disk_t disk, int block, unsigned char *databuf) 
{
  off_t offset;
  ssize_t bytes_written;

  if(block < 0) {
    printf("writeblock: There are no blocks < 0!!!\n");
    exit(-1);
  }

  // Check the block number against the size of the disk
  if(block >= disk->size) {
    printf("writeblock: tried to write past the end of the disk!\n");
    printf("writeblock: write block %d, disk size is %d.\n", block, disk->size);
    exit(-1);
  }

  // Seek to the right location, if non_sequential
  if(block != disk->last_block + 1)
    seekblock(disk, block);

  disk->last_block = block;
  bytes_written = write(disk->fd, databuf, disk->block_size);

  if(bytes_written == -1) {
    printf("writeblock failed\n");
    perror("writeblock");
    exit(-1);
  }

  if(bytes_written != disk->block_size) {
    printf("writeblock: write failed, too few bytes written\n");
    perror("writeblock");
    exit(-1);
  }

  return 0;
}
