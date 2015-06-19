/*
** yolofs.c for yolofs in yolofs
**
** Made by chauvo_t
** Login   <chauvo_t@epitech.net>
**
** Started on  Fri Jun 12 16:29:26 2015 chauvo_t
** Last update Fri Jun 19 18:47:46 2015 chauvo_t
*/

#include <linux/fs.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/pagemap.h>
#include <linux/parser.h>
#include <linux/printk.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/syscalls.h>

#include "yolofs.h"

static struct file_system_type yolofs_type = {
	.owner		= THIS_MODULE,
	.name		= "yolofs",
	.mount		= yolofs_mount,
	.kill_sb	= yolofs_kill_sb,
	.fs_flags	= FS_USERNS_MOUNT,
};

static struct super_operations const yolofs_super_ops = {
	.statfs		= simple_statfs,
	.drop_inode	= generic_delete_inode,
	.show_options	= generic_show_options,
	.put_super	= yolofs_put_super, /* Todo: see if relevant */
};

static const struct inode_operations yolofs_dir_inode_operations = {
	.create		= yolofs_create,
	.lookup		= simple_lookup,
	.link		= simple_link,
	.unlink		= simple_unlink,
	.symlink	= yolofs_symlink,
	.mkdir		= yolofs_mkdir,
	.rmdir		= simple_rmdir,
	.mknod		= yolofs_mknod,
	.rename		= simple_rename,
};

const struct file_operations yolofs_file_operations = {
	.read		= yolofs_read,
	.write		= yolofs_write,
	.open		= simple_open,
	.read_iter	= generic_file_read_iter,
	.write_iter	= generic_file_write_iter,
	.mmap		= generic_file_mmap,
	.fsync		= noop_fsync,
	.splice_read	= generic_file_splice_read,
	.splice_write	= iter_file_splice_write,
	.llseek		= generic_file_llseek,
};

const struct file_operations yolofs_dir_operations = {
        .open           = dcache_dir_open,
        .release        = dcache_dir_close,
        .llseek         = dcache_dir_lseek,
        .read           = generic_read_dir,
        .iterate        = yolofs_readdir,
        .fsync          = noop_fsync,
};

const struct inode_operations yolofs_file_inode_operations = {
	.setattr	= simple_setattr,
	.getattr	= simple_getattr,
};

static const struct address_space_operations yolofs_aops = {
	.readpage	= simple_readpage,
	.write_begin	= simple_write_begin,
	.write_end	= simple_write_end,
};

static ssize_t yolofs_read(struct file *file, char __user *buf,
			   size_t count, loff_t *ppos)
{
	struct file	*yolofile;
	char		name[512];

	name[0] = '\0';
	strcat(name, ((struct yolofs_args*)(file->f_inode->i_sb->s_fs_info))->dev_name);
	strcat(name, "/");
	strcat(name, file->f_path.dentry->d_name.name);
	yolofile = filp_open(name, O_RDWR | O_CREAT, 0777);
	if (IS_ERR(yolofile))
		return PTR_ERR(yolofile);
	count = kernel_read(yolofile, *ppos, buf, count);
	filp_close(yolofile, NULL);
	return count;
}

static ssize_t yolofs_write(struct file *file, const char __user *buf,
			    size_t count, loff_t *ppos)
{
	struct file	*yolofile;
	char		name[512];

	name[0] = '\0';
	strcat(name, ((struct yolofs_args*)(file->f_inode->i_sb->s_fs_info))->dev_name);
	strcat(name, "/");
	strcat(name, file->f_path.dentry->d_name.name);
	pr_info("name = %s\n", name);
	yolofile = filp_open(name, O_RDWR | O_CREAT, 0777);
	if (IS_ERR(yolofile))
		return PTR_ERR(yolofile);
	count = kernel_write(yolofile, buf, count, *ppos);
	filp_close(yolofile, NULL);
	return count;
}

static int yolofs_readdir(struct file *file, struct dir_context *ctx)
{
	return file->f_op->iterate(file, ctx);
}

static int yolofs_mknod(struct inode *dir, struct dentry *dentry,
		       umode_t mode, dev_t dev)
{
	struct inode * in = yolofs_make_inode(dir->i_sb, dir, mode, dev);

	if (!in)
		return -ENOSPC;
	d_instantiate(dentry, in);
	dget(dentry);	/* Extra count - pin the dentry in core */
	dir->i_mtime = dir->i_ctime = CURRENT_TIME;
	return 0;
}

static int yolofs_mkdir(struct inode * dir, struct dentry * dentry, umode_t mode)
{
	int ret;

	ret = yolofs_mknod(dir, dentry, mode | S_IFDIR, 0);
	if (!ret)
		inc_nlink(dir);
	return ret;
}

