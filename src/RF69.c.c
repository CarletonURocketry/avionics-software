/**
 * @file RFM_69.c
 * @desc driver for the RFM_69 radio transciever which is controlled using SPI by a SAMD21
 * @author Arsalan Syed
 * @date 2019-01-02
 * Last Author:
 * Last Edited On:
 */

#include <RFM_69.h>
#include <RFM69registers.h>
#include <sercom-spi.c>

#define volatile uint8_t RFM69_boudrate
#define volatile uint8_t networkID
#define volatile uint8_t nodeID
#define volatile uint8_t MessageCTB
#define volatile uint64_t syncWord

#define volatile uint8_t currentMode
#define volatile uint8_t interrupts


void RFM_69_init(struct RFM_69_desc_t *descriptor, //descriptor should contain information about uart
                   struct sercom_spi_desc_t *spi_inst,
                   uint8_t cs_pin_group,
                   uint32_t cs_pin_mask,){

	 /*need to set sercom*/

	/*start in standby mode*/
	Write_Reg(REG_OPMODE, RF_OPMODE_SEQUENCER_ON | RF_OPMODE_LISTEN_OFF | RF_OPMODE_STANDBY);

	/* set to packet mode, with Data modulation done through frequency shift keying (FSK), with no shaping */
  Write_Reg(REG_DATAMODUL, RF_DATAMODUL_DATAMODE_PACKET | RF_DATAMODUL_MODULATIONTYPE_FSK | RF_DATAMODUL_MODULATIONSHAPING_00);

	/*set to high power mode. This is mandatory*/
	Write_Reg(REG_OCP, _isRFM69HW ? RF_OCP_OFF : RF_OCP_ON); //turns on the overload current protection
	Write_Reg(REG_PALEVEL, (uint8_t*)(RF_PALEVEL_PA1_ON | RF_PALEVEL_PA2_ON | 0x1f)); //turns on power ampplifiers 1 and 2, and 0x1f sets their power to maximum

	/* setting the carrier frequency. For rocket applications, should be set to 915MHz*/
	if(freqBand == RF69_315MHZ){
	 	 Write_Reg(REG_FRFMSB, (uint8_t*)RF69_315MHZ);
		 Write_Reg(REG_FRFMID, (uint8_t*)RF69_315MHZ);
		 Write_Reg(REG_FRFLSB, (uint8_t*)RF69_315MHZ);
	}
	if(freqBand == RF_FRFMID_433){
	 	 Write_Reg(REG_FRFMSB, (uint8_t*)RF_FRFMID_433);
		 Write_Reg(REG_FRFMID, (uint8_t*)RF_FRFMID_433);
		 Write_Reg(REG_FRFLSB, (uint8_t*)RF_FRFMID_433);
	}

	if(freqBand == RF_FRFMSB_915){
	 	 Write_Reg(REG_FRFMSB, (uint8_t*)RF_FRFMSB_915);
		 Write_Reg(REG_FRFMID, (uint8_t*)RF_FRFMSB_915);
		 Write_Reg(REG_FRFLSB, (uint8_t*)RF_FRFMSB_915);
  }
	/*set bitrate to 4.8 KBPs, check page 22 on the data sheet for other bitrates */
	Write_Reg(REG_BITRATEMSB, RF_BITRATEMSB_55555);
  Write_Reg(REG_BITRATELSB, RF_BITRATELSB_55555);

	/* setting frequency deviation to 5kH (default value)*/
	Write_Reg(REG_FDEVMSB, RF_FDEVMSB_50000);
	Write_Reg(REG_FDEVLSB, RF_FDEVLSB_50000);

	 /*sets the recieving bandwidth to the default value = 10kHz. Note: bitRate < 2 * bandwidth */
	Write_Reg(REG_RXBW, RF_RXBW_DCCFREQ_010 | RF_RXBW_MANT_16 | RF_RXBW_EXP_2);

	 /*clears flags and FIFO*/
  Write_Reg(REG_IRQFLAGS2, RF_IRQFLAGS2_FIFOOVERRUN);

	 /*threshold of signal strength; when we should start listening*/
  Write_Reg(REG_RSSITHRESH, 220);

    /*start filling the FIFO if 1)sync interrupt occurs, 2)there are no errors in the sync word.  Also, we expect the
	sync word to be 3 bytes long*/
  Write_Reg(REG_SYNCCONFIG, RF_SYNC_ON | RF_SYNC_FIFOFILL_AUTO | RF_SYNC_SIZE_2 | RF_SYNC_TOL_0);

	/*setting the sync Word and network ID and nodeID*/
  Write_Reg(REG_SYNCVALUE1, syncWord);
  Write_Reg(REG_SYNCVALUE2, networkID);
	Write_Reg(REG_NODEADRS, nodeID);

	/*1)allow address filter 2)set to variable length packet mode 3)turn off encryption 4) turn on checksum 5)if crc fails, clear FIFO and do not generate payloadReady interrupt*/
  Write_Reg(REG_PACKETCONFIG1, RF_PACKET1_ADRSFILTERING_NODE| RF_PACKET1_FORMAT_VARIABLE | RF_PACKET1_DCFREE_OFF | RF_PACKET1_CRC_ON | RF_PACKET1_CRCAUTOCLEAR_ON);

	/*1)set delay between each transmission to be 2bit (based on PA ramp down time - do not change), 2) clears FIFO after a packet has been delivered and FIFO read
	3) disable AES encryption*/
	Write_Reg(REG_PACKETCONFIG2, RF_PACKET2_RXRESTARTDELAY_2BITS | RF_PACKET2_AUTORXRESTART_ON | RF_PACKET2_AES_OFF);

	/*maximum length of payload that can be recieved in variable length packet mode*/
  Write_Reg(REG_PAYLOADLENGTH, 66);

	/*when Fifo is not empty, and module is set to TX mode, the module will begin transmitting immedietly*/
  Write_Reg(REG_FIFOTHRESH, RF_FIFOTHRESH_TXSTART_FIFONOTEMPTY | RF_FIFOTHRESH_VALUE);

	/*default config: run DAGC continously in RX mode for fade margain improvement*/
    Write_Reg(REG_TESTDAGC, RF_DAGC_IMPROVED_LOWBETA0);

}

