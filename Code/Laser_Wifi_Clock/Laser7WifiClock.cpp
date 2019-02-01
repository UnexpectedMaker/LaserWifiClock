// ---------------------------------------------------------------------------
// Created by Seon Rozenblum - seon@unexpectedmaker.com
// Copyright 2016 License: GNU GPL v3 http://www.gnu.org/licenses/gpl-3.0.html
//
// See "Laser7WifiClock.h" for purpose, syntax, version history, links, and more.
// ---------------------------------------------------------------------------

#include "Laser7WifiClock.h"
// This library is 100% entwined with the Adafruit NeoPixel library - please check their licensing terms in their library.
#include <Adafruit_NeoPixel.h>

// #define DEBUG
#define NUM_PIXELS_PER_BOARD 7

#define ELEMENTS(x)   (sizeof(x) / sizeof(x[0]))

// Array of pixels per segment, 7 segments with 4 pixels each
  int segmentsPixels[14][5] 	{ { 0, 1, 2, 3, 4 }, 
                                { 5, 6, 7, 8, 9 }, 
                                { 10, 11, 12, 13, 14 }, 
                                { 15, 16, 17, 18, 19 }, 
                                { 20, 21, 22, 23, 24 }, 
                                { 25, 26, 27, 28, 29 }, 
                                { 30, 31, 32, 33, 34 },
                							  { 35, 36, 37, -1, -1 },
                							  { 38, 39, 40, -1, -1 },
                							  { 41, 42, 43, -1, -1 },
                							  { 44, 45, 46, -1, -1 },
                							  { 47, 48, 49, 50, 51 },
                							  { 52, 53, 54, 55, 56 },
                							  { 57, -1, -1, -1, -1}
                						  };

byte pixelsXY[7][2]		        { {1,0}, {2,1}, {2,3}, {1,4}, {0, 3}, {0,1}, {1,2 } };

					  
// Array of segment based rainbow colour values
uint32_t segmentRainbow[7][3] {
                              { 255,0,0 },
                              { 255,124,42 },
                              { 255,255,0 },
                              { 69,223,91 },
                              { 31,189,255 },
                              { 150,11,255 },
                              { 255,30,237 }
                            };

// Available characters a 7 Segment display can show					  
const byte ARRAY_SIZE = 34;
						
CharacterX characters_pixel[ ARRAY_SIZE ] {
                							{ '0', 0b00111111 }, 
                							{ '1', 0b00000110 },
                							{ '2', 0b01011011 },
                							{ '3', 0b01001111 },
                							{ '4', 0b01100110 },
                							{ '5', 0b01101101 },
                							{ '6', 0b01111101 },
                							{ '7', 0b00000111 },
                							{ '8', 0b01111111 },
                							{ '9', 0b01101111 },
                							{ 'a', 0b01110111 },
                							{ 'b', 0b01111111 },
                							{ 'c', 0b00111001 },
                							{ 'd', 0b00111111 },
                							{ 'e', 0b01111001 },
                							{ 'f', 0b01110001 },
                							{ 'g', 0b01100111 },
                							{ 'h', 0b01110110 },
                							{ 'i', 0b00110000 },
                							{ 'j', 0b00011110 },
                							{ 'l', 0b00111000 },
                							{ 'n', 0b00110111 },
                							{ 'o', 0b00111111 },
                							{ 'p', 0b01110011 },
                							{ 'q', 0b01100111 },
                							{ 'r', 0b00110001 },
                							{ 's', 0b01101101 },
                							{ 'u', 0b00111110 },
                							{ 'x', 0b01110110 },
                							{ 'y', 0b01101110 },
                							{ '-', 0b01000000 },
                							{ '[', 0b00111001 },
                							{ ']', 0b00001111 },
                							{ ' ', 0b00000000 }
                            };

											
Laser7WifiClock::Laser7WifiClock( uint8_t displayCount, uint8_t dPin, uint8_t extraPixels )
{
	dispCount = displayCount;
	dispPin = dPin;
	pixels = Adafruit_NeoPixel ();
	pixels.updateType( NEO_GRB + NEO_KHZ800 );
  pixels.updateLength( dispCount * NUM_PIXELS_PER_BOARD + extraPixels );
  pixels.setPin(dispPin);
	isReady = false;
}

Laser7WifiClock::~Laser7WifiClock()
{
	isReady = false;
}

