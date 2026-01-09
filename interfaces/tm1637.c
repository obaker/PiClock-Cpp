#include <gpiod.h>
#include <stdio.h>
#include <time.h>
// Library to interface with the TM1637 LED display


const unsigned char digitsToSegment[] = {0x3f, 0x6, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f};
static unsigned char clk_pin, dio_pin;
struct gpiod_chip *chip;
struct gpiod_line_request* clk_request;
struct gpiod_line_request* dio_request;
static struct timespec clk_delay = {0, 5000}; // Clock delay in seconds and nano seconds

#define LOW 0
#define HIGH 1

int gpio_init(struct gpiod_line_request **request_ptr, int pin){
    struct gpiod_line_settings *settings = NULL;
    struct gpiod_line_config *line_cfg = NULL;

    settings = gpiod_line_settings_new();
    if (!settings) return -1;

    // Configure as output
    gpiod_line_settings_set_direction(settings, GPIOD_LINE_DIRECTION_OUTPUT);

    line_cfg = gpiod_line_config_new();
    if (!line_cfg) {
        gpiod_line_settings_free(settings);
        return -1;
    }

    if (gpiod_line_config_add_line_settings(line_cfg, &pin, 1, settings) < 0) {
        perror("gpiod_line_config_add_line_settings");
        gpiod_line_config_free(line_cfg);
        gpiod_line_settings_free(settings);
        return -1;
    }

    // Request the line - this "locks" the pin for our use
    *request_ptr = gpiod_chip_request_lines(chip, NULL, line_cfg);
    if (!*request_ptr) {
        perror("gpiod_chip_request_lines");
    }
    return 0;
}

int write_gpio(struct gpiod_line_request *request, int pin, int value){
    if (!request) return -1;
    enum gpiod_line_value g_val = value ? GPIOD_LINE_VALUE_ACTIVE : GPIOD_LINE_VALUE_INACTIVE;
    if (gpiod_line_request_set_value(request, pin, g_val) < 0) {
        perror("gpiod_line_request_set_value");
        return -1;
    }

    return 0;
}

int tm1637_init(struct gpiod_chip *chip_ptr, int clk, int dio){
    chip = chip_ptr;
    clk_pin = clk;
    dio_pin = dio;
    if (gpio_init(&clk_request, clk) != 0 || gpio_init(&dio_request, dio) != 0) {
        return -1;
    }
    write_gpio(dio_request, dio_pin, LOW);
    write_gpio(clk_request, clk_pin, LOW);
    printf("TM1637 initialised\n");
    return 0;
}

static void tm1637_start(){
    write_gpio(clk_request, clk_pin, HIGH);
    write_gpio(dio_request, dio_pin, HIGH);
    nanosleep(&clk_delay, NULL);
    write_gpio(dio_request, dio_pin, LOW);
}

static void tm1637_stop(){
    write_gpio(clk_request, clk_pin, LOW);
    nanosleep(&clk_delay, NULL);
    write_gpio(dio_request, dio_pin, LOW);
    nanosleep(&clk_delay, NULL);
    write_gpio(clk_request, clk_pin, HIGH);
    nanosleep(&clk_delay, NULL);
    write_gpio(dio_request, dio_pin, HIGH);
}

static void tm1637_ack(){
    write_gpio(clk_request, clk_pin, LOW);
    write_gpio(dio_request, dio_pin, HIGH);
    nanosleep(&clk_delay, NULL);
    write_gpio(clk_request, clk_pin, HIGH);
    nanosleep(&clk_delay, NULL);
}

static void tm1637_write_byte(uint8_t byte){
    for (size_t i = 0; i < 8; i++)
    {
        write_gpio(clk_request, clk_pin, LOW);
        write_gpio(dio_request, dio_pin, (byte & 0x01));
        nanosleep(&clk_delay, NULL);
        write_gpio(clk_request, clk_pin, HIGH);
        nanosleep(&clk_delay, NULL);
        byte >>= 1;
    }
    tm1637_ack();
}

static void tm1637_write(uint8_t *data, uint8_t len){
    tm1637_start();
    for (size_t i = 0; i < len; i++)
    {
        tm1637_write_byte(data[i]);
    }
    tm1637_stop();
}

void tm1637_brightness(uint8_t brightness){
    uint8_t data;
    if (brightness == 0) data = 0x80; // Display off
    else
    {
        if (brightness > 8) brightness = 8;
        data = 0x88 | (brightness - 1);
    }
    tm1637_write(&data, 1);
}

void tm1637_show(char *display){
    tm1637_start();
    tm1637_write_byte(0x40); // Auto address increment.
    tm1637_stop();

    tm1637_start();
    tm1637_write_byte(0xC0); // Start at first digit
    for (int i = 0; i < 4; i++) {
        uint8_t segments = 0;
        if (display[i] >= '0' && display[i] <= '9') {
            segments = digitsToSegment[display[i] - '0'];
        }
        // Add colon for divider
        if (i == 1 && display[4] == ':') {
            segments |= 0x80;
        }
        tm1637_write_byte(segments);
    }
    tm1637_stop();
}
