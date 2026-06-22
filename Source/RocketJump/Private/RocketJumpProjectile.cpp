#include "RocketJumpProjectile.h"

#include "RocketJumpHealthComponent.h"
#include "TurretProjectile.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

ARocketJumpProjectile::ARocketJumpProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollision"));
	RootComponent = CollisionComp;
	CollisionComp->InitSphereRadius(12.f);
	CollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionComp->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionComp->SetCollisionResponseToAllChannels(ECR_Block);
	CollisionComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	CollisionComp->SetGenerateOverlapEvents(false);
	CollisionComp->CanCharacterStepUpOn = ECB_No;
	CollisionComp->OnComponentHit.AddDynamic(this, &ARocketJumpProjectile::OnProjectileHit);

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	MeshComp->SetupAttachment(CollisionComp);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> RocketMesh(TEXT("/Game/Weapons/GrenadeLauncher/Meshes/FirstPersonProjectileMesh.FirstPersonProjectileMesh"));
	if (RocketMesh.Succeeded())
	{
		MeshComp->SetStaticMesh(RocketMesh.Object);
	}
	else
	{
		static ConstructorHelpers::FObjectFinder<UStaticMesh> FallbackMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
		if (FallbackMesh.Succeeded()) MeshComp->SetStaticMesh(FallbackMesh.Object);
	}
	MeshComp->SetRelativeScale3D(FVector(0.18f, 0.18f, 0.18f));
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComp->CastShadow = true;

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = 2400.f;
	ProjectileMovement->MaxSpeed = 10000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->ProjectileGravityScale = 0.f;
	ProjectileMovement->Buoyancy = 0.f;
}

void ARocketJumpProjectile::BeginPlay()
{
	Super::BeginPlay();

	if (AActor* OwningActor = GetOwner())
	{
		CollisionComp->IgnoreActorWhenMoving(OwningActor, true);
	}
}

void ARocketJumpProjectile::InitializeRocket(
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
	float InSelfDamageMin)
{
	FiringPawn = InOwner;
	ExplosionRadius = InExplosionRadius;
	ExplosionLaunchStrength = InExplosionStrength;
	ExplosionFalloffExponent = InFalloffExponent;
	bOverrideXY = bInOverrideXY;
	bOverrideZ = bInOverrideZ;
	bApplySelfDamage = bInApplySelfDamage;
	SelfDamageMax = InSelfDamageMax;
	SelfDamageMin = InSelfDamageMin;
	ExplosionSound = InExplosionSound;
	ExplosionFX = InExplosionFX;

	if (ProjectileMovement)
	{
		ProjectileMovement->ProjectileGravityScale = InGravityScale;
		ProjectileMovement->Velocity = InitialVelocity;
		ProjectileMovement->UpdateComponentVelocity();
		ProjectileMovement->Activate(true);
	}

	if (AActor* OwningActor = GetOwner())
	{
		CollisionComp->IgnoreActorWhenMoving(OwningActor, true);
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(LifetimeTimer, this, &ARocketJumpProjectile::LifetimeExpired, InMaxFlightSeconds, false);
	}
}

void ARocketJumpProjectile::OnProjectileHit(
	UPrimitiveComponent* HitComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	FVector NormalImpulse,
	const FHitResult& Hit)
{
	// Taret mermisine carpti: onu yok et, roket devam etsin
	if (ATurretProjectile* EnemyBullet = Cast<ATurretProjectile>(OtherActor))
	{
		EnemyBullet->Destroy();
		return;
	}

	Explode(Hit.ImpactPoint, Hit.ImpactNormal);
}

void ARocketJumpProjectile::LifetimeExpired()
{
	Explode(GetActorLocation(), FVector::UpVector);
}

void ARocketJumpProjectile::Explode(const FVector& ExplosionLocation, const FVector& SurfaceNormal)
{
	if (bExploded)
	{
		return;
	}
	bExploded = true;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(LifetimeTimer);
	}

	if (UParticleSystem* FX = ExplosionFX.Get())
	{
		UGameplayStatics::SpawnEmitterAtLocation(this, FX, ExplosionLocation, FRotator::ZeroRotator, FVector(ExplosionRadius / 250.f), true);
	}

	if (USoundBase* Snd = ExplosionSound.Get())
	{
		UGameplayStatics::PlaySoundAtLocation(this, Snd, ExplosionLocation);
	}

	if (ACharacter* Char = Cast<ACharacter>(FiringPawn.Get()))
	{
		const FVector CharLoc = Char->GetActorLocation();
		FVector ToChar = CharLoc - ExplosionLocation;
		const float Dist = ToChar.Size();
		float NormalizedDist = 1.f;

		if (Dist > KINDA_SMALL_NUMBER && Dist <= ExplosionRadius)
		{
			FVector Dir = ToChar / Dist;
			NormalizedDist = Dist / ExplosionRadius;
			const float Falloff = FMath::Pow(1.f - FMath::Clamp(NormalizedDist, 0.f, 1.f), ExplosionFalloffExponent);
			const float VelocityMag = ExplosionLaunchStrength * Falloff;
			FVector LaunchVel = Dir * VelocityMag;

			Dir = LaunchVel.GetSafeNormal();
			if (!Dir.IsNearlyZero())
			{
				Char->LaunchCharacter(LaunchVel, bOverrideXY, bOverrideZ);
			}
		}
		else if (Dist <= KINDA_SMALL_NUMBER)
		{
			NormalizedDist = 0.f;
			FVector Push = SurfaceNormal.GetSafeNormal();
			if (Push.Z < 0.35f)
			{
				Push = (Push + FVector::UpVector * 0.65f).GetSafeNormal();
			}
			Char->LaunchCharacter(Push * ExplosionLaunchStrength * 0.75f, bOverrideXY, bOverrideZ);
		}

		if (bApplySelfDamage && Dist <= ExplosionRadius)
		{
			if (URocketJumpHealthComponent* HC = Char->FindComponentByClass<URocketJumpHealthComponent>())
			{
				const float DamageFalloff = FMath::Pow(1.f - FMath::Clamp(NormalizedDist, 0.f, 1.f), ExplosionFalloffExponent);
				const float Damage = FMath::Lerp(SelfDamageMax, SelfDamageMin, 1.f - DamageFalloff);
				HC->TakeDamage(Damage);
			}
		}
	}

	// Patlama yarıcapındaki tüm taret mermilerini yok et
	TArray<AActor*> NearbyBullets;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATurretProjectile::StaticClass(), NearbyBullets);
	for (AActor* Bullet : NearbyBullets)
	{
		if (Bullet && FVector::Dist(Bullet->GetActorLocation(), ExplosionLocation) <= ExplosionRadius)
		{
			Bullet->Destroy();
		}
	}

	Destroy();
}