bool Laser7WifiClock::IsReady()
{
	return isReady;
}

void Laser7WifiClock::Begin( uint8_t brightness )
{
	pixels.begin(); // This initializes the NeoPixel library.
	pixels.show();
	pixels.setBrightness( brightness );

	cachedString = "";
	cachedCharacters = ( CharacterX *) malloc(dispCount * sizeof(CharacterX) );
	cachedBytes = ( uint8_t *) malloc(dispCount * sizeof(uint8_t) );
	
	// Set all cached bytes to 0
	for ( int i = 0; i < dispCount; i++ )
		cachedBytes[i] = 0;

	//Digits are initialised and ready
	isReady = true;
}

void Laser7WifiClock::ClearAll()
{
	if ( isReady )
	{
		for ( int p = 0; p < 30; p++ )
			pixels.setPixelColor( p, Color(0,0,0) );

		pixels.show();
	}
}


void Laser7WifiClock::Clear( uint8_t display )
{
	if ( isReady )
	{
		for ( int p = 0; p < NUM_PIXELS_PER_BOARD; p++ )
			pixels.setPixelColor( p + ( display * NUM_PIXELS_PER_BOARD ), Color(0,0,0) );

		pixels.show();
	}
}

void Laser7WifiClock::SetBrightness( uint8_t brightness )
{
	if ( isReady )
	{
		pixels.setBrightness( brightness );
		pixels.show();
	}
}


byte Laser7WifiClock::GetArraySize()
{
	return ARRAY_SIZE;
}


CharacterX Laser7WifiClock::FindCharacterX( byte c )
{
	for ( int i = 0; i < ELEMENTS( characters_pixel ); i++ )
	{
		if ( characters_pixel[i].id == c )
			return characters_pixel[i];
	}

	return characters_pixel[0];
}

String Laser7WifiClock::GetCharacterAtArrayIndex( int index )
{
	return ( (String)(char)characters_pixel[ index ].id );
}

// Convert separate R,G,B into packed 32-bit RGB color.
// Packed format is always RGB, regardless of LED strand color order.
uint32_t Laser7WifiClock::Color(uint8_t r, uint8_t g, uint8_t b)
{
  return ((uint32_t)r << 16) | ((uint32_t)g <<  8) | b;
}

void Laser7WifiClock::CheckToCacheBytes( String str )
{
  // We only re-cache the string if it is different to the cached one, so spamming the same string doesn't keep allocating on the heap
	if ( str != cachedString )
	{
		cachedString = str;
		
		int index = 0;
		for ( int s = 0; s < str.length(); s++ )
		{
      // Caching the character and pixel data so no heap allocations if the values dont change
			cachedCharacters[index] = FindCharacterX( str.charAt(s) );
			cachedBytes[index] = cachedCharacters[index].pixels;
			index++;

		}
	}
}


void Laser7WifiClock::DisplayTextColorCycle( String text, uint8_t index )
{
	if ( !isReady )
		return;
		
	CheckToCacheBytes( text );	

	// Clamp the length, so text longer than the display count is ignored
	int lengthOfLoop = min( dispCount, (uint8_t)text.length() );

	// Grab the byte (bits) for the segmens for the character passed in
	for ( int s = 0; s < lengthOfLoop; s++ )
	{
		int colorStart = index;
		for ( int i = 0; i < 63; i++ )
		{
			// twirly the colours
			uint32_t color = Wheel( colorStart & 255 );
			colorStart+=(255/28);

			bool on = ( bitRead( cachedBytes[s], i) == 1 );
			pixels.setPixelColor( i + ( s * NUM_PIXELS_PER_BOARD ), on ? color : Color(0,0,0) );

		}
	}
		
	// we have finished setting all of the colors on each segment for this Laser7WifiClock, so lets turn on the pixels
	pixels.show();
}


void Laser7WifiClock::DisplayError( uint32_t color )
{
  if ( !isReady )
    return;

  String text = "1200";
    
  CheckToCacheBytes( text);
  
  // Clamp the length, so text longer than the display count is ignored
  int lengthOfLoop = min( dispCount, (uint8_t)text.length() );

  // Grab the byte (bits) for the segmens for the character passed in
  for ( int s = 0; s < lengthOfLoop; s++ )
  {
    for ( int i = 0; i < NUM_PIXELS_PER_BOARD; i++ )
    {
      bool on = ( bitRead( cachedBytes[s], i) == 1 );
      pixels.setPixelColor( i + ( s * NUM_PIXELS_PER_BOARD ), on ? color : Color(0,0,0) );
    }
  }

  pixels.setPixelColor( 28, color );
  pixels.setPixelColor( 29, color );
    
  // we have finished setting all of the colors on each segment for this Laser7WifiClock, so lets turn on the pixels
  pixels.show();
}

