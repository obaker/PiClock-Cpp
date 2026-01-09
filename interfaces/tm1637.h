#ifndef __LIBTM1637_H__
#define __LIBTM1637_H__

#include <gpiod.h>

#ifdef __cplusplus
extern "C" {
#endif

int tm1637_init(struct gpiod_chip *chip, int clk_pin, int dio_pin);
void tm1637_brightness(uint8_t brightness);
void tm1637_show(char *display);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LIBTM1637_H*/
