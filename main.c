/*
 * File:   main.c
 * Author: Eduardo Almeida
 * 
 *
 * Created on May 27, 2016, 10:56 AM
 */



// PIC16F877A Configuration Bit Settings

// 'C' source line config statements

#include <stdint.h>
#include <xc.h>


#include "DS1307.h"


// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

// CONFIG
#pragma config FOSC = HS        // Oscillator Selection bits (HS oscillator)
#pragma config WDTE = OFF        // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config BOREN = ON       // Brown-out Reset Enable bit (BOR enabled)
#pragma config LVP = OFF         // Low-Voltage (Single-Supply) In-Circuit Serial Programming Enable bit (RB3/PGM pin has PGM function; low-voltage programming enabled)
#pragma config CPD = OFF        // Data EEPROM Memory Code Protection bit (Data EEPROM code protection off)
#pragma config WRT = OFF        // Flash Program Memory Write Enable bits (Write protection off; all program memory may be written to by EECON control)
#pragma config CP = OFF         // Flash Program Memory Code Protection bit (Code protection off)


#define MAGIC_NUMBER		( 0x5A3789D5 )
#define TIME_RELOAD			( 64910 )

#define KEY_SEL_pin			RC2
#define KEY_INC_pin			RC1

#define TECLA_SEL			( 1 )
#define TECLA_INC			( 2 )

/******************************************************************************
 * 
 * Protótipo de Funções.
 * 
 ******************************************************************************/
void BoardConfig( );
void UpdateDisplays();

void executaModoNormal();
void executaModoAjuste();
void executaModoInicial();


/******************************************************************************
 * 
 * Tabela de Conversão 7SEG.
 * 
 ******************************************************************************/
#define SEGa        ( (unsigned char) 0x01 )
#define SEGb        ( (unsigned char) 0x02 )
#define SEGc        ( (unsigned char) 0x04 )
#define SEGd        ( (unsigned char) 0x08 )
#define SEGe        ( (unsigned char) 0x10 )
#define SEGf        ( (unsigned char) 0x20 )
#define SEGg        ( (unsigned char) 0x40 )

const unsigned char tabela_7seg[] = 
{
	SEGa | SEGb | SEGc | SEGd | SEGe | SEGf, // 0
	SEGb | SEGc, // 1
	SEGa | SEGb | SEGe | SEGd | SEGg, // 2
	SEGa | SEGb | SEGc | SEGd | SEGg, // 3
	SEGb | SEGc | SEGf | SEGg, // 4
	SEGa | SEGc | SEGd | SEGf | SEGg, // 5
	SEGa | SEGc | SEGd | SEGe | SEGf | SEGg, // 6
	SEGa | SEGb | SEGc, // 7
	SEGa | SEGb | SEGc | SEGd | SEGe | SEGf | SEGg, // 8
	SEGa | SEGb | SEGc | SEGd | SEGf | SEGg, // 9
};

/******************************************************************************
 * 
 * Variáveis Globais.
 * 
 ******************************************************************************/
unsigned char buf_display[11] = { 0 };
static void (*modoDeOperacao)(void);
static volatile unsigned char flg500ms = 0;
static volatile unsigned char tecla_atual = 0;
static unsigned char faseModo = 0;

/******************************************************************************
 * 
 * Função Principal.
 * 
 ******************************************************************************/
void main( void )
{
	volatile uint16_t i;
	
	BoardConfig( );
	
	for( i=0; i<30000; i++ )
	{
		__nop();
	}
	
	modoDeOperacao = executaModoInicial;
	
	/* Loop Principal */
	for (;; )
	{
		CLRWDT( ); // Idly kick the dog

		( *modoDeOperacao )();
	}
}

inline unsigned char ConvertTo7Seg( unsigned char idx )
{
	if( idx < sizeof(tabela_7seg) )
	{
		return tabela_7seg[ idx ];
	}
	else 
	{
		return(SEGa | SEGd | SEGe | SEGf | SEGg);
	}
}

