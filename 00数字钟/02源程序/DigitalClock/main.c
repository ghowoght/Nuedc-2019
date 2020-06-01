#include "driverlib.h"
#include "stdio.h"
#include "sys.h"
#include "bsp.h"
#include "oled.h"
#include "clock.h"
#include "flash.h"

/**
 * main.c
 */
int main(void)
{
    /* 关闭看门狗 */
    WDT_A_hold(WDT_A_BASE);
	
    //将主频设置为4MHz
    set_freq_4MHz();

    //GPIO初始化
    GPIO_Init();

    //串口初始化
    Uart_Init();

    //定时器A0初始化
    TimerA0_Init();

    //定时器B0初始化
    TimerB0_Init();

    //显示屏初始化
    OLED_Init();

    //数字钟初始化
    clock_init();

    __enable_interrupt();

    __bis_SR_register(LPM0_bits + GIE);
    __no_operation();
}

uint8_t key_val = 0;
#pragma vector=TIMER0_A0_VECTOR
__interrupt void TIMER0_A0_ISR (void)
{
    /* 执行频率:400Hz */

    //扫描按键
    flag.key_val = key_scan();

    //数字钟任务
    clock_task();

}

#pragma vector=TIMER0_B0_VECTOR
__interrupt void TIMER0_B0_ISR (void)
{
    /* 执行频率:10Hz */

    GPIO_toggleOutputOnPin(GPIO_PORT_P8, GPIO_PIN0);

    //刷新显示屏
    oled_task();
}

uint8_t receivedData = 0x00;
#pragma vector=USCI_A1_VECTOR
__interrupt void USCI_A1_ISR (void)
{
    switch (__even_in_range(UCA1IV,4))
    {
        case 2: //接收中断标志
            receivedData = USCI_A_UART_receiveData(USCI_A1_BASE);

            // 将接收到的字符回传给上位机
            USCI_A_UART_transmitData(USCI_A1_BASE,receivedData);

            break;
        default: break;
    }
}

#pragma vector=PORT2_VECTOR
__interrupt void PORT2_ISR (void)
{
    GPIO_clearInterrupt(GPIO_PORT_P2, GPIO_PIN6);
}

