#include "sys.h"
#include "driverlib.h"

#include "clock.h"
#include "flash.h"

save_st save[4];

//INFO address in flash from which data is copied
#define INFOC_START (0x1880)
//Number of bytes being copied
#define NUMBER_OF_BYTES 128
//Delay cycles
#define DELAY_TIME 1000000

//数据拆分宏定义，在发送大于1字节的数据类型时，比如int16、float等，需要把数据拆分成单独字节进行发送
#define BYTE0(dwTemp)       ( *( (char *)(&dwTemp)    ) )
#define BYTE1(dwTemp)       ( *( (char *)(&dwTemp) + 1) )
#define BYTE2(dwTemp)       ( *( (char *)(&dwTemp) + 2) )
#define BYTE3(dwTemp)       ( *( (char *)(&dwTemp) + 3) )

// 参数保存
void param_save(void)
{
    int i = 0;
    for(i = 0; i < 4; i ++)
    {
        save[i].year[0] = BYTE1(time[i].year);
        save[i].year[1] = BYTE0(time[i].year);
        save[i].month   = time[i].month;
        save[i].day     = time[i].day;
        save[i].hour    = time[i].hour;
        save[i].minute  = time[i].minute;
        save[i].second  = time[i].second;
        save[i].week    = time[i].week;

    }
    write_Seg((uint8_t*)save);
}

// 参数读取
void param_read(void)
{
    uint8_t* ptr = (uint8_t*)INFOC_START;
    int i = 0;
    for(i = 0; i < 4; i ++)
    {
        time[i].year    = (uint16_t)((ptr[i * 8 + 0] << 8) + ptr[i * 8 + 1]);
        time[i].month   = ptr[i * 8 + 2];
        time[i].day     = ptr[i * 8 + 3];
        time[i].hour    = ptr[i * 8 + 4];
        time[i].minute  = ptr[i * 8 + 5];
        time[i].second  = ptr[i * 8 + 6];
        time[i].week    = ptr[i * 8 + 7];

        if(time[i].month <= 12
            && time[i].day <= year_days[year_leap_or_common(time[i].year)][time[i].month - 1]
            && time[i].hour < 24
            && time[i].minute < 60
            && time[i].second < 60)
        {

        }
        else
        {
            time[i].year    = 2020;
            time[i].month   = 1;
            time[i].day     = 1;
            time[i].hour    = 0;
            time[i].minute  = 0;
            time[i].second  = 0;
            time[i].week    = 3;
        }
    }
}
//----------------------------------------------------------------------------
//Input = value, holds value to write to Seg
//----------------------------------------------------------------------------
void write_Seg(uint8_t* ptr)
{
    uint16_t status;
    //Erase INFOC
    do
    {
        FlashCtl_eraseSegment((uint8_t *)INFOC_START);
        status = FlashCtl_performEraseCheck((uint8_t *)INFOC_START, NUMBER_OF_BYTES );
    } while(status == STATUS_FAIL);
    //Flash Write
    FlashCtl_write8(ptr, (uint8_t *)INFOC_START, NUMBER_OF_BYTES );
}
