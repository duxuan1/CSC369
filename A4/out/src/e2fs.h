/*
 *------------
 * This code is provided solely for the personal and private use of
 * students taking the CSC369H5 course at the University of Toronto.
 * Copying for purposes other than this use is expressly prohibited.
 * All forms of distribution of this code, whether as given or with
 * any changes, are expressly prohibited.
 *
 * All of the files in this directory and all subdirectories are:
 * Copyright (c) 2019 MCS @ UTM
 * -------------
 */

#ifndef CSC369_E2FS_H
#define CSC369_E2FS_H

#include "ext2.h"
#include <string.h>

// new import from lab 10
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <time.h>
#include <errno.h>


unsigned char *disk; // disk image
struct ext2_super_block *sb;
struct ext2_group_desc *gd;
unsigned char *block_bitmap;
unsigned char *inode_bitmap;
int block_count;
int inode_count;
struct ext2_inode *inode_table;
int inode_start;
int inode_end;

/*
 * One for directory traversal, one for updating the inodes, 
 *
 * mkdir uses directory traversal to find the location, 
 * create and update an inode using helper,
 * and just add directory entries (another helper for this)
 *
 * cp is also straightforward, much like mkdir. 
 * make sure to keep track of the parent directory. 
 * copy the file contents properly (just a while loop)
 */
#define PTRS_PER_INDIRECT_BLOCK (EXT2_BLOCK_SIZE / sizeof(unsigned int))
/*
 * open the disk image by mmap-ing it
 */
void open_disk(const char* image);

void close_disk();


/*
 * check if path is an absolute path
 * return 1 if is
 * return 0 if isn't
 */
int check_abs_path(const char *path);

/*
 * slice the string, like python slicing
 */
void slice(char *str, char *buffer, size_t start, size_t end);

/*
 * split the path to parent and dir
 */
void split(char **parent, char **dir, char *path);

/*
 * get type of an inode
 */
char get_type_in(struct ext2_inode *inode);

/*
 * get type of an directory entry
 */
char get_type_de(struct ext2_dir_entry *de);

/*
 * get inode from inode index
 */
struct ext2_inode *get_inode(int inode_idx);

/*
 * get entry from block index
 */
struct ext2_dir_entry *get_block(int block_idx);

/*
 * find a inode or block that is not in use
 */
int free_space(unsigned char *bitmap, int start, int end);

/*
 * get rec len
 */
int get_rec_len(int name_len);

struct ext2_dir_entry *last_entry(struct ext2_dir_entry *entry, int* fs_dest);

void change_bit(unsigned char *bitmap, int bit, int val);

void change_bitmap(struct ext2_group_desc *gd, unsigned char *bitmap, int bit, int val) ;

struct ext2_dir_entry *traverse_parent_path(char *parent);

void init_inode(struct ext2_inode *inode);

void create_in_entry(struct ext2_dir_entry *entry, struct ext2_inode *inode, 
                    int inode_idx, char *fname, unsigned char type);

struct ext2_dir_entry *find_de(struct ext2_inode* inode_table, 
                                    int index, char *path, int need_prev);

int write_buffer(struct ext2_inode *inode, char *buffer, int block_idx, int size);

#endif
