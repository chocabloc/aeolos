#pragma once

#include "../vfs/vfs.h"

extern vfs_fsinfo_t ttyfs;

vfs_inode_t* ttyfs_mount(vfs_inode_t* at);
int64_t ttyfs_mknode(vfs_tnode_t* this);
int64_t ttyfs_read(vfs_inode_t* this, size_t offset, size_t len, void* buff);
int64_t ttyfs_write(vfs_inode_t* this, size_t offset, size_t len, const void* buff);
int64_t ttyfs_sync(vfs_inode_t* this);
int64_t ttyfs_refresh(vfs_inode_t* this);
int64_t ttyfs_setlink(vfs_tnode_t* this, vfs_inode_t* target);
