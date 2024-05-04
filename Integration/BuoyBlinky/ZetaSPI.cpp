/*
    Ported and modified by Guy Ringshaw 2024
    Class for interfacing with a Si4455 (rev C2) RF transceiver chip (Zeta 433) with an STM32 device with Arm Mbed
    Ported from arduino version: https://github.com/deeplyembeddedWP/EZRadio_SI4455-Library.git
*/

#include "ZetaSPI.h"
#include <cstdio>

#define ENABLE_POR

#define RADIO_CTS_TIMEOUT			10000
#define RESP_BYTES_SIZE				16

/* Static & Global Variables */
static unsigned char cts_flag;
static unsigned char radioCmd[RESP_BYTES_SIZE];
union si4455_cmd_reply_union Si4455Cmd;
const unsigned char Radio_Configuration_Data_Array[] = RADIO_CONFIGURATION_DATA_ARRAY;
const tRadioConfiguration RadioConfiguration = RADIO_CONFIGURATION_DATA;
static Si4455_T Ret_state;

//extern DigitalOut TriggerLine;

zetaspi::zetaspi(SPIConfig_t Pins, ZetaConfig_t Zpins): spidevice(Pins.MOSI, Pins.MISO, Pins.SCLK), CS(Pins.CS), SDN(Zpins.SDN)
{
    
}


/********************************************************************************************
 * Function Name		:	Wait_POR
 * Description			:	Gives delay for POR (Power on Reset) to occur 
 * Returns		        :	None
 * Params			:	None
 * Note				:	If you are using the SDN pin, then you can do a POR 
					by pulling the SDN pin high for 10us and then make 
					it low for 10ms for POR to complete.
                    ********************************************************
                    (GUY'S NOTE): Interestingly the programming guide says you ought
                    to wait for 14 ms to account for max POR + SPI timeout but let's
                    see where this goes I guess
 ********************************************************************************************/
void zetaspi::Wait_POR() 
{
	SDN = 1; //SDN HIGH
   
    wait_us(10);
    
	/* Pull the SDN pin low for 5-10 ms */
    SDN = 0; //SDN LOW
    ThisThread::sleep_for(5ms);
   
    //TriggerLine = 1; //triggerline for testing

}

/********************************************************************************************
 * Function Name		:	SpiWriteBytes
 * Description			:	Writes bytes on the SPI bus - MOSI
 * Returns		        :	None
 * Params			:	1. byteCount 	- No. of bytes to write
					2. pData	- Points to the bytes in the array 
 * *****************************************************************************************/
void zetaspi::SpiWriteBytes(unsigned char byteCount, const unsigned char* pData)
{
	const unsigned char* ptr = pData;

	for (int i = 0; i < byteCount; i++)
	{
		spidevice.write(*ptr++); //write data to SPI
	}
}

/********************************************************************************************
 * Function Name		:	SpiReadBytes
 * Description			:	Reads bytes from the SPI bus - MISO
 * Returns		        :	None
 * Params			:	1. byteCount 	- No. of bytes to read
					2. pData	- Points to the array/var to be 
							  filled
 * ******************************************************************************************/
void zetaspi::SpiReadBytes(unsigned char byteCount, unsigned char* pData)
{
	unsigned char* ptr = pData;

	for (int i = 0; i < byteCount; i++)
	{
		*ptr++ = spidevice.write(0xFF);
	}
}
/*********************************************************************************************
 * Function Name		:	ReadRX
 * Description			:	Reads directly from RX buffer
 * 					Si4455
 * Returns		        :	Nothing
 * Params			:	1. byteCount 	- No. of bytes to read and filled
					2. pData	- Points to the array/var to be 
							  filled 
                    ********************************************************
                    (GUY's NOTE): Read buffer doesn't work with CTS check, this skips that
 * *******************************************************************************************/

void zetaspi::ReadRX(unsigned char bytecount, unsigned char*pData)
{
    unsigned char* ptr = pData;
    
    CS = 0;
    spidevice.write(SI4455_CMD_ID_READ_RX_FIFO); //0x77 cmd ID

    for(int i = 0; i<bytecount; i++)
    {
        *ptr++ = spidevice.write(0xFF);
    }
    
    CS = 1;
}

/*********************************************************************************************
 * Function Name		:	GetResponse_CTS
 * Description			:	API to Poll CTS and also read an array of data from
 * 					Si4455
 * Returns		        :	0xFF on success and 0x01 on CTS time out
 * Params			:	1. byteCount 	- No. of bytes to read and filled
					2. pData	- Points to the array/var to be 
							  filled 
 * *******************************************************************************************/
