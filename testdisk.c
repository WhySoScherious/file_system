#include <math.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "file_system.h"

char * get_input () {
    char buffer[1000];

    char *line = fgets (buffer, sizeof buffer, stdin);
    line = strchr (buffer, '\n');

    while (line == NULL || buffer[0] == '\0') {
        line = fgets (buffer, sizeof buffer, stdin);
        line = strchr (buffer, '\n');
    }
    *line = '\0';
    return strdup (buffer);
}

int firstEmpty (int *blocks) {
    int i;
    for (i = 0; i < BLOCK_SIZE; i++) {
        if (blocks[i] == 0) {
            return i;
        }
    }
    return -1;
}

int write_file (disk_t disk, char *file_name,Inode * root) {
    FILE *file = fopen (file_name, "r");

    if (file == NULL) {
        fflush (NULL);
        fprintf (stderr, "%s: %s\n", file_name, strerror (errno));
        fflush (NULL);

        return -1;
    } else {
        unsigned char *databuf = calloc (BLOCK_SIZE, sizeof (unsigned char));
        unsigned char *filebuf = calloc (BLOCK_SIZE * disk->size, sizeof (unsigned char));

        int i = 0;

        while (feof (file) == 0)
            filebuf[i++] = fgetc(file);
        filebuf[i - 1] = '\0';

        int num_blocks = ceil ((double) i / BLOCK_SIZE);

        fclose (file);
        printf ("\nFinish reading file\n");

        int *retublocks = read_block_map(disk);
        int i_node_block = firstEmpty (retublocks);

        retublocks[i_node_block] = 1;
        write_block_map (disk, retublocks);

        int file_block;
        int num_pointers = num_blocks + 1;
        int pointers[num_pointers];
        pointers[num_pointers - 1] = '\0';

        for (i = 0; i < num_blocks; i++) {
            retublocks = read_block_map(disk);
            file_block = firstEmpty (retublocks);
            pointers[i] = file_block;

            retublocks[file_block] = 1;
            write_block_map (disk, retublocks);
        }

        printf("\nWriting Inode\n");

        Inode *node = writeInode(disk, i_node_block, pointers, false, file_name);
        //Inode *root = readInode (disk, 1);

        int size = arrayLength(root->pointers) + 1;

        int *ptrs = calloc (size + 1, sizeof (int));
        for (i = 0; i < size - 1; i++) {
            ptrs[i] = root->pointers[i];
        }

        ptrs[i++] = i_node_block;
        ptrs[i] = '\0';
    	free(root->pointers);
        root->pointers = ptrs;
        writeInode (disk, 1, root->pointers, root->isDirectory, root->name);

        int k = 0;
        for(i = 0; i < num_blocks; ++i, k = k + BLOCK_SIZE) {
            strncpy (databuf, filebuf + k, BLOCK_SIZE);
            writeblock (disk, pointers[i], databuf);
        }

        printf("Wrote successfully\n");

        free (filebuf);
        free (databuf);
		freeInode(node);
        return i_node_block;
    }
}

void ls (disk_t disk,Inode * root) {
    int size = arrayLength (root->pointers);
    int i;
    for (i = 0; i < size; i++) {
        Inode* node = readInode (disk, root->pointers[i]);

        printf ("%s   ", node->name);
    }
    printf ("\n");
}

/*
 * Return -1 if the file name is invalid, else return i-node number of
 * file.
 */
int check_file (disk_t disk, char *file_name, Inode* root) {
    int size = arrayLength (root->pointers);

    int file_num = -1;
    int i;
    for (i = 0; i < size; i++) {
        Inode* node = readInode (disk, root->pointers[i]);

        if (strcmp (file_name, node->name) == 0) {
            file_num = root->pointers[i];
            break;
        }
		freeInode(node);
        file_num = -1;
    }

    return file_num;
}

void readdisk(disk_t disk, int blocknum){
    Inode *file = readInode(disk,blocknum);
    if(file->size == 0) {
        printf("Is a directory\n");
        return;
    }
    unsigned char *databuf = malloc(disk->block_size * (sizeof(unsigned char)));
    unsigned char *filebuf = malloc(disk->block_size * file->size * sizeof(unsigned char));
    int i,j,k;
    k = 0;
    for(i=0;i<file->size;++i,++blocknum){
        readblock(disk,file->pointers[i],databuf);
        for(j=0;j<(disk->block_size * sizeof(unsigned char));++j,++k){
            filebuf[k] = databuf[j];
        }
    }
    for(i=0;i<disk->block_size * file->size * sizeof(unsigned char)
            || filebuf[i] == '\0';++i){
        putchar(filebuf[i]);
    }
    freeInode (file);
}

