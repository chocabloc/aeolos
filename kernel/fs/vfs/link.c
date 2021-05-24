/*
    Functions related to node linking/unlinking
*/

#include "common.h"

int64_t vfs_link(char* oldpath, char* newpath)
{
    lock_wait(&vfs_lock);

    // get the old node
    vfs_tnode_t* old_tnode = path_to_node(oldpath, NO_CREATE, 0);
    if (!old_tnode)
        goto fail;
    vfs_inode_t* old_inode = old_tnode->inode;

    // create the new node
    vfs_tnode_t* new_tnode = path_to_node(newpath, CREATE | ERR_ON_EXIST, old_tnode->inode->type);
    if (!new_tnode)
        goto fail;
    vfs_inode_t* new_inode = new_tnode->inode;

    // the mountpoints of the nodes must match
    if (new_inode->mountpoint != old_inode->mountpoint) {
        klog_err("mountpoints do not match\n");
        new_inode->fs->setlink(new_tnode, NULL);
        vfs_free_nodes(new_tnode);
        goto fail;
    }

    // link the two nodes
    old_inode->refcount++;
    new_inode->refcount = 0;
    old_inode->fs->setlink(new_tnode, old_inode);
    new_tnode->inode = old_inode;

    // free the new inode
    kmfree(new_inode);

    lock_release(&vfs_lock);
    return 0;
fail:
    lock_release(&vfs_lock);
    return -1;
}

int64_t vfs_unlink(char* path)
{
    lock_wait(&vfs_lock);
    vfs_tnode_t* tnode = path_to_node(path, NO_CREATE, 0);
    if (!tnode)
        goto fail;

    if (tnode->inode->child.len != 0) {
        klog_err("target not an empty folder\n");
        goto fail;
    }

    // decrease refcount and unlink
    tnode->inode->refcount--;
    int64_t status = tnode->inode->fs->setlink(tnode, NULL);

    // remove the tnode from the parent
    vfs_inode_t* parent = tnode->parent;
    vec_erase_val(&(parent->child), tnode);

    // free the node data
    vfs_free_nodes(tnode);

    lock_release(&vfs_lock);
    return status;
fail:
    lock_release(&vfs_lock);
    return -1;
}
