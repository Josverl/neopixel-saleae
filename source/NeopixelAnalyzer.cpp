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
	long byteCounter = 0;
	int rgbwCounter = 0;
	char strColors[] = "GRB\0";				// Assume : todo Add AS AN OPTION GRB - Defaul - GRBW 
	int MAX_RGBW = strlen(strColors);			

	mResults.reset( new NeopixelAnalyzerResults( this, mSettings.get() ) );
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


		// todo: mark each Color Sequence as a 
		//let's put a dot exactly where we sample this byte
		U64 starting_sample = mSerial->GetSampleNumber();
		mResults->AddMarker( starting_sample, AnalyzerResults::Dot, mSettings->mInputChannel );

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
		frame.mData2 = strColors[rgbwCounter++];
		if (rgbwCounter >= MAX_RGBW) {
			rgbwCounter = 0;
		}
		frame.mFlags = 0;
		frame.mStartingSampleInclusive = starting_sample;
		frame.mEndingSampleInclusive = std::min(last_rising_sample + samples_per_bit, mSerial->GetSampleNumber());


		mResults->AddFrame( frame );
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
	return "Neopixel";
}

const char* GetAnalyzerName()
{
	return "Neopixel";
}

Analyzer* CreateAnalyzer()
{
	return new NeopixelAnalyzer();
}

void DestroyAnalyzer( Analyzer* analyzer )
{
	delete analyzer;
}
