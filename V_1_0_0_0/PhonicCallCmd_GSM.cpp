/*********************************************************************
 *
 *       PhonicCall command
 *
 *********************************************************************
 * FileName:        PhonicCallCmd_GSM.cpp
 * Revision:        1.0.0
 * Date:			01/10/2016
 * Dependencies:	PhonicCallCmd_GSM.h
 *					PhoneBookCmd_GSM.h
 * Arduino Board:	Arduino Uno, Arduino Mega 2560, Fishino Uno, Fishino Mega 2560       
 *
 * Company:         Futura Group srl
 *  				www.Futurashop.it
 *  				www.open-electronics.org
 *
 * Developer:		Destro Matteo
 *
 * Support:			info@open-electronics.org
 * 
 * Software License Agreement
 *
 * Copyright (c) 2016, Futura Group srl 
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 *	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 *	ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *	WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * 	DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 *	ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *	(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *	LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *	ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *	SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 **********************************************************************/

#include "PhonicCallCmd_GSM.h"
#include "PhoneBookCmd_GSM.h"

#ifdef __AVR__
#include <avr/pgmspace.h>
#endif

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

/****************************************************************************
 * Function:        SetCmd_ATA
 *
 * Overview:        This function is used to answer an incoming call. For details see AT commands datasheet
 *
 * PreCondition:    None
 *
 * GSM cmd syntax:	ATA	
 *
 * Input:           None
 *
 * Command Note:	None
 *
 * Output:          Return unsigned char
 *                  Return -> 0 (System Busy. Command not executed)
 *					Return -> 1 (Command sent)
 *
 * GSM answer det:  None
 *
 * Side Effects:    None
 *
 * Note:            This is a public function
 *****************************************************************************/
uint8_t PhonicCallCmd_GSM::SetCmd_ATA(void) {	
	if ((Gsm.StateWaitAnswerCmd != CMD_WAIT_IDLE) || (Gsm.UartState != UART_IDLE_STATE) || (Gsm.GsmFlag.Bit.CringOccurred == 1)) {
		return(0);		//	System Busy
	} else {
		Gsm.ClearBuffer();
		Gsm.GsmFlag.Bit.GsmSendCmdInProgress = 1;
		Gsm.ResetFlags();
		Gsm.ReadStringFLASH((uint8_t *)AT_ATA, (uint8_t *)Gsm.GSM_Data_Array, strlen(AT_ATA));
		Gsm.WritePointer = strlen(AT_ATA);			
		Gsm.StartSendData(CMD_PHONIC_CALL_IDLE, WAIT_ANSWER_CMD_ATA, ANSWER_PHONIC_CALL_AT_CMD_STATE);
	}
	return(1);			//	Command sent
}
/****************************************************************************/	

/****************************************************************************
 * Function:        SetCmd_ATH
 *
 * Overview:        This function is used to disconnect existing connection. For details see AT commands datasheet
 *
 * PreCondition:    None
 *
 * GSM cmd syntax:	ATH[n]	
 *
 * Input:           n: ATH parameter, see below
 *
 * Command Note:	<n>	"0" Disconnect ALL calls on the channel the command is requested. All active or waiting calls,
 *						    CS data calls, GPSR call of the channel will be disconnected
 *					<n>	"1" Disconnect all calls on ALL connected channels. All active or waiting calls, CSD calls,
 *						    GPRS call will be disconnected. (clean up all calls of the ME)
 *					<n>	"2" Disconnect all connected CS data call only on the channel the command is required.
 *							(speech calls (active or waiting) or GPRS calls are not disconnected)
 *					<n>	"3" Disconnect all connected GPRS calls only on the channel the command is requested.
 *							(speech calls (active or waiting) or CS data calls are not disconnected)
 *					<n>	"4" Disconnect all CS calls (either speech or data) but does not disconnect waiting call
 *							(either speech or data) on the channel the command is required
 *					<n>	"5" Disconnect waiting call (either speech or data) but does not disconnect other active call
 *							(either CS speech, CS data or GPRS) on the channel the command is requested. (rejection of incoming call)
 *
 * Output:          Return unsigned char
 *                  Return -> 0 (System Busy. Command not executed)
 *					Return -> 1 (Command sent)
 *
 * GSM answer det:  None
 *
 * Side Effects:    None
 *
 * Note:            This is a public function
 *****************************************************************************/