void RFM_69_serivce(){

	/*IRQFLAG2[3] == payloadReady flag*/
	if(currentMode == RF69_MODE_RX && Read_Reg(REG_IRQFLAGS2)[3] == true){

		/*must be in standby to read the FIFO*/
		SetMode(RF69_MODE_STANDBY);
		payloadLength = Read_Reg(REG_FIFO);

	}

	//if there has been a packet recieved
	//start to read that package



}


void Write_Reg(uint8_t address, uint8_t value){
	/* writes to a register vs SPI*/

    //setting the r/w bit to write
  	address = (address | 0x80);

    //merge the value and the address into a single message
    uint16_t message = 0x00;
    message |= address ;
    message = message <<8 ;
    message |= value ;

		//sending the message...
		sercom_spi_start(RFM69_desc_t.spi_inst,
                                *trans_id, RFM_69_baudrate,
                                cs_pin_group, cs_pin_mask,
                                &message,16,
                                NULL, 0);




	//checks to see if the transaction is done, if so, clear from queue
  bool done = 0 ;
  do{
    done = sercom_spi_transaction_done(struct sercom_spi_desc_t *spi_inst,  uint8_t trans_id);
  }
	while(done != 1) {
    done = sercom_spi_transaction_done(struct sercom_spi_desc_t *spi_inst,  uint8_t trans_id);
	}

  sercom_spi_clear_transaction(struct sercom_spi_desc_t *spi_inst, uint8_t trans_id);
}

void Read_Reg(uint8_t address){
	/* Reads registers via SPI*/

  //setting the wnr bit to 0 (read)
	address = (address & 0x7F);

  //sending address byte, and expecting a data byte back
	sercom_spi_start(struct  *spi_inst,
                               *trans_id, RFM_69_baudrate,
                                cs_pin_group, cs_pin_mask,
                                NULL, 0,
                                &address, 8);


	//checks to see if the transaction is done, and if so, then clears it from queue
  bool done = 0 ;
  do{
    done = sercom_spi_transaction_done(struct sercom_spi_desc_t *spi_inst,  uint8_t trans_id);
  }
  while(done != 1) {
    done = sercom_spi_transaction_done(struct sercom_spi_desc_t *spi_inst,  uint8_t trans_id);
  }

  sercom_spi_clear_transaction(struct sercom_spi_desc_t *spi_inst, uint8_t trans_id);

}

