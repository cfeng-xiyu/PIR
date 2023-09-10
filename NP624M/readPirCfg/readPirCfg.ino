#include <stdlib.h>

const int interruptPin = 2;         /* 替换为你要使用的GPIO引脚号Serial1 */
const int readPirPin = 3;
volatile bool buttonState = LOW;    /* 初始状态为低电平 */
unsigned long debounceDelay = 10;   /* 设置消抖延迟时间，单位为毫秒 */
unsigned long lastDebounceTime = 0;
unsigned long pir_cfg = 0;

char PIR_OUT, DATA_H, DATA_L, SENS_R, BLIND_R, PULSE_R, WINDOW_R, MOTION_R, INT_R;
char VOLT_R, SUPP_R, RSV_R, BUF1, CNT;
char gBuf[128];


void onInterrupt()
{
    buttonState = HIGH;
    lastDebounceTime = millis();
}

/* ======= 读结束清零子程序 ======= */
void RD_END()
{
    pinMode(readPirPin, OUTPUT);
    digitalWrite(readPirPin, LOW);
    delayMicroseconds(200);         /* 延时必须准确，共200us */
    pinMode(readPirPin, INPUT);     /* 将引脚设置为浮空输入模式 */
}

/* ======= 读Nbit ======= */
void RD_NBIT(unsigned char num)
{
    unsigned char i;
    BUF1 = 0x00;

    for (i = 0; i < num; i++)
    {
        pinMode(readPirPin, OUTPUT);
        digitalWrite(readPirPin, LOW);
        delayMicroseconds(2);       /* 延时必须准确，共2us */

        digitalWrite(readPirPin, HIGH); 
        delayMicroseconds(2);       /* 延时必须准确，共2us */

        pinMode(readPirPin, INPUT); /* 将引脚设置为浮空输入模式 */
        delayMicroseconds(2);       /* 延时必须准确，共2us */

        BUF1 = BUF1 << 1;
        if(digitalRead(readPirPin) == HIGH)
        {
            BUF1 = BUF1 + 1;
        }
        else
        {
            BUF1 = BUF1 + 0;
        }
    }
    return;
}

/* ======= 强制DOCI中断子程序 ======= */
void F_INT()
{
    pinMode(readPirPin, OUTPUT);
    digitalWrite(readPirPin, HIGH);
    delayMicroseconds(200);         /* 延时必须准确，共2us */
}

/* ======= DOCI读出 ======= */
void readPirCfg()
{
    F_INT();

    PIR_OUT = 0X00;
    RD_NBIT(1);
    PIR_OUT = BUF1;

    DATA_H = 0X00;
    RD_NBIT(6);
    DATA_H = BUF1;

    DATA_L = 0X00;
    RD_NBIT(8);
    DATA_L = BUF1;

    SENS_R = 0X00;
    RD_NBIT(8);
    SENS_R = BUF1;

    BLIND_R = 0X00;
    RD_NBIT(4);
    BLIND_R = BUF1;

    PULSE_R = 0X00;
    RD_NBIT(2);
    PULSE_R = BUF1;

    WINDOW_R = 0X00;
    RD_NBIT(2);
    WINDOW_R = BUF1;

    MOTION_R = 0X00;
    RD_NBIT(1);
    MOTION_R = BUF1;

    INT_R = 0X00;
    RD_NBIT(1);
    INT_R = BUF1;

    VOLT_R = 0X00;
    RD_NBIT(2);
    VOLT_R = BUF1;

    SUPP_R = 0X00;
    RD_NBIT(1);
    SUPP_R = BUF1;

    RSV_R = 0X00;
    RD_NBIT(4);
    RSV_R = BUF1;

    RD_END();
}

void printPirCfg()
{
    pir_cfg = (SENS_R << 17) | (BLIND_R << 13) | (PULSE_R << 11) | (WINDOW_R << 9) | (MOTION_R << 8) | (INT_R << 7) | (VOLT_R << 5) | (SUPP_R << 4) | (RSV_R << 0);
    snprintf(gBuf, sizeof(gBuf), "PirCfg\t\t\t= 0x%x", pir_cfg);
    Serial1.println(gBuf);

    Serial1.print("Sensitivity[24:17]\t= ");
    snprintf(gBuf, sizeof(gBuf), "%d", SENS_R);
    Serial1.println(gBuf);

    Serial1.print("BlindTime[16:13]\t= ");
    snprintf(gBuf, sizeof(gBuf), "%d", BLIND_R);
    Serial1.println(gBuf);

    Serial1.print("PluseCounter[12:11]\t= ");
    snprintf(gBuf, sizeof(gBuf), "%d", PULSE_R);
    Serial1.println(gBuf);

    Serial1.print("WindowTime[10:9]\t= ");
    snprintf(gBuf, sizeof(gBuf), "%d", WINDOW_R);
    Serial1.println(gBuf);

    Serial1.print("MotionDetectorEnable[8]\t= ");
    snprintf(gBuf, sizeof(gBuf), "%d", MOTION_R);
    Serial1.println(gBuf);

    Serial1.print("InterruptSource[7]\t= ");
    snprintf(gBuf, sizeof(gBuf), "%d", INT_R);
    Serial1.println(gBuf);

    Serial1.print("ADCVoltageSource[6:5]\t= ");
    snprintf(gBuf, sizeof(gBuf), "%d", VOLT_R);
    Serial1.println(gBuf);

    Serial1.print("PIRPowerEnable[4]\t= ");
    snprintf(gBuf, sizeof(gBuf), "%d", SUPP_R);
    Serial1.println(gBuf);

    Serial1.print("TestModes[3:0]\t\t= ");
    snprintf(gBuf, sizeof(gBuf), "%d", RSV_R);
    Serial1.println(gBuf);
}

void setup()
{
    pinMode(interruptPin, INPUT_PULLDOWN); /* 将引脚设置为输入模式，并启用下拉电阻 */
    attachInterrupt(digitalPinToInterrupt(interruptPin), onInterrupt, HIGH); /* 附加中断处理函数到高电平触发 */
    Serial1.begin(9600); /* 初始化串口通信，波特率设置为9600 */
}

void loop()
{
    unsigned long currentTime = millis();
    if ((buttonState == HIGH) && (currentTime - lastDebounceTime > debounceDelay))
    {
        /* 检测到高电平且消抖时间已过，执行按键按下后的操作 */
        Serial1.println("\n*** Start read pir cfg ***");
        readPirCfg();
        printPirCfg();
        buttonState = LOW;
    }
}

