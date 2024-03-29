#pragma once

#include "lib/vector.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// some limits
#define VFS_MAX_PATH_LEN 4096
#define VFS_MAX_NAME_LEN 256

typedef int vfs_handle_t;

// forward declaration
typedef struct _vfs_inode_t vfs_inode_t;
typedef struct _vfs_tnode_t vfs_tnode_t;

// stores type of node
typedef enum {
    VFS_NODE_FILE,
    VFS_NODE_FOLDER,
    VFS_NODE_PIPE,
    VFS_NODE_BLOCK_DEVICE,
    VFS_NODE_CHAR_DEVICE,
    VFS_NODE_MOUNTPOINT
} vfs_node_type_t;

typedef enum {
    VFS_MODE_READ,
    VFS_MODE_WRITE,
    VFS_MODE_READWRITE
} vfs_openmode_t;

// structure storing details about a fs format
typedef struct vfs_fsinfo_t {
    char name[16]; // name of the fs
    bool istemp; // is it a temporary filesystem

    vfs_inode_t* (*mount)(vfs_inode_t* device);
    int64_t (*mknode)(vfs_tnode_t* this);

    // return value is number of bytes read/written
    int64_t (*read)(vfs_inode_t* this, size_t offset, size_t len, void* buff);
    int64_t (*write)(vfs_inode_t* this, size_t offset, size_t len, const void* buff);

    // return value is -1 on error, 0 on success
    int64_t (*sync)(vfs_inode_t* this);
    int64_t (*refresh)(vfs_inode_t* this);
    int64_t (*setlink)(vfs_tnode_t* this, vfs_inode_t* target);
    int64_t (*ioctl)(vfs_inode_t* this, int64_t req_param, void* req_data);
} vfs_fsinfo_t;

struct _vfs_tnode_t {
    char name[VFS_MAX_NAME_LEN];
    vfs_inode_t* inode;
    vfs_inode_t* parent;
    vfs_tnode_t* sibling;
};

struct _vfs_inode_t {
    vfs_node_type_t type;
    size_t size;
    uint32_t perms;
    uint32_t uid;
    uint32_t refcount;
    vfs_fsinfo_t* fs;
    void* ident;
    vfs_tnode_t* mountpoint;
    vec_struct(vfs_tnode_t*) child;
};

// structure describing an open node
typedef struct {
    vfs_tnode_t* tnode;
    vfs_inode_t* inode;
    vfs_openmode_t mode;
    size_t seek_pos;
} vfs_node_desc_t;

// directory entry structure
typedef struct {
    vfs_node_type_t type;
    size_t record_len;
    char name[VFS_MAX_NAME_LEN];
} vfs_dirent_t;

void vfs_init();
void vfs_register_fs(vfs_fsinfo_t* fs);
vfs_fsinfo_t* vfs_get_fs(char* name);
void vfs_debug();

vfs_handle_t vfs_open(char* path, vfs_openmode_t mode);
int64_t vfs_create(char* path, vfs_node_type_t type);
int64_t vfs_close(vfs_handle_t handle);
int64_t vfs_seek(vfs_handle_t handle, size_t pos);
int64_t vfs_read(vfs_handle_t handle, size_t len, void* buff);
int64_t vfs_write(vfs_handle_t handle, size_t len, const void* buff);
int64_t vfs_chmod(vfs_handle_t handle, int32_t newperms);
int64_t vfs_link(char* oldpath, char* newpath);
int64_t vfs_unlink(char* path);
int64_t vfs_getdent(vfs_handle_t handle, vfs_dirent_t* dirent);
int64_t vfs_mount(char* device, char* path, char* fsname);
