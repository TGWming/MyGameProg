// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Camera/Data/CameraStateEnums.h"
#include "SoulsCameraParams_Collision.generated.h"

/**
 * D-Group: Collision Parameters (30+ params total)
 * Controls behavior of the 20 collision strategies
 * 
 * Strategy Categories:
 * - Detection (D01-D04): How collisions are detected
 * - Response (RS01-RS05): How camera responds to collisions
 * - Occlusion (OC01-OC04): How occluding objects are handled
 * - Recovery (RC01-RC03): How camera returns to ideal position
 * - Special (SP01-SP04): Special situation handling
 */

//========================================
// D1: Detection Parameters
// Strategies: D01-D04
//========================================
USTRUCT(BlueprintType)
struct FCollisionParams_Detection
{
	GENERATED_BODY()

	//--- Strategy Enable Flags ---
	
	/** Enable D01: Single Ray detection */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection|Strategies")
	bool bEnableD01_SingleRay;

	/** Enable D02: Sphere Sweep detection (default, recommended) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection|Strategies")
	bool bEnableD02_SphereSweep;

	/** Enable D03: Multi Ray detection */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection|Strategies")
	bool bEnableD03_MultiRay;

	/** Enable D04: Multi Sphere Sweep detection (predictive) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection|Strategies")
	bool bEnableD04_MultiSphereSweep;

	//--- Common Parameters ---

	/** Main collision probe radius for sphere sweeps (D02, D04) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection", meta = (ClampMin = "5.0", ClampMax = "50.0"))
	float ProbeRadius;

	/** Number of rays for multi-ray detection (D03) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection", meta = (ClampMin = "3", ClampMax = "12", EditCondition = "bEnableD03_MultiRay"))
	int32 MultiRayCount;

	/** Collision channel to use */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
	TEnumAsByte<ECollisionChannel> CollisionChannel;

	/** Minimum distance from character to start detection */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection", meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float DetectionStartOffset;

	/** Detection frequency per second */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection", meta = (ClampMin = "10.0", ClampMax = "120.0"))
	float DetectionRate;

	/** Use complex collision for detailed geometry */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
	bool bUseComplexCollision;

	/** Extra padding from collision surfaces */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection", meta = (ClampMin = "0.0", ClampMax = "50.0"))
	float CollisionPadding;

	/** Time ahead to predict camera position (D04) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection", meta = (ClampMin = "0.0", ClampMax = "0.5", EditCondition = "bEnableD04_MultiSphereSweep"))
	float PredictionTime;

	FCollisionParams_Detection()
		: bEnableD01_SingleRay(false)
		, bEnableD02_SphereSweep(true)  // Default detection method
		, bEnableD03_MultiRay(false)
		, bEnableD04_MultiSphereSweep(false)
		, ProbeRadius(12.0f)
		, MultiRayCount(5)
		, CollisionChannel(ECC_Camera)
		, DetectionStartOffset(20.0f)
		, DetectionRate(60.0f)
		, bUseComplexCollision(false)
		, CollisionPadding(10.0f)
		, PredictionTime(0.1f)
	{}
};

//========================================
// D2: Response Parameters
// Strategies: RS01-RS05
//========================================
USTRUCT(BlueprintType)
struct FCollisionParams_Response
{
	GENERATED_BODY()

	//--- Strategy Enable Flags ---

	/** Enable RS01: Pull In response (default, most common) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Response|Strategies")
	bool bEnableRS01_PullIn;

	/** Enable RS02: Slide response (slide along surfaces) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Response|Strategies")
	bool bEnableRS02_Slide;

	/** Enable RS03: Orbit response (rotate around character) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Response|Strategies")
	bool bEnableRS03_Orbit;

	/** Enable RS04: FOV Compensate response (widen FOV when close) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Response|Strategies")
	bool bEnableRS04_FOVCompensate;

	/** Enable RS05: Instant Snap response (emergency snap to safe position) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Response|Strategies")
	bool bEnableRS05_InstantSnap;

	//--- Pull In Parameters (RS01) ---

	/** Speed to pull camera in on collision */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Response|PullIn", meta = (ClampMin = "1.0", ClampMax = "50.0"))
	float PullInSpeed;

	/** Minimum distance camera can be pulled to */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Response|PullIn", meta = (ClampMin = "20.0", ClampMax = "200.0"))
	float MinPullInDistance;

	/** Acceleration during pull-in (RS01) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Response|PullIn", meta = (ClampMin = "500.0", ClampMax = "5000.0"))
	float PullInAcceleration;

	//--- Slide Parameters (RS02) ---

	/** Slide friction coefficient */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Response|Slide", meta = (ClampMin = "0.0", ClampMax = "1.0", EditCondition = "bEnableRS02_Slide"))
	float SlideFriction;