static int yolofs_create(struct inode *dir, struct dentry *dentry, umode_t mode, bool excl)
{
	return yolofs_mknod(dir, dentry, mode | S_IFREG, 0);
}

static int yolofs_symlink(struct inode * dir, struct dentry *dentry, const char * symname)
{
	struct inode	*in;
	int		error = -ENOSPC;
	int		l;

	in = yolofs_make_inode(dir->i_sb, dir, S_IFLNK | S_IRWXUGO, 0);
	if (!in)
		goto err;
	l = strlen(symname) + 1;
	error = page_symlink(in, symname, l);
	if (!error) {
		d_instantiate(dentry, in);
		dget(dentry);
		dir->i_mtime = dir->i_ctime = CURRENT_TIME;
	} else
		iput(in);
err:
	return error;
}

/*
 * i_ino: I believe inode number is for cache management. Everything works just
 * fine with it always equal to 1.
 */
static struct inode *yolofs_make_inode(struct super_block *sb,
				       const struct inode *dir, int mode, dev_t dev)
{
	struct inode *in = new_inode(sb);

	if (!in)
		goto err;
	in->i_ino = get_next_ino();
	in->i_mode = mode;
	in->i_atime = CURRENT_TIME;
	in->i_mtime = CURRENT_TIME;
	in->i_ctime = CURRENT_TIME;
	in->i_mapping->a_ops = &yolofs_aops;
	inode_init_owner(in, dir, mode);
	switch (mode & S_IFMT) {
	case S_IFREG:
		in->i_op = &yolofs_file_inode_operations;
		in->i_fop = &yolofs_file_operations;
		break;
	case S_IFDIR:
		in->i_op = &yolofs_dir_inode_operations;
		in->i_fop = &yolofs_dir_operations;
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
err:
	return in;
}

enum {
	Opt_rotator,
	Opt_err
};

static const match_table_t tokens = {
	{Opt_rotator, "rot=%d"},
	{Opt_err, NULL}
};

static int yolofs_parse_options(char *data, struct yolofs_args *opts)
{
	substring_t args[MAX_OPT_ARGS];
	int rotator;
	int token;
	char *opt;

	while ((opt = strsep(&data, ",")) != NULL) {
		if (*opt == '\0')
			continue;
		token = match_token(opt, tokens, args);
		switch (token) {
		case Opt_rotator:
			if (match_int(&args[0], &rotator))
				return -EINVAL;
			opts->rot = rotator;
			break;
		default:
			return -EINVAL;
		}
	}
	return 0;
}

static int yolofs_fill_super(struct super_block *sb, void *data, int silent)
{
	struct inode		*root = NULL;
	int			err;

	save_mount_options(sb, data); /* Needed for generic_show_options */
	if ((err = yolofs_parse_options(data, ((struct yolofs_args *)(sb->s_fs_info)))) != 0) {
		pr_info("err = %d\n", err);
		return err;
	}
	pr_info("rot = %d", ((struct yolofs_args *)(sb->s_fs_info))->rot);
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

struct dentry *mount_yolodev(struct file_system_type *fs_type,
			     int flags,
			     const char *dev_name,
			     void *data,
			     int (*fill_super)(struct super_block *, void *, int))
{
	struct super_block	*sb = sget(fs_type, NULL, set_anon_super, flags, NULL);
	struct yolofs_args	*args;
	int			error;

	if (IS_ERR(sb))
		return ERR_CAST(sb);
	if ((args = kzalloc(sizeof(*args), GFP_KERNEL)) == NULL)
		return ERR_CAST(sb);
	sb->s_fs_info = args;
	((struct yolofs_args *)(sb->s_fs_info))->dev_name = kstrdup(dev_name, GFP_KERNEL);
	pr_info("in mount_yolodev: dev_name = %s\n",
		((struct yolofs_args *)(sb->s_fs_info))->dev_name);
	error = fill_super(sb, data, flags & MS_SILENT ? 1 : 0);
	if (error) {
		deactivate_locked_super(sb);
		return ERR_PTR(error);
	}
	sb->s_flags |= MS_ACTIVE;
	return dget(sb->s_root);
}

/*
 * char const *dev: le nom du device sous forme de chemin (par exemple /dev/sdc1)
 */
static struct dentry *yolofs_mount(struct file_system_type *type, int flags,
				   char const *dev, void *data)
{
	struct dentry * const entry = mount_yolodev(type, flags, dev,
						    data, yolofs_fill_super);

	if (IS_ERR(entry))
		pr_err("yolofs mounting failed\n");
	else
		pr_info("yolofs mounted\n");
	return entry;
}

static void yolofs_kill_sb(struct super_block *sb)
{
	kfree(sb->s_fs_info);
	kill_litter_super(sb);
}

static void yolofs_put_super(struct super_block *sb)
{
	pr_info("yolofs_put_super called\n");
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
