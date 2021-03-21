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

#include "ext2fsal.h"
#include "e2fs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>


int32_t ext2_fsal_rm(const char *path) {

    // path must be an absolute path on your ext2-formatted disk image
    if (check_abs_path(path) != 1) {
        return ENOENT;
    }

    // split
    char *parent, *dest;
    split(&parent, &dest, (char *)path);

    // traverse lats parent path
    struct ext2_dir_entry *parent_block = traverse_parent_path(parent);
    if (parent_block == NULL) {
        return ENOENT; // path not exist
    }
    int parent_inode_idx = parent_block->inode;

    // traverse last parent dir, find dir exist or not
    struct ext2_dir_entry *de;
    int inode_idx = parent_inode_idx;
    de = find_de(inode_table, inode_idx, dest, 0);
    if (de != NULL){ // find matched dest, meaning file or dir exist
        inode_idx = de->inode;
        struct ext2_inode *inode = get_inode(inode_idx);
        if (get_type_in(inode) == 'd'){ // cannot delete direcotry
            return EISDIR;
        }
    } else { // file not exist
        return ENOENT;
    }

    // find bit need to be changed for block bitmap
    struct ext2_inode *inode = get_inode(inode_idx);
    int bit = inode->i_block[0];

    // find previous entry
    struct ext2_dir_entry *prev_entry = find_de(inode_table, parent_inode_idx, dest, 1);

    // add rec_len to previous entry
    prev_entry->rec_len = prev_entry->rec_len + de->rec_len;
    change_bitmap(gd, block_bitmap, bit - 1, 0);
    change_bitmap(gd, inode_bitmap, de->inode - 1, 0);

    return 0;
}
