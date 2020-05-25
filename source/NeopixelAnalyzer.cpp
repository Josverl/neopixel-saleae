#include "NeopixelAnalyzer.h"
#include "NeopixelAnalyzerSettings.h"
#include <AnalyzerChannelData.h>
#include <algorithm>

NeopixelAnalyzer::NeopixelAnalyzer()
:	Analyzer(),  
	mSettings( new NeopixelAnalyzerSettings() ),
	mSimulationInitilized( false )
{
	SetAnalyzerSettings( mSettings.get() );
}

NeopixelAnalyzer::~NeopixelAnalyzer()
{
	KillThread();
}

void NeopixelAnalyzer::WorkerThread()
{
	U64 pixelCounter = 0;					// more than long enough 
	int colorCounter = 0;
	char strColors[] = "abcd\0";				

	// needs to match the sequence number use in setting 
	char *PixelTypes[6] = { "BGR\0","RGB\0", "GRB\0", "BGR\0", "BGRW\0", "RGBW\0" };
	mResults.reset( new NeopixelAnalyzerResults( this, mSettings.get() ) );

	// convert double to int to use as index 
	int i = int(mSettings->mPixelType);

	if (0 <= i && i < 6 ) {
		strcpy_s(strColors, PixelTypes[i] );
	}
	size_t MAX_RGBW = strlen(strColors);

	SetAnalyzerResults( mResults.get() );
	mResults->AddChannelBubblesWillAppearOn( mSettings->mInputChannel );

	mSampleRateHz = GetSampleRate();

	mSerial = GetAnalyzerChannelData( mSettings->mInputChannel );

	//todo: - check timing against specs and flag errors 
	if( mSerial->GetBitState() == BIT_LOW ) {
		// Go to rising edge
		mSerial->AdvanceToNextEdge();
	} else {
		// Go to falling edge
		mSerial->AdvanceToNextEdge();
		// Go to rising edge
		mSerial->AdvanceToNextEdge();
	}

	U32 samples_per_bit = mSampleRateHz / (mSettings->mBitRate * 1000);

	for( ; ; )
	{
		U8 data = 0;
		U8 mask = 1 << 7;
		// frame = 1 byte 
		
		U64 starting_sample = mSerial->GetSampleNumber();
		if (colorCounter == 0) {
			//mark start of a led with a Square
			mResults->AddMarker(starting_sample, AnalyzerResults::Square, mSettings->mInputChannel);
		}
		else {
			// let's put a Dot exactly where we sample this byte
			mResults->AddMarker(starting_sample, AnalyzerResults::Dot, mSettings->mInputChannel);
		}

		U64 last_rising_sample;

		// count 8 bits per color 
		for( U32 i=0; i<8; i++ )
		{
			// At rising edge
			U64 rising_sample = mSerial->GetSampleNumber();
			last_rising_sample = rising_sample;
		
			// Go to falling edge
			mSerial->AdvanceToNextEdge();
			U64 falling_sample = mSerial->GetSampleNumber();
			// todo , check for timing is in bounds , if too long then show error 

			// found bit = 1, add to data 
			if(falling_sample - rising_sample > samples_per_bit / 2) {
				data |= mask;
			}
			mask = mask >> 1;

			// Go to rising edge
			mSerial->AdvanceToNextEdge();
		}

		//we have a byte to save. 
		Frame frame;
		frame.mData1 = data;
		frame.mData2 = pixelCounter;
		// RGBW as Type 
		frame.mType = U8 ( strColors[colorCounter++]) ;
		frame.mFlags = 0;
		frame.mStartingSampleInclusive = starting_sample;
		frame.mEndingSampleInclusive = std::min(last_rising_sample + samples_per_bit, mSerial->GetSampleNumber());
		


		mResults->AddFrame( frame );
		// at end of led 
		if (colorCounter >= MAX_RGBW) {
			// mark end of a led 
			pixelCounter++;
			colorCounter = 0;
			// note that Currently, Packets are only used when exporting data to text/csv. :-( 
			mResults->CommitPacketAndStartNewPacket();
		}

		mResults->CommitResults();
		ReportProgress( mSerial->GetSampleNumber() );
	}
}

bool NeopixelAnalyzer::NeedsRerun()
{
	return false;
}

U32 NeopixelAnalyzer::GenerateSimulationData( U64 minimum_sample_index, U32 device_sample_rate, SimulationChannelDescriptor** simulation_channels )
{
	if( mSimulationInitilized == false )
	{
		mSimulationDataGenerator.Initialize( GetSimulationSampleRate(), mSettings.get() );
		mSimulationInitilized = true;
	}

	return mSimulationDataGenerator.GenerateSimulationData( minimum_sample_index, device_sample_rate, simulation_channels );
}

U32 NeopixelAnalyzer::GetMinimumSampleRateHz()
{
	// The rule of thumb is to require oversampling by x4 
	return mSettings->mBitRate * 4;
}

const char* NeopixelAnalyzer::GetAnalyzerName() const
{

	return "Neopixel IJK";
}

const char* GetAnalyzerName()
{
	return "NeoPixel / WS2812";
}

Analyzer* CreateAnalyzer()
{
	return new NeopixelAnalyzer();
}

void DestroyAnalyzer( Analyzer* analyzer )
{
	delete analyzer;
}
