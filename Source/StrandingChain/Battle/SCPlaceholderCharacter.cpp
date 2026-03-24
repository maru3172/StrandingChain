// Copyright StrandingChain. All Rights Reserved.
// File: Source/StrandingChain/Battle/SCPlaceholderCharacter.cpp

#include "SCPlaceholderCharacter.h"
#include "StrandingChain.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "UObject/ConstructorHelpers.h"

ASCPlaceholderCharacter::ASCPlaceholderCharacter()
{
	// 캡슐 크기
	GetCapsuleComponent()->InitCapsuleSize(35.f, 90.f);

	// 엔진 기본 실린더 메시로 캐릭터 표현 (구체보다 캐릭터처럼 보임)
	DebugMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DebugMesh"));
	DebugMesh->SetupAttachment(GetCapsuleComponent());
	DebugMesh->SetRelativeLocation(FVector(0.f, 0.f, -90.f));
	DebugMesh->SetRelativeScale3D(FVector(0.5f, 0.5f, 1.8f));
	DebugMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	DebugMesh->SetCastShadow(true);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(
		TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	if (CylinderMesh.Succeeded())
	{
		DebugMesh->SetStaticMesh(CylinderMesh.Object);
	}

	// 기본 AI 컨트롤러 없음
	AutoPossessAI = EAutoPossessAI::Disabled;

	// 테스트 기본값
	MaxHP = 100;
}

void ASCPlaceholderCharacter::BeginPlay()
{
	Super::BeginPlay();

	// 턴제 게임 — 중력/이동 없이 스폰 위치에 고정
	if (IsValid(GetCharacterMovement()))
	{
		GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);
		GetCharacterMovement()->StopMovementImmediately();
	}

	// 포지션별 색상 동적 머티리얼 적용
	if (IsValid(DebugMesh))
	{
		UMaterialInstanceDynamic* DynMat =
			DebugMesh->CreateAndSetMaterialInstanceDynamic(0);
		if (IsValid(DynMat))
		{
			DynMat->SetVectorParameterValue(TEXT("Color"), DebugColor);
		}
	}

	UE_LOG(LogStrandingChain, Log,
		TEXT("[SCPlaceholderCharacter] %s BeginPlay. Team=%d, Position=%d, HP=%d"),
		*GetName(),
		static_cast<int32>(Team),
		static_cast<int32>(Position),
		MaxHP);
}
