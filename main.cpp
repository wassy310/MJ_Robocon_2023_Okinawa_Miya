#include "mbed.h"

#define max_speed 1.0    // モータの最大回転速度
#define max_rotate 0.05    // アームの最大回転速度

int PS3[7];
int i;

RawSerial pc(USBTX, USBRX, 9600);
RawSerial sbdbt(PA_0, PA_1, 2400);

DigitalOut motor[3] = {PA_10, PA_8, PB_5};    // 右輪、アーム、左輪
PwmOut motor_p[3] = {PB_10, PB_4, PC_7};    // 右輪、アーム、左輪

void PS3_data() {
    int sbdbt_data = sbdbt.getc();
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
    sbdbt.attach(&PS3_data, Serial::RxIrq);
}

void init_pwm() {
    for(size_t i=0; i<3; i++) {
        motor_p[i].period_us(100);
    }

    pc.printf("PWM initialization is complete.");
    wait_us(100000);
}

void move_pwm_chassis(double status_chassis) {
    motor_p[0] = status_chassis * max_speed;
    motor_p[2] = status_chassis * max_speed;

    pc.printf("%x\tstatus = %lf\r\n", PS3[2], status_chassis);
}

void move_pwm_arm(double status_chassis) {
    motor_p[1] = status_chassis * max_speed;

    pc.printf("%x\tstatus = %lf\r\n", PS3[2], status_chassis);
}

void setup() {
    config();

    pc.printf("Enter both sticks below.\r\n");
    do {
        wait_us(1000);
    } while((PS3[4] != 0x7F) && (PS3[6] != 0x7F));

    init_pwm();

    pc.printf("Robot initialization is complete.\r\n");
    wait_us(500000);
}

void move_chassis() {
    switch(PS3[2]) {
        case 0x01:
            motor[0] = 0;
            motor[2] = 1;
            pc.printf("Robot is moving forward.\r\n");
            move_pwm_chassis(0.5);
            break;

        case 0x02:
            motor[0] = 1;
            motor[2] = 0;
            pc.printf("Robot is moving back.\r\n");
            move_pwm_chassis(0.5);
            break;

        case 0x04:
            motor[0] = 1;
            motor[2] = 1;
            pc.printf("Robot is moving right.\r\n");
            move_pwm_chassis(0.5);
            break;

        case 0x08:
            motor[0] = 0;
            motor[2] = 0;
            pc.printf("Robot is moving left.\r\n");
            move_pwm_chassis(0.5);
            break;

        case 0x11:
            motor[1] = 1;
            pc.printf("Arm is rising.\r\n");
            move_pwm_arm(0.5);
            break;

        case 0x12:
            motor[1] = 0;
            pc.printf("Arm is descending.\r\n");
            move_pwm_arm(0.5);
            break;

        default:
            pc.printf("%d\r\n", PS3[1]);
            move_pwm_chassis(0);
            move_pwm_arm(0);
            
            // if(PS3[2] == 0x10) {
            //     switch(PS3[1]) {
            //         case 0x11:
            //             motor[1] = 1;
            //             pc.printf("Arm is rising.\r\n");
            //             move_pwm(0.5);
            //             break;

            //         case 0x12:
            //             motor[1] = 0;
            //             pc.printf("Arm is descending.\r\n");
            //             move_pwm(0.5);
            //             break;

            //         default:
            //             move_pwm(0);
            //     }
            // } else
            //     move_pwm(0);
    }
}

// void move_arm() {
//     switch(PS3[2]) {
//         case 0x11:    //△＋上キーの入力: 上
//             motor[1] = 1;
//             motor_p[1] = max_rotate;
//             pc.printf("Arm is rising.\r\n");
//             move_pwm(0.5);
//             break;

//         case 0x12:    //△＋下キーの入力: 下
//             motor[1] = 0;
//             motor_p[1] = max_rotate;
//             pc.printf("Arm is descending.\r\n");
//             move_pwm(0.5);
//             break;

//         default:
//             pc.printf("%d\r\n", PS3[1]);
//             move_pwm(0);
//     }
// }

int main() {
    pc.printf("Writing completed.\r\n");

    setup();

    while(1) {
        move_chassis();
        // move_arm();
    }
}
