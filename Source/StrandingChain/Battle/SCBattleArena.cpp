// Copyright StrandingChain. All Rights Reserved.
// File: Source/StrandingChain/Battle/SCBattleArena.cpp

#include "SCBattleArena.h"
#include "SCBattleSlot.h"
#include "SCCharacterBase.h"
#include "StrandingChain.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"

ASCBattleArena::ASCBattleArena()
{
	PrimaryActorTick.bCanEverTick = false;

	// 사이드뷰 고정 카메라
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(GetRootComponent());
	SpringArm->TargetArmLength = 1400.f;
	// 사이드뷰 + 살짝 위에서: Yaw=-90(Y축 방향), Pitch=-25(위에서 아래로)
	SpringArm->SetRelativeRotation(FRotator(-25.f, -90.f, 0.f));
	SpringArm->bDoCollisionTest = false;
	SpringArm->bInheritPitch = false;
	SpringArm->bInheritRoll = false;
	SpringArm->bInheritYaw = false;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;

	SetDefaultSlotOffsets();
}

void ASCBattleArena::SetDefaultSlotOffsets()
{
	// ── 마름모 진형 (X-Y 평면, 카메라 위에서 봤을 때 마름모) ──
	// 카메라: Yaw=-90(사이드), Pitch=-25(약간 위)
	// X축 = 화면 좌우(팀 구분), Y축 = 화면 깊이(앞뒤)
	//
	// TeamA 좌측 마름모:
	//   슬롯0 (선봉, 팀B 방향)
	//   슬롯1 (위쪽)   슬롯2 (아래쪽)
	//   슬롯3 (후방)
	TeamASlotOffsets =
	{
		FVector(-150.f,    0.f, 0.f),   // 0: 선봉
		FVector(-350.f, -220.f, 0.f),  // 1: 좌측
		FVector(-350.f,  220.f, 0.f),  // 2: 우측
		FVector(-550.f,    0.f, 0.f)   // 3: 후방
	};

	// TeamB 우측 마름모 (X 반전, TeamA와 마주보는 형태)
	TeamBSlotOffsets =
	{
		FVector( 150.f,    0.f, 0.f),  // 0: 선봉
		FVector( 350.f, -220.f, 0.f), // 1: 좌측
		FVector( 350.f,  220.f, 0.f), // 2: 우측
		FVector( 550.f,    0.f, 0.f)  // 3: 후방
	};
}

void ASCBattleArena::BeginPlay()
{
	Super::BeginPlay();
	SpawnSlots();
}

void ASCBattleArena::SpawnSlots()
{
	if (!IsValid(SlotClass))
	{
		UE_LOG(LogStrandingChain, Warning,
			TEXT("[SCBattleArena] SlotClass가 설정되지 않음. Blueprint에서 지정 필요."));
		return;
	}

	UWorld* World = GetWorld();
	if (!IsValid(World)) { return; }

	auto SpawnTeamSlots = [&](ESCTeam Team,
		const TArray<FVector>& Offsets,
		TArray<TObjectPtr<ASCBattleSlot>>& OutSlots)
	{
		OutSlots.Empty();
		for (int32 i = 0; i < 4; ++i)
		{
			FVector WorldLoc = GetActorLocation() + Offsets[i];
			FActorSpawnParameters Params;
			Params.Owner = this;
			Params.SpawnCollisionHandlingOverride =
				ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			ASCBattleSlot* Slot = World->SpawnActor<ASCBattleSlot>(
				SlotClass, WorldLoc, FRotator::ZeroRotator, Params);

			if (ensureMsgf(IsValid(Slot),
				TEXT("[SCBattleArena] 슬롯 스폰 실패. Team=%d, Index=%d"),
				static_cast<int32>(Team), i))
			{
				Slot->Team = Team;
				Slot->SlotIndex = i;
				OutSlots.Add(Slot);
			}
		}
		UE_LOG(LogStrandingChain, Log,
			TEXT("[SCBattleArena] 팀=%d 슬롯 %d개 스폰 완료."),
			static_cast<int32>(Team), OutSlots.Num());
	};

	SpawnTeamSlots(ESCTeam::TeamA, TeamASlotOffsets, TeamASlots);
	SpawnTeamSlots(ESCTeam::TeamB, TeamBSlotOffsets, TeamBSlots);
}

void ASCBattleArena::PlaceCharactersToSlots(
	const TArray<ASCCharacterBase*>& Characters, ESCTeam Team)
{
	TArray<TObjectPtr<ASCBattleSlot>>& Slots =
		(Team == ESCTeam::TeamA) ? TeamASlots : TeamBSlots;

	int32 PlaceCount = FMath::Min(Characters.Num(), Slots.Num());
	for (int32 i = 0; i < PlaceCount; ++i)
	{
		ASCCharacterBase* Char = Characters[i];
		ASCBattleSlot* Slot = Slots[i].Get();

		if (!IsValid(Char) || !IsValid(Slot)) { continue; }
		Slot->PlaceCharacter(Char);
	}
}

ASCBattleSlot* ASCBattleArena::GetSlot(ESCTeam Team, int32 Index) const
{
	const TArray<TObjectPtr<ASCBattleSlot>>& Slots =
		(Team == ESCTeam::TeamA) ? TeamASlots : TeamBSlots;

	if (!Slots.IsValidIndex(Index))
	{
		UE_LOG(LogStrandingChain, Warning,
			TEXT("[SCBattleArena] GetSlot: 잘못된 인덱스=%d"), Index);
		return nullptr;
	}
	return Slots[Index].Get();
}

TArray<ASCBattleSlot*> ASCBattleArena::GetTeamSlots(ESCTeam Team) const
{
	const TArray<TObjectPtr<ASCBattleSlot>>& Slots =
		(Team == ESCTeam::TeamA) ? TeamASlots : TeamBSlots;

	TArray<ASCBattleSlot*> Result;
	for (const TObjectPtr<ASCBattleSlot>& S : Slots)
	{
		if (IsValid(S)) Result.Add(S.Get());
	}
	return Result;
}
