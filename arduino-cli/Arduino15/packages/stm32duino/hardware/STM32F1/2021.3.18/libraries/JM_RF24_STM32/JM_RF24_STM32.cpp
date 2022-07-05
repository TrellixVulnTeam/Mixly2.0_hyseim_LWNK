/*
 Copyright (C) 2011 J. Coliz <maniacbug@ymail.com>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 2 as published by the Free Software Foundation.
 */

#include "JM_nRF24L01_STM32.h"
#include "JM_RF24_config_STM32.h"
#include "JM_RF24_STM32.h"


#if ARDUINO_API_VERSION >= 10000 && !defined(__DOXYGEN__)
inline void digitalWrite(uint8_t pin, bool value) 
{
    digitalWrite(pin, value ? HIGH : LOW);
}
#endif

/****************************************************************************/

void JM_RF24_STM32::csn(bool mode)
{
    #if defined(RF24_TINY)
    if (ce_pin != csn_pin) {
        digitalWrite(csn_pin, mode);
    }
    else {
        if (mode == HIGH) {
            PORTB |= (1<<PINB2);  	// SCK->CSN HIGH
            delayMicroseconds(100); // allow csn to settle.
        }
        else {
            PORTB &= ~(1<<PINB2);	// SCK->CSN LOW
            delayMicroseconds(11);  // allow csn to settle
        }
    }
    // Return, CSN toggle complete
    return;

    #elif defined(ARDUINO) && !defined(RF24_spi_TRANSACTIONS)
    // Minimum ideal SPI bus speed is 2x data rate
    // If we assume 2Mbs data rate and 16Mhz clock, a
    // divider of 4 is the minimum we want.
    // CLK:BUS 8Mhz:2Mhz, 16Mhz:4Mhz, or 20Mhz:5Mhz

        #if !defined(SOFTSPI)
        _spi->setBitOrder(MSBFIRST);
        _spi->setDataMode(SPI_MODE0);
                #if !defined(F_CPU) || F_CPU < 20000000
            _spi->setClockDivider(SPI_CLOCK_DIV2);
                    #elif F_CPU < 40000000
            _spi->setClockDivider(SPI_CLOCK_DIV4);
                    #elif F_CPU < 80000000
            _spi->setClockDivider(SPI_CLOCK_DIV8);
                    #elif F_CPU < 160000000
            _spi->setClockDivider(SPI_CLOCK_DIV16);
                    #elif F_CPU < 320000000
            _spi->setClockDivider(SPI_CLOCK_DIV32);
                    #elif F_CPU < 640000000
            _spi->setClockDivider(SPI_CLOCK_DIV64);
                    #elif F_CPU < 1280000000
            _spi->setClockDivider(SPI_CLOCK_DIV128);
                    #else
                        #error "Unsupported CPU frequency. Please set correct SPI divider."
                    #endif

                #endif
    #elif defined(RF24_RPi)
    if(!mode)
      _spi->chipSelect(csn_pin);
    #endif // defined(RF24_RPi)

    #if !defined(RF24_LINUX)
    digitalWrite(csn_pin, mode);
    delayMicroseconds(csDelay);
    #endif // !defined(RF24_LINUX)
}

/****************************************************************************/

void JM_RF24_STM32::ce(bool level)
{
    //Allow for 3-pin use on ATTiny
    if (ce_pin != csn_pin) {
        digitalWrite(ce_pin, level);
    }
}

/****************************************************************************/

inline void JM_RF24_STM32::beginTransaction()
{
    #if defined(RF24_spi_TRANSACTIONS)
    _spi->beginTransaction(SPISettings(RF24_spi_SPEED, MSBFIRST, SPI_MODE0));
    #endif // defined(RF24_spi_TRANSACTIONS)
    csn(LOW);
}

/****************************************************************************/

inline void JM_RF24_STM32::endTransaction()
{
    csn(HIGH);
    #if defined(RF24_spi_TRANSACTIONS)
    _spi->endTransaction();
    #endif // defined(RF24_spi_TRANSACTIONS)
}

/****************************************************************************/

uint8_t JM_RF24_STM32::read_register(uint8_t reg, uint8_t* buf, uint8_t len)
{
    uint8_t status;

    beginTransaction();
    status = _spi->transfer(R_REGISTER | (REGISTER_MASK & reg));
    while (len--) {
        *buf++ = _spi->transfer(0xff);
    }
    endTransaction();

    return status;
}

/****************************************************************************/

uint8_t JM_RF24_STM32::read_register(uint8_t reg)
{
    uint8_t result;

    beginTransaction();
    _spi->transfer(R_REGISTER | (REGISTER_MASK & reg));
    result = _spi->transfer(0xff);
    endTransaction();

    return result;
}

/****************************************************************************/

uint8_t JM_RF24_STM32::write_register(uint8_t reg, const uint8_t* buf, uint8_t len)
{
    uint8_t status;

    beginTransaction();
    status = _spi->transfer(W_REGISTER | (REGISTER_MASK & reg));
    while (len--) {
        _spi->transfer(*buf++);
    }
    endTransaction();

    return status;
}

/****************************************************************************/

uint8_t JM_RF24_STM32::write_register(uint8_t reg, uint8_t value)
{
    uint8_t status;

    IF_SERIAL_DEBUG(printf_P(PSTR("write_register(%02x,%02x)\r\n"), reg, value));

    beginTransaction();
    status = _spi->transfer(W_REGISTER | (REGISTER_MASK & reg));
    _spi->transfer(value);
    endTransaction();

    return status;
}

/****************************************************************************/

uint8_t JM_RF24_STM32::write_payload(const void* buf, uint8_t data_len, const uint8_t writeType)
{
    uint8_t status;
    const uint8_t* current = reinterpret_cast<const uint8_t*>(buf);

    data_len = rf24_min(data_len, payload_size);
    uint8_t blank_len = dynamic_payloads_enabled ? 0 : payload_size - data_len;

    //printf("[Writing %u bytes %u blanks]",data_len,blank_len);
    IF_SERIAL_DEBUG(printf("[Writing %u bytes %u blanks]\n", data_len, blank_len); );

    beginTransaction();
    status = _spi->transfer(writeType);
    while (data_len--) {
        _spi->transfer(*current++);
    }
    while (blank_len--) {
        _spi->transfer(0);
    }
    endTransaction();

    return status;
}

