#include "common.h"
#include "kmalloc.h"
#include "memutils.h"

// allocates a tnode in memory
vfs_tnode_t* vfs_alloc_tnode(char* name, vfs_inode_t* inode, vfs_inode_t* parent)
{
    vfs_tnode_t* tnode = (vfs_tnode_t*)kmalloc(sizeof(vfs_tnode_t));
    memset(tnode, 0, sizeof(vfs_tnode_t));
    memcpy(name, tnode->name, sizeof(tnode->name));
    tnode->inode = inode;
    tnode->parent = parent;
    return tnode;
}

// allocates an inode in memory
vfs_inode_t* vfs_alloc_inode(vfs_node_type_t type, uint32_t perms, uint32_t uid,
    vfs_fsinfo_t* fs, vfs_tnode_t* mountpoint)
{
    vfs_inode_t* inode = (vfs_inode_t*)kmalloc(sizeof(vfs_inode_t));
    memset(inode, 0, sizeof(vfs_inode_t));
    *inode = (vfs_inode_t) {
        .type = type,
        .perms = perms,
        .uid = uid,
        .fs = fs,
        .ident = NULL,
        .mountpoint = mountpoint,
        .refcount = 1
    };
    return inode;
}

// frees a tnode, and the inode if needed
void vfs_free_nodes(vfs_tnode_t* tnode)
{
    vfs_inode_t* inode = tnode->inode;
    if (inode->refcount <= 0)
        kmfree(inode);
    kmfree(tnode);
}
