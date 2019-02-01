// ---------------------------------------------------------------------------
// Laser7WifiClock Library - v1.0.0 - 20/01/2019
//
// AUTHOR/LICENSE:
// Created by Seon Rozenblum - seon@unexpectedmaker.com
// Copyright 2016 License: GNU GPL v3 http://www.gnu.org/licenses/gpl-3.0.html
//
// LINKS:
// Project home: https://www.tindie.com/products/15501/
// Website: http://unexpectedmaker.com
//
// DISCLAIMER:
// This software is furnished "as is", without technical support, and with no 
// warranty, express or implied, as to its usefulness for any purpose.
//
// PURPOSE:
// Seven Segment Library for the Neo7Segment display boards, or for use with strips of NeoPixels arranged as 7 Segment displays
//
// SYNTAX:
//   Laser7WifiClock( digits, pin, extras ) - Initialise the array of displays
//     Parameters:
//		* digits		- The number of digits you will be displaying on
//		* pin			  - Pin speaker is wired to (other wire to ground, be sure to add an inline 100 ohm resistor).
//    * extras    - The extra pixels used for the colon,nthey live outside the digits
//
// HISTORY:
//
// 20/01/2019 v1.0 - Initial release.
//
// ---------------------------------------------------------------------------

#ifndef Laser7WifiClock_h
	#define Laser7WifiClock_h

	#include <Adafruit_NeoPixel.h>

  	#if defined(ARDUINO) && ARDUINO >= 100
    	#include <Arduino.h>
  	#else
    	#include <WProgram.h>
		#include <pins_arduino.h>
	#endif

	#ifdef __AVR__
	  #include <avr/power.h>
	#endif
	

typedef struct 
{
	char id;
	uint8_t pixels;
} CharacterX;

class Laser7WifiClock
{
	public:
		Laser7WifiClock( uint8_t displayCount, uint8_t dPin, uint8_t extraPixels );
		~Laser7WifiClock();
		
		void Begin( uint8_t brightness );
		void SetBrightness( uint8_t brightness );
		void Clear( uint8_t display );
		void ClearAll();
		  

		void DisplayTextHorizontalRainbow( String text, uint32_t colorA, uint32_t colorB );
		void DisplayTextColor( String text, uint32_t color );
		void DisplayTextColorCycle( String text, uint8_t index );
    
    void DisplayError( uint32_t color );
    
		void DisplayTime( uint8_t hours, uint8_t mins, uint8_t secs, uint32_t color );
    void DisplayTime( uint8_t hours, uint8_t mins, uint8_t secs, uint32_t color, uint32_t colon );
		void DisplayTimeRandom( uint8_t hours, uint8_t mins, uint8_t secs );
		void DisplayTimeCycleH( uint8_t hours, uint8_t mins, uint8_t secs, uint8_t index, uint8_t dir );
		void DisplayTimeCycleV( uint8_t hours, uint8_t mins, uint8_t secs, uint8_t index, uint8_t dir );
		void DisplayTimeVerticalRainbow(  uint8_t hours, uint8_t mins, uint8_t secs, uint32_t colorA, uint32_t colorB );
		
		void SetDigit( uint8_t digit, String text, uint32_t color );
		void SetDigitSegment( uint8_t digit, uint8_t segment, uint32_t color );
		void SetDigitSegments( uint8_t digit, uint16_t segments, uint32_t color );

		void SetPixel( uint8_t pixel, uint32_t color );

		String SpaceToDisplay( String data );
		
	  static uint32_t Color(uint8_t r, uint8_t g, uint8_t b);
		
		uint32_t Wheel( byte WheelPos );

		bool IsReady( void );

		byte GetArraySize();
		String GetCharacterAtArrayIndex( int index );

    uint8_t Red( uint32_t col );
    uint8_t Green( uint32_t col );
    uint8_t Blue( uint32_t col );
		
	protected:

		
	private:
		Adafruit_NeoPixel pixels;
		uint8_t dispCount;
		uint8_t dispPin;
		CharacterX FindCharacterX( byte );

		void CheckToCacheBytes( String s );
		String PadTimeData( int8_t data );
		String SpaceTimeData( int8_t data );

		uint8_t *cachedBytes;
		CharacterX *cachedCharacters;
		String cachedString;
		bool isReady;
};



#endif
