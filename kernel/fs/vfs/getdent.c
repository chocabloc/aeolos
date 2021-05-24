/*
    vfs_getdent() function
*/
#include "common.h"

// get next directory entry
int64_t vfs_getdent(vfs_handle_t handle, vfs_dirent_t* dirent) {
    int64_t status;
    vfs_node_desc_t* fd = handle_to_fd(handle);
    if (!fd)
        return -1;

    lock_wait(&vfs_lock);

    // can only traverse folders
    if (!IS_TRAVERSABLE(fd->inode)) {
        klog_err("node not traversable\n");
        status = -1;
        goto done;
    }

    // we've reached the end
    if (fd->seek_pos >= fd->inode->child.len) {
        status = 0;
        goto done;
    }

    // initialize the dirent
    vfs_tnode_t* entry = vec_at(&(fd->inode->child), fd->seek_pos);
    dirent->type = entry->inode->type;
    memcpy(entry->name, dirent->name, sizeof(entry->name));

    // we're done here, advance the offset
    status = 1;
    fd->seek_pos++;

done:
    lock_release(&vfs_lock);
    return status;
}