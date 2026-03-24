// File: Source/StrandingChain/Battle/Skills/SCSkill_BasicAttack.cpp
#include "SCSkill_BasicAttack.h"
#include "Battle/SCCharacterBase.h"
#include "StrandingChain.h"

USCSkill_BasicAttack::USCSkill_BasicAttack()
{
	SkillData.SkillID    = FName("Skill_BasicAttack");
	SkillData.SkillName  = FText::FromString(TEXT("기본 공격"));
	SkillData.CostType   = ESCSkillCostType::Consume;
	SkillData.CostAmount = 1;
}

bool USCSkill_BasicAttack::Execute_Implementation(AActor* Caster, const TArray<AActor*>& Targets)
{
	if (!IsValid(Caster)) { return false; }
	for (AActor* Target : Targets)
	{
		if (ASCCharacterBase* Char = Cast<ASCCharacterBase>(Target))
		{
			Char->ApplyDamage(Damage);
			UE_LOG(LogStrandingChain, Log,
				TEXT("[BasicAttack] %s → %s, 데미지=%d"), *Caster->GetName(), *Target->GetName(), Damage);
		}
	}
	return true;
}
