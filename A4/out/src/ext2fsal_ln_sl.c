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

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>


int32_t ext2_fsal_ln_sl(const char *src, const char *dst) {
    // path must be an absolute path on your ext2-formatted disk image
    if ((check_abs_path(src) != 1) || (check_abs_path(dst) != 1)) {
        return ENOENT;
    }
    
    // split
    char *parent_src, *dest_src;
    split(&parent_src, &dest_src, (char *)src);

    // split
    char *parent_dst, *dest_dst;
    split(&parent_dst, &dest_dst, (char *)dst);

    if (strlen(parent_src) != strlen(parent_dst)) {
        return ENOENT;
    }
    if (strncmp(parent_src, parent_dst, strlen(parent_src) != 0)) {
        return ENOENT;
    }

    // traverse lats parent path
    struct ext2_dir_entry *parent_block = traverse_parent_path(parent_src);
    if (parent_block == NULL) {
        return ENOENT; // path not exist
    }
    int parent_inode_idx = parent_block->inode;
    // struct ext2_inode *parent_inode = get_inode(parent_inode_idx);

    // traverse last parent dir, find dir exist or not
    struct ext2_dir_entry *de_src;
    int inode_idx = parent_inode_idx;
    de_src = find_de(inode_table, inode_idx, dest_src, 0);
    if (de_src != NULL){ // find matched dest, meaning duplicated path
        inode_idx = de_src->inode;
        struct ext2_inode *inode = &inode_table[inode_idx - 1];
        if (get_type_in(inode) == 'd') {
            return EISDIR; // dest is a directory
        }
    }
    struct ext2_inode *inode = get_inode(inode_idx);
    // traverse last parent dir, find dir exist or not
    struct ext2_dir_entry *de_dst;
    de_dst = find_de(inode_table, inode_idx, dest_src, 0);
    if (de_dst != NULL){ // find matched dest, meaning duplicated path
        return EEXIST; // dst exist
    }

    // need a new block
    int free_block_idx = free_space(block_bitmap, 0, sb->s_blocks_count);
    if (free_block_idx == -1) {
        return ENOSPC;
    }

    // update block
    for (int i = 0; i < 12; i++) {
        if (inode->i_block[0] != 0) {
            inode->i_block[0] = free_block_idx;
            break;
        }
    }

    char type = EXT2_FT_SYMLINK;
    create_in_entry(de_src, inode, inode_idx, dest_dst, type);
    write_buffer(inode, (char *)src, free_block_idx, strlen(src));

    change_bitmap(gd, block_bitmap, free_block_idx - 1, 1);

    return 0;
}
