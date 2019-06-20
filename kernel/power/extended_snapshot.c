#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/suspend.h>
#include <linux/reboot.h>
#include <linux/string.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/mount.h>
#include <linux/pm.h>
#include <linux/console.h>
#include <linux/cpu.h>
#include <linux/freezer.h>
#include <linux/gfp.h>
#include <linux/swap.h>
#include <linux/swapops.h>
#include <linux/list.h>
#include <linux/pagevec.h>
#include <linux/fs_struct.h>
#include "../../fs/mount.h"
#include <linux/delay.h>
#include "power.h"
#include <linux/ctype.h>
#include <linux/slab.h>
#include <linux/mm.h>


/**********
 *
 * data format
 * header page
 *
 * last 4 bytes of a block is pointer for next block offset
 * last pgoff_t is (-1) and it's size is total page count of the file.
 *
 *********/
#ifndef EOF
#define EOF 0x8000
#endif
extern unsigned int i_hash_shift;       /* XXX */
extern struct hlist_head *inode_hashtable __read_mostly;    /* XXX */
extern spinlock_t inode_hash_lock;
static pgoff_t term_offset = -1;
static char page_list_file[256] = CONFIG_PM_SCORE_STD_SUB_PARTITION;
struct block_device *page_list_bdev=NULL;
static struct bio *w_bio=NULL;

#define MAGIC_EXDATA "PAGECACHELIST"
#define LEN_MAGIC   16
#define DATA_LEN_PER_PAGE   (PAGE_SIZE - sizeof(sector_t))
struct header {
    char magic[LEN_MAGIC];
    unsigned long file_count;
    unsigned long block_count;   /* total data block count
                                  * block size == PAGE_SIZE
                                  */
    sector_t next_sector;
    sector_t tail_sector;
} header ;

#define BOOT_NORMAL         'N'
#define BOOT_SNAPSHOT       'S'
#define BOOT_EXSNAPSHOT     'E'
extern void lock_inode_hash_lock(void);
extern void unlock_inode_hash_lock(void);
static char boot = BOOT_NORMAL;
static int exdata_used=0;
void set_exdata_used_flag(void)
{
    exdata_used=1;
}
void clear_exdata_used_flag(void)
{
    exdata_used=0;
}
void set_snapshot_boot_info(void)
{
    if (exdata_used)
        boot = BOOT_EXSNAPSHOT;
    else
        boot = BOOT_SNAPSHOT;
}

#ifdef CONFIG_SCORE_TEST
void start_enable_printk(void);
void finish_enable_printk(void);
#endif
static const unsigned long starter[2] = { 0xfffffffa, 0xfffffffe };
/*******************************/
/* write                       */
/* write sequentially for nand */
/*******************************/
static unsigned short page_list_swap = 0xffff;
sector_t page_list_block=0;
dev_t page_list_device;
/* TODO : CRC */

struct {
    unsigned long tot_file;
    char *buf;
    int  offset;
    sector_t sector;    /* record : current
                         * read   : next
                         */
    unsigned long tot_page;      /* total page per a file */
    unsigned long block_count;   /* read block count */
    unsigned long total_readahead;   /* total readahead pages */

