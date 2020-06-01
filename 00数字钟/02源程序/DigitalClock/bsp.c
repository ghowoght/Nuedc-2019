#include "driverlib.h"

void set_freq_4MHz(void)
{
    /* ������ƵΪ4MHz */
    PMM_setVCore(PMM_CORE_LEVEL_1);//��Ƶ��ߺ�VCore��ѹҲ��Ҫ��֮����
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P7,GPIO_PIN2 + GPIO_PIN3);//XT2�ڲ���Ϊ��ͨIO
    UCS_turnOnXT2WithTimeout(UCS_XT2_DRIVE_4MHZ_8MHZ,5000);//����XT2
    UCS_initClockSignal(UCS_FLLREF,UCS_XT2CLK_SELECT,UCS_CLOCK_DIVIDER_1);//XT2��ΪFLL�ο�
    UCS_initFLLSettle(24000,6);//MCLK����Ϊ24MHz
    UCS_initClockSignal(UCS_ACLK,UCS_REFOCLK_SELECT,UCS_CLOCK_DIVIDER_1);//ACLK����Ϊ32.768kHz
    UCS_initClockSignal(UCS_SMCLK,UCS_XT2CLK_SELECT,UCS_CLOCK_DIVIDER_1);//SMCLK����Ϊ4MHz
}

void GPIO_Init(void)
{
    /* ��ʼ��LEDΪ���ģʽ */
    GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN7);
    GPIO_setAsOutputPin(GPIO_PORT_P8, GPIO_PIN0);
    GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN7);
    GPIO_setOutputHighOnPin(GPIO_PORT_P8, GPIO_PIN0);

    /* ��ʼ��oled�ܽ� */
    GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN0 | GPIO_PIN1 | GPIO_PIN2 | GPIO_PIN3 | GPIO_PIN4);
    GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN0 | GPIO_PIN1 | GPIO_PIN2 | GPIO_PIN3 | GPIO_PIN4);

    /* ��ʼ��������̹ܽ� */
    GPIO_setAsOutputPin(GPIO_PORT_P4, GPIO_PIN0 | GPIO_PIN1 | GPIO_PIN2 | GPIO_PIN3);
    GPIO_setOutputHighOnPin(GPIO_PORT_P4, GPIO_PIN0 | GPIO_PIN1 | GPIO_PIN2 | GPIO_PIN3);

    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P4, GPIO_PIN4 | GPIO_PIN5 | GPIO_PIN6 | GPIO_PIN7);

    /* ��ʼ�����ذ����ܽ�Ϊ��������ģʽ */
    //��������û������ʱ���ܽŵ�ƽΪ�ߵ�ƽ
    //https://blog.csdn.net/quinn1994/article/details/82530590
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P2, GPIO_PIN6);
    //�½��ش���
    GPIO_selectInterruptEdge(GPIO_PORT_P2, GPIO_PIN6, GPIO_HIGH_TO_LOW_TRANSITION);
    GPIO_enableInterrupt(GPIO_PORT_P2, GPIO_PIN6);

}

void Uart_Init(void)
{
    /* �������� */
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P8, GPIO_PIN2 + GPIO_PIN3);
    //Baudrate = 115200, clock freq = 4MHz
    //UCBRx = ʱ��ԴƵ�� / ������ = 4Mhz / 115200Hz �� 34.7222 = 35
    //UCBRFx = 0, UCBRSx = 2, UCOS16 = 0
    //https://blog.csdn.net/liu__ke/article/details/27535547
    USCI_A_UART_initParam uart_a_param = {0};
    uart_a_param.selectClockSource = USCI_A_UART_CLOCKSOURCE_SMCLK;
    uart_a_param.clockPrescalar = 35;
    uart_a_param.firstModReg = 0;
    uart_a_param.secondModReg = 2;
    uart_a_param.parity = USCI_A_UART_NO_PARITY;
    uart_a_param.msborLsbFirst = USCI_A_UART_LSB_FIRST;
    uart_a_param.numberofStopBits = USCI_A_UART_ONE_STOP_BIT;
    uart_a_param.uartMode = USCI_A_UART_MODE;
    uart_a_param.overSampling = USCI_A_UART_LOW_FREQUENCY_BAUDRATE_GENERATION;

    if (STATUS_FAIL == USCI_A_UART_init(USCI_A1_BASE, &uart_a_param)){
        return;
    }

    //Enable UART module for operation
    USCI_A_UART_enable(USCI_A1_BASE);

    //Enable Receive Interrupt
    USCI_A_UART_clearInterrupt(USCI_A1_BASE,USCI_A_UART_RECEIVE_INTERRUPT);
    USCI_A_UART_enableInterrupt(USCI_A1_BASE,USCI_A_UART_RECEIVE_INTERRUPT);
}

void TimerA0_Init(void)
{
    //���ö�ʱ��AΪ������ģʽ
    //ѡ��SMLK��Ϊʱ��Դ
    //�ж�Ƶ��Ϊ400Hz
    //�ж�Ƶ�� = ʱ��ԴƵ�� / (��Ƶϵ�� * Ԥװ��ֵ)
    Timer_A_initUpModeParam timer_a_param = {0};
    timer_a_param.clockSource = TIMER_A_CLOCKSOURCE_SMCLK;
    timer_a_param.captureCompareInterruptEnable_CCR0_CCIE = TIMER_A_CCIE_CCR0_INTERRUPT_ENABLE;
    timer_a_param.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_16;
    timer_a_param.timerPeriod = 625;
    timer_a_param.timerClear = TIMER_A_DO_CLEAR;
    timer_a_param.startTimer = true;
    Timer_A_initUpMode(TIMER_A0_BASE, &timer_a_param);

    Timer_A_startCounter( TIMER_A0_BASE,TIMER_A_UP_MODE);
}

void TimerB0_Init(void)
{
    //���ö�ʱ��BΪ������ģʽ
    //ѡ��SMLK��Ϊʱ��Դ
    //�ж�Ƶ��Ϊ10Hz
    //�ж�Ƶ�� = ʱ��ԴƵ�� / (��Ƶϵ�� * Ԥװ��ֵ)
    Timer_B_initUpModeParam timer_b_param = {0};
    timer_b_param .clockSource = TIMER_B_CLOCKSOURCE_SMCLK;
    timer_b_param .captureCompareInterruptEnable_CCR0_CCIE = TIMER_B_CCIE_CCR0_INTERRUPT_ENABLE;
    timer_b_param .clockSourceDivider = TIMER_B_CLOCKSOURCE_DIVIDER_64;
    timer_b_param .timerPeriod = 6250;
    timer_b_param .timerClear = TIMER_B_DO_CLEAR;
    timer_b_param .startTimer = true;
    Timer_B_initUpMode(TIMER_B0_BASE, &timer_b_param );

    Timer_B_startCounter( TIMER_B0_BASE,TIMER_B_UP_MODE);
}
