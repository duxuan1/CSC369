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

/**
 * TODO: Make sure to add all necessary includes here.
 */

#include "e2fs.h"

void open_disk(const char* image) {

    int fd = open(image, O_RDWR);

    disk = mmap(NULL, 128 * 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(disk == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }
    // set up all variable needed
    sb = (struct ext2_super_block *)(disk + 1024);
    gd = (struct ext2_group_desc *)(disk + (EXT2_BLOCK_SIZE * 2));
    block_bitmap = (unsigned char *)(disk + (EXT2_BLOCK_SIZE * gd->bg_block_bitmap));
    inode_bitmap = (unsigned char *)(disk + (EXT2_BLOCK_SIZE * gd->bg_inode_bitmap));
    block_count = sb->s_blocks_count >> 3;
    inode_count = sb->s_inodes_count >> 3;
    inode_table = (struct ext2_inode *)(disk + EXT2_BLOCK_SIZE * gd->bg_inode_table);
    inode_start = sb->s_first_ino;
    inode_end = sb->s_inodes_count;
}

void close_disk(const char* image) { 

}


int check_abs_path(const char *path) {

    if (strncmp(path, "/", 1) == 0){
        return 1;
    }
    return 0;
}

void slice(char *str, char *buffer, size_t start, size_t end) {

    size_t j = 0;
    for (size_t i = start; i <= end; i++) {
        buffer[j++] = str[i];
    }
    buffer[j] = 0;
}

void split(char **parent, char **dir, char *path) {

    int idx_last_slash = -1;
    int i = 0;
    while (path[i] != '\0') {
        if (path[i] == '/') {
            idx_last_slash = i;
        }
        i++;
    }
    int len_path = strlen(path);

    *parent = malloc(sizeof(char) * len_path);
    slice(path, *parent, 0, idx_last_slash);

    *dir = malloc(sizeof(char) * len_path);
    slice(path, *dir, idx_last_slash + 1, len_path);
}

char get_type_in(struct ext2_inode *inode) {
    char type;
    if (inode->i_mode & EXT2_S_IFLNK) { /* symbolic link */
        type = 'l';
    } else if (inode->i_mode & EXT2_S_IFREG) { /* regular file */
        type = 'f';
    } else if (inode->i_mode & EXT2_S_IFDIR) { /* directory */
        type = 'd';
    }
    return type;
}

char get_type_de(struct ext2_dir_entry *de) {
    char type;
    if (de->file_type & EXT2_FT_SYMLINK) { /* symbolic link */
        type = 'l';
    } else if (de->file_type & EXT2_FT_REG_FILE) { /* regular file */
        type = 'f';
    } else if (de->file_type & EXT2_FT_DIR) { /* directory */
        type = 'd';
    }
    return type;
}

int free_space(unsigned char *bitmap, int start, int end) {
    for (int i = start; i < end; i++) {
        int byte = i >> 3;
        int index = i - (byte << 3);
        if (bitmap[byte] & (1 << index)){
            continue;
        } else{
            return i + 1;
        }
    }
    return -1;
}

int get_rec_len(int name_len) {
    int padding = 4 - (name_len % 4);
    return 8 + name_len + padding;
}

/* Finds the last entry in a directory */
struct ext2_dir_entry *last_entry(struct ext2_dir_entry *entry, int* free_space) {
    int size = 0;
    while (size < EXT2_BLOCK_SIZE) {
        int rec_size = get_rec_len(entry->name_len);
        if (rec_size != entry->rec_len) {
            size += rec_size;
            break;
        }
        size += entry->rec_len;
        entry = (struct ext2_dir_entry *) (((unsigned char *) entry) + entry->rec_len);
    }
    if (free_space) {
        *free_space = EXT2_BLOCK_SIZE - size;
    }
    return entry;
}

struct ext2_dir_entry *get_block(int block_idx) {
    return (struct ext2_dir_entry *)(disk + (EXT2_BLOCK_SIZE * block_idx));
}

struct ext2_inode *get_inode(int inode_idx) {
    return & ((struct ext2_inode *)(disk + EXT2_BLOCK_SIZE * gd->bg_inode_table))[inode_idx-1];
}

void change_bit(unsigned char *bitmap, int bit, int val) {
    unsigned char *byte = bitmap + (bit / 8);
    if (val == 1) {
        *byte |= 1 << (bit % 8);
    } else {
        *byte &= ~(1 << (bit % 8));
    }
}

void change_bitmap(struct ext2_group_desc *gd, unsigned char *bitmap, int bit, int val) {
    change_bit(bitmap, bit, val);
    if (val == 1) {
        gd->bg_free_blocks_count--;
        sb->s_free_blocks_count--;
    } else {
        gd->bg_free_inodes_count++;
        sb->s_free_inodes_count++;
    }
}

struct ext2_dir_entry *traverse_parent_path(char *parent) {
    char *subpath = strtok(parent, "/");
    int inode_idx = EXT2_ROOT_INO;
    struct ext2_dir_entry *de;

    // traverse parent path, find last parent
    while (subpath) {
        // printf("subpath = %s\n", subpath);
        de = find_de(inode_table, inode_idx, subpath, 0);
        if (de != NULL) { // matched directory found
            inode_idx = de->inode;
        }
        subpath = strtok(NULL, "/");
    }

    // set up parent information
    struct ext2_dir_entry *parent_block;
    if (strlen(parent) == 1) { // direct insert to 
        parent_block = find_de(inode_table, EXT2_ROOT_INO, ".", 0);
        return parent_block;
    } 
    parent_block = de;
    parent_block = find_de(inode_table, parent_block->inode, ".", 0);
    return parent_block;
}

void init_inode(struct ext2_inode *inode) {
    /* Use 0 as the user id for the assignment. */
    inode->i_uid = 0;
    inode->i_size = 0;
	/* d_time must be set when appropriate */
    inode->i_dtime = time(NULL);
	/* Use 0 as the group id for the assignment. */
    inode->i_gid = 0;
    inode->i_links_count = 0;
    inode->i_blocks= 0;
	/* You should set it to 0. */
    inode->osd1 = 0;
    for (int i = 0 ; i < 15 ; i++) {
        inode->i_block[i] = 0;
    }
	/* The following fields should be 0 for the assignment.  */
    inode->i_file_acl = 0;
    inode->i_dir_acl = 0;
    inode->i_faddr = 0;
    for (int i = 0; i < 3; i++) {
        inode->extra[i] = 0;
    }
}

void create_in_entry(struct ext2_dir_entry *entry, struct ext2_inode *inode, 
                    int inode_idx, char *fname, unsigned char type) {
    int len = strlen(fname);
    int free_space = -1;
    entry = last_entry(entry, &free_space);

    if (entry->rec_len < EXT2_BLOCK_SIZE) {
        int size = get_rec_len(entry->name_len);
        entry->rec_len = size;
        entry = (struct ext2_dir_entry *)(((unsigned char *) entry) + size);
    }
    entry->inode = inode_idx;
    entry->rec_len = free_space;
    entry->name_len = len;
    entry->file_type = type;
    memcpy(entry->name, fname, len);
}

struct ext2_dir_entry *find_de(struct ext2_inode* inode_table, 
                                    int index, char *path, int need_prev) {
    int path_len = strlen(path);
    struct ext2_inode *inode = &inode_table[index - 1];
    for (int j = 0 ; j < inode->i_blocks / 2 ; j++) {
        int i_block = inode->i_block[j];
        int i = 0;
        int prev = 0;
        while (i < EXT2_BLOCK_SIZE) {
            struct ext2_dir_entry *de = (struct ext2_dir_entry *)
                                            (disk + EXT2_BLOCK_SIZE * i_block + i);
            if (path_len == ((int) de->name_len)) { // find matching de
                if (strncmp(path, de->name, path_len) == 0) {
                    struct ext2_dir_entry *prev_dir = (struct ext2_dir_entry *)
                                (disk + EXT2_BLOCK_SIZE * i_block + prev);
                    if (need_prev == 1) { //return prev entry for rm
                        return prev_dir;
                    } else { // return entry
                        return de;
                    }
                }
            }
            prev = i;
            i += de->rec_len;
        }
    }
    return NULL;
}

int write_buffer(struct ext2_inode *inode, char *buffer, int block_idx, int size) {
    // update block
    struct ext2_dir_entry *entry = get_block(block_idx);
    entry->rec_len = EXT2_BLOCK_SIZE;
    for (int i = 0; i < 12; i++) {
        if (inode->i_block[i] == 0) {
            inode->i_block[i] = block_idx;
            inode->i_blocks += 2;
            break;
        }
        if (inode->i_block[i] == block_idx) {
            break;
        }
    }

    unsigned char *block = (unsigned char *)(entry);
    strncpy((char*)block, buffer, size);

    change_bitmap(gd, block_bitmap, block_idx - 1, 1);
    return 0;
}

