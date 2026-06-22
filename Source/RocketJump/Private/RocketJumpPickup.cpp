#include "RocketJumpPickup.h"

#include "Components/PointLightComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

ARocketJumpPickup::ARocketJumpPickup()
{
	PrimaryActorTick.bCanEverTick = true;

	// --- Trigger ---
	TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Trigger"));
	RootComponent = TriggerSphere;
	TriggerSphere->InitSphereRadius(55.f);
	TriggerSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerSphere->SetCollisionObjectType(ECC_WorldDynamic);
	TriggerSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	TriggerSphere->SetGenerateOverlapEvents(true);
	TriggerSphere->OnComponentBeginOverlap.AddDynamic(this, &ARocketJumpPickup::OnPickupOverlap);

	// --- Kutu / teneke gövde ---
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylMesh(
		TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(
		TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> GlowMat(
		TEXT("/Game/LevelPrototyping/Interactable/JumpPad/Assets/Materials/M_SimpleGlow.M_SimpleGlow"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> BasicMat(
		TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));

	CanMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Can"));
	CanMesh->SetupAttachment(TriggerSphere);
	CanMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CanMesh->CastShadow = false;
	if (CylMesh.Succeeded()) CanMesh->SetStaticMesh(CylMesh.Object);
	// Enerji icecegi mavisi: parlak cyan renk
	if (GlowMat.Succeeded())  CanMesh->SetMaterial(0, GlowMat.Object);
	else if (BasicMat.Succeeded()) CanMesh->SetMaterial(0, BasicMat.Object);
	// Silindir yüksekliği 100cm → 0.4 scale = 40cm yükseklik, 0.22 yatay = 22cm çap
	CanMesh->SetRelativeScale3D(FVector(0.22f, 0.22f, 0.40f));

	// --- Parlayan halka (alt) ---
	GlowRing = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GlowRing"));
	GlowRing->SetupAttachment(TriggerSphere);
	GlowRing->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GlowRing->CastShadow = false;
	if (SphereMesh.Succeeded()) GlowRing->SetStaticMesh(SphereMesh.Object);
	if (GlowMat.Succeeded())    GlowRing->SetMaterial(0, GlowMat.Object);
	else if (BasicMat.Succeeded()) GlowRing->SetMaterial(0, BasicMat.Object);
	GlowRing->SetRelativeScale3D(FVector(0.5f, 0.5f, 0.08f));
	GlowRing->SetRelativeLocation(FVector(0.f, 0.f, -28.f));

	// --- Işık ---
	PickupLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("Light"));
	PickupLight->SetupAttachment(TriggerSphere);
	PickupLight->SetLightColor(FLinearColor(0.1f, 0.6f, 1.f));   // cyan-mavi
	PickupLight->SetIntensity(3000.f);
	PickupLight->SetAttenuationRadius(220.f);
	PickupLight->SetCastShadows(false);
	PickupLight->bUseInverseSquaredFalloff = false;
	PickupLight->LightFalloffExponent = 3.f;
}

void ARocketJumpPickup::BeginPlay()
{
	Super::BeginPlay();
	BaseLocation = GetActorLocation();

	// Dinamik materyal instance — Tick'te renk parametresi güncellenebilsin
	CanMesh->CreateAndSetMaterialInstanceDynamic(0);
	GlowRing->CreateAndSetMaterialInstanceDynamic(0);
}

void ARocketJumpPickup::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	BobTime  += DeltaSeconds * 2.2f;
	SpinTime += DeltaSeconds;

	// Yukarı-aşağı salınım
	const FVector CurLoc = GetActorLocation();
	const float BobZ = BaseLocation.Z + FMath::Sin(BobTime) * BobAmplitude;
	SetActorLocation(FVector(CurLoc.X, CurLoc.Y, BobZ));

	// Dönme
	SetActorRotation(FRotator(0.f, SpinTime * SpinSpeed, 0.f));

	// Işık nabzı
	const float Pulse = 0.5f + 0.5f * FMath::Sin(BobTime * 1.4f);
	PickupLight->SetIntensity(FMath::Lerp(1800.f, 4000.f, Pulse));

	// Halka renk nabzı
	if (UMaterialInstanceDynamic* MID = Cast<UMaterialInstanceDynamic>(GlowRing->GetMaterial(0)))
	{
		const float Bright = FMath::Lerp(2.f, 6.f, Pulse);
		MID->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.1f * Bright, 0.5f * Bright, Bright, 1.f));
		MID->SetVectorParameterValue(TEXT("Color"),     FLinearColor(0.1f * Bright, 0.5f * Bright, Bright, 1.f));
	}
	if (UMaterialInstanceDynamic* MID = Cast<UMaterialInstanceDynamic>(CanMesh->GetMaterial(0)))
	{
		const float Bright = FMath::Lerp(3.f, 8.f, Pulse);
		MID->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.1f * Bright, 0.55f * Bright, Bright, 1.f));
		MID->SetVectorParameterValue(TEXT("Color"),     FLinearColor(0.1f * Bright, 0.55f * Bright, Bright, 1.f));
	}
}

void ARocketJumpPickup::OnPickupOverlap(
	UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	APawn* Pawn = Cast<APawn>(OtherActor);
	if (!Pawn || !Pawn->IsPlayerControlled()) return;
	if (!CanMesh->IsVisible()) return;          // zaten alınmış, bekleniyor

	GrantDoubleJump(Pawn);

	// Görünümü gizle, collision kapat
	CanMesh->SetVisibility(false);
	GlowRing->SetVisibility(false);
	PickupLight->SetVisibility(false);
	TriggerSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Süre dolunca double jump iptal
	if (DurationSeconds > 0.f)
	{
		GetWorld()->GetTimerManager().SetTimer(
			RevokeTimer, [this, Pawn]()
			{
				RevokeDoubleJump(Pawn);
			},
			DurationSeconds, false);
	}

	// Yeniden belirme
	if (RespawnDelay > 0.f)
	{
		GetWorld()->GetTimerManager().SetTimer(
			RespawnTimer, this, &ARocketJumpPickup::ShowPickup, RespawnDelay, false);
	}
}

void ARocketJumpPickup::GrantDoubleJump(APawn* Pawn)
{
	GrantedPawn = Pawn;
	if (ACharacter* Char = Cast<ACharacter>(Pawn))
	{
		Char->JumpMaxCount = 2;   // double jump aç
	}
}

void ARocketJumpPickup::RevokeDoubleJump(APawn* Pawn)
{
	if (ACharacter* Char = Cast<ACharacter>(Pawn))
	{
		Char->JumpMaxCount = 1;   // double jump kapat
	}
}

void ARocketJumpPickup::ShowPickup()
{
	CanMesh->SetVisibility(true);
	GlowRing->SetVisibility(true);
	PickupLight->SetVisibility(true);
	TriggerSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	BaseLocation = GetActorLocation();  // yeniden bob başlangıcı
}
