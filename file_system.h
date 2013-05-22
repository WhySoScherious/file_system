struct Inode{
  int size;
  int ** pointers;
  int nextInodePointer;
}
Inode * readInode(char * filename);

void writeInode(int block, Inode * inode);
