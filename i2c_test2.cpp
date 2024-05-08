#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>


class deserializer9702 {
    public:
        deserializer9702(char *i2cInterfaceName, uint8_t devAddr) : device(i2cInterfaceName), devAddr(devAddr) {};
        bool i2cOpen();
        bool i2cWrite(const uint8_t regAddr, const uint8_t regVal);
        bool i2cRead(const uint8_t regAddr, uint8_t *regVal);
        bool i2cClose();

        private: 
            /// @brief File descriptor for the I2C bus
            int file;
            /// @brief I2C device name[]
            const char *device;
            /// @brief I2C device address
            const uint8_t devAddr;
            /// @brief Register data (for return value on reads)
            unsigned char data;
};

bool deserializer9702::i2cOpen(){
    file = open(device, O_RDWR);
    if (file < 0) {
        std::cerr << "Failed to open the I2C bus." << std::endl;
        return false;
    }

    // Set the I2C device address
    if (ioctl(file, I2C_SLAVE, devAddr) < 0) {
        std::cerr << "Failed to set I2C address." << std::endl;
        close(file);
        return false;
    }

    //Return indicating success opening the i2c device
    return true;
}

bool deserializer9702::i2cWrite(const uint8_t regAddr, const uint8_t regVal) {
    //Length of the write operation (register address (1 byte) + value to write to the register (1 byte))
    const int length = 2;

    //Buffer to write to the i2c bus
    uint8_t buffer[length] = {regAddr, regVal};

    // Write to the register
    if (write(file, buffer, length) != length) {
        std::cerr << "Failed to write to I2C device." << std::endl;
        return false;
    }
    return true;
}

bool deserializer9702::i2cRead(const uint8_t regAddr, uint8_t *regVal) {
    // Read from the register
    if (write(file, &regAddr, 1) != 1) {
        std::cerr << "Failed to write register address." << std::endl;
        close(file);
        return false;
    }
    
    unsigned char data;
    if (read(file, regVal, 1) != 1) {
        std::cerr << "Failed to read from register." << std::endl;
        close(file);
        return false;
    }

    return true;
}

bool deserializer9702::i2cClose(){
    // Close the I2C bus
    close(file);

    //Return true indicating success closing the i2c device
    return true;
}

deserializer9702 *deserializer;

//I2C interface name
#define I2C_INTERFACE_NAME "/dev/i2c-2"
//I2C device address (TI 9702 deserializer)
#define DESERIALIZER_DEVICE_ADDRESS 0x30

int main() {
    bool status = false;

    deserializer = new deserializer9702(I2C_INTERFACE_NAME, DESERIALIZER_DEVICE_ADDRESS);
    if (deserializer == NULL){
        std::cerr << "Failed to create deserializer object." << std::endl;
        return 1;
    }

    //Initialize the deserializer object by opening the i2c interface
    status = deserializer->i2cOpen();
    if (!status){
        std::cerr << "Failed to open the I2C bus." << std::endl;
        return 1;
    }

    uint8_t data;

    for (int i = 0; i < 255; i++){
        deserializer->i2cWrite((uint8_t)0x27, (uint8_t)i);
        //Read back the register
        deserializer->i2cRead((uint8_t)0x27, &data);
        if (data != i) {
            std::cout << "Failed to write to register at write " << i << std::endl;
            break;
        }
    }
    
    std::cout << "Successfully wrote and read to register 255 times..." << std::endl;

    // Close the I2C bus
    deserializer->i2cClose();

    return 0;
}