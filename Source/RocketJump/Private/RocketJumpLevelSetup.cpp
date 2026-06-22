#include "RocketJumpLevelSetup.h"

#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "EngineUtils.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerStart.h"
#include "RocketJumpLaserBeam.h"
#include "RocketJumpRotatingPlatformComponent.h"
#include "RocketJumpTurret.h"
#include "UObject/ConstructorHelpers.h"

ARocketJumpLevelSetup::ARocketJumpLevelSetup()
{
	PrimaryActorTick.bCanEverTick = false;

	static ConstructorHelpers::FClassFinder<ARocketJumpTurret> TurretFinder(TEXT("/Script/RocketJump.RocketJumpTurret"));
	if (TurretFinder.Succeeded())
	{
		TurretClass = TurretFinder.Class;
	}
}

void ARocketJumpLevelSetup::BeginPlay()
{
	Super::BeginPlay();

	if (bEnableRotatingPlatforms)
	{
		SetupRotatingPlatforms();
	}

	SpawnTurretsOnPlatforms();

	if (bEnableSpyLasers)
	{
		SpawnSpyLasers();
	}

	Destroy();
}

TArray<AActor*> ARocketJumpLevelSetup::CollectLevelAssets() const
{
	TArray<AActor*> Assets;
	if (!GetWorld())
	{
		return Assets;
	}

	for (TActorIterator<AActor> It(GetWorld()); It; ++It)
	{
		if (IsRotatableLevelAsset(*It))
		{
			Assets.Add(*It);
		}
	}
	return Assets;
}

bool ARocketJumpLevelSetup::IsRotatableLevelAsset(AActor* Actor) const
{
	if (!Actor || Actor == this)
	{
		return false;
	}

	// Sistem aktorleri atla
	if (Actor->IsA<APlayerStart>() || Actor->IsA<APawn>() ||
		Actor->IsA<ARocketJumpTurret>() || Actor->IsA<ARocketJumpLaserBeam>())
	{
		return false;
	}

	if (Actor->ActorHasTag(FName("NoRotate")))
	{
		return false;
	}

	// Tag veya isim eslesiyorsa her zaman al
	if (Actor->ActorHasTag(PlatformActorTag))
	{
		return true;
	}

	if (bAlsoSpawnOnActorsNamedPlatform)
	{
		const FString Label = Actor->GetActorLabel();
		if (Label.Contains(TEXT("Platform"), ESearchCase::IgnoreCase) ||
			Label.Contains(TEXT("Kat"), ESearchCase::IgnoreCase))
		{
			return true;
		}
	}

	if (!bRotateAllStaticMeshAssets)
	{
		return false;
	}

	// StaticMesh component zorunlu
	const UStaticMeshComponent* MeshComp = Actor->FindComponentByClass<UStaticMeshComponent>();
	if (!MeshComp || !MeshComp->GetStaticMesh())
	{
		return false;
	}

	FVector Origin;
	FVector Extent;
	Actor->GetActorBounds(false, Origin, Extent);
	const float MaxE = Extent.GetMax();

	if (MaxE < MinRotatableExtent || MaxE > MaxRotatableExtent)
	{
		return false;
	}

	// Cok genis yatay zemin duzlemini atla
	if (Extent.Z < 60.f && (Extent.X > 2000.f || Extent.Y > 2000.f))
	{
		return false;
	}

	return true;
}

void ARocketJumpLevelSetup::GetPlatformBounds(AActor* Actor, FVector& OutOrigin, FVector& OutExtent) const
{
	Actor->GetActorBounds(false, OutOrigin, OutExtent);
}

FVector ARocketJumpLevelSetup::GetPlatformTopCenter(AActor* Actor) const
{
	FVector Origin;
	FVector Extent;
	GetPlatformBounds(Actor, Origin, Extent);
	return FVector(Origin.X, Origin.Y, Origin.Z + Extent.Z);
}

void ARocketJumpLevelSetup::AddRotatorToActor(AActor* Actor)
{
	if (!Actor || Actor->FindComponentByClass<URocketJumpRotatingPlatformComponent>())
	{
		return;
	}

	URocketJumpRotatingPlatformComponent* Rotator = NewObject<URocketJumpRotatingPlatformComponent>(Actor);
	Rotator->RotationSpeedDegreesPerSecond = FMath::FRandRange(MinPlatformRotationSpeed, MaxPlatformRotationSpeed);
	Rotator->bRotateYaw = true;
	Actor->AddInstanceComponent(Rotator);
	Rotator->RegisterComponent();
	Rotator->Activate(true);
}

void ARocketJumpLevelSetup::SetupRotatingPlatforms()
{
	for (AActor* Asset : CollectLevelAssets())
	{
		AddRotatorToActor(Asset);
	}
}

