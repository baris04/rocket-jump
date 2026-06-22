#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RocketJumpProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UStaticMeshComponent;
class USoundBase;
class UParticleSystem;

UCLASS()
class ARocketJumpProjectile : public AActor
{
	GENERATED_BODY()

public:
	ARocketJumpProjectile();

	void InitializeRocket(
		APawn* InOwner,
		const FVector& InitialVelocity,
		float InExplosionRadius,
		float InExplosionStrength,
		float InFalloffExponent,
		float InGravityScale,
		float InMaxFlightSeconds,
		USoundBase* InExplosionSound,
		UParticleSystem* InExplosionFX,
		bool bInOverrideXY,
		bool bInOverrideZ,
		bool bInApplySelfDamage,
		float InSelfDamageMax,
		float InSelfDamageMin);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RocketJump")
	TObjectPtr<USphereComponent> CollisionComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RocketJump")
	TObjectPtr<UStaticMeshComponent> MeshComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RocketJump")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	UFUNCTION()
	void OnProjectileHit(
		UPrimitiveComponent* HitComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		FVector NormalImpulse,
		const FHitResult& Hit);

	void Explode(const FVector& ExplosionLocation, const FVector& SurfaceNormal);
	void LifetimeExpired();

	bool bExploded = false;

	float ExplosionRadius = 450.f;
	float ExplosionLaunchStrength = 2450.f;
	float ExplosionFalloffExponent = 1.45f;

	bool bApplySelfDamage = true;
	float SelfDamageMax = 22.f;
	float SelfDamageMin = 6.f;

	TWeakObjectPtr<APawn> FiringPawn;
	bool bOverrideXY = false;
	bool bOverrideZ = true;

	TObjectPtr<USoundBase> ExplosionSound = nullptr;
	TObjectPtr<UParticleSystem> ExplosionFX = nullptr;

	FTimerHandle LifetimeTimer;
};
