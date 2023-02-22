#include "mbed.h"

int PS3[7];    // コントローラの信号
int i;    // for文用の変数

RawSerial pc(USBTX, USBRX, 9600);
RawSerial sbdbt(PA_0, PA_1, 2400);

DigitalOut motor[3] = {PB_5, PA_10, PA_8};    // 左輪、右輪、アーム
PwmOut motor_p[3] = {PC_7, PB_10, PB_4};    // 左輪、右輪、アーム

void PS3_data() {    // コントローラからの入力処理
    int sbdbt_data = sbdbt.getc();    // 生データの受け取り
    static int bits = 0;

    if(sbdbt_data == 128) {
        bits = 0;
    }

    if(sbdbt_data >= 0) {
        PS3[bits] = sbdbt_data;

        if(bits == 7) {
            bits = 0;
        } else {
            bits++;
        }
    }
}

void config() {
    sbdbt.attach(&PS3_data, Serial::RxIrq);    // 受信割込みの時にPS3_data()関数を呼び出すよう設定
}

void init_pwm() {
    for(size_t i=0; i<3; i++) {
        motor_p[i].period_us(100);    // PWM周期の調整(速さ・トルク)
    }

    pc.printf("PWM initialization is complete.");
    ThisThread::sleep_for(100);
}

void move_pwm_chassis(double status_chassis) {    // status_chassis: 速度値
    for(i=0; i<2; i++) {
        motor_p[i] = status_chassis;
    }

    pc.printf("%x\tstatus = %lf\r\n", PS3[2], status_chassis);
}

void move_pwm_arm(double status_chassis) {
    motor_p[2] = status_chassis;

    pc.printf("%x\tstatus = %lf\r\n", PS3[2], status_chassis);
}

void setup() {
    config();

    pc.printf("Enter both sticks below.\r\n");
    do {
        ThisThread::sleep_for(10);
    } while((PS3[4] != 0x7F) && (PS3[6] != 0x7F));    // 両スティック下方向入力待ち

    init_pwm();

    pc.printf("Robot initialization is complete.\r\n");
    ThisThread::sleep_for(500);
}

void move_robot() {
    switch(PS3[2]) {
        case 0x01:    // 上キー
            motor[0] = 1;
            motor[1] = 0;
            pc.printf("Robot is moving forward.\r\n");
            move_pwm_chassis(1);
            break;

        case 0x02:    // 下キー
            motor[0] = 0;
            motor[1] = 1;
            pc.printf("Robot is moving back.\r\n");
            move_pwm_chassis(1);
            break;

        case 0x04:    // 右キー
            motor[0] = 1;
            motor[1] = 1;
            pc.printf("Robot is moving right.\r\n");
            move_pwm_chassis(0.5);
            break;

        case 0x08:    // 左キー
            motor[0] = 0;
            motor[1] = 0;
            pc.printf("Robot is moving left.\r\n");
            move_pwm_chassis(0.5);
            break;

        case 0x11:    // △+上キー
            motor[2] = 1;
            pc.printf("Arm is rising.\r\n");
            move_pwm_arm(0.4);
            break;

        case 0x12:    // △+下キー
            motor[2] = 0;
            pc.printf("Arm is descending.\r\n");
            move_pwm_arm(0.2);
            break;

        default:
            pc.printf("%d\r\n", PS3[1]);
            move_pwm_chassis(0);
            move_pwm_arm(0);
    }
}

int main() {
    pc.printf("Writing completed.\r\n");

    setup();

    while(1) {
        move_robot();
    }
}
