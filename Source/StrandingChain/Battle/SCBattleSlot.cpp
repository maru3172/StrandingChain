// Copyright StrandingChain. All Rights Reserved.
// File: Source/StrandingChain/Battle/SCBattleSlot.cpp

#include "SCBattleSlot.h"
#include "SCCharacterBase.h"
#include "StrandingChain.h"

ASCBattleSlot::ASCBattleSlot()
{
	PrimaryActorTick.bCanEverTick = false;

	PlatformMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlatformMesh"));
	SetRootComponent(PlatformMesh);

	CharacterSpawnPoint = CreateDefaultSubobject<USceneComponent>(TEXT("CharacterSpawnPoint"));
	CharacterSpawnPoint->SetupAttachment(PlatformMesh);
	// 발판 위 기본 오프셋 (Blueprint에서 조정 가능)
	CharacterSpawnPoint->SetRelativeLocation(FVector(0.f, 0.f, 50.f));
}

void ASCBattleSlot::PlaceCharacter(ASCCharacterBase* Character)
{
	if (!IsValid(Character))
	{
		UE_LOG(LogStrandingChain, Warning,
			TEXT("[SCBattleSlot][%s] PlaceCharacter: 유효하지 않은 캐릭터."), *GetName());
		return;
	}
	OccupiedCharacter = Character;
	Character->SetActorLocation(GetSpawnWorldLocation());
	Character->SetActorRotation(GetActorRotation());

	UE_LOG(LogStrandingChain, Log,
		TEXT("[SCBattleSlot] 슬롯[팀=%d, 인덱스=%d]에 %s 배치 완료."),
		static_cast<int32>(Team), SlotIndex, *Character->GetName());
}

void ASCBattleSlot::ClearSlot()
{
	OccupiedCharacter.Reset();
}

FVector ASCBattleSlot::GetSpawnWorldLocation() const
{
	if (IsValid(CharacterSpawnPoint))
		return CharacterSpawnPoint->GetComponentLocation();
	return GetActorLocation();
}
