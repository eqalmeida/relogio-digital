/* 
 * File:   i2c.h
 * Author: eduardoalmeida
 *
 * Created on May 28, 2016, 12:53 PM
 */

#ifndef I2C_H
#define	I2C_H

#ifdef	__cplusplus
extern "C" {
#endif

    void I2CInit(void);
    void I2CStart();
    void I2CStop();
    void I2CSend(unsigned char dat);
    unsigned char I2CRead(void);
    void I2CRestart();
    void I2CNak();
    void I2CAck();

#ifdef	__cplusplus
}
#endif

#endif	/* I2C_H */

