#include <linux/init.h>
#include <linux/module.h>
#include <linux/vmalloc.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/fs.h>

#define POLICY_FILE_PATH "policy"
#define START_MARKER "----------------dynamic_measure_start"
#define END_MARKER "----------------dynamic_measure_end"

struct LSM_Policy
{
};

struct Cycle_Policy
{
    char policy_name[32];
    int policy_value;
};

struct Measure_Policy
{
    struct LSM_Policy lsm_policy[10];
    struct Cycle_Policy cycle_policy[10];
};

static int __init kernel_read_write_init(void)
{
    int parsed, value, policy_i = 0;
    struct file *file;
    char *buffer;
    char *start, *end;
    char *line, *next_line;
    char policy[100];
    loff_t pos = 0;
    ssize_t ret = 0;
    struct Measure_Policy Policy;

    // 打开策略文件
    file = filp_open(POLICY_FILE_PATH, O_RDONLY, 0);
    if (IS_ERR(file))
    {
        printk("无法打开策略文件\n");
        return -ENOENT;
    }

    // 分配内存缓冲区
    buffer = kmalloc(PAGE_SIZE, GFP_KERNEL);
    if (!buffer)
    {
        printk("无法分配内存缓冲区\n");
        ret = -ENOMEM;
        goto out;
    }

    // 读取策略文件的内容
    ret = kernel_read(file, buffer, PAGE_SIZE, &pos);
    if (ret < 0)
    {
        printk("无法读取策略文件\n");
        goto out;
    }

    // 定位开始标记位置
    start = strstr(buffer, START_MARKER);
    if (!start)
    {
        printk("找不到开始标记\n");
        goto out;
    }
    start += strlen(START_MARKER);

    // 定位结束标记位置
    end = strstr(start, END_MARKER);
    if (!end)
    {
        printk("找不到结束标记\n");
        goto out;
    }

    // 逐行读取策略文件的内容
    line = start;
    while (line < end)
    {
        next_line = strchr(line, '\n');
        if (!next_line)
            break;

        *next_line = '\0'; // 将行末尾字符设置为结束符

        // 跳过空行
        if (line[0] == '\0')
        {
            line = next_line + 1; // 指向下一行
            continue;
        }

        // 解析策略文件的内容
        parsed = sscanf(line, "%s %d", policy, &value);
        if (parsed != 2)
        {
            printk("策略文件行格式错误\n");
            *next_line = '\n';    // 恢复行末尾字符
            line = next_line + 1; // 指向下一行
            continue;
        }

        // 打印读取到的内容
        strcpy(Policy.cycle_policy[policy_i].policy_name, policy);
        Policy.cycle_policy[policy_i].policy_value = value;
        pr_info("策略: %s, 策略值: %d\n", Policy.cycle_policy[policy_i].policy_name, Policy.cycle_policy[policy_i].policy_value);
        policy_i++;

        *next_line = '\n'; // 恢复行末尾字符

        line = next_line + 1; // 指向下一行
    }
    printk("加载%d条策略完成\n", policy_i);


out:
    // 释放内存缓冲区
    kfree(buffer);

    // 关闭策略文件
    filp_close(file, NULL);

    return 0;
}

static void kernel_read_write_exit(void)
{
}

module_init(kernel_read_write_init);
module_exit(kernel_read_write_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("yzc");
MODULE_DESCRIPTION("kernel read write op");
