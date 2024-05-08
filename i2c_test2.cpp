#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

/*!
 * @brief Structure used to hold a SerDes register entry (8-bit address)
 */
struct SerdesReg_t
{
    uint8_t addr;
    uint8_t val;
};

SerdesReg_t end_both[] = {
            // set transmitter speed to 1.5gbps
            {0x1F, 0x00},
            {0xC9, 0x0F},
            {0xB0, 0x1C},
            {0xB1, 0x92},
            {0xB2, 0x82},
            {0x01, 0x01},
            {0x32, 0x01},
            {0x33, 0x43},  // 0x41
            {0x34, 0x42},
            {0x0C, 0x0C},
            {0x02, 0x1E},
            {0x07, 0xFE}
        };

class deserializer9702 {
    public:
        deserializer9702(char *i2cInterfaceName, uint8_t devAddr) : device(i2cInterfaceName), devAddr(devAddr) {};
        //i2c functions
        bool i2cOpen();
        bool i2cWrite(const uint8_t regAddr, const uint8_t regVal);
        bool i2cRead(const uint8_t regAddr, uint8_t *regVal);
        bool i2cClose();
        //9702 specific functions
        bool init9702();
        bool reset9702();
        bool init9702Port(int portNum);


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
        i2cClose();
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
        i2cClose();
        return false;
    }
    return true;
}

bool deserializer9702::i2cRead(const uint8_t regAddr, uint8_t *regVal) {
    // Read from the register
    if (write(file, &regAddr, 1) != 1) {
        std::cerr << "Failed to write register address." << std::endl;
        i2cClose();
        return false;
    }
    
    unsigned char data;
    if (read(file, regVal, 1) != 1) {
        std::cout << "Failed to read from register." << std::endl;
        i2cClose();
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

bool deserializer9702::init9702(){
    std::cout << "Initializing 9702..." << std::endl;

    //Reset the 9702 (perform a digital reset)
    i2cWrite(0x01, 0x02);

    // Sleep for 1 second to allow the 9702 to reset properly
    usleep(1E6);

    //CSI_EXCLUSE_FWD2 - Disable FPD4 Auto recover
    i2cWrite(0x3C, 0x0F);

    //Initialize the 9702 ports for receiving data
    init9702Port(0);
    init9702Port(1);

    //Reset the 9702 (perform a digital reset)
    i2cWrite(0x01, 0x21);

    // Sleep for 20 ms 
    usleep(20E3);

    aeqRestartFPD4OnPort_9702(0);
    aeqRestartFPD4OnPort_9702(1);

    // Sleep for 20 ms 
    usleep(20E3);
    
    checkLockRecoverOnPort_9702(0);
    checkLockRecoverOnPort_9702(1);

    enAEQ_LMS_9702(0);
    enAEQ_LMS_9702(1);

    // Reset the 9702 ()
    i2cWrite(0x01, 0x21);

    // Sleep for 100 ms 
    usleep(100E3);

    enDFE_LMS(0);
    enDFE_LMS(1);

    uint8_t final_AEQ;

    for (int i = 0; i < 2; i++){
        setupPort9702(i);
        i2cWrite(0xB1, 0x2C); //read/write indirect registers
        final_AEQ = i2cRead(0xB2, &final_AEQ);
        std::cout << "Final AEQ value for port " << i <<": " << final_AEQ << std::endl;
    }

    // Sleep for 100 ms
    usleep(100E3);

    

    //Return indicating success initializing the 9702
    std::cout << "9702 has been initialized successfully..." << std::endl;
    return true;
}

bool deserializer9702::reset9702(){
    std::cout << "Resetting 9702..." << std::endl;

    // Perform a digital reset on the 9702
    i2cWrite(0x01, 0x02);



    //Return indicating success resetting the 9702
    std::cout << "9702 has been reset successfully..." << std::endl;

    return true;
}

bool deserializer9702::init9702Port(int portNum){

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