/****************************************************************************/

uint8_t JM_RF24_STM32::read_payload(void* buf, uint8_t data_len)
{
    uint8_t status;
    uint8_t* current = reinterpret_cast<uint8_t*>(buf);

    if (data_len > payload_size) {
        data_len = payload_size;
    }
    uint8_t blank_len = dynamic_payloads_enabled ? 0 : payload_size - data_len;

    //printf("[Reading %u bytes %u blanks]",data_len,blank_len);

    IF_SERIAL_DEBUG(printf("[Reading %u bytes %u blanks]\n", data_len, blank_len); );

    beginTransaction();
    status = _spi->transfer(R_RX_PAYLOAD);
    while (data_len--) {
        *current++ = _spi->transfer(0xFF);
    }
    while (blank_len--) {
        _spi->transfer(0xff);
    }
    endTransaction();

    return status;
}

/****************************************************************************/

uint8_t JM_RF24_STM32::flush_rx(void)
{
    return spiTrans(FLUSH_RX);
}

/****************************************************************************/

uint8_t JM_RF24_STM32::flush_tx(void)
{
    return spiTrans(FLUSH_TX);
}

/****************************************************************************/

uint8_t JM_RF24_STM32::spiTrans(uint8_t cmd)
{

    uint8_t status;

    beginTransaction();
    status = _spi->transfer(cmd);
    endTransaction();

    return status;
}

/****************************************************************************/

uint8_t JM_RF24_STM32::get_status(void)
{
    return spiTrans(RF24_NOP);
}

/****************************************************************************/
#if !defined(MINIMAL)

void JM_RF24_STM32::print_status(uint8_t status)
{
    printf_P(PSTR("STATUS\t\t = 0x%02x RX_DR=%x TX_DS=%x MAX_RT=%x RX_P_NO=%x TX_FULL=%x\r\n"), status, (status & _BV(RX_DR)) ? 1 : 0,
            (status & _BV(TX_DS)) ? 1 : 0, (status & _BV(MAX_RT)) ? 1 : 0, ((status >> RX_P_NO) & 0x07), (status & _BV(TX_FULL)) ? 1 : 0);
}

/****************************************************************************/

void JM_RF24_STM32::print_observe_tx(uint8_t value)
{
    printf_P(PSTR("OBSERVE_TX=%02x: POLS_CNT=%x ARC_CNT=%x\r\n"), value, (value >> PLOS_CNT) & 0x0F, (value >> ARC_CNT) & 0x0F);
}

/****************************************************************************/

void JM_RF24_STM32::print_byte_register(const char* name, uint8_t reg, uint8_t qty)
{
    printf_P(PSTR(PRIPSTR
    "\t ="),name);
    while (qty--) {
        printf_P(PSTR(" 0x%02x"), read_register(reg++));
    }
    printf_P(PSTR("\r\n"));
}

/****************************************************************************/

void JM_RF24_STM32::print_address_register(const char* name, uint8_t reg, uint8_t qty)
{
    printf_P(PSTR(PRIPSTR"\t ="),name);
    while (qty--) {
        uint8_t buffer[addr_width];
        read_register(reg++, buffer, sizeof buffer);

        printf_P(PSTR(" 0x"));
        uint8_t* bufptr = buffer + sizeof buffer;
        while (--bufptr >= buffer) {
            printf_P(PSTR("%02x"), *bufptr);
        }
    }

    printf_P(PSTR("\r\n"));
}

#endif

/****************************************************************************/

JM_RF24_STM32::JM_RF24_STM32(uint16_t _cepin, uint16_t _cspin,  SPIClass *spi)
        :ce_pin(_cepin), csn_pin(_cspin), p_variant(false), payload_size(32), dynamic_payloads_enabled(false), addr_width(5),
         csDelay(5)//,pipe0_reading_address(0)
{
    _spi = spi;
    pipe0_reading_address[0] = 0;
}

/****************************************************************************/

#if defined(RF24_LINUX) && !defined(MRAA)//RPi constructor

JM_RF24_STM32::JM_RF24_STM32(uint16_t _cepin, uint16_t _cspin, uint32_t _spi_speed):
  ce_pin(_cepin),csn_pin(_cspin),spi_speed(_spi_speed),p_variant(false), payload_size(32), dynamic_payloads_enabled(false),addr_width(5)//,pipe0_reading_address(0) 
{
  pipe0_reading_address[0]=0;
}
#endif

/****************************************************************************/

void JM_RF24_STM32::setChannel(uint8_t channel)
{
    const uint8_t max_channel = 125;
    write_register(RF_CH, rf24_min(channel, max_channel));
}

uint8_t JM_RF24_STM32::getChannel()
{

    return read_register(RF_CH);
}

/****************************************************************************/

void JM_RF24_STM32::setPayloadSize(uint8_t size)
{
    payload_size = rf24_min(size, 32);
}

/****************************************************************************/

uint8_t JM_RF24_STM32::getPayloadSize(void)
{
    return payload_size;
}

/****************************************************************************/

#if !defined(MINIMAL)

static const char rf24_datarate_e_str_0[] PROGMEM = "1MBPS";
static const char rf24_datarate_e_str_1[] PROGMEM = "2MBPS";
static const char rf24_datarate_e_str_2[] PROGMEM = "250KBPS";
static const char * const rf24_datarate_e_str_P[] PROGMEM = {
  rf24_datarate_e_str_0,
  rf24_datarate_e_str_1,
  rf24_datarate_e_str_2,
};
static const char rf24_model_e_str_0[] PROGMEM = "nRF24L01";
static const char rf24_model_e_str_1[] PROGMEM = "nRF24L01+";
static const char * const rf24_model_e_str_P[] PROGMEM = {
  rf24_model_e_str_0,
  rf24_model_e_str_1,
};
static const char rf24_crclength_e_str_0[] PROGMEM = "Disabled";
static const char rf24_crclength_e_str_1[] PROGMEM = "8 bits";
static const char rf24_crclength_e_str_2[] PROGMEM = "16 bits" ;
static const char * const rf24_crclength_e_str_P[] PROGMEM = {
  rf24_crclength_e_str_0,
  rf24_crclength_e_str_1,
  rf24_crclength_e_str_2,
};
static const char rf24_pa_dbm_e_str_0[] PROGMEM = "PA_MIN";
static const char rf24_pa_dbm_e_str_1[] PROGMEM = "PA_LOW";
static const char rf24_pa_dbm_e_str_2[] PROGMEM = "PA_HIGH";
static const char rf24_pa_dbm_e_str_3[] PROGMEM = "PA_MAX";
static const char * const rf24_pa_dbm_e_str_P[] PROGMEM = {
  rf24_pa_dbm_e_str_0,
  rf24_pa_dbm_e_str_1,
  rf24_pa_dbm_e_str_2,
  rf24_pa_dbm_e_str_3,
};

    #if defined(RF24_LINUX)
