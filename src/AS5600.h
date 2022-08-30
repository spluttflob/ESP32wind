/** @file AS5600.h
 *  This Arduino Library operates an AS5600L angle sensor.
 *  @author Kane C. Stoboi, created original file
 *  @author JR Ridgely, added more documentation and functionality
 *  @date 2018-Aug-22 Original file by Stoboi
 *  @date 2019-Sep-20 Modified version by Ridgely
 *  @date 2022-Aug-28 Replaced constructors with one that's given an I2C port
 *  @copyright Original file released by Stoboi into the public domain,
 *      available at https://github.com/kanestoboi/AS5600.
 *  	Modified version with Doxygen comments and code enhancements (c) 2022
 *  	by JR Ridgely and released under the Lesser GNU Public License V2.
 */

#ifndef _AS5600_H_
#define _AS5600_H_

#include "Arduino.h"
#include <Wire.h>


/** This class operates an AS5600L magnetic angle sensor using an Arduino
 *  TwoWire I2C interface such as @c Wire.
 */

class AS5600
{
protected:
	uint8_t _AS5600Address = 0x36;

	const byte _ZMCOAddress = 0x00;
	const byte _ZPOSAddressMSB = 0x01;
	const byte _ZPOSAddressLSB = 0x02;
	const byte _MPOSAddressMSB = 0x03;
	const byte _MPOSAddressLSB = 0x04;
	const byte _MANGAddressMSB = 0x05;
	const byte _MANGAddressLSB = 0x06;
	const byte _CONFAddressMSB = 0x07;
	const byte _CONFAddressLSB = 0x08;
	const byte _RAWANGLEAddressMSB = 0x0C;
	const byte _RAWANGLEAddressLSB = 0x0D;
	const byte _ANGLEAddressMSB = 0x0E;
	const byte _ANGLEAddressLSB = 0x0F;
	const byte _STATUSAddress = 0x0B;
	const byte _AGCAddress = 0x1A;
	const byte _MAGNITUDEAddressMSB = 0x1B;
	const byte _MAGNITUDEAddressLSB = 0x1C;
	const byte _BURNAddress = 0xFF;

	const long _msbMask = 0b00001111;

	uint16_t getRegisters2 (uint8_t registerMSB);
	uint8_t getRegister (uint8_t register1);

    TwoWire* p_i2c;   ///< Pointer to the I2C port to which sensor is connected

public:
    AS5600 (TwoWire& i2c, uint8_t address = 0x36);

    uint16_t getPosition (void);
    uint16_t getAngle (void);
    uint8_t getStatus (void);
    uint8_t getGain (void);
    uint16_t getMagnitude (void);
    void setZero (void);
};

#endif // _AS5600_H_

