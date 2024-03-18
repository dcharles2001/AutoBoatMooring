/*
    Ported and modified by Guy Ringshaw 2024
    STM32 'bare metal' C code for interfacing with an Si4455 RF transceiver chip (zeta 433 config)
		STM32L432kc used
		Ported from arduino version: https://github.com/deeplyembeddedWP/EZRadio_SI4455-Library.git
*/
#include <stdbool.h>

#include "zeta.h"

/*
#define false 0
#define true 1
*/

#define RADIO_CTS_TIMEOUT			10000
#define RESP_BYTES_SIZE				16

/* Static & Global Variables */
static bool cts_flag;
static unsigned char radioCmd[RESP_BYTES_SIZE];
union si4455_cmd_reply_union Si4455Cmd;
const unsigned char Radio_Configuration_Data_Array[] = RADIO_CONFIGURATION_DATA_ARRAY;
const tRadioConfiguration RadioConfiguration = RADIO_CONFIGURATION_DATA;
static Si4455_T Ret_state;
volatile unsigned int stepcount = 0;

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
void Wait_POR(void) 
{

	/* Pull the SDN pin high for 10 us */
	GPIOB->ODR |= (1u << SDN); //SDN high
	for(int i=0; i<40; i++) //approx 10us delay
  {
      __NOP(); //do nothing
  }
    
	/* Pull the SDN pin low */
  GPIOB->ODR &=~ (1u << SDN);//SDN LOW
  
	
	for(int i=0; i<4000; i++) //approx ?ms delay
	{
				__NOP(); //do nothing
	}
	

}

/********************************************************************************************
 * Function Name		:	SpiWriteBytes
 * Description			:	Writes bytes on the SPI bus - MOSI
 * Returns		        :	None
 * Params			:	1. byteCount 	- No. of bytes to write
					2. pData	- Points to the bytes in the array                  
 * *****************************************************************************************/
void SpiWriteBytes(unsigned char byteCount, const unsigned char* pData)
{
	const unsigned char* ptr = pData;
	//while(!(SPI_MODULE->SR & (1u << 1))); //wait on TXE 
	for (int i = 0; i < byteCount; i++)
	{
		write_SPI_noCS(*ptr++);
		//while(!(SPI_MODULE->SR & (1u << 7))); //wait on busy
	}
	//while(!(SPI_MODULE->SR & (1u << 1))); //wait on TXE 
	//while(!(SPI_MODULE->SR & (1u << 7))); //wait on busy bit to indicate end of transmission, lest we bring CS high too early
	/*
	for(int i = 0; i<6; i++) //hacky delay to stop CS from rising early
	{
		__NOP();
	}
	*/
	
}

/********************************************************************************************
 * Function Name		:	SpiReadBytes
 * Description			:	Reads bytes from the SPI bus - MISO
 * Returns		        :	None
 * Params			:	1. byteCount 	- No. of bytes to read
					2. pData	- Points to the array/var to be 
							  filled
 * ******************************************************************************************/
void SpiReadBytes(unsigned char byteCount, unsigned char* pData)
{
	unsigned char* ptr = pData;
	//uint8_t temp = *(__IO uint8_t*)(&SPI_MODULE->DR); //initiate read/clear buffer
	for (int i = 0; i < byteCount; i++)
	{
		*ptr++ = read_SPI_noCS(); //keep clock on and read
		//while(!(SPI_MODULE->SR & (1u << 1))); //wait on TXE
		//while(!(SPI_MODULE->SR & (1u << 7))); //wait on busy bit to indicate end of transmission, lest we bring CS high too early
	}
 
}

/*********************************************************************************************
 * Function Name		:	GetResponse_CTS
 * Description			:	API to Poll CTS and also read an array of data from
 * 					Si4455
 * Returns		        :	0xFF on success and 0x01 on CTS time out
 * Params			:	1. byteCount 	- No. of bytes to read and filled
					2. pData	- Points to the array/var to be 
							  filled 
                    ********************************************************
                    (GUY's NOTE): second SPI second write has me uneasy, check 
                    again later if this isn't working 
 * *******************************************************************************************/