	/** Speed of slide movement (RS02) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Response|Slide", meta = (ClampMin = "50.0", ClampMax = "1000.0", EditCondition = "bEnableRS02_Slide"))
	float SlideSpeed;

	//--- Orbit Parameters (RS03) ---

	/** Speed of orbit rotation in degrees/sec (RS03) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Response|Orbit", meta = (ClampMin = "30.0", ClampMax = "180.0", EditCondition = "bEnableRS03_Orbit"))
	float OrbitSpeed;

	//--- FOV Compensation Parameters (RS04) ---

	/** FOV increase when camera is pulled in (RS04) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Response|FOV", meta = (ClampMin = "0.0", ClampMax = "30.0", EditCondition = "bEnableRS04_FOVCompensate"))
	float PullInFOVCompensation;

	/** Maximum FOV compensation allowed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Response|FOV", meta = (ClampMin = "0.0", ClampMax = "30.0", EditCondition = "bEnableRS04_FOVCompensate"))
	float MaxFOVCompensation;

	//--- Instant Snap Parameters (RS05) ---

	/** Distance threshold for instant snap */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Response|InstantSnap", meta = (ClampMin = "50.0", ClampMax = "200.0", EditCondition = "bEnableRS05_InstantSnap"))
	float InstantSnapThreshold;

	/** Distance at which soft collision begins */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Response", meta = (ClampMin = "50.0", ClampMax = "300.0"))
	float SoftCollisionDistance;

	FCollisionParams_Response()
		: bEnableRS01_PullIn(true)  // Default response method
		, bEnableRS02_Slide(false)
		, bEnableRS03_Orbit(false)
		, bEnableRS04_FOVCompensate(false)
		, bEnableRS05_InstantSnap(false)
		, PullInSpeed(15.0f)
		, MinPullInDistance(50.0f)
		, PullInAcceleration(2000.0f)
		, SlideFriction(0.3f)
		, SlideSpeed(300.0f)
		, OrbitSpeed(90.0f)
		, PullInFOVCompensation(10.0f)
		, MaxFOVCompensation(15.0f)
		, InstantSnapThreshold(100.0f)
		, SoftCollisionDistance(100.0f)
	{}
};

//========================================
// D3: Occlusion Parameters
// Strategies: OC01-OC04
//========================================
USTRUCT(BlueprintType)
struct FCollisionParams_Occlusion
{
	GENERATED_BODY()

	//--- Strategy Enable Flags ---

	/** Enable OC01: Fade Out occlusion (default, gradual fade) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Occlusion|Strategies")
	bool bEnableOC01_FadeOut;

	/** Enable OC02: Hide occlusion (instant hide) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Occlusion|Strategies")
	bool bEnableOC02_Hide;

	/** Enable OC03: Pull In Further occlusion (move camera closer) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Occlusion|Strategies")
	bool bEnableOC03_PullInFurther;

	/** Enable OC04: Dither Fade occlusion (dithered transparency) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Occlusion|Strategies")
	bool bEnableOC04_DitherFade;

	//--- Fade Parameters (OC01, OC04) ---

	/** Fade speed for occluding objects */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Occlusion|Fade", meta = (ClampMin = "1.0", ClampMax = "20.0"))
	float OccluderFadeSpeed;

	/** Minimum opacity for faded occluders */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Occlusion|Fade", meta = (ClampMin = "0.0", ClampMax = "0.5"))
	float OccluderMinOpacity;

	/** Distance at which fade begins (OC01, OC04) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Occlusion|Fade", meta = (ClampMin = "50.0", ClampMax = "300.0"))
	float FadeStartDistance;

	/** Distance at which fade is complete (OC01, OC04) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Occlusion|Fade", meta = (ClampMin = "10.0", ClampMax = "150.0"))
	float FadeEndDistance;

	//--- Hide Parameters (OC02) ---

	/** Delay before hiding occluding objects (OC02) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Occlusion|Hide", meta = (ClampMin = "0.0", ClampMax = "0.5", EditCondition = "bEnableOC02_Hide"))
	float HideDelay;

	//--- Pull In Further Parameters (OC03) ---

	/** Additional distance to pull in when occluded (OC03) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Occlusion|PullIn", meta = (ClampMin = "10.0", ClampMax = "150.0", EditCondition = "bEnableOC03_PullInFurther"))
	float AdditionalPullInDistance;

	//--- Common Parameters ---

	/** Check if character is occluded */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Occlusion")
	bool bCheckCharacterOcclusion;

	/** Check if lock-on target is occluded */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Occlusion")
	bool bCheckTargetOcclusion;

