// Copyright StrandingChain. All Rights Reserved.
// File: Source/StrandingChain/UI/SCHPBarWidget.h
// 역할: 캐릭터 머리 위에 붙는 체력바 UMG 위젯
//       C++에서 HPPercent만 갱신, 시각 표현은 Blueprint에서 구현

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SCHPBarWidget.generated.h"

/**
 * SCHPBarWidget
 * - UProgressBar 바인딩용 Percent 값 제공
 * - Blueprint(WBP_HPBar)에서 ProgressBar의 Percent를
 *   GetHPPercent() 함수에 바인딩하거나 UpdateHP()로 직접 갱신
 */
UCLASS(Abstract, Blueprintable)
class STRANDINGCHAIN_API USCHPBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/**
	 * 체력 갱신 — 캐릭터의 HP 변경 시 호출
	 * @param CurrentHP 현재 HP
	 * @param MaxHP     최대 HP (0 이하면 무시)
	 */
	UFUNCTION(BlueprintCallable, Category = "SC|UI")
	void UpdateHP(int32 CurrentHP, int32 MaxHP);

	/** ProgressBar Percent 바인딩용 (0.0 ~ 1.0) */
	UFUNCTION(BlueprintPure, Category = "SC|UI")
	float GetHPPercent() const { return HPPercent; }

	/** Blueprint 이벤트: HP 변경 시 애니메이션 등 처리 */
	UFUNCTION(BlueprintImplementableEvent, Category = "SC|UI")
	void OnHPChanged(float NewPercent);

private:
	UPROPERTY()
	float HPPercent = 1.f;
};
