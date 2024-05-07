#include <iostream>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <i2c/smbus.h>

#define I2C_BUS "/dev/i2c-1" // Adjust based on your Jetson model and connected device
#define DEVICE_ADDRESS 0x53 // Change this to the address of your device
#define REGISTER_TO_READ 0x10 // Change this to the register you want to read

int main() {
    int file;

    // Open the I2C bus
    if ((file = open(I2C_BUS, O_RDWR)) < 0) {
        std::cerr << "Failed to open the bus." << std::endl;
        return 1;
    }

    // Set the slave address
    if (ioctl(file, I2C_SLAVE, DEVICE_ADDRESS) < 0) {
        std::cerr << "Failed to acquire bus access and/or talk to slave." << std::endl;
        return 1;
    }

    // Read one byte from the register
    __s32 result = i2c_smbus_read_byte_data(file, REGISTER_TO_READ);
    if (result < 0) {
        std::cerr << "Failed to read from the I2C bus." << std::endl;
        return 1;
    }

    // Print the value read
    std::cout << "Value read from register 0x" << std::hex << REGISTER_TO_READ << ": " << result << std::endl;

    // Close the I2C bus
    close(file);

    return 0;
}