	/** Interval between occlusion checks (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Occlusion", meta = (ClampMin = "0.01", ClampMax = "0.2"))
	float OcclusionCheckInterval;

	/** Maximum number of occluders to track */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Occlusion", meta = (ClampMin = "1", ClampMax = "20"))
	int32 MaxTrackedOccluders;

	FCollisionParams_Occlusion()
		: bEnableOC01_FadeOut(true)  // Default occlusion method
		, bEnableOC02_Hide(false)
		, bEnableOC03_PullInFurther(false)
		, bEnableOC04_DitherFade(false)
		, OccluderFadeSpeed(8.0f)
		, OccluderMinOpacity(0.2f)
		, FadeStartDistance(150.0f)
		, FadeEndDistance(50.0f)
		, HideDelay(0.1f)
		, AdditionalPullInDistance(50.0f)
		, bCheckCharacterOcclusion(true)
		, bCheckTargetOcclusion(true)
		, OcclusionCheckInterval(0.05f)
		, MaxTrackedOccluders(5)
	{}
};

//========================================
// D4: Recovery Parameters
// Strategies: RC01-RC03
//========================================
USTRUCT(BlueprintType)
struct FCollisionParams_Recovery
{
	GENERATED_BODY()

	//--- Strategy Enable Flags ---

	/** Enable RC01: Delayed Recovery (default, wait then recover) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery|Strategies")
	bool bEnableRC01_DelayedRecovery;

	/** Enable RC02: Smooth Recovery (continuous smooth return) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery|Strategies")
	bool bEnableRC02_SmoothRecovery;

	/** Enable RC03: Step Recovery (discrete step return) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery|Strategies")
	bool bEnableRC03_StepRecovery;

	//--- Delayed Recovery Parameters (RC01) ---

	/** Delay before starting recovery after collision ends (RC01) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery|Delayed", meta = (ClampMin = "0.0", ClampMax = "3.0"))
	float RecoveryDelay;

	//--- Common Recovery Parameters ---

	/** Speed to recover to ideal distance */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery", meta = (ClampMin = "1.0", ClampMax = "20.0"))
	float RecoverySpeed;

	/** Acceleration during recovery */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery", meta = (ClampMin = "100.0", ClampMax = "2000.0"))
	float RecoveryAcceleration;

	//--- Step Recovery Parameters (RC03) ---

	/** Step size for stepped recovery (RC03) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery|Step", meta = (ClampMin = "10.0", ClampMax = "100.0", EditCondition = "bEnableRC03_StepRecovery"))
	float RecoveryStepSize;

	/** Time between recovery steps (RC03) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery|Step", meta = (ClampMin = "0.05", ClampMax = "0.5", EditCondition = "bEnableRC03_StepRecovery"))
	float StepInterval;

	//--- Safety Parameters ---

	/** Abort recovery if new collision detected */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery")
	bool bAbortRecoveryOnNewCollision;

	/** Verify path is clear before recovering */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery")
	bool bVerifyRecoveryPath;

	FCollisionParams_Recovery()
		: bEnableRC01_DelayedRecovery(true)  // Default recovery method
		, bEnableRC02_SmoothRecovery(false)
		, bEnableRC03_StepRecovery(false)
		, RecoveryDelay(0.5f)
		, RecoverySpeed(5.0f)
		, RecoveryAcceleration(500.0f)
		, RecoveryStepSize(25.0f)
		, StepInterval(0.1f)
		, bAbortRecoveryOnNewCollision(true)
		, bVerifyRecoveryPath(true)
	{}
};

//========================================
// D5: Special Situation Parameters
// Strategies: SP01-SP04
//========================================
USTRUCT(BlueprintType)
struct FCollisionParams_Special
{
	GENERATED_BODY()

	//--- Strategy Enable Flags ---

