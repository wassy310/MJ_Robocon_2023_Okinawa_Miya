#include "mbed.h"

#define max_speed 0.4

RawSerial pc(USBTX, USBRX, 9600);
RawSerial sbdbt(A0, A1, 2400);

DigitalInOut chassis[3] = {D2, D7, D4}; // r, arm, l
PwmOut chassis_p[3] = {D6, D5, D9}; // r, arm, l

int PS3[7];

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

    pc.printf("PWM initialization is complete.");
    wait_ms(200);
}

void move_pwm(int status_chassis) {
    chassis_p[0] = status_chassis * max_speed;
    chassis_p[1] = status_chassis * max_speed;
    
    pc.printf("%x\tstatus = %d\r\n", PS3[2], status_chassis);
}

void init_robot() {
    config();

    pc.printf("Enter both sticks below.\r\n");
    do {
        wait_ms(1);
    } while((PS3[4] != 0x7F) && (PS3[6] != 0x7F));

    init_pwm();

    pc.printf("Robot initialization is complete.\r\n\r\n\r\n\r\n");
    wait_ms(500);
    pc.printf("To exit, press the '〇' button.\r\n");
    do {
        wait_ms(1);
    } while(PS3[2] != 0x40);
    wait_ms(1000);
}

void move_chassis() {
    switch(PS3[2]) {
        case 0x01:
            chassis[0] = 0;
            chassis[1] = 1;
            pc.printf("Robot is moving forward.\r\n");
            move_pwm(1);
            break;

        case 0x02:
            chassis[0] = 1;
            chassis[1] = 0;
            pc.printf("Robot is moving Back.\r\n");
            move_pwm(1);
            break;

        default:
            pc.printf("%d\r\n", PS3[1]);

        if(PS3[2] == 0x10) {
            switch(PS3[1]) {
                case 0x04:
                    chassis[0] = 0;
                    chassis[1] = 0;
                    move_pwm(1);
                    pc.printf("Robot is rotating Left.\r\n");
                    break;

                case 0x10:
                    chassis[0] = 1;
                    chassis[1] = 1;
                    move_pwm(1);
                    pc.printf("Robot is rotating Right.\r\n");
                    break;

                default:
                    move_pwm(0);
            }
        } else
            move_pwm(0);
    }
}

int main() {
    pc.printf("Writing Completed.\r\n");

    init_robot();
}
