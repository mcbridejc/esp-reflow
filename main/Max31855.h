#include "ISensor.h"

#include "driver/gpio.h"
#include "driver/spi_master.h"

class Max31855 : public ISensor {
public:
    Max31855(spi_host_device_t spi_host, int clk_pin, int miso_pin, int cs_pin);

    void connect();

    float read();

    float lastReading() { return mLastReading; }

    uint8_t faultFlag() { return mFaultFlags; }


private:
    spi_host_device_t mSpiHost;
    int mSpiClkPin;
    int mSpiMisoPin;
    int mSpiCsPin;
    spi_device_handle_t mSpiDevice;
    float mLastReading;
    uint8_t mFaultFlags;
};