// Copyright StrandingChain. All Rights Reserved.
// File: Source/StrandingChain/Battle/SCSkillBase.h

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SCBattleTypes.h"
#include "SCSkillBase.generated.h"

class USCCharacterBase;

/**
 * SCSkillBase
 * - 스킬 1개를 표현하는 UObject 기반 클래스
 * - Blueprint에서 상속하여 효과 구현 (OnExecute)
 */
UCLASS(Abstract, Blueprintable, BlueprintType)
class STRANDINGCHAIN_API USCSkillBase : public UObject
{
	GENERATED_BODY()

public:
	/** 스킬 정적 데이터 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill")
	FSCSkillData SkillData;

	/**
	 * 스킬 실행
	 * @param Caster  시전자 (Null 검사 후 호출할 것)
	 * @param Targets 대상 목록
	 * @return 실행 성공 여부
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Skill")
	bool Execute(AActor* Caster, const TArray<AActor*>& Targets);
	virtual bool Execute_Implementation(AActor* Caster, const TArray<AActor*>& Targets);

	/** 이 스킬이 코스트를 획득하는 스킬인지 반환 */
	UFUNCTION(BlueprintPure, Category = "Skill")
	bool IsGeneratingCost() const
	{
		return SkillData.CostType == ESCSkillCostType::Generate;
	}
};
