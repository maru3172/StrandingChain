// File: Source/StrandingChain/Battle/Skills/SCSkill_Heal.h
// 더미 스킬 4: 회복 (코스트 소모 2, 아군 HP 회복)
#pragma once
#include "CoreMinimal.h"
#include "Battle/SCSkillBase.h"
#include "SCSkill_Heal.generated.h"

UCLASS()
class STRANDINGCHAIN_API USCSkill_Heal : public USCSkillBase
{
	GENERATED_BODY()
public:
	USCSkill_Heal();
	virtual bool Execute_Implementation(AActor* Caster, const TArray<AActor*>& Targets) override;

	UPROPERTY(EditDefaultsOnly, Category = "SC|Skill")
	int32 HealAmount = 20;
};
