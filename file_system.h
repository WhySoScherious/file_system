struct Inode{
  int size;
  int ** pointers;
  int nextInodePointer;
};

typedef struct Inode *Inode;

Inode readInode(int block);

void writeInode(int block, Inode inode);

Inode createInode(int size, int ** pointers);

void write_super_block(disk_t disk);

void read_super_block(disk_t disk);

void write_root_dir(disk_t disk);

void read_root_dir(disk_t disk);

void write_block_map(disk_t disk);

void read_block_map(disk_t disk);