static const char rf24_csn_e_str_0[] = "CE0 (PI Hardware Driven)";
static const char rf24_csn_e_str_1[] = "CE1 (PI Hardware Driven)";
static const char rf24_csn_e_str_2[] = "CE2 (PI Hardware Driven)";
static const char rf24_csn_e_str_3[] = "Custom GPIO Software Driven";
static const char * const rf24_csn_e_str_P[] = {
  rf24_csn_e_str_0,
  rf24_csn_e_str_1,
  rf24_csn_e_str_2,
  rf24_csn_e_str_3,
};
    #endif // defined(RF24_LINUX)

void JM_RF24_STM32::printDetails(void)
{

    #if defined(RF24_RPi)
    printf("================ SPI Configuration ================\n" );
    if (csn_pin < BCM2835_spi_CS_NONE ){
      printf("CSN Pin  \t = %s\n",rf24_csn_e_str_P[csn_pin]);
    }else{
      printf("CSN Pin  \t = Custom GPIO%d%s\n", csn_pin,
      csn_pin==RPI_V2_GPIO_P1_26 ? " (CE1) Software Driven" : "" );
    }
    printf("CE Pin  \t = Custom GPIO%d\n", ce_pin );
    printf("Clock Speed\t = " );
      switch (spi_speed)
      {
          case BCM2835_spi_SPEED_64MHZ : printf("64 Mhz");	break ;
          case BCM2835_spi_SPEED_32MHZ : printf("32 Mhz");	break ;
          case BCM2835_spi_SPEED_16MHZ : printf("16 Mhz");	break ;
          case BCM2835_spi_SPEED_8MHZ  : printf("8 Mhz");	break ;
          case BCM2835_spi_SPEED_4MHZ  : printf("4 Mhz");	break ;
          case BCM2835_spi_SPEED_2MHZ  : printf("2 Mhz");	break ;
          case BCM2835_spi_SPEED_1MHZ  : printf("1 Mhz");	break ;
          case BCM2835_spi_SPEED_512KHZ: printf("512 KHz");	break ;
          case BCM2835_spi_SPEED_256KHZ: printf("256 KHz");	break ;
          case BCM2835_spi_SPEED_128KHZ: printf("128 KHz");	break ;
          case BCM2835_spi_SPEED_64KHZ : printf("64 KHz");	break ;
          case BCM2835_spi_SPEED_32KHZ : printf("32 KHz");	break ;
          case BCM2835_spi_SPEED_16KHZ : printf("16 KHz");	break ;
          case BCM2835_spi_SPEED_8KHZ  : printf("8 KHz");	break ;
          default : printf("8 Mhz");	break ;
      }
      printf("\n================ NRF Configuration ================\n");

    #endif // defined(RF24_RPi)

    print_status(get_status());

    print_address_register(PSTR("RX_ADDR_P0-1"), RX_ADDR_P0, 2);
    print_byte_register(PSTR("RX_ADDR_P2-5"), RX_ADDR_P2, 4);
    print_address_register(PSTR("TX_ADDR\t"), TX_ADDR);

    print_byte_register(PSTR("RX_PW_P0-6"), RX_PW_P0, 6);
    print_byte_register(PSTR("EN_AA\t"), EN_AA);
    print_byte_register(PSTR("EN_RXADDR"), EN_RXADDR);
    print_byte_register(PSTR("RF_CH\t"), RF_CH);
    print_byte_register(PSTR("RF_SETUP"), RF_SETUP);
    print_byte_register(PSTR("CONFIG\t"), NRF_CONFIG);
    print_byte_register(PSTR("DYNPD/FEATURE"), DYNPD, 2);

    printf_P(PSTR("Data Rate\t = "
    PRIPSTR
    "\r\n"),pgm_read_ptr(&rf24_datarate_e_str_P[getDataRate()]));
    printf_P(PSTR("Model\t\t = "
    PRIPSTR
    "\r\n"),pgm_read_ptr(&rf24_model_e_str_P[isPVariant()]));
    printf_P(PSTR("CRC Length\t = "
    PRIPSTR
    "\r\n"),pgm_read_ptr(&rf24_crclength_e_str_P[getCRCLength()]));
    printf_P(PSTR("PA Power\t = "
    PRIPSTR
    "\r\n"),  pgm_read_ptr(&rf24_pa_dbm_e_str_P[getPALevel()]));

}

#endif // !defined(MINIMAL)

/****************************************************************************/

