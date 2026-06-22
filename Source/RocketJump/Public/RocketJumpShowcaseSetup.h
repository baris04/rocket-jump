#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RocketJumpShowcaseSetup.generated.h"

class AStaticMeshActor;
class ARocketJumpPickup;
class ARocketJumpTurret;

/**
 * Bos bir levele yerlestirildiginde otomatik olarak:
 *  - 5 kademeli platform spawnar (Cube mesh, kosiniz collision)
 *  - PlayerStart noktasi ekler
 *  - Her platformun ustune taret ve/veya lazer koyar
 * 
 * Kullanim:
 *  1. Unreal Editor'da yeni bos level ac
 *  2. Bu aktoru sahneye suru-birak
 *  3. World Settings -> GameMode -> RocketJumpGameMode sec
 *  4. Play
 */
UCLASS()
class ARocketJumpShowcaseSetup : public AActor
{
	GENERATED_BODY()

public:
	ARocketJumpShowcaseSetup();
	virtual void BeginPlay() override;

	/** Kac platform olusturulsun (max 8). */
	UPROPERTY(EditAnywhere, Category = "Showcase", meta = (ClampMin = "2", ClampMax = "8"))
	int32 PlatformCount = 5;

	/** Platformlar arasi yatay mesafe (cm). */
	UPROPERTY(EditAnywhere, Category = "Showcase")
	float PlatformSpacing = 680.f;

	/** Her platformun bir öncekinden yüksekligi (cm). */
	UPROPERTY(EditAnywhere, Category = "Showcase")
	float PlatformHeightStep = 110.f;

	/** Platformlar zigzag mi gitsin (daha görsel). */
	UPROPERTY(EditAnywhere, Category = "Showcase")
	bool bZigzag = true;

	/** Taret ve lazerleri de olustur. */
	UPROPERTY(EditAnywhere, Category = "Showcase")
	bool bSpawnEnemies = true;

private:
	AStaticMeshActor* SpawnPlatform(const FVector& Location, const FVector& Scale);
	void              SpawnTurretOn(AStaticMeshActor* Platform);
	void              SpawnLaserOn(AStaticMeshActor* Platform);

	TSubclassOf<ARocketJumpTurret> TurretClass;

	// Constructor'da yüklenen assetler
	TObjectPtr<UStaticMesh>      PlatformMesh;
	TObjectPtr<UMaterialInterface> PlatformMaterial;
};
