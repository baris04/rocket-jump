#include "RocketJumpGameMode.h"

#include "RocketJumpHUD.h"
#include "RocketJumpLevelSetup.h"
#include "RocketJumpPlayerController.h"
#include "GameFramework/Character.h"
#include "RocketJumpComponent.h"
#include "RocketJumpHealthComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

ARocketJumpGameMode::ARocketJumpGameMode()
{
	// First Person sablonunun mouse look / Enhanced Input kurulumu BP controller'da.
	static ConstructorHelpers::FClassFinder<APlayerController> PCFinder(
		TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonPlayerController.BP_FirstPersonPlayerController_C"));
	if (PCFinder.Succeeded())
	{
		PlayerControllerClass = PCFinder.Class;
	}
	else
	{
		PlayerControllerClass = ARocketJumpPlayerController::StaticClass();
	}

	HUDClass = ARocketJumpHUD::StaticClass();

	static ConstructorHelpers::FClassFinder<APawn> PawnFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter.BP_FirstPersonCharacter_C"));
	if (PawnFinder.Succeeded())
	{
		DefaultPawnClass = PawnFinder.Class;
	}
}

void ARocketJumpGameMode::StartPlay()
{
	Super::StartPlay();

	EnsurePlayerRocketJumpSetup();

	if (!bAutoSetupLevel1Turrets)
	{
		return;
	}

	const FString MapName = GetWorld() ? GetWorld()->GetMapName() : FString();
	if (!MapName.Contains(TEXT("level1"), ESearchCase::IgnoreCase))
	{
		return;
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (ARocketJumpLevelSetup* Setup = GetWorld()->SpawnActor<ARocketJumpLevelSetup>(ARocketJumpLevelSetup::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, Params))
	{
		Setup->TurretHeightAbovePlatform = TurretHeightAbovePlatform;
	}
}

void ARocketJumpGameMode::EnsurePlayerRocketJumpSetup()
{
	ACharacter* Character = UGameplayStatics::GetPlayerCharacter(this, 0);
	if (!Character)
	{
		if (UWorld* World = GetWorld())
		{
			FTimerHandle RetryHandle;
			World->GetTimerManager().SetTimer(RetryHandle, this, &ARocketJumpGameMode::EnsurePlayerRocketJumpSetup, 0.1f, false);
		}
		return;
	}

	if (!Character->FindComponentByClass<URocketJumpComponent>())
	{
		URocketJumpComponent* RocketJump = NewObject<URocketJumpComponent>(Character, TEXT("RocketJumpComponent"));
		Character->AddInstanceComponent(RocketJump);
		RocketJump->RegisterComponent();
	}

	if (!Character->FindComponentByClass<URocketJumpHealthComponent>())
	{
		URocketJumpHealthComponent* Health = NewObject<URocketJumpHealthComponent>(Character, TEXT("RocketJumpHealth"));
		Character->AddInstanceComponent(Health);
		Health->RegisterComponent();
	}
}
