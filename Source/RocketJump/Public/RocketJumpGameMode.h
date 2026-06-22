#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "RocketJumpGameMode.generated.h"

UCLASS()
class ARocketJumpGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ARocketJumpGameMode();

	virtual void StartPlay() override;

	UPROPERTY(EditAnywhere, Category = "RocketJump|Level")
	bool bAutoSetupLevel1Turrets = true;

	UPROPERTY(EditAnywhere, Category = "RocketJump|Level")
	float TurretHeightAbovePlatform = 80.f;
};
