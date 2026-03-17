// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "CameraModifierParameterData.generated.h"

/**
 * 鐩告満淇敼鍣ㄥ弬鏁版暟鎹〃琛岀粨鏋勪綋
 * Camera Modifier Parameter Data Table Row Structure
 * 
 * 鐢ㄤ簬浠嶤SV瀵煎叆鐩告満淇敼鍣ㄥ弬鏁伴厤缃暟鎹?
 * Used for importing camera modifier parameter configuration data from CSV
 * 
 * CSV鏍煎紡璇存槑锛?
 * - ParamID: 鍙傛暟鍞竴鏍囪瘑绗︼紙浼氭垚涓篟owName锛岀粨鏋勪綋涓繚鐣欎絾瀵煎叆鍚庝负绌猴級
 *   鏍煎紡锛歁S寮€澶?Shake绫伙紝MR寮€澶?Reaction绫伙紝MC寮€澶?Cinematic绫伙紝
 *         MZ寮€澶?Zoom绫伙紝ME寮€澶?Effect绫伙紝MX寮€澶?Special绫?
 * - ParamName: 鍙傛暟鍚嶇О锛岀▼搴忓紩鐢ㄦ爣璇嗙
 * - ChineseName: 涓枃鏄剧ず鍚嶇О
 * - Category: 鍙傛暟鍒嗙被锛圫hake, Reaction, Cinematic, Zoom, Effect, Special锛?
 * - DataType: 鏁版嵁绫诲瀷锛坒loat, FLinearColor锛?
 * - DefaultValue: 榛樿鍊硷紙鏁板€煎"25.0"锛屾垨棰滆壊鏍煎紡濡?(1.0,0.2,0.2,1.0)"锛?
 * - MinValue: 鏈€灏忓€硷紙"-"琛ㄧず鏃犻檺鍒讹級
 * - MaxValue: 鏈€澶у€硷紙"-"琛ㄧず鏃犻檺鍒讹級
 * - Unit: 鍗曚綅锛堝 "Hz", "cm", "degree", "s", "-"锛?
 * - Description: 鍙傛暟鎻忚堪
 * - RelatedModifiers: 鍏宠仈淇敼鍣↖D鍒楄〃锛岀敤"/"鍒嗛殧锛堝 "S01", "C01/C02/C03"锛?
 */
USTRUCT(BlueprintType)
struct FCameraModifierParameterRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	/** 鍙傛暟鍞竴鏍囪瘑绗︼紙瀵煎叆鍚庢瀛楁浼氭垚涓篟owName锛屾澶勪繚鐣欎絾涓虹┖锛?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic Info")
	FString ParamID;

	/** 鍙傛暟鍚嶇О - 绋嬪簭寮曠敤鏍囪瘑绗?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic Info")
	FName ParamName;

	/** 涓枃鏄剧ず鍚嶇О */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic Info")
	FString ChineseName;

	/** 鍙傛暟鍒嗙被锛圫hake, Reaction, Cinematic, Zoom, Effect, Special锛?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Classification")
	FString Category;

	/** 鏁版嵁绫诲瀷锛坒loat, FLinearColor锛?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Classification")
	FString DataType;

	/** 榛樿鍊硷紙鏁板€煎"25.0"锛屾垨棰滆壊鏍煎紡濡?(1.0,0.2,0.2,1.0)"锛?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Value")
	FString DefaultValue;

	/** 鏈€灏忓€硷紙"-"琛ㄧず鏃犻檺鍒讹級 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Value")
	FString MinValue;

	/** 鏈€澶у€硷紙"-"琛ㄧず鏃犻檺鍒讹級 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Value")
	FString MaxValue;

	/** 鍗曚綅锛堝 "Hz", "cm", "degree", "s", "-"锛?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display")
	FString Unit;

	/** 鍙傛暟鎻忚堪 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display")
	FString Description;

	/** 鍏宠仈淇敼鍣↖D鍒楄〃锛岀敤"/"鍒嗛殧锛堝 "S01", "C01/C02/C03"锛?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relations")
	FString RelatedModifiers;

	// 鏋勯€犲嚱鏁?
	FCameraModifierParameterRow()
		: ParamID(TEXT(""))
		, ParamName(NAME_None)
		, ChineseName(TEXT(""))
		, Category(TEXT("Shake"))
		, DataType(TEXT("float"))
		, DefaultValue(TEXT("0"))
		, MinValue(TEXT("-"))
		, MaxValue(TEXT("-"))
		, Unit(TEXT("-"))
		, Description(TEXT(""))
		, RelatedModifiers(TEXT("-"))
	{
	}
};