void Laser7WifiClock::DisplayTextColor( String text, uint32_t color )
{
	if ( !isReady )
		return;
		
	CheckToCacheBytes( text );
	
	// Clamp the length, so text longer than the display count is ignored
	int lengthOfLoop = min( dispCount, (uint8_t)text.length() );

	// Grab the byte (bits) for the segmens for the character passed in
	for ( int s = 0; s < lengthOfLoop; s++ )
	{
		for ( int i = 0; i < NUM_PIXELS_PER_BOARD; i++ )
		{
			bool on = ( bitRead( cachedBytes[s], i) == 1 );
			pixels.setPixelColor( i + ( s * NUM_PIXELS_PER_BOARD ), on ? color : Color(0,0,0) );
		}
	}
	// we have finished setting all of the colors on each segment for this Laser7WifiClock, so lets turn on the pixels
	pixels.show();
}


void Laser7WifiClock::DisplayTime( uint8_t hours, uint8_t mins, uint8_t secs, uint32_t color )
{
	if ( !isReady )
		return;
		
	String text = SpaceTimeData( hours ) + PadTimeData( mins );


	CheckToCacheBytes( text );
	
	// Clamp the length, so text longer than the display count is ignored
	int lengthOfLoop = min( dispCount, (uint8_t)text.length() );

	// Grab the byte (bits) for the segmens for the character passed in
	for ( int s = 0; s < lengthOfLoop; s++ )
	{
		for ( int i = 0; i < NUM_PIXELS_PER_BOARD; i++ )
		{
			bool on = ( bitRead( cachedBytes[s], i) == 1 );
			pixels.setPixelColor( i + ( s * NUM_PIXELS_PER_BOARD ), on ? color : Color(0,0,0) );
		}
	}

	uint32_t dpColor = (( secs % 2 == 0 ) ? Color(0,0,0 ) : Color( 255,0,0) );
  	pixels.setPixelColor( 28, dpColor );
  	pixels.setPixelColor( 29, dpColor );
  

	// we have finished setting all of the colors on each segment for this Laser7WifiClock, so lets turn on the pixels
	pixels.show();
}

void Laser7WifiClock::DisplayTime( uint8_t hours, uint8_t mins, uint8_t secs, uint32_t color, uint32_t colon )
{
  if ( !isReady )
    return;
    
  String text = SpaceTimeData( hours ) + PadTimeData( mins );


  CheckToCacheBytes( text );
  
  // Clamp the length, so text longer than the display count is ignored
  int lengthOfLoop = min( dispCount, (uint8_t)text.length() );

  // Grab the byte (bits) for the segmens for the character passed in
  for ( int s = 0; s < lengthOfLoop; s++ )
  {
    for ( int i = 0; i < NUM_PIXELS_PER_BOARD; i++ )
    {
      bool on = ( bitRead( cachedBytes[s], i) == 1 );
      pixels.setPixelColor( i + ( s * NUM_PIXELS_PER_BOARD ), on ? color : Color(0,0,0) );
    }
  }

    uint32_t dpColor = (( secs % 2 == 0 ) ? Color(0,0,0 ) : colon );
    pixels.setPixelColor( 28, dpColor );
    pixels.setPixelColor( 29, dpColor );
  

  // we have finished setting all of the colors on each segment for this Laser7WifiClock, so lets turn on the pixels
  pixels.show();
}

