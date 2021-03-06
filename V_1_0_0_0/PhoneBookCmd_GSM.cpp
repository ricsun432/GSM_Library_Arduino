/*********************************************************************
 *
 *       Phonebook command
 *
 *********************************************************************
 * FileName:        Phonebookcmd_GSM.cpp
 * Revision:        1.0.0
 * Date:			10/09/2016
 * Dependencies:	PhoneBookCmd_GSM.h
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
 * Function:        SetCmd_AT_CPBS
 *
 * Overview:        This function is used to Select Phonebook Memory Storage or view selected Phonebook memory. For details see AT commands datasheet
 *			
 *
 * PreCondition:    None
 *
 * GSM cmd syntax:	AT+CPBS=<storage> to select Phonebook memory and AT+CPBS? to view selected Phonebook memory	
 *					This command returns also the number of locations available and the memory used
 *
 * Input:           TypeOfPhoneBook: PhoneBook Memory storage string, see PhoneBookCmd_GSM.h file for details
 *					Query: 			 If "true" esecute a query command otherwise set Phonebook memory storage
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
uint8_t PhoneBookCmd_GSM::SetCmd_AT_CPBS(uint8_t TypeOfPhoneBook, bool Query) {
	if ((Gsm.StateWaitAnswerCmd != CMD_WAIT_IDLE) || (Gsm.UartState != UART_IDLE_STATE) || (Gsm.GsmFlag.Bit.CringOccurred == 1)) {
		return(0);		//	System Busy
	} else {
		Gsm.ClearBuffer();
		Gsm.BckCmdData[0] = TypeOfPhoneBook;
		Gsm.BckCmdData[1] = Query;
		Gsm.GsmFlag.Bit.GsmSendCmdInProgress = 1;
		Gsm.ResetFlags();
		if (Query == false) {
			Gsm.ReadStringFLASH((uint8_t *)AT_CPBS, (uint8_t *)Gsm.GSM_Data_Array, strlen(AT_CPBS));
			FlashAddPhoneBook(TypeOfPhoneBook);
			Gsm.ReadStringFLASH(FlashPbmAdd, ((uint8_t *)Gsm.GSM_Data_Array + CPBS_PBM), strlen(PBM_ME));
			Gsm.GSM_Data_Array[CPBS_PBM + 4] = ASCII_CARRIAGE_RET;
			Gsm.GSM_Data_Array[CPBS_PBM + 5] = ASCII_LINE_FEED;	
			Gsm.WritePointer = CPBS_PBM + 6;			
		} else {
			Gsm.ReadStringFLASH((uint8_t *)ATQ_CPBS, (uint8_t *)Gsm.GSM_Data_Array, strlen(ATQ_CPBS));
			Gsm.WritePointer = strlen(ATQ_CPBS);
		}
		Gsm.StartSendData(CMD_PHONEBOOK_IDLE, WAIT_ANSWER_CMD_AT_CPBS, ANSWER_PHONEBOOK_AT_CMD_STATE);
	}
	return(1);			//	Command sent
}
/****************************************************************************/

/****************************************************************************
 * Function:        SetCmd_AT_CPBR
 *
 * Overview:        This function is used to Read Phonebook entries. For details see AT commands datasheet
 *			
 *
 * PreCondition:    None
 *
 * GSM cmd syntax:	AT+CPBR=<inex> to select Phonebook location number and AT+CPBR=? to view the max number of locations,
 *					max lenght for phone number and max number of characters for the text
 *
 * Input:           Index: PhoneBook Index
 *					Query: If "true" esecute a query command otherwise read Phonebook entries
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
uint8_t PhoneBookCmd_GSM::SetCmd_AT_CPBR(uint8_t Index, bool Query) {
	uint8_t Offset;
	
	if ((Gsm.StateWaitAnswerCmd != CMD_WAIT_IDLE) || (Gsm.UartState != UART_IDLE_STATE) || (Gsm.GsmFlag.Bit.CringOccurred == 1)) {
		return(0);		//	System Busy
	} else {
		Gsm.ClearBuffer();
		Gsm.BckCmdData[0] = Index;
		Gsm.BckCmdData[1] = Query;
		Gsm.GsmFlag.Bit.GsmSendCmdInProgress = 1;
		Gsm.ResetFlags();
		if (Query == false) {
			Gsm.ReadStringFLASH((uint8_t *)AT_CPBR, (uint8_t *)Gsm.GSM_Data_Array, strlen(AT_CPBR));
			Offset = Gsm.ConvertNumberToChar(Index, CPBR_INDEX, 0);
			Gsm.GSM_Data_Array[CPBR_INDEX + Offset]     = ASCII_CARRIAGE_RET;
			Gsm.GSM_Data_Array[CPBR_INDEX + Offset + 1] = ASCII_LINE_FEED;	
			Gsm.WritePointer = CPBR_INDEX + Offset + 2;			
		} else {
			Gsm.ReadStringFLASH((uint8_t *)ATQ_CPBR, (uint8_t *)Gsm.GSM_Data_Array, strlen(ATQ_CPBR));
			Gsm.WritePointer = strlen(ATQ_CPBR);
		}
		Gsm.StartSendData(CMD_PHONEBOOK_IDLE, WAIT_ANSWER_CMD_AT_CPBR, ANSWER_PHONEBOOK_AT_CMD_STATE);
	}
	return(1);			//	Command sent
}
/****************************************************************************/

