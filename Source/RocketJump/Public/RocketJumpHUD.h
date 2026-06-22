#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "RocketJumpHUD.generated.h"

UCLASS()
class ARocketJumpHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;

	UPROPERTY(EditAnywhere, Category = "RocketJump|UI")
	FLinearColor HealthBarColor = FLinearColor(0.15f, 0.85f, 0.25f, 1.f);

	UPROPERTY(EditAnywhere, Category = "RocketJump|UI")
	FLinearColor HealthBarBgColor = FLinearColor(0.1f, 0.1f, 0.1f, 0.75f);

	UPROPERTY(EditAnywhere, Category = "RocketJump|UI")
	FLinearColor DamageFlashColor = FLinearColor(0.9f, 0.1f, 0.1f, 0.35f);
};
