// Copyright StrandingChain. All Rights Reserved.
// File: Source/StrandingChain/UI/SCSkillQueueSlotWidget.h
// 역할: 스킬 큐 슬롯 1개
//       드래그앤드롭 제거 — 클릭 등록 방식으로 변경
//       좌클릭으로 슬롯 해제

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SCSkillQueueSlotWidget.generated.h"

class USCSkillCardWidget;
class USCSkillQueueWidget;

UCLASS(Abstract, Blueprintable)
class STRANDINGCHAIN_API USCSkillQueueSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 현재 등록된 카드 */
	UPROPERTY(BlueprintReadOnly, Category = "SC|Queue")
	TObjectPtr<USCSkillCardWidget> SlottedCard;

	/** 슬롯 인덱스 (0~2) */
	UPROPERTY(BlueprintReadOnly, Category = "SC|Queue")
	int32 SlotIndex = 0;

	/** 소속 큐 위젯 */
	UPROPERTY(BlueprintReadWrite, Category = "SC|Queue")
	TWeakObjectPtr<USCSkillQueueWidget> OwnerQueue;

	UFUNCTION(BlueprintPure, Category = "SC|Queue")
	bool IsEmpty() const { return SlottedCard == nullptr; }

	UFUNCTION(BlueprintCallable, Category = "SC|Queue")
	void PlaceCard(USCSkillCardWidget* Card);

	UFUNCTION(BlueprintCallable, Category = "SC|Queue")
	USCSkillCardWidget* TakeCard();

	UFUNCTION(BlueprintImplementableEvent, Category = "SC|Queue")
	void OnSlotFilled(USCSkillCardWidget* Card);

	UFUNCTION(BlueprintImplementableEvent, Category = "SC|Queue")
	void OnSlotCleared();

protected:
	/** 좌클릭 → 슬롯 해제 (카드를 큐에서 제거) */
	virtual FReply NativeOnMouseButtonDown(
		const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
};