void mkdir2(disk_t disk, Inode ** currdir, char * name){
	int * blkmap = read_block_map(disk);
	int newblkloc;
	int i,j;
	for(newblkloc = 0; newblkloc < disk->size;newblkloc+=1){
		if(blkmap[newblkloc] == 0)
			break;
	}
	int pointers[1];
	pointers[0] = 0;
	freeInode(writeInode(disk,newblkloc,pointers,true,name));
	for(i = 0; currdir[0]->pointers[i] != '\0'; i+=1);
	int * newpointers = calloc(i+2,sizeof(int));
	for(j = 0; j < i;j+=1){
		newpointers[j] = currdir[0]->pointers[j];
	}
	newpointers[j] = newblkloc;
	newpointers[j+1] = '\0';
	free(currdir[0]->pointers);
	currdir[0]->pointers = newpointers;

	*currdir = rewriteInode(disk,currdir[0]);
}
Inode * cd2(disk_t disk,char * arg,Inode *currdir,int ** history){
	int i,j,k;
	if(strcmp(arg,"..") == 0){
		for(i=0; history[0][i] != 0; i+=1);
		if(i != 0){
			int * newhistory = calloc(i-1,sizeof(int));
			for(k = 0; k < i-1;k+=1){
				newhistory[k] = history[0][k];
			}
			int returnto = history[0][i-1];
			newhistory[k] = 0;
			free(*history);
			*history = newhistory;
			freeInode(currdir);
			printf("returning to %d\n",returnto);
			return(readInode(disk,returnto));
		}
	}else{
		for(i = 0; currdir->pointers[i] != '\0';i+=1){
			printf("looking for pointer %d\n",currdir->pointers[i]);
			Inode * node = readInode(disk,currdir->pointers[i]);
			if(strcmp(arg,node->name) == 0 && node->isDirectory){
				for(j = 0; history[0][j] != 0; j+=1);
				int * newhistory = calloc(j+1,sizeof(int));
				for(k = 0; k < j;k+=1){
					newhistory[k] = history[0][k];
				}
				newhistory[k] = currdir->block;
				newhistory[k+1] = 0;
				free(history[0]);
				*history = newhistory;
				freeInode(currdir);
				printf("found\n");
				return node;
			}
		}
	}
	printf("dir %s, not found\n",arg);
	return currdir;
}


void main(int argc, char *argv[])
{
	Inode * currdir;
    char *disk_name;
    disk_t disk;
    unsigned char *databuf;
    int i, j;
	int * history = calloc(2,sizeof(int));
	history[0] = 1;
	history[1] = 0;
    // Read the parameters
    if(argc != 2) {
        printf("Usage: testdisk <disk_name>\n");
        exit(-1);
    }

    disk_name = (char *)argv[1];

    // Open the disk
    disk = opendisk(disk_name);

    //Read and write super block
    write_super_block(disk);
    read_super_block(disk);

    // Set up a buffer for writing and reading
    databuf = malloc(disk->block_size);

    //test writing block map
    printf("testing read and write block map\n");
    int * blocks = calloc(disk->size,sizeof(int));

    for(i = 0; i < 5; i+=1){
        blocks[i] = 1;
    }

    write_block_map(disk,blocks);
    int * retublocks = read_block_map(disk);
    for(i = 0; i < 6; i+=1){
        printf("%d, ",retublocks[i]);
    }

    printf("\n");
    free(retublocks);
    free(blocks);

    // Write the root directory
    printf ("Writing root directory\n");
    write_root_dir (disk);
	currdir = readInode(disk,1);

    while (1) {
        printf ("\n$ ");
        char *command = get_input();

        if (strcmp (command, "ls") == 0) {
            ls (disk,currdir);
        } else if (strcmp (command, "exit") == 0) {
            break;
        } else if (strstr (command, " ") == NULL) {
            fflush (NULL);
            fprintf (stderr, "%s: command not found\n", command);
            fflush (NULL);
        } else {
            char *cmd = strstr (command, " ");
            *cmd = '\0';
            char *arg = cmd + 1;

            if (strcmp (command, "cp") == 0) {
                if (check_file (disk, arg,currdir) == -1) {
                    write_file (disk, arg,currdir);
                } else {
                    fflush (NULL);
                    fprintf (stderr, "%s: %s: a file with that name"
                            " already exits\n", command, arg);
                    fflush (NULL);
                }
            }else if((strcmp (command,"mkdir") == 0)){
				mkdir2(disk,&currdir,arg);
				printf("new pointers = ");
	for(i = 0;currdir->pointers[i] != '\0';i+=1){
		printf("%d",currdir->pointers[i]);
	}
	printf("\n");
			}else if((strcmp(command,"cd") == 0)){
				currdir = cd2(disk,arg,currdir,&history);
			}else if ((strcmp (command, "cat") == 0)) {
                int file_num = check_file (disk, arg,currdir);

                if (file_num != -1) {
                    readdisk (disk, file_num);
                } else {
                    fflush (NULL);
                    fprintf (stderr, "%s: No such file or directory\n", arg);
                    fflush (NULL);
                }
            } else {
                fflush (NULL);
                fprintf (stderr, "%s: command not found\n", command);
                fflush (NULL);
            }
        }
    }

    free(disk);
    free(databuf);
    exit(0);
}