void SetFrequency(uint32_t desired_freqHz){

	//set to standby mode
  uint8_t current_mode = Read_Reg(REG_OPMODE) & 0xE0
	if (current_mode != RF69_OPMODE_STANDBY){
    SetMode(RF69_OPMODE_STANDBY);
  }

  // FSTEP is the minimum incriment between frequencies
	desired_freqHz /= RF69_FSTEP

	//note: frf = FSTEP * frf(23;0)

	Write_Reg(REG_FRFMSB, desired_freqHz >> 16);  //writing to MSB
  Write_Reg(REG_FRFMID, desired_freqHz >> 8);   //writing to mid sig byte
  Write_Reg(REG_FRFLSB, desired_freqHz);        //writing to LSB

	//return to the mode it was in before reconfiguration
	setMode(current_mode)
}

void SetMode(uint8_t opMode){

	switch(opMode){

    //only bits 4-2 in REG_OPMODE control the mode of the module.
		case RF69_MODE_TX:
			Write_Reg(REG_OPMODE, (Read_Reg(REG_OPMODE) & 0xE0)| RF_OPMODE_TRANSMITTER);
			break;
		case RF69_MODE_RX:
			Write_Reg(REG_OPMODE, (Read_Reg(REG_OPMODE) & 0xE0) | RF_OPMODE_RECEIVER);
			//should check rssi
			break;
		case RF69_MODE_SYNTH:
			Write_Reg(REG_OPMODE, (Read_Reg(REG_OPMODE) & 0xE0) | RF_OPMODE_SYNTHESIZER);
			break;
		case RF69_MODE_STANDBY:
			Write_Reg(REG_OPMODE, (Read_Reg(REG_OPMODE) & 0xE0) | RF_OPMODE_STANDBY | RF_OPMODE_LISTEN_ON);

			break;
		case RF69_MODE_SLEEP:
			Write_Reg(REG_OPMODE, (Read_Reg(REG_OPMODE) & 0xE0) | RF_OPMODE_SLEEP);
			break;

		default:
			return;

    //wait until mode change has occured before returning
    //will fail if given an 'opmode' that doesn't fall into one of the cases
    while(Read_Reg(REG_OPMODE) & 0xE0 != opmode){
      //wait
    }

	}

}

void Sleep(){
  setMode(RF_OPMODE_SLEEP);
}

void SetMyAddress(uint8_t address){
  //sets this node's addresses
  Write_Reg(REG_NODEADRS, _address);
}

void SetMyNetworkID(uint8_t networkID){
  // set this node's SetMyNetworkID
  Write_Reg(REG_SYNCVALUE2, networkID);
}

void SetPowerLevel(uint8_t powerLevel){

}
void PacketRecieved(){
		/*can only read from FIFO in standby mode*/
		SetMode(RF69_MODE_STANDBY);

	//set to standby to read from fifo and then reset the fifo
	//start extracting the stuff






	//enable interrupts
	}





}
void transmit(uint8_t networkID, uint8_t nodeID, uint8_t buffer_length, const void* out_buffer, bool requestACK, book sendACK){
	//transmits message from FIFO

	//control byte
	if(requestACK == true){
		CTB = RFM69_CTL_REQACK;
	if(sendACK == true){
		CTB = RFM69_CTL_SENDACK;
	}

	setMode(RF69_MODE_STANDBY); //to prevent broadcasting before FIFO has been filled with our message

	//write to FIFO
	write_Reg(REG_FIFO,NULL); //writes the FIFO address with a bit that indicates that we want to write to the FIFO we are sending to

	/*begining of message we will be sending */
	write_Reg(REG_FIFO, buffer_length);
	Write_Reg(REG_FIFO, networkID);
	write_Reg(REG_FIFO, nodeID);
	write_Reg(REG_FIFO, CTB);

  //writing the radio's FIFO with the payload
	for(uint8_t i ==0; i < buffer_length; i++){
		Write_Reg(REG_FIFO, (uint8_t)buffer[i])      //one byte is sent at a time
	}
	/*end of message we will be sending */

	/*disable interupts*/
	ToggleInterrupts(0);

	/*initiate transmission*/
	setMode(RF69_MODE_TX);

	/*transmit for as long as we can, the limit is 1 second*/
	uint8_t txStartTime = millis();
	while(txStartTime - millis() < 1000){
		//wait
	}
	setMode(RF69_MODE_STANDBY);
	toggleInterrupts(1);

	/*stop transmission*/


	}


}
void ToggleInterrupts(uint8_t on_off){
	if(on_off == 1){
		//interrupts are turned on
		interrupts = 1;
	}
	if(on_off == 0){
		//interrupts are turned off
		interrupts = 0;
	}


}
