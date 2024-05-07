#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <errno.h>

#define I2C_BUS "/dev/i2c-2" // Change this to match the I2C bus you're using
//#define DEVICE_ADDRESS 0x30 // Change this to the address of your device
//#define DEVICE_ADDRESS 0b1100000	
#define REGISTER_TO_READ 0x0 // Change this to the register you want to read

int main() {
    int file;
    char buffer[1];
    int device_address = 0b1100000;

    // Open the I2C bus
    if ((file = open(I2C_BUS, O_RDWR)) < 0) {
        std::cerr << "Failed to open the bus." << std::endl;
        return 1;
    }

    // Set the slave address
    if (ioctl(file, I2C_SLAVE, device_address) < 0) {
        std::cerr << "Failed to acquire bus access and/or talk to slave." << std::endl;
        return 1;
    }

    buffer[0] = REGISTER_TO_READ;

    // Write the register you want to read
    int status = write(file, buffer, 1);

    if (status != 1) {
        std::cout << "errno: " << errno << std::endl;
        std::cerr << "Failed to write to the I2C bus with status " << status << std::endl;
        return 1;
    }

    // Read one byte from the register
    if (read(file, buffer, 1) != 1) {
        std::cerr << "Failed to read from the I2C bus." << std::endl;
        return 1;
    }

    // Print the value read
    std::cout << "Value read from register 0x" << std::hex << REGISTER_TO_READ << ": " << static_cast<int>(buffer[0]) << std::endl;

    // Close the I2C bus
    close(file);

    return 0;
}
