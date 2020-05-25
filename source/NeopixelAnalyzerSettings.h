#ifndef NEOPIXEL_ANALYZER_SETTINGS
#define NEOPIXEL_ANALYZER_SETTINGS

#include <AnalyzerSettings.h>
#include <AnalyzerTypes.h>

class NeopixelAnalyzerSettings : public AnalyzerSettings
{
public:
	NeopixelAnalyzerSettings();
	virtual ~NeopixelAnalyzerSettings();

	virtual bool SetSettingsFromInterfaces();
	void UpdateInterfacesFromSettings();
	virtual void LoadSettings( const char* settings );
	virtual const char* SaveSettings();

	
	Channel mInputChannel;
	U32 mBitRate;
	double mPixelType; // double , but only used as an index 

protected:
	std::auto_ptr< AnalyzerSettingInterfaceChannel >	mInputChannelInterface;
	std::auto_ptr< AnalyzerSettingInterfaceInteger>		mBitRateInterface;
	std::auto_ptr< AnalyzerSettingInterfaceNumberList >	mPixelTypeInterface;
};

#endif //NEOPIXEL_ANALYZER_SETTINGS
