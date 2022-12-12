#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <ctime>
#include <thread>
#include <chrono>

using namespace std;
class netItem
{
public:
    string name;
    long recv_bytes;
    long trans_bytes;
};

map<string, netItem> curr_items;
map<string, netItem> last_items;
map<string, netItem> init_items;
int maxLen;
// trim from start (in place)
static inline void ltrim(string &s)
{
    s.erase(s.begin(), find_if(s.begin(), s.end(), [](unsigned char ch)
                               { return !isspace(ch); }));
}

// trim from end (in place)
static inline void rtrim(string &s)
{
    s.erase(find_if(s.rbegin(), s.rend(), [](unsigned char ch)
                    { return !isspace(ch); })
                .base(),
            s.end());
}

// trim from both ends (in place)
static inline void trim(string &s)
{
    rtrim(s);
    ltrim(s);
}

// 按照字符切割字符串，多个连续字符跳过
static inline int explode(string str, const char split, vector<string> &res)
{
    istringstream iss(str);
    string token;
    int count = 0;
    while (getline(iss, token, split))
    {
        if (token.empty())
        {
            continue;
        }
        count++;
        res.push_back(token);
    }
    return count;
}

int readInfo(const string &filename = "/tmp/1")
{
    // ifstream是输入文件流（input file stream）的简称, std::ifstream
    // 离开作用域后，fh文件将被析构器自动关闭
    ifstream fh(filename); // 打开一个文件
    if (!fh)
    {
        // open file failed
        return -1;
    }
    string str;
    int linecount = 0;
    while (getline(fh, str))
    {
        linecount++;
        if (linecount <= 2)
        {
            continue;
        }
        vector<string> parts;
        if (explode(str, ':', parts) == 2)
        {
            string name = parts.at(0);
            vector<string> arr;
            if (explode(parts.at(1), ' ', arr) != 16)
            {
                continue;
            }
            trim(name);
            netItem item;
            item.name = name;
            item.recv_bytes = stol(arr.at(0));
            item.trans_bytes = stol(arr.at(8));
            curr_items[name] = item;
            if (maxLen < name.length())
            {
                maxLen = name.length();
            }
        }
    }
    return 0;
}

// 时间戳，毫秒
static inline long time_unix_mill()
{
    return chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();
}

int main()
{

    unsigned long init_clock = time_unix_mill();
    unsigned long last_clock = init_clock;
    while (true)
    {
        unsigned long clock_now = time_unix_mill();
        float t = ((float)(clock_now - last_clock)) / 1000;
        float tt = ((float)(clock_now - init_clock)) / 1000;
        if (readInfo() < 0)
        {
            perror("");
            exit(-1);
        }
        printf("\ec");
        for (const auto &item : curr_items)
        {
            auto lastItem = last_items.find(item.first);
            auto initItem = init_items.find(item.first);
            float total_recv = 0,
                  total_trans = 0,
                  recv = 0,
                  trans = 0,
                  recv_speed = 0,
                  trans_speed = 0,
                  recv_avg_speed = 0,
                  trans_avg_speed = 0;
            if (!(lastItem == last_items.end() || initItem == init_items.end()))
            {
                total_recv = (item.second.recv_bytes - initItem->second.recv_bytes) / 1024;
                total_trans = (item.second.trans_bytes - initItem->second.trans_bytes) / 1024;

                recv = (item.second.recv_bytes - lastItem->second.recv_bytes) / 1024;
                trans = (item.second.trans_bytes - lastItem->second.trans_bytes) / 1024;

                recv_speed = recv / t;
                trans_speed = trans / t;

                recv_avg_speed = total_recv / tt;
                trans_avg_speed = total_trans / tt;
            }
            char buff[1024];
            total_recv > 1048576 ? snprintf(buff, sizeof(buff), "%.2fGB", total_recv / 1024 / 1024) : snprintf(buff, sizeof(buff), "%.2fMB", total_recv / 1024);
            string total_recv_s = buff;
            recv_avg_speed > 1024 ? snprintf(buff, sizeof(buff), "%.1fMB/S", recv_avg_speed / 1024) : snprintf(buff, sizeof(buff), "%.1fKB/S", recv_avg_speed);
            string recv_avg_speed_s = buff;
            recv_speed > 1024 ? snprintf(buff, sizeof(buff), "%.1fMB/S", recv_speed / 1024) : snprintf(buff, sizeof(buff), "%.1fKB/S", recv_speed);
            string recv_speed_s = buff;
            total_trans > 1048576 ? snprintf(buff, sizeof(buff), "%.2fGB", total_trans / 1024 / 1024) : snprintf(buff, sizeof(buff), "%.2fMB", total_trans / 1024);
            string total_trans_s = buff;
            trans_avg_speed > 1024 ? snprintf(buff, sizeof(buff), "%.1fMB/S", trans_avg_speed / 1024) : snprintf(buff, sizeof(buff), "%.1fKB/S", trans_avg_speed);
            string trans_avg_speed_s = buff;
            trans_speed > 1024 ? snprintf(buff, sizeof(buff), "%.1fMB/S", trans_speed / 1024) : snprintf(buff, sizeof(buff), "%.1fKB/S", trans_speed);
            string trans_speed_s = buff;
            snprintf(buff, sizeof(buff), "%-11s %-11s %-11s", total_recv_s.c_str(), recv_avg_speed_s.c_str(), recv_speed_s.c_str());
            string str_recv = buff;
            snprintf(buff, sizeof(buff), "%-11s %-11s %-11s", total_trans_s.c_str(), trans_avg_speed_s.c_str(), trans_speed_s.c_str());
            string str_trans = buff;
            snprintf(buff, sizeof(buff), "%s%*s", item.first.c_str(), maxLen - (int)item.first.length(), "");
            string name_pad = buff;
            snprintf(buff, sizeof(buff), "\e[1;34m%s\e[00m  接收: \e[1;32m%s\e[00m 发送: \e[1;31m%s\e[00m", name_pad.c_str(), str_recv.c_str(), str_trans.c_str());
            last_items[item.first] = item.second;
            cout << buff << endl;
        }
        last_clock = clock_now;
        if (init_clock == last_clock)
        {
            for (const auto t : curr_items)
            {
                init_items[t.first] = t.second;
            }
            this_thread::sleep_for(chrono::milliseconds(200));
        }
        else
        {
            this_thread::sleep_for(chrono::seconds(1));
        }
    }
}