    struct inode **inodes;
    int i_cnt;
    int i_arrsize;
} handle;
static sector_t get_swapdev_block(void)
{
    unsigned long offset;
	offset = swp_offset(get_swap_page_of_type(page_list_swap));
    if (!offset)
        return 0;
    return swapdev_block(page_list_swap, offset);
}
static void *get_free_page(void)
{
    void *src = (void *)__get_free_page(__GFP_WAIT | __GFP_NOWARN |
                                  __GFP_NORETRY);
    //get_page(virt_to_page(src));
    return src;
}
static int finish_recording(void)
{
    int ret;
    char *buf;
    *((sector_t *)(handle.buf + DATA_LEN_PER_PAGE)) = 0;

    ret = bio_write_page(page_list_bdev, handle.sector, handle.buf, &w_bio);
    if (ret)
        return ret;

    ret = hib_wait_on_bio_chain(&w_bio);
    if (ret)
        return ret;

    buf = (char *) get_free_page();
    if (!buf)
        return -ENOMEM;

    ret = bio_read_page(page_list_bdev, page_list_block, buf, NULL);
    if (ret)
    {
        free_page((unsigned long) buf);
        return ret;
    }
    memcpy(buf + (PAGE_SIZE - sizeof(header)), &header, sizeof(header));
    ret = bio_write_page(page_list_bdev, page_list_block, buf, NULL);
    free_page((unsigned long) buf);
    if (ret)
        return ret;

    return ioctl_by_bdev(page_list_bdev, BLKFLSBUF, 0);
}
static int init_record_data(void)
{
    void *buf;
    sector_t sector;

    buf = (char *) get_free_page();
    if (!buf)
        return -ENOMEM;
    header.block_count = 1;
    strncpy(header.magic, MAGIC_EXDATA, LEN_MAGIC);

    handle.inodes = kmalloc(PAGE_SIZE, GFP_KERNEL);
    if (!handle.inodes)
    {
        free_page((unsigned long) buf);
        return -ENOMEM;
    }
    handle.i_cnt = 0;
    handle.i_arrsize = PAGE_SIZE / sizeof(struct inode *);

    sector = get_swapdev_block();
    if (!sector)
    {
        free_page((unsigned long) buf);
        kfree(handle.inodes);
        handle.inodes=NULL;
        return -ENOSPC;
    }
    handle.buf = buf;
    handle.offset = 0;
    handle.sector = sector;
    handle.total_readahead = 0;
    header.tail_sector = sector;

    header.next_sector = sector;

    return 0;
}
static int record_alloc_new_block(void)
{
    void *buf;
    int ret;
    sector_t sector = get_swapdev_block();
    if (!sector)
        return -ENOSPC;
    *((sector_t *)(handle.buf + DATA_LEN_PER_PAGE)) = sector;

    ret = bio_write_page(page_list_bdev, handle.sector, handle.buf, &w_bio);
    if (ret)
        return ret;

    /* alloc page */
    buf = (char *) get_free_page();
    if (!buf)
        return -ENOMEM;
    header.block_count ++;

    /* init */
    handle.buf = buf;
    handle.offset = 0;
    handle.sector = sector;
    handle.tot_page = 0;

    if (sector > header.tail_sector)
        header.tail_sector = sector;

    return 0;
}
static int __record_data(void *data, int len)
{
    if (len == 0)
        return 0;
    memcpy(handle.buf + handle.offset, data, len);
    handle.offset += len;
    return 0;
}
/* padding space for alignment is not over than a page */
static int __record_data_align(void *data, int len)
{
    int align_len ;

    if (len == 0)
        return 0;

    align_len = round_up(len, 4);
    memcpy(handle.buf + handle.offset, data, len);
    handle.offset += align_len;
    return 0;
}
#define REMAIN_LEN()  (DATA_LEN_PER_PAGE - handle.offset)
static int record_data(char *data, int len)
{
    int ret;
    int remain_len = REMAIN_LEN();

    if (remain_len >= len)
        return __record_data(data, len);

    ret = __record_data(data, remain_len);
    if (ret)
        return ret;
    ret = record_alloc_new_block();
    if (ret)
        return ret;
    return __record_data(data + remain_len, len - remain_len);
}
static int record_data_align(char *data, int len)
{
    int ret;
    int remain_len = REMAIN_LEN();

    if (remain_len >= len)
        return __record_data_align(data, len);

    ret = __record_data(data, remain_len);
    if (ret)
        return ret;
    ret = record_alloc_new_block();
    if (ret)
        return ret;
    return __record_data_align(data + remain_len, len - remain_len);
}
static int record_new_file(char *fname)
{
    unsigned long fname_len = strlen(fname);
    int ret;

    if (fname_len <= 0)
        return -EINVAL;
    //printk(KERN_INFO "FNAME [%s]\n", fname);
    ret = record_data((char *)starter, sizeof(starter));
    if (ret)
        return ret;
    ret = record_data((char *)&fname_len, sizeof(unsigned long));
    if (ret)
        return ret;
    ret = record_data_align(fname, fname_len);
    if (ret)
        return ret;
    handle.tot_page = 0;
    return 0;
}
static int record_one_chunk_info(pgoff_t start, unsigned long count)
{
    int ret = record_data((char *)&start, sizeof(pgoff_t));
    if (ret)
        return ret;
    ret = record_data((char *)&count, sizeof(unsigned long));
    if (ret)
        return ret;
    handle.tot_page += count;
    handle.total_readahead += count;
    //printk(KERN_INFO "CHUNK [%ld:%ld]\n", start, count);
    return 0;
}
static int record_finish_one_file(void)
{
    int ret = record_data((char *)&term_offset, sizeof(pgoff_t));
    if (ret)
        return ret;
    ret = record_data((char *)&handle.tot_page, sizeof(unsigned long));
    if (ret)
        return ret;
    header.file_count++;

    return 0;
}
static int init_recording(void)
{
    int res;

    if (!strlen(page_list_file))
        return -EINVAL;

    page_list_device = name_to_dev_t(page_list_file);

    /* check if swap device is registered */
    res = swap_type_of(page_list_device, page_list_block,
			&page_list_bdev);
    if (res < 0)
        return res;
    page_list_swap = res;

	res = blkdev_get(page_list_bdev, FMODE_READ|FMODE_WRITE, NULL);
	if (res)
		return res;

	res = set_blocksize(page_list_bdev, PAGE_SIZE);
	if (res < 0)
        goto error;

    res = init_record_data();
    if (res)
        goto error;
    return 0;
error:
    blkdev_put(page_list_bdev, FMODE_READ|FMODE_WRITE);
    return res;
}

