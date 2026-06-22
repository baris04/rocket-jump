#include "RocketJumpShowcaseSetup.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "GameFramework/PlayerStart.h"
#include "Materials/MaterialInterface.h"
#include "RocketJumpLaserBeam.h"
#include "RocketJumpPickup.h"
#include "RocketJumpTurret.h"
#include "UObject/ConstructorHelpers.h"

ARocketJumpShowcaseSetup::ARocketJumpShowcaseSetup()
{
	PrimaryActorTick.bCanEverTick = false;

	// Taret sinifi
	static ConstructorHelpers::FClassFinder<ARocketJumpTurret> TurretFinder(
		TEXT("/Script/RocketJump.RocketJumpTurret"));
	if (TurretFinder.Succeeded())
	{
		TurretClass = TurretFinder.Class;
	}

	// Platform mesh — her zaman mevcut temel sekil
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(
		TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeFinder.Succeeded())
	{
		PlatformMesh = CubeFinder.Object;
	}

	// Malzeme: once prototip grid, yoksa temel materyal
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> GridMat(
		TEXT("/Game/LevelPrototyping/Materials/MI_PrototypeGrid_Gray.MI_PrototypeGrid_Gray"));
	if (GridMat.Succeeded())
	{
		PlatformMaterial = GridMat.Object;
	}
	else
	{
		static ConstructorHelpers::FObjectFinder<UMaterialInterface> FallbackMat(
			TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
		if (FallbackMat.Succeeded())
		{
			PlatformMaterial = FallbackMat.Object;
		}
	}
}

void ARocketJumpShowcaseSetup::BeginPlay()
{
	Super::BeginPlay();
	if (!GetWorld()) return;

	const FVector Origin = GetActorLocation();

	// ------------------------------------------------------------------
	// 1. Genis baslangic platformu (güvenli alan, dusmanlar yok)
	// ------------------------------------------------------------------
	AStaticMeshActor* StartPad = SpawnPlatform(Origin, FVector(8.f, 8.f, 0.8f));
	if (StartPad)
	{
		// "Platform" tag'ini kaldir ki taret/lazer yerlesmesin
		StartPad->Tags.Remove(FName("Platform"));
	}

	// PlayerStart: baslangic platformu uzerinde
	{
		FActorSpawnParameters PSP;
		PSP.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		GetWorld()->SpawnActor<APlayerStart>(
			APlayerStart::StaticClass(),
			Origin + FVector(0.f, 0.f, 130.f),
			FRotator(0.f, 0.f, 0.f),
			PSP);
	}

	// Başlangıç platformu üzerine ilk pickup (double jump ver)
	if (StartPad)
	{
		FVector Orig, Ext;
		StartPad->GetActorBounds(false, Orig, Ext);
		FActorSpawnParameters PP;
		PP.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		GetWorld()->SpawnActor<ARocketJumpPickup>(
			ARocketJumpPickup::StaticClass(),
			FVector(Orig.X + 120.f, Orig.Y, Orig.Z + Ext.Z + 70.f),
			FRotator::ZeroRotator, PP);
	}

	// ------------------------------------------------------------------
	// 2. Parkur platformlari (kademeli yukselme)
	// ------------------------------------------------------------------
	const int32 Count = FMath::Clamp(PlatformCount, 2, 8);

	// Her platform icin dusmanlik tipi: 0=turret, 1=laser, 2=ikisi, 3=hicbiri
	const int32 EnemyPattern[] = { 3, 0, 1, 2, 0, 1, 2, 0 };

	TArray<AStaticMeshActor*> Platforms;
	float CurX = PlatformSpacing;
	float CurZ = PlatformHeightStep;

	for (int32 i = 0; i < Count; ++i)
	{
		// Zigzag yan kayma
		const float ZigY = bZigzag ? (i % 2 == 0 ? -240.f : 240.f) : 0.f;
		const FVector Loc = Origin + FVector(CurX, ZigY, CurZ);

		// Giderek kücülen platform boyutu (ilk büyük, son orta)
		const float SzLerp = static_cast<float>(i) / FMath::Max(Count - 1, 1);
		const float SzXY   = FMath::Lerp(4.8f, 3.2f, SzLerp);
		AStaticMeshActor* P = SpawnPlatform(Loc, FVector(SzXY, SzXY, 0.6f));
		if (P) Platforms.Add(P);

		CurX += PlatformSpacing;
		CurZ += PlatformHeightStep;
	}

	// ------------------------------------------------------------------
	// 3. Zirve platformu (genis, tum mekankler)
	// ------------------------------------------------------------------
	{
		const FVector TopLoc = Origin + FVector(CurX, 0.f, CurZ);
		AStaticMeshActor* Top = SpawnPlatform(TopLoc, FVector(7.f, 7.f, 0.8f));
		if (Top)
		{
			Platforms.Add(Top);

			// Zirveye ikinci pickup
			FVector Orig, Ext;
			Top->GetActorBounds(false, Orig, Ext);
			FActorSpawnParameters PP;
			PP.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			GetWorld()->SpawnActor<ARocketJumpPickup>(
				ARocketJumpPickup::StaticClass(),
				FVector(Orig.X, Orig.Y, Orig.Z + Ext.Z + 70.f),
				FRotator::ZeroRotator, PP);
		}
	}

	// ------------------------------------------------------------------
	// 4. Taret / lazer yerlesimi
	// ------------------------------------------------------------------
	if (bSpawnEnemies)
	{
		for (int32 i = 0; i < Platforms.Num(); ++i)
		{
			AStaticMeshActor* P = Platforms[i];
			if (!P) continue;

			const int32 Type = EnemyPattern[i % 8];
			if (Type == 0 || Type == 2) SpawnTurretOn(P);
			if (Type == 1 || Type == 2) SpawnLaserOn(P);
		}
	}

	// Bu aktöre artik gerek yok
	Destroy();
}

// ---------------------------------------------------------------------------
AStaticMeshActor* ARocketJumpShowcaseSetup::SpawnPlatform(const FVector& Location, const FVector& Scale)
{
	UWorld* W = GetWorld();
	if (!W || !PlatformMesh) return nullptr;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AStaticMeshActor* Actor = W->SpawnActor<AStaticMeshActor>(
		AStaticMeshActor::StaticClass(), Location, FRotator::ZeroRotator, Params);
	if (!Actor) return nullptr;

	UStaticMeshComponent* MC = Actor->GetStaticMeshComponent();
	if (!MC) return Actor;

	MC->SetStaticMesh(PlatformMesh);
	if (PlatformMaterial) MC->SetMaterial(0, PlatformMaterial);
	MC->SetWorldScale3D(Scale);
	MC->SetCollisionProfileName(TEXT("BlockAll"));

	// Taret/lazer sistemi bu tag'i arar
	Actor->Tags.Add(FName("Platform"));

	return Actor;
}

// ---------------------------------------------------------------------------
void ARocketJumpShowcaseSetup::SpawnTurretOn(AStaticMeshActor* Platform)
{
	if (!Platform || !TurretClass || !GetWorld()) return;

	FVector Orig, Ext;
	Platform->GetActorBounds(false, Orig, Ext);
	const FVector Loc = FVector(Orig.X, Orig.Y, Orig.Z + Ext.Z + 60.f);
	const FRotator Rot(0.f, FMath::FRandRange(0.f, 360.f), 0.f);

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	if (ARocketJumpTurret* T = GetWorld()->SpawnActor<ARocketJumpTurret>(TurretClass, Loc, Rot, Params))
	{
		T->AttachToActor(Platform, FAttachmentTransformRules::KeepWorldTransform);
	}
}

// ---------------------------------------------------------------------------
void ARocketJumpShowcaseSetup::SpawnLaserOn(AStaticMeshActor* Platform)
{
	if (!Platform || !GetWorld()) return;

	FVector Orig, Ext;
	Platform->GetActorBounds(false, Orig, Ext);

	const float LaserZ = Orig.Z + Ext.Z + 80.f;
	const float SpanX  = FMath::Max(Ext.X * 0.80f, 60.f);

	const FVector Start(Orig.X - SpanX, Orig.Y, LaserZ);
	const FVector End  (Orig.X + SpanX, Orig.Y, LaserZ);

	ARocketJumpLaserBeam::SpawnBeam(GetWorld(), Start, End, 4.f, 18.f, Platform);
}
