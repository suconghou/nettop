#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/time.h>

// 定义一个网卡结构体，同时用作链表节点
typedef struct netItem
{
    char *name;
    long recv_bytes;
    long trans_bytes;
    struct netItem *next;
} netItem;

netItem curr_items;
netItem last_items;
netItem init_items;
int maxLen;

char *trim(char *str)
{
    char *end;
    while (isspace((unsigned char)*str))
        str++;
    if (*str == 0)
        return str;
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end))
        end--;
    end[1] = '\0';
    return str;
}

int setItem(netItem *items, char *name, long recv_bytes, long trans_bytes)
{
    // 创建一个node节点
    netItem *node = (netItem *)malloc(sizeof(netItem));
    node->name = strdup(name);
    node->recv_bytes = recv_bytes;
    node->trans_bytes = trans_bytes;

    if (items == NULL || items->name == NULL)
    {
        *items = *node;
        return 1;
    }
    // 用于遍历node的临时游标
    netItem *temp = items;
    // 记录上一个node
    netItem *prev = temp;
    while (temp != NULL)
    {
        // 遍历链表
        // 如果两个key相同，则直接用新node的value覆盖旧的
        if (strcmp(temp->name, node->name) == 0)
        {
            // 找到则更新
            temp->recv_bytes = node->recv_bytes;
            temp->trans_bytes = node->trans_bytes;
            return 2;
        }
        prev = temp;
        temp = temp->next;
    }
    // 如果链表中未遍历到
    // 最后一个节点node_end的next指向新建的node
    prev->next = node;
    return 1;
}

netItem *getItem(netItem *items, char *name)
{
    // 用于遍历node的临时游标
    netItem *temp = items;
    while (temp != NULL && temp->name != NULL)
    {
        // 遍历链表
        if (strcmp(temp->name, name) == 0)
        {
            return temp;
        }
        temp = temp->next;
    }
    return NULL;
}

int readInfo()
{
    FILE *fp = fopen("/proc/net/dev", "r");
    if (fp == NULL)
    {
        return -1;
    }
    char s[1024];
    int linecount = 0;
    while (fgets(s, sizeof s, fp) != NULL)
    {
        linecount++;
        if (linecount <= 2)
        {
            continue;
        }
        // 首次使用:分割，后续使用空格分隔，当总分隔次数大于8，则有效;理论总分隔次数应该是 17； 8项接收，8项发送 和 网卡名
        char *tokenPtr = strtok(s, ":");
        short part = 0;
        char name[100];
        long recv_bytes = 0;
        long trans_bytes = 0;
        while (tokenPtr != NULL)
        {
            // printf("[%s]\n", tokenPtr);
            if (part == 0)
            {
                strcpy(name, trim(tokenPtr));
            }
            else if (part == 1)
            {
                recv_bytes = atol(tokenPtr);
            }
            else if (part == 9)
            {
                trans_bytes = atol(tokenPtr);
            }
            tokenPtr = strtok(NULL, " ");
            part++;
        }
        if (part >= 9)
        {
            // 数据有效,添加或更新链表
            if (maxLen < strlen(name))
            {
                // 计算网卡名最大长度
                maxLen = strlen(name);
            }
            setItem(&curr_items, name, recv_bytes, trans_bytes);
        }
    }
    fclose(fp);
    return 0;
}

int main()
{

    struct timeval tv;
    gettimeofday(&tv, NULL);
    long init_clock = tv.tv_sec * 1000 + tv.tv_usec / 1000;
    long last_clock = init_clock;
    while (1)
    {
        gettimeofday(&tv, NULL);
        long clock_now = tv.tv_sec * 1000 + tv.tv_usec / 1000;
        float t = ((float)(clock_now - last_clock)) / 1000;
        float tt = ((float)(clock_now - init_clock)) / 1000;
        if (readInfo() < 0)
        {
            perror("");
            exit(-1);
        }
        netItem *temp = &curr_items;
        printf("\ec");
        while (temp != NULL)
        {
            // 遍历链表
            netItem *lastItem = getItem(&last_items, temp->name);
            netItem *initItem = getItem(&init_items, temp->name);
            float total_recv,
                total_trans,
                recv,
                trans,
                recv_speed,
                trans_speed,
                recv_avg_speed,
                trans_avg_speed;
            if (!(lastItem == NULL || initItem == NULL))
            {
                total_recv = (temp->recv_bytes - initItem->recv_bytes) / 1024;
                total_trans = (temp->trans_bytes - initItem->trans_bytes) / 1024;

                recv = (temp->recv_bytes - lastItem->recv_bytes) / 1024;
                trans = (temp->trans_bytes - lastItem->trans_bytes) / 1024;

                recv_speed = recv / t;
                trans_speed = trans / t;

                recv_avg_speed = total_recv / tt;
                trans_avg_speed = total_trans / tt;
            }
            char total_recv_mb[1024];
            char recv_avg_speed_kb[1024];
            char recv_speed_kb[1024];
            char total_trans_mb[1024];
            char trans_avg_speed_kb[1024];
            char trans_speed_kb[1024];

            sprintf(total_recv_mb, "%.2fMB", total_recv / 1024);
            sprintf(recv_avg_speed_kb, "%.1fKB/S", recv_avg_speed);
            sprintf(recv_speed_kb, "%.1fKB/S", recv_speed);
            sprintf(total_trans_mb, "%.2fMB", total_trans / 1024);
            sprintf(trans_avg_speed_kb, "%.1fKB/S", trans_avg_speed);
            sprintf(trans_speed_kb, "%.1fKB/S", trans_speed);

            char str_recv[1024];
            char str_trans[1024];
            sprintf(str_recv, "%-11s %-11s %-11s", total_recv_mb, recv_avg_speed_kb, recv_speed_kb);
            sprintf(str_trans, "%-11s %-11s %-11s", total_trans_mb, trans_avg_speed_kb, trans_speed_kb);
            char name_pad[1024];
            sprintf(name_pad, "%*s%s", maxLen - (int)strlen(temp->name), "", temp->name);
            printf("\e[1;34m%s\e[00m  接收: \e[1;32m%s\e[00m 发送: \e[1;31m%s\e[00m\n", name_pad, str_recv, str_trans);
            setItem(&last_items, temp->name, temp->recv_bytes, temp->trans_bytes);
            temp = temp->next;
        }
        last_clock = clock_now;
        if (init_clock == last_clock)
        {
            // 填充首次获取的值
            netItem *it = &curr_items;
            while (it != NULL)
            {
                setItem(&init_items, it->name, it->recv_bytes, it->trans_bytes);
                it = it->next;
            }
            usleep(200000);
        }
        else
        {
            sleep(1);
        }
    }
    return 0;
}