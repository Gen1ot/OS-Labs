#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/init.h>
#include <linux/printk.h>
#include <linux/proc_fs.h>
#include <linux/time.h>

MODULE_DESCRIPTION("Tsu lab");
MODULE_AUTHOR("x5113nc3x");
MODULE_LICENSE("GPL");

#define procfs_name "tsu"
static struct proc_dir_entry *our_proc_file = NULL;
static time64_t last_view_time;


static ssize_t procfile_read(
  struct file *file_pointer, char __user *buffer,
  size_t buffer_length, loff_t *offset
) {
  time64_t now = ktime_get_real_seconds();
  if (now - last_view_time <= 20) {
    pr_info("procfile don't work");
    return 0;
  }
  last_view_time = now;

  char msg[] = "Hello\n";
  size_t msg_len = strlen(msg);
  copy_to_user(buffer, msg, msg_len);
  pr_info("procfile read(%lld): %s\n", now, file_pointer->f_path.dentry->d_name.name);
  return msg_len;
} 

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0)
static const struct proc_ops proc_file_fops = {
  .proc_read = procfile_read,
};
#else
static const struct file_operations proc_file_fops = {
  .read = procfile_read,
};
#endif

int init_module(void)
{
    pr_info("Welcome to the Tomsk State University\n");
    last_view_time = ktime_get_real_seconds() - 30;
    our_proc_file = proc_create(
      procfs_name, 0644, NULL, &proc_file_fops);

    return 0;
}


void cleanup_module(void)
{
    proc_remove(our_proc_file);
    pr_info("Tomsk State University forever!\n");
}
