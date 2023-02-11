#include "mbed.h"

void Config();
void PS3Data();

RawSerial(USBTX, USBRX);
RawSerial(PC_10, PC_11, 2400);

DigitalInOut Chassis[2] = {PF_6, PA_14, PA_13, PF_7};
PwmOut Chassis_p[2] = {PA_6, PB_6, PC_7, PA_7};

int PS3[7];

void init_robot() {
    Config();

    PC.printf("Enter both sticks below.\r\n");
    do {
        wait_ms(1);
    } while((PS3[4] != 0x7F) && (PS3[6] != 0x7F));

    Init_pwm();

    PC.printf("Robot initialization is complete.\r\n\r\n\r\n\r\n");
    wait_ms(500);
    Pc.printf("To exit, press the '〇' button.\r\n");
    do {
        wait_ms(1);
    } while(PS3[2] != 0x40);
    wait_ms(1000);
}

void init_pwm() {
    Chassis_p[0].period_us(100);
    Chassis_p[1].period_us(100);

    PC.printf("PWM initialization is complete.");
    wait_ms(200);
}

void move_chassis() {
    switch(PS3[2]) {
        case 0x01:
            Chassis[0] = 0;
            Chassis[1] = 1;
            Chassis[2] = 1;
            Chassis[3] = 0;
            PC.printf("Robot is moving forward.\r\n");
            move_pwm(1);
            break;

        case 0x02:
            Chassis[0] = 1;
            Chassis[1] = 0;
            Chassis[2] = 0;
            Chassis[3] = 1;
            PC.printf("Robot is moving Back.\r\n");
            move_pwm(1);
            break;

        case 0x04:
            Chassis[0] = 0;
            Chassis[1] = 0;
            Chassis[2] = 1;
            Chassis[3] = 1;
            move_pwm(1);
            PC.printf("Robot is moving Right.\r\n");
            break;

        case 0x08:
            Chassis[0] = 1;
            Chassis[1] = 1;
            Chassis[2] = 0;
            Chassis[3] = 0;
            move_pwm(1);
            PC.printf("Robot is moving Left.\r\n");
            break;

        default:
            PC.printf("%d\r\n", PS3[1]);

            if(PS3[2] == 0x10) {
                switch(PS3[1]) {
                    case 0x04:
                        Chassis[0] = 1;
                        Chassis[1] = 1;
                        Chassis[2] = 1;
                        Chassis[3] = 1;
                        move_pwm(1);
                        PC.printf("Robot is rotating Left.\r\n");
                        break;

                    case 0x10:
                        Chassis[0] = 0;
                        Chassis[1] = 0;
                        Chassis[2] = 0;
                        Chassis[3] = 0;
                        move_pwm(1);
                        PC.printf("Robot is rotating Right.\r\n");
                        break;

                    default:
                        move_pwm(0);
                }
            } else
                move_pwm(0);
    }
}

void move_pwm(int status_chassis) {
    Chassis_p[0] = status_chassis * MAX_SPEED;
    Chassis_p[1] = status_chassis * MAX_SPEED;
    Chassis_p[2] = status_chassis * MAX_SPEED;
    Chassis_p[3] = status_chassis * MAX_SPEED;
    
    PC.printf("%x\tstatus = %d\r\n", PS3[2], status_chassis);
}

void Config() {
    SBDBT.attach(&PS3Data, Serial::RxIrq);
}

void PS3Data() {
    int SBDBT_Data = SBDBT.getc();
    static int bits = 0;
    
    if(SBDBT_Data == 128) {
        bits = 0;
    }

    if (SBDBT_Data >= 0) {
        PS3[bits] = SBDBT_Data;
        
        if (bits == 7) {
            bits = 0;
        } else {
            bits++;
        }
    }
}

int main() {
    PC.printf("Writing Completed.\r\n");

    init_robot();
}
