// Copyright FlowVisualizer Plugin. All Rights Reserved.

#include "FlowVisualizerStyle.h"
#include "Styling/CoreStyle.h"

FSlateFontInfo FlowVisStyle::NodeTitleFont()
{
	return FCoreStyle::GetDefaultFontStyle("Bold", 14);
}

FSlateFontInfo FlowVisStyle::DataValueFont()
{
	return FCoreStyle::GetDefaultFontStyle("Mono", 11);
}

FSlateFontInfo FlowVisStyle::StatusBarFont()
{
	return FCoreStyle::GetDefaultFontStyle("Mono", 10);
}

FSlateFontInfo FlowVisStyle::PipelineTitleFont()
{
	return FCoreStyle::GetDefaultFontStyle("Bold", 12);
}
