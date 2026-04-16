// Copyright StrandingChain. All Rights Reserved.
// File: Source/StrandingChain/UI/SCSkillQueueSlotWidget.h
// 역할: 스킬 큐 등록창 슬롯 1개
//       카드를 드래그 드롭으로 받고, 우클릭 시 원위치 반환

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SCSkillQueueSlotWidget.generated.h"

class USCSkillCardWidget;
class USCSkillQueueWidget;
class UBorder;
class UTextBlock;

UCLASS(Abstract, Blueprintable)
class STRANDINGCHAIN_API USCSkillQueueSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 현재 슬롯에 등록된 카드 (없으면 null) */
	UPROPERTY(BlueprintReadOnly, Category = "SC|Queue")
	TObjectPtr<USCSkillCardWidget> SlottedCard;

	/** 슬롯 인덱스 (0~2) */
	UPROPERTY(BlueprintReadOnly, Category = "SC|Queue")
	int32 SlotIndex = 0;

	/** 소속 큐 위젯 참조 */
	UPROPERTY(BlueprintReadWrite, Category = "SC|Queue")
	TWeakObjectPtr<USCSkillQueueWidget> OwnerQueue;

	UFUNCTION(BlueprintPure, Category = "SC|Queue")
	bool IsEmpty() const { return SlottedCard == nullptr; }

	/** 카드 등록 (Panel에서 호출) */
	UFUNCTION(BlueprintCallable, Category = "SC|Queue")
	void PlaceCard(USCSkillCardWidget* Card);

	/** 카드 제거 후 반환 (Panel에서 호출) */
	UFUNCTION(BlueprintCallable, Category = "SC|Queue")
	USCSkillCardWidget* TakeCard();

	/** Blueprint 이벤트: 카드 등록 시 슬롯 비주얼 갱신 */
	UFUNCTION(BlueprintImplementableEvent, Category = "SC|Queue")
	void OnSlotFilled(USCSkillCardWidget* Card);

	/** Blueprint 이벤트: 슬롯 비워질 때 비주얼 갱신 */
	UFUNCTION(BlueprintImplementableEvent, Category = "SC|Queue")
	void OnSlotCleared();

protected:
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual bool NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
};