void ARocketJumpLevelSetup::SpawnTurretsOnPlatforms()
{
	if (!GetWorld() || !TurretClass)
	{
		return;
	}

	for (AActor* Actor : CollectLevelAssets())
	{
		FVector Origin;
		FVector Extent;
		GetPlatformBounds(Actor, Origin, Extent);
		if (Extent.GetMax() < MinExtentForTurret)
		{
			continue;
		}

		const FVector SpawnLoc = GetPlatformTopCenter(Actor) + FVector(0.f, 0.f, TurretHeightAbovePlatform);
		const FRotator SpawnRot(0.f, FMath::FRandRange(0.f, 360.f), 0.f);

		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		if (ARocketJumpTurret* Turret = GetWorld()->SpawnActor<ARocketJumpTurret>(TurretClass, SpawnLoc, SpawnRot, Params))
		{
			Turret->AttachToActor(Actor, FAttachmentTransformRules::KeepWorldTransform);
		}
	}
}

void ARocketJumpLevelSetup::SpawnLasersOnPlatform(AActor* Platform, const FVector& Origin, const FVector& Extent)
{
	const float LaserZ = Origin.Z + Extent.Z + LaserHeightAbovePlatform;
	const float SpanX = FMath::Max(Extent.X * 0.85f, 60.f);
	const float SpanY = FMath::Max(Extent.Y * 0.85f, 60.f);

	for (int32 i = 0; i < LaserLinesPerPlatform; ++i)
	{
		const float T = LaserLinesPerPlatform > 1 ? static_cast<float>(i) / static_cast<float>(LaserLinesPerPlatform - 1) : 0.5f;
		const float OffsetY = FMath::Lerp(-SpanY, SpanY, T);

		const FVector StartA(Origin.X - SpanX, Origin.Y + OffsetY, LaserZ);
		const FVector EndA(Origin.X + SpanX, Origin.Y + OffsetY, LaserZ);
		ARocketJumpLaserBeam::SpawnBeam(GetWorld(), StartA, EndA, LaserVisualThickness, LaserTriggerThickness, Platform);

		const float OffsetX = FMath::Lerp(-SpanX, SpanX, T);
		const FVector StartB(Origin.X + OffsetX, Origin.Y - SpanY, LaserZ);
		const FVector EndB(Origin.X + OffsetX, Origin.Y + SpanY, LaserZ);
		ARocketJumpLaserBeam::SpawnBeam(GetWorld(), StartB, EndB, LaserVisualThickness, LaserTriggerThickness, Platform);
	}
}

void ARocketJumpLevelSetup::SpawnGapLasersBetweenPlatforms(const TArray<AActor*>& Platforms)
{
	if (Platforms.Num() < 2)
	{
		return;
	}

	const float MaxDistSq = FMath::Square(MaxGapLaserDistance);

	for (int32 i = 0; i < Platforms.Num(); ++i)
	{
		for (int32 j = i + 1; j < Platforms.Num(); ++j)
		{
			const FVector TopI = GetPlatformTopCenter(Platforms[i]);
			const FVector TopJ = GetPlatformTopCenter(Platforms[j]);
			const float DistSq = FVector::DistSquared2D(TopI, TopJ);
			if (DistSq > MaxDistSq || DistSq < FMath::Square(120.f))
			{
				continue;
			}

			const float LaserZ = FMath::Max(TopI.Z, TopJ.Z) + LaserHeightAbovePlatform * 0.5f;
			const FVector Start(TopI.X, TopI.Y, LaserZ);
			const FVector End(TopJ.X, TopJ.Y, LaserZ);
			ARocketJumpLaserBeam::SpawnBeam(GetWorld(), Start, End, LaserVisualThickness, LaserTriggerThickness);
		}
	}
}

void ARocketJumpLevelSetup::SpawnSpyLasers()
{
	const TArray<AActor*> Assets = CollectLevelAssets();

	// PlayerStart konumlarini topla — bu platformlara lazer koyma
	TArray<FVector> SpawnPoints;
	for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
	{
		SpawnPoints.Add(It->GetActorLocation());
	}

	for (AActor* Asset : Assets)
	{
		FVector Origin;
		FVector Extent;
		GetPlatformBounds(Asset, Origin, Extent);
		if (Extent.GetMax() < MinExtentForTurret * 0.5f)
		{
			continue;
		}

		// Oyuncu baslangic noktasina cok yakin platformlari atla
		bool bNearSpawn = false;
		for (const FVector& SP : SpawnPoints)
		{
			if (FVector::Dist2D(Origin, SP) < 550.f)
			{
				bNearSpawn = true;
				break;
			}
		}
		if (bNearSpawn) continue;

		SpawnLasersOnPlatform(Asset, Origin, Extent);
	}

	if (bSpawnLasersBetweenPlatforms)
	{
		SpawnGapLasersBetweenPlatforms(Assets);
	}
}
