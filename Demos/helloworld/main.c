#include "stdio.h"
#include "stdarg.h"
#include "ctype.h"
#include "MICO.h"


void application_start(void)
{
    int print1=0,print2=0;
    //MicoGpioInitialize((mico_gpio_t)MFG_SEL, INPUT_PULL_UP);
    printf("hello MICO\r\n");
    MicoGpioInitialize((mico_gpio_t)MFG_SEL, INPUT_PULL_UP);

    while(1) {
        //sleep(2);
        //msleep(1);
        
        if(MicoGpioInputGet((mico_gpio_t)MFG_SEL)){
            if (print1==0)
                printf(" No MFG mode %d\r\n", mico_get_time());
            print1 = 1;
        } else {
            if (print2==0)
                printf( "Enter MFG mode by MFG button\r\n" );
            print2 = 1;
        }
        
    }
}


