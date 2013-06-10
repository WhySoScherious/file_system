#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "file_system.h"

/*
 * Gets the user input from keyboard and returns it as a string.
 */
char * get_input () {
    char buffer[1000];

    /*
     * Get user input and replace the newline character with a null
     * character.
     */
    char *line = fgets (buffer, sizeof buffer, stdin);
    line = strchr (buffer, '\n');

    while (line == NULL || buffer[0] == '\0') {
        line = fgets (buffer, sizeof buffer, stdin);
        line = strchr (buffer, '\n');
    }

    *line = '\0';
    return strdup (buffer);
}

/*
 * Checks if the string passed has only whitespaces. Returns true if it
 * has only whitespaces, false otherwise.
 */
int is_line_spaced (const char *line) {
    int i;

    for(i = 0; i < strlen (line); i++) {
        if (line[i] != ' ') {
            return 0;
        }
    }

    return 1;
}

/*
 * Returns the block number of the first empty block in the free block
 * map.
 */
int first_empty (int *blocks) {
    int i;
    for (i = 0; i < BLOCK_SIZE; i++) {
        if (blocks[i] == 0) {
            return i;
        }
    }

    return -1;
}

/*
 * Copies a file from your Unix directory into your "disk" directory.
 */
Inode * write_file (disk_t disk, char *file_name, Inode *root) {
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

        // Copy the contents of the file to the file buffer
        while (feof (file) == 0) {
            filebuf[i++] = fgetc(file);
        }

        filebuf[i - 1] = '\0';
        fclose (file);

        /*
         * Determine the number of blocks the file will use on the
         * "disk".
         */
        int num_blocks = ceil ((double) i / BLOCK_SIZE);

        int *retublocks = read_block_map(disk);     // Free block map

        /*
         * Block number of the i-node the file will go into.
         */
        int i_node_block = first_empty (retublocks);

        /*
         * Set the file i-node block as used in the free block map and
         * write the block map back to the "disk".
         */
        retublocks[i_node_block] = 1;
        write_block_map (disk, retublocks);
        int file_block;         // Block number the file will go in

        // Size of the array of disk block pointers
        int num_pointers = num_blocks + 1;
        int pointers[num_pointers]; // Disk block pointers

        pointers[num_pointers - 1] = '\0';

        /*
         * For the amount of blocks needed to write the file, find the
         * first available block in the free block map, assign it
         * as used, and store the block number in the disk block
         * pointer array.
         */
    	
        for (i = 0; i < num_blocks; i++) {
            file_block = first_empty (retublocks);
            pointers[i] = file_block;

            retublocks[file_block] = 1;
            
        }
		write_block_map (disk, retublocks);
		free(retublocks);

        /*
         * Now add the i-node number of the file to the root i-node
         * list of pointers.
         */
        // I-node of the file
        Inode *node = writeInode(disk, i_node_block, pointers, false, file_name);

        // Size of the number of files on the disk
        int size = arrayLength(root->pointers);

        // New array of pointers for file i-nodes.
        int *ptrs = calloc (size + 2, sizeof (int));

        // Copy the existing array of pointers to new array.
        for (i = 0; i < size; i++) {
            ptrs[i] = root->pointers[i];
        }

        /*
         * Add the new i-node block number to the array of pointers and
         * rewrite the root i-node.
         */
        ptrs[i++] = i_node_block;
        ptrs[i] = '\0';
		printf("printing POINTERS:");
		for(i = 0; ptrs[i] != '\0';i+=1){
			printf("%d,",ptrs[i]);
		}
		printf("\n");
        free(root->pointers);

        root->pointers = ptrs;
        root = rewriteInode (disk, root);

        // Write the file to the "disk"
        int k = 0;
        for(i = 0; i < num_blocks; i++) {
            strncpy (databuf, filebuf + k, BLOCK_SIZE);
            writeblock (disk, pointers[i], databuf);
            k = k + BLOCK_SIZE;
        }

        free (filebuf);
        free (databuf);
        freeInode(node);
        return root;
    }
}

/*
 * Lists the files from the "disk" directory and prints file names to
 * screen.
 */
