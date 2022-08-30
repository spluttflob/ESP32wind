/** @file AS5600.cpp
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

#include "Arduino.h"
#include "AS5600.h"


/** This constructor creates an AS5600 object using the given I2C (TwoWire) 
 *  driver, which has already been created, and the given address if given.
 *  @param i2c An I2C object, created as TwoWire(params)
 *  @param address The address to use for the AS5600, default 
 */
AS5600::AS5600 (TwoWire& i2c, uint8_t address)
{
    p_i2c = &i2c;
    _AS5600Address = address;
}


/** This method returns the 'raw' angular position measured by the AS5600.
 *  This unscaled and unmodified angle comes out in a 12-bit number.
 *  @return The unscaled position
 */
uint16_t AS5600::getPosition (void)
{
  return getRegisters2 (_RAWANGLEAddressMSB);
}


/** This method returns the scaled and corrected angle measured by the AS5600.
 *  @return Our best estimate of the actual angle, in a 12-bit integer
 */
uint16_t AS5600::getAngle (void)
{
  return getRegisters2 (_ANGLEAddressMSB);
}


uint8_t AS5600::getStatus (void)
{
  return getRegister (_STATUSAddress) & 0b00111000;
}


uint8_t AS5600::getGain (void)
{
  return getRegister (_AGCAddress);
}


uint16_t AS5600::getMagnitude (void)
{
  return getRegisters2 (_MAGNITUDEAddressMSB);
}


/** This method returns the contents of one register in the AS5600.
 *  @param reg_addr The register whose contents are to be found
 *  @return The contents of the register within the AS5600
 */

uint8_t AS5600::getRegister (byte reg_addr)
{
	p_i2c->beginTransmission (_AS5600Address);
	p_i2c->write (reg_addr);
	p_i2c->endTransmission ();

	uint8_t _byte = 0xFF;
	p_i2c->requestFrom (_AS5600Address, (uint8_t)1);
	if (p_i2c->available () <= 1)
	{
		_byte = p_i2c->read ();
	}

	return _byte;
}


/** This method returns the contents of two registers in the AS5600.
 *  @param registerMSB The register holding the most significant byte of the
 *  	two-byte number to be received. In the AS5600, the least significant
 *  	byte will be at the next higher register address
 *  @return A 16-bit integer containing the contents of the two registers
 */
uint16_t AS5600::getRegisters2 (uint8_t registerMSB)
{
	p_i2c->beginTransmission (_AS5600Address);
	p_i2c->write (registerMSB);
	p_i2c->endTransmission ();
	delay (10);

	uint16_t _word = 0xFFFF;
	p_i2c->requestFrom (_AS5600Address, (uint8_t)2);
	if (p_i2c->available () >= 2)
	{
		_word = p_i2c->read ();
		_word <<= 8;
		_word |= p_i2c->read ();
	}

	return _word;
}

