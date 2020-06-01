#include "sys.h"
#include "driverlib.h"
#include "stdio.h"

/* printf重定向 */
#ifdef __GNUC__
    #define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
    #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */

    PUTCHAR_PROTOTYPE
    {
        USCI_A_UART_transmitData(USCI_A1_BASE, (uint8_t)ch);
        return ch;
    }
    int fputs(const char *_ptr, register FILE *_fp)
    {
      unsigned int i, len;
      len = strlen(_ptr);
      for(i=0 ; i<len ; i++)
          USCI_A_UART_transmitData(USCI_A1_BASE, (uint8_t) _ptr[i]);
      return len;
    }

uint8_t key_scan(void)
{

    int i = 0;
    uint8_t key_value[4] = {0}, real_key_val = 0;;
    for(i = 0; i < 4; i ++)
    {
        //依次扫描第i列按键，一共四列
        P4OUT = (~(0x01 << i)) | 0xF0;
        __delay_cycles(100);

        //读出P4.4~P4.7管脚电平，并右移4位
        key_value[i] = ((~P4IN) >> 4) & 0x0F;

        //转换为0x01/0x02/0x04/0x08格式，按键消抖时可将该值左移4位后代替管脚编号
        key_value[i] = key_value[i]  > 7 ? (~key_value[i])& 0x0F : key_value[i];
        key_value[i] = key_value[i] == 7 ? 0x08 : key_value[i];

        if(key_value[i] != 0x00) //有按键被按下
        {
            // 按键消抖，即等待被按下的按键释放
            while(!GPIO_getInputPinValue(GPIO_PORT_P4, key_value[i] << 4));
            switch(key_value[i])
            {
                case 0x01: real_key_val =      i + 1; break; // 第1行
                case 0x02: real_key_val = 4  + i + 1; break; // 第2行
                case 0x04: real_key_val = 8  + i + 1; break; // 第3行
                case 0x08: real_key_val = 12 + i + 1; break; // 第4行
            }

//            OLED_ShowNum(10, 10, real_key_val, 2, 16);
            GPIO_toggleOutputOnPin(GPIO_PORT_P2, GPIO_PIN7);

        }

    }
    return real_key_val;

}

void breathe_led(int enabled)
{
    static int pulse = 0, cnt = 0;
    static int last_period = 0;
    int period = 10;
    //模仿定时器的PWM波，周期为period，pulse在0~period间变化，对应占空比为0~100%
    //在定时器中断中执行，该函数每执行一次，cnt则计数一次，计满一个周期后清零
    if(enabled == 1)
    {
        if(cnt++ <= pulse)
        {
            GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN7);
        }
        else
        {
            GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN7);
        }
        if(cnt >= period)
        {
            cnt = 0;
            static int count_mode = 0; //pulse计数模式 0:增计数  1:减计数
            pulse = count_mode ? pulse - 1 : pulse + 1;
            if(pulse == period)
            {
                count_mode = 1; //减计数
            }
            else if(pulse == 0)
            {
                count_mode = 0; //增计数
            }
        }
    }

}
