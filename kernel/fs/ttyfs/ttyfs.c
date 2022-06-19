#include "ttyfs.h"
#include "../vfs/common.h"
#include "kmalloc.h"
#include "memutils.h"

#define MIN(x, y) ((x) < (y) ? (x) : (y))

#define TTY_BUFFER_SIZE 16

// filesystem information
vfs_fsinfo_t ttyfs = {
    .name = "ttyfs",
    .istemp = true,
    .mount = ttyfs_mount,
    .mknode = ttyfs_mknode,
    .sync = ttyfs_sync,
    .refresh = ttyfs_refresh,
    .read = ttyfs_read,
    .write = ttyfs_write,
    .setlink = ttyfs_setlink
};

typedef struct {
    char *buff_in, *buff_out;
    size_t in_start, in_size;
    size_t out_start, out_size;
} ttyfs_ident_t;

static ttyfs_ident_t* create_ident()
{
    ttyfs_ident_t* id = (ttyfs_ident_t*)kmalloc(sizeof(ttyfs_ident_t));
    *id = (ttyfs_ident_t) {
        .buff_in = kmalloc(TTY_BUFFER_SIZE),
        .buff_out = kmalloc(TTY_BUFFER_SIZE),
        .in_start = 0,
        .in_size = 0,
        .out_start = 0,
        .out_size = 0
    };
    return id;
}

int64_t ttyfs_read(vfs_inode_t* this, size_t offset, size_t len, void* buff)
{
    ttyfs_ident_t* id = this->ident;

    // no of bytes that should be read
    size_t rlen = MIN(len, id->in_size);

    // read the bytes, wrap around if needed
    for (size_t i = 0; i < rlen; i++) {
        size_t index = (id->in_start + i) % TTY_BUFFER_SIZE;
        ((char*)buff)[i] = id->buff_in[index];
    }

    // update start and size of buffer
    id->in_start += rlen;
    id->in_start %= TTY_BUFFER_SIZE;
    id->in_size -= rlen;

    // TODO: block if read less than len bytes
    return rlen;
}

int64_t ttyfs_write(vfs_inode_t* this, size_t offset, size_t len, const void* buff)
{
    ttyfs_ident_t* id = this->ident;

    // write bytes at end, wrap around if needed
    size_t end = id->out_start + id->out_size;
    for (size_t i = 0; i < len; i++) {
        size_t index = (end + i) % TTY_BUFFER_SIZE;
        id->buff_out[index] = ((char*)buff)[i];
    }

    // update start and size
    if (len > TTY_BUFFER_SIZE - id->out_size)
        id->out_start = (end + len) % TTY_BUFFER_SIZE;
    id->out_size = MIN(len + id->out_size, TTY_BUFFER_SIZE);

    return len;
}

int64_t ttyfs_sync(vfs_inode_t* this)
{
    (void)this;
    return 0;
}

int64_t ttyfs_setlink(vfs_tnode_t* this, vfs_inode_t* inode)
{
    (void)inode;
    (void)this;
    return 0;
}

int64_t ttyfs_refresh(vfs_inode_t* this)
{
    (void)this;
    return 0;
}

int64_t ttyfs_mknode(vfs_tnode_t* this)
{
    this->inode->ident = create_ident();
    return 0;
}

vfs_inode_t* ttyfs_mount(vfs_inode_t* at)
{
    (void)at;
    vfs_inode_t* ret = vfs_alloc_inode(VFS_NODE_MOUNTPOINT, 0777, 0, &ttyfs, NULL);
    return ret;
}
