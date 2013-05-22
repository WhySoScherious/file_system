struct Inode
  int size;
  int ** pointers;
  int nextInodePointer;
};

typedef struct Inode *Inode;

Inode * readInode(int block);

void writeInode(int block, Inode inode);