unsigned char zetaspi::GetResponse_CTS(unsigned char byteCount, unsigned char* pData)
{
	cts_flag = false;
	unsigned char ctsVal = 0u;
	unsigned int errCnt = RADIO_CTS_TIMEOUT;

	while (errCnt != 0) //attempt based error detection                                                                                              
	{
		CS = 0; //bring CS low
		spidevice.write(0x44); //write CTS command                                                                                                   
		ctsVal = spidevice.write(0xFF); //dummy bits to maintain clock for the read (did we not just write the above function for this?)

		if (ctsVal == 0xFF)
		{
			if (byteCount)
			{
				SpiReadBytes(byteCount, pData);
			}
			CS = 1; //bring CS high
			break;
		}
		CS = 1; //bring CS high
		errCnt--;
	}

	if (errCnt == 0)
	{
		/* ERROR!!!!  CTS should never take this long. */
		printf("CTS Time Out \r\n");
		return 0x01;
	}
	if (ctsVal == 0xFF)
	{
		cts_flag = true;
	}

	return ctsVal;
}

/*******************************************************************************************
 * Function Name		:	PollCTS
 * Description			:	Poll for CTS
 * Returns		        :	0xFF on success and 0x01 on CTS time out
 
 * *****************************************************************************************/
unsigned char zetaspi::PollCTS()
{
	return GetResponse_CTS(0,0);
}

/*******************************************************************************************
 * Function Name		:	SendCmdArgs
 * Description			:	Write Command and Data to Si4455
 * Returns		        :	None
 * Params			:	1. cmd		- command
					2. pollcts	- check for ctsVal
					3. byteCount	- No. of bytes to write
					4. pData	- Points to the bytes in the array
 * *****************************************************************************************/
void zetaspi::SendCmdArgs(unsigned char cmd, unsigned char pollcts, 
		unsigned char bytecount, const unsigned char *pData)
{
	if (pollcts)
	{
		while (!cts_flag)
		{
			PollCTS();
		}
	}
	CS = 0; //CS low
	spidevice.write(cmd); //write command byte
	SpiWriteBytes(bytecount, pData); //write param bytes 
	CS = 1; //CS high
	cts_flag = false;
}

/*********************************************************************************************
 * Function Name		:	WriteEZConfigArray
 * Description			:	Write Configuration array to Si4455
 * Returns		        :	None
 * Params			:	1. numBytes		- No. of bytes to write
					2. pEzConfigArray	- points to the config array
 * ******************************************************************************************/
void zetaspi::WriteEZConfigArray(unsigned char numBytes, const 
		unsigned char* pEzConfigArray)
{
	SendCmdArgs(SI4455_CMD_ID_WRITE_TX_FIFO, 1, numBytes, pEzConfigArray);
}

/***********************************************************************************************
 * Function Name		:	SendCmds
 * Description			:	Send Commands to Si4455
 * Returns		        :	None
 * Params			:	1. ByteCount	-	No. of commands to send
					2. pData	-	Points to the bytes in the array
 * *********************************************************************************************/
void zetaspi::SendCmds(unsigned char ByteCount, unsigned char* pData)
{
	while (!cts_flag)
	{
		PollCTS();
	}
	CS = 0; //CS low
	SpiWriteBytes(ByteCount, pData); //write bytes
	CS = 1; //CS high
	cts_flag = false;
}

/**************************************************************************************************
 * Function Name		:	SendCmdGetResp
 * Description			:	Send commands and get response for Si4455
 * Returns		        :	None
 * Params			:	1. cmdByteCount		- No. of commands to send
					2. pCmdData		- Points to the bytes in the array
					3. respByteCount	- No. of bytes to read
					4. pRespData		- Points to the array to be filled 
 * ***********************************************************************************************/
unsigned char zetaspi::SendCmdGetResp(unsigned char cmdByteCount, unsigned char *pCmdData, 
		unsigned char respByteCount, unsigned char* pRespData)
{
	SendCmds(cmdByteCount, pCmdData);
	return GetResponse_CTS(respByteCount, pRespData);
}

/**************************************************************************************************
 * Function Name		:	GetIntStatus
 * Description			:	Get interrupt status
 * Returns		        :	None
 * Params			:	1. PH_CLR_PEND		- Command to clear pending 
							   	  interrupts	
					2. MODEM_CLR_PEND	- Command to clear modem 
							   	  interrupts
					3. CHIP_CLR_PEND	- Command to chip interrupts
 * ************************************************************************************************/
