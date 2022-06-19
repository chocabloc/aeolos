/*
    Functions related to reading and writing of files
*/

#include "common.h"

// only block devices and files have a fixed size
#define HAS_SIZE(n) ((n)->type == VFS_NODE_BLOCK_DEVICE || (n)->type == VFS_NODE_FILE)

// read specified number of bytes from a file
int64_t vfs_read(vfs_handle_t handle, size_t len, void* buff)
{
    vfs_node_desc_t* fd = handle_to_fd(handle);
    if (!fd)
        return 0;

    lock_wait(&vfs_lock);
    vfs_inode_t* inode = fd->inode;

    // truncate if asking for more data than available,
    // and the file has a size (i.e. not a pipe or char device)
    if (fd->seek_pos + len > inode->size && HAS_SIZE(inode)) {
        len = inode->size - fd->seek_pos;
        if (len == 0)
            goto end;
    }

    int64_t len_read = fd->inode->fs->read(fd->inode, fd->seek_pos, len, buff);
    fd->seek_pos += len_read;

end:
    lock_release(&vfs_lock);
    return (int64_t)len;
}

// write specified number of bytes to file
int64_t vfs_write(vfs_handle_t handle, size_t len, const void* buff)
{
    vfs_node_desc_t* fd = handle_to_fd(handle);
    if (!fd)
        return 0;

    // we cannot write to read-only files
    if (fd->mode == VFS_MODE_READ) {
        klog_err("file is read only\n");
        return 0;
    }

    lock_wait(&vfs_lock);
    vfs_inode_t* inode = fd->inode;

    // expand file if writing more data than its size,
    // and the file has a size (i.e. not a pipe or char device)
    if (fd->seek_pos + len > inode->size && HAS_SIZE(inode)) {
        inode->size = fd->seek_pos + len;
        inode->fs->sync(inode);
    }

    int64_t len_written = inode->fs->write(inode, fd->seek_pos, len, buff);
    fd->seek_pos += len_written;

    lock_release(&vfs_lock);
    return (int64_t)len;
}

// seek to specified position in file
int64_t vfs_seek(vfs_handle_t handle, size_t pos)
{
    vfs_node_desc_t* fd = handle_to_fd(handle);
    if (!fd)
        return -1;

    // seek position is out of bounds and mode is read only
    if (pos >= fd->inode->size && fd->mode == VFS_MODE_READ) {
        klog_err("seek position out of bounds\n");
        return -1;
    }

    fd->seek_pos = pos;
    return 0;
}
