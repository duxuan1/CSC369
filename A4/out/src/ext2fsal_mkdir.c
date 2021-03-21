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


int32_t ext2_fsal_mkdir(const char *path) {

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
    struct ext2_inode *parent_inode = get_inode(parent_inode_idx);

    // traverse last parent dir, find dir exist or not
    struct ext2_dir_entry *de;
    int inode_idx = parent_inode_idx;
    de = find_de(inode_table, inode_idx, dest, 0);
    if (de != NULL){ // find matched dest, meaning duplicated path
        inode_idx = de->inode;
        struct ext2_inode *inode = &inode_table[inode_idx - 1];
        if (get_type_in(inode) == 'd'){
            return EEXIST; // dest is a directory
        }
        return ENOENT;
    }

    // everything good, find a free inode and create dir
    int free_inode_idx = free_space(inode_bitmap, inode_start, inode_end);
    // need a new inode 
    if (free_inode_idx == -1) {
        return ENOSPC;
    }
    struct ext2_inode *inode = get_inode(free_inode_idx);
    init_inode(inode);

    // need a new block
    int free_block_idx = free_space(block_bitmap, 0, sb->s_blocks_count);
    if (free_block_idx == -1) {
        return ENOSPC;
    }

    // update block
    inode->i_block[0] = free_block_idx;
    struct ext2_dir_entry *block = get_block(free_block_idx);
    block->rec_len = EXT2_BLOCK_SIZE;

    // create
    create_in_entry(block, inode, free_inode_idx, ".", EXT2_FT_DIR);
    create_in_entry(block, inode, parent_inode_idx, "..", EXT2_FT_DIR);
    create_in_entry(parent_block, parent_inode, free_inode_idx, dest, EXT2_FT_DIR);

    // update inodes
    parent_inode->i_links_count++;
    inode->i_mode = EXT2_S_IFDIR;
    inode->i_links_count = 2;
    inode->i_size = EXT2_BLOCK_SIZE;
    inode->i_blocks += 2;

    // change bitmaps
    change_bitmap(gd, block_bitmap, free_block_idx - 1, 1);
    change_bitmap(gd, inode_bitmap, free_inode_idx - 1, 1);
    return 0;
}
