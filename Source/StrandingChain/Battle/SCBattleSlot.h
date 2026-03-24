// Copyright StrandingChain. All Rights Reserved.
// File: Source/StrandingChain/Battle/SCBattleSlot.h
// 역할: 캐릭터가 올라서는 발판 액터. 팀/슬롯 인덱스 보유.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SCBattleTypes.h"
#include "SCBattleSlot.generated.h"

class ASCCharacterBase;

UCLASS(Blueprintable)
class STRANDINGCHAIN_API ASCBattleSlot : public AActor
{
	GENERATED_BODY()

public:
	ASCBattleSlot();

	// ── 슬롯 정보 ───────────────────────────
	/** 소속 팀 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SC|Slot")
	ESCTeam Team = ESCTeam::TeamA;

	/** 팀 내 슬롯 인덱스 (0~3) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SC|Slot",
		meta = (ClampMin = 0, ClampMax = 3))
	int32 SlotIndex = 0;

	// ── 컴포넌트 ────────────────────────────
	/** 발판 메시 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SC|Slot")
	TObjectPtr<UStaticMeshComponent> PlatformMesh;

	/** 캐릭터 스폰 위치 (발판 위) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SC|Slot")
	TObjectPtr<USceneComponent> CharacterSpawnPoint;

	// ── 런타임 ──────────────────────────────
	/** 현재 슬롯에 배치된 캐릭터 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SC|Slot")
	TWeakObjectPtr<ASCCharacterBase> OccupiedCharacter;

	/** 캐릭터를 슬롯에 배치 */
	UFUNCTION(BlueprintCallable, Category = "SC|Slot")
	void PlaceCharacter(ASCCharacterBase* Character);

	/** 슬롯 비우기 */
	UFUNCTION(BlueprintCallable, Category = "SC|Slot")
	void ClearSlot();

	/** 슬롯에 캐릭터가 있는지 */
	UFUNCTION(BlueprintPure, Category = "SC|Slot")
	bool IsOccupied() const { return OccupiedCharacter.IsValid(); }

	/** 캐릭터 스폰 월드 위치 반환 */
	UFUNCTION(BlueprintPure, Category = "SC|Slot")
	FVector GetSpawnWorldLocation() const;

	// ── Blueprint 이벤트 ────────────────────
	/** 선택 상태 하이라이트 */
	UFUNCTION(BlueprintImplementableEvent, Category = "SC|Slot")
	void OnSelected(bool bIsSelected);
};