uint8_t PhonicCallCmd_GSM::SetCmd_ATH(uint8_t n) {	
	if ((Gsm.StateWaitAnswerCmd != CMD_WAIT_IDLE) || (Gsm.UartState != UART_IDLE_STATE) || (Gsm.GsmFlag.Bit.CringOccurred == 1)) {
		return(0);		//	System Busy
	} else {
		Gsm.ClearBuffer();
		Gsm.BckCmdData[0] = n;
		Gsm.GsmFlag.Bit.GsmSendCmdInProgress = 1;
		Gsm.ResetFlags();
		Gsm.ReadStringFLASH((uint8_t *)AT_ATH, (uint8_t *)Gsm.GSM_Data_Array, strlen(AT_ATH));
		#if defined(SIMCOM_SIM900) || defined(SIMCOM_SIM928A) || defined(QUECTEL_M95)
			Gsm.GSM_Data_Array[ATH_INDEX] = n + 0x30;
		#endif
		Gsm.WritePointer = strlen(AT_ATH);			
		Gsm.StartSendData(CMD_PHONIC_CALL_IDLE, WAIT_ANSWER_CMD_ATH, ANSWER_PHONIC_CALL_AT_CMD_STATE);
	}
	return(1);			//	Command sent
}
/****************************************************************************/

/****************************************************************************
 * Function:        SetCmd_ATD
 *
 * Overview:        This function is used to set up outgoing voice, data or fax calls. For details see AT commands datasheet
 *					This command may be aborted generaly by receiving an ATH command or a caracter during execution.
 *					The aborting is not possible during some states of connection establishment such as handshaking.
 *
 * PreCondition:    This command use <n> parameter only
 *
 * GSM cmd syntax:	ATD<n>[<mgsm>][;]	
 *
 * Input:           None
 *
 * Command Note:	<n>	   0-9, *, #, +, A, B, C
 *					<mgsm> String of GSM modifiers:
 *	   					   I	Actives CLIR (Disables presentation of own number to called party)
 *		   				   i	Deactivates CLIR (Enable presentation of own number to called party)
 *		   				   G	Activates Closed User Group invocation for this call only
 *		   				   g	Deactivates Closed User Group invocation for this call only
 *					<;>	   Only required to set up voice call, return to command state
 *
 * Output:          Return unsigned char
 *                  Return -> 0 (System Busy. Command not executed)
 *					Return -> 1 (Command sent)
 *
 * GSM answer det:  None
 *
 * Side Effects:    None
 *
 * Note:            This is a public function
 *****************************************************************************/
uint8_t PhonicCallCmd_GSM::SetCmd_ATD(void) {
	uint8_t Offset;
	uint8_t StrLenght;
	
	if ((Gsm.StateWaitAnswerCmd != CMD_WAIT_IDLE) || (Gsm.UartState != UART_IDLE_STATE) || (Gsm.GsmFlag.Bit.CringOccurred == 1)) {
		return(0);		//	System Busy
	} else {
		Gsm.ClearBuffer();
		Gsm.GsmFlag.Bit.GsmSendCmdInProgress = 1;
		Gsm.ResetFlags();
		Gsm.ReadStringFLASH((uint8_t *)AT_ATD, (uint8_t *)Gsm.GSM_Data_Array, strlen(AT_ATD));
		
		Offset = ATD_INDEX;	
		StrLenght = strlen(PhoneBook.PhoneNumber);
		strncat((char *)(&Gsm.GSM_Data_Array[Offset]), (char *)(&PhoneBook.PhoneNumber[0]), StrLenght);
		Offset += StrLenght;
		Gsm.GSM_Data_Array[Offset]     = ASCII_SEMI_COLON;
		Gsm.GSM_Data_Array[Offset + 1] = ASCII_CARRIAGE_RET;	
		Gsm.GSM_Data_Array[Offset + 2] = ASCII_LINE_FEED;	
		Gsm.WritePointer = Offset + 3;
		Gsm.StartSendData(CMD_PHONIC_CALL_IDLE, WAIT_ANSWER_CMD_ATD, ANSWER_PHONIC_CALL_AT_CMD_STATE);
	}
	return(1);			//	Command sent
}
/****************************************************************************/

/****************************************************************************
 * Function:        SetCmd_ATD_PhoneNumberMemory
 *
 * Overview:        This function is used to dial a phone number from current phonebook. For details see AT commands datasheet
 *					This command may be aborted generaly by receiving an ATH command or a caracter during execution.
 *					The aborting is not possible during some states of connection establishment such as handshaking.
 *
 * PreCondition:    This command use <n> parameter only
 *
 * GSM cmd syntax:	ATD><n>[<clir>][<cug>][;]	
 *
 * Input:           n: PhoneBook entry
 *
 * Command Note:	<n> 	Integer type memory location should be in the range of locations available in the memory used
 *					<clir>  String of GSM modifiers:
 *							I	Override the CLIR supplementary service subscription default value for this call
 *								Invocation (restrict CLI presentation)
 *							i 	Override the CLIR supplementary service subscription default value for this call
 *								Suppression (allow CLI presentation)
 *					<cug>	G   Control the CUG supplementary service information for this call
 *								CUG Not supported
 *							g	Control the CUG supplementary service information for this call
 *								CUG Not supported
 *					<;>		Only required to set up voice call , return to command state
 *
 * Output:          Return unsigned char
 *                  Return -> 0 (System Busy. Command not executed)
 *					Return -> 1 (Command sent)
 *
 * GSM answer det:  None
 *
 * Side Effects:    None
 *
 * Note:            This is a public function
 *****************************************************************************/
