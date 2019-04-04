#include "Max31855.h"

Max31855::Max31855(spi_host_device_t spi_host, int clk_pin, int miso_pin, int cs_pin) :
        mSpiHost(spi_host),
        mSpiClkPin(clk_pin),
        mSpiMisoPin(miso_pin),
        mSpiCsPin(cs_pin),
        mSpiDevice(NULL)
{
}

void Max31855::connect() {
    spi_bus_config_t cfg;
    spi_device_interface_config_t devcfg;

    cfg.mosi_io_num = -1; // Not used
    cfg.miso_io_num = 19;
    cfg.sclk_io_num = 18;
    cfg.quadwp_io_num = -1;
    cfg.quadhd_io_num = -1;
    cfg.max_transfer_sz = 64;
    cfg.flags = SPICOMMON_BUSFLAG_MASTER;
    cfg.intr_flags = 0;

    ESP_ERROR_CHECK( spi_bus_initialize(
        HSPI_HOST,
        &cfg,
        0 // dma chan
    ));

    devcfg.command_bits = 0;
    devcfg.address_bits = 0;
    devcfg.dummy_bits = 0;
    devcfg.mode = 0;
    devcfg.duty_cycle_pos = 128;
    devcfg.cs_ena_pretrans = 1;
    devcfg.cs_ena_posttrans = 1;
    devcfg.clock_speed_hz = 1000000;
    devcfg.input_delay_ns = 0;
    devcfg.spics_io_num = 5;
    devcfg.flags = 0;
    devcfg.queue_size = 1;
    devcfg.pre_cb = NULL;
    devcfg.post_cb = NULL;
    ESP_ERROR_CHECK( spi_bus_add_device(HSPI_HOST, &devcfg, &mSpiDevice) );
}

float Max31855::read() {
    spi_transaction_t trans;
    
    if(!mSpiDevice) {
        printf("Max318555 read() called before connect()\n");
        return 0.0;
    }

    trans.flags = SPI_TRANS_USE_RXDATA;
    trans.length = 32;
    trans.rxlength = 32;
    trans.tx_buffer = NULL;
    
    ESP_ERROR_CHECK(spi_device_polling_transmit(mSpiDevice, &trans));

    uint32_t rxWord = __builtin_bswap32(*(uint32_t*)trans.rx_data);

    uint16_t rawTemp = (rxWord >> 18) & 0x1fff;
    mFaultFlags = rxWord & 0x7;
    mLastReading = (float)rawTemp / 4.0f;
    if(rxWord & (1<<31)) {
        mLastReading *= -1;
    }
    return mLastReading;
}
