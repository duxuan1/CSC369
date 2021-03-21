/*
 *------------
 * This code is provided solely for the personal and private use of
 * students taking the CSC369H5 course at the University of Toronto.
 * Copying for purposes other than this use is expressly prohibited.
 * All forms of distribution of this code, whether as given or with
 * any changes, are expressly prohibited.
 *
 * Authors:
 * vlad.sytchenko@mail.utoronto.ca, Vladislav Sytchenko
 * bogdan@cs.toronto.edu, Bogdan Simion
 *
 * All of the files in this directory and all subdirectories are:
 * Copyright (c) 2019 MCS @ UTM
 * -------------
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

// Displays the string from the server's side.
// Note that this is a real NOP that will be submitted to the server and executed (skippeded over) by it.
//
// message is a pointer to a zero terminated string that will be displayed
//
// returns 0 if the operation completed succefully. 
// Otherwise, one of the following errors may be returned:
//         ETIMEDOUT if the command hasn't finished within the timeout window
int32_t ext2umfs_print(const char *message);

// src is a pointer to a zero terminated string
// dst is a pointer to a zero terminated string
//
// returns 0 if the operation completed succefully. 
// Otherwise, one of the following errors may be returned:
//         ETIMEDOUT if the command hasn't finished within the timeout window
//         See handout for cp-specific errors.
int32_t ext2umfs_cp(const char *src,
                    const char *dst);

// src is a pointer to a zero terminated string
// dst is a pointer to a zero terminated string
//
// returns 0 if the operation completed succefully. 
// Otherwise, one of the following errors may be returned:
//         ETIMEDOUT if the command hasn't finished within the timeout window
//         See handout for ln-specific errors.
int32_t ext2umfs_ln_hl(const char *src,
                       const char *dst);

// src is a pointer to a zero terminated string
// dst is a pointer to a zero terminated string
//
// returns 0 if the operation completed succefully. 
// Otherwise, one of the following errors may be returned:
//         ETIMEDOUT if the command hasn't finished within the timeout window
//         See handout for ln-specific errors.
int32_t ext2umfs_ln_sl(const char *src,
                       const char *dst);

// path is a pointer to a zero terminated string
//
// returns 0 if the operation completed succefully. 
// Otherwise, one of the following errors may be returned:
//         ETIMEDOUT if the command hasn't finished within the timeout window
//         See handout for rm-specific errors.
int32_t ext2umfs_rm(const char *path);

// path is a pointer to a zero terminated string
//
// returns 0 if the operation completed succefully. 
// Otherwise, one of the following errors may be returned:
//         ETIMEDOUT if the command hasn't finished within the timeout window
//         See handout for mkdir-specific errors.
int32_t ext2umfs_mkdir(const char *path);

// Enables the server to run in synchronous mode.
// All file system operations will be blocked from being submitted 
// if there are any currently in flight.
void ext2umfs_enable_sync_mode();
// Enables the server to run in asynchronous mode.
// Multiple file system operations can be submitted concurrently.
void ext2umfs_disable_sync_mode();

// Enables logging on the server side.
// At submit time all file systems operations will log their timestamp
// and the command that will be executed.
// When a file system completes returns it will log its timestamp and status code.
void ext2umfs_enable_debug_mode();
// Disables logging on the server side.
void ext2umfs_disable_debug_mode();
