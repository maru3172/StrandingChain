// Copyright StrandingChain. All Rights Reserved.
// File: Source/StrandingChain/Battle/Skills/SCSkill_Heal.cpp
// 회복 스킬 — 시전자(Caster) 자신을 회복. Targets는 사용하지 않음.

#include "SCSkill_Heal.h"
#include "Battle/SCCharacterBase.h"
#include "StrandingChain.h"

USCSkill_Heal::USCSkill_Heal()
{
	SkillData.SkillID    = FName("Skill_Heal");
	SkillData.SkillName  = FText::FromString(TEXT("회복"));
	SkillData.CostType   = ESCSkillCostType::Consume;
	SkillData.CostAmount = 2;
}

bool USCSkill_Heal::Execute_Implementation(AActor* Caster, const TArray<AActor*>& Targets)
{
	if (!IsValid(Caster))
	{
		UE_LOG(LogStrandingChain, Warning, TEXT("[SCSkill_Heal] Caster 유효하지 않음."));
		return false;
	}

	ASCCharacterBase* CasterChar = Cast<ASCCharacterBase>(Caster);
	if (!IsValid(CasterChar))
	{
		UE_LOG(LogStrandingChain, Warning, TEXT("[SCSkill_Heal] Caster가 SCCharacterBase가 아님."));
		return false;
	}

	CasterChar->ApplyHeal(HealAmount);
	UE_LOG(LogStrandingChain, Log,
		TEXT("[SCSkill_Heal] %s 자가회복 +%d"), *Caster->GetName(), HealAmount);
	return true;
}