/****************************************************************************
 * Function:        SetCmd_AT_CPBW
 *
 * Overview:        This function is used to Write Phonebook entry. For details see AT commands datasheet
 *			
 *
 * PreCondition:    None
 *
 * GSM cmd syntax:	AT+CPBW=<inex>[,<number>,[<type>,[<text>]]]
 *					Examples: 
 *					AT+CPBW=3 							(Erase location 3)
 *			 	 	AT+CPBW=5,”112”,129,”SOS”			(Write at location 5)
 *			  		AT+CPBW=5,”01290917”,129,”Jacky”	(Overwrite location 5)
 *			  		AT+CPBW=,”+33145221100”,145,”SOS”	(Write at the first location available)
 *
 *					The following characters in <text> must be entered via the escape sequence:
 *  				Char	Seq.Seq.(hex)	   Note
 *                  -------------------------------------------------
 *	   				 \		\5C 5C 35 43 	(backslash)
 *     				 " 		\22 5C 32 32 	(string delimiter)
 *    				BSP     \08 5C 30 38    (backspace)
 *    				NULL    \00 5C 30 30    (GSM null)
 *					-------------------------------------------------
 *
 * Input:           Index:   		 PhoneBook Index
 *					CmdType: 		 Type of command to be execute: "0" Erase Entry; "1" Read Phonebook Entry; "2" Write Phonebook Entry
 *					PhoneNumberType: 129	Email address o mailing list
 *									 161	National number
 *									 145	International number
 *									 177	Network specific
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
uint8_t PhoneBookCmd_GSM::SetCmd_AT_CPBW(uint8_t Index, uint8_t CmdType) {
	uint8_t Offset;
	uint8_t StrLenght;
	
	if ((Gsm.StateWaitAnswerCmd != CMD_WAIT_IDLE) || (Gsm.UartState != UART_IDLE_STATE) || (Gsm.GsmFlag.Bit.CringOccurred == 1)) {
		return(0);		//	System Busy
	} else {
		Gsm.ClearBuffer();
		Gsm.BckCmdData[0] = Index;
		Gsm.BckCmdData[1] = CmdType;
		Gsm.GsmFlag.Bit.GsmSendCmdInProgress = 1;
		Gsm.ResetFlags();
		Gsm.ReadStringFLASH((uint8_t *)AT_CPBW, (uint8_t *)Gsm.GSM_Data_Array, strlen(AT_CPBW));
		switch (CmdType)
		{
			case CPBW_ERASE_ENTRY:
				Offset = CPBR_INDEX + Gsm.ConvertNumberToChar(Index, CPBW_INDEX, 0);
				break;
				
			case CPBW_WRITE_ENTRY:
			case CPBW_WRITE_FIRST_EMPTY_ENTRY:
				if (CmdType == CPBW_WRITE_ENTRY) {
					Offset = Gsm.ConvertNumberToChar(Index, CPBW_INDEX, 0) + CPBR_INDEX;
				} else {
					Offset = CPBW_INDEX;
				}
				Gsm.GSM_Data_Array[Offset] = ASCII_COMMA;
				StrLenght = strlen(PhoneNumber);
				strncat((char *)(&Gsm.GSM_Data_Array[Offset + 1]), (char *)(&PhoneNumber[0]), StrLenght);
				Offset += (StrLenght + 1);				
				Gsm.GSM_Data_Array[Offset] = ASCII_COMMA;
				Offset += Gsm.ConvertNumberToChar(PhoneNumberType, (Offset + 1), 0) + 1;
				Gsm.GSM_Data_Array[Offset] = ASCII_COMMA;
				StrLenght = strlen(PhoneText);
				strncat((char *)(&Gsm.GSM_Data_Array[Offset + 1]), (char *)(&PhoneText[0]), StrLenght);
				Offset += StrLenght + 1;
				break;
		}
		Gsm.GSM_Data_Array[Offset]     = ASCII_CARRIAGE_RET;
		Gsm.GSM_Data_Array[Offset + 1] = ASCII_LINE_FEED;	
		Gsm.WritePointer = Offset + 2;			

		Gsm.StartSendData(CMD_PHONEBOOK_IDLE, WAIT_ANSWER_CMD_AT_CPBW, ANSWER_PHONEBOOK_AT_CMD_STATE);
	}
	return(1);			//	Command sent
}
/****************************************************************************/