bool JM_RF24_STM32::begin(void)
{

    uint8_t setup = 0;

    #if defined(RF24_LINUX)

        #if defined(MRAA)
    GPIO();
    gpio.begin(ce_pin,csn_pin);
        #endif

        #if defined(RF24_RPi)
    switch(csn_pin){     //Ensure valid hardware CS pin
      case 0: break;
      case 1: break;
      // Allow BCM2835 enums for RPi
      case 8: csn_pin = 0; break;
      case 7: csn_pin = 1; break;
      case 18: csn_pin = 10; break; //to make it work on SPI1
      case 17: csn_pin = 11; break;
      case 16: csn_pin = 12; break;
      default: csn_pin = 0; break;
    }
        #endif // RF24_RPi

    _spi->begin(csn_pin);

    pinMode(ce_pin,OUTPUT);
    ce(LOW);

    delay(100);

    #elif defined(LITTLEWIRE)

    pinMode(csn_pin,OUTPUT);
        _spi->begin();
        csn(HIGH);

    #elif defined(XMEGA_D3)
    if (ce_pin != csn_pin) {
        pinMode(ce_pin,OUTPUT);
    };
    _spi->begin(csn_pin);
    ce(LOW);
    csn(HIGH);
    delay(200);
    #else
    // Initialize pins
    if (ce_pin != csn_pin) {
        pinMode(ce_pin, OUTPUT);
    }

        #if !defined(LITTLEWIRE)
    if (ce_pin != csn_pin)
        #endif // !defined(LITTLEWIRE)
    {
        pinMode(csn_pin, OUTPUT);
    }

    _spi->begin();
    ce(LOW);
    csn(HIGH);
            #if defined(__ARDUINO_X86__)
    delay(100);
            #endif
        #endif //Linux

    // Must allow the radio time to settle else configuration bits will not necessarily stick.
    // This is actually only required following power up but some settling time also appears to
    // be required after resets too. For full coverage, we'll always assume the worst.
    // Enabling 16b CRC is by far the most obvious case if the wrong timing is used - or skipped.
    // Technically we require 4.5ms + 14us as a worst case. We'll just call it 5ms for good measure.
    // WARNING: Delay is based on P-variant whereby non-P *may* require different timing.
    delay(5);

    // Reset NRF_CONFIG and enable 16-bit CRC.
    write_register(NRF_CONFIG, 0x0C);

    // Set 1500uS (minimum for 32B payload in ESB@250KBPS) timeouts, to make testing a little easier
    // WARNING: If this is ever lowered, either 250KBS mode with AA is broken or maximum packet
    // sizes must never be used. See documentation for a more complete explanation.
    setRetries(5, 15);

    // Reset value is MAX
    //setPALevel( RF24_PA_MAX ) ;

    // check for connected module and if this is a p nRF24l01 variant
    //
    if (setDataRate(RF24_250KBPS)) {
        p_variant = true;
    }
    setup = read_register(RF_SETUP);
    /*if( setup == 0b00001110 )     // register default for nRF24L01P
    {
      p_variant = true ;
    }*/

    // Then set the data rate to the slowest (and most reliable) speed supported by all
    // hardware.
    setDataRate(RF24_1MBPS);

    // Initialize CRC and request 2-byte (16bit) CRC
    //setCRCLength( RF24_CRC_16 ) ;

    // Disable dynamic payloads, to match dynamic_payloads_enabled setting - Reset value is 0
    toggle_features();
    write_register(FEATURE, 0);
    write_register(DYNPD, 0);
    dynamic_payloads_enabled = false;

    // Reset current status
    // Notice reset and flush is the last thing we do
    write_register(NRF_STATUS, _BV(RX_DR) | _BV(TX_DS) | _BV(MAX_RT));

    // Set up default configuration.  Callers can always change it later.
    // This channel should be universally safe and not bleed over into adjacent
    // spectrum.
    setChannel(76);

    // Flush buffers
    flush_rx();
    flush_tx();

    powerUp(); //Power up by default when begin() is called

    // Enable PTX, do not write CE high so radio will remain in standby I mode ( 130us max to transition to RX or TX instead of 1500us from powerUp )
    // PTX should use only 22uA of power
    write_register(NRF_CONFIG, (read_register(NRF_CONFIG)) & ~_BV(PRIM_RX));

    // if setup is 0 or ff then there was no response from module
    return (setup != 0 && setup != 0xff);
}

/****************************************************************************/

bool JM_RF24_STM32::isChipConnected()
{
    uint8_t setup = read_register(SETUP_AW);
    if (setup >= 1 && setup <= 3) {
        return true;
    }

    return false;
}

/****************************************************************************/

void JM_RF24_STM32::startListening(void)
{
    #if !defined(RF24_TINY) && !defined(LITTLEWIRE)
    powerUp();
    #endif
    write_register(NRF_CONFIG, read_register(NRF_CONFIG) | _BV(PRIM_RX));
    write_register(NRF_STATUS, _BV(RX_DR) | _BV(TX_DS) | _BV(MAX_RT));
    ce(HIGH);
    // Restore the pipe0 adddress, if exists
    if (pipe0_reading_address[0] > 0) {
        write_register(RX_ADDR_P0, pipe0_reading_address, addr_width);
    } else {
        closeReadingPipe(0);
    }

    // Flush buffers
    //flush_rx();
    if (read_register(FEATURE) & _BV(EN_ACK_PAY)) {
        flush_tx();
    }

    // Go!
    //delayMicroseconds(100);
}

/****************************************************************************/
static const uint8_t child_pipe_enable[]
PROGMEM = {ERX_P0, ERX_P1, ERX_P2, ERX_P3, ERX_P4, ERX_P5};

void JM_RF24_STM32::stopListening(void)
{
    ce(LOW);

    delayMicroseconds(txDelay);

    if (read_register(FEATURE) & _BV(EN_ACK_PAY)) {
        delayMicroseconds(txDelay); //200
        flush_tx();
    }
    //flush_rx();
    write_register(NRF_CONFIG, (read_register(NRF_CONFIG)) & ~_BV(PRIM_RX));

    #if defined(RF24_TINY) || defined(LITTLEWIRE)
    // for 3 pins solution TX mode is only left with additonal powerDown/powerUp cycle
    if (ce_pin == csn_pin) {
      powerDown();
      powerUp();
    }
    #endif
    write_register(EN_RXADDR, read_register(EN_RXADDR) | _BV(pgm_read_byte(&child_pipe_enable[0]))); // Enable RX on pipe0

    //delayMicroseconds(100);

}

/****************************************************************************/

void JM_RF24_STM32::powerDown(void)
{
    ce(LOW); // Guarantee CE is low on powerDown
    write_register(NRF_CONFIG, read_register(NRF_CONFIG) & ~_BV(PWR_UP));
}

/****************************************************************************/

