#include <stdint.h>

#include "i2c.h"
#include "DS1307.h"

/* Buffer where we will read/write our data */
unsigned char buf[8] = {0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x09, 0x00};


void DS1307_Read()
{
	unsigned char i;
	
 	/* We will now read data from DS1307 */
	/* Reading for a memory based device always starts with a dummy write */
	/* Send a start condition */
	I2CStart();
	
	/* Send slave address with write */
	I2CSend(0xD0);
	
	/* Send address for dummy write operation
	this address is actually where we are going to read from */
	I2CSend(0x00);
 
	/* Send a repeated start, after a dummy 
	write to start reading */
	I2CRestart();
	/* send slave address with read bit set */
	I2CSend(0xD1);
	
	/* Loop to read 8 bytes from I2C slave */
	for( i = 0; i < 8; i++ ) 
	{
		/* read a byte */
		buf[i] = I2CRead();
		
		/* ACK if its not the last byte to read */
		/* if its the last byte then send a NAK */
		if( i == 7 )
			I2CNak();
		else
			I2CAck();
	}
	/* Send stop */
	I2CStop();
}

void DS1307_Write()
{
	unsigned char i;
	
	buf[SECOND] &= 0x7F;
	
	if( buf[DAY_OF_WEEK] == 0x00 )
		buf[DAY_OF_WEEK] = 1;
	
	/* Send Start condition */
	I2CStart();
	/* Send DS1307 slave address with write operation */
	I2CSend(0xD0);
	/* Send subaddress 0x00, we are writing to this location */
	I2CSend(0x00);
 
	/* Loop to write 8 bytes */
	for (i = 0; i < 8; i++) {
		/* send I2C data one by one */
		I2CSend( buf[i] );
	}
 
	/* Send a stop condition - as transfer finishes */
	I2CStop();
}

void DS1307_Init()
{
	/* Initialize I2C Port */
	I2CInit();
}

#define BCD2INT(a)			((((a)>>4)*10)+((a)&0x0F))

unsigned char DS1307_isPaused()
{
	return ( ( buf[0] & 0x80 ) != 0x00 );
}

unsigned char DS1307_isWrong()
{
	if( buf[SECOND] > 0x59 )
		return 1;
	if( buf[MINUTE] > 0x59 )
		return 1;
	if( buf[DAY_OF_WEEK] > 0x07 )
		return 1;
	if( buf[DAY_OF_WEEK] == 0x00 )
		return 1;
	
	if( buf[DAY] > 0x31 )
		return 1;
	return 0;
}

unsigned char DS1307_Get( DS1307_FIELD field )
{
	switch( field )
	{
		case HOUR:
		{
			/* TODO: Verificar tratamento de 12/24 horas */
			return buf[ field ] & 0x3F;
		}
		
		
		case SECOND:
		{
			return buf[ field ] & 0x7F;
		}
		
		case DAY:
		case MONTH:
		case YEAR:
		case MINUTE:
		{
			return buf[ field ];
		}
		
		case DAY_OF_WEEK:
		{
			return (buf[ field ] & 0x0F);	
		}
		
		default:
			return 0;
		
	}
	
	return 0;
}

void DS1307_Set( DS1307_FIELD field, unsigned char value )
{
	unsigned char bcd;
	
	bcd = value / 10;
	bcd <<= 4;
	bcd |= (value%10);
	
	switch( field )
	{
		case DAY:		
		case MONTH:		
		case YEAR:		
		case DAY_OF_WEEK:		
		case MINUTE:		
		case SECOND:		
		case HOUR:		
			buf[ field ] = bcd;		break;
			
	}
}

void DS1307_WriteMem( unsigned char addr, unsigned char *data, unsigned char size )
{
	if( addr > 55 )
		return;
	
	addr += 0x08;

	/* Send Start condition */
	I2CStart();
	/* Send DS1307 slave address with write operation */
	I2CSend(0xD0);
	/* Send subaddress 0x00, we are writing to this location */
	I2CSend(addr);
 
	while( size )
	{
		/* send I2C data one by one */
		I2CSend( *data );
		data++;
		size--;
	}
	
	/* Send a stop condition - as transfer finishes */
	I2CStop();
}

unsigned char DS1307_ReadMem( unsigned char addr )
{
	unsigned char resp;
	
	if( addr > 55 )
		return;
	
	addr += 0x08;

 	/* We will now read data from DS1307 */
	/* Reading for a memory based device always starts with a dummy write */
	/* Send a start condition */
	I2CStart();
	
	/* Send slave address with write */
	I2CSend(0xD0);
	
	/* Send address for dummy write operation
	this address is actually where we are going to read from */
	I2CSend(addr);
 
	/* Send a repeated start, after a dummy 
	write to start reading */
	I2CRestart();
	/* send slave address with read bit set */
	I2CSend(0xD1);
	
	/* read a byte */
	resp = I2CRead();
		
	I2CNak();
	/* Send stop */
	I2CStop();
	
	return resp;
}

void DS1307_WriteInt32( unsigned char addr, uint32_t data )
{
	DS1307_WriteMem( addr,   (unsigned char)( data >> 24 ), 1 );
	DS1307_WriteMem( addr+1, (unsigned char)( data >> 16 ), 1 );
	DS1307_WriteMem( addr+2, (unsigned char)( data >> 8 ), 1 );
	DS1307_WriteMem( addr+3, (unsigned char)( data >> 0 ), 1 );
}

uint32_t DS1307_ReadInt32( unsigned char addr )
{
	uint32_t data;
	
	data = (uint32_t)DS1307_ReadMem( addr ); data <<= 8;
	data += (uint32_t)DS1307_ReadMem( addr + 1 ); data <<= 8;
	data += (uint32_t)DS1307_ReadMem( addr + 2 ); data <<= 8;
	data += (uint32_t)DS1307_ReadMem( addr + 3 );
}


