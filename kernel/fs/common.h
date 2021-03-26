#pragma once

#include "vfs.h"

vfs_tnode_t* vfs_alloc_tnode(char* name, vfs_inode_t* inode, vfs_inode_t* parent);
vfs_inode_t* vfs_alloc_inode(vfs_node_type_t type, uint32_t perms, uint32_t uid,
    vfs_fsinfo_t* fs, void* ident, vfs_tnode_t* mountpoint);
