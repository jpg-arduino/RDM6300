#include <RDM6300.h>

RDM6300::RDM6300(uint8_t receivePin, uint8_t transmitPin) : _rfidSerial(receivePin, transmitPin)
{
}

RDM6300::~RDM6300()
{
	_rfidSerial.end();
}

void RDM6300::begin()
{
	// RDM6300 works at 9600bps, N, 8, 1
	_rfidSerial.begin(9600);
}

void RDM6300::end()
{
	_rfidSerial.end();
}

bool RDM6300::read(RDM6300_tag *tag)
{
	StatusCode result;
	byte buffer[6];
	byte bufferSize = 6;

	result = readSerial(buffer, &bufferSize);
	if (result == STATUS_OK) {
		tag->header = buffer[0];
		tag->id = ((uint32_t)buffer[1] << 24) | ((uint32_t)buffer[2] << 16) | ((uint32_t)buffer[3] << 8) | buffer[4];
		tag->checksum = buffer[bufferSize - 1];

		return true;
	}

	return false;
}

bool RDM6300::read(byte *data, byte *length, byte *checksum)
{
	StatusCode result;
	byte buffer[6];
	byte bufferSize = 6;

	result = readSerial(buffer, &bufferSize);
	if (result == STATUS_OK) {
		memcpy(data, buffer, bufferSize - 1);
		if (checksum)
			*checksum = buffer[bufferSize - 1];
		*length = bufferSize - 1;

		return true;
	}

	return false;
}

uint32_t RDM6300::readInt(byte *checksum)
{
	StatusCode result;
	byte buffer[6];
	byte bufferSize = 6;

	result = readSerial(buffer, &bufferSize);
	if (result == STATUS_OK) {
		if (checksum)
			*checksum = buffer[bufferSize - 1];
		return ((uint32_t)buffer[1] << 24) | ((uint32_t)buffer[2] << 16) | ((uint32_t)buffer[3] << 8) | buffer[4];
	}

	return 0;
}

/**
 * Read the serial data from RDM6300.
 *
 * RDM6300 module
 * _____________________________________________
 * |0x02| DATA(10 ascii chars) | CHECKSUM | 03 |
 *  =============================================
 * params: 96008n1
 */
RDM6300::StatusCode RDM6300::readSerial(byte *data, byte *length)
{
	bool synched = false;
	uint8_t syncErrors = 0;
	uint8_t offset = 0;
	byte buffer[12]; // 10 Data ASCII chars + 2 Checksum ASCII chars
	byte out[6];
	byte outOffset = 0;
	byte checksum = 0x00;

	// STATUS_NO_DATA and STATUS_OK are treated equally on the
	// global error variable.
	_error = STATUS_OK;

	// Check the length is ok
	if (*length < RDM6300_DATA_SIZE) {
		_error = STATUS_NO_ROOM;
		return STATUS_NO_ROOM;
	}

	// If no data is available return right away
	if (!_rfidSerial.available())
		return STATUS_NO_DATA;

	// Zero memory
	memset(buffer, 0, 12);
	memset(out, 0, 6);

	while (_rfidSerial.available())
	{
		// delay to make sure the byte gets read
		delayMicroseconds(936);

		byte rxByte = _rfidSerial.read();
		if (!synched) {
			if (rxByte == 0x02)
			{
				synched = true;
			}
			else
			{
				delayMicroseconds(104);
				syncErrors++;
				if (syncErrors > 14) {
					_error = STATUS_SYNC_ERROR;
					return STATUS_SYNC_ERROR;
				}
			}

			continue;
		}

		// Have we reached the end?
		if (rxByte == 0x03) {
			// Convert the ASCII has values to real bytes
			for (byte i = 0; i < offset; (i = i + 2)) {
				byte highNibble = buffer[i] - '0';
				byte lowNibble = buffer[i + 1] - '0';

				if (highNibble > 9) highNibble -= 7; // A..F
				if (lowNibble > 9) lowNibble -= 7; // A..F

				out[outOffset++] = (highNibble << 4) | lowNibble;
				if (outOffset <= 5)
					checksum ^= out[outOffset - 1];
			}

			// Copy data to the return byte array
			*length = outOffset;
			memcpy(data, out, outOffset);

			// check the length
			if (offset < 12) {
				_error = STATUS_TOO_SHORT;
				return STATUS_TOO_SHORT;
			}
			
			// checksum
			if (out[5] != checksum) {
				_error = STATUS_BAD_CHECKSUM;
				return STATUS_BAD_CHECKSUM;
			}
			
			return STATUS_OK;
		}

		// Sanity check to prevent buffer overflow
		if (offset >= 12) {
			_error = STATUS_OVERFLOW;
			return STATUS_OVERFLOW;
		}
		
		// Store the byte
		buffer[offset++] = (byte)rxByte;
	}

	// The result should be returned before
	// but just in case...
	return STATUS_NO_DATA;
}