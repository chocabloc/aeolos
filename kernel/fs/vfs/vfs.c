#include "vfs.h"
#include "common.h"
#include "fs/ramfs/ramfs.h"
#include "klog.h"
#include "kmalloc.h"
#include "lock.h"
#include "memutils.h"
#include "vector.h"

// vfs-wide lock
lock_t vfs_lock;

// the root node
vfs_tnode_t vfs_root;

// list of installed filesystems
vec_new_static(vfs_fsinfo_t*, vfs_fslist);

static void dumpnodes_helper(vfs_tnode_t* from, int lvl)
{
    for (int i = 0; i < 7 + lvl; i++)
        klog_putchar(' ');
    klog_printf(" %d: %s -> %x inode, (%d refs)\n", lvl, from->name, from->inode, from->inode->refcount);

    if (IS_TRAVERSABLE(from->inode))
        for (size_t i = 0; i < from->inode->child.len; i++)
            dumpnodes_helper(vec_at(&(from->inode->child), i), lvl + 1);
}

void vfs_debug()
{
    klog_info("dumping nodes\n");
    dumpnodes_helper(&vfs_root, 0);
    klog_printf("\n");
}

void vfs_register_fs(vfs_fsinfo_t* fs)
{
    vec_push_back(&vfs_fslist, fs);
}

// get fs with specified name
vfs_fsinfo_t* vfs_get_fs(char* name)
{
    for (size_t i = 0; i < vfs_fslist.len; i++)
        if (strncmp(name, vfs_fslist.data[i]->name, sizeof(((vfs_fsinfo_t) { 0 }).name)) == 0)
            return vfs_fslist.data[i];

    klog_err("filesystem %s not found\n", name);
    return NULL;
}

void vfs_init()
{
    klog_warn("partial stub\n");

    // initialize the root folder and mount ramfs there
    vfs_root.inode = vfs_alloc_inode(VFS_NODE_FOLDER, 0777, 0, NULL, NULL);
    vfs_register_fs(&ramfs);
    vfs_mount(NULL, "/", "ramfs");

    klog_ok("done\n");
}