/****************************************************************************
 * Function:        GsmPhoneBookWaitAnswer
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
void PhoneBookCmd_GSM::GsmPhoneBookWaitAnswer(void) {
	uint8_t Counter;
	uint8_t Mul;
	
	if ((Gsm.StateSendCmd != CMD_PHONEBOOK_IDLE) || (Gsm.UartState != UART_IDLE_STATE)) {
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

				case WAIT_ANSWER_CMD_AT_CPBS:
					Gsm.StateWaitAnswerCmd = CMD_WAIT_IDLE;
										
					if (Gsm.TestAT_Cmd_Answer((uint8_t *)AT_ANSW_CPBS, (uint8_t *)Gsm.TempStringCompare, strlen(AT_ANSW_CPBS)) != 0xFF) {
						if (Gsm.TestAT_Cmd_Answer((uint8_t *)PBM_DC, (uint8_t *)Gsm.TempStringCompare, strlen(PBM_DC)) != 0xFF) {
							PhoneBookFlag.Bit.StorageCode = CODE_PBM_DC;
						} else if (Gsm.TestAT_Cmd_Answer((uint8_t *)PBM_EN, (uint8_t *)Gsm.TempStringCompare, strlen(PBM_EN)) != 0xFF) {
							PhoneBookFlag.Bit.StorageCode = CODE_PBM_EN;
						} else if (Gsm.TestAT_Cmd_Answer((uint8_t *)PBM_FD, (uint8_t *)Gsm.TempStringCompare, strlen(PBM_FD)) != 0xFF) {
							PhoneBookFlag.Bit.StorageCode = CODE_PBM_FD;
						} else if (Gsm.TestAT_Cmd_Answer((uint8_t *)PBM_MC, (uint8_t *)Gsm.TempStringCompare, strlen(PBM_MC)) != 0xFF) {
							PhoneBookFlag.Bit.StorageCode = CODE_PBM_MC;
						} else if (Gsm.TestAT_Cmd_Answer((uint8_t *)PBM_ON, (uint8_t *)Gsm.TempStringCompare, strlen(PBM_ON)) != 0xFF) {
							PhoneBookFlag.Bit.StorageCode = CODE_PBM_ON;
						} else if (Gsm.TestAT_Cmd_Answer((uint8_t *)PBM_RC, (uint8_t *)Gsm.TempStringCompare, strlen(PBM_RC)) != 0xFF) {
							PhoneBookFlag.Bit.StorageCode = CODE_PBM_RC;
						} else if (Gsm.TestAT_Cmd_Answer((uint8_t *)PBM_SM, (uint8_t *)Gsm.TempStringCompare, strlen(PBM_SM)) != 0xFF) {
							PhoneBookFlag.Bit.StorageCode = CODE_PBM_SM;
						} else if (Gsm.TestAT_Cmd_Answer((uint8_t *)PBM_LA, (uint8_t *)Gsm.TempStringCompare, strlen(PBM_LA)) != 0xFF) {
							PhoneBookFlag.Bit.StorageCode = CODE_PBM_LA;
						} else if (Gsm.TestAT_Cmd_Answer((uint8_t *)PBM_ME, (uint8_t *)Gsm.TempStringCompare, strlen(PBM_ME)) != 0xFF) {
							PhoneBookFlag.Bit.StorageCode = CODE_PBM_ME;
						} else if (Gsm.TestAT_Cmd_Answer((uint8_t *)PBM_BN, (uint8_t *)Gsm.TempStringCompare, strlen(PBM_BN)) != 0xFF) {
							PhoneBookFlag.Bit.StorageCode = CODE_PBM_BN;
						} else if (Gsm.TestAT_Cmd_Answer((uint8_t *)PBM_SD, (uint8_t *)Gsm.TempStringCompare, strlen(PBM_SD)) != 0xFF) {
							PhoneBookFlag.Bit.StorageCode = CODE_PBM_SD;
						} else if (Gsm.TestAT_Cmd_Answer((uint8_t *)PBM_VM, (uint8_t *)Gsm.TempStringCompare, strlen(PBM_VM)) != 0xFF) {
							PhoneBookFlag.Bit.StorageCode = CODE_PBM_VM;
						} else if (Gsm.TestAT_Cmd_Answer((uint8_t *)PBM_LD, (uint8_t *)Gsm.TempStringCompare, strlen(PBM_LD)) != 0xFF) {
							PhoneBookFlag.Bit.StorageCode = CODE_PBM_LD;
						}
						
						if (Gsm.FindColonCommaCarriageRet() != 0xFF) {
							PhoneBookFlag.Bit.UsedLocations  = Gsm.ExtractParameterByte((Gsm.CharPointers[1] + 1), (Gsm.CharPointers[2] - (Gsm.CharPointers[1] + 1)));
							PhoneBookFlag.Bit.TotalLocations = Gsm.ExtractParameterByte((Gsm.CharPointers[2] + 1), (Gsm.CharPointers[3] - (Gsm.CharPointers[2] + 1)));		
						}
					}
					break;
				
				case WAIT_ANSWER_CMD_AT_CPBR:
					Gsm.StateWaitAnswerCmd = CMD_WAIT_IDLE;

					if (Gsm.TestAT_Cmd_Answer((uint8_t *)AT_ANSW_CPBR, (uint8_t *)Gsm.TempStringCompare, strlen(AT_ANSW_CPBR)) != 0xFF) {
						if (Gsm.FindBracketMinus() != 0xFF) {
							PhoneBookFlag.Bit.TotalLocations = Gsm.ExtractParameterByte((Gsm.CharPointers[1] + 1), (Gsm.CharPointers[2] - (Gsm.CharPointers[1] + 1)));						
							if (Gsm.FindColonCommaCarriageRet() != 0xFF) {
								MaxPhoneNumberLenght     = Gsm.ExtractParameterByte((Gsm.CharPointers[1] + 1), (Gsm.CharPointers[2] - (Gsm.CharPointers[1] + 1)));
								MaxPhoneNumberTextLenght = Gsm.ExtractParameterByte((Gsm.CharPointers[2] + 1), (Gsm.CharPointers[3] - (Gsm.CharPointers[2] + 1)));							
							}										
						} else {
							if (Gsm.FindColonCommaCarriageRet() != 0xFF) {
								Gsm.ExtractCharArray((char *)(&PhoneBook.PhoneNumber[0]), (char *)(&Gsm.GSM_Data_Array[Gsm.CharPointers[1] + 1]), sizeof(PhoneBook.PhoneNumber), ASCII_COMMA);								
								PhoneNumberType = Gsm.ExtractParameterByte((Gsm.CharPointers[2] + 1), (Gsm.CharPointers[3] - (Gsm.CharPointers[2] + 1)));								
								Gsm.ExtractCharArray((char *)(&PhoneBook.PhoneText[0]), (char *)(&Gsm.GSM_Data_Array[Gsm.CharPointers[3] + 1]), sizeof(PhoneBook.PhoneText), ASCII_CARRIAGE_RET);						
							}
						}
					}				
					break;
				
				case WAIT_ANSWER_CMD_AT_CPBW:
					Gsm.StateWaitAnswerCmd = CMD_WAIT_IDLE;					
					break;
					
				default:
					break;	
			}
		} else {
			Gsm.ProcessGsmError();
			GsmPhoneBookRetrySendCmd();
		}		
	} else {
		Gsm.InitReset_GSM();
	}
}	
/****************************************************************************/

