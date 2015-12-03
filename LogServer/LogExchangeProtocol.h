// LogExchangeProtocol.h
// Contains any constant definitions for the Log Exchange Protocol between the Log Server & Client.
// This Protocol is built on TCP.
//

#pragma once

#include "..\\BaseUtils\\UtilsDef.h"

// Command IDs
#define	CMD_ID_INVALID			0x00
#define	CMD_ID_VALIDATE			0xA0
#define	CMD_ID_REQUESTLOG		0xB0
#define	CMD_ID_HASMORERECORD	0xC0
#define	CMD_ID_LOADRECORD		0xC1
#define	CMD_ID_CLOSEREQUEST		0xD0
#define	CMD_ID_EXTENDCMD_0		0xE0

// Size in bytes
#define FLAG_VERSION_OFFSET		0
#define FLAG_OPTION_OFFSET		1
#define FLAG_CMDID_OFFSET		2
#define FLAG_PARAM_OFFSET		3
#define FLAG_CMD_SIZE_OFFSET	4
#define FLAG_VERSION_LEN		1
#define FLAG_OPTION_LEN			1
#define FLAG_CMDID_LEN			1
#define FLAG_PARAM_LEN			1
#define FLAG_CMD_SIZE_LEN		4
#define	CMD_HEADER_SIZE			(FLAG_VERSION_LEN + FLAG_OPTION_LEN + FLAG_CMDID_LEN + FLAG_PARAM_LEN + FLAG_CMD_SIZE_LEN)
#define CMD_PARAM_SIZE_LEN		4	// the length of a returned parameter

#define	CMD_PARSING_ERROR		0xFF

// Bit flag mask
#define	CMD_MASK_RTN			0x80	// a mask to get the first bit in an option flag
#define OPTION_RTN				0x80
#define OPTION_CMD				0x00

// Field types
#define FIELD_TYPE_INVALID		0
#define FIELD_TYPE_INTEGER		1
#define FIELD_TYPE_REAL			2
#define FIELD_TYPE_TIMESTAMP	3
#define FIELD_TYPE_STRING		4

// Field flags
#define FIELD_SIZE_OFFSET		0
#define FIELD_RECORD_ID_OFFSET	4
#define FIELD_TYPE_OFFSET		5
#define FIELD_SIZE_LEN			4	// sizeof(int)
#define FIELD_RECORD_ID_LEN		1
#define FIELD_TYPE_LEN			1

// Value size descriptor of IntField, RealField and StringField starting offset
#define FIELD_VALUE_SIZE_OFFSET 6
#define FIELD_VALUE_SIZE_LEN	4

const int FIELD_INT_VALUE_SIZE = (sizeof(int));
const int FIELD_REAL_VALUE_SIZE = (sizeof(BaseUtils::Real));

// Field of Timestamp value offset
#define FIELD_TIME_VALUE_OFFSET 6
#define FIELD_TIME_YEAR_LEN		2
#define FIELD_TIME_MONTH_LEN	1
#define FIELD_TIME_DAY_LEN		1
#define FIELD_TIME_HOUR_LEN		1
#define FIELD_TIME_MINUTE_LEN	1
#define FIELD_TIME_SECOND_LEN	1

#define FIELD_TIME_VALUE_SIZE	(FIELD_TIME_YEAR_LEN + FIELD_TIME_MONTH_LEN + FIELD_TIME_DAY_LEN \
	+ FIELD_TIME_HOUR_LEN + FIELD_TIME_MINUTE_LEN + FIELD_TIME_SECOND_LEN)

#define CharCountToSize(X)	((X) * sizeof(TCHAR))
#define ByteToCharBuffer(X)	((char)((BYTE)(X)))
#define CharBufferToByte(X)	((BYTE)(X))
