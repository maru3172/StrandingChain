// Copyright StrandingChain. All Rights Reserved.
// File: Source/StrandingChain/Battle/SCBattlePlayerController.cpp

#include "SCBattlePlayerController.h"
#include "SCBattleArena.h"
#include "SCCharacterBase.h"
#include "StrandingChain.h"
#include "UI/SCSkillPanelWidget.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"

ASCBattlePlayerController::ASCBattlePlayerController()
{
	PrimaryActorTick.bCanEverTick = true;
	bShowMouseCursor = true;
}

void ASCBattlePlayerController::BeginPlay()
{
	Super::BeginPlay();

	bShowMouseCursor   = true;
	bEnableClickEvents = true;
	SetInputMode(FInputModeGameAndUI());

	// 아레나 참조 캐싱
	AActor* Found = UGameplayStatics::GetActorOfClass(
		GetWorld(), ASCBattleArena::StaticClass());
	if (IsValid(Found))
	{
		ArenaRef = Cast<ASCBattleArena>(Found);
		UE_LOG(LogStrandingChain, Log,
			TEXT("[SCBattlePC] 아레나 참조: %s"), *Found->GetName());
	}

	// 스킬 패널 위젯 생성 (숨김 상태로 Viewport에 추가)
	if (IsValid(SkillPanelClass))
	{
		SkillPanelWidget = CreateWidget<USCSkillPanelWidget>(this, SkillPanelClass);
		if (IsValid(SkillPanelWidget))
		{
			SkillPanelWidget->AddToViewport(0);
			SkillPanelWidget->SetVisibility(ESlateVisibility::Hidden);
			UE_LOG(LogStrandingChain, Log,
				TEXT("[SCBattlePC] SkillPanelWidget 생성 완료."));
		}
	}
	else
	{
		UE_LOG(LogStrandingChain, Warning,
			TEXT("[SCBattlePC] SkillPanelClass 미설정. "
				 "BP_BattlePlayerController의 SkillPanelClass에 WBP_SkillPanel 지정 필요."));
	}
}

void ASCBattlePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	if (!IsValid(InputComponent)) { return; }

	InputComponent->BindAxis("SC_CameraMove",
		this, &ASCBattlePlayerController::OnCameraMoveAxis);

	InputComponent->BindAction("SC_ZoomIn",  IE_Pressed,
		this, &ASCBattlePlayerController::ZoomIn);
	InputComponent->BindAction("SC_ZoomOut", IE_Pressed,
		this, &ASCBattlePlayerController::ZoomOut);

	// 마우스 좌클릭 — 캐릭터 선택
	InputComponent->BindAction("SC_Select", IE_Pressed,
		this, &ASCBattlePlayerController::OnMouseLeftClick);

	// 기본 Pawn 이동 입력 차단
	InputComponent->BindAxis("MoveForward");
	InputComponent->BindAxis("MoveRight");
	InputComponent->BindAxis("MoveUp");
}

void ASCBattlePlayerController::OnCameraMoveAxis(float Value)
{
	MoveAxisValue = Value;
}

void ASCBattlePlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (FMath::IsNearlyZero(MoveAxisValue)) { return; }

	ASCBattleArena* Arena = ArenaRef.Get();
	if (!IsValid(Arena)) { return; }

	FVector Loc = Arena->GetActorLocation();
	Loc.X += MoveAxisValue * CameraMoveSpeed * DeltaTime;
	Arena->SetActorLocation(Loc);
}

void ASCBattlePlayerController::ZoomIn()
{
	ASCBattleArena* Arena = ArenaRef.Get();
	if (!IsValid(Arena) || !IsValid(Arena->SpringArm)) { return; }

	Arena->SpringArm->TargetArmLength = FMath::Clamp(
		Arena->SpringArm->TargetArmLength - ZoomStep, ZoomMin, ZoomMax);
}

void ASCBattlePlayerController::ZoomOut()
{
	ASCBattleArena* Arena = ArenaRef.Get();
	if (!IsValid(Arena) || !IsValid(Arena->SpringArm)) { return; }

	Arena->SpringArm->TargetArmLength = FMath::Clamp(
		Arena->SpringArm->TargetArmLength + ZoomStep, ZoomMin, ZoomMax);
}

void ASCBattlePlayerController::OnMouseLeftClick()
{
	// 패널이 열려있으면 먼저 닫기
	if (IsValid(SkillPanelWidget) &&
		SkillPanelWidget->GetVisibility() == ESlateVisibility::Visible)
	{
		CloseSkillPanel();
		return;
	}

	TrySelectCharacter();
}

void ASCBattlePlayerController::TrySelectCharacter()
{
	FHitResult HitResult;
	GetHitResultUnderCursor(ECC_Visibility, false, HitResult);

	if (!HitResult.bBlockingHit) { return; }

	ASCCharacterBase* HitChar = Cast<ASCCharacterBase>(HitResult.GetActor());
	if (!IsValid(HitChar)) { return; }

	// 아군(TeamA)만 선택 가능
	if (HitChar->Team != ESCTeam::TeamA)
	{
		UE_LOG(LogStrandingChain, Log,
			TEXT("[SCBattlePC] 적팀 캐릭터 클릭 — 무시."));
		return;
	}

	if (HitChar->IsDead())
	{
		UE_LOG(LogStrandingChain, Log,
			TEXT("[SCBattlePC] 사망한 캐릭터 클릭 — 무시."));
		return;
	}

	SelectedCharacter = HitChar;
	OpenSkillPanel(HitChar);
}

void ASCBattlePlayerController::OpenSkillPanel(ASCCharacterBase* InCharacter)
{
	if (!IsValid(SkillPanelWidget))
	{
		UE_LOG(LogStrandingChain, Error,
			TEXT("[SCBattlePC] SkillPanelWidget이 유효하지 않음."));
		return;
	}

	if (InCharacter->DrawnSkills.IsEmpty())
	{
		UE_LOG(LogStrandingChain, Log,
			TEXT("[SCBattlePC] DrawnSkills 비어있음 — BeginTurn 호출 후 패널 열기."));
		InCharacter->BeginTurn();
	}

	SkillPanelWidget->OpenPanel(InCharacter);

	UE_LOG(LogStrandingChain, Log,
		TEXT("[SCBattlePC] 스킬 패널 열림: %s"), *InCharacter->GetName());
}

void ASCBattlePlayerController::CloseSkillPanel()
{
	if (!IsValid(SkillPanelWidget)) { return; }
	SkillPanelWidget->ClosePanel();
	SelectedCharacter.Reset();
}
