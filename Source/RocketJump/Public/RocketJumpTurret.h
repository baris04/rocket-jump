#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RocketJumpTurret.generated.h"

class UStaticMeshComponent;
class ATurretProjectile;

UCLASS()
class ARocketJumpTurret : public AActor
{
	GENERATED_BODY()

public:
	ARocketJumpTurret();

	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(EditAnywhere, Category = "RocketJump|Turret", meta = (ClampMin = "0.2"))
	float FireInterval = 1.35f;

	UPROPERTY(EditAnywhere, Category = "RocketJump|Turret", meta = (ClampMin = "50.0"))
	float ProjectileSpeed = 480.f;

	UPROPERTY(EditAnywhere, Category = "RocketJump|Turret", meta = (ClampMin = "1.0"))
	float ProjectileDamage = 14.f;

	/** Mermi mesh olcegi (1 = buyuk kure, ~0.3 kucuk). */
	UPROPERTY(EditAnywhere, Category = "RocketJump|Turret", meta = (ClampMin = "0.08"))
	float ProjectileVisualScale = 0.85f;

	UPROPERTY(EditAnywhere, Category = "RocketJump|Turret", meta = (ClampMin = "100.0"))
	float MaxEngageDistance = 6000.f;

	UPROPERTY(EditAnywhere, Category = "RocketJump|Turret", meta = (ClampMin = "100.0"))
	float MinEngageDistance = 200.f;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> BaseMesh;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> HeadMesh;

	void TryFireAtTarget(APawn* Target);
	void UpdateHeadRecoil(float DeltaSeconds);

	float FireCooldown = 0.f;
	float HeadRecoilKick = 0.f;
	FVector HeadMeshBaseRelativeLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, Category = "RocketJump|Turret")
	TSubclassOf<ATurretProjectile> TurretProjectileClass;
};