void Laser7WifiClock::DisplayTimeRandom( uint8_t hours, uint8_t mins, uint8_t secs )
{
	if ( !isReady )
		return;
		
	String text = SpaceTimeData( hours ) + PadTimeData( mins );

	CheckToCacheBytes( text );

	int colorStart = random( 1, 255 );
	//int inc = random( 32,66 );
	uint32_t color = Wheel(colorStart & 255 );
	
	// Clamp the length, so text longer than the display count is ignored
	int lengthOfLoop = min( dispCount, (uint8_t)text.length() );

	// Grab the byte (bits) for the segmens for the character passed in
	for ( int s = 0; s < lengthOfLoop; s++ )
	{
		for ( int i = 0; i < NUM_PIXELS_PER_BOARD; i++ )
		{
			bool on = ( bitRead( cachedBytes[s], i) == 1 );
			pixels.setPixelColor( i + ( s * NUM_PIXELS_PER_BOARD ), on ? color : Color(0,0,0) );
		}
	}

	uint32_t dpColor = (( secs % 2 == 0 ) ? Color(0,0,0 ) : color );
  	pixels.setPixelColor( 28, dpColor );
  	pixels.setPixelColor( 29, dpColor );
  

	// we have finished setting all of the colors on each segment for this Laser7WifiClock, so lets turn on the pixels
	pixels.show();
}

void Laser7WifiClock::DisplayTimeCycleH( uint8_t hours, uint8_t mins, uint8_t secs, uint8_t index, uint8_t dir  )
{
	if ( !isReady )
		return;
		
	String text = SpaceTimeData( hours ) + PadTimeData( mins );


	CheckToCacheBytes( text );
	
	// Clamp the length, so text longer than the display count is ignored
	int lengthOfLoop = min( dispCount, (uint8_t)text.length() );

	int colorStart = index;
	// Grab the byte (bits) for the segmens for the character passed in
	for ( int s = 0; s < lengthOfLoop; s++ )
	{
		// twirly the colours
		uint32_t color = Wheel( colorStart & 255 );
		colorStart+=( (255/14) * dir );

		for ( int i = 0; i < NUM_PIXELS_PER_BOARD; i++ )
		{
			bool on = ( bitRead( cachedBytes[s], i) == 1 );
			pixels.setPixelColor( i + ( s * NUM_PIXELS_PER_BOARD ), on ? color : Color(0,0,0) );
		}
	}

	uint32_t dpColor = (( secs % 2 == 0 ) ? Color(0,0,0 ) : Wheel( colorStart & 255 ) );
  	pixels.setPixelColor( 28, dpColor );
  	pixels.setPixelColor( 29, dpColor );
  

	// we have finished setting all of the colors on each segment for this Laser7WifiClock, so lets turn on the pixels
	pixels.show();
}

void Laser7WifiClock::DisplayTimeCycleV( uint8_t hours, uint8_t mins, uint8_t secs, uint8_t index, uint8_t dir  )
{
	if ( !isReady )
		return;
		
	String text = SpaceTimeData( hours ) + PadTimeData( mins );


	CheckToCacheBytes( text );
	
	// Clamp the length, so text longer than the display count is ignored
	int lengthOfLoop = min( dispCount, (uint8_t)text.length() );

	int colorStart = index;

	uint32_t colorA = Wheel( colorStart & 255 );
	colorStart+=( 65 * dir );
	uint32_t colorB = Wheel( colorStart & 255 );

	// Grab the byte (bits) for the segmens for the character passed in
	for ( int s = 0; s < lengthOfLoop; s++ )
	{
		for ( int i = 0; i < 7; i++ )
		{
			int y = pixelsXY[ i ][1];

			uint8_t red = ((Red(colorA) * (5 - y)) + (Red(colorB) * y)) * 0.2;
			uint8_t green = ((Green(colorA) * (5 - y)) + (Green(colorB) * y)) * 0.2;
			uint8_t blue = ((Blue(colorA) * (5 - y)) + (Blue(colorB) * y)) * 0.2;

			uint32_t color = Color(red, green, blue );

		// for ( int i = 0; i < NUM_PIXELS_PER_BOARD; i++ )
		// 	{
				bool on = ( bitRead( cachedBytes[s], i) == 1 );
				pixels.setPixelColor( i + ( s * NUM_PIXELS_PER_BOARD ), on ? color : Color(0,0,0) );
			// }
		}
	}

	uint32_t dpColor = (( secs % 2 == 0 ) ? Color(0,0,0 ) : Wheel( colorStart & 255 ) );
  	pixels.setPixelColor( 28, dpColor );
  	pixels.setPixelColor( 29, dpColor );
  

	// we have finished setting all of the colors on each segment for this Laser7WifiClock, so lets turn on the pixels
	pixels.show();
}


