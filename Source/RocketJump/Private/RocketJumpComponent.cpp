#include "RocketJumpComponent.h"
#include "RocketJumpProjectile.h"
#include "RocketJumpHealthComponent.h"

#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "UObject/ConstructorHelpers.h"

URocketJumpComponent::URocketJumpComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	static ConstructorHelpers::FClassFinder<ARocketJumpProjectile> ProjectileFinder(TEXT("/Script/RocketJump.RocketJumpProjectile"));
	if (ProjectileFinder.Succeeded())
	{
		ProjectileClass = ProjectileFinder.Class;
	}

	static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultGun(TEXT("/Game/Weapons/GrenadeLauncher/Meshes/SM_GrenadeLauncher.SM_GrenadeLauncher"));
	if (DefaultGun.Succeeded())
	{
		GunMeshAsset = DefaultGun.Object;
	}

	static ConstructorHelpers::FObjectFinder<USoundBase> DefaultExplosionSnd(TEXT("/Game/Weapons/GrenadeLauncher/Audio/FirstPersonTemplateWeaponFire02.FirstPersonTemplateWeaponFire02"));
	if (DefaultExplosionSnd.Succeeded())
	{
		ExplosionSound = DefaultExplosionSnd.Object;
	}
}

void URocketJumpComponent::BeginPlay()
{
	Super::BeginPlay();

	if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
	{
		if (!Character->FindComponentByClass<URocketJumpHealthComponent>())
		{
			URocketJumpHealthComponent* Health = NewObject<URocketJumpHealthComponent>(Character, TEXT("RocketJumpHealth"));
			Health->RegisterComponent();
		}
		AttachGunVisual(Character);
	}

	InputBindRetriesRemaining = 40;
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(InputBindRetryTimer, this, &URocketJumpComponent::TryBindEnhancedInputRetry, 0.05f, true);
	}
}

void URocketJumpComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(InputBindRetryTimer);
	}

	UnbindEnhancedInput();

	if (GunMeshComponent)
	{
		GunMeshComponent->DestroyComponent();
		GunMeshComponent = nullptr;
	}

	Super::EndPlay(EndPlayReason);
}

void URocketJumpComponent::AttachGunVisual(ACharacter* Character)
{
	if (!Character || !GunMeshAsset)
	{
		return;
	}

	GunMeshComponent = NewObject<UStaticMeshComponent>(Character, TEXT("RocketJumpGunMesh"));
	GunMeshComponent->SetStaticMesh(GunMeshAsset);
	GunMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GunMeshComponent->SetCastShadow(false);
	GunMeshComponent->SetRelativeScale3D(GunRelativeScale);
	// Duvara gecmesin: SDPG_Foreground dunya geometrisinden SONRA render eder
	GunMeshComponent->DepthPriorityGroup = SDPG_Foreground;
	// Sadece bu oyuncuya gorunsun (FP gorunum)
	GunMeshComponent->SetOnlyOwnerSee(true);

	bool bAttached = false;

	// Birincil: kamera'ya dogrudan bagla — her zaman gorunur, duvara girmez
	if (UCameraComponent* Cam = Character->FindComponentByClass<UCameraComponent>())
	{
		GunMeshComponent->AttachToComponent(Cam, FAttachmentTransformRules::KeepRelativeTransform);
		GunMeshComponent->SetRelativeLocation(GunCameraFallbackOffset);
		GunMeshComponent->SetRelativeRotation(GunCameraFallbackRotation);
		bAttached = true;
	}

	if (!bAttached)
	{
		// Kamera yoksa kok bilesene bagla
		GunMeshComponent->AttachToComponent(Character->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		GunMeshComponent->SetRelativeLocation(FVector(60.f, 24.f, 40.f));
	}

	GunMeshComponent->RegisterComponent();

	GunBaseRelativeLocation = GunMeshComponent->GetRelativeLocation();
	GunBaseRelativeRotation = GunMeshComponent->GetRelativeRotation();
}

void URocketJumpComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UpdateRecoilVisuals(DeltaTime);
}

