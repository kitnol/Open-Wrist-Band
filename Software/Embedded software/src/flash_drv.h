#ifndef FLASH_DRV_H
#define FLASH_DRV_H

int init_fs(void);
int lfs_write_file(const char *path, const void *data, size_t len);
int lfs_read_file(const char *path, void *buf, size_t len);
int lfs_append_file(const char *path, const void *data, size_t len);
int lfs_read_file_partial(struct fs_file_t *file, void *buf, size_t len);
int is_littlefs_signature_present(const struct flash_area *fa);
int dirls(const char *path);
#endif