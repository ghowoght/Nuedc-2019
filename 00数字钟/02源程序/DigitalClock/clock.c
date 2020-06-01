#include "sys.h"
#include "driverlib.h"
#include "stdio.h"
#include "clock.h"
#include "flash.h"
#include "oled.h"

time_st time[4]; // 包括一个实时时钟和三个闹钟结构体
time_st time_to_set[2]; //调整值，数组第一个结构体为过渡值，第二个为已经验证的值，再按下确认即可更新到实时时钟或者闹钟结构体中
flag_st flag; //存放各类标志变量

int stopwatch_cnt; //秒表的计数值

void clock_init(void)
{
    flag.clock_mode   = COUNTING;  //计数模式
    flag._24_or_12    = _24;       //24小时制
    flag.stopwatch_on = OFF;       //关闭秒表

    time[REAL_TIME].year   = 2020;
    time[REAL_TIME].month  = 1;
    time[REAL_TIME].day    = 1;
    time[REAL_TIME].hour   = 0;
    time[REAL_TIME].minute = 0;
    time[REAL_TIME].second = 0;
    time[REAL_TIME].week   = 3;

    //开机读取上一次保存的时间
    param_read();
}

enum YEAR
{
    COMMON_YEAR, //平年
    LEAP_YEAR,   //闰年
};

uint8_t year_days[2][12] = {{31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},  //第一列为平年各月天数
                            {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}}; //第二列为闰年各月天数

void clock_task(void)
{
    // 模式切换任务
    switch_task();

    // 闹钟任务
    alarm_task();

    // 以下条件语句用于正常的计时
    if(flag.clock_mode == COUNTING)
    {
        static int cnt = 0;
        cnt = ++cnt % 400;
        if(cnt == 0)
        {
            time[REAL_TIME].second = ++time[REAL_TIME].second % 60;
            if(time[REAL_TIME].second == 0)
            {
                time[REAL_TIME].minute = ++time[REAL_TIME].minute % 60;
                if(time[REAL_TIME].minute == 0)
                {

                    time[REAL_TIME].hour = ++time[REAL_TIME].hour % 24;
                    if(time[REAL_TIME].hour == 0)
                    {
                        // 通过查表方式判断该月份的天数
                        time[REAL_TIME].day = time[REAL_TIME].day % year_days[year_leap_or_common(time[REAL_TIME].year)][time[REAL_TIME].month - 1] + 1;
                        if(time[REAL_TIME].day == 1)
                        {
                            time[REAL_TIME].month = time[REAL_TIME].month % 12 + 1;
                            if(time[REAL_TIME].month == 1)
                            {
                                time[REAL_TIME].year ++;
                            }
                        }
                    }
                }
                param_save(); //一分钟保存一次时钟
            }
            // 当秒表打开且处于计数状态时，秒表计数值加一
            // 频率: 1Hz
            if(flag.stopwatch_on == ON && flag.stopwatch_state == S2)
            {
                stopwatch_cnt ++;
            }
        }
    }

}

// 由年月日计算出星期
int week_calc(int y, int m, int d)
{
    if (m == 1 || m == 2)
    {
        m += 12;
        --y;
    }
    return (d + 2 * m + 3 * (m + 1) / 5 + y + y / 4 - y / 100 + y / 400 + 1) % 7;
}

// 判断平年/闰年
// 返回: 平年 0 / 闰年 1
int year_leap_or_common(int year)
{
    int a = 0;
    if(year % 400 == 0)
    {
        a = 1;
    }
    else
    {
        if(year % 4 == 0 && year % 100 != 0)
            a = 1;
        else
            a = 0;
    }

    return a;
}

