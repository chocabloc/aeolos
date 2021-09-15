#include "sys/panic.h"
#include "initrd.h"
#include "mm/mm.h"
#include "saf.h"
#include "fs/vfs/vfs.h"
#include "klog.h"
#include "memutils.h"

static char path_buff[1024];

static void initrd_helper(uint8_t* base, saf_node_hdr_t* from, size_t path_offset) {
    if (from->magic != SAF_MAGIC_NUMBER)
        kernel_panic("initrd format invalid or corrupt\n");

    // update the path
    size_t namelen = strlen(from->name);
    strcpy(from->name, path_buff + path_offset);

    if (from->flags & SAF_FLAG_ISFOLDER) {
        // create the node
        vfs_create(path_buff, VFS_NODE_FOLDER);
        saf_node_folder_t* folder = (saf_node_folder_t*)from;

        // add leading slash
        path_buff[path_offset + namelen] = '/';
        path_buff[path_offset + namelen + 1] = '\0';

        // process its children
        for(size_t i = 0; i < folder->num_children; i++)
            initrd_helper(base, (saf_node_hdr_t*)(base + folder->children[i]),
                          path_offset + namelen + 1);
    } else {
        // create the node
        vfs_create(path_buff, VFS_NODE_FILE);
        saf_node_file_t* file = (saf_node_file_t*)from;

        // open the file and write the data to it
        vfs_handle_t fh = vfs_open(path_buff, VFS_MODE_WRITE);
        vfs_write(fh, file->size, (void*)(base + file->addr));
        vfs_close(fh);
    }
}

void initrd_init(stv2_struct_tag_modules* mod) {
    if(strcmp(mod->modules[0].string, "initrd") != 0)
        kernel_panic("initrd not found\n");

    saf_node_hdr_t* root = (saf_node_hdr_t*)PHYS_TO_VIRT(mod->modules[0].begin);
    initrd_helper((uint8_t*)root, root, 0);

    // free memory used by the raw initrd data
    size_t initrd_size = mod->modules[0].end - mod->modules[0].begin;
    pmm_free(mod->modules[0].begin, NUM_PAGES(initrd_size));

    klog_ok("done\n");
}