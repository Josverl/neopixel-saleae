#include "NeopixelAnalyzerResults.h"
#include <AnalyzerHelpers.h>
#include "NeopixelAnalyzer.h"
#include "NeopixelAnalyzerSettings.h"
#include <iostream>
#include <fstream>

NeopixelAnalyzerResults::NeopixelAnalyzerResults( NeopixelAnalyzer* analyzer, NeopixelAnalyzerSettings* settings )
:	AnalyzerResults(),
	mSettings( settings ),
	mAnalyzer( analyzer )
{
}

NeopixelAnalyzerResults::~NeopixelAnalyzerResults()
{
}

void NeopixelAnalyzerResults::GenerateBubbleText( U64 frame_index, Channel& channel, DisplayBase display_base )
{
	ClearResultStrings();
	Frame frame = GetFrame( frame_index );
	

	char number_str[12];
	char counter_str[12];
	char color_str[2] = "X";

	// mData1 = led value 
	// mData2 = neopixelcount
	// mtype = RGBW char 
	AnalyzerHelpers::GetNumberString(frame.mData1, display_base, 8, number_str, 12); 
	AnalyzerHelpers::GetNumberString(frame.mData2, display_base, 8, counter_str, 12);
	color_str[0] = char(frame.mType);
	// add a few different lengths 
	AddResultString(color_str);
	AddResultString( number_str);
	AddResultString(color_str, "=",number_str);
	AddResultString(counter_str,":" ,color_str, "=", number_str);
	AddResultString("Pixel ", counter_str, " :", color_str, "=", number_str);
}

void NeopixelAnalyzerResults::GenerateExportFile( const char* file, DisplayBase display_base, U32 export_type_user_id )
{
	std::ofstream file_stream( file, std::ios::out );

	U64 trigger_sample = mAnalyzer->GetTriggerSample();
	U32 sample_rate = mAnalyzer->GetSampleRate();

	file_stream << "Time [s],Value" << std::endl;

	U64 num_frames = GetNumFrames();
	for( U32 i=0; i < num_frames; i++ )
	{
		Frame frame = GetFrame( i );
		
		char time_str[128];
		AnalyzerHelpers::GetTimeString( frame.mStartingSampleInclusive, trigger_sample, sample_rate, time_str, 128 );

		char number_str[128];
		AnalyzerHelpers::GetNumberString( frame.mData1, display_base, 8, number_str, 128 );

		file_stream << time_str << "," << number_str << std::endl;

		if( UpdateExportProgressAndCheckForCancel( i, num_frames ) == true )
		{
			file_stream.close();
			return;
		}
	}

	file_stream.close();
}

void NeopixelAnalyzerResults::GenerateFrameTabularText( U64 frame_index, DisplayBase display_base )
{
	Frame frame = GetFrame( frame_index );
	ClearResultStrings();

	char number_str[128];
	// led data in mData1
	AnalyzerHelpers::GetNumberString(frame.mData1, display_base, 8, number_str, 128);
	AddResultString( number_str );
}

void NeopixelAnalyzerResults::GeneratePacketTabularText( U64 packet_id, DisplayBase display_base )
{
	ClearResultStrings();
	AddResultString( "not supported" );
}

void NeopixelAnalyzerResults::GenerateTransactionTabularText( U64 transaction_id, DisplayBase display_base )
{
	ClearResultStrings();
	AddResultString( "not supported" );
}
