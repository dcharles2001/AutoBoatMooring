#ifndef ZETASPI_H
#define ZETASPI_H

//#include "mbed.h"
#include "SPI.h"
#include "F4config.h"
#include "RadioConfig.h"
#include "si4455_defs.h"

/* Structure for accessing parameters from config file */
typedef struct
{
	const unsigned char   *Radio_ConfigurationArray;
	unsigned char   Radio_ChannelNumber;
	unsigned char   Radio_PacketLength;
	unsigned char   Radio_State_After_Power_Up;
	unsigned int  Radio_Delay_Cnt_After_Reset;
} tRadioConfiguration;

/* enum to return status of the radio chip */
typedef enum
{
	SI4455_SUCCESS,
	SI4455_NO_PATCH,
	SI4455_CTS_TIMEOUT,
	SI4455_PATCH_FAIL,
	SI4455_COMMAND_ERROR,
	SI4455_FAIL
}Si4455_T;



class zetaspi
{
    public:

        zetaspi(SPIConfig_t Pins, DigitalOut sdn, DigitalIn gpio1, DigitalIn nirq);
       
        void Wait_POR();
        void SpiWriteBytes(unsigned char byteCount, const unsigned char* pData);
        void SpiReadBytes(unsigned char byteCount, unsigned char* pData);
        unsigned char GetResponse_CTS(unsigned char byteCount, unsigned char* pData);
        unsigned char PollCTS();
        void SendCmdArgs(unsigned char cmd, unsigned char pollcts, unsigned char bytecount, const unsigned char *pData);
        void WriteEZConfigArray(unsigned char numBytes, const unsigned char* pEzConfigArray);
        void SendCmds(unsigned char ByteCount, unsigned char* pData);
        unsigned char SendCmdGetResp(unsigned char cmdByteCount, unsigned char *pCmdData, unsigned char respByteCount, unsigned char* pRespData);
        void GetIntStatus(unsigned char PH_CLR_PEND, unsigned char MODEM_CLR_PEND, unsigned char CHIP_CLR_PEND);
        unsigned char Si4455_Configure(const unsigned char *pSetPropCmd);
        void Radio_PowerUp();
        Si4455_T Si4455_HWInitialize();
        void StartRX(unsigned char CHANNEL, unsigned char CONDITION, unsigned int RX_LEN, unsigned char NEXT_STATE1, unsigned char NEXT_STATE2, unsigned char NEXT_STATE3);
        void Radio_StartRx();
        void StartTX(unsigned char CHANNEL, unsigned char CONDITION, unsigned int TX_LEN);
        void Start_Tx(unsigned char *pioFixRadioPacket);
        void SPI_SI4455_Init();
        void SPI_Init();



    private:

        void radioconfig(volatile const unsigned char *cmdPTR);
        void writemultiple(volatile const unsigned char *arrayPTR, unsigned int arraySize);

       
        SPI spidevice;
        DigitalOut CS;
        DigitalOut SDN;
        DigitalIn GPIO1;
        DigitalIn nIRQ; 

        
};




#endif