unsigned char GetResponse_CTS(unsigned char byteCount, unsigned char* pData)
{
	cts_flag = false;
	unsigned char ctsVal = 0u;
	unsigned int errCnt = 20000;
	//char ctsstring[15];

	while (errCnt != 0) //attempt based error detection                                                                                              
	{
		
		SPI_PORT->ODR &=~ (1u << SPI_NSS);//bring CS low
		write_SPI_noCS(0x44); //write CTS command
		/*
		for(int i=0; i<3; i++) //approx 10us delay, this prevents CTS fail
		{
      __NOP(); //do nothing
		}		
		*/
		ctsVal = read_SPI_noCS(); 
		//while(!(SPI_MODULE->SR & (1u << 1))); //wait on TXE
		//while(!(SPI_MODULE->SR & (1u << 7))); //wait on busy bit to indicate end of transmission, lest we bring CS high too early

		/*
		char ctsstring[15];
		sprintf(ctsstring, "CTS: %x \n\r", ctsVal);
		send_array_USART(ctsstring);
		*/
		
		if (ctsVal == 0xFF)
		{
			if (byteCount)
			{
				SpiReadBytes(byteCount, pData); //also used for getting responses back
			}
			/*
			for(int i=0; i<3; i++) //approx 10us delay, this prevents CTS fail
			{
				__NOP(); //do nothing
			}		
			*/
			SPI_PORT->ODR |= (1u << SPI_NSS); //bring CS high
			break;
		}
		SPI_PORT->ODR |= (1u << SPI_NSS); //bring CS high
		errCnt--;
	}
	
	
	if (errCnt == 0)
	{
		/* ERROR!!!!  CTS should never take this long. */
		send_array_USART("CTS Time Out\n\r"); //send message via USART
		return 0x01;
	}
	if (ctsVal == 0xFF)
	{
		cts_flag = true;
	}
	/*
	char ctsresp[12];
	sprintf(ctsresp, "CTS: %x\n\r", ctsVal);
	send_array_USART(ctsresp);
	*/
	return ctsVal;
}

/*******************************************************************************************
 * Function Name		:	PollCTS
 * Description			:	Poll for CTS
 * Returns		        :	0xFF on success and 0x01 on CTS time out
 
 * *****************************************************************************************/
unsigned char PollCTS(void)
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
void SendCmdArgs(unsigned char cmd, unsigned char pollcts, 
		unsigned char bytecount, const unsigned char *pData)
{
	if (pollcts)
	{
		while (!cts_flag)
		{
			PollCTS();
		}
	}
	SPI_PORT->ODR &=~ (1u << SPI_NSS); //CS low
	write_SPI_noCS(cmd); //write command byte
	SpiWriteBytes(bytecount, pData); //write param bytes 
	SPI_PORT->ODR |= (1u << SPI_NSS); //CS high
	cts_flag = false;
}

/*********************************************************************************************
 * Function Name		:	WriteEZConfigArray
 * Description			:	Write Configuration array to Si4455
 * Returns		        :	None
 * Params			:	1. numBytes		- No. of bytes to write
					2. pEzConfigArray	- points to the config array
 * ******************************************************************************************/
