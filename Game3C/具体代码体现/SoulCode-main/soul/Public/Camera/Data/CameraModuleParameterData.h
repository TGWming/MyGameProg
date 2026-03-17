// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "CameraModuleParameterData.generated.h"

/**
 * 鐩告満妯″潡鍙傛暟鏁版嵁琛ㄨ缁撴瀯浣?
 * Camera Module Parameter Data Table Row Structure
 * 
 * 鐢ㄤ簬浠嶤SV瀵煎叆鐩告満妯″潡鍙傛暟閰嶇疆鏁版嵁
 * Used for importing camera module parameter configuration data from CSV
 * 
 * CSV鏍煎紡璇存槑锛?
 * - ParamID: 鍙傛暟鍞竴鏍囪瘑绗︼紙浼氭垚涓篟owName锛岀粨鏋勪綋涓繚鐣欎絾瀵煎叆鍚庝负绌猴級
 * - ParamName: 鍙傛暟鍚嶇О锛岀▼搴忓紩鐢ㄦ爣璇嗙
 * - ChineseName: 涓枃鏄剧ず鍚嶇О
 * - Category: 鍙傛暟鍒嗙被锛圥osition, Rotation, Distance, FOV, Offset, Constraint锛?
 * - DataType: 鏁版嵁绫诲瀷锛坒loat, FVector, FName锛?
 * - DefaultValue: 榛樿鍊硷紙鏁板€笺€佸悜閲忔牸寮?(x,y,z)"銆佹垨鍚嶇О锛?
 * - MinValue: 鏈€灏忓€硷紙"-"琛ㄧず鏃犻檺鍒讹級
 * - MaxValue: 鏈€澶у€硷紙"-"琛ㄧず鏃犻檺鍒讹級
 * - Unit: 鍗曚綅锛堝 "1/s", "cm", "degree", "s"绛夛級
 * - Description: 鍙傛暟鎻忚堪
 * - RelatedModules: 鍏宠仈妯″潡ID鍒楄〃锛岀敤"/"鍒嗛殧
 */
USTRUCT(BlueprintType)
struct FCameraModuleParameterRow : public FTableRowBase
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

	/** 鍙傛暟鍒嗙被锛圥osition, Rotation, Distance, FOV, Offset, Constraint锛?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Classification")
	FString Category;

	/** 鏁版嵁绫诲瀷锛坒loat, FVector, FName锛?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Classification")
	FString DataType;

	/** 榛樿鍊硷紙鏁板€笺€佸悜閲忔牸寮?(x,y,z)"銆佹垨鍚嶇О锛?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Value")
	FString DefaultValue;

	/** 鏈€灏忓€硷紙"-"琛ㄧず鏃犻檺鍒讹級 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Value")
	FString MinValue;

	/** 鏈€澶у€硷紙"-"琛ㄧず鏃犻檺鍒讹級 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Value")
	FString MaxValue;

	/** 鍗曚綅锛堝 "1/s", "cm", "degree", "s", "cm/s"绛夛級 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display")
	FString Unit;

	/** 鍙傛暟鎻忚堪 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display")
	FString Description;

	/** 鍏宠仈妯″潡ID鍒楄〃锛岀敤"/"鍒嗛殧锛堝 "P01/P02"锛?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relations")
	FString RelatedModules;

	// 鏋勯€犲嚱鏁?
	FCameraModuleParameterRow()
		: ParamID(TEXT(""))
		, ParamName(NAME_None)
		, ChineseName(TEXT(""))
		, Category(TEXT("Position"))
		, DataType(TEXT("float"))
		, DefaultValue(TEXT("0"))
		, MinValue(TEXT("-"))
		, MaxValue(TEXT("-"))
		, Unit(TEXT("-"))
		, Description(TEXT(""))
		, RelatedModules(TEXT("-"))
	{
	}
};
