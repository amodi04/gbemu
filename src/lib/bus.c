#include <bus.h>

// 0x0000 - 0x3FFF: 16KB ROM bank 00 (in cartridge, fixed at bank 00)
// 0x4000 - 0x7FFF: 16KB ROM Bank 01..NN (in cartridge, switchable bank number)
// 0x8000 - 0x9FFF: 8KB Video RAM (VRAM) (switchable bank 0-1 in CGB Mode)
// 0xA000 - 0xBFFF: 8KB External RAM (in cartridge, switchable bank, if any)
// 0xC000 - 0xCFFF: 4KB Work RAM Bank 0 (WRAM)
// 0xD000 - 0xDFFF: 4KB Work RAM Bank 1 (WRAM) (switchable bank 1-7 in CGB Mode)
// 0xE000 - 0xFDFF: Same as C000-DDFF (ECHO) (typically not used)
// 0xFE00 - 0xFE9F: Object Attribute Memory (OAM)
// 0xFEA0 - 0xFEFF: Not Usable
// 0xFF00 - 0xFF7F: I/O Registers
// 0xFF80 - 0xFFFE: High RAM (HRAM)
// 0xFFFF: Interrupt Enable Register

u8 bus_read(u16 address) {
    if (address < 0x8000) {
        // ROM Data
        return cart_read(address);
    }

    NO_IMPL
}

void bus_write(u16 address, u8 value) {
    if (address < 0x8000) {
        // ROM Data
        cart_write(address, value);
        return;
    }

    NO_IMPL
}