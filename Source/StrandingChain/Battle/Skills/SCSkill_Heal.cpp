// File: Source/StrandingChain/Battle/Skills/SCSkill_Heal.cpp
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
	if (!IsValid(Caster)) { return false; }
	for (AActor* Target : Targets)
	{
		if (ASCCharacterBase* Char = Cast<ASCCharacterBase>(Target))
		{
			// 회복 = 음수 데미지
			int32 Healed = FMath::Min(HealAmount, Char->MaxHP - Char->CurrentHP);
			Char->CurrentHP = FMath::Min(Char->CurrentHP + HealAmount, Char->MaxHP);
			UE_LOG(LogStrandingChain, Log,
				TEXT("[Heal] %s → %s, 회복=%d"), *Caster->GetName(), *Target->GetName(), Healed);
		}
	}
	return true;
}
