
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/input.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/pagevec.h>


#include <linux/gpio.h>
#include <linux/fs_struct.h>
#include <linux/path.h>
#include <mach/gpio.h>
#include <linux/mm_types.h>
#include "../../kernel/power/power.h"
#include "../../fs/mount.h"

#define GPIO_PK_MAJOR 214
#define DEVICE_NAME		"gpio_pk"
void start_enable_printk(void);
void finish_enable_printk(void);
#ifdef CONFIG_SCORE_FBDBG_FILE
int from_hib=0;
int major_fault_cnt=0;
int minor_fault_cnt=0;
int ubi_fault_cnt=0;
int skip_cnt1=0;
int skip_cnt2=0;
void hib_logging_fault(int major_fault)
{
    if (from_hib == 0) {
        skip_cnt1++;
        return ;
    }
    if (major_fault)
        major_fault_cnt++;
    else
        minor_fault_cnt++;
}
void hib_ubi_logging_fault(void)
{
    if (from_hib == 0) {
        skip_cnt2++;
        return ;
    }
    ubi_fault_cnt++;
}
void print_fault_info(void)
{
    start_enable_printk();
    printk(KERN_INFO " MAJOR [ %d ] minor [ %d ] \n", major_fault_cnt, minor_fault_cnt);
    printk(KERN_INFO " UBI FAULT [ %d ] \n", ubi_fault_cnt);
    printk(KERN_INFO " SKIP COUNT [ %d : %d ] \n", skip_cnt1, skip_cnt2);
    finish_enable_printk();
}
#ifdef CONFIG_SCORE_UBIDATA_STD_DEBUG
int score_dbg_info_enable_flag=0;
void score_dbg_info_print(struct task_struct *p)
{
    if (score_dbg_info_enable_flag==0)
        return ;
    start_enable_printk();
    printk(KERN_INFO "%d %d %7lukB %7lukB %s]\n"
           , p->pid, p->tgid, p->ubi_read/1024, p->ubifs_read/1024, p->comm);
    finish_enable_printk();
}
void score_dbg_info_enable(void)
{
    score_dbg_info_enable_flag=1;
}
void score_dbg_info_disable(void)
{
    score_dbg_info_enable_flag=0;
}
void score_dbg_info_add(int ubi_read, int ubifs_read)
{
    if (score_dbg_info_enable_flag==0)
        return ;
    current->ubi_read += ubi_read;
    current->ubifs_read += ubifs_read;
}
static
void score_dbg_info_print_all_tsk(void)
{
	struct task_struct *p, *g;
    unsigned long t_ubifs_read=0, t_ubi_read=0;
    start_enable_printk();
    printk(KERN_INFO "@@ -----------------------------------------------------\n");
    printk(KERN_INFO "@@ pid tgid  ubi_read  ubifs_read tsk\n");
    printk(KERN_INFO "@@ -----------------------------------------------------\n");
	do_each_thread(g, p) {
        printk(KERN_INFO "%d %d %7lukB %7lukB %s\n"
               , p->pid, p->tgid, p->ubi_read/1024, p->ubifs_read/1024, p->comm);
        t_ubifs_read += p->ubifs_read;
        t_ubi_read += p->ubi_read;
	} while_each_thread(g, p);
    printk(KERN_INFO "@@ --------------------------\n");
    printk(KERN_INFO "@@ Total      :   %lukB %lukB\n", t_ubifs_read/1024, t_ubi_read/1024);
    printk(KERN_INFO "@@ --------------------------\n");
    finish_enable_printk();
}
#endif
extern unsigned int i_hash_shift;
extern struct hlist_head *inode_hashtable __read_mostly;
#define MARK_STR    "<<"

extern struct dentry *mounted_dentry(struct dentry *dentry);

#define MIN_PATH_LEN 10
/* CAUTION!! recursive function */
static char *__dentry_name(char *path_buf, struct dentry *dentry, int len)
{
	struct dentry *parent;
    if (dentry == NULL)
    {
        len -= 4;
		strncpy(path_buf + len , "NULL" , 4);
        return path_buf + len;
    }
	parent = dentry;
	while (parent->d_parent != parent) {
		len -= parent->d_name.len + 1;
        if (len < MIN_PATH_LEN)
        {
            len -= 2;
            strncpy(path_buf + len , "@@" , 2);
            return path_buf + len;
        }
		path_buf[len] = '/';
		strncpy(path_buf + len + 1, parent->d_name.name,
			parent->d_name.len);
		parent = parent->d_parent;
        if (!parent)
        {
            printk(KERN_INFO "parent null\n");
            break;
        }
	}
    if (current->fs->root.dentry != parent)
    {
#if 0   /* mark mount point */
        len -= 3;
		strncpy(path_buf + len , "/##" , 3);
#endif
        return __dentry_name(path_buf, mounted_dentry(parent), len);
    }
	return path_buf + len;
}
static char _path_buf[4096];
static char *dentry_name(struct dentry *dentry)
{
    int len = PAGE_SIZE - 1;
    _path_buf[len] = 0;
    return __dentry_name(_path_buf, dentry, len);
}
static void print_inode_pages(struct inode *inode)
{
    struct address_space *mapping;
	pgoff_t next;
	struct pagevec pvec;
    int i;
    int start,end;
    start=end=-1;
    mapping = &inode->i_data;
    next=0;
    pagevec_init(&pvec, 0);
    for (;;)
    {
        if (!pagevec_lookup(&pvec, mapping, next, PAGEVEC_SIZE))
            break;
        for (i = 0; i < pagevec_count(&pvec); i++) {
            struct page *page = pvec.pages[i];
            pgoff_t page_index = page->index;
            next = page_index;
            if (start==-1)
            {
                start=end=page_index;
            }
            else if (end + 1 == page_index)
            {
                end = page_index;
            }
            else
            {
                if (start == end )
                {
                    printk(KERN_INFO MARK_STR "       %d \n", start);
                }
                else
                {
                    printk(KERN_INFO MARK_STR "       %d - %d \n", start, end);
                }
                start=end=page_index;
            }
        }
        next++;
        pagevec_release(&pvec);
    }
    if (start != -1)
    {
        if (start == end )
        {
            printk(KERN_INFO MARK_STR "       %d \n", start);
        }
        else
        {
            printk(KERN_INFO MARK_STR "       %d - %d \n", start, end);
        }
    }
}
static char *my_dentry_name(struct dentry *dentry)
{
    return dentry_name(dentry);
}
static void print_file_pages(int infos)
{
    int i;
    struct dentry *dentry;
	struct hlist_node *node;
	struct inode *inode = NULL;
	struct hlist_node *n;
    int total=0;
	for (i = 0; i < (1 << i_hash_shift); i++)
    {
        struct hlist_head *head =inode_hashtable + i;
	    hlist_for_each_entry(inode, node, head, i_hash) {
            hlist_for_each_entry(dentry, n, &inode->i_dentry, d_alias)
            {
                if (inode->i_data.nrpages > 0) {
                    total += inode->i_data.nrpages;
                    start_enable_printk();
                    printk(KERN_INFO MARK_STR " %lu : %s \n",  inode->i_data.nrpages, my_dentry_name(dentry));
                    print_inode_pages(inode);
                    finish_enable_printk();
                }
                break;
            }
        }
    }
    start_enable_printk();
    printk(KERN_INFO MARK_STR " TOTAL FILE PAGES [ %d ]\n", total);
    finish_enable_printk();
}
#endif
ssize_t gpio_pk_read(struct file *filp, char *buf, size_t count, loff_t *f_pos)
{
	return count;
}
int gpio_set_val=-1;
static void __test(const char *buf)
{
    switch (buf[0])
    {
    case 'p':
        switch (buf[1])
        {
        case '0':
            SCORE_GPIO_OFF();
            break;
        case '1':
            SCORE_GPIO_ON();
            break;
        case '3':
            if (gpio_set_val==1)
            {
                SCORE_GPIO_ON();
                //printk(KERN_INFO " LIVEVIEW ON\n");
            }
            else if (gpio_set_val==0)
            {
                SCORE_GPIO_OFF();
                //printk(KERN_INFO " LIVEVIEW ON\n");
            }
            else
            {
                //printk(KERN_INFO " LIVEVIEW ON\n");
            }

#ifdef CONFIG_SCORE_UBIDATA_STD_DEBUG
            if (score_dbg_info_enable_flag)
            {
                score_dbg_info_print_all_tsk();
                score_dbg_info_enable_flag=0;
            }
#endif
            gpio_set_val = -1;
            break;
        }
        break;
#ifdef CONFIG_SCORE_FBDBG_FILE
    case 'f':
        print_file_pages(buf[1]);
#endif
    default:
        break;
    }
}
ssize_t gpio_pk_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos)
{
    __test(buf);

    return count;
}

int gpio_pk_open(struct inode *inode, struct file *filp)
{
    return 0;
}


int gpio_pk_release(struct inode *inode, struct file *filp)
{
    return 0;
}


static struct file_operations gpio_pk_fops = {
read:
    gpio_pk_read,
write :
    gpio_pk_write,
open :
    gpio_pk_open,
release :
    gpio_pk_release,
};


int __init gpio_pk_init_module(void)
{
	int result;
	result = register_chrdev(GPIO_PK_MAJOR, DEVICE_NAME, &gpio_pk_fops);
	if (result < 0) {
		printk(KERN_WARNING "\n%s : Can't get Major Number [%d]\n", DEVICE_NAME, GPIO_PK_MAJOR);
		return result;
	}
	printk("Init madule, Succeed.\n");
	return 0;
}


void gpio_pk_cleanup_module(void)
{

}

module_init(gpio_pk_init_module);
module_exit(gpio_pk_cleanup_module);