void URocketJumpComponent::ApplyWeaponRecoil(ACharacter* Character, const FVector& AimDir)
{
	if (!bEnableWeaponRecoil || !Character)
	{
		return;
	}

	CurrentRecoilLocation += FVector(-RecoilGunKickBack, FMath::FRandRange(-2.f, 2.f), FMath::FRandRange(0.5f, 3.f));
	CurrentRecoilRotation += FRotator(RecoilGunKickUpDegrees, FMath::FRandRange(-2.5f, 2.5f), FMath::FRandRange(-1.f, 1.f));

	if (APlayerController* PC = Cast<APlayerController>(Character->GetController()))
	{
		PC->AddPitchInput(RecoilCameraPitchDegrees);
		PC->AddYawInput(FMath::FRandRange(-0.6f, 0.6f));
	}
}

void URocketJumpComponent::UpdateRecoilVisuals(float DeltaTime)
{
	if (!GunMeshComponent)
	{
		return;
	}

	CurrentRecoilLocation = FMath::VInterpTo(CurrentRecoilLocation, FVector::ZeroVector, DeltaTime, RecoilRecoverySpeed);
	CurrentRecoilRotation = FMath::RInterpTo(CurrentRecoilRotation, FRotator::ZeroRotator, DeltaTime, RecoilRecoverySpeed);

	GunMeshComponent->SetRelativeLocation(GunBaseRelativeLocation + CurrentRecoilLocation);
	GunMeshComponent->SetRelativeRotation(GunBaseRelativeRotation + CurrentRecoilRotation);
}

void URocketJumpComponent::TryBindEnhancedInputRetry()
{
	if (AttemptBindEnhancedInput())
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(InputBindRetryTimer);
		}
		return;
	}

	InputBindRetriesRemaining--;
	if (InputBindRetriesRemaining <= 0)
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(InputBindRetryTimer);
		}
	}
}

bool URocketJumpComponent::AttemptBindEnhancedInput()
{
	if (bDidRegisterMapping)
	{
		return true;
	}

	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (!Character || !Character->IsLocallyControlled())
	{
		return false;
	}

	APlayerController* PC = Cast<APlayerController>(Character->GetController());
	if (!PC)
	{
		return false;
	}

	ULocalPlayer* LocalPlayer = PC->GetLocalPlayer();
	if (!LocalPlayer)
	{
		return false;
	}

	UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(Character->InputComponent);
	if (!EIC)
	{
		EIC = Cast<UEnhancedInputComponent>(PC->InputComponent);
	}

	if (!EIC || !Subsystem)
	{
		return false;
	}

	RocketJumpAction = NewObject<UInputAction>(this, TEXT("RocketJumpAction_DA"));
	RocketJumpAction->ValueType = EInputActionValueType::Boolean;

	RocketJumpContext = NewObject<UInputMappingContext>(this, TEXT("RocketJumpContext_DA"));
	RocketJumpContext->MapKey(RocketJumpAction, RocketJumpKey);

	if (bBindPrimaryMouseButton)
	{
		RocketJumpContext->MapKey(RocketJumpAction, EKeys::LeftMouseButton);
	}

	Subsystem->AddMappingContext(RocketJumpContext, MappingPriority);
	bDidRegisterMapping = true;

	EIC->BindAction(RocketJumpAction, ETriggerEvent::Started, this, &URocketJumpComponent::OnRocketJumpPressed);

	return true;
}

void URocketJumpComponent::UnbindEnhancedInput()
{
	if (!bDidRegisterMapping || !RocketJumpContext)
	{
		return;
	}

	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (!Character)
	{
		return;
	}

	APlayerController* PC = Cast<APlayerController>(Character->GetController());
	if (!PC)
	{
		return;
	}

	ULocalPlayer* LocalPlayer = PC->GetLocalPlayer();
	if (!LocalPlayer)
	{
		return;
	}

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
	{
		Subsystem->RemoveMappingContext(RocketJumpContext);
	}

	bDidRegisterMapping = false;
}