void ls (disk_t disk, Inode *root) {
    int size = arrayLength (root->pointers);

    int i;
    for (i = 0; i < size; i++) {
        Inode* node = readInode (disk, root->pointers[i]);
        printf ("%s   ", node->name);
		freeInode(node);
    }
    printf ("\n");
}

/*
 * Checks if the file exists on the disk. Returns -1 if the file
 * doesn't exist, else return i-node number of file.
 */
int check_file (disk_t disk, char *file_name, Inode *root) {
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
    }

    return file_num;
}

/*
 * Reads a file from your "disk" and prints it out to screen.
 */
void readdisk(disk_t disk, int blocknum){
    Inode *file = readInode (disk, blocknum);

    unsigned char *databuf = malloc(disk->block_size
            * (sizeof(unsigned char)));
    unsigned char *filebuf = malloc(disk->block_size * file->size
            * sizeof(unsigned char));
    int i,j,k;

    /*
     * Read the disk blocks the file are stored in and place the
     * contents into the file buffer.
     */
    k = 0;
    for (i=0; i < file->size; ++i, ++blocknum){
        readblock (disk, file->pointers[i], databuf);
        for(j = 0; j < (disk->block_size * sizeof (unsigned char));
                ++j, ++k){
            filebuf[k] = databuf[j];
        }
    }

    // Print the file to the screen
    for (i = 0; i < disk->block_size * file->size * sizeof(unsigned char)
            || filebuf[i] == '\0'; ++i) {
        putchar(filebuf[i]);
    }

    freeInode (file);
}

/*
 * Creates a directory within the current directory.
 */
