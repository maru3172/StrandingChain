// Copyright StrandingChain. All Rights Reserved.
// File: Source/StrandingChain/Battle/SCSkillBase.cpp

#include "SCSkillBase.h"
#include "StrandingChain.h"

bool USCSkillBase::Execute_Implementation(AActor* Caster, const TArray<AActor*>& Targets)
{
	if (!IsValid(Caster))
	{
		UE_LOG(LogStrandingChain, Warning,
			TEXT("[SCSkillBase] Execute 호출 시 Caster가 유효하지 않음. SkillID=%s"),
			*SkillData.SkillID.ToString());
		return false;
	}

	UE_LOG(LogStrandingChain, Log,
		TEXT("[SCSkillBase] 기본 Execute 호출됨. SkillID=%s, Caster=%s — Blueprint에서 오버라이드 필요"),
		*SkillData.SkillID.ToString(), *Caster->GetName());

	return true;
}