void UpdateDisplays()
{
	unsigned char temp;

	temp = DS1307_Get( DAY );
	buf_display[ 0 ] = ConvertTo7Seg( (temp >> 4) % 10 );
	buf_display[ 1 ] = ConvertTo7Seg( (temp & 0x0F) % 10 );

	temp = DS1307_Get( MONTH );
	buf_display[ 2 ] = ConvertTo7Seg( temp >> 4 );
	buf_display[ 3 ] = ConvertTo7Seg( temp & 0x0F );

	temp = DS1307_Get( YEAR );
	buf_display[ 4 ] = ConvertTo7Seg( temp >> 4 );
	buf_display[ 5 ] = ConvertTo7Seg( temp & 0x0F );

	temp = DS1307_Get( HOUR );
	buf_display[ 6 ] = ConvertTo7Seg( temp >> 4 );
	buf_display[ 7 ] = ConvertTo7Seg( temp & 0x0F );

	temp = DS1307_Get( MINUTE );
	buf_display[ 8 ] = ConvertTo7Seg( temp >> 4 );
	buf_display[ 9 ] = ConvertTo7Seg( temp & 0x0F );

	temp = DS1307_Get( DAY_OF_WEEK );
	buf_display[ 10 ] = ( unsigned char ) ( 0x80 >> temp );
}

/**
 * Função de configuração Inicial do Hardware.
 */
void BoardConfig( )
{
	PORTA = 0x00;
	PORTB = 0x00;
	PORTB = 0x00;
	PORTE = 0x00;

	/* DIG1..DIG6 - Output */
	TRISA0 = 0;
	TRISA1 = 0;
	TRISA2 = 0;
	TRISA3 = 0;
	TRISA4 = 0;
	TRISA5 = 0;
	
	TRISB0 = 0;
	TRISB1 = 0;
	TRISB2 = 0;
	TRISB4 = 0;

	TRISD = 0x00;				/* Barramento de Dados - Output */
	
	TRISEbits.PSPMODE = 0;
	TRISE0 = 0;
	TRISE1 = 0;
	
	TRISC1 = 1;
	TRISC2 = 1;

//	OPTION_REGbits.T0CS = 0;	/* Timer increments on instruction clock */
//	INTCONbits.T0IE = 1;		/* Enable interrupt on TMR0 overflow */
	
	T1CONbits.T1CKPS0 = 1;		/* Prescaler 1/8 */
	T1CONbits.T1CKPS1 = 1;

	T1CONbits.T1OSCEN = 0;
	T1CONbits.TMR1ON = 1;
	
	TMR1H = 0xFB;
	TMR1L = 0x1D;
	
	PIE1bits.TMR1IE = 1;
	INTCONbits.PEIE = 1;
	INTCONbits.GIE  = 1;		/* Global interrupt enable */

	DS1307_Init( );

}

inline void varreduraDisplay()
{
	static unsigned char idx = 0;
	
	/* Desliga dígitos */
	PORTA &= 0xC0;
	PORTB &= 0xF8;
	PORTE &= 0xFC;

	/* Atualiza Porta */
	PORTD = buf_display[ idx ];

	/* Liga o Dígito Atual */
	if ( idx > 8 )
	{
		PORTE |= ( ( unsigned char ) 0x01 << ( idx - 9 ) );
	}
	else if ( idx > 5 )
	{
		PORTB |= ( ( unsigned char ) 0x01 << ( idx - 6 ) );
	}
	else
	{
		PORTA |= ( ( unsigned char ) 0x01 << idx );
	}

	if ( ++idx > 10 )
		idx = 0;
}

/*----------------------------------------------------------------------------*/

