// File: Source/StrandingChain/Battle/Skills/SCSkill_CostGenerate.cpp
#include "SCSkill_CostGenerate.h"
#include "StrandingChain.h"

USCSkill_CostGenerate::USCSkill_CostGenerate()
{
	SkillData.SkillID    = FName("Skill_CostGenerate");
	SkillData.SkillName  = FText::FromString(TEXT("충전"));
	SkillData.CostType   = ESCSkillCostType::Generate;
	SkillData.CostAmount = 2;
}

bool USCSkill_CostGenerate::Execute_Implementation(AActor* Caster, const TArray<AActor*>& Targets)
{
	// 코스트 획득은 SCTurnManager에서 처리 — 여기선 로그만
	UE_LOG(LogStrandingChain, Log,
		TEXT("[CostGenerate] %s: 코스트 +2 (TurnManager에서 처리됨)"), *GetNameSafe(Caster));
	return true;
}
