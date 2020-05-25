#include "NeopixelAnalyzerSettings.h"
#include <AnalyzerHelpers.h>


NeopixelAnalyzerSettings::NeopixelAnalyzerSettings()
:	mInputChannel( UNDEFINED_CHANNEL ),
	mPixelType(5),
	mBitRate( 800 )

{
	//channel
	mInputChannelInterface.reset( new AnalyzerSettingInterfaceChannel() );
	mInputChannelInterface->SetTitleAndTooltip( "NeoPixel WS2812", "Standard Neopixel" );
	mInputChannelInterface->SetChannel( mInputChannel );


	// pixel type / color order 
	// gleaned info on types from https://github.com/FastLED/FastLED/blob/master/examples/FirstLight/FirstLight.ino

	mPixelTypeInterface.reset(new AnalyzerSettingInterfaceNumberList());
	mPixelTypeInterface->SetTitleAndTooltip("Type", "Specify Color order and count");
	mPixelTypeInterface->AddNumber(0, "BGR", "Most common WS2812, WS2812B, WS2852, and SK6812");
	mPixelTypeInterface->AddNumber(1, "RGB", "such as SM16703, TM18xx, UCS19xx, GS19xx");
	mPixelTypeInterface->AddNumber(2, "GRB", "such as LPD6803, and LPD6806");
	mPixelTypeInterface->AddNumber(3, "BGR", "such as P9813, DOTSTAR, APA102 and SK9822");
	mPixelTypeInterface->AddNumber(4, "BGRW", "BGR and White, perhaps they exist");
	mPixelTypeInterface->AddNumber(5, "RGBW", "NeoPixel RGBW, Four Color");
	mPixelTypeInterface->SetNumber(mPixelType);
	
	// bitrate
	mBitRateInterface.reset(new AnalyzerSettingInterfaceInteger());
	mBitRateInterface->SetTitleAndTooltip("Bit Rate (kBits/S)", "Specify the bit rate in kilobits per second.");
	mBitRateInterface->SetMax(800);
	mBitRateInterface->SetMin(400);
	mBitRateInterface->SetInteger(mBitRate);
	
	// build the ux 
	AddInterface( mInputChannelInterface.get() );
	AddInterface(mPixelTypeInterface.get());
	AddInterface( mBitRateInterface.get() );

	//export options
	AddExportOption( 0, "Export as text/csv file" );
	AddExportExtension( 0, "text", "txt" );
	AddExportExtension( 0, "csv", "csv" );

	ClearChannels();
	AddChannel( mInputChannel, "NeoPixel", false );
}

NeopixelAnalyzerSettings::~NeopixelAnalyzerSettings()
{
}

bool NeopixelAnalyzerSettings::SetSettingsFromInterfaces()
{
	mInputChannel = mInputChannelInterface->GetChannel();
	mBitRate = mBitRateInterface->GetInteger();
	mPixelType = mPixelTypeInterface->GetNumber();

	ClearChannels();
		
	// todo: Cleanup Quick and Dirty 
	// needs to match the sequence number use in setting 
	char *PixelTypes[6] = { "BGR\0","RGB\0", "GRB\0", "BGR\0", "BGRW\0", "RGBW\0" };
	// convert double to int to use as index 
	int i = int(mPixelType);

	// here we can display the subtype 
	AddChannel( mInputChannel, PixelTypes[i], true );

	return true;
}

void NeopixelAnalyzerSettings::UpdateInterfacesFromSettings()
{
	mInputChannelInterface->SetChannel( mInputChannel );
	mBitRateInterface->SetInteger( mBitRate );
	mPixelTypeInterface->SetNumber(mPixelType);

}

void NeopixelAnalyzerSettings::LoadSettings( const char* settings )
{
	SimpleArchive text_archive;
	text_archive.SetString( settings );

	text_archive >> mInputChannel;
	text_archive >> mBitRate;
	text_archive >> mPixelType;

	ClearChannels();
	AddChannel( mInputChannel, "Neopixel", true );

	UpdateInterfacesFromSettings();
}

const char* NeopixelAnalyzerSettings::SaveSettings()
{
	SimpleArchive text_archive;

	text_archive << mInputChannel;
	text_archive << mBitRate;
	text_archive << mPixelType;

	return SetReturnString( text_archive.GetString() );
}
