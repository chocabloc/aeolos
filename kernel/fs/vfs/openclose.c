/*
    Functions related to opening and closing of nodes
*/
#include "common.h"

// open a node and return a handle to it
vfs_handle_t vfs_open(char* path, vfs_openmode_t mode)
{
    lock_wait(&vfs_lock);

    // find the node
    vfs_tnode_t* req = path_to_node(path, NO_CREATE, 0);
    if (!req)
        goto fail;
    req->inode->refcount++;

    // create node descriptor
    vfs_node_desc_t* nd = (vfs_node_desc_t*)kmalloc(sizeof(vfs_node_desc_t));
    nd->tnode = req;
    nd->inode = req->inode;
    nd->file_pos = 0;
    nd->mode = mode;

    // add to current task
    task_t* curr = sched_get_current();
    vector_push_back(&(curr->openfiles), nd);

    // return the handle
    lock_release(&vfs_lock);
    return ((vfs_handle_t)(curr->openfiles.len - 1));
fail:
    lock_release(&vfs_lock);
    return -1;
}

// close a node, given its handle
int64_t vfs_close(vfs_handle_t handle)
{
    lock_wait(&vfs_lock);

    // get current task
    task_t* curr = sched_get_current();

    // get the file descriptor
    vfs_node_desc_t* fd = handle_to_fd(handle);
    if (!fd)
        goto fail;

    // ...and free it
    fd->inode->refcount--;
    kmfree(fd);
    curr->openfiles.data[handle] = NULL;

    lock_release(&vfs_lock);
    return 0;
fail:
    lock_release(&vfs_lock);
    return -1;
}