	/** Enable SP01: Tight Space handling (narrow corridors) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Special|Strategies")
	bool bEnableSP01_TightSpace;

	/** Enable SP02: Low Ceiling handling (overhead obstacles) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Special|Strategies")
	bool bEnableSP02_LowCeiling;

	/** Enable SP03: Cliff Edge handling (drop-off detection) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Special|Strategies")
	bool bEnableSP03_CliffEdge;

	/** Enable SP04: Corner Case handling (wall intersections) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Special|Strategies")
	bool bEnableSP04_CornerCase;

	//--- Tight Space Parameters (SP01) ---

	/** Width threshold to detect tight space in cm (SP01) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Special|TightSpace", meta = (ClampMin = "100.0", ClampMax = "500.0", EditCondition = "bEnableSP01_TightSpace"))
	float TightSpaceThreshold;

	/** Distance reduction in tight spaces (SP01) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Special|TightSpace", meta = (ClampMin = "0.0", ClampMax = "200.0", EditCondition = "bEnableSP01_TightSpace"))
	float TightSpaceDistanceReduction;

	/** FOV increase in tight spaces (SP01) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Special|TightSpace", meta = (ClampMin = "0.0", ClampMax = "30.0", EditCondition = "bEnableSP01_TightSpace"))
	float TightSpaceFOVBonus;

	//--- Low Ceiling Parameters (SP02) ---

	/** Height threshold for low ceiling detection (SP02) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Special|LowCeiling", meta = (ClampMin = "150.0", ClampMax = "400.0", EditCondition = "bEnableSP02_LowCeiling"))
	float LowCeilingThreshold;

	/** Pitch adjustment in low ceiling areas (SP02) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Special|LowCeiling", meta = (ClampMin = "0.0", ClampMax = "45.0", EditCondition = "bEnableSP02_LowCeiling"))
	float LowCeilingPitchAdjust;

	/** Maximum pitch adjustment for low ceiling (SP02) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Special|LowCeiling", meta = (ClampMin = "0.0", ClampMax = "45.0", EditCondition = "bEnableSP02_LowCeiling"))
	float MaxPitchAdjustment;

	//--- Cliff Edge Parameters (SP03) ---

	/** Distance to look ahead for cliff edges (SP03) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Special|Cliff", meta = (ClampMin = "50.0", ClampMax = "300.0", EditCondition = "bEnableSP03_CliffEdge"))
	float CliffDetectionDistance;

	/** Height difference for cliff detection (SP03) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Special|Cliff", meta = (ClampMin = "50.0", ClampMax = "500.0", EditCondition = "bEnableSP03_CliffEdge"))
	float CliffHeightThreshold;

	/** Distance to pull back from edge (SP03) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Special|Cliff", meta = (ClampMin = "0.0", ClampMax = "100.0", EditCondition = "bEnableSP03_CliffEdge"))
	float EdgePullbackDistance;

	//--- Corner Case Parameters (SP04) ---

	/** Distance to check for corner walls (SP04) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Special|Corner", meta = (ClampMin = "50.0", ClampMax = "200.0", EditCondition = "bEnableSP04_CornerCase"))
	float CornerCheckDistance;

	/** Maximum escape distance from corner (SP04) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Special|Corner", meta = (ClampMin = "10.0", ClampMax = "100.0", EditCondition = "bEnableSP04_CornerCase"))
	float MaxCornerEscapeDistance;

	FCollisionParams_Special()
		: bEnableSP01_TightSpace(true)
		, bEnableSP02_LowCeiling(true)
		, bEnableSP03_CliffEdge(true)
		, bEnableSP04_CornerCase(true)
		, TightSpaceThreshold(250.0f)
		, TightSpaceDistanceReduction(100.0f)
		, TightSpaceFOVBonus(15.0f)
		, LowCeilingThreshold(250.0f)
		, LowCeilingPitchAdjust(15.0f)
		, MaxPitchAdjustment(20.0f)
		, CliffDetectionDistance(150.0f)
		, CliffHeightThreshold(200.0f)
		, EdgePullbackDistance(50.0f)
		, CornerCheckDistance(100.0f)
		, MaxCornerEscapeDistance(50.0f)
	{}
};

//========================================
// Complete D-Group Structure
//========================================
/**
 * Complete collision parameters (D-Group)
 * Contains all parameters for collision handling configuration
 * 
 * Strategy Summary (20 total):
 * - Detection (D01-D04): 4 strategies
 * - Response (RS01-RS05): 5 strategies
 * - Occlusion (OC01-OC04): 4 strategies
 * - Recovery (RC01-RC03): 3 strategies
 * - Special (SP01-SP04): 4 strategies
 */
USTRUCT(BlueprintType)
struct FCollisionParams
{
	GENERATED_BODY()

	/** D1: Detection parameters - D01-D04 strategies */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "D1_Detection")
	FCollisionParams_Detection Detection;

	/** D2: Response parameters - RS01-RS05 strategies */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "D2_Response")
	FCollisionParams_Response Response;

	/** D3: Occlusion parameters - OC01-OC04 strategies */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "D3_Occlusion")
	FCollisionParams_Occlusion Occlusion;

	/** D4: Recovery parameters - RC01-RC03 strategies */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "D4_Recovery")
	FCollisionParams_Recovery Recovery;

	/** D5: Special situation parameters - SP01-SP04 strategies */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "D5_Special")
	FCollisionParams_Special Special;

	/** Master enable for collision system */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
	bool bEnableCollision;

	/** Draw debug visualization */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision|Debug")
	bool bDrawDebug;

	/** Debug draw duration in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision|Debug", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DebugDrawDuration;

	FCollisionParams()
		: bEnableCollision(true)
		, bDrawDebug(false)
		, DebugDrawDuration(0.0f)
	{
		// All sub-structs use their own default constructors
	}
};
