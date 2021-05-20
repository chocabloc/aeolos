/*
    Miscellaneous VFS-related functions    
*/
#include "common.h"

// creates a node with specified type
int64_t vfs_create(char* path, vfs_node_type_t type)
{
    int64_t status = 0;
    lock_wait(&vfs_lock);

    vfs_tnode_t* node = path_to_node(path, CREATE | ERR_ON_EXIST, type);
    if (!node)
        status = -1;

    lock_release(&vfs_lock);
    return status;
}

// changes permissions of node
int64_t vfs_chmod(vfs_handle_t handle, int32_t newperms)
{
    vfs_node_desc_t* fd = handle_to_fd(handle);
    if (!fd)
        return -1;

    // opened in read only mode
    if (fd->mode == VFS_MODE_READ) {
        klog_err("opened as read-only\n");
        return -1;
    }

    // set new permissions and sync
    fd->inode->perms = newperms;
    fd->inode->fs->sync(fd->inode);
    return 0;
}

// mounts a block device with specified filesystem at a path
int64_t vfs_mount(char* device, char* path, char* fsname)
{
    lock_wait(&vfs_lock);

    // get the fs info
    vfs_fsinfo_t* fs = vfs_get_fs(fsname);
    if (!fs)
        goto fail;

    // get the block device if needed
    vfs_tnode_t* dev = NULL;
    if (!fs->istemp) {
        dev = path_to_node(device, NO_CREATE, 0);
        if (!dev)
            goto fail;
        if (dev->inode->type != VFS_NODE_BLOCK_DEVICE) {
            klog_err("%s is not a block device\n", device);
            goto fail;
        }
    }

    // get the node where it is to be mounted (should be an empty folder)
    vfs_tnode_t* at = path_to_node(path, NO_CREATE, 0);
    if (!at)
        goto fail;
    if (at->inode->type != VFS_NODE_FOLDER || at->inode->child) {
        klog_err("'%s' is not an empty folder\n", path);
        goto fail;
    }
    kmfree(at->inode);

    // mount the fs
    at->inode = fs->mount(dev ? dev->inode : NULL);
    at->inode->mountpoint = at;

    klog_info("mounted %s at %s as %s\n", device ? device : "<no-device>", path, fsname);
    lock_release(&vfs_lock);
    return 0;
fail:
    lock_release(&vfs_lock);
    return -1;
}
