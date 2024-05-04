#include <debug.h>
#include <bus.h>

static char debug_msg[1024] = {0};
static int msg_size = 0;

void debug_update() {
    if (bus_read(0xFF02) == 0x81) {
        char c = bus_read(0xFF01);

        debug_msg[msg_size++] = c;

        bus_write(0xFF02, 0);
    }
}

void debug_print() {
    if (debug_msg[0]) {
        printf("debug: %s\n", debug_msg);
    }
}