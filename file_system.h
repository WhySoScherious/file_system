struct Inode{
  int size;
  int ** pointers;
  int nextInodePointer;
};

typedef struct Inode *Inode;

Inode * readInode(char * filename);

void writeInode(int block, Inode * inode);
