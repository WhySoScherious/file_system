#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>


#define BLOCK_SIZE 512

// This is the "partition table" information describing the format of the disk
// NOTE: THIS STRUCTURE LIVES IN DISK BLOCK 0
// We adjust the block numbers of all seeks to account for it
struct diskt
{
  int fd;
  int last_block;
  int block_size;
  int size;  // in blocks
};

typedef struct diskt *disk_t;


/*
 * Create a new disk file
 * Parameters:
 * - char *disk_name -- the name of the disk file
 * - int size -- the size in blocks of the disk to create
 */
void createdisk(char *disk_name, int size);

/*
 * Open an existing disk file
 * Parameters:
 * - char *disk_name -- the name of the disk file
 */
disk_t opendisk(char *disk_name);

/*
 * Seek to a specific block within the file system partition of a disk file
 * Parameters: 
 * - disk_t disk -- the disk 
 * - int block, the block to seek to
 */
int seekblock(disk_t disk, int block);

/*
 * Read the specified block of the disk file system partition
 * Parameters:
 * - disk_t disk -- The disk to read
 * - int block -- the block to read
 * - unsigned char *databuf -- a pointer to a block-sized data buffer to read the data into
 */
int readblock(disk_t disk, int block, unsigned char *databuf);

/*
 * Write the specified block of the disk file system partition
 * Parameters:
 * - disk_t disk -- The disk to write
 * - int block -- the block to write
 * - unsigned char *databuf -- a pointer to a block-sized data buffer containing the data to write
 */
int writeblock(disk_t disk, int block, unsigned char *databuf);
