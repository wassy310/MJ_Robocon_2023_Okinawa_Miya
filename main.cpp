#include "mbed.h"

#define max_speed 1.0
// #define max_rotate 0.05

RawSerial pc(USBTX, USBRX, 9600);
RawSerial sbdbt(PA_0, PA_1, 2400);

DigitalOut chassis[3] = {PA_10, PA_8, PB_5};    // r, arm, l
PwmOut chassis_p[3] = {PB_10, PB_4, PC_7};    // r, arm, l

int PS3[7];
// int flag_command;
// int counter;

void PS3_data() {
    int sbdbt_data = sbdbt.getc();
    static int bits = 0;

    if(sbdbt_data == 128) {
        bits = 0;
    }

    if (sbdbt_data >= 0) {
        PS3[bits] = sbdbt_data;

        if (bits == 7) {
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
    chassis_p[0].period_us(100);
    chassis_p[1].period_us(100);
    chassis_p[2].period_us(100);

    pc.printf("PWM initialization is complete.");
    wait_us(200000);
}

void move_pwm(double status_chassis) {
    chassis_p[0] = status_chassis * max_speed;
    //chassis_p[1] = status_chassis * max_speed;
    chassis_p[2] = status_chassis * max_speed;
    
    pc.printf("%x\tstatus = %lf\r\n", PS3[2], status_chassis);
}

// void init_arm() {
//     chassis[1] = 0.0;
//     wait_us(100000);

//     chassis[1].period_ms(12);

//     flag_command = 0;
//     counter = 0;

//     pc.printf("Arm initalization is complete.\r\n");
//     wait_us(100000);
// }

void setup() {
    config();

    pc.printf("Enter both sticks below.\r\n");
    do {
        wait_us(1000);
    } while((PS3[4] != 0x7F) && (PS3[6] != 0x7F));

    init_pwm();

    pc.printf("Robot initialization is complete.\r\n");
    wait_us(500000);

    pc.printf("To exit, press the '〇' button.\r\n");
    do {
        wait_us(1000);
    } while(PS3[2] != 0x40);
    wait_us(1000000);
}

void move_chassis() {
    switch(PS3[2]) {
        case 0x01:
            chassis[0] = 0;
            chassis[2] = 1;
            pc.printf("Robot is moving forward.\r\n");
            move_pwm(0.5);
            break;

        case 0x02:
            chassis[0] = 1;
            chassis[2] = 0;
            pc.printf("Robot is moving Back.\r\n");
            move_pwm(0.5);
            break;

        default:
            pc.printf("%d\r\n", PS3[1]);


        if(PS3[2] == 0x10) {
            switch(PS3[1]) {
                case 0x04:
                    chassis[0] = 0;
                    chassis[2] = 0;
                    move_pwm(0.5);
                    pc.printf("Robot is rotating Left.\r\n");
                    break;

                case 0x10:
                    chassis[0] = 1;
                    chassis[2] = 1;
                    move_pwm(0.5);
                    pc.printf("Robot is rotating Right.\r\n");
                    break;

                default:
                    move_pwm(0);
            }
        } else
            move_pwm(0);
    }
}

// void move_arm() {
//     chassis[1] = 0;

//     switch(PS3[2]){
//         //△＋上キーの入力: 上
//         case 0x11:
//             chassis[1] = 0;
//             chassis[1] = max_rotate * 0.4;
//             pc.printf("UNIT_C is rotating to the Up.\r\n");
//             break;
//         //△＋下キーの入力: 下
//         case 0x12:
//             chassis[1] = 1;
//             chassis[1] = max_rotate;
//             pc.printf("UNIT_C is rotating to the Down.\r\n");
//             break;
//         default:
//             chassis[1] = 0;
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
