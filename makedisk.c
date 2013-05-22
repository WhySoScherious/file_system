#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "mydisk.h"

void main(int argc, char *argv[])
{
  char *disk_name;
  int disk_size;

  // Read the parameters
  if(argc != 3) {
    printf("Usage: makedisk <disk_name> <disk_size> (in blocks)\n");
    exit(-1);
  }

  disk_name = (char *)argv[1];
  disk_size = atoi((char *)argv[2]);

  // Format the disk
  createdisk(disk_name, disk_size);

  printf("Disk %s created with %d blocks\n", disk_name, disk_size);

  exit(0);
}
