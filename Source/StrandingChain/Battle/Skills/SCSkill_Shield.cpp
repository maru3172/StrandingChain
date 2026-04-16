// Copyright StrandingChain. All Rights Reserved.
// File: Source/StrandingChain/Battle/Skills/SCSkill_Shield.cpp
// 방어막 스킬 — 시전자에게 TempShield 부여. 피해 흡수 후 소멸.

#include "SCSkill_Shield.h"
#include "Battle/SCCharacterBase.h"
#include "StrandingChain.h"

USCSkill_Shield::USCSkill_Shield()
{
	SkillData.SkillID    = FName("Skill_Shield");
	SkillData.SkillName  = FText::FromString(TEXT("방어 태세"));
	SkillData.CostType   = ESCSkillCostType::Consume;
	SkillData.CostAmount = 1;
}

bool USCSkill_Shield::Execute_Implementation(AActor* Caster, const TArray<AActor*>& Targets)
{
	if (!IsValid(Caster))
	{
		UE_LOG(LogStrandingChain, Warning, TEXT("[SCSkill_Shield] Caster 유효하지 않음."));
		return false;
	}

	ASCCharacterBase* CasterChar = Cast<ASCCharacterBase>(Caster);
	if (!IsValid(CasterChar))
	{
		UE_LOG(LogStrandingChain, Warning, TEXT("[SCSkill_Shield] Caster가 SCCharacterBase가 아님."));
		return false;
	}

	CasterChar->AddShield(ShieldAmount);
	UE_LOG(LogStrandingChain, Log,
		TEXT("[SCSkill_Shield] %s 방어막 +%d"), *Caster->GetName(), ShieldAmount);
	return true;
}
