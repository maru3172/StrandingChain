// File: Source/StrandingChain/Battle/Skills/SCSkill_Shield.cpp
#include "SCSkill_Shield.h"
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
	// 실제 방어 로직은 추후 구현 — 현재는 더미
	UE_LOG(LogStrandingChain, Log,
		TEXT("[Shield] %s: 방어 태세 발동 (더미)"), *GetNameSafe(Caster));
	return true;
}