void URocketJumpComponent::OnRocketJumpPressed(const FInputActionValue& Value)
{
	if (!Value.Get<bool>())
	{
		return;
	}

	PerformRocketJump();
}

void URocketJumpComponent::PerformRocketJump()
{
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (!Character)
	{
		return;
	}

	UCharacterMovementComponent* Move = Character->GetCharacterMovement();
	if (bRequireGround && Move && !Move->IsMovingOnGround())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FVector EyeLoc;
	FRotator EyeRot;
	Character->GetActorEyesViewPoint(EyeLoc, EyeRot);
	const FVector AimDir = EyeRot.Vector().GetSafeNormal();

	if (bFireProjectileRocket && ProjectileClass)
	{
		const FVector SpawnLoc = EyeLoc + AimDir * ProjectileSpawnOffset;

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = Character;
		SpawnParams.Instigator = Character;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		ARocketJumpProjectile* Rocket = World->SpawnActor<ARocketJumpProjectile>(ProjectileClass, SpawnLoc, AimDir.Rotation(), SpawnParams);
		if (Rocket)
		{
			Rocket->InitializeRocket(
				Character,
				AimDir * ProjectileSpeed,
				RocketExplosionRadius,
				RocketExplosionStrength,
				RocketExplosionFalloff,
				ProjectileGravityScale,
				ProjectileMaxLifetime,
				ExplosionSound.Get(),
				ExplosionParticle.Get(),
				bOverrideXY,
				bOverrideZ,
				bRocketSelfDamage,
				RocketSelfDamageMax,
				RocketSelfDamageMin);
		}

		ApplyWeaponRecoil(Character, AimDir);
		return;
	}

	const FVector TraceEnd = EyeLoc + AimDir * AimTraceDistance;

	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(RocketJumpAimTrace), true, Character);
	if (GunMeshComponent)
	{
		Params.AddIgnoredComponent(GunMeshComponent.Get());
	}

	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_WorldStatic);
	ObjectParams.AddObjectTypesToQuery(ECC_WorldDynamic);

	const bool bHit = World->LineTraceSingleByObjectType(Hit, EyeLoc, TraceEnd, ObjectParams, Params);

	FVector LaunchDir = FVector::ZeroVector;

	if (bHit)
	{
		const FVector AlongNormal = Hit.ImpactNormal.GetSafeNormal();
		const FVector AlongNegAim = (-AimDir).GetSafeNormal();

		const float Flatness = FVector::DotProduct(AlongNormal, FVector::UpVector);
		float Blend = AimDirectionBlend;
		if (Flatness > HorizontalSurfaceNormalDotStart)
		{
			const float Denom = 1.f - HorizontalSurfaceNormalDotStart;
			const float T = Denom > KINDA_SMALL_NUMBER ? (Flatness - HorizontalSurfaceNormalDotStart) / Denom : 1.f;
			Blend = FMath::Lerp(AimDirectionBlend, FMath::Max(AimDirectionBlend, MinAimBlendOnHorizontalSurfaces), T);
		}

		LaunchDir = FMath::Lerp(AlongNormal, AlongNegAim, Blend).GetSafeNormal();
	}
	else if (bLaunchOppositeAimWhenNoHit)
	{
		LaunchDir = (-AimDir + FVector::UpVector * NoHitUpwardMix).GetSafeNormal();
	}
	else if (bLaunchStraightUpIfNoHitAndNoOpposite)
	{
		LaunchDir = FVector::UpVector;
	}
	else
	{
		return;
	}

	if (FinalUpKickBlend > KINDA_SMALL_NUMBER)
	{
		LaunchDir = (LaunchDir * (1.f - FinalUpKickBlend) + FVector::UpVector * FinalUpKickBlend).GetSafeNormal();
	}

	if (LaunchDir.IsNearlyZero())
	{
		LaunchDir = FVector::UpVector;
	}

	Character->LaunchCharacter(LaunchDir * LaunchStrength, bOverrideXY, bOverrideZ);
	ApplyWeaponRecoil(Character, AimDir);
}
