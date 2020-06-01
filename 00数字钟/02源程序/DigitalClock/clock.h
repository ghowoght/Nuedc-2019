/*
 * clock.h
 *
 *  Created on: 2020年1月8日
 *      Author: 白鸟无言
 */

#ifndef CLOCK_H_
#define CLOCK_H_

#include <stdint.h>

enum KEY
{
    DEFAULT,
    NUM_0,
    NUM_1,
    NUM_2,
    NUM_3,
    NUM_4,
    NUM_5,
    NUM_6,
    NUM_7,
    NUM_8,
    NUM_9,
    CHANGE, //修改模式
    ENSURE, //确认修改
    MOV_LEF,//左移
    MOV_RIG,//右移
    BUTTON_1,//未定义按钮1
    BUTTON_2,//未定义按钮2
};

enum SWITCH
{
    OFF,
    ON,
};

enum STATE
{
    S0,
    S1,
    S2,
    S3,
    S4,
};

enum CLOCK_MODE
{
    COUNTING,
    REAL_TIME_MODIFING,
    ALARM_CLOCK_1_MODIFING,
    ALARM_CLOCK_2_MODIFING,
    ALARM_CLOCK_3_MODIFING,
};

enum WEEK
{
    SUNDAY,
    MONDAY,
    TUESDAY,
    WEDNESDAY,
    THURSDAY,
    FRIDAY,
    SATURDAY,
};

enum BIT_SELECT
{
    SEC_1,
    SEC_2,
    MIN_1,
    MIN_2,
    HOU_1,
    HOU_2,
    DAY_1,
    DAY_2,
    MON_1,
    MON_2,
    YEA_1,
    YEA_2,
    YEA_3,
    YEA_4,
};

enum HOUR_MODE
{
    _24, // 默认24小时制
    _12, // 12小时制
};
enum _AM_PM
{
    AM,
    PM,
};

enum TIME
{
    REAL_TIME,
    ALARM_CLOCK_1,
    ALARM_CLOCK_2,
    ALARM_CLOCK_3,
};

typedef struct
{
    uint16_t year;
    uint8_t  month;
    uint8_t  day;
    uint8_t  hour;
    uint8_t  minute;
    uint8_t  second;
    uint8_t  week;
    uint8_t  bit[14];
}time_st;
extern time_st time[4];
extern time_st time_to_set[2];

typedef struct
{
    uint8_t  year[2];
    uint8_t  month;
    uint8_t  day;
    uint8_t  hour;
    uint8_t  minute;
    uint8_t  second;
    uint8_t  week;
}save_st;
extern save_st save[4];

typedef struct
{
    int clock_mode;
    int key_val;
    int bit_select;
    int _24_or_12;
    int stopwatch_on;
    int stopwatch_state;
    int alarm_flag;
}flag_st;
extern flag_st flag;

extern uint8_t year_days[2][12];
extern int stopwatch_cnt;

void clock_init(void);
void clock_task(void);
void switch_task(void);
int week_calc(int y, int m, int d);
int year_leap_or_common(int year);
void oled_task(void);
void oled_show_single_num(uint8_t x,uint8_t y,uint32_t num, uint8_t len);
void refresh_bit_to_time(void);
void refresh_time_to_bit(void);
int check_bit_is_in_range(void);
void alarm_task(void);

#endif /* CLOCK_H_ */
