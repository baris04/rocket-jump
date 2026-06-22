#include "RocketJumpTurret.h"

#include "Components/StaticMeshComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInterface.h"
#include "TurretProjectile.h"
#include "UObject/ConstructorHelpers.h"

ARocketJumpTurret::ARocketJumpTurret()
{
	PrimaryActorTick.bCanEverTick = true;

	BaseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Base"));
	RootComponent = BaseMesh;

	HeadMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Head"));
	HeadMesh->SetupAttachment(BaseMesh);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> ChamferCube(TEXT("/Game/LevelPrototyping/Meshes/SM_ChamferCube.SM_ChamferCube"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> FallbackCube(TEXT("/Engine/BasicShapes/Cube.Cube"));

	UStaticMesh* BodyMesh = ChamferCube.Succeeded() ? ChamferCube.Object : (FallbackCube.Succeeded() ? FallbackCube.Object : nullptr);
	if (BodyMesh)
	{
		BaseMesh->SetStaticMesh(BodyMesh);
		HeadMesh->SetStaticMesh(BodyMesh);
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> BaseMat(TEXT("/Game/LevelPrototyping/Materials/MI_PrototypeGrid_Gray.MI_PrototypeGrid_Gray"));
	if (BaseMat.Succeeded())
	{
		BaseMesh->SetMaterial(0, BaseMat.Object);
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> HeadMat(TEXT("/Game/LevelPrototyping/Materials/MI_PrototypeGrid_TopDark.MI_PrototypeGrid_TopDark"));
	if (HeadMat.Succeeded())
	{
		HeadMesh->SetMaterial(0, HeadMat.Object);
	}

	BaseMesh->SetRelativeScale3D(FVector(0.55f, 0.55f, 0.35f));
	HeadMesh->SetRelativeLocation(FVector(0.f, 0.f, 45.f));
	HeadMesh->SetRelativeScale3D(FVector(0.35f, 0.35f, 0.5f));
	BaseMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	HeadMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	static ConstructorHelpers::FClassFinder<ATurretProjectile> ProjectileFinder(TEXT("/Script/RocketJump.TurretProjectile"));
	if (ProjectileFinder.Succeeded())
	{
		TurretProjectileClass = ProjectileFinder.Class;
	}
}

void ARocketJumpTurret::BeginPlay()
{
	Super::BeginPlay();
	FireCooldown = FMath::FRandRange(0.f, FireInterval * 0.5f);
	HeadMeshBaseRelativeLocation = HeadMesh->GetRelativeLocation();
}

void ARocketJumpTurret::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	FireCooldown -= DeltaSeconds;

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!PlayerPawn)
	{
		return;
	}

	const FVector ToPlayer = PlayerPawn->GetActorLocation() - GetActorLocation();
	const float DistSq = ToPlayer.SizeSquared();
	if (DistSq < FMath::Square(MinEngageDistance) || DistSq > FMath::Square(MaxEngageDistance))
	{
		return;
	}

	const FRotator LookAt = ToPlayer.Rotation();
	SetActorRotation(FRotator(0.f, LookAt.Yaw, 0.f));
	HeadMesh->SetWorldRotation(LookAt);

	if (FireCooldown <= 0.f)
	{
		TryFireAtTarget(PlayerPawn);
		FireCooldown = FireInterval;
	}

	UpdateHeadRecoil(DeltaSeconds);
}

void ARocketJumpTurret::UpdateHeadRecoil(float DeltaSeconds)
{
	HeadRecoilKick = FMath::FInterpTo(HeadRecoilKick, 0.f, DeltaSeconds, 16.f);
	const FVector RecoilLoc = HeadMeshBaseRelativeLocation + FVector(-18.f * HeadRecoilKick, 0.f, 0.f);
	HeadMesh->SetRelativeLocation(RecoilLoc);
}

void ARocketJumpTurret::TryFireAtTarget(APawn* Target)
{
	if (!Target || !TurretProjectileClass)
	{
		return;
	}

	const FVector MuzzleLoc = HeadMesh->GetComponentLocation() + HeadMesh->GetForwardVector() * 40.f;
	const FVector Dir = (Target->GetActorLocation() + FVector(0.f, 0.f, 50.f) - MuzzleLoc).GetSafeNormal();
	if (Dir.IsNearlyZero())
	{
		return;
	}

	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (ATurretProjectile* Proj = GetWorld()->SpawnActor<ATurretProjectile>(TurretProjectileClass, MuzzleLoc, Dir.Rotation(), Params))
	{
		Proj->InitTurretShot(Dir * ProjectileSpeed, ProjectileDamage, this, ProjectileVisualScale);
	}

	HeadRecoilKick = 1.f;
}