/****************************************************************************
 * Function:        GsmPhoneBookRetrySendCmd
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
void PhoneBookCmd_GSM::GsmPhoneBookRetrySendCmd(void) {
	if (Gsm.RetryCounter++ < 2) {
		switch (Gsm.StateWaitAnswerCmd)
		{
			case WAIT_ANSWER_CMD_AT_CPBS:
				Gsm.StateWaitAnswerCmd = CMD_WAIT_IDLE;
				SetCmd_AT_CPBS(Gsm.BckCmdData[0], Gsm.BckCmdData[1]);
				break;
				
			case WAIT_ANSWER_CMD_AT_CPBR:
				Gsm.StateWaitAnswerCmd = CMD_WAIT_IDLE;
				SetCmd_AT_CPBR(Gsm.BckCmdData[0], Gsm.BckCmdData[1]);
				break;
				
			case WAIT_ANSWER_CMD_AT_CPBW:
				Gsm.StateWaitAnswerCmd = CMD_WAIT_IDLE;
				SetCmd_AT_CPBW(Gsm.BckCmdData[0], Gsm.BckCmdData[1]);
				break;
				
			default:
				break;
		}
	} else {
		Gsm.InitReset_GSM();
	}
}
/****************************************************************************/

/****************************************************************************
 * Function:        FlashAddPhoneBook
 *
 * Overview:        This function return address of FLASH memory
 *
 * PreCondition:    None
 *
 * GSM cmd syntax:	None
 *
 * Input:           TypeOfPhoneBook: PHONE BOOK Memory type
 *
 * Command Note:	None
 *
 * Output:          None
 *
 * GSM answer det:	None
 *
 * Side Effects:    None
 *
 * Note:            This is a private function
 *****************************************************************************/
