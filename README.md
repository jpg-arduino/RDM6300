# RDM6300 #

Arduino library for RDM6300 RFID module.

Read 125 kHz Radio-Frequency IDentification (RFID) cards
on your Arduino using a RDM6300 reader.

## Methods ##

- **begin()** Starts serial communication with the card reader.
- **end()** Ends serial communication with the card reader.
- **read(RDM6300_tag \*tag)** Reads the card information and places this information in a `RDM6300_tag` structure. Returns `true`if the card was present and read was succesfull and `false` in any other case.
- **read(byte \*data, byte \*length = NULL, byte \*checksum = NULL)** Reads the card data. The `data` must be 5 bytes long, and length should be `5`. The `checksum` parameter is optional in case you want to retrieve the checksum byte aswell. Returns `true`if the card was present and read was succesfull and `false` in any other case.  
- **readInt(byte \*checksum)** This method returns the card identification number as a `uint32_t` (As printed on the cards you may buy). The `checksum` parameter is optional in case you want to retrieve the checksum byte aswell. In case of an error `0` is returned.