#include "common.h"
#include "kmalloc.h"
#include "memutils.h"

vfs_tnode_t* vfs_alloc_tnode(char* name, vfs_inode_t* inode, vfs_inode_t* parent)
{
    vfs_tnode_t* tnode = (vfs_tnode_t*)kmalloc(sizeof(vfs_tnode_t));
    memset(tnode, 0, sizeof(vfs_tnode_t));
    memcpy(name, tnode->name, sizeof(tnode->name));
    tnode->inode = inode;
    tnode->parent = parent;
    return tnode;
}

vfs_inode_t* vfs_alloc_inode(vfs_node_type_t type, uint32_t perms, uint32_t uid,
    vfs_fsinfo_t* fs, void* ident, vfs_tnode_t* mountpoint)
{
    vfs_inode_t* inode = (vfs_inode_t*)kmalloc(sizeof(vfs_inode_t));
    memset(inode, 0, sizeof(vfs_inode_t));
    *inode = (vfs_inode_t) {
        .type = type,
        .perms = perms,
        .uid = uid,
        .fs = fs,
        .refcount = 1,
        .ident = ident,
        .mountpoint = mountpoint
    };
    return inode;
}

void vfs_free_tnode(vfs_tnode_t* tnode)
{
    vfs_inode_t* inode = tnode->inode;
    inode->refcount--;
    if (inode->refcount == 0)
        kmfree(inode);
    kmfree(tnode);
}
