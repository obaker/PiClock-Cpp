#include <stdio.h>
#include <unistd.h>
#include <time.h>

#include "interfaces/tm1637.h"

int main(int argc, char const *argv[])
{
    char* chip_path = "/dev/gpiochip0";
    struct gpiod_chip *chip;
    chip = gpiod_chip_open(chip_path);
    if (!chip) {
        perror("Open chip failed");
        return -1;
    }
    (void) tm1637_init(chip, 5, 6);
    time_t cur_time = time(NULL);
    while (1)
    {
        time(&cur_time);
        struct tm *t = localtime(&cur_time);
        char buf[6];
        sprintf(buf, "%02d%02d", t->tm_hour, t->tm_min);
        if (8 < t->tm_hour && t->tm_hour < 21) { tm1637_brightness(7); }
        else if (t->tm_hour > 20 && t->tm_hour < 22) { tm1637_brightness(2); }
        else { tm1637_brightness(1); } 
        tm1637_show(buf);
        sleep(1);
    }
    return 0;
}
