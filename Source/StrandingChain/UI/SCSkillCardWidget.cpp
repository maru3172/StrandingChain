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
			TEXT("[SCSkillCardWidget] InitCard: InSkill 유효하지 않음."));
		return;
	}
	SkillRef   = InSkill;
	DrawnIndex = InDrawnIndex;
	bEnqueued  = false;

	OnCardInitialized(InSkill->SkillData);
}

void USCSkillCardWidget::SetEnqueued(bool bInEnqueued)
{
	bEnqueued = bInEnqueued;
	OnEnqueueStateChanged(bInEnqueued);
}

void USCSkillCardWidget::NotifyClicked()
{
	if (!IsValid(SkillRef))
	{
		UE_LOG(LogStrandingChain, Warning,
			TEXT("[SCSkillCardWidget] NotifyClicked: SkillRef 유효하지 않음."));
		return;
	}
	OnSkillCardClicked.Broadcast(SkillRef, DrawnIndex);
	UE_LOG(LogStrandingChain, Log,
		TEXT("[SCSkillCardWidget] 클릭: %s DrawnIndex=%d"),
		*SkillRef->SkillData.SkillID.ToString(), DrawnIndex);
}

FReply USCSkillCardWidget::NativeOnMouseButtonDown(
	const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		NotifyClicked();
		return FReply::Handled();
	}
	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}
