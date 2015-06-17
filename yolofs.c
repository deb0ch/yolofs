/*
** yolofs.c for yolofs in yolofs
**
** Made by chauvo_t
** Login   <chauvo_t@epitech.net>
**
** Started on  Fri Jun 12 16:29:26 2015 chauvo_t
** Last update Wed Jun 17 16:13:22 2015 chauvo_t
*/

#include <linux/fs.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/pagemap.h>
#include <linux/printk.h>

#include "yolofs.h"

static struct file_system_type yolofs_type = {
	.owner = THIS_MODULE,
	.name = "yolofs",
	.mount = yolofs_mount,
	.kill_sb = kill_block_super,
	.fs_flags = FS_USERNS_MOUNT,
};

static struct super_operations const yolofs_super_ops = {
	.statfs		= simple_statfs,
	.drop_inode	= generic_delete_inode,
	.show_options	= generic_show_options,
	.put_super = yolofs_put_super, /* Todo: see if relevant */
};

static struct inode *yolofs_make_inode(struct super_block *sb,
				       const struct inode *dir, int mode, dev_t dev)
{
	struct inode *in = new_inode(sb);

	if (!in)
		goto err_yolofs_make_inode;
	in->i_ino = 0;
	in->i_mode = mode;
	in->i_atime = CURRENT_TIME;
	in->i_mtime = CURRENT_TIME;
	in->i_ctime = CURRENT_TIME;
	inode_init_owner(in, dir, mode);
	switch (mode & S_IFMT) {
	case S_IFREG:
		in->i_op = &simple_dir_inode_operations;
		in->i_fop = &simple_dir_operations;
		break;
	case S_IFDIR:
		in->i_op = &simple_dir_inode_operations;
		in->i_fop = &simple_dir_operations;
		/* directory inodes start off with i_nlink == 2 (for ".") */
		inc_nlink(in);
		break;
	case S_IFLNK:
		in->i_op = &page_symlink_inode_operations;
		break;
	default:
		init_special_inode(in, mode, dev);
		break;
	}
err_yolofs_make_inode:
	return in;
}

static int yolofs_fill_super(struct super_block *sb, void *data, int silent)
{
	struct inode *root = NULL;

	save_mount_options(sb, data); /* Needed for generic_show_options */
	sb->s_magic		= YOLOFS_MAGIC;
	sb->s_op		= &yolofs_super_ops;
	sb->s_maxbytes		= MAX_LFS_FILESIZE;
	sb->s_blocksize		= PAGE_CACHE_SIZE;
	sb->s_blocksize_bits	= PAGE_CACHE_SHIFT;
	sb->s_time_gran		= 1;
	root = yolofs_make_inode(sb, NULL, S_IFDIR, 0);
	if (!root) {
		pr_err("inode allocation failed\n");
		return -ENOMEM;
	}
	sb->s_root = d_make_root(root);
	if (!sb->s_root) {
		pr_err("root creation failed\n");
		return -ENOMEM;
	}
	return 0;
}

static void yolofs_put_super(struct super_block *sb)
{
	pr_info("yolofs_put_super called\n");
}

static struct dentry *yolofs_mount(struct file_system_type *type, int flags,
				   char const *dev, void *data)
{
	struct dentry * const entry = mount_nodev(type, flags,
						  data, yolofs_fill_super);

	(void)dev;
	if (IS_ERR(entry))
		pr_err("yolofs mounting failed\n");
	else
		pr_info("yolofs mounted\n");
	return entry;
}

static int __init yolo_init(void)
{
	int	ret;

	ret = register_filesystem(&yolofs_type);
	if (ret) {
		pr_err("yolofs init failure\n");
		return ret;
	}
	pr_info("Yolo, World ! yolofs init successful\n");
	return 0;
}

static void __exit yolo_exit(void)
{
	if (unregister_filesystem(&yolofs_type)) {
		pr_err("yolofs exit failure\n");
		return;
	}
	pr_info("Goodbye, World ! yolofs exit successful\n");
}

module_init(yolo_init);
module_exit(yolo_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Yolofs is just a simple useless filesystem.");
MODULE_AUTHOR("Thomas de Beauchene");
