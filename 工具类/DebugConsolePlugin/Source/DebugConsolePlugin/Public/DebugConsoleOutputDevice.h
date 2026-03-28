// DebugConsoleOutputDevice.h
#pragma once

#include "CoreMinimal.h"
#include "Misc/OutputDevice.h"
#include "UObject/UObjectGlobals.h"

class DEBUGCONSOLEPLUGIN_API FDebugConsoleOutputDevice : public FOutputDevice
{
public:
    FDebugConsoleOutputDevice();
    virtual ~FDebugConsoleOutputDevice();
    
    // FOutputDevice interface
    virtual void Serialize(const TCHAR* V, ELogVerbosity::Type Verbosity, const FName& Category) override;
    virtual void Serialize(const TCHAR* V, ELogVerbosity::Type Verbosity, const FName& Category, const double Time) override;
    virtual bool CanBeUsedOnAnyThread() const override { return true; }
    
private:
    bool ShouldDisplayLog(ELogVerbosity::Type Verbosity, const FName& Category) const;
    FString FormatLogMessage(const TCHAR* V, ELogVerbosity::Type Verbosity, const FName& Category) const;
    const TCHAR* GetVerbosityString(ELogVerbosity::Type Verbosity) const;
    void SetConsoleColorForVerbosity(ELogVerbosity::Type Verbosity) const;
    void ResetConsoleColor() const;
    
    FCriticalSection CriticalSection;
};
