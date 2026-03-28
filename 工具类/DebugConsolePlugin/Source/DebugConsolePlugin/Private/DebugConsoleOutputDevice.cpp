// DebugConsoleOutputDevice.cpp
#include "DebugConsoleOutputDevice.h"
#include "DebugConsoleSettings.h"

#if WITH_DEBUG_CONSOLE && PLATFORM_WINDOWS
#include "Windows/AllowWindowsPlatformTypes.h"
#include <Windows.h>
#include <stdio.h>
#include "Windows/HideWindowsPlatformTypes.h"
#endif

FDebugConsoleOutputDevice::FDebugConsoleOutputDevice()
{
}

FDebugConsoleOutputDevice::~FDebugConsoleOutputDevice()
{
}

void FDebugConsoleOutputDevice::Serialize(const TCHAR* V, ELogVerbosity::Type Verbosity, const FName& Category)
{
    Serialize(V, Verbosity, Category, -1.0);
}

void FDebugConsoleOutputDevice::Serialize(const TCHAR* V, ELogVerbosity::Type Verbosity, const FName& Category, const double Time)
{
#if WITH_DEBUG_CONSOLE && PLATFORM_WINDOWS
    if (!ShouldDisplayLog(Verbosity, Category))
    {
        return;
    }
    
    FScopeLock Lock(&CriticalSection);
    
    SetConsoleColorForVerbosity(Verbosity);
    
    FString FormattedMessage = FormatLogMessage(V, Verbosity, Category);
    printf("%s\n", TCHAR_TO_UTF8(*FormattedMessage));
    
    ResetConsoleColor();
#endif
}

bool FDebugConsoleOutputDevice::ShouldDisplayLog(ELogVerbosity::Type Verbosity, const FName& Category) const
{
#if WITH_DEBUG_CONSOLE && PLATFORM_WINDOWS
    if (!UObjectInitialized())
    {
        return true;
    }
    
    const UDebugConsoleSettings* Settings = UDebugConsoleSettings::Get();
    if (!Settings)
    {
        return true;
    }
    
    // 检查日志级别
    if (Verbosity > Settings->GetMinVerbosityAsLogVerbosity())
    {
        return false;
    }
    
    // 检查类别过滤
    if (Settings->CategoryFilter.Num() > 0)
    {
        if (!Settings->CategoryFilter.Contains(Category))
        {
            return false;
        }
    }
#endif
    return true;
}

FString FDebugConsoleOutputDevice::FormatLogMessage(const TCHAR* V, ELogVerbosity::Type Verbosity, const FName& Category) const
{
    FString Result;
    
#if WITH_DEBUG_CONSOLE && PLATFORM_WINDOWS
    if (!UObjectInitialized())
    {
        Result += V;
        return Result;
    }
    
    const UDebugConsoleSettings* Settings = UDebugConsoleSettings::Get();
    
    // 时间戳
    if (Settings && Settings->bShowTimestamp)
    {
        Result += FString::Printf(TEXT("[%s] "), *FDateTime::Now().ToString(TEXT("%H:%M:%S.%s")));
    }
    
    // 日志级别
    if (Settings && Settings->bShowVerbosity)
    {
        Result += FString::Printf(TEXT("[%s] "), GetVerbosityString(Verbosity));
    }
    
    // 类别
    if (Settings && Settings->bShowCategory)
    {
        Result += FString::Printf(TEXT("[%s] "), *Category.ToString());
    }
    
    // 消息内容
    Result += V;
#else
    Result += V;
#endif
    
    return Result;
}

const TCHAR* FDebugConsoleOutputDevice::GetVerbosityString(ELogVerbosity::Type Verbosity) const
{
    switch (Verbosity)
    {
        case ELogVerbosity::Fatal:       return TEXT("FATAL");
        case ELogVerbosity::Error:       return TEXT("ERROR");
        case ELogVerbosity::Warning:     return TEXT("WARN ");
        case ELogVerbosity::Display:     return TEXT("DISP ");
        case ELogVerbosity::Log:         return TEXT("LOG  ");
        case ELogVerbosity::Verbose:     return TEXT("VERB ");
        case ELogVerbosity::VeryVerbose: return TEXT("VVERB");
        default:                         return TEXT("?????");
    }
}

void FDebugConsoleOutputDevice::SetConsoleColorForVerbosity(ELogVerbosity::Type Verbosity) const
{
#if WITH_DEBUG_CONSOLE && PLATFORM_WINDOWS
    HANDLE hConsole = ::GetStdHandle(STD_OUTPUT_HANDLE);
    WORD color = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE; // Default: white
    
    switch (Verbosity)
    {
        case ELogVerbosity::Fatal:
            color = FOREGROUND_RED | FOREGROUND_INTENSITY | BACKGROUND_RED;
            break;
        case ELogVerbosity::Error:
            color = FOREGROUND_RED | FOREGROUND_INTENSITY;
            break;
        case ELogVerbosity::Warning:
            color = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
            break;
        case ELogVerbosity::Display:
            color = FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
            break;
        case ELogVerbosity::Log:
            color = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
            break;
        case ELogVerbosity::Verbose:
        case ELogVerbosity::VeryVerbose:
            color = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
            break;
    }
    
    ::SetConsoleTextAttribute(hConsole, color);
#endif
}

void FDebugConsoleOutputDevice::ResetConsoleColor() const
{
#if WITH_DEBUG_CONSOLE && PLATFORM_WINDOWS
    HANDLE hConsole = ::GetStdHandle(STD_OUTPUT_HANDLE);
    ::SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
#endif
}
