// Copyright StrandingChain. All Rights Reserved.
// File: Source/StrandingChain/Battle/Skills/SCSkill_Shield.h
// 방어막 스킬 — 시전자에게 TempShield 부여, 피해 흡수 후 소멸

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

	/** 부여할 방어막 수치 */
	UPROPERTY(EditDefaultsOnly, Category = "SC|Skill", meta = (ClampMin = 1))
	int32 ShieldAmount = 20;
};
