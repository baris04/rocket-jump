#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RocketJumpLaserBeam.generated.h"

class UBoxComponent;
class USceneComponent;
class UStaticMeshComponent;
class UPointLightComponent;
class USpotLightComponent;

UCLASS()
class ARocketJumpLaserBeam : public AActor
{
	GENERATED_BODY()

public:
	ARocketJumpLaserBeam();

	virtual void Tick(float DeltaSeconds) override;

	static ARocketJumpLaserBeam* SpawnBeam(UWorld* World, const FVector& Start, const FVector& End,
		float VisualThickness = 4.f, float TriggerThickness = 18.f, AActor* AttachTo = nullptr);

	UFUNCTION()
	void OnLaserOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UBoxComponent> TriggerBox;

	/** Ince parlak merkez cizgi */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> BeamMesh;

	/** Yumusak dis hale */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> GlowMesh;

	/** Fiziksel lazer cihazi kutusu (sabit duran gövde) */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> EmitterHousingMesh;

	/** Cihazin nazlu/apeture parcasi — nabiz gibi titreyen glow */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> EmitterMesh;

	/** Lazer boyunca ortadaki ambient isik */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UPointLightComponent> LaserLight;

	/** Kaynak ucundaki guclu isik (lazer buradan cikar gibi) */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UPointLightComponent> SourceLight;

	/** SpotLight: kaynaktan hedef yonune dogru isik huzesi */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USpotLightComponent> BeamSpot;

	void ConfigureBeam(const FVector& Start, const FVector& End,
		float VisualThickness, float TriggerThickness);

	/** Isini konumunu/donusunu yeniden hesaplar (Sweep Tick'te kullanir). */
	void UpdateBeamTransform(const FVector& Start, const FVector& End);

	// --- Titreme (pulse) ---
	float PulseTime = 0.f;
	float BeamLength = 0.f;

	// --- Tarama hareketi (sweep) ---
	/** Acilirken ConfigureBeam tarafindan ayarlanir; false yaparak durdurulabilir. */
	bool bSweepBeam = true;

	/** Iki yone maksimum acilma (derece). */
	float SweepAmplitude = 38.f;

	/** Tarama hizi (daha buyuk = daha hizli). */
	float SweepSpeed = 0.7f;

	float SweepTime = 0.f;

	FVector BeamSourcePosition  = FVector::ZeroVector;
	FVector BeamBaseDirection   = FVector::ForwardVector;
	float StoredVisualThickness = 4.f;
	float StoredTriggerThickness = 18.f;
};
