#include "RocketJumpLaserBeam.h"
#include "Components/BoxComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Pawn.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "RocketJumpHealthComponent.h"
#include "UObject/ConstructorHelpers.h"

ARocketJumpLaserBeam::ARocketJumpLaserBeam()
{
	PrimaryActorTick.bCanEverTick = true;

	// Boş SceneComponent root — SetRelativeLocation çağrıları aktörü taşımaz
	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	// Collision kutusu — root'a bağlı, başlangıçta sıfır pozisyonda
	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Trigger"));
	TriggerBox->SetupAttachment(Root);
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetCollisionObjectType(ECC_WorldDynamic);
	TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	TriggerBox->SetGenerateOverlapEvents(true);
	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ARocketJumpLaserBeam::OnLaserOverlap);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> ChamferCube(TEXT("/Game/LevelPrototyping/Meshes/SM_ChamferCube.SM_ChamferCube"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CircularGlow(TEXT("/Game/LevelPrototyping/Interactable/JumpPad/Assets/Meshes/SM_CircularGlow.SM_CircularGlow"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> FallbackSphere(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> ShapeMat(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> GlowMat(TEXT("/Game/LevelPrototyping/Interactable/JumpPad/Assets/Materials/M_SimpleGlow.M_SimpleGlow"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> DarkMat(TEXT("/Game/LevelPrototyping/Materials/MI_PrototypeGrid_TopDark.MI_PrototypeGrid_TopDark"));

	// Lazer cihaz kasası — aktör köküne bağlı (= kaynak noktasında sabit)
	EmitterHousingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("EmitterHousing"));
	EmitterHousingMesh->SetupAttachment(Root);
	EmitterHousingMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	EmitterHousingMesh->CastShadow = true;
	if (ChamferCube.Succeeded())   EmitterHousingMesh->SetStaticMesh(ChamferCube.Object);
	if (DarkMat.Succeeded())       EmitterHousingMesh->SetMaterial(0, DarkMat.Object);
	else if (ShapeMat.Succeeded()) EmitterHousingMesh->SetMaterial(0, ShapeMat.Object);
	EmitterHousingMesh->SetRelativeScale3D(FVector(0.22f, 0.22f, 0.18f));

	// Aperture (parlayan lens)
	EmitterMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("EmitterAperture"));
	EmitterMesh->SetupAttachment(Root);
	EmitterMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	EmitterMesh->CastShadow = false;
	if (CircularGlow.Succeeded())  EmitterMesh->SetStaticMesh(CircularGlow.Object);
	else if (FallbackSphere.Succeeded()) EmitterMesh->SetStaticMesh(FallbackSphere.Object);
	if (GlowMat.Succeeded())   EmitterMesh->SetMaterial(0, GlowMat.Object);
	else if (ShapeMat.Succeeded()) EmitterMesh->SetMaterial(0, ShapeMat.Object);
	EmitterMesh->SetRelativeScale3D(FVector(0.12f));

	// İnce parlak merkez ışını
	BeamMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BeamCore"));
	BeamMesh->SetupAttachment(Root);
	BeamMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BeamMesh->CastShadow = false;
	if (CylMesh.Succeeded())   BeamMesh->SetStaticMesh(CylMesh.Object);
	if (ShapeMat.Succeeded())  BeamMesh->SetMaterial(0, ShapeMat.Object);

	// Dış yumuşak hale
	GlowMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BeamGlow"));
	GlowMesh->SetupAttachment(Root);
	GlowMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GlowMesh->CastShadow = false;
	if (CylMesh.Succeeded())   GlowMesh->SetStaticMesh(CylMesh.Object);
	if (GlowMat.Succeeded())   GlowMesh->SetMaterial(0, GlowMat.Object);
	else if (ShapeMat.Succeeded()) GlowMesh->SetMaterial(0, ShapeMat.Object);

	// Kaynak ışığı
	SourceLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("SourceLight"));
	SourceLight->SetupAttachment(Root);
	SourceLight->SetLightColor(FLinearColor(1.f, 0.1f, 0.1f));
	SourceLight->SetIntensity(5500.f);
	SourceLight->SetAttenuationRadius(180.f);
	SourceLight->SetCastShadows(false);
	SourceLight->bUseInverseSquaredFalloff = false;
	SourceLight->LightFalloffExponent = 3.f;

	// Orta ambient ışık
	LaserLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("LaserLight"));
	LaserLight->SetupAttachment(Root);
	LaserLight->SetLightColor(FLinearColor(1.f, 0.05f, 0.05f));
	LaserLight->SetIntensity(2200.f);
	LaserLight->SetAttenuationRadius(280.f);
	LaserLight->SetCastShadows(false);
	LaserLight->bUseInverseSquaredFalloff = false;
	LaserLight->LightFalloffExponent = 4.f;

	// SpotLight — cihazdan ışın boyunca
	BeamSpot = CreateDefaultSubobject<USpotLightComponent>(TEXT("BeamSpot"));
	BeamSpot->SetupAttachment(Root);
	BeamSpot->SetLightColor(FLinearColor(1.f, 0.08f, 0.08f));
	BeamSpot->SetIntensity(3000.f);
	BeamSpot->SetAttenuationRadius(600.f);
	BeamSpot->SetInnerConeAngle(1.5f);
	BeamSpot->SetOuterConeAngle(4.f);
	BeamSpot->SetCastShadows(false);

	PulseTime = FMath::FRandRange(0.f, 6.28f);
}

void ARocketJumpLaserBeam::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// --- Tarama hareketi ---
	// Aktör kaynak noktasında sabit kalır, sadece YAW rotasyonu değişir
	if (bSweepBeam && BeamLength > KINDA_SMALL_NUMBER)
	{
		SweepTime += DeltaSeconds * SweepSpeed;
		const float SweepAngleDeg = FMath::Sin(SweepTime) * SweepAmplitude;
		const FQuat  SweepQ = FQuat(FVector::UpVector, FMath::DegreesToRadians(SweepAngleDeg));
		const FVector SweptDir = SweepQ.RotateVector(BeamBaseDirection);
		const FRotator NewRot = SweptDir.Rotation() + FRotator(90.f, 0.f, 0.f);
		SetActorRotation(NewRot);
		// Işının ortası her kare güncelleniyor
		LaserLight->SetRelativeLocation(FVector(0.f, 0.f, BeamLength * 0.5f));
	}

	// --- Titreme (pulse) ---
	PulseTime += DeltaSeconds * 3.8f;
	const float Pulse     = 0.5f + 0.5f * FMath::Sin(PulseTime);
	const float FastPulse = 0.5f + 0.5f * FMath::Sin(PulseTime * 2.3f);

	LaserLight->SetIntensity(FMath::Lerp(1500.f, 2800.f, Pulse));
	SourceLight->SetIntensity(FMath::Lerp(3500.f, 7000.f, FastPulse));
	BeamSpot->SetIntensity(FMath::Lerp(1800.f, 3500.f, Pulse));

	const float CoreBrightness = FMath::Lerp(8.f, 14.f, Pulse);
	if (UMaterialInstanceDynamic* MID = Cast<UMaterialInstanceDynamic>(BeamMesh->GetMaterial(0)))
	{
		MID->SetVectorParameterValue(TEXT("Color"), FLinearColor(CoreBrightness, 0.f, 0.f, 1.f));
	}

	const float GlowBrightness = FMath::Lerp(1.5f, 3.f, Pulse);
	if (UMaterialInstanceDynamic* MID = Cast<UMaterialInstanceDynamic>(GlowMesh->GetMaterial(0)))
	{
		MID->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(GlowBrightness, 0.f, 0.f, 1.f));
		MID->SetVectorParameterValue(TEXT("Color"),     FLinearColor(GlowBrightness, 0.f, 0.f, 0.6f));
	}

	const float EmitScale = FMath::Lerp(0.10f, 0.20f, FastPulse);
	EmitterMesh->SetRelativeScale3D(FVector(EmitScale));
	if (UMaterialInstanceDynamic* MID = Cast<UMaterialInstanceDynamic>(EmitterMesh->GetMaterial(0)))
	{
		const float EmitBright = FMath::Lerp(4.f, 10.f, FastPulse);
		MID->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(EmitBright, 0.1f, 0.1f, 1.f));
		MID->SetVectorParameterValue(TEXT("Color"),     FLinearColor(EmitBright, 0.1f, 0.1f, 1.f));
	}
}

