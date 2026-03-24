// File: Source/StrandingChain/Battle/Skills/SCSkill_HeavyAttack.h
// 더미 스킬 2: 강공격 (코스트 소모 2, 높은 데미지)
#pragma once
#include "CoreMinimal.h"
#include "Battle/SCSkillBase.h"
#include "SCSkill_HeavyAttack.generated.h"

UCLASS()
class STRANDINGCHAIN_API USCSkill_HeavyAttack : public USCSkillBase
{
	GENERATED_BODY()
public:
	USCSkill_HeavyAttack();
	virtual bool Execute_Implementation(AActor* Caster, const TArray<AActor*>& Targets) override;

	UPROPERTY(EditDefaultsOnly, Category = "SC|Skill")
	int32 Damage = 30;
};
