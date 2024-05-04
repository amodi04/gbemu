#include <bus.h>
#include <cart.h>
#include <ram.h>
#include <cpu.h>
#include <io.h>

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
    // ROM banks located at 0x0000 - 0x7FFF
    if (address < 0x8000) {
        return cart_read(address);
    } else if (address < 0xA000) {
        // Char/Map Data
        // TODO
        printf("Unsupported bus_read(0x%04X)\n", address);
        // NO_IMPL
        return 0x0;
    } else if (address < 0xC000) {
        // Cartridge RAM
        return cart_read(address);
    } else if (address < 0xE000) {
        // WRAM (Working RAM)
        return wram_read(address);
    } else if (address < 0xFE00) {
        // Reserved ECHO RAM
        return 0;
    } else if (address < 0xFEA0) {
        // OAM
        // TODO
        printf("Unsupported bus_read(0x%04X)\n", address);
        // NO_IMPL
        return 0x0;
    } else if (address < 0xFF00) {
        // Not Usable
        return 0;
    } else if (address < 0xFF80) {
        // I/O Registers
        return io_read(address);
    } else if (address == 0xFFFF) {
        // CPU Interrupt Enable Register
        return cpu_get_ie_register();
    }
    
    return hram_read(address);
}

void bus_write(u16 address, u8 value) {
    // ROM banks located at 0x0000 - 0x7FFF
    if (address < 0x8000) {
        cart_write(address, value);
    } else if (address < 0xA000) {
        // Char/Map Data
        // TODO
        printf("Unsupported bus_write(0x%04X, 0x%02X)\n", address, value);
        // NO_IMPL
    } else if (address < 0xC000) {
        // Cartridge RAM
        cart_write(address, value);
    } else if (address < 0xE000) {
        // WRAM (Working RAM)
        wram_write(address, value);
    } else if (address < 0xFE00) {
        // Reserved ECHO RAM
    } else if (address < 0xFEA0) {
        // OAM
        // TODO
        printf("Unsupported bus_write(0x%04X, 0x%02X)\n", address, value);
        // NO_IMPL
    } else if (address < 0xFF00) {
        // Not Usable
    } else if (address < 0xFF80) {
        // I/O Registers
        io_write(address, value);
    } else if (address == 0xFFFF) {
        // CPU Interrupt Enable Register
        cpu_set_ie_register(value);
    } else {
        // High RAM
        hram_write(address, value);
    }
}

u16 bus_read16(u16 address) {
    u16 lo = bus_read(address);
    u16 hi = bus_read(address + 1);
    return lo | (hi << 8);
}

void bus_write16(u16 address, u16 value) {
    // Write the high byte first
    bus_write(address + 1, (value >> 8) & 0xFF);
    bus_write(address, value & 0xFF);
}