ARocketJumpLaserBeam* ARocketJumpLaserBeam::SpawnBeam(UWorld* World, const FVector& Start, const FVector& End,
	float VisualThickness, float TriggerThickness, AActor* AttachTo)
{
	if (!World) return nullptr;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ARocketJumpLaserBeam* Beam = World->SpawnActor<ARocketJumpLaserBeam>(
		ARocketJumpLaserBeam::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, Params);

	if (Beam)
	{
		Beam->ConfigureBeam(Start, End, VisualThickness, TriggerThickness);
		if (AttachTo)
		{
			Beam->AttachToActor(AttachTo, FAttachmentTransformRules::KeepWorldTransform);
		}
	}
	return Beam;
}

void ARocketJumpLaserBeam::ConfigureBeam(const FVector& Start, const FVector& End,
	float VisualThickness, float TriggerThickness)
{
	const FVector Delta = End - Start;
	BeamLength = Delta.Size();
	if (BeamLength < KINDA_SMALL_NUMBER) return;

	BeamSourcePosition     = Start;
	BeamBaseDirection      = (Delta / BeamLength);
	StoredVisualThickness  = VisualThickness;
	StoredTriggerThickness = TriggerThickness;
	SweepTime = FMath::FRandRange(0.f, 6.28f);

	// Aktör (Root) kaynak noktasında, ışın oradan ileri (local +Z) uzanıyor
	SetActorLocation(Start);
	const FRotator Rot = BeamBaseDirection.Rotation() + FRotator(90.f, 0.f, 0.f);
	SetActorRotation(Rot);

	const float HalfLen = BeamLength * 0.5f;

	// Collision kutusu: ışının ortasında, ±HalfLen boyunca Z ekseninde
	// (Root'a bağlı, bu yüzden SetRelativeLocation aktörü taşımaz)
	TriggerBox->SetRelativeLocation(FVector(0.f, 0.f, HalfLen));
	const float T = FMath::Max(TriggerThickness, 8.f);
	TriggerBox->SetBoxExtent(FVector(T * 0.5f, T * 0.5f, HalfLen));

	// Işın mesh'leri: ışının ortasında (silindir kendi merkezini pivot alır)
	const float V = FMath::Clamp(VisualThickness * 0.5f, 1.2f, 3.5f);
	BeamMesh->SetRelativeLocation(FVector(0.f, 0.f, HalfLen));
	BeamMesh->SetRelativeScale3D(FVector(V / 100.f, V / 100.f, BeamLength / 100.f));

	const float G = V * 5.f;
	GlowMesh->SetRelativeLocation(FVector(0.f, 0.f, HalfLen));
	GlowMesh->SetRelativeScale3D(FVector(G / 100.f, G / 100.f, BeamLength / 100.f));

	// Orta ışık
	LaserLight->SetRelativeLocation(FVector(0.f, 0.f, HalfLen));

	// Kaynak bileşenleri: Root'ta (= Start, lazer cihazı konumu)
	EmitterHousingMesh->SetRelativeLocation(FVector::ZeroVector);
	EmitterMesh->SetRelativeLocation(FVector::ZeroVector);
	SourceLight->SetRelativeLocation(FVector::ZeroVector);

	// SpotLight: local +Z yönünde (= ışın yönünde, çünkü aktör o yönde döndürüldü)
	BeamSpot->SetRelativeLocation(FVector::ZeroVector);
	BeamSpot->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));

	// Dinamik malzeme
	if (UMaterialInstanceDynamic* MID = BeamMesh->CreateAndSetMaterialInstanceDynamic(0))
	{
		MID->SetVectorParameterValue(TEXT("Color"), FLinearColor(12.f, 0.f, 0.f, 1.f));
	}
	if (UMaterialInstanceDynamic* MID = GlowMesh->CreateAndSetMaterialInstanceDynamic(0))
	{
		MID->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(2.f, 0.f, 0.f, 1.f));
		MID->SetVectorParameterValue(TEXT("Color"),     FLinearColor(2.f, 0.f, 0.f, 1.f));
	}
	if (UMaterialInstanceDynamic* MID = EmitterMesh->CreateAndSetMaterialInstanceDynamic(0))
	{
		MID->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(6.f, 0.1f, 0.1f, 1.f));
		MID->SetVectorParameterValue(TEXT("Color"),     FLinearColor(6.f, 0.1f, 0.1f, 1.f));
	}

	BeamMesh->SetVisibility(true);
	GlowMesh->SetVisibility(true);
	EmitterHousingMesh->SetVisibility(true);
	EmitterMesh->SetVisibility(true);
}

void ARocketJumpLaserBeam::UpdateBeamTransform(const FVector& Start, const FVector& End)
{
	(void)Start; (void)End; // Artık kullanılmıyor, Tick direkt aktör rotasyonunu değiştiriyor
}

void ARocketJumpLaserBeam::OnLaserOverlap(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	APawn* Pawn = Cast<APawn>(OtherActor);
	if (!Pawn || !Pawn->IsPlayerControlled()) return;

	if (URocketJumpHealthComponent* HC = Pawn->FindComponentByClass<URocketJumpHealthComponent>())
	{
		HC->TakeDamage(HC->GetMaxHealth() + 100.f);
	}
}
