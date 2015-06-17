/*
** yolofs.h for yolofs in yolofs
**
** Made by chauvo_t
** Login   <chauvo_t@epitech.net>
**
** Started on  Tue Jun 16 16:36:43 2015 chauvo_t
** Last update Tue Jun 16 18:11:48 2015 chauvo_t
*/

#ifndef YOLOFS_H_
# define YOLOFS_H_

# define YOLOFS_MAGIC 0xdeadb00b

static struct dentry	*yolofs_mount(struct file_system_type *type, int flags,
				      char const *dev, void *data);
static int		yolofs_fill_super(struct super_block *sb,
					  void *data, int silent);
static void		yolofs_put_super(struct super_block *sb);

#endif /* !YOLOFS_H_ */
