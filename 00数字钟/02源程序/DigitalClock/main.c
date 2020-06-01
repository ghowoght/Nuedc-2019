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
    /* �رտ��Ź� */
    WDT_A_hold(WDT_A_BASE);
	
    //����Ƶ����Ϊ4MHz
    set_freq_4MHz();

    //GPIO��ʼ��
    GPIO_Init();

    //���ڳ�ʼ��
    Uart_Init();

    //��ʱ��A0��ʼ��
    TimerA0_Init();

    //��ʱ��B0��ʼ��
    TimerB0_Init();

    //��ʾ����ʼ��
    OLED_Init();

    //�����ӳ�ʼ��
    clock_init();

    __enable_interrupt();

    __bis_SR_register(LPM0_bits + GIE);
    __no_operation();
}

uint8_t key_val = 0;
#pragma vector=TIMER0_A0_VECTOR
__interrupt void TIMER0_A0_ISR (void)
{
    /* ִ��Ƶ��:400Hz */

    //ɨ�谴��
    flag.key_val = key_scan();

    //����������
    clock_task();

}

#pragma vector=TIMER0_B0_VECTOR
__interrupt void TIMER0_B0_ISR (void)
{
    /* ִ��Ƶ��:10Hz */

    GPIO_toggleOutputOnPin(GPIO_PORT_P8, GPIO_PIN0);

    //ˢ����ʾ��
    oled_task();
}

uint8_t receivedData = 0x00;
#pragma vector=USCI_A1_VECTOR
__interrupt void USCI_A1_ISR (void)
{
    switch (__even_in_range(UCA1IV,4))
    {
        case 2: //�����жϱ�־
            receivedData = USCI_A_UART_receiveData(USCI_A1_BASE);

            // �����յ����ַ��ش�����λ��
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

