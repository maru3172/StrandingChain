// Copyright StrandingChain. All Rights Reserved.
// File: Source/StrandingChain/Battle/SCTeamState.cpp

#include "SCTeamState.h"
#include "StrandingChain.h"

bool USCTeamState::TryConsumeCost(int32 Amount)
{
	if (Amount <= 0)
	{
		UE_LOG(LogStrandingChain, Warning,
			TEXT("[SCTeamState] TryConsumeCost: Amount=%d가 0 이하. 팀=%d"),
			Amount, static_cast<int32>(Team));
		return false;
	}
	if (CurrentCost < Amount)
	{
		UE_LOG(LogStrandingChain, Log,
			TEXT("[SCTeamState] 코스트 부족. 필요=%d, 보유=%d, 팀=%d"),
			Amount, CurrentCost, static_cast<int32>(Team));
		return false;
	}
	CurrentCost -= Amount;
	UE_LOG(LogStrandingChain, Log,
		TEXT("[SCTeamState] 코스트 소모: -%d, 잔여=%d, 팀=%d"),
		Amount, CurrentCost, static_cast<int32>(Team));
	OnCostChanged.Broadcast(Team, CurrentCost);
	return true;
}

void USCTeamState::AddCost(int32 Amount)
{
	if (Amount <= 0) { return; }
	CurrentCost = FMath::Min(CurrentCost + Amount, MaxCost);
	UE_LOG(LogStrandingChain, Log,
		TEXT("[SCTeamState] 코스트 획득: +%d, 현재=%d, 팀=%d"),
		Amount, CurrentCost, static_cast<int32>(Team));
	OnCostChanged.Broadcast(Team, CurrentCost);
}

void USCTeamState::ResetCost(int32 InitialAmount)
{
	CurrentCost = FMath::Clamp(InitialAmount, 0, MaxCost);
	OnCostChanged.Broadcast(Team, CurrentCost);
}
