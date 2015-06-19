/*
** yolofs.h for yolofs in yolofs
**
** Made by chauvo_t
** Login   <chauvo_t@epitech.net>
**
** Started on  Tue Jun 16 16:36:43 2015 chauvo_t
** Last update Fri Jun 19 18:42:40 2015 chauvo_t
*/

#ifndef YOLOFS_H_
# define YOLOFS_H_

# define YOLOFS_MAGIC 0xdeadb00b

struct yolofs_args
{
	const char	*dev_name;
	int		rot;
};

/*
 * mount_yolodev.c
 */
struct dentry *mount_yolodev(struct file_system_type *fs_type,
			     int flags,
			     const char *dev_name,
			     void *data,
			     int (*fill_super)(struct super_block *, void *, int));

/*
 * yolofs.c
 */
static ssize_t yolofs_read(struct file *file, char __user *buf, size_t n, loff_t *ppos);
static ssize_t yolofs_write(struct file *, const char __user *, size_t n, loff_t *ppos);
static int yolofs_mknod(struct inode *dir, struct dentry *dentry,
			umode_t mode, dev_t dev);
static int yolofs_mkdir(struct inode * dir, struct dentry * dentry,
			umode_t mode);
static int yolofs_create(struct inode *dir, struct dentry *dentry,
			 umode_t mode, bool excl);
static int yolofs_symlink(struct inode * dir, struct dentry *dentry,
			  const char * symname);
static struct inode *yolofs_make_inode(struct super_block *sb,
				       const struct inode *dir,
				       int mode,
				       dev_t dev);
static int yolofs_parse_options(char *data, struct yolofs_args *opts);
static int yolofs_fill_super(struct super_block *sb, void *data, int silent);
static struct dentry *yolofs_mount(struct file_system_type *type, int flags,
				   char const *dev, void *data);
static void yolofs_kill_sb(struct super_block *sb);
static void yolofs_put_super(struct super_block *sb);

#endif /* !YOLOFS_H_ */
