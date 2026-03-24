// File: Source/StrandingChain/Battle/Skills/SCSkill_HeavyAttack.cpp
#include "SCSkill_HeavyAttack.h"
#include "Battle/SCCharacterBase.h"
#include "StrandingChain.h"

USCSkill_HeavyAttack::USCSkill_HeavyAttack()
{
	SkillData.SkillID    = FName("Skill_HeavyAttack");
	SkillData.SkillName  = FText::FromString(TEXT("강 공격"));
	SkillData.CostType   = ESCSkillCostType::Consume;
	SkillData.CostAmount = 2;
}

bool USCSkill_HeavyAttack::Execute_Implementation(AActor* Caster, const TArray<AActor*>& Targets)
{
	if (!IsValid(Caster)) { return false; }
	for (AActor* Target : Targets)
	{
		if (ASCCharacterBase* Char = Cast<ASCCharacterBase>(Target))
		{
			Char->ApplyDamage(Damage);
			UE_LOG(LogStrandingChain, Log,
				TEXT("[HeavyAttack] %s → %s, 데미지=%d"), *Caster->GetName(), *Target->GetName(), Damage);
		}
	}
	return true;
}
