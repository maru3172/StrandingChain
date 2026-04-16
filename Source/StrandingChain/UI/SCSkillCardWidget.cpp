// Copyright StrandingChain. All Rights Reserved.
// File: Source/StrandingChain/UI/SCSkillCardWidget.cpp

#include "UI/SCSkillCardWidget.h"
#include "Battle/SCSkillBase.h"
#include "StrandingChain.h"
#include "Blueprint/DragDropOperation.h"
#include "Blueprint/WidgetBlueprintLibrary.h"

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

FReply USCSkillCardWidget::NativeOnPreviewMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FEventReply Reply = UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton);
	return Reply.NativeReply;
}

void USCSkillCardWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

	USCSkillCardWidget* DragVisual = CreateWidget<USCSkillCardWidget>(GetOwningPlayer(), GetClass());
	if (IsValid(DragVisual) && IsValid(SkillRef))
	{
		DragVisual->InitCard(SkillRef, DrawnIndex);
		const FVector2D CardSize = InGeometry.GetLocalSize();
		DragVisual->SetDesiredSizeInViewport(CardSize.X > 0.f ? CardSize : FVector2D(120.f, 160.f));
	}

	UDragDropOperation* DragOp = NewObject<UDragDropOperation>(GetTransientPackage());
	DragOp->Payload           = this;
	DragOp->DefaultDragVisual = DragVisual;
	DragOp->Pivot             = EDragPivot::MouseDown;
	OutOperation = DragOp;

	UE_LOG(LogStrandingChain, Log, TEXT("[SCSkillCard] 드래그 시작: %s"), *GetName());
}
