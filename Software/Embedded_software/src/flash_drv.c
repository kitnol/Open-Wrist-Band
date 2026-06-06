#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/fs/fs.h>
#include <zephyr/fs/littlefs.h>
#include <zephyr/storage/flash_map.h>
#include <zephyr/logging/log.h>
#include <zephyr/logging/log_ctrl.h>
LOG_MODULE_REGISTER(flash_drv, LOG_LEVEL_DBG);

#define MKFS_FS_TYPE FS_LITTLEFS
#define MKFS_DEV_ID FIXED_PARTITION_ID(external_flash)
#define MKFS_FLAGS 0

// /* LittleFS configuration */
// static struct fs_littlefs lfs_cfg = {
//     .cfg = {
//         .read_size = 16,
//         .prog_size = 256,
//         .cache_size = 256,
//         .lookahead_size = 32,
//         .block_cycles = 512
//     },
// };

/* Mount structure for LittleFS */
FS_LITTLEFS_DECLARE_DEFAULT_CONFIG(storage);
static struct fs_mount_t lfs_mnt = {
    .type = FS_LITTLEFS,
    .mnt_point = "/lfs",
    .storage_dev = (void *)FLASH_AREA_ID(external_flash),
    .fs_data = &storage,
};

int init_fs(void)
{
    LOG_INF("Initializing LittleFS on external flash...");
    log_flush();
    
    /* Get the flash device */
    const struct device *flash_dev = DEVICE_DT_GET(DT_NODELABEL(is25wp01g));
    if (!device_is_ready(flash_dev)) {
        LOG_ERR("Flash device not ready");
        return -ENODEV;
    }
    LOG_INF("Flash device ready: %s", flash_dev->name);
    log_flush();
    
    // /* Get the flash area for the littlefs partition */
    
    
    // /* Configure the LittleFS backend with the flash area */
    // lfs_cfg.backend = (void *)fa;
    
    // /* Set the flash device for mounting */
    // mp->storage_dev = (void *)flash_dev;
    
    /* Try to mount the existing filesystem */
    int rc = fs_mount(&lfs_mnt);
    LOG_INF("Mount attempt result: %d", rc);
    log_flush();
    
    /* If mount failed, format and retry */
    if (rc == -ENOENT || rc == -EINVAL) {
        LOG_WRN("No valid filesystem found (%d), formatting...", rc);
        log_flush();
        // const struct flash_area *fa;
        // int rc = flash_area_open(FLASH_AREA_ID(external_flash), &fa);
        // if (rc < 0) {
        //     LOG_ERR("Failed to open flash area: %d", rc);
        //     return rc;
        // }
        
        // LOG_INF("Flash area opened: offset=0x%lx, size=0x%x", fa->fa_off, (unsigned int)fa->fa_size);
        // log_flush();
        //rc = fs_mkfs(FS_LITTLEFS, (uintptr_t)fa, NULL, 0);
        rc = fs_mkfs(MKFS_FS_TYPE, MKFS_DEV_ID, NULL, MKFS_FLAGS);
        if (rc < 0) {
            LOG_ERR("Format failed: %d", rc);
            return rc;
        }
        LOG_INF("Filesystem formatted successfully");
        log_flush();
        
        /* Retry mount after formatting */
        rc = fs_mount(&lfs_mnt);
        LOG_INF("Mount after format result: %d", rc);
        log_flush();
    }
    
    if (rc < 0) {
        LOG_ERR("Mount failed: %d", rc);
        return rc;
    }

    LOG_INF("LittleFS mounted successfully at %s", lfs_mnt.mnt_point);
    log_flush();
    return 0;
}
// /* Write data to a file on the already-mounted /lfs partition */
int lfs_write_file(const char *path, const void *data, size_t len)
{
    struct fs_file_t file;
    fs_file_t_init(&file);

    int rc = fs_open(&file, path, FS_O_CREATE | FS_O_WRITE | FS_O_TRUNC);
    if (rc < 0) {
        LOG_ERR("Failed to open %s: %d", path, rc);
        return rc;
    }

    rc = fs_write(&file, data, len);
    if (rc < 0) {
        LOG_ERR("Failed to write %s: %d", path, rc);
    }

    fs_close(&file);
    return rc;
}
// /* Append data to a file on the already-mounted /lfs partition */
int lfs_append_file(const char *path, const void *data, size_t len)
{
    struct fs_file_t file;
    fs_file_t_init(&file);

    int rc = fs_open(&file, path, FS_O_CREATE | FS_O_WRITE | FS_O_APPEND);
    if (rc < 0) {
        LOG_ERR("Failed to open %s: %d", path, rc);
        return rc;
    }

    rc = fs_write(&file, data, len);
    if (rc < 0) {
        LOG_ERR("Failed to write %s: %d", path, rc);
    }

    fs_close(&file);
    return rc;
}

/* Read data from a file on the already-mounted /lfs partition */
int lfs_read_file(const char *path, void *buf, size_t len)
{
    struct fs_file_t file;
    fs_file_t_init(&file);

    int rc = fs_open(&file, path, FS_O_READ);
    if (rc < 0) {
        LOG_ERR("Failed to open %s: %d", path, rc);
        log_flush();
        return rc;
    }

    rc = fs_read(&file, buf, len);
    if (rc < 0) {
        LOG_ERR("Failed to read %s: %d", path, rc);
        log_flush();

    }

    fs_close(&file);
    return rc;
}
/* Read data from a file on the already-mounted /lfs partition */
int lfs_read_file_partial(struct fs_file_t *file, void *buf, size_t len)
{
    int rc = fs_read(file, buf, len);
    if (rc < 0) {
        LOG_ERR("Failed to partial read %d", rc);
        log_flush();

    }
    return rc;
}

int dirls(const char *path)
{
	int res;
	struct fs_dir_t dirp;
	static struct fs_dirent entry;

	fs_dir_t_init(&dirp);

	/* Verify fs_opendir() */
	res = fs_opendir(&dirp, path);
	if (res) {
		LOG_ERR("Error opening dir %s [%d]\n", path, res);
		return res;
	}

	LOG_PRINTK("\nListing dir %s ...\n", path);
	for (;;) {
		/* Verify fs_readdir() */
		res = fs_readdir(&dirp, &entry);

		/* entry.name[0] == 0 means end-of-dir */
		if (res || entry.name[0] == 0) {
			if (res < 0) {
				LOG_ERR("Error reading dir [%d]\n", res);
			}
			break;
		}

		if (entry.type == FS_DIR_ENTRY_DIR) {
			LOG_PRINTK("[DIR ] %s\n", entry.name);
		} else {
			LOG_PRINTK("[FILE] %s (size = %zu)\n",
				   entry.name, entry.size);
		}
	}

	/* Verify fs_closedir() */
	fs_closedir(&dirp);

	return res;
}