inline void trataTeclado()
{
	static unsigned char tecCount = 0;
	static uint16_t burstCount = 0;
	
	switch( tecCount )
	{
		case 0:
		{
			burstCount = 0;

			if( KEY_INC_pin == 0 )
			{
				tecla_atual = TECLA_INC;
				tecCount++ ;
			}
			else if( KEY_SEL_pin == 0 )
			{
				tecla_atual = TECLA_SEL;
				tecCount++ ;
			}
		}
		break;
		
		case 100:
		{
			tecCount = 0;
		}
		break;
		
		case 110:
		{
			if( KEY_INC_pin == 0 )
			{
				burstCount++ ;
				
				if( burstCount > 100 )
				{
					tecla_atual = TECLA_INC;
					burstCount = 0;
				}
			}
			else
			{
				tecCount = 0;
			}
		}
		break;
		
		default:
		{
			if( KEY_INC_pin == 0 || KEY_SEL_pin == 0 )
			{
				tecCount = 1 ;
				
				burstCount++ ;
				
				if( burstCount > 1000 )
				{
					tecCount = 110;
				}
			}
			else
			{
				tecCount++;
			}
		}
		break;
		
	}
	
}

/*----------------------------------------------------------------------------*/

/**
 * Função de Interrupção.
 * Deve rodar a cada 2ms.
 */
void interrupt tc_int( void )
{
	/* if timer flag is set & interrupt enabled */
	if( PIR1bits.TMR1IF && PIE1bits.TMR1IE )
	{
		static uint16_t temp = 0;
		
		if( temp > 0 ) 
		{
			temp--;
		}
		else
		{
			flg500ms = 1;
			temp = 500;
		}
		
		TMR1H = TIME_RELOAD >> 8;
		TMR1L = TIME_RELOAD & 0xFF;
		
		varreduraDisplay();
		trataTeclado();
		
		PIR1bits.TMR1IF = 0;
		
	}
}

/*----------------------------------------------------------------------------*/

void executaModoNormal()
{
	if( tecla_atual == TECLA_SEL )
	{
		tecla_atual = 0;
		
		faseModo = 0;
		modoDeOperacao = executaModoAjuste;
	}
	
	if( flg500ms )
	{
		flg500ms = 0;

		DS1307_Read( );
		UpdateDisplays( );

		if( !DS1307_isPaused() )
		{
			RB4 = ~RB4;
		}
	}
}

/*----------------------------------------------------------------------------*/