//Power up now. Radio will not power down unless instructed by MCU for config changes etc.
void JM_RF24_STM32::powerUp(void)
{
    uint8_t cfg = read_register(NRF_CONFIG);

    // if not powered up then power up and wait for the radio to initialize
    if (!(cfg & _BV(PWR_UP))) {
        write_register(NRF_CONFIG, cfg | _BV(PWR_UP));

        // For nRF24L01+ to go from power down mode to TX or RX mode it must first pass through stand-by mode.
        // There must be a delay of Tpd2stby (see Table 16.) after the nRF24L01+ leaves power down mode before
        // the CEis set high. - Tpd2stby can be up to 5ms per the 1.0 datasheet
        delay(5);
    }
}

/******************************************************************/
#if defined(FAILURE_HANDLING) || defined(RF24_LINUX)

void JM_RF24_STM32::errNotify()
{
    #if defined(SERIAL_DEBUG) || defined(RF24_LINUX)
    printf_P(PSTR("RF24 HARDWARE FAIL: Radio not responding, verify pin connections, wiring, etc.\r\n"));
    #endif
    #if defined(FAILURE_HANDLING)
    failureDetected = 1;
    #else
    delay(5000);
    #endif
}

#endif
/******************************************************************/

//Similar to the previous write, clears the interrupt flags
bool JM_RF24_STM32::write(const void* buf, uint8_t len, const bool multicast)
{
    //Start Writing
    startFastWrite(buf, len, multicast);

    //Wait until complete or failed
    #if defined(FAILURE_HANDLING) || defined(RF24_LINUX)
    uint32_t timer = millis();
    #endif // defined(FAILURE_HANDLING) || defined(RF24_LINUX)

    while (!(get_status() & (_BV(TX_DS) | _BV(MAX_RT)))) {
        #if defined(FAILURE_HANDLING) || defined(RF24_LINUX)
        if (millis() - timer > 95) {
            errNotify();
            #if defined(FAILURE_HANDLING)
            return 0;
            #else
            delay(100);
            #endif
        }
        #endif
    }

    ce(LOW);

    uint8_t status = write_register(NRF_STATUS, _BV(RX_DR) | _BV(TX_DS) | _BV(MAX_RT));

    //Max retries exceeded
    if (status & _BV(MAX_RT)) {
        flush_tx(); //Only going to be 1 packet int the FIFO at a time using this method, so just flush
        return 0;
    }
    //TX OK 1 or 0
    return 1;
}

bool JM_RF24_STM32::write(const void* buf, uint8_t len)
{
    return write(buf, len, 0);
}
/****************************************************************************/

//For general use, the interrupt flags are not important to clear
bool JM_RF24_STM32::writeBlocking(const void* buf, uint8_t len, uint32_t timeout)
{
    //Block until the FIFO is NOT full.
    //Keep track of the MAX retries and set auto-retry if seeing failures
    //This way the FIFO will fill up and allow blocking until packets go through
    //The radio will auto-clear everything in the FIFO as long as CE remains high

    uint32_t timer = millis();                              //Get the time that the payload transmission started

    while ((get_status()
            & (_BV(TX_FULL)))) {          //Blocking only if FIFO is full. This will loop and block until TX is successful or timeout

        if (get_status() & _BV(MAX_RT)) {                      //If MAX Retries have been reached
            reUseTX();                                          //Set re-transmit and clear the MAX_RT interrupt flag
            if (millis() - timer > timeout) {
                return 0;
            }          //If this payload has exceeded the user-defined timeout, exit and return 0
        }
        #if defined(FAILURE_HANDLING) || defined(RF24_LINUX)
        if (millis() - timer > (timeout + 95)) {
            errNotify();
            #if defined(FAILURE_HANDLING)
            return 0;
            #endif
        }
        #endif

    }

    //Start Writing
    startFastWrite(buf, len, 0);                                  //Write the payload if a buffer is clear

    return 1;                                                  //Return 1 to indicate successful transmission
}

/****************************************************************************/

void JM_RF24_STM32::reUseTX()
{
    write_register(NRF_STATUS, _BV(MAX_RT));              //Clear max retry flag
    spiTrans(REUSE_TX_PL);
    ce(LOW);                                          //Re-Transfer packet
    ce(HIGH);
}

/****************************************************************************/

bool JM_RF24_STM32::writeFast(const void* buf, uint8_t len, const bool multicast)
{
    //Block until the FIFO is NOT full.
    //Keep track of the MAX retries and set auto-retry if seeing failures
    //Return 0 so the user can control the retrys and set a timer or failure counter if required
    //The radio will auto-clear everything in the FIFO as long as CE remains high

    #if defined(FAILURE_HANDLING) || defined(RF24_LINUX)
    uint32_t timer = millis();
    #endif

    //Blocking only if FIFO is full. This will loop and block until TX is successful or fail
    while ((get_status() & (_BV(TX_FULL)))) {
        if (get_status() & _BV(MAX_RT)) {
            //reUseTX();                                 //Set re-transmit
            write_register(NRF_STATUS, _BV(MAX_RT));     //Clear max retry flag
            return 0;                                    //Return 0. The previous payload has been retransmitted
            // From the user perspective, if you get a 0, just keep trying to send the same payload
        }
        #if defined(FAILURE_HANDLING) || defined(RF24_LINUX)
        if (millis() - timer > 95) {
            errNotify();
            #if defined(FAILURE_HANDLING)
            return 0;
            #endif // defined(FAILURE_HANDLING)
        }
        #endif
    }
    //Start Writing
    startFastWrite(buf, len, multicast);

    return 1;
}

bool JM_RF24_STM32::writeFast(const void* buf, uint8_t len)
{
    return writeFast(buf, len, 0);
}

/****************************************************************************/

//Per the documentation, we want to set PTX Mode when not listening. Then all we do is write data and set CE high
//In this mode, if we can keep the FIFO buffers loaded, packets will transmit immediately (no 130us delay)
//Otherwise we enter Standby-II mode, which is still faster than standby mode
//Also, we remove the need to keep writing the config register over and over and delaying for 150 us each time if sending a stream of data

void JM_RF24_STM32::startFastWrite(const void* buf, uint8_t len, const bool multicast, bool startTx)
{ //TMRh20

    //write_payload( buf,len);
    write_payload(buf, len, multicast ? W_TX_PAYLOAD_NO_ACK : W_TX_PAYLOAD);
    if (startTx) {
        ce(HIGH);
    }

}

/****************************************************************************/