void mkdir2(disk_t disk, Inode ** currdir, char * name){
    int * blkmap = read_block_map(disk);
    int newblkloc;
    int i,j;
    for(newblkloc = 0; newblkloc < disk->size;newblkloc+=1){
        if(blkmap[newblkloc] == 0)
            break;
    }
	free(blkmap);
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

/*
 * Changes the current directory in the "disk". Returns to the previous
 * level directory if "cd .." was passed. If the directory exists,
 * return the i-node of the directory, else return the current
 * directory.
 */
Inode * cd2(disk_t disk,char * arg,Inode *currdir,int ** history){
    int i,j,k;
    if(strcmp(arg,"..") == 0){
        for(i=0; history[0][i] != 0; i+=1);
        if(i != 0){
            int * newhistory = calloc(i,sizeof(int));
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
    } else {
        for(i = 0; currdir->pointers[i] != '\0';i+=1){
            printf("looking for pointer %d\n",currdir->pointers[i]);
            Inode * node = readInode(disk,currdir->pointers[i]);
            if(strcmp(arg,node->name) == 0 && node->isDirectory){
                for(j = 0; history[0][j] != 0; j+=1);
                int * newhistory = calloc(j+2,sizeof(int));
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
    printf("cd: %s: No such file or directory\n",arg);
    return currdir;
}

Inode * rm2(disk_t disk, char * arg, Inode * currdir){
	int i,j;
	Inode * match_inode = NULL;
	for(i = 0; currdir->pointers[i] != 0; i+=1){
		Inode * node = readInode(disk,currdir->pointers[i]);
		if(strcmp(node->name,arg) == 0){
			match_inode = node;
			break;
		}else
			freeInode(node);
	}
	if(match_inode != NULL){
		if(match_inode->isDirectory){
			while( match_inode->pointers[0] != 0){
				Inode * node = readInode(disk,match_inode->pointers[0]);
				match_inode = rm2(disk,node->name,match_inode);
				freeInode(node);
			}
		}
		//free up blocks
		deleteInode(disk,match_inode);
		int * bmap = read_block_map(disk);
		for(i = 0; match_inode->pointers[i] != 0;i+=1){
			bmap[match_inode->pointers[i]] = 0;
		}
		write_block_map(disk,bmap);
		free(bmap);
		//remove pointer from curr dir;
		int length;
		for(length = 0; currdir->pointers[length] != 0; length+=1);
		int * pointers = calloc(length,sizeof(int));
		j=0;
		for(i = 0; i < length; i +=1){
			if(currdir->pointers[i] != match_inode->block){
				pointers[j] = currdir->pointers[i];
				j+=1;
			}
		}
		pointers[j] = '\0';
		free(currdir->pointers);
		currdir->pointers = pointers;
		currdir = rewriteInode(disk,currdir);
		freeInode(match_inode);
		return currdir;
	}else{
		printf("no file or folder with that name was found\n");
		return currdir;
	}
}

void exec_shell (disk_t disk) {
    Inode *currdir;

    // Set the current directory to the root
    currdir = readInode(disk, 1);

    int *history = calloc (2, sizeof (int));
    history[0] = 1;
    history[1] = 0;

    char *curr_dir_string = malloc (BLOCK_SIZE);
    curr_dir_string[0] = '\0';

    while (1) {
        printf ("\n~%s\n", curr_dir_string);
        printf ("$ ");
        char *command = get_input();

        if (strcmp (command, "ls") == 0) {
            ls (disk, currdir);
        } else if (strcmp (command, "exit") == 0) {
			free(command);
            break;
        } else if (strcmp (command, "") == 0
                || is_line_spaced (command)) {
            continue;
        } else if (strcmp (command, "cp") == 0
                || strcmp (command, "cat") == 0) {
            fflush (NULL);
            fprintf (stderr, "%s: missing file operand\n", command);
            fflush (NULL);
        } else if (strstr (command, " ") == NULL) {

            // If an invalid command was passed, print error
            fflush (NULL);
            fprintf (stderr, "%s: command not found\n", command);
            fflush (NULL);
        } else {

            // Parse the argument from the user input
            char *cmd = strstr (command, " ");
            *cmd = '\0';
            char *arg = cmd + 1;

            if (strcmp (command, "cp") == 0) {
                if (check_file (disk, arg, currdir) == -1) {
                    currdir = write_file (disk, arg, currdir);
                } else {
                    fflush (NULL);
                    fprintf (stderr, "%s: %s: a file with that name"
                            " already exits\n", command, arg);
                    fflush (NULL);
                }
            } else if (strcmp (command,"mkdir") == 0) {
                if (check_file (disk, arg, currdir) == -1) {
                    mkdir2 (disk, &currdir, arg);
                }
            } else if(strcmp (command, "rm") == 0 ){
				currdir = rm2(disk,arg, currdir);
			}else if (strcmp(command,"cd") == 0) {

                // Store current directory's name
                char *old_dir = strdup (currdir->name);

                // Set current directory to argument passed
                currdir = cd2 (disk, arg, currdir, &history);

                /*
                 * If the current directory is root, print out nothing
                 * signifying root directory.
                 */
                if (strcmp (currdir->name, "/") == 0) {
                    curr_dir_string[0] = '\0';
                } else if (strcmp (arg, "..") == 0) {

                    /*
                     * If changing to the previous directory, remove
                     * the old directory from the current directory
                     * string printout.
                     */
                    char *remove = strstr (curr_dir_string, old_dir);
                    if (remove != NULL) {
                        remove = remove - 1;
                        *remove = '\0';
                    }
					
                } else if (strcmp (old_dir, currdir->name) != 0) {

                    /*
                     * If the directory changed, and the directory to
                     * the current directory string.
                     */
                    strcat (curr_dir_string, "/");
                    strcat (curr_dir_string, currdir->name);
                }
				free(old_dir);
            } else if (strcmp (command, "cat") == 0) {

                // Check if the file exists on the "disk"
                int file_num = check_file (disk, arg, currdir);

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
		free(command);
    }
	free(curr_dir_string);
	free(history);
	freeInode(currdir);
}

void main(int argc, char *argv[])
{
    char *disk_name;
    disk_t disk;
    unsigned char *databuf;
    int i, j;

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

    // Fill the first 5 blocks in bitmap as used
    int * blocks = calloc(disk->size,sizeof(int));

    // Set the corresponding blocks as used with a '1'
    for(i = 0; i < 5; i+=1){
        blocks[i] = 1;
    }

    write_block_map(disk,blocks);

    free(blocks);

    // Write the root directory
    printf ("Writing root directory\n");
    write_root_dir (disk);

    // Execute the shell program
    exec_shell (disk);

    free(disk);
    free(databuf);
    exit(0);
}
