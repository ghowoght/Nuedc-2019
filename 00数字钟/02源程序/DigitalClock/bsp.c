#include "driverlib.h"

void set_freq_4MHz(void)
{
    /* 设置主频为4MHz */
    PMM_setVCore(PMM_CORE_LEVEL_1);//主频提高后，VCore电压也需要随之配置
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P7,GPIO_PIN2 + GPIO_PIN3);//XT2口不作为普通IO
    UCS_turnOnXT2WithTimeout(UCS_XT2_DRIVE_4MHZ_8MHZ,5000);//启动XT2
    UCS_initClockSignal(UCS_FLLREF,UCS_XT2CLK_SELECT,UCS_CLOCK_DIVIDER_1);//XT2作为FLL参考
    UCS_initFLLSettle(24000,6);//MCLK设置为24MHz
    UCS_initClockSignal(UCS_ACLK,UCS_REFOCLK_SELECT,UCS_CLOCK_DIVIDER_1);//ACLK设置为32.768kHz
    UCS_initClockSignal(UCS_SMCLK,UCS_XT2CLK_SELECT,UCS_CLOCK_DIVIDER_1);//SMCLK设置为4MHz
}

void GPIO_Init(void)
{
    /* 初始化LED为输出模式 */
    GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN7);
    GPIO_setAsOutputPin(GPIO_PORT_P8, GPIO_PIN0);
    GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN7);
    GPIO_setOutputHighOnPin(GPIO_PORT_P8, GPIO_PIN0);

    /* 初始化oled管脚 */
    GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN0 | GPIO_PIN1 | GPIO_PIN2 | GPIO_PIN3 | GPIO_PIN4);
    GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN0 | GPIO_PIN1 | GPIO_PIN2 | GPIO_PIN3 | GPIO_PIN4);

    /* 初始化矩阵键盘管脚 */
    GPIO_setAsOutputPin(GPIO_PORT_P4, GPIO_PIN0 | GPIO_PIN1 | GPIO_PIN2 | GPIO_PIN3);
    GPIO_setOutputHighOnPin(GPIO_PORT_P4, GPIO_PIN0 | GPIO_PIN1 | GPIO_PIN2 | GPIO_PIN3);

    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P4, GPIO_PIN4 | GPIO_PIN5 | GPIO_PIN6 | GPIO_PIN7);

    /* 初始化板载按键管脚为上拉输入模式 */
    //上拉即当没有输入时，管脚电平为高电平
    //https://blog.csdn.net/quinn1994/article/details/82530590
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P2, GPIO_PIN6);
    //下降沿触发
    GPIO_selectInterruptEdge(GPIO_PORT_P2, GPIO_PIN6, GPIO_HIGH_TO_LOW_TRANSITION);
    GPIO_enableInterrupt(GPIO_PORT_P2, GPIO_PIN6);

}

void Uart_Init(void)
{
    /* 开启串口 */
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P8, GPIO_PIN2 + GPIO_PIN3);
    //Baudrate = 115200, clock freq = 4MHz
    //UCBRx = 时钟源频率 / 波特率 = 4Mhz / 115200Hz ≈ 34.7222 = 35
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
    //配置定时器A为增计数模式
    //选择SMLK作为时钟源
    //中断频率为400Hz
    //中断频率 = 时钟源频率 / (分频系数 * 预装载值)
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
    //配置定时器B为增计数模式
    //选择SMLK作为时钟源
    //中断频率为10Hz
    //中断频率 = 时钟源频率 / (分频系数 * 预装载值)
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