//Added the original startWrite back in so users can still use interrupts, ack payloads, etc
//Allows the library to pass all tests
void JM_RF24_STM32::startWrite(const void* buf, uint8_t len, const bool multicast)
{

    // Send the payload

    //write_payload( buf, len );
    write_payload(buf, len, multicast ? W_TX_PAYLOAD_NO_ACK : W_TX_PAYLOAD);
    ce(HIGH);
    #if !defined(F_CPU) || F_CPU > 20000000
    delayMicroseconds(10);
    #endif
    ce(LOW);
}

/****************************************************************************/

bool JM_RF24_STM32::rxFifoFull()
{
    return read_register(FIFO_STATUS) & _BV(RX_FULL);
}

/****************************************************************************/

bool JM_RF24_STM32::txStandBy()
{

    #if defined(FAILURE_HANDLING) || defined(RF24_LINUX)
    uint32_t timeout = millis();
    #endif
    while (!(read_register(FIFO_STATUS) & _BV(TX_EMPTY))) {
        if (get_status() & _BV(MAX_RT)) {
            write_register(NRF_STATUS, _BV(MAX_RT));
            ce(LOW);
            flush_tx();    //Non blocking, flush the data
            return 0;
        }
        #if defined(FAILURE_HANDLING) || defined(RF24_LINUX)
        if (millis() - timeout > 95) {
            errNotify();
            #if defined(FAILURE_HANDLING)
            return 0;
            #endif
        }
        #endif
    }

    ce(LOW);               //Set STANDBY-I mode
    return 1;
}

/****************************************************************************/

bool JM_RF24_STM32::txStandBy(uint32_t timeout, bool startTx)
{

    if (startTx) {
        stopListening();
        ce(HIGH);
    }
    uint32_t start = millis();

    while (!(read_register(FIFO_STATUS) & _BV(TX_EMPTY))) {
        if (get_status() & _BV(MAX_RT)) {
            write_register(NRF_STATUS, _BV(MAX_RT));
            ce(LOW); // Set re-transmit
            ce(HIGH);
            if (millis() - start >= timeout) {
                ce(LOW);
                flush_tx();
                return 0;
            }
        }
        #if defined(FAILURE_HANDLING) || defined(RF24_LINUX)
        if (millis() - start > (timeout + 95)) {
            errNotify();
            #if defined(FAILURE_HANDLING)
            return 0;
            #endif
        }
        #endif
    }

    ce(LOW);  //Set STANDBY-I mode
    return 1;

}

/****************************************************************************/

void JM_RF24_STM32::maskIRQ(bool tx, bool fail, bool rx)
{

    uint8_t config = read_register(NRF_CONFIG);
    /* clear the interrupt flags */
    config &= ~(1 << MASK_MAX_RT | 1 << MASK_TX_DS | 1 << MASK_RX_DR);
    /* set the specified interrupt flags */
    config |= fail << MASK_MAX_RT | tx << MASK_TX_DS | rx << MASK_RX_DR;
    write_register(NRF_CONFIG, config);
}

/****************************************************************************/

uint8_t JM_RF24_STM32::getDynamicPayloadSize(void)
{
    uint8_t result = 0;

    #if defined(RF24_LINUX)
    spi_txbuff[0] = R_RX_PL_WID;
    spi_txbuff[1] = 0xff;
    beginTransaction();
    _spi->transfernb( (char *) spi_txbuff, (char *) spi_rxbuff, 2);
    result = spi_rxbuff[1];
    endTransaction();
    #else
    beginTransaction();
    _spi->transfer(R_RX_PL_WID);
    result = _spi->transfer(0xff);
    endTransaction();
    #endif

    if (result > 32) {
        flush_rx();
        delay(2);
        return 0;
    }
    return result;
}

/****************************************************************************/

bool JM_RF24_STM32::available(void)
{
    return available(NULL);
}

/****************************************************************************/

bool JM_RF24_STM32::available(uint8_t* pipe_num)
{
    if (!(read_register(FIFO_STATUS) & _BV(RX_EMPTY))) {

        // If the caller wants the pipe number, include that
        if (pipe_num) {
            uint8_t status = get_status();
            *pipe_num = (status >> RX_P_NO) & 0x07;
        }
        return 1;
    }

    return 0;


}

/****************************************************************************/

void JM_RF24_STM32::read(void* buf, uint8_t len)
{

    // Fetch the payload
    read_payload(buf, len);

    //Clear the two possible interrupt flags with one command
    write_register(NRF_STATUS, _BV(RX_DR) | _BV(MAX_RT) | _BV(TX_DS));

}

/****************************************************************************/

void JM_RF24_STM32::whatHappened(bool& tx_ok, bool& tx_fail, bool& rx_ready)
{
    // Read the status & reset the status in one easy call
    // Or is that such a good idea?
    uint8_t status = write_register(NRF_STATUS, _BV(RX_DR) | _BV(TX_DS) | _BV(MAX_RT));

    // Report to the user what happened
    tx_ok = status & _BV(TX_DS);
    tx_fail = status & _BV(MAX_RT);
    rx_ready = status & _BV(RX_DR);
}

/****************************************************************************/

void JM_RF24_STM32::openWritingPipe(uint64_t value)
{
    // Note that AVR 8-bit uC's store this LSB first, and the NRF24L01(+)
    // expects it LSB first too, so we're good.

    write_register(RX_ADDR_P0, reinterpret_cast<uint8_t*>(&value), addr_width);
    write_register(TX_ADDR, reinterpret_cast<uint8_t*>(&value), addr_width);


    //const uint8_t max_payload_size = 32;
    //write_register(RX_PW_P0,rf24_min(payload_size,max_payload_size));
    write_register(RX_PW_P0, payload_size);
}

/****************************************************************************/
void JM_RF24_STM32::openWritingPipe(const uint8_t* address)
{
    // Note that AVR 8-bit uC's store this LSB first, and the NRF24L01(+)
    // expects it LSB first too, so we're good.
    write_register(RX_ADDR_P0, address, addr_width);
    write_register(TX_ADDR, address, addr_width);

    //const uint8_t max_payload_size = 32;
    //write_register(RX_PW_P0,rf24_min(payload_size,max_payload_size));
    write_register(RX_PW_P0, payload_size);
}

/****************************************************************************/
static const uint8_t child_pipe[]
PROGMEM = {RX_ADDR_P0, RX_ADDR_P1, RX_ADDR_P2, RX_ADDR_P3, RX_ADDR_P4, RX_ADDR_P5};
static const uint8_t child_payload_size[]
PROGMEM = {RX_PW_P0, RX_PW_P1, RX_PW_P2, RX_PW_P3, RX_PW_P4, RX_PW_P5};

void JM_RF24_STM32::openReadingPipe(uint8_t child, uint64_t address)
{
    // If this is pipe 0, cache the address.  This is needed because
    // openWritingPipe() will overwrite the pipe 0 address, so
    // startListening() will have to restore it.
    if (child == 0) {
        memcpy(pipe0_reading_address, &address, addr_width);
    }

    if (child <= 6) {
        // For pipes 2-5, only write the LSB
        if (child < 2) {
            write_register(pgm_read_byte(&child_pipe[child]), reinterpret_cast<const uint8_t*>(&address), addr_width);
        } else {
            write_register(pgm_read_byte(&child_pipe[child]), reinterpret_cast<const uint8_t*>(&address), 1);
        }

        write_register(pgm_read_byte(&child_payload_size[child]), payload_size);

        // Note it would be more efficient to set all of the bits for all open
        // pipes at once.  However, I thought it would make the calling code
        // more simple to do it this way.
        write_register(EN_RXADDR, read_register(EN_RXADDR) | _BV(pgm_read_byte(&child_pipe_enable[child])));
    }
}

/****************************************************************************/
void JM_RF24_STM32::setAddressWidth(uint8_t a_width)
{

    if (a_width -= 2) {
        write_register(SETUP_AW, a_width % 4);
        addr_width = (a_width % 4) + 2;
    } else {
        write_register(SETUP_AW, 0);
        addr_width = 2;
    }

}

/****************************************************************************/

void JM_RF24_STM32::openReadingPipe(uint8_t child, const uint8_t* address)
{
    // If this is pipe 0, cache the address.  This is needed because
    // openWritingPipe() will overwrite the pipe 0 address, so
    // startListening() will have to restore it.
    if (child == 0) {
        memcpy(pipe0_reading_address, address, addr_width);
    }
    if (child <= 6) {
        // For pipes 2-5, only write the LSB
        if (child < 2) {
            write_register(pgm_read_byte(&child_pipe[child]), address, addr_width);
        } else {
            write_register(pgm_read_byte(&child_pipe[child]), address, 1);
        }
        write_register(pgm_read_byte(&child_payload_size[child]), payload_size);

        // Note it would be more efficient to set all of the bits for all open
        // pipes at once.  However, I thought it would make the calling code
        // more simple to do it this way.
        write_register(EN_RXADDR, read_register(EN_RXADDR) | _BV(pgm_read_byte(&child_pipe_enable[child])));

    }
}

/****************************************************************************/

void JM_RF24_STM32::closeReadingPipe(uint8_t pipe)
{
    write_register(EN_RXADDR, read_register(EN_RXADDR) & ~_BV(pgm_read_byte(&child_pipe_enable[pipe])));
}

/****************************************************************************/

void JM_RF24_STM32::toggle_features(void)
{
    beginTransaction();
    _spi->transfer(ACTIVATE);
    _spi->transfer(0x73);
    endTransaction();
}

/****************************************************************************/

void JM_RF24_STM32::enableDynamicPayloads(void)
{
    // Enable dynamic payload throughout the system

    //toggle_features();
    write_register(FEATURE, read_register(FEATURE) | _BV(EN_DPL));

    IF_SERIAL_DEBUG(printf("FEATURE=%i\r\n", read_register(FEATURE)));

    // Enable dynamic payload on all pipes
    //
    // Not sure the use case of only having dynamic payload on certain
    // pipes, so the library does not support it.
    write_register(DYNPD, read_register(DYNPD) | _BV(DPL_P5) | _BV(DPL_P4) | _BV(DPL_P3) | _BV(DPL_P2) | _BV(DPL_P1) | _BV(DPL_P0));

    dynamic_payloads_enabled = true;
}

/****************************************************************************/
void JM_RF24_STM32::disableDynamicPayloads(void)
{
    // Disables dynamic payload throughout the system.  Also disables Ack Payloads

    //toggle_features();
    write_register(FEATURE, 0);

    IF_SERIAL_DEBUG(printf("FEATURE=%i\r\n", read_register(FEATURE)));

    // Disable dynamic payload on all pipes
    //
    // Not sure the use case of only having dynamic payload on certain
    // pipes, so the library does not support it.
    write_register(DYNPD, 0);

    dynamic_payloads_enabled = false;
}

/****************************************************************************/

void JM_RF24_STM32::enableAckPayload(void)
{
    //
    // enable ack payload and dynamic payload features
    //

    //toggle_features();
    write_register(FEATURE, read_register(FEATURE) | _BV(EN_ACK_PAY) | _BV(EN_DPL));

    IF_SERIAL_DEBUG(printf("FEATURE=%i\r\n", read_register(FEATURE)));

    //
    // Enable dynamic payload on pipes 0 & 1
    //
    write_register(DYNPD, read_register(DYNPD) | _BV(DPL_P1) | _BV(DPL_P0));
    dynamic_payloads_enabled = true;
}

/****************************************************************************/

void JM_RF24_STM32::enableDynamicAck(void)
{
    //
    // enable dynamic ack features
    //
    //toggle_features();
    write_register(FEATURE, read_register(FEATURE) | _BV(EN_DYN_ACK));

    IF_SERIAL_DEBUG(printf("FEATURE=%i\r\n", read_register(FEATURE)));


}

/****************************************************************************/

void JM_RF24_STM32::writeAckPayload(uint8_t pipe, const void* buf, uint8_t len)
{
    const uint8_t* current = reinterpret_cast<const uint8_t*>(buf);

    uint8_t data_len = rf24_min(len, 32);

    #if defined(RF24_LINUX)
    beginTransaction();
    uint8_t * ptx = spi_txbuff;
    uint8_t size = data_len + 1 ; // Add register value to transmit buffer
    *ptx++ =  W_ACK_PAYLOAD | ( pipe & 0x07 );
    while ( data_len-- ){
      *ptx++ =  *current++;
    }

    _spi->transfern( (char *) spi_txbuff, size);
    endTransaction();
    #else
    beginTransaction();
    _spi->transfer(W_ACK_PAYLOAD | (pipe & 0x07));

    while (data_len--) {
        _spi->transfer(*current++);
    }
    endTransaction();

    #endif

}