uint8_t PhonicCallCmd_GSM::SetCmd_ATD_PhoneNumberMemory(uint8_t n) {
	if ((Gsm.StateWaitAnswerCmd != CMD_WAIT_IDLE) || (Gsm.UartState != UART_IDLE_STATE) || (Gsm.GsmFlag.Bit.CringOccurred == 1)) {
		return(0);		//	System Busy
	} else {
		Gsm.ClearBuffer();
		Gsm.BckCmdData[0] = n;
		Gsm.GsmFlag.Bit.GsmSendCmdInProgress = 1;
		Gsm.ResetFlags();
		Gsm.ReadStringFLASH((uint8_t *)AT_ATD_PHONE_NUMBER_MEMORY, (uint8_t *)Gsm.GSM_Data_Array, strlen(AT_ATD_PHONE_NUMBER_MEMORY));
		Gsm.GSM_Data_Array[ATD_PHONE_NUMBER_MEMORY_INDEX] = n + 0x30;
		Gsm.GSM_Data_Array[ATD_PHONE_NUMBER_MEMORY_INDEX + 1] = ASCII_SEMI_COLON;
		Gsm.GSM_Data_Array[ATD_PHONE_NUMBER_MEMORY_INDEX + 2] = ASCII_CARRIAGE_RET;	
		Gsm.GSM_Data_Array[ATD_PHONE_NUMBER_MEMORY_INDEX + 3] = ASCII_LINE_FEED;	
		Gsm.WritePointer = ATD_PHONE_NUMBER_MEMORY_INDEX + 4;			
		Gsm.StartSendData(CMD_PHONIC_CALL_IDLE, WAIT_ANSWER_CMD_ATD_PHONE_NUMBER_MEMORY, ANSWER_PHONIC_CALL_AT_CMD_STATE);
	}
	return(1);			//	Command sent
}
/****************************************************************************/

/****************************************************************************
 * Function:        SetCmd_ATDL
 *
 * Overview:        This function is used to Redial Last Telephone Number Used. For details see AT commands datasheet
 *					This command may be aborted generaly by receiving an ATH command or a caracter during execution.
 *					The aborting is not possible during some states of connection establishment such as handshaking.
 *
 * PreCondition:    None
 *
 * GSM cmd syntax:	ATDL	
 *
 * Input:           None
 *
 * Command Note:	None
 *
 * Output:          Return unsigned char
 *                  Return -> 0 (System Busy. Command not executed)
 *					Return -> 1 (Command sent)
 *
 * GSM answer det:  None
 *
 * Side Effects:    None
 *
 * Note:            This is a public function
 *****************************************************************************/
uint8_t PhonicCallCmd_GSM::SetCmd_ATDL(void) {
	uint8_t Offset;
	
	if ((Gsm.StateWaitAnswerCmd != CMD_WAIT_IDLE) || (Gsm.UartState != UART_IDLE_STATE) || (Gsm.GsmFlag.Bit.CringOccurred == 1)) {
		return(0);		//	System Busy
	} else {
		Gsm.ClearBuffer();
		Gsm.GsmFlag.Bit.GsmSendCmdInProgress = 1;
		Gsm.ResetFlags();
		Gsm.ReadStringFLASH((uint8_t *)AT_ATDL, (uint8_t *)Gsm.GSM_Data_Array, strlen(AT_ATDL));
		Gsm.WritePointer = strlen(AT_ATDL);
		Gsm.StartSendData(CMD_PHONIC_CALL_IDLE, WAIT_ANSWER_CMD_ATDL, ANSWER_PHONIC_CALL_AT_CMD_STATE);	
	}
	return(1);			//	Command sent
}
/****************************************************************************/

/****************************************************************************
 * Function:        GsmPhonicCallWaitAnswer
 *
 * Overview:        This function process the AT command answer of the command sent.
 *					The answer received and processed by this code regard the Generic Command Functions
 *                  implemented in this library file
 *
 * PreCondition:    None
 *
 * GSM cmd syntax:	None
 *
 * Input:           None
 *
 * Command Note:	None
 *
 * Output:          None
 *
 * GSM answer det:	None
 *
 * Side Effects:    None
 *
 * Note:            This is a public function
 *****************************************************************************/