void executaModoAjuste()
{
	static unsigned char localCount = 0;
	
	if( tecla_atual == TECLA_SEL )
	{
		tecla_atual = 0;
		
		localCount = 0;
		faseModo++;
		UpdateDisplays( );
		
		if( faseModo > 5 )
		{
			DS1307_Set( SECOND, 0 );
			DS1307_Write();
			
			flg500ms = 0;
			while(flg500ms==0);			
			flg500ms = 0;
			while(flg500ms==0);
			
			//DS1307_WriteInt32( 0, MAGIC_NUMBER );
			DS1307_WriteMem( 0, "\x5A", 1 );
			
			faseModo = 0;
			modoDeOperacao = executaModoInicial;
			return;
		}
	}
	
	else if( tecla_atual == TECLA_INC )
	{
		tecla_atual = 0;
		
		switch( faseModo )
		{
			case 0:	/* Dia */
			{
				unsigned char temp;
				
				temp = DS1307_Get( DAY );
				
				temp = ( temp & 0x0F ) + (( (temp>>4) & 0x0F )*10);
				
				if( temp < 31 )
					temp++;
				else
					temp = 1;
				
				DS1307_Set( DAY, temp );
				
			}
			break;
			
			case 1:	/* Mês */
			{
				unsigned char temp;
				
				temp = DS1307_Get( MONTH );
				
				temp = ( temp & 0x0F ) + (( (temp>>4) & 0x0F )*10);
				
				if( temp < 12 )
					temp++;
				else
					temp = 1;
				
				DS1307_Set( MONTH, temp );
				
			}
			break;
			
			case 2:	/* Ano */
			{
				unsigned char temp;
				
				temp = DS1307_Get( YEAR );
				
				temp = ( temp & 0x0F ) + (( (temp>>4) & 0x0F )*10);
				
				if( temp < 99 )
					temp++;
				else
					temp = 0;
				
				DS1307_Set( YEAR, temp );
				
			}
			break;
			
			case 3:	/* Hora */
			{
				unsigned char temp;
				
				temp = DS1307_Get( HOUR );
				
				temp = ( temp & 0x0F ) + (( (temp>>4) & 0x0F )*10);
				
				if( temp < 23 )
					temp++;
				else
					temp = 0;
				
				DS1307_Set( HOUR, temp );
				
			}
			break;
			
			case 4:	/* Minuto */
			{
				unsigned char temp;
				
				temp = DS1307_Get( MINUTE );
				
				temp = ( temp & 0x0F ) + (( (temp>>4) & 0x0F )*10);
				
				if( temp < 59 )
					temp++;
				else
					temp = 0;
				
				DS1307_Set( MINUTE, temp );
				
			}
			break;
			
			case 5:	/* Dia da Semana */
			{
				unsigned char temp;
				
				temp = DS1307_Get( DAY_OF_WEEK );
				
				temp = ( temp & 0x0F ) + (( (temp>>4) & 0x0F )*10);
				
				if( temp < 7 )
					temp++;
				else
					temp = 1;
				
				DS1307_Set( DAY_OF_WEEK, temp );
				
			}
			break;
			
		}

		UpdateDisplays( );
		
		flg500ms = 0;  /* Cancela 1 piscada */
		localCount = 2;
		
	}
	
	if( flg500ms )
	{
		flg500ms = 0;
		

		if( buf_display[ faseModo * 2 ] == 0x00 )
		{
			UpdateDisplays( );
		}
		else if( localCount == 0 )
		{
			buf_display[ faseModo * 2 ] = 0x00;
			if( faseModo < 5 )
				buf_display[ faseModo * 2 + 1 ] = 0x00;
		}
		
		if( localCount > 0 )
		{
			localCount--;
		}

		RB4 = 0;
	}
}

/*----------------------------------------------------------------------------*/

void executaModoInicial()
{
	unsigned char j;
	unsigned char magic;
	
	/* Cende todos os displays */
	PIE1bits.TMR1IE = 0;
	for( j=0; j<11; j++ )
	{
		buf_display[ j ] = 0x00;
	}
	PIE1bits.TMR1IE = 1;
	RB4 = 0;
	
	DS1307_Read();
	
	magic = DS1307_ReadMem(0);
	
	if( DS1307_isWrong() || magic != 0x5A )
	{
		DS1307_Set( DAY, 9 );
		DS1307_Set( MONTH, 6 );
		DS1307_Set( YEAR, 16 );
		DS1307_Set( HOUR, 23 );
		DS1307_Set( MINUTE, 59 );
		DS1307_Set( SECOND, 0 );
		DS1307_Write();
		DS1307_Set( DAY_OF_WEEK, 1 );
		faseModo = 0;
		modoDeOperacao = executaModoAjuste;
	}
//	else if( DS1307_isPaused() )
//	{
////		DS1307_Set( DAY, 9 );
////		DS1307_Set( MONTH, 6 );
////		DS1307_Set( YEAR, 16 );
////		DS1307_Set( HOUR, 23 );
////		DS1307_Set( MINUTE, 59 );
//		DS1307_Set( SECOND, 0 );
//		DS1307_Write();
////		DS1307_Set( DAY_OF_WEEK, 1 );
//		faseModo = 0;
//		modoDeOperacao = executaModoNormal;
//	}
	else
	{
		modoDeOperacao = executaModoNormal;
	}
}

/*----------------------------------------------------------------------------*/

