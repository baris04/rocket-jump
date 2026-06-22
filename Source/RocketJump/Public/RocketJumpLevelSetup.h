#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RocketJumpLevelSetup.generated.h"

UCLASS(NotPlaceable)
class ARocketJumpLevelSetup : public AActor
{
	GENERATED_BODY()

public:
	ARocketJumpLevelSetup();

	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "RocketJump|Level")
	float TurretHeightAbovePlatform = 80.f;

	UPROPERTY(EditAnywhere, Category = "RocketJump|Level")
	FName PlatformActorTag = TEXT("RocketJumpPlatform");

	UPROPERTY(EditAnywhere, Category = "RocketJump|Level")
	bool bAlsoSpawnOnActorsNamedPlatform = true;

	/** level1'deki StaticMesh actor'larini otomatik dondur (Platform adi sart degil). */
	UPROPERTY(EditAnywhere, Category = "RocketJump|Level")
	bool bRotateAllStaticMeshAssets = true;

	UPROPERTY(EditAnywhere, Category = "RocketJump|Level", meta = (ClampMin = "10.0"))
	float MinRotatableExtent = 25.f;

	UPROPERTY(EditAnywhere, Category = "RocketJump|Level", meta = (ClampMin = "100.0"))
	float MaxRotatableExtent = 12000.f;

	UPROPERTY(EditAnywhere, Category = "RocketJump|Level", meta = (ClampMin = "40.0"))
	float MinExtentForTurret = 60.f;

	UPROPERTY(EditAnywhere, Category = "RocketJump|Level")
	TSubclassOf<class ARocketJumpTurret> TurretClass;

	UPROPERTY(EditAnywhere, Category = "RocketJump|Level|Rotation")
	bool bEnableRotatingPlatforms = false;

	UPROPERTY(EditAnywhere, Category = "RocketJump|Level|Rotation", meta = (ClampMin = "1.0"))
	float MinPlatformRotationSpeed = 10.f;

	UPROPERTY(EditAnywhere, Category = "RocketJump|Level|Rotation", meta = (ClampMin = "1.0"))
	float MaxPlatformRotationSpeed = 32.f;

	UPROPERTY(EditAnywhere, Category = "RocketJump|Level|Lasers")
	bool bEnableSpyLasers = true;

	UPROPERTY(EditAnywhere, Category = "RocketJump|Level|Lasers", meta = (ClampMin = "20.0"))
	float LaserHeightAbovePlatform = 80.f;

	UPROPERTY(EditAnywhere, Category = "RocketJump|Level|Lasers", meta = (ClampMin = "1"))
	int32 LaserLinesPerPlatform = 2;

	UPROPERTY(EditAnywhere, Category = "RocketJump|Level|Lasers", meta = (ClampMin = "1.0"))
	float LaserVisualThickness = 4.f;

	UPROPERTY(EditAnywhere, Category = "RocketJump|Level|Lasers", meta = (ClampMin = "8.0"))
	float LaserTriggerThickness = 20.f;

	UPROPERTY(EditAnywhere, Category = "RocketJump|Level|Lasers")
	bool bSpawnLasersBetweenPlatforms = true;

	UPROPERTY(EditAnywhere, Category = "RocketJump|Level|Lasers", meta = (ClampMin = "200.0"))
	float MaxGapLaserDistance = 3500.f;

protected:
	void SetupRotatingPlatforms();
	void SpawnTurretsOnPlatforms();
	void SpawnSpyLasers();
	void SpawnLasersOnPlatform(AActor* Platform, const FVector& Origin, const FVector& Extent);
	void SpawnGapLasersBetweenPlatforms(const TArray<AActor*>& Platforms);
	void AddRotatorToActor(AActor* Actor);

	bool IsRotatableLevelAsset(AActor* Actor) const;
	FVector GetPlatformTopCenter(AActor* Actor) const;
	void GetPlatformBounds(AActor* Actor, FVector& OutOrigin, FVector& OutExtent) const;
	TArray<AActor*> CollectLevelAssets() const;
};