void PhonicCallCmd_GSM::GsmPhonicCallWaitAnswer(void) {	
	uint8_t StrPointer = 0xFF;
	uint8_t Offset;
	uint8_t StrLenght;
	uint8_t Counter;
	uint8_t Mul;
	
	if ((Gsm.StateSendCmd != CMD_PHONIC_CALL_IDLE) || (Gsm.UartState != UART_IDLE_STATE)) {
		return;
	}
	
	if (Gsm.UartFlag.Bit.ReceivedAnswer == 0) {
		return;
	}
	Gsm.UartFlag.Bit.ReceivedAnswer = 0;
	if (Gsm.GsmFlag.Bit.CringOccurred == 1) {
		//	CRING OCCURRED. CMD SEND ABORTED
		Gsm.RetryCounter = 0;
		Gsm.GsmFlag.Bit.GsmSendCmdInProgress = 0;
		Gsm.StateWaitAnswerCmd = CMD_WAIT_IDLE;
		return;
	}
	
	if (Gsm.ReadPointer > 0) {
		if (Gsm.TestAT_Cmd_Answer((uint8_t *)AT_ANSW_OK, (uint8_t *)Gsm.TempStringCompare, strlen(AT_ANSW_OK)) != 0xFF) {
			Gsm.RetryCounter = 0;
			Gsm.GsmFlag.Bit.GsmSendCmdInProgress = 0;
			
			switch (Gsm.StateWaitAnswerCmd)
			{
				case CMD_WAIT_IDLE:
					break;

				case WAIT_ANSWER_CMD_ATH:
					PhonicCall.PhonicCallFlag.Bit.OutgoingCallInProgress = 0;
					PhonicCall.PhonicCallFlag.Bit.IncomingCallInProgress = 0;
					Gsm.StateWaitAnswerCmd = CMD_WAIT_IDLE;
					break;
					
				case WAIT_ANSWER_CMD_ATA:
					PhonicCall.PhonicCallFlag.Bit.IncomingCallInProgress = 1;
					Gsm.StateWaitAnswerCmd = CMD_WAIT_IDLE;
					break;
					
				case WAIT_ANSWER_CMD_ATDL:
				case WAIT_ANSWER_CMD_ATD:
				case WAIT_ANSWER_CMD_ATD_PHONE_NUMBER_MEMORY:
					PhonicCall.PhonicCallFlag.Bit.OutgoingCallInProgress = 1;
					Gsm.StateWaitAnswerCmd = CMD_WAIT_IDLE;
					break;
				
				default:
					break;
			}		
		} else {
			Gsm.ProcessGsmError();
			GsmPhonicCallRetrySendCmd();
		}
	} else {		
		Gsm.InitReset_GSM();
	}
}	
/****************************************************************************/

/****************************************************************************
 * Function:        GsmPhonicCallRetrySendCmd
 *
 * Overview:        This function retry to send AT command for a maximum of three time
 *
 * PreCondition:    None
 *
 * GSM cmd syntax:	None
 *
 * Input:           None
 *
 * Command Note:	None
 *
 * Output:          None
 *
 * GSM answer det:	None
 *
 * Side Effects:    None
 *
 * Note:            This is a public function
 *****************************************************************************/
void PhonicCallCmd_GSM::GsmPhonicCallRetrySendCmd(void) {
	if (Gsm.RetryCounter++ < 2) {
		switch (Gsm.StateWaitAnswerCmd)
		{
			case WAIT_ANSWER_CMD_ATH:
				Gsm.StateWaitAnswerCmd = CMD_WAIT_IDLE;
				SetCmd_ATH(Gsm.BckCmdData[0]);
				break;
			
			case WAIT_ANSWER_CMD_ATD_PHONE_NUMBER_MEMORY:
				Gsm.StateWaitAnswerCmd = CMD_WAIT_IDLE;
				SetCmd_ATD_PhoneNumberMemory(Gsm.BckCmdData[0]);
				break;
						
			case WAIT_ANSWER_CMD_ATA:
				Gsm.StateWaitAnswerCmd = CMD_WAIT_IDLE;
				SetCmd_ATA();
				break;
	
			case WAIT_ANSWER_CMD_ATDL:
				Gsm.StateWaitAnswerCmd = CMD_WAIT_IDLE;
				SetCmd_ATDL();
				break;
				
			case WAIT_ANSWER_CMD_ATD:
				Gsm.StateWaitAnswerCmd = CMD_WAIT_IDLE;
				SetCmd_ATD();
				break;
				
			default:
				break;
		}
	} else {
		Gsm.InitReset_GSM();
	}
}
/****************************************************************************/