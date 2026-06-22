#include "TurretProjectile.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "RocketJumpHealthComponent.h"
#include "UObject/ConstructorHelpers.h"

ATurretProjectile::ATurretProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	RootComponent = CollisionComp;
	CollisionComp->InitSphereRadius(8.f);
	CollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionComp->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionComp->SetCollisionResponseToAllChannels(ECR_Block);
	CollisionComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	CollisionComp->OnComponentHit.AddDynamic(this, &ATurretProjectile::OnProjectileHit);

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	MeshComp->SetupAttachment(CollisionComp);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> BulletMesh(TEXT("/Game/Variant_Shooter/Blueprints/Pickups/Projectiles/Meshes/SM_FoamBullet.SM_FoamBullet"));
	if (BulletMesh.Succeeded())
	{
		MeshComp->SetStaticMesh(BulletMesh.Object);
	}
	else
	{
		static ConstructorHelpers::FObjectFinder<UStaticMesh> FallbackMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
		if (FallbackMesh.Succeeded()) MeshComp->SetStaticMesh(FallbackMesh.Object);
	}
	MeshComp->SetRelativeScale3D(FVector(0.85f));
	MeshComp->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Movement"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = 480.f;
	ProjectileMovement->MaxSpeed = 1200.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->ProjectileGravityScale = 0.f;
}

void ATurretProjectile::BeginPlay()
{
	Super::BeginPlay();
	if (TurretOwner.IsValid())
	{
		CollisionComp->IgnoreActorWhenMoving(TurretOwner.Get(), true);
	}
}

void ATurretProjectile::InitTurretShot(const FVector& Velocity, float InDamage, AActor* InTurretOwner, float VisualScale)
{
	Damage = InDamage;
	TurretOwner = InTurretOwner;

	const float ClampedScale = FMath::Max(VisualScale, 0.08f);
	MeshComp->SetRelativeScale3D(FVector(ClampedScale));
	// Eski referans: mesh 0.12, carpisma yaricapi 8 — olcek orantili buyur
	CollisionComp->SetSphereRadius((8.f / 0.12f) * ClampedScale);

	if (ProjectileMovement)
	{
		ProjectileMovement->Velocity = Velocity;
		ProjectileMovement->UpdateComponentVelocity();
	}
	if (InTurretOwner)
	{
		CollisionComp->IgnoreActorWhenMoving(InTurretOwner, true);
	}
}

void ATurretProjectile::OnProjectileHit(
	UPrimitiveComponent* HitComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	FVector NormalImpulse,
	const FHitResult& Hit)
{
	if (APawn* Pawn = Cast<APawn>(OtherActor))
	{
		ApplyDamageToPawn(Pawn);
	}
	Destroy();
}

void ATurretProjectile::ApplyDamageToPawn(APawn* Pawn)
{
	if (!Pawn)
	{
		return;
	}

	if (URocketJumpHealthComponent* HC = Pawn->FindComponentByClass<URocketJumpHealthComponent>())
	{
		HC->TakeDamage(Damage);
	}
}
