
struct Inode{
  int size;
  int ** pointers;
  int nextInodePointer;
}
readInode(char * filename);