void zetaspi::GetIntStatus(unsigned char PH_CLR_PEND, unsigned char MODEM_CLR_PEND, 
		unsigned char CHIP_CLR_PEND)
{
	radioCmd[0] = SI4455_CMD_ID_GET_INT_STATUS;
	radioCmd[1] = PH_CLR_PEND;
	radioCmd[2] = MODEM_CLR_PEND;
	radioCmd[3] = CHIP_CLR_PEND;

	SendCmdGetResp(SI4455_CMD_ARG_COUNT_GET_INT_STATUS, radioCmd, SI4455_CMD_REPLY_COUNT_GET_INT_STATUS, radioCmd);

	Si4455Cmd.GET_INT_STATUS.INT_PEND       = radioCmd[0];
	Si4455Cmd.GET_INT_STATUS.INT_STATUS     = radioCmd[1];
	Si4455Cmd.GET_INT_STATUS.PH_PEND        = radioCmd[2];
	Si4455Cmd.GET_INT_STATUS.PH_STATUS      = radioCmd[3];
	Si4455Cmd.GET_INT_STATUS.MODEM_PEND     = radioCmd[4];
	Si4455Cmd.GET_INT_STATUS.MODEM_STATUS   = radioCmd[5];
	Si4455Cmd.GET_INT_STATUS.CHIP_PEND      = radioCmd[6];
	Si4455Cmd.GET_INT_STATUS.CHIP_STATUS    = radioCmd[7];
}

/*************************************************************************************************
 * Function Name		:	Si4455_Configure
 * Description			:	Initialize the Si4455 by loading all the configuration
 * 							properties into it
 * @ Param *pSetPropCmd 	: 	Bytes from the patch and radio config files
 * Returns		        :	1. SI4455_SUCCESS on success
					2. SI4455_CTS_TIMEOUT on CTS Timeout
					3. SI4455_COMMAND_ERROR on error
 * **********************************************************************************************/
unsigned char zetaspi::Si4455_Configure(const unsigned char *pSetPropCmd)
{
	unsigned char col;
	unsigned char response;
	unsigned char numOfBytes;
    unsigned int stepcount = 0;
	/* While cycle as far as the pointer points to a command */
	while (*pSetPropCmd != 0x00)
	{
        stepcount++;
		/* Commands structure in the array:
      --------------------------------
      LEN | <LEN length of data>
		 */
		numOfBytes = *pSetPropCmd++;

		if (numOfBytes > 16u)
		{
			/* Initial configuration of Si4x55 */
			if (SI4455_CMD_ID_WRITE_TX_FIFO == *pSetPropCmd)
			{
				if (numOfBytes > 128u)
				{
					/* Number of command bytes exceeds maximal allowable length */
					return SI4455_COMMAND_ERROR;
				}

				/* Load array to the device */
				pSetPropCmd++;
				WriteEZConfigArray(numOfBytes - 1, pSetPropCmd);

				/* Point to the next command */
				pSetPropCmd += numOfBytes - 1 ;

				/* Continue command interpreter */
				continue;
			}
			else
			{
				/* Number of command bytes exceeds maximal allowable length */
				return SI4455_COMMAND_ERROR;
			}
		}

		for (col = 0u; col < numOfBytes; col++)
		{
			radioCmd[col] = *pSetPropCmd;
			pSetPropCmd++;
		}
        
        if(SI4455_CMD_ID_EZCONFIG_CHECK == radioCmd[0])
        {
            __NOP();
        }
        
		if (SendCmdGetResp(numOfBytes, radioCmd, 1, &response) != 0xFF)
		{
			/* Timeout occured */
			return SI4455_CTS_TIMEOUT;
		}

		/* Check response byte of EZCONFIG_CHECK command */
		if (SI4455_CMD_ID_EZCONFIG_CHECK == radioCmd[0])
		{
            printf("Stepcount: %d\n\r", stepcount);
			if (response)
			{
				/* Number of command bytes exceeds maximal allowable length */
				return SI4455_COMMAND_ERROR;
			}
		}

		/* Get and clear all interrupts.  An error has occured... */
		GetIntStatus(0, 0, 0);
		if (Si4455Cmd.GET_INT_STATUS.CHIP_PEND & SI4455_CMD_GET_CHIP_STATUS_REP_CMD_ERROR_PEND_MASK)
		{
			return SI4455_COMMAND_ERROR;
		}
	}

	return SI4455_SUCCESS;
}

/***************************************************************************************************
 * Function Name		:	Radio_PowerUp
 * Description			:	When the Si4455 is powered by the MCU, it gives some
 * 					time to SI4455 for POR and clears the CTS flag
 * Returns		        :	None
 * *************************************************************************************************/
void zetaspi::Radio_PowerUp()
{
	Wait_POR();
	cts_flag = false;
	
}

/***************************************************************************************************
 * Function Name		:	Si4455_HWInitialize
 * Description			:	Initialize and configure the SI4455
 * Returns		        :	SI4455_SUCCESS on success and SI4455_FAIL on failure
 * *************************************************************************************************/
