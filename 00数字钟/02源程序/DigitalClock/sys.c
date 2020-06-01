#include "sys.h"
#include "driverlib.h"
#include "stdio.h"

/* printf�ض��� */
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
        //����ɨ���i�а�����һ������
        P4OUT = (~(0x01 << i)) | 0xF0;
        __delay_cycles(100);

        //����P4.4~P4.7�ܽŵ�ƽ��������4λ
        key_value[i] = ((~P4IN) >> 4) & 0x0F;

        //ת��Ϊ0x01/0x02/0x04/0x08��ʽ����������ʱ�ɽ���ֵ����4λ�����ܽű��
        key_value[i] = key_value[i]  > 7 ? (~key_value[i])& 0x0F : key_value[i];
        key_value[i] = key_value[i] == 7 ? 0x08 : key_value[i];

        if(key_value[i] != 0x00) //�а���������
        {
            // �������������ȴ������µİ����ͷ�
            while(!GPIO_getInputPinValue(GPIO_PORT_P4, key_value[i] << 4));
            switch(key_value[i])
            {
                case 0x01: real_key_val =      i + 1; break; // ��1��
                case 0x02: real_key_val = 4  + i + 1; break; // ��2��
                case 0x04: real_key_val = 8  + i + 1; break; // ��3��
                case 0x08: real_key_val = 12 + i + 1; break; // ��4��
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
    //ģ�¶�ʱ����PWM��������Ϊperiod��pulse��0~period��仯����Ӧռ�ձ�Ϊ0~100%
    //�ڶ�ʱ���ж���ִ�У��ú���ÿִ��һ�Σ�cnt�����һ�Σ�����һ�����ں�����
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
            static int count_mode = 0; //pulse����ģʽ 0:������  1:������
            pulse = count_mode ? pulse - 1 : pulse + 1;
            if(pulse == period)
            {
                count_mode = 1; //������
            }
            else if(pulse == 0)
            {
                count_mode = 0; //������
            }
        }
    }

}
