// Copyright StrandingChain. All Rights Reserved.
// File: Source/StrandingChain/UI/SCSkillQueueWidget.h

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SCSkillQueueWidget.generated.h"

class USCSkillCardWidget;
class USCSkillQueueSlotWidget;
class UHorizontalBox;

UCLASS(Abstract, Blueprintable)
class STRANDINGCHAIN_API USCSkillQueueWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SC|Queue")
	TSubclassOf<USCSkillQueueSlotWidget> QueueSlotClass;

	UFUNCTION(BlueprintCallable, Category = "SC|Queue")
	void InitSlots();

	UFUNCTION(BlueprintCallable, Category = "SC|Queue")
	void ClearSlots();

	UFUNCTION(BlueprintCallable, Category = "SC|Queue")
	void RegisterCardToSlot(USCSkillCardWidget* Card, USCSkillQueueSlotWidget* InSlot);

	UFUNCTION(BlueprintCallable, Category = "SC|Queue")
	void ReturnCardFromSlot(USCSkillQueueSlotWidget* InSlot);

	/**
	 * 현재 큐 슬롯에 채워진 카드들의 DrawnIndex 배열 반환
	 * PlayerController::ConfirmSkillQueue()에서 호출하여 EnqueueSkill에 전달
	 */
	UFUNCTION(BlueprintCallable, Category = "SC|Queue")
	TArray<int32> GetQueuedDrawnIndices() const;

protected:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UHorizontalBox> SlotContainer;

private:
	TArray<TObjectPtr<USCSkillQueueSlotWidget>> Slots;
};
