// File: Source/StrandingChain/Battle/Skills/SCSkill_CostGenerate.h
// 더미 스킬 3: 코스트 충전 (코스트 획득 2, 데미지 없음)
#pragma once
#include "CoreMinimal.h"
#include "Battle/SCSkillBase.h"
#include "SCSkill_CostGenerate.generated.h"

UCLASS()
class STRANDINGCHAIN_API USCSkill_CostGenerate : public USCSkillBase
{
	GENERATED_BODY()
public:
	USCSkill_CostGenerate();
	virtual bool Execute_Implementation(AActor* Caster, const TArray<AActor*>& Targets) override;
};
