// File: Source/StrandingChain/Battle/Skills/SCSkill_Shield.h
// 더미 스킬 5: 방어 태세 (코스트 소모 1, 탱커 전용 더미)
#pragma once
#include "CoreMinimal.h"
#include "Battle/SCSkillBase.h"
#include "SCSkill_Shield.generated.h"

UCLASS()
class STRANDINGCHAIN_API USCSkill_Shield : public USCSkillBase
{
	GENERATED_BODY()
public:
	USCSkill_Shield();
	virtual bool Execute_Implementation(AActor* Caster, const TArray<AActor*>& Targets) override;
};