// 显示屏刷新任务
//一个字节大小为16，宽度为8
void oled_task(void)
{
    //修改x_offset可以调整画面的x轴位置
    int x_offset = 0;
    char* week[7] = {"Sun.", "Mon.", "Tue.", "Wed.", "Thu.", "Fri.", "Sat."};

    if(flag.clock_mode == COUNTING) // 显示实时时钟
    {
        oled_show_single_num(x_offset +  0,11,time[REAL_TIME].year, 4);
        OLED_ShowChar(x_offset +  32, 11, '/');
        oled_show_single_num(x_offset + 48,11,time[REAL_TIME].month, 2);
        OLED_ShowChar(x_offset +  64, 11, '/');
        oled_show_single_num(x_offset + 80,11,time[REAL_TIME].day, 2);

        //根据小时时制显示hour
        if(flag._24_or_12 == _24)
        {
            OLED_ShowString(x_offset, 9, "  ");
            oled_show_single_num(x_offset + 16,13,time[REAL_TIME].hour, 2);
        }
        else
        {
            char* str[2] = {"AM", "PM"};
            OLED_ShowString(x_offset, 9, str[time[REAL_TIME].hour / 12]);
            oled_show_single_num(x_offset + 16,13,time[REAL_TIME].hour % 13 + time[REAL_TIME].hour / 13, 2);
        }
        OLED_ShowChar(x_offset +  32, 13, ':');
        oled_show_single_num(x_offset + 48,13,time[REAL_TIME].minute, 2);
        OLED_ShowChar(x_offset +  64, 13, ':');
        oled_show_single_num(x_offset + 80,13,time[REAL_TIME].second, 2);

        // 判断星期，并查表显示
        time[REAL_TIME].week = week_calc(time[REAL_TIME].year, time[REAL_TIME].month, time[REAL_TIME].day);
        OLED_ShowString(x_offset + 32, 9, week[time[REAL_TIME].week]);
    }
    else // 显示调整值
    {
        //这段用来显示正在修改的位置
        //被选中的位置应该会闪烁
        int bit = flag.bit_select;
        if(bit < 2)
            bit = bit;
        else if(bit < 4)
            bit = bit + 2;
        else if(bit < 6)
            bit = bit + 4;
        else if(bit < 8)
            bit = bit + 6;
        else if(bit < 10)
            bit = bit + 8;
        else
            bit = bit + 10;
        if(bit > 11)
            OLED_ShowChar(x_offset +  88 - (bit % 12) * 8, 11, ' ');
        else
            OLED_ShowChar(x_offset +  88 - bit * 8, 13, ' ');

        // 显示调整值
        oled_show_single_num(x_offset +  0, 11, time_to_set[1].year, 4);
        OLED_ShowChar(x_offset +  32, 11, '/');
        oled_show_single_num(x_offset + 48, 11, time_to_set[1].month, 2);
        OLED_ShowChar(x_offset +  64, 11, '/');
        oled_show_single_num(x_offset + 80, 11, time_to_set[1].day, 2);

        OLED_ShowString(x_offset, 9, "  ");
        oled_show_single_num(x_offset + 16, 13, time_to_set[1].hour, 2);
        OLED_ShowChar(x_offset +  32, 13, ':');
        oled_show_single_num(x_offset + 48, 13, time_to_set[1].minute, 2);
        OLED_ShowChar(x_offset +  64, 13, ':');
        oled_show_single_num(x_offset + 80, 13, time_to_set[1].second, 2);

        time_to_set[1].week = week_calc(time_to_set[1].year, time_to_set[1].month, time_to_set[1].day);
        OLED_ShowString(x_offset + 32, 9, week[time_to_set[1].week]);
    }

    //在右上角显示工作模式
    //分别为: 实时时钟、实时时钟调整、闹钟1调整、闹钟2调整、闹钟3调整
    char* str[5] = {"   ", " RT", " A1", " A2", " A3"};
    OLED_ShowString(x_offset + 72, 9, str[flag.clock_mode]);

    //秒表已打开并且处于实时时钟模式，则在右上角显示秒表计数值
    if(flag.stopwatch_on == ON && flag.clock_mode == COUNTING)
    {
        OLED_ShowNum(x_offset + 72, 9, stopwatch_cnt, 3, 16);
    }

}

#include <math.h>
void oled_show_single_num(uint8_t x,uint8_t y,uint32_t num, uint8_t len)
{
    int len0 = len;
    while(len)
    {
        int temp = num / (int)pow(10, --len);
        OLED_ShowNum(x + 8 * (len0 - len - 1), y, temp,   1, 16);
        num -= temp * (int)pow(10, len);
    }
}

