#pragma once

#include "Components/ActorComponent.h"
#include "RocketJumpRotatingPlatformComponent.generated.h"

/** Sahibini kendi ekseninde (Yaw) dondurur. */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class URocketJumpRotatingPlatformComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	URocketJumpRotatingPlatformComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RocketJump|Platform")
	float RotationSpeedDegreesPerSecond = 22.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RocketJump|Platform")
	bool bRotateYaw = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RocketJump|Platform")
	bool bRotatePitch = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RocketJump|Platform")
	bool bRotateRoll = false;
};
