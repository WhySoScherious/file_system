struct InodeStruct{
  int size;
  int ** pointers;
  int nextInodePointer;
};

typedef struct Inode *InodeStruct;

Inode * readInode(int block);

void writeInode(int block, Inode inode);
