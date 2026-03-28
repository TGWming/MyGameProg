// DebugConsoleColors.h
#pragma once

#include "CoreMinimal.h"

#if PLATFORM_WINDOWS
#include "Windows/AllowWindowsPlatformTypes.h"
#include <Windows.h>
#include "Windows/HideWindowsPlatformTypes.h"

namespace DebugConsoleColors
{
	// 事件类别颜色
	constexpr WORD Loading      = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY;  // 青色 - 加载事件
	constexpr WORD Spawn        = FOREGROUND_GREEN | FOREGROUND_INTENSITY;                     // 亮绿 - 生成事件
	constexpr WORD Destroy      = FOREGROUND_RED | FOREGROUND_INTENSITY;                       // 亮红 - 销毁事件
	constexpr WORD Modify       = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;    // 黄色 - 修改事件
	constexpr WORD World        = FOREGROUND_BLUE | FOREGROUND_INTENSITY;                      // 亮蓝 - 世界事件
	constexpr WORD GameFlow     = FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY;     // 品红 - 游戏流程
	constexpr WORD Editor       = FOREGROUND_GREEN | FOREGROUND_BLUE;                          // 暗青 - 编辑器事件
	constexpr WORD System       = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;         // 白色 - 系统事件
	constexpr WORD ActorInfo    = FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;   // 亮青 - Actor信息
	constexpr WORD Highlight    = BACKGROUND_BLUE | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY; // 蓝底白字 - 重要高亮
}

#endif // PLATFORM_WINDOWS
