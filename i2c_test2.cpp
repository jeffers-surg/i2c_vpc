#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

int i2cWrite(const int file, const unsigned char *buffer, const int length) {
    if (write(file, buffer, length) != length) {
        std::cerr << "Failed to write to I2C device." << std::endl;
        return 1;
    }
    return 0;
}

int i2cRead(const int file, const unsigned char *buffer, const int length) {
    // Read from the register
    if (write(file, &registerAddr, 1) != 1) {
        std::cerr << "Failed to write register address." << std::endl;
        close(file);
        return 1;
    }
    
    unsigned char data;
    if (read(file, &data, 1) != 1) {
        std::cerr << "Failed to read from register." << std::endl;
        close(file);
        return 1;
    }
    return 0;
}



int main() {
    const char *device = "/dev/i2c-2"; // Change this to match the correct I2C device
    const int address = 0x30; // Change this to match the address of your I2C device
    const int registerAddr = 0x27; // Change this to the address of the register you want to read/write

    // Open the I2C bus
    int file = open(device, O_RDWR);
    if (file < 0) {
        std::cerr << "Failed to open the I2C bus." << std::endl;
        return 1;
    }

    // Set the I2C device address
    if (ioctl(file, I2C_SLAVE, address) < 0) {
        std::cerr << "Failed to set I2C address." << std::endl;
        close(file);
        return 1;
    }

    // Write to the register
    unsigned char buffer[2] = {registerAddr, 0x02}; // Example data to write to the register
    if (write(file, buffer, 2) != 2) {
        std::cerr << "Failed to write to register." << std::endl;
        close(file);
        return 1;
    }

    // Read from the register
    if (write(file, &registerAddr, 1) != 1) {
        std::cerr << "Failed to write register address." << std::endl;
        close(file);
        return 1;
    }
    
    unsigned char data;
    if (read(file, &data, 1) != 1) {
        std::cerr << "Failed to read from register." << std::endl;
        close(file);
        return 1;
    }

    std::cout << "Data read from register: " << static_cast<int>(data) << std::endl;
    std::cout << std::hex << static_cast<int>(data) << std::endl;

    // Close the I2C bus
    close(file);

    return 0;
}