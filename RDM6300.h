#ifndef RDM6300_H
#define RDM6300_H

#include <Arduino.h>
#include <SoftwareSerial.h>

/**
 * RDM6300 module
 * _____________________________________________
 * |0x02| DATA(10 ascii chars) | CHECKSUM | 03 |
 *  =============================================
 * params: 96008n1
 */

//#define RDM6300_MAX_RX_BUFF 14 // RX buffer size (1+10+2+1)

// Data is 10 ASCII characters representing HEX values. 
// When converted to byte we only need half of the size
// Therefore 10/2 = 5.
#define RDM6300_DATA_SIZE   5 

class RDM6300
{
public:
	enum StatusCode : byte {
		STATUS_OK = 0x00,
		STATUS_NO_DATA,
		STATUS_SYNC_ERROR,
		STATUS_BAD_CHECKSUM,
		STATUS_OVERFLOW,
		STATUS_TOO_SHORT,
		STATUS_NO_ROOM
	};

	typedef struct {
		byte header;
		uint32_t id;
		byte checksum;
	} RDM6300_tag;

	RDM6300(uint8_t receivePin, uint8_t transmitPin);
	~RDM6300();

	void begin();
	void end();

	bool read(RDM6300_tag *tag);
	bool read(byte *data, byte *length = NULL, byte *checksum = NULL);
	uint32_t readInt(byte *checksum = NULL);

	////////////////////////////////////////////////
	// Helper Methods
	////////////////////////////////////////////////

protected:
	StatusCode readSerial(byte *data, byte *length);

private:
	byte _error;
	SoftwareSerial _rfidSerial;
};

#endif