void Laser7WifiClock::DisplayTimeVerticalRainbow(  uint8_t hours, uint8_t mins, uint8_t secs, uint32_t colorA, uint32_t colorB )
{
	if ( !isReady )
		return;

	String text = SpaceTimeData( hours ) + PadTimeData( mins );

	CheckToCacheBytes( text );	

	// Clamp the length, so text longer than the display count is ignored
	int lengthOfLoop = min( dispCount, (uint8_t)text.length() );

	// Grab the byte (bits) for the segmens for the character passed in
	for ( int s = 0; s < lengthOfLoop; s++ )
	{
		for ( int i = 0; i < 7; i++ )
		{
			int y = pixelsXY[ i ][1];

			uint8_t red = ((Red(colorA) * (5 - y)) + (Red(colorB) * y)) * 0.2;
			uint8_t green = ((Green(colorA) * (5 - y)) + (Green(colorB) * y)) * 0.2;
			uint8_t blue = ((Blue(colorA) * (5 - y)) + (Blue(colorB) * y)) * 0.2;

			uint32_t color = Color(red, green, blue );

			bool on = ( bitRead( cachedBytes[s], i) == 1 );
			pixels.setPixelColor( i + ( s * NUM_PIXELS_PER_BOARD ), on ? color : Color(0,0,0) );

		}
	}
		

	uint32_t dpColor = (( secs % 2 == 0 ) ? Color(0,0,0 ) : colorB );
  	pixels.setPixelColor( 28, dpColor );
  	pixels.setPixelColor( 29, dpColor );

	// we have finished setting all of the colors on each segment for this Laser7WifiClock, so lets turn on the pixels
	pixels.show();
		
}

void Laser7WifiClock::SetPixel( uint8_t pixel, uint32_t color )
{
	pixels.setPixelColor( pixel, color );
	pixels.show();
}


void Laser7WifiClock::SetDigit( uint8_t digit, String text, uint32_t color )
{
	if ( !isReady )
		return;
		
}

void Laser7WifiClock::SetDigitSegment( uint8_t digit, uint8_t segment, uint32_t color )
{
	if ( !isReady )
		return;
	
	if ( digit < 0 || digit > dispCount )
		return;

	if ( segment < 0 || segment > 14 )
		return;

	for ( int p = 0; p < 5; p++ )
	{
		int pixel = segmentsPixels[ segment ][p];
		if ( pixel > -1 )
		{
			pixels.setPixelColor( pixel + ( digit * NUM_PIXELS_PER_BOARD ), color );
		}
	}

	pixels.show();
}

void Laser7WifiClock::SetDigitSegments( uint8_t digit, uint16_t segments, uint32_t color )
{
	if ( !isReady )
		return;
	
	if ( digit < 0 || digit > dispCount )
		return;

	for ( int s = 0; s < 16; s++ )
	{
		bool on = ( bitRead( segments, s) == 1 );

		if ( on )
		{
			for ( int p = 0; p < 5; p++ )
			{
				int pixel = segmentsPixels[ s ][p];
				if ( pixel > -1 )
				{
					pixels.setPixelColor( pixel + ( digit * NUM_PIXELS_PER_BOARD ), color );
				}
			}
		}
	}

	pixels.show();
}

String Laser7WifiClock::PadTimeData( int8_t data )
{
  if( data < 10 )
	return String("0") + String(data);

  return String(data);
}

String Laser7WifiClock::SpaceTimeData( int8_t data )
{
  if( data < 10 )
	return String(" ") + String(data);

  return String(data);
}

String Laser7WifiClock::SpaceToDisplay( String data )
{
	int num = dispCount - (uint8_t)data.length();
	for ( int i = 0; i < num; i++ )
	{
		data = " "+data;
	}
	return data;
}


// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Laser7WifiClock::Wheel(byte WheelPos )
{
	WheelPos = 255 - WheelPos;
	if(WheelPos < 85)
		return Color(255 - WheelPos * 3, 0, WheelPos * 3);

	if(WheelPos < 170)
	{
		WheelPos -= 85;
		return Color(0, WheelPos * 3, 255 - WheelPos * 3);
	}
	
	WheelPos -= 170;
	return Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

uint8_t Laser7WifiClock::Red( uint32_t col )
{
	return col >> 16;
}

uint8_t Laser7WifiClock::Green( uint32_t col )
{
	return col >> 8;
}

uint8_t Laser7WifiClock::Blue( uint32_t col )
{
	return col;
}
