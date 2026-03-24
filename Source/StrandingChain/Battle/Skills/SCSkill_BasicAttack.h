// Copyright StrandingChain. All Rights Reserved.
// File: Source/StrandingChain/Battle/Skills/SCSkill_BasicAttack.h
// 더미 스킬 1: 기본 공격 (코스트 소모 1, 단일 대상 데미지)

#pragma once
#include "CoreMinimal.h"
#include "Battle/SCSkillBase.h"
#include "SCSkill_BasicAttack.generated.h"

UCLASS()
class STRANDINGCHAIN_API USCSkill_BasicAttack : public USCSkillBase
{
	GENERATED_BODY()
public:
	USCSkill_BasicAttack();
	virtual bool Execute_Implementation(AActor* Caster, const TArray<AActor*>& Targets) override;

	UPROPERTY(EditDefaultsOnly, Category = "SC|Skill")
	int32 Damage = 15;
};