void WriteEZConfigArray(unsigned char numBytes, const 
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
void SendCmds(unsigned char ByteCount, unsigned char* pData)
{
	while (!cts_flag)
	{
		PollCTS();
	}
	SPI_PORT->ODR &=~ (1u << SPI_NSS); //CS low
	SpiWriteBytes(ByteCount, pData); //write bytes
	SPI_PORT->ODR |= (1u << SPI_NSS); //CS high
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
unsigned char SendCmdGetResp(unsigned char cmdByteCount, unsigned char *pCmdData, 
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
void GetIntStatus(unsigned char PH_CLR_PEND, unsigned char MODEM_CLR_PEND, 
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
                    ********************************************************
                    (GUY's NOTE): I have a suspicion that some of the functionality
                    of this accounts for patch files and is therefore redundant for now,
                    since the si4455 rev B1 does not require a patch  
 * **********************************************************************************************/
unsigned char Si4455_Configure(const unsigned char *pSetPropCmd)
{
	unsigned char col;
	unsigned char response;
	unsigned char numOfBytes;

	/* While cycle as far as the pointer points to a command */
	while (*pSetPropCmd != 0x00)
	{
		/* Commands structure in the array:
      --------------------------------
      LEN | <LEN length of data>
		*/
		stepcount++;
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

		if(stepcount == 72) //ez config step check
		{
			GPIOB->ODR |= (1u << TriggerLine); //triggerline high
		}
		
		if (SendCmdGetResp(numOfBytes, radioCmd, 1, &response) != 0xFF)
		{
			/* Timeout occured */
			return SI4455_CTS_TIMEOUT;
		}

		/* Check response byte of EZCONFIG_CHECK command */
		if (SI4455_CMD_ID_EZCONFIG_CHECK == radioCmd[0])
		{
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
void Radio_PowerUp()
{
	Wait_POR();
	cts_flag = false;
	
}

/***************************************************************************************************
 * Function Name		:	Si4455_HWInitialize
 * Description			:	Initialize and configure the SI4455
 * Returns		        :	SI4455_SUCCESS on success and SI4455_FAIL on failure
 * *************************************************************************************************/
Si4455_T Si4455_HWInitialize()
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
void StartRX(unsigned char CHANNEL, unsigned char CONDITION, unsigned int RX_LEN, 
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
void Radio_StartRx()
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
void StartTX(unsigned char CHANNEL, unsigned char CONDITION, 
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
void Start_Tx(unsigned char *pioFixRadioPacket)
{
	unsigned char channel;
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
void SPI_SI4455_Init()
{
	Ret_state = Si4455_HWInitialize();
	if(Ret_state == SI4455_SUCCESS)
	{
		send_array_USART("Configuration Successful\n\r");
	}
	if(Ret_state == SI4455_FAIL)
	{
		char countstring[20];
		sprintf(countstring, "Steps: %d\n\r", stepcount);
		send_array_USART(countstring);
		send_array_USART("Configuration Failed\n\r");
	}
}

/********************************************************************************
 * Function Name		:	SPI_Init
 * Description			:	Initialize the SPI module in the MCU
 * Returns		        :	None
 ********************************************************
    (GUY's NOTE): The 100ms delay is interesting, I do not
    know what "WDT" is, the programming manual does not seem to make
    any reference to such a term, we'll roll with it for now

 * *****************************************************************************/
void SPI_Init()
{
	SPI_PORT->MODER &=~ (  3u << ( SPI_NSS * 2 ) ); //clear pin 4
	GPIOB->MODER &=~ ( ( 3u << ( SDN * 2 ) ) | //clear SDN
										 ( 3u << ( TriggerLine * 2 ) ) ); //clear triggerine
	
	SPI_PORT->MODER |= ( 1u << ( SPI_NSS * 2 ) );	//output mode pin 4
  GPIOB->MODER |= ( (1u << (SDN * 2 ) ) | //SDN output
										(1u << (TriggerLine * 2 ) ) ); //triggerline output
	
	SPI_PORT->ODR &= ~(1u << SPI_NSS); //Bring CS low
	GPIOB->ODR &= ~(1u << SDN); //SDN low
	GPIOB->ODR &= ~(1u << TriggerLine); //triggerline default low
	/*
  	Configure the SPI bus as follows
    	1. SPI bus speed 	= 1 MHz
    	2. Data Out 		= From MSB bit ---> To LSB bit
    	3. Data Mode 		= SPI MODE0
	 */
			/* POR */
 	Radio_PowerUp();   
	
	init_SPI();
}
