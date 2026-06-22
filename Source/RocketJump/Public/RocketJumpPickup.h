#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RocketJumpPickup.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UPointLightComponent;

/**
 * Havada yüzen enerji içeceği collectible.
 * Oyuncu alırca CharacterMovement JumpMaxCount = 2 → double jump kazanir.
 * DurationSeconds sonra (0 = kalici) double jump iptal edilir.
 */
UCLASS()
class ARocketJumpPickup : public AActor
{
	GENERATED_BODY()

public:
	ARocketJumpPickup();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	/** Efektin ne kadar sure gecerli oldugu (saniye). 0 = süresiz. */
	UPROPERTY(EditAnywhere, Category = "Pickup")
	float DurationSeconds = 15.f;

	/** Havada yukarı aşağı salınım yüksekliği (cm). */
	UPROPERTY(EditAnywhere, Category = "Pickup")
	float BobAmplitude = 12.f;

	/** Döner hız (derece/saniye). */
	UPROPERTY(EditAnywhere, Category = "Pickup")
	float SpinSpeed = 90.f;

	/** Alındıktan kaç saniye sonra yeniden belirir (0 = bir kez). */
	UPROPERTY(EditAnywhere, Category = "Pickup")
	float RespawnDelay = 12.f;

	UFUNCTION()
	void OnPickupOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

private:
	UPROPERTY(VisibleAnywhere) TObjectPtr<USphereComponent>       TriggerSphere;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent>   CanMesh;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent>   GlowRing;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UPointLightComponent>   PickupLight;

	float  BobTime     = 0.f;
	float  SpinTime    = 0.f;
	FVector BaseLocation;

	void GrantDoubleJump(APawn* Pawn);
	void RevokeDoubleJump(APawn* Pawn);
	void ShowPickup();

	FTimerHandle RevokeTimer;
	FTimerHandle RespawnTimer;
	TWeakObjectPtr<APawn> GrantedPawn;
};
