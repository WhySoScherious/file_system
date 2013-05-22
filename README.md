file_system
===========

Program that implements a file system in user space.

The assignment is to:

Write a format program that creates a superblock, a free block map, and an initial root directory for your disk. These functions should use a disk that has been created with makedisk and will have to use opendisk(), seekblock(), and writeblock() to format the disk.
  Your superblock should include the size of the partition, the location of the free block map, the location of the root 
  directory, and the start of the data blocks.
  Your superblock and inodes can each be one block long. Your free block map may be larger.
  Your free block map can be a bitmap with 0s for free blocks and 1s for allocated blocks. I would just include the     
  superblock, root directory, and bitmap blocks in the bitmap. That will make the block numbers correspond to the blocks 
  on your "disk".

Write functions to read and write your root directory (writing a directory = creating a file).
  Keep in mind that your files should have an inode, but your inodes may be very simple, containing
  Your inodes can be very simple. They can be fixed size (1 block) and need only contain the size of the file and an 
  array of direct block pointers (i.e., a list of block numbers for the blocks in the file).

Write functions to read and write files on your disk.
  It might be a good idea to start with 1 block files. Once you get them working, then make it possible for your files   
  to be larger than 1 block.
  A good way to test all of the above would be to create three small test programs: 1) A program that copies files from 
  your Unix directory into your "disk" directory, 2) A program that lists the directory on your "disk", and 3) A program 
  that reads a file from your "disk" and prints it out.

To turn in: a tar file containing the above code and your test code that shows that it all works. Do NOT turn in .o files or executables. Just the source file and a makefile.
