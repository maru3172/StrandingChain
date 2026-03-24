// Copyright StrandingChain. All Rights Reserved.
// File: Source/StrandingChain/UI/SCSkillCardWidget.cpp

#include "UI/SCSkillCardWidget.h"
#include "Battle/SCSkillBase.h"
#include "StrandingChain.h"

void USCSkillCardWidget::InitCard(USCSkillBase* InSkill, int32 InDrawnIndex)
{
	if (!IsValid(InSkill))
	{
		UE_LOG(LogStrandingChain, Warning,
			TEXT("[SCSkillCardWidget] InitCard: InSkill이 유효하지 않음."));
		return;
	}
	SkillRef    = InSkill;
	DrawnIndex  = InDrawnIndex;

	// Blueprint에서 텍스트/이미지 세팅
	OnCardInitialized(InSkill->SkillData);
}

void USCSkillCardWidget::NotifyClicked()
{
	if (!IsValid(SkillRef)) { return; }
	OnSkillCardClicked.Broadcast(SkillRef, DrawnIndex);
}