void PhoneBookCmd_GSM::FlashAddPhoneBook(uint8_t TypeOfPhoneBook) {
	switch (TypeOfPhoneBook)
	{
		case CODE_PBM_DC:	//	ME dialed calls list (+CPBW may not be applicable for this storage)
			FlashPbmAdd = (uint8_t *)PBM_DC;
			break;
		case CODE_PBM_EN:	//	SIM (or MT) emergency number (+CPBW is not be applicable for this storage)
			FlashPbmAdd = (uint8_t *)PBM_EN;
			break;
		case CODE_PBM_FD:	//	SIM fix dialing phonebook. If a SIM card is present or if a UICC with an active GSM application is present, the information in EFFDN under DFTelecom is selected
			FlashPbmAdd = (uint8_t *)PBM_FD;
			break;
		case CODE_PBM_MC:	//	MT missed (unaswered received) calls list (+CPBW may not be applicable for this storage)
			FlashPbmAdd = (uint8_t *)PBM_MC;
			break;
		case CODE_PBM_ON:	//	SIM (or MT) own numbers (MSISDNs) list (Reading of this storage may be available through +CNUM also). When storing information in the SIM/UICC, if a SIM card is present or if a UICC with an active GSM application is present, the information in EFMSISDN under DFTelecom is selected
			FlashPbmAdd = (uint8_t *)PBM_ON;
			break;
		case CODE_PBM_RC:	//	MT received calls list (+CPBW may not be applicable for this storage)
			FlashPbmAdd = (uint8_t *)PBM_RC;
			break;
		case CODE_PBM_SM:	//	SIM/UICC phonebook. If a SIM card is present or if a UICC whit an active GSM application is present, the EFADN under DFTelecom is selected 
			FlashPbmAdd = (uint8_t *)PBM_SM;
			break;
		case CODE_PBM_LA:	//	Last number all list (LND/LNM/LNR)	
			FlashPbmAdd = (uint8_t *)PBM_LA;
			break;
		case CODE_PBM_ME:	//	ME phonebook
			FlashPbmAdd = (uint8_t *)PBM_ME;
			break;
		case CODE_PBM_BN:	//	SIM barred dialed number
			FlashPbmAdd = (uint8_t *)PBM_BN;
			break;
		case CODE_PBM_SD:	//	SIM service dial number
			FlashPbmAdd = (uint8_t *)PBM_SD;
			break;
		case CODE_PBM_VM:	//	SIM voice mailbox
			FlashPbmAdd = (uint8_t *)PBM_VM;
			break;
		case CODE_PBM_LD:	//	SIM last dialing phonebook
			FlashPbmAdd = (uint8_t *)PBM_LD;
			break;
		default:
			FlashPbmAdd = (uint8_t *)PBM_ME;
			break;
	}
}
/****************************************************************************/