Si4455_T zetaspi::Si4455_HWInitialize()
{
	if (SI4455_SUCCESS != Si4455_Configure(RadioConfiguration.Radio_ConfigurationArray))
		return SI4455_FAIL;
	else

		/* On success, read the Interrupt status and clear the pending one's */
		GetIntStatus(0, 0, 0);
	return SI4455_SUCCESS;
}

/********************************************************************************
 * Function Name		:	StartRX
 * Description			:	Update states of SI4455 to RX
 * Returns		        :	None
 * *****************************************************************************/
void zetaspi::StartRX(unsigned char CHANNEL, unsigned char CONDITION, unsigned int RX_LEN, 
		unsigned char NEXT_STATE1, unsigned char NEXT_STATE2, unsigned char NEXT_STATE3)
{
	radioCmd[0] = SI4455_CMD_ID_START_RX;
	radioCmd[1] = CHANNEL;
	radioCmd[2] = CONDITION;
	radioCmd[3] = (unsigned char)(RX_LEN >> 8);
	radioCmd[4] = (unsigned char)(RX_LEN);
	radioCmd[5] = NEXT_STATE1;
	radioCmd[6] = NEXT_STATE2;
	radioCmd[7] = NEXT_STATE3;

	SendCmds(SI4455_CMD_ARG_COUNT_START_RX, radioCmd);
}

/********************************************************************************
 * Function Name		:	Radio_StartRx
 * Description			:	Configure SI4455 to RX Mode
 * Returns		        :	None
 * *****************************************************************************/
void zetaspi::Radio_StartRx()
{
	unsigned char channel;
	channel = RadioConfiguration.Radio_ChannelNumber;
	// Read ITs, clear pending ones
	GetIntStatus(0, 0, 0);

	// Start Receiving packet, channel 0, START immediately, Packet n bytes long
	StartRX(channel, 0, RadioConfiguration.Radio_PacketLength,
			SI4455_CMD_START_RX_ARG_RXTIMEOUT_STATE_ENUM_NOCHANGE,
			SI4455_CMD_START_RX_ARG_RXVALID_STATE_ENUM_RX,
			SI4455_CMD_START_RX_ARG_RXINVALID_STATE_ENUM_RX );
}

/********************************************************************************
 * Function Name		:	StartTX
 * Description			:	Update states of SI4455 to TX
 * Returns		        :	None
 * *****************************************************************************/
void zetaspi::StartTX(unsigned char CHANNEL, unsigned char CONDITION, 
		unsigned int TX_LEN)
{
	radioCmd[0] = SI4455_CMD_ID_START_TX;
	radioCmd[1] = CHANNEL;
	radioCmd[2] = CONDITION;
	radioCmd[3] = (unsigned char)(TX_LEN >> 8);
	radioCmd[4] = (unsigned char)(TX_LEN);

	SendCmds(SI4455_CMD_ARG_COUNT_START_TX, radioCmd);
}

/********************************************************************************
 * Function Name		:	StartTx
 * Description			:	Configure SI4455 to TX Mode
 * @param Channel 		: 	Channel Frequency
 * @param Packet 		: 	Packet to be sent
 * Returns		        :	None
 * *****************************************************************************/
void zetaspi::Start_Tx(unsigned char *pioFixRadioPacket)
{
    volatile unsigned char channel;
	channel = RadioConfiguration.Radio_ChannelNumber;
	GetIntStatus(0, 0, 0);

	/* Start sending packet on channel, START immediately, Packet according to PH */
	StartTX(channel, 0, 0);
   
}

/********************************************************************************
 * Function Name		:	SPI_SI4455_Init
 * Description			:	Initialize and configure the SI4455
 * Returns		        :	None
 * *****************************************************************************/
void zetaspi::SPI_SI4455_Init()
{
	Ret_state = Si4455_HWInitialize();
	if(Ret_state == SI4455_SUCCESS)
    {
		printf("Configuration Successful\n\r");
    }
	if(Ret_state == SI4455_FAIL)
    {
		printf("Configuration Failed\n\r");
    }
}

/********************************************************************************
 * Function Name		:	SPI_Init
 * Description			:	Initialize the SPI module in the MCU
 * Returns		        :	None
 * *****************************************************************************/
void zetaspi::SPI_Init()
{
	//SPI setup
	spidevice.format(8, 0); //8 bits, cpol, cpha 0, MSB first default
    spidevice.frequency(1000000); //1 MHz
	/* POR */
 	Radio_PowerUp();   
	
	/*
  	Configure the SPI bus as follows
    	1. SPI bus speed 	= 1 MHz
    	2. Data Out 		= From MSB bit ---> To LSB bit
    	3. Data Mode 		= SPI MODE0
	 */
	
    CS = 1;
}