// 模式切换任务
void switch_task(void)
{
    if(flag.key_val == CHANGE)
    {
        flag.clock_mode = ++flag.clock_mode % 5;
        refresh_time_to_bit();
        int i = 0;
        for(i = 0; i < 14; i ++)
        {
            // 将时钟或者闹钟的值复制到设定值中
            time_to_set[1].bit[i] = time[flag.clock_mode - 1].bit[i];
        }
        time_to_set[1].year   = time_to_set[1].bit[YEA_4] * 1000 + time_to_set[1].bit[YEA_3] * 100
                              + time_to_set[1].bit[YEA_2] * 10   + time_to_set[1].bit[YEA_1];
        time_to_set[1].month  = time_to_set[1].bit[MON_2] * 10   + time_to_set[1].bit[MON_1];
        time_to_set[1].day    = time_to_set[1].bit[DAY_2] * 10   + time_to_set[1].bit[DAY_1];
        time_to_set[1].hour   = time_to_set[1].bit[HOU_2] * 10   + time_to_set[1].bit[HOU_1];
        time_to_set[1].minute = time_to_set[1].bit[MIN_2] * 10   + time_to_set[1].bit[MIN_1];
        time_to_set[1].second = time_to_set[1].bit[SEC_2] * 10   + time_to_set[1].bit[SEC_1];
    }

    if(flag.clock_mode != COUNTING) //处于调整模式时
    {
        if(flag.key_val == MOV_LEF) //待调整位左移
        {
            flag.bit_select = ++flag.bit_select % 14;
        }
        else if(flag.key_val == MOV_RIG) //待调整位右移
        {
            flag.bit_select = flag.bit_select ? flag.bit_select : 14;
            flag.bit_select = (--flag.bit_select % 14);
        }
        else if(flag.key_val == ENSURE) //按下确认键
        {
            refresh_bit_to_time();
            param_save();
            flag.clock_mode = COUNTING;
        }
        else if(flag.key_val >= NUM_0 && flag.key_val <= NUM_9)
        {
            check_bit_is_in_range();
        }
    }
    // 在正常时钟模式时，可以通过按ENSURE键切换24/12小时时制
    else if(flag.clock_mode == COUNTING)
    {

        if(flag.key_val == ENSURE)
        {   // 切换24/12小时时制
            // 只需要更改标志变量
            // 在显示时进行判断即可
            flag._24_or_12 = ++flag._24_or_12 % 2;
        }
        else if(flag.key_val == BUTTON_2)
        {
           flag.alarm_flag = flag.alarm_flag == 0 ? 1 : 0;
        }
    }

    /* 秒表 */
    static int state = S0;
    if(flag.key_val == BUTTON_1)
    {
        flag.stopwatch_on = ON; //开启计数器
        flag.stopwatch_state = flag.stopwatch_state % 2 + 1; // 开始计数 / 暂停计数
        state = S0;
    }
    else if(flag.key_val == BUTTON_2)
    {
        if(state == S0)
        {
            stopwatch_cnt = 0; //清零计算
            flag.stopwatch_state = S1; //暂停计数
            state ++;
        }
        else if(state == S1)
        {
            flag.stopwatch_on = OFF; //关闭计数器
            flag.stopwatch_state = S0; // 状态清零
            state = S0; // 状态清零
        }
    }

}

// 判断调整后各位是不是在正常范围内
int check_bit_is_in_range(void)
{
    int i = 0;
    for(i = 0; i < 14; i ++)
    {
        // 将时钟或者闹钟的值复制到设定值中
        time_to_set[0].bit[i] = time[flag.clock_mode - 1].bit[i];
    }
    time_to_set[0].bit[flag.bit_select] = flag.key_val - 1;

    time_to_set[0].year   = time_to_set[0].bit[YEA_4] * 1000 + time_to_set[0].bit[YEA_3] * 100
                          + time_to_set[0].bit[YEA_2] * 10   + time_to_set[0].bit[YEA_1];
    time_to_set[0].month  = time_to_set[0].bit[MON_2] * 10   + time_to_set[0].bit[MON_1];
    time_to_set[0].day    = time_to_set[0].bit[DAY_2] * 10   + time_to_set[0].bit[DAY_1];
    time_to_set[0].hour   = time_to_set[0].bit[HOU_2] * 10   + time_to_set[0].bit[HOU_1];
    time_to_set[0].minute = time_to_set[0].bit[MIN_2] * 10   + time_to_set[0].bit[MIN_1];
    time_to_set[0].second = time_to_set[0].bit[SEC_2] * 10   + time_to_set[0].bit[SEC_1];

    if(time_to_set[0].month <= 12
        && time_to_set[0].day <= year_days[year_leap_or_common(time_to_set[0].year)][time_to_set[0].month - 1]
        && time_to_set[0].hour < 24
        && time_to_set[0].minute < 60
        && time_to_set[0].second < 60)
    {
        for(i = 0; i < 14; i ++)
        {
            // 将设定值复制到时钟或者闹钟的结构体中
            time[flag.clock_mode - 1].bit[i] = time_to_set[0].bit[i];
        }
        time_to_set[1].year   = time_to_set[0].year;
        time_to_set[1].month  = time_to_set[0].month  ;
        time_to_set[1].day    = time_to_set[0].day    ;
        time_to_set[1].hour   = time_to_set[0].hour   ;
        time_to_set[1].minute = time_to_set[0].minute ;
        time_to_set[1].second = time_to_set[0].second ;
        return 1;
    }
    return 0;
}

// 将bit更新到time_st结构体中
void refresh_bit_to_time(void)
{
    time[flag.clock_mode - 1].year   = time[flag.clock_mode - 1].bit[YEA_4] * 1000 + time[flag.clock_mode - 1].bit[YEA_3] * 100
                                     + time[flag.clock_mode - 1].bit[YEA_2] * 10   + time[flag.clock_mode - 1].bit[YEA_1];
    time[flag.clock_mode - 1].month  = time[flag.clock_mode - 1].bit[MON_2] * 10   + time[flag.clock_mode - 1].bit[MON_1];
    time[flag.clock_mode - 1].day    = time[flag.clock_mode - 1].bit[DAY_2] * 10   + time[flag.clock_mode - 1].bit[DAY_1];
    time[flag.clock_mode - 1].hour   = time[flag.clock_mode - 1].bit[HOU_2] * 10   + time[flag.clock_mode - 1].bit[HOU_1];
    time[flag.clock_mode - 1].minute = time[flag.clock_mode - 1].bit[MIN_2] * 10   + time[flag.clock_mode - 1].bit[MIN_1];
    time[flag.clock_mode - 1].second = time[flag.clock_mode - 1].bit[SEC_2] * 10   + time[flag.clock_mode - 1].bit[SEC_1];
}

// 将time_st结构体各成员拆分成个位
void refresh_time_to_bit(void)
{
    int i = 0;
    for(i = 0; i < 4; i ++)
    {
        if(time[i].year > 9999 || time[i].year < 0)
            continue;
        time[i].bit[YEA_4] =  time[i].year   / 1000;
        time[i].bit[YEA_3] = (time[i].year   % 1000) / 100;
        time[i].bit[YEA_2] = (time[i].year   % 100)  / 10;
        time[i].bit[YEA_1] = (time[i].year   % 10)   / 1;

        time[i].bit[MON_2] =  time[i].month  / 10;
        time[i].bit[MON_1] = (time[i].month  % 10) / 1;

        time[i].bit[DAY_2] =  time[i].day    / 10;
        time[i].bit[DAY_1] = (time[i].day    % 10) / 1;

        time[i].bit[HOU_2] =  time[i].hour   / 10;
        time[i].bit[HOU_1] = (time[i].hour   % 10) / 1;

        time[i].bit[MIN_2] =  time[i].minute / 10;
        time[i].bit[MIN_1] = (time[i].minute % 10) / 1;

        time[i].bit[SEC_2] =  time[i].second / 10;
        time[i].bit[SEC_1] = (time[i].second % 10) / 1;
    }
}

void alarm_task(void)
{
    int state = S0;
    int i;
    for(i = ALARM_CLOCK_1; i < ALARM_CLOCK_3 + 1; i++)
    {
        if(time[i].hour == time[REAL_TIME].hour
                && time[i].minute == time[REAL_TIME].minute)
        {
            state = 1;
            break;
        }
        else
        {
            state = S0;
        }
    }
    //呼吸灯
    if(flag.alarm_flag == ON)
        breathe_led(state);
    else
        breathe_led(S0);
}
