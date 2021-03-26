#include "ramfs.h"
#include "../common.h"
#include "klog.h"
#include "kmalloc.h"
#include "memutils.h"

vfs_fsinfo_t ramfs = {
    .name = "ramfs",
    .istemp = true,
    .mount = ramfs_mount,
    .mknode = ramfs_mknode,
    .sync = ramfs_sync,
    .refresh = ramfs_refresh,
    .read = ramfs_read,
    .write = ramfs_write,
    .setlink = ramfs_setlink
};

typedef struct {
    size_t alloc_size;
    void* data;
} ramfs_ident_t;

// TODO: move bounds calculation from here to vfs.c
int64_t ramfs_read(vfs_inode_t* this, size_t offset, size_t len, void* buff)
{
    // out of bounds
    if (offset + len > this->size)
        return -1;

    ramfs_ident_t* id = (ramfs_ident_t*)this->ident;
    memcpy(((uint8_t*)id->data) + offset, buff, len);
    return 0;
}

int64_t ramfs_write(vfs_inode_t* this, size_t offset, size_t len, const void* buff)
{
    // out of bounds
    if (offset + len > this->size)
        return -1;

    ramfs_ident_t* id = (ramfs_ident_t*)this->ident;
    memcpy(buff, ((uint8_t*)id->data) + offset, len);
    return 0;
}

int64_t ramfs_sync(vfs_inode_t* this)
{
    ramfs_ident_t* id = (ramfs_ident_t*)this->ident;
    if (this->size > id->alloc_size) {
        id->alloc_size = this->size;
        id->data = kmrealloc(id->data, id->alloc_size);
    }
    return 0;
}

int64_t ramfs_setlink(vfs_tnode_t* this, vfs_inode_t* inode)
{
    // should the previous inode data be freed
    if (this->inode->refcount == 0) {
        ramfs_ident_t* id = (ramfs_ident_t*)this->inode->ident;
        if (id->data)
            kmfree(id->data);
        kmfree(id);
    }

    // point the tnode to the new inode
    this->inode = inode;
    return 0;
}

int64_t ramfs_refresh(vfs_inode_t* this __attribute__((unused))) { return 0; }

vfs_tnode_t* ramfs_mknode(vfs_inode_t* this, char* name, vfs_node_type_t type)
{
    // create identifying information
    ramfs_ident_t* id = (ramfs_ident_t*)kmalloc(sizeof(ramfs_ident_t));
    *id = (ramfs_ident_t) { .alloc_size = 0, .data = NULL };

    // create tnode and inode
    vfs_inode_t* inode = vfs_alloc_inode(type, 0777, 0, &ramfs, id, this->mountpoint);
    return vfs_alloc_tnode(name, inode, this);
}

vfs_inode_t* ramfs_mount(vfs_inode_t* at __attribute__((unused)))
{
    // create identifying information
    ramfs_ident_t* id = (ramfs_ident_t*)kmalloc(sizeof(ramfs_ident_t));
    *id = (ramfs_ident_t) { .alloc_size = 0, .data = NULL };

    // allocate the inode
    return vfs_alloc_inode(VFS_NODE_MOUNTPOINT, 0777, 0, &ramfs, id, NULL);
}
