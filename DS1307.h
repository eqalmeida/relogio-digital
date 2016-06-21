/* 
 * File:   DS1307.h
 * Author: eduardoalmeida
 *
 * Created on May 28, 2016, 2:27 PM
 */

#ifndef DS1307_H
#define	DS1307_H

#ifdef	__cplusplus
extern "C" {
#endif
    
typedef enum{
    SECOND = 0,
    MINUTE,        
    HOUR,
    DAY_OF_WEEK,        
    DAY,
    MONTH,
    YEAR,
}
DS1307_FIELD;    

void DS1307_Read();
void DS1307_Write();
void DS1307_Init();
unsigned char DS1307_Get(DS1307_FIELD field);
void DS1307_Set( DS1307_FIELD field, unsigned char value );
unsigned char DS1307_isWrong();
unsigned char DS1307_isPaused();

void DS1307_WriteMem( unsigned char addr, unsigned char *data, unsigned char size );
unsigned char DS1307_ReadMem( unsigned char addr );
void DS1307_WriteInt32( unsigned char addr, uint32_t data );
uint32_t DS1307_ReadInt32( unsigned char addr );


#ifdef	__cplusplus
}
#endif

#endif	/* DS1307_H */

