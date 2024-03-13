#include "../include/nilm.h"

/******************************************************************************
function:  main (reads ADC values)
parameter:
Info:
******************************************************************************/
int main(void)
{
	struct data_point arr_datapoint1[seconds2save * wSize]; // structure to store each data point time,voltage,current
	struct data_point arr_datapoint2[seconds2save * wSize];
	int Loop;
	int offset0 = 0, offset1 = 0;
	double sumP;
	float voltage, current, power, activePower;
	signal(SIGINT, sig_handler);
	signal(SIGTERM, sig_handler);

	setUsedCore(ioCPU);
	int initSpi = spi_init();
	if (initSpi != 1)
	{
		perror("Error SPI init failed: ");
	}
	int Init = ADC_Init(ADS1256_GAIN_1, ADS1256_7500SPS);
	if (Init != 0)
	{
		perror("Error ADC Init failed: ");
	}
	/******************************************************************************
	 * mutexes to lock threads
	 ******************************************************************************/
	pthread_mutex_lock(&lock1); // locks writer1
	pthread_mutex_lock(&lock2); // locks writer2
	pthread_mutex_lock(&lock3); // locks calcmetrics1
	pthread_mutex_lock(&lock4); // locks calcmetrics2
	/******************************************************************************
	 * threads creation (writers not need for v0.0 LE)
	 ******************************************************************************/
	pthread_create(&(tid[0]), NULL, calcMetrics1, (void *)arr_datapoint1);
	pthread_create(&(tid[1]), NULL, calcMetrics2, (void *)arr_datapoint2);
	pthread_create(&(tid[2]), NULL, writeFile1, (void *)arr_datapoint1);
	pthread_create(&(tid[3]), NULL, writeFile2, (void *)arr_datapoint2);
	/******************************************************************************
	 * set ADCs to read continuously
	 ******************************************************************************/
	CS_0(); /* SPI   cs = 0 */
	bsp_DelayUS(1);
	ADS1256_Send8Bit(CMD_RDATAC); /* read ADC command  */
	bsp_DelayUS(20);
	CS_1();
	CS1_0(); /* SPI   cs = 0 */
	bsp_DelayUS(1);
	ADS1256_Send8Bit(CMD_RDATAC); /* read ADC command  */
	bsp_DelayUS(20);
	CS1_1();
	int sampleSet = seconds2save * wSize;
	/******************************************************************************
	Info: Read ADCs, to loops to avoid concurrency on access to arr_datapoin* by
	main and writer threads
	******************************************************************************/
	getMacAddr();
	printf("Device ID %s\n", mac);
	// offset calibration over a 10 second record
	// (does not require the constant multiplication TODO remove this was for evaluating the offset "real" value)
	for (Loop = 0; Loop < sampleSet / 6; Loop++)
	{
		arr_datapoint1[Loop].voltage = (int32_t)ADC_ReadData();
		arr_datapoint1[Loop].current = (int32_t)ADC1_ReadData();
		voltage = (arr_datapoint1[Loop].voltage / pow(2, 23) - (0.5)) * 2 * sqrt(2) * voltRating * voltResComp;
		current = (arr_datapoint1[Loop].current / pow(2, 23) - (0.5)) * 2 * sqrt(2) * ctRating * ctResComp;
		offset0 = offset0 + voltage;
		offset1 = offset1 + current;
		power = voltage * current; // not squared here to be used for energy calculation
		sumP += power;			   // for average in the next loop
	}
	activePower = sumP / (float)(sampleSet / 6);
	if (activePower < 0)
		invertedCT = TRUE;
	else
		invertedCT = FALSE;
	offsetVoltage = offset0 / (float)(sampleSet / 6);
	offsetCurrent = offset1 / (float)(sampleSet / 6);
	// printf("offsetV %f e offsetI %f\n", offsetV, offsetI);
	printf("offsetV %f e offsetI %f\n", offsetVoltage, offsetCurrent);
	printf("active power %f e invertedCT %d\n", activePower, invertedCT);

	while (1)
	{
		for (Loop = 0; Loop < sampleSet; Loop++)
		{
			arr_datapoint1[Loop].voltage = (int32_t)ADC_ReadData();
			arr_datapoint1[Loop].current = (int32_t)ADC1_ReadData();
		}
		pthread_mutex_unlock(&lock3); // unlocks calcmetrics1

		for (Loop = 0; Loop < sampleSet; Loop++)
		{
			arr_datapoint2[Loop].voltage = (int32_t)ADC_ReadData();
			arr_datapoint2[Loop].current = (int32_t)ADC1_ReadData();
		}
		pthread_mutex_unlock(&lock4); // unlocks calcmetrics2
	}
	return 0;
}

void getMacAddr(void)
{
	int fdMAC;
	struct ifreq ifr;
	char *iface = "wlan0";
	fdMAC = socket(AF_INET, SOCK_DGRAM, 0);
	ifr.ifr_addr.sa_family = AF_INET;
	strncpy(ifr.ifr_name, iface, IFNAMSIZ - 1);
	ioctl(fdMAC, SIOCGIFHWADDR, &ifr);
	close(fdMAC);
	sprintf(mac, "%.2x%.2x%.2x%.2x%.2x%.2x",
			(unsigned char)ifr.ifr_hwaddr.sa_data[0],
			(unsigned char)ifr.ifr_hwaddr.sa_data[1],
			(unsigned char)ifr.ifr_hwaddr.sa_data[2],
			(unsigned char)ifr.ifr_hwaddr.sa_data[3],
			(unsigned char)ifr.ifr_hwaddr.sa_data[4],
			(unsigned char)ifr.ifr_hwaddr.sa_data[5]);
}

/******************************************************************************
function:  Handles the closure of the program
parameter:
Info:
******************************************************************************/
void sig_handler()
{
	int i;
	printf("Received signal to terminate\n");
	CS_0(); /* SPI   cs = 0 */
	ADS1256_WaitDRDY_LOW();
	ADS1256_Send8Bit(CMD_STANDBY);
	CS_1();
	printf("Issued a STDBY to ADC0\n");
	CS1_0(); /* SPI   cs = 0 */
	ADC_WaidDRDY_LOW();
	ADS1256_Send8Bit(CMD_STANDBY);
	CS1_1();
	printf("Issued a STDBY to ADC1\n");
	for (i = 0; i < nThreads; i++)
	{
		pthread_kill(tid[i], SIGTERM);
		pthread_detach(tid[i]);
	}
	i = ADC_DAC_Close();
	if (i < 0)
	{
		perror("Error Closing ADC: \n");
	}
	printf("Closed ADCs\n");
	sleep(3);
	printf("Successfully closed the program\n");
}