#include "../include/AD-DA-WS-RPI.h"

void bsp_DelayUS(uint64_t micros)
{
    bcm2835_delayMicroseconds(micros);
}

int spi_begin()
{
    return bcm2835_spi_begin();
}

int spi_init()
{
    return bcm2835_init();
}

bool DRDYIsLow(void)
{
    return DRDY_IS_LOW();
}

bool DRDY1IsLow(void)
{
    return DRDY1_IS_LOW();
}

int spi_init_adc_dac_board()
{
    // SPI params
    bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST); // Since bcm2835 V1.56
    bcm2835_spi_setDataMode(BCM2835_SPI_MODE1);              // The default
    bcm2835_spi_setClockDivider(128);                        // The default 256
    // ADS1256
    bcm2835_gpio_fsel(SPI_CS_ADC1256, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(SPI_CS_ADC1, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_write(SPI_CS_ADC1256, HIGH);
    bcm2835_gpio_write(SPI_CS_ADC1, HIGH);
    bcm2835_gpio_fsel(DRDY, BCM2835_GPIO_FSEL_INPT);
    bcm2835_gpio_fsel(DRDY1, BCM2835_GPIO_FSEL_INPT);
    bcm2835_gpio_set_pud(DRDY, BCM2835_GPIO_PUD_UP);
    bcm2835_gpio_set_pud(DRDY1, BCM2835_GPIO_PUD_UP);
    bcm2835_gpio_set_pad(BCM2835_PAD_GROUP_GPIO_0_27, BCM2835_PAD_SLEW_RATE_UNLIMITED | BCM2835_PAD_DRIVE_16mA);
    return 1;
}
/******************************************************************************
function:   Waits for ADC conversion to end
******************************************************************************/
int WaitCondition(bool (*f)(void))
{
    int i;
    for (i = 0; i < 410002; i++) // TODO find a better way. E.g. with time.h clock_gettime as you know you shall timeout if time is over CYCLING_TROUGHPUT_USEC
    {
        if ((*f)())
            return 0;
        bsp_DelayUS(1); // to increase speed
    }

    return -1;
}

int ADS1256_WaitDRDY_LOW(void) // first ADC DRDY (CS0)
{
    return WaitCondition(DRDYIsLow);
}

int ADC_WaidDRDY_LOW(void) // second ADC DRDY (CS1)
{
    return WaitCondition(DRDY1IsLow);
}
/*******************************************************************************/
uint8_t ADS1256_Receive8Bit(void)
{
    return bcm2835_spi_transfer(0xff);
}

void ADS1256_Send8Bit(uint8_t _data)
{
    bsp_DelayUS(2);
    bcm2835_spi_transfer(_data);
}
/******************************************************************************
function:   send command
parameter:
        Cmd: command
Info:
******************************************************************************/
/*void ADS1256_WriteCmd(uint8_t _cmd)
{
    CS_0();
    ADS1256_Send8Bit(_cmd);
    CS_1();
}*/

/******************************************************************************
function:   Write a data to the destination register
parameter:
        Reg : Target register
        data: Written data
Info:
******************************************************************************/
void ADS1256_WriteReg(uint8_t _RegID, uint8_t _RegValue)
{
    CS_0();                              /* SPI  cs  = 0 */
    ADS1256_Send8Bit(CMD_WREG | _RegID); /*Write command register */
    ADS1256_Send8Bit(0x00);              /*Write the register number */
    ADS1256_Send8Bit(_RegValue);         /*send register value */
    CS_1();                              /* SPI   cs = 1 */
}

void ADC1_WriteReg(uint8_t _RegID, uint8_t _RegValue) // second ADC
{
    CS1_0();                             /* SPI  cs  = 0 */
    ADS1256_Send8Bit(CMD_WREG | _RegID); /*Write command register */
    ADS1256_Send8Bit(0x00);              /*Write the register number */
    ADS1256_Send8Bit(_RegValue);         /*send register value */
    CS1_1();                             /* SPI   cs = 1 */
}
/******************************************************************************
function:  Set the channel to be read
parameter:
    Channal : Set channel number
Info:
******************************************************************************/
void ADS1256_SetChannel(uint8_t channel)
{
    ADS1256_WriteReg(REG_MUX, (channel << 4) | (1 << 3));
    ADC1_WriteReg(REG_MUX, (channel << 4) | (1 << 3));
}

uint8_t ADS1256_ReadReg(uint8_t _RegID)
{
    uint8_t read;
    CS_0();                              /* SPI  cs  = 0 */
    ADS1256_Send8Bit(CMD_RREG | _RegID); /* Write command register */
    ADS1256_Send8Bit(0x00);              /* Write the register number */
    bsp_DelayUS(7);                      // ADS1256_DelayDATA(); /*delay time */
    read = ADS1256_Receive8Bit();        /* Read the register values */
    CS_1();                              /* SPI   cs  = 1 */
    return read;
}
/******************************************************************************
function:   Read a data from the destination register
parameter:
        Reg : Target register
Info:
    Return the read data
******************************************************************************/
uint8_t ADS1_ReadReg(uint8_t _RegID)
{
    uint8_t read;
    CS1_0();                             /* SPI  cs  = 0 */
    ADS1256_Send8Bit(CMD_RREG | _RegID); /* Write command register */
    ADS1256_Send8Bit(0x00);              /* Write the register number */
    bsp_DelayUS(7);                      // ADS1256_DelayDATA(); /*delay time */
    read = ADS1256_Receive8Bit();        /* Read the register values */
    CS1_1();                             /* SPI   cs  = 1 */
    return read;
}

uint8_t ADS1256_ReadChipID(void)
{
    uint8_t id;
    id = ADS1256_ReadReg(REG_STATUS);
    return (id >> 4);
}
/******************************************************************************
function:  Configure ADC gain and sampling speed
parameter:
    gain : Enumeration type gain
    drate: Enumeration type sampling speed
Info:
******************************************************************************/
int ADS1256_ConfigureADC(ADS1256_GAIN_E _gain, ADS1256_DRATE_E _drate)
{
    int w = ADS1256_WaitDRDY_LOW();
    if (w != 0)
    {
        printf("In ADS1256_ConfigureADC, ADS1256_WaitDRDY_LOW returned %d\r\n", w);
        return -1;
    }
    {
        uint8_t buf[4]; /* Storage ads1256 register configuration parameters */
        // buf[0] = (0 << 3) | (1 << 2) | (1 << 1);//enable the internal buffer
        buf[0] = (0 << 3) | (1 << 2) | (0 << 1); // The internal buffer is prohibited
        // ADS1256_WriteReg(REG_STATUS, (0 << 3) | (1 << 2) | (1 << 1));
        buf[1] = 0x08;
        buf[2] = (0 << 5) | (0 << 3) | ((uint8_t)_gain << 0);
        // ADS1256_WriteReg(REG_ADCON, (0 << 5) | (0 << 2) | (GAIN_1 << 1));	/*choose 1: gain 1 ;input 5V/
        buf[3] = s_tabDataRate[_drate]; // e.g. DRATE_10SPS;
        CS_0();                         /* SPIÆ¬Ñ¡ = 0 */
        ADS1256_Send8Bit(CMD_WREG | 0); /* Write command register, send the register address */
        ADS1256_Send8Bit(0x03);         /* Register number 4,Initialize the number  -1*/
        ADS1256_Send8Bit(buf[0]);       /* Set the status register */
        ADS1256_Send8Bit(buf[1]);       /* Set the input channel parameters */
        ADS1256_Send8Bit(buf[2]);       /* Set the ADCON control register,gain */
        ADS1256_Send8Bit(buf[3]);       /* Set the output rate */
        CS_1();                         /* SPI  cs = 1 */
        CS1_0();                        /* SPIÆ¬Ñ¡ = 0 */
        ADS1256_Send8Bit(CMD_WREG | 0); /* Write command register, send the register address */
        ADS1256_Send8Bit(0x03);         /* Register number 4,Initialize the number  -1*/
        ADS1256_Send8Bit(buf[0]);       /* Set the status register */
        ADS1256_Send8Bit(buf[1]);       /* Set the input channel parameters */
        ADS1256_Send8Bit(buf[2]);       /* Set the ADCON control register,gain */
        ADS1256_Send8Bit(buf[3]);       /* Set the output rate */
        CS1_1();                        /* SPI  cs = 1 */
    }
    bsp_DelayUS(50);
    return 0;
}

/******************************************************************************
function:  Device initialization
parameter:
    gain : Enumeration type gain
    drate: Enumeration type sampling speed
Info:
******************************************************************************/
int ADC_Init(ADS1256_GAIN_E aGain, ADS1256_DRATE_E aDrate)
{
    int initBcm = spi_init();
    if (initBcm != 1)
    {
        printf("In ADS1256_Init, spi_init returned %d\r\n", initBcm);
        return -1;
    }
    int spiBegin = spi_begin();
    if (spiBegin != 1)
    {
        printf("In ADS1256_Init, spi_begin returned %d\r\n", spiBegin);
        return -2;
    }
    int spiPrepare = spi_init_adc_dac_board();
    if (spiPrepare != 1)
    {
        printf("In ADS1256_Init, spi_init_adc_dac_board returned %d\r\n", spiBegin);
        return -4;
    }
    // ADS1256_ReadChipID();
    /*if (*id != 3)
    {
        printf("In ADS1256_Init, Error, ASD1256 Chip ID = 0x%d which is != 3\r\n", (int)(*id));
        return -3;
    }*/
    ADS1256_ConfigureADC(aGain, aDrate);
    return 0;
}

int ADC_DAC_Close(void)
{
    bsp_DelayUS(25);
    bcm2835_spi_end();
    int closingBcm = bcm2835_close();
    if (closingBcm != 1)
    {
        printf("In ADS1256_Close, spi_close returned %d\r\n", closingBcm);
        return -1;
    }
    return 0;
}
/******************************************************************************
function:  Read ADC data
parameter:
Info:
******************************************************************************/
int32_t ADC_ReadData(void)
{
    uint32_t read = 0;
    static uint8_t buf[3];
    CS_0(); /* SPI   cs = 0 */
    // bsp_DelayUS(1);
    // ADS1256_Send8Bit(CMD_RDATA); /* read ADC command  */
    // bsp_DelayUS(20);
    ADS1256_WaitDRDY_LOW();
    /*Read the sample results 24bit*/
    buf[0] = ADS1256_Receive8Bit();
    buf[1] = ADS1256_Receive8Bit();
    buf[2] = ADS1256_Receive8Bit();
    read = ((uint32_t)buf[0] << 16) & 0x00FF0000;
    read |= ((uint32_t)buf[1] << 8); /* Pay attention to It is wrong   read |= (buf[1] << 8) */
    read |= buf[2];
    CS_1();
    if (read & 0x800000)
    {
        read |= 0xFF000000;
    }
    return (int32_t)read;
}

int32_t ADC1_ReadData(void)
{
    uint32_t read = 0;
    static uint8_t buf[3];
    CS1_0(); /* SPI   cs = 0 */
    // bsp_DelayUS(1);
    // ADS1256_Send8Bit(CMD_RDATA); /* read ADC command  */
    // bsp_DelayUS(20);
    ADC_WaidDRDY_LOW();
    /*Read the sample results 24bit*/
    buf[0] = ADS1256_Receive8Bit();
    buf[1] = ADS1256_Receive8Bit();
    buf[2] = ADS1256_Receive8Bit();
    read = ((uint32_t)buf[0] << 16) & 0x00FF0000;
    read |= ((uint32_t)buf[1] << 8); /* Pay attention to It is wrong   read |= (buf[1] << 8) */
    read |= buf[2];
    CS1_1();
    if (read & 0x800000)
    {
        read |= 0xFF000000;
    }
    return (int32_t)read;
}