/****************************************************************************/

bool JM_RF24_STM32::isAckPayloadAvailable(void)
{
    return !(read_register(FIFO_STATUS) & _BV(RX_EMPTY));
}

/****************************************************************************/

bool JM_RF24_STM32::isPVariant(void)
{
    return p_variant;
}

/****************************************************************************/

void JM_RF24_STM32::setAutoAck(bool enable)
{
    if (enable) {
        write_register(EN_AA, 0x3F);
    } else {
        write_register(EN_AA, 0);
    }
}

/****************************************************************************/

void JM_RF24_STM32::setAutoAck(uint8_t pipe, bool enable)
{
    if (pipe <= 6) {
        uint8_t en_aa = read_register(EN_AA);
        if (enable) {
            en_aa |= _BV(pipe);
        } else {
            en_aa &= ~_BV(pipe);
        }
        write_register(EN_AA, en_aa);
    }
}

/****************************************************************************/

bool JM_RF24_STM32::testCarrier(void)
{
    return (read_register(CD) & 1);
}

/****************************************************************************/

bool JM_RF24_STM32::testRPD(void)
{
    return (read_register(RPD) & 1);
}

/****************************************************************************/

void JM_RF24_STM32::setPALevel(uint8_t level)
{

    uint8_t setup = read_register(RF_SETUP) & 0xF8;

    if (level > 3) {                        // If invalid level, go to max PA
        level = (RF24_PA_MAX << 1) + 1;        // +1 to support the SI24R1 chip extra bit
    } else {
        level = (level << 1) + 1;            // Else set level as requested
    }

    write_register(RF_SETUP, setup |= level);    // Write it to the chip
}

/****************************************************************************/

uint8_t JM_RF24_STM32::getPALevel(void)
{

    return (read_register(RF_SETUP) & (_BV(RF_PWR_LOW) | _BV(RF_PWR_HIGH))) >> 1;
}

/****************************************************************************/

uint8_t JM_RF24_STM32::getARC(void)
{

    return read_register(OBSERVE_TX) & 0x0F;
}

/****************************************************************************/

bool JM_RF24_STM32::setDataRate(rf24_datarate_e speed)
{
    bool result = false;
    uint8_t setup = read_register(RF_SETUP);

    // HIGH and LOW '00' is 1Mbs - our default
    setup &= ~(_BV(RF_DR_LOW) | _BV(RF_DR_HIGH));

    #if !defined(F_CPU) || F_CPU > 20000000
    txDelay = 250;
    #else //16Mhz Arduino
    txDelay=85;
    #endif
    if (speed == RF24_250KBPS) {
        // Must set the RF_DR_LOW to 1; RF_DR_HIGH (used to be RF_DR) is already 0
        // Making it '10'.
        setup |= _BV(RF_DR_LOW);
        #if !defined(F_CPU) || F_CPU > 20000000
        txDelay = 450;
        #else //16Mhz Arduino
        txDelay = 155;
        #endif
    } else {
        // Set 2Mbs, RF_DR (RF_DR_HIGH) is set 1
        // Making it '01'
        if (speed == RF24_2MBPS) {
            setup |= _BV(RF_DR_HIGH);
            #if !defined(F_CPU) || F_CPU > 20000000
            txDelay = 190;
            #else // 16Mhz Arduino
            txDelay = 65;
            #endif
        }
    }
    write_register(RF_SETUP, setup);

    // Verify our result
    if (read_register(RF_SETUP) == setup) {
        result = true;
    }
    return result;
}

/****************************************************************************/

rf24_datarate_e JM_RF24_STM32::getDataRate(void)
{
    rf24_datarate_e result;
    uint8_t dr = read_register(RF_SETUP) & (_BV(RF_DR_LOW) | _BV(RF_DR_HIGH));

    // switch uses RAM (evil!)
    // Order matters in our case below
    if (dr == _BV(RF_DR_LOW)) {
        // '10' = 250KBPS
        result = RF24_250KBPS;
    } else if (dr == _BV(RF_DR_HIGH)) {
        // '01' = 2MBPS
        result = RF24_2MBPS;
    } else {
        // '00' = 1MBPS
        result = RF24_1MBPS;
    }
    return result;
}

/****************************************************************************/

void JM_RF24_STM32::setCRCLength(rf24_crclength_e length)
{
    uint8_t config = read_register(NRF_CONFIG) & ~(_BV(CRCO) | _BV(EN_CRC));

    // switch uses RAM (evil!)
    if (length == RF24_CRC_DISABLED) {
        // Do nothing, we turned it off above.
    } else if (length == RF24_CRC_8) {
        config |= _BV(EN_CRC);
    } else {
        config |= _BV(EN_CRC);
        config |= _BV(CRCO);
    }
    write_register(NRF_CONFIG, config);
}

/****************************************************************************/

rf24_crclength_e JM_RF24_STM32::getCRCLength(void)
{
    rf24_crclength_e result = RF24_CRC_DISABLED;

    uint8_t config = read_register(NRF_CONFIG) & (_BV(CRCO) | _BV(EN_CRC));
    uint8_t AA = read_register(EN_AA);

    if (config & _BV(EN_CRC) || AA) {
        if (config & _BV(CRCO)) {
            result = RF24_CRC_16;
        } else {
            result = RF24_CRC_8;
        }
    }

    return result;
}

/****************************************************************************/

void JM_RF24_STM32::disableCRC(void)
{
    uint8_t disable = read_register(NRF_CONFIG) & ~_BV(EN_CRC);
    write_register(NRF_CONFIG, disable);
}

/****************************************************************************/
void JM_RF24_STM32::setRetries(uint8_t delay, uint8_t count)
{
    write_register(SETUP_RETR, (delay & 0xf) << ARD | (count & 0xf) << ARC);
}