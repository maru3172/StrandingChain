// Copyright StrandingChain. All Rights Reserved.
// File: Source/StrandingChain/Battle/SCTeamState.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "SCBattleTypes.h"
#include "SCTeamState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTeamCostChanged, ESCTeam, Team, int32, NewCost);

/**
 * SCTeamState
 * - 각 팀의 공용 코스트(Cost) 관리
 * - GameState에서 참조
 */
UCLASS(BlueprintType)
class STRANDINGCHAIN_API USCTeamState : public UObject
{
	GENERATED_BODY()

public:
	/** 팀 구분 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SC|Team")
	ESCTeam Team = ESCTeam::TeamA;

	/** 현재 코스트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SC|Team")
	int32 CurrentCost = 0;

	/** 코스트 최댓값 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SC|Team", meta = (ClampMin = 1))
	int32 MaxCost = 10;

	/** 코스트 변경 델리게이트 */
	UPROPERTY(BlueprintAssignable, Category = "SC|Team")
	FOnTeamCostChanged OnCostChanged;

	/**
	 * 코스트 소모 시도
	 * @return 코스트가 충분하면 true, 부족하면 false
	 */
	UFUNCTION(BlueprintCallable, Category = "SC|Team")
	bool TryConsumeCost(int32 Amount);

	/** 코스트 획득 */
	UFUNCTION(BlueprintCallable, Category = "SC|Team")
	void AddCost(int32 Amount);

	/** 코스트 초기화 */
	UFUNCTION(BlueprintCallable, Category = "SC|Team")
	void ResetCost(int32 InitialAmount = 0);
};