/**********/
/* read   */
/**********/
static int init_reading(void)
{
    int ret;

    if (!strlen(page_list_file))
        return -EINVAL;

    page_list_bdev = blkdev_get_by_path(page_list_file, FMODE_READ, NULL);
	if (IS_ERR(page_list_bdev))
        return -EINVAL;

	ret = set_blocksize(page_list_bdev, PAGE_SIZE);
	if (ret < 0)
        goto error;

    return 0;
error:
    blkdev_put(page_list_bdev, FMODE_READ);
    return ret;
}
static int start_read_page_list(void)
{
    int ret;
    char *buf;

    ret = init_reading();
    if (ret)
        return ret;

    ret = -ENOMEM;
    buf = (char *) get_free_page();
    if (!buf)
        goto error_blkdev_put;

    ret = bio_read_page(page_list_bdev, page_list_block, buf, NULL);
    if (ret)
        goto error;

    memcpy(&header, buf + (PAGE_SIZE - sizeof(header)), sizeof(header));

    ret = -EINVAL;
    if (strncmp(header.magic, MAGIC_EXDATA, LEN_MAGIC) != 0)
        goto error;

    ret = bio_read_page(page_list_bdev, header.next_sector, buf, NULL);
    if (ret)
        goto error;
    handle.block_count = 1;
    handle.buf = buf;
    handle.offset = 0;
    handle.sector = *((sector_t *)(handle.buf + DATA_LEN_PER_PAGE));
    handle.total_readahead = 0;

    set_exdata_used_flag();

    return 0;
error:
    free_page((unsigned long) buf);
error_blkdev_put:
    blkdev_put(page_list_bdev, FMODE_READ);
    return ret;
}
static int finish_read_page_list(void)
{
    free_page((unsigned long) handle.buf);
    blkdev_put(page_list_bdev, FMODE_READ);
    return 0;
}
static int read_next_block(void)
{
    int ret;

    if (header.block_count <=  handle.block_count)
        return -EOF;

    if (handle.sector == 0)
        return -EOF;

    ret = bio_read_page(page_list_bdev, handle.sector, handle.buf, NULL);
    if (ret)
        return ret;

    handle.offset = 0;
    handle.sector = *((sector_t *)(handle.buf + DATA_LEN_PER_PAGE));
    handle.block_count++;

    return 0;
}
static int read_fname_length(unsigned long *fname_len)
{
    int ret;
    if (REMAIN_LEN() < sizeof(unsigned long))
    {
        ret = read_next_block();
        if (ret)
            return ret;
    }
    *fname_len = *((unsigned long *) (handle.buf + handle.offset));
    handle.offset += sizeof(unsigned long);
    return 0;
}
static int __read_fname(char *buf, int fname_len)
{
    int ret;
    int remain_len = REMAIN_LEN();
    int align_len = round_up(fname_len, 4);

    if (remain_len >= fname_len)
    {
        memcpy(buf, handle.buf + handle.offset, fname_len);
        buf[fname_len] = 0;
        handle.offset += align_len;
        return 0;
    }
    if (remain_len > 0)
    {
        memcpy(buf, handle.buf + handle.offset, remain_len);
    }
    ret = read_next_block();
    if (ret)
        return ret;
    memcpy(buf + remain_len, handle.buf , fname_len - remain_len);
    buf[fname_len] = 0;
    handle.offset = align_len - remain_len;
    return 0;
}
static int read_data(char *buf, int len)
{
    int ret;
    int remain_len = REMAIN_LEN();

    if (remain_len >= len)
    {
        memcpy(buf, handle.buf + handle.offset, len);
        handle.offset += len;
        return 0;
    }
    if (remain_len > 0)
    {
        memcpy(buf, handle.buf + handle.offset, remain_len);
    }
    ret = read_next_block();
    if (ret)
        return ret;
    memcpy(buf + remain_len, handle.buf , len - remain_len);
    handle.offset = len - remain_len;
    return 0;
}
static int read_starter(void)
{
    unsigned long buf[2];
    int ret;

    for (;;)
    {
        ret = read_data((char *)buf, sizeof(buf));
        if (ret)
            return ret;
        if (starter[0] == buf[0] && starter[1] == buf[1])
            return 0;
    }
    /* not reach */
    return -EOF;
}
static int read_fname(char *buf, int buf_len)
{
    int ret;
    unsigned long fname_len;

    ret = read_starter();
    if (ret)
        return ret;

    ret = read_fname_length(&fname_len);
    if (ret)
        return ret;

    if (fname_len >= buf_len )
    {
        return -EFAULT;
    }
    return __read_fname(buf, fname_len);
}
static int read_one_chunk_info(pgoff_t *start, unsigned long *count)
{
    int ret;
    ret = read_data((char *)start, sizeof(pgoff_t));
    if (ret)
        return ret;
    return read_data((char *)count, sizeof(unsigned long));
}
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
        return __dentry_name(path_buf, mounted_dentry(parent), len);
    }
	return path_buf + len;
}
#define PATH_BUF_SIZE 4096
static char _path_buf[PATH_BUF_SIZE];
static char *dentry_name(struct dentry *dentry)
{
    int len = PAGE_SIZE - 1;
    _path_buf[len] = 0;
    return __dentry_name(_path_buf, dentry, len);
}
static int record_inode_pages(struct inode *inode, struct dentry *dentry)
{
    struct address_space *mapping;
	pgoff_t next;
	struct pagevec pvec;
    int i;
    int start,end;
    int ret;
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
                ret = record_new_file(dentry_name(dentry));
                if (ret)
                    return ret;
                start=end=page_index;
            }
            else if (end + 1 == page_index)
            {
                end = page_index;
            }
            else
            {
                ret = record_one_chunk_info(start, end - start + 1);
                if (ret)
                    return ret;
                start=end=page_index;
            }
        }
        next++;
        pagevec_release(&pvec);
    }
    if (start != -1)
    {
        ret = record_one_chunk_info(start, end - start + 1);
        if (ret)
            return ret;
        return record_finish_one_file();
    }
    return 0;
}
static void put_inodes(void)
{
    int i;
    for (i = 0; i < handle.i_cnt; i++)
    {
        iput(handle.inodes[i]);
    }
    handle.i_cnt = 0;
}
static int expand_inode_arr(void)
{
    int size;
    kfree(handle.inodes);
    handle.inodes = NULL;
    handle.i_arrsize *= 2;

    size = handle.i_arrsize * sizeof(struct inode *);
    handle.inodes = kmalloc(size, GFP_KERNEL);
    if (!handle.inodes)
    {
        return -ENOMEM;
    }
    return 0;
}
static int record_files_pages_a_bucket(struct hlist_head *head)
{
	struct inode *inode = NULL;
	struct hlist_node *node;
    int ret;
    int i;

    /* get inode list in this hash bucket */
retry:
    lock_inode_hash_lock();
    hlist_for_each_entry(inode, node, head, i_hash) {
        if (handle.i_cnt >= handle.i_arrsize)
        {
            /*
             * There are too many inodes in this bucket.
             * Buffer needs to be expanded.
             */
            unlock_inode_hash_lock();
            put_inodes();
            ret = expand_inode_arr();
            if (ret)
                return ret;
            goto retry;
        }
        inode = igrab(inode);
        if (inode)
        {
            handle.inodes[handle.i_cnt] = inode;
            handle.i_cnt++;
        }
    }
    unlock_inode_hash_lock();

    /* recording inodes in inode list */
    for (i = 0; i < handle.i_cnt; i++)
    {
        struct inode *inode = handle.inodes[i];
        struct dentry *dentry;
        struct hlist_node *n;

        spin_lock(&inode->i_lock);
        hlist_for_each_entry(dentry, n, &inode->i_dentry, d_alias)
        {break;}
        dget(dentry);
        spin_unlock(&inode->i_lock);

        if (dentry && inode->i_data.nrpages > 0) {
            ret = record_inode_pages(inode, dentry);
            if (ret)
            {
                dput(dentry);
                goto error;
            }
        }
        dput(dentry);
    }

    put_inodes();
    return 0;
error:
    put_inodes();
    return ret;
}
static int record_files_pages(void)
{
    int ret;
    int i;
	for (i = 0; i < (1 << i_hash_shift); i++)
    {
        struct hlist_head *head = inode_hashtable + i;
        ret = record_files_pages_a_bucket(head);
        if (ret)
            return ret;
    }
    return 0;
}
static void put_record_data(void)
{
    kfree(handle.inodes);
    blkdev_put(page_list_bdev, FMODE_READ|FMODE_WRITE);
}
int record_page_list(void)
{
    int ret;

	lock_system_sleep();
	ret = freeze_processes();
    if (ret)
        goto unlock;

	ret = freeze_kernel_threads();
	if (ret)
		goto Thaw;

    ret = init_recording();
    if (ret)
        goto k_Thaw;

    ret = record_files_pages();
    if (ret)
        goto error;

    ret = finish_recording();
    if (ret)
        goto error;

    put_record_data();

    clear_snapshot(); /* we can do nothing about this failure */

    kernel_restart(NULL);
    /* can't reach here */
    return -1;
error:
    put_record_data();
k_Thaw:
	thaw_kernel_threads();
Thaw:
	thaw_processes();
unlock:
	unlock_system_sleep();
    return ret;
}
extern ssize_t do_readahead(struct address_space *mapping, struct file *filp, pgoff_t index, unsigned long nr);
extern void fput(struct file *file);

static int readahead_one_file(const char *fname)
{
    int ret;
    pgoff_t start;
    unsigned long size;
    struct file *f = filp_open(fname, O_RDONLY, 0);
    if (IS_ERR(f))
    {
        return 0;
    }
    for (;;)
    {
        ret = read_one_chunk_info(&start, &size);
        if (ret)
            goto error;
        if (start == term_offset)
            break;
        //printk(KERN_INFO "CHUNK [%ld:%ld]\n", start, size);
        ret = do_readahead(f->f_mapping, f, start, size);
        if (ret)
        {
            goto error;
        }
        handle.total_readahead+= size;
    }
    /* TODO : check total page count */
    fput(f);
    return 0;
error:
    fput(f);
    return 0;
}
int snapshot_readahead(void)
{
    int ret ;
    int i;

    ret = start_read_page_list();
    if (ret)
        return ret;

    for (i = 0; i < header.file_count; i++)
    {
        ret = read_fname(_path_buf, PATH_BUF_SIZE);
        if (ret)
            goto error;
        //printk(KERN_INFO "FNAME [%s]\n", _path_buf);
        ret = readahead_one_file(_path_buf);
        if (ret)
            goto error;
    }

#ifdef CONFIG_SCORE_TEST
    start_enable_printk();
    printk(KERN_INFO "total readahead pages [%ld]\n", handle.total_readahead);
    finish_enable_printk();
#endif
    finish_read_page_list();
    return 0;
error:
    finish_read_page_list();
    return ret;
}
struct {
    /* registered value */
    char *addr;
    unsigned long len;

    /* internal value */
    unsigned long pfn;
    unsigned long cnt;
    unsigned long used;
} extra_info;
static ssize_t extra_show(struct kobject *kobj, struct kobj_attribute *attr,
			     char *buf)
{
    if (extra_info.addr)
        return sprintf(buf, "0x%p:%ld\n", extra_info.addr, extra_info.len);
    else
        return sprintf(buf, "%p:%ld\n", extra_info.addr, extra_info.len);
}
static int extra_pte_range(pte_t *pte, unsigned long addr, unsigned long end,
			   struct mm_walk *walk)
{
    unsigned long *pfn = walk->private;
    struct page *page;
    if (*pfn)
    {
#ifdef CONFIG_SCORE_TEST
        start_enable_printk();
        printk(KERN_INFO "pfn [%lu]\n", *pfn);
        finish_enable_printk();
#endif
        return 0;
    }
    page = pte_page(*pte);
    *pfn = page_to_pfn(page);
#ifdef CONFIG_SCORE_TEST
    start_enable_printk();
    printk(KERN_INFO "pfn [%lu]\n", *pfn);
    finish_enable_printk();
#endif
    return 0;
}
static
unsigned long pfn_from_user_addr(char *addr)
{
    unsigned long pfn=0;
	struct mm_walk extra_walk = {
		.pte_entry = extra_pte_range,
		.mm = current->mm,
        .private = &pfn,
	};
    walk_page_range((unsigned long)addr, (unsigned long)addr + PAGE_SIZE, &extra_walk);
    return pfn;
}
static
ssize_t extra_store(struct kobject *kobj, struct kobj_attribute *attr,
			      const char *buf, size_t n)
{
	unsigned long val;
    unsigned long pfn=0;
    char *kbuf;
    char *addr;
    int error = -EINVAL;
    char *p;

    kbuf = kstrndup(buf, n, GFP_KERNEL);
    if (!kbuf)
    {
        return -ENOMEM;
    }
    p = memchr(kbuf, ':', n);
    if (!p)
    {
        kfree(kbuf);
        return error;
    }
    *p=0;

    if (strict_strtoul(kbuf, 0x10, &val))
    {
        kfree(kbuf);
        return error;
    }
    addr = (char *)val;

    if (strict_strtoul(p + 1, 10, &val))
    {
        kfree(kbuf);
        return error;
    }
    pfn = pfn_from_user_addr(addr);
    if (!pfn)
        return error;

    /* TODO : area is in CMA ? */
    extra_info.addr = addr;
    extra_info.len = val;
    extra_info.pfn = pfn;
    extra_info.cnt = val >> PAGE_SHIFT;
    extra_info.used = 0;
    kfree(kbuf);
	return n;
}
static
ssize_t boot_show(struct kobject *kobj, struct kobj_attribute *attr,
			     char *buf)
{
    return sprintf(buf, "%c\n", boot);
}
static
ssize_t boot_store(struct kobject *kobj, struct kobj_attribute *attr,
			      const char *buf, size_t n)
{
    return -EIO;
}
power_attr(extra);
power_attr(boot);
static struct attribute * g[] = {
	&extra_attr.attr,
	&boot_attr.attr,
	NULL,
};


static struct attribute_group attr_group = {
	.attrs = g,
};
static int __init pm_extra_init(void)
{
	return sysfs_create_group(power_kobj, &attr_group);
}
unsigned long reserved_extra_size(void)
{
    return extra_info.len / PAGE_SIZE;
}
struct page *alloc_image_page_extra(void)
{
    unsigned long pfn;
    if (!extra_info.addr)
        return NULL;
    if (extra_info.used >= extra_info.cnt)
        return NULL;
    pfn = extra_info.pfn + extra_info.used;
    extra_info.used++;
    return pfn_to_page(pfn);
}
core_initcall(pm_extra_init);
