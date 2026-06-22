#include "RocketJumpHealthComponent.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

URocketJumpHealthComponent::URocketJumpHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void URocketJumpHealthComponent::BeginPlay()
{
	Super::BeginPlay();
	CurrentHealth = MaxHealth;
	LastDamageWorldTime = -RegenDelayAfterNoDamage;
	BroadcastHealth();

	if (bEnableHealthRegen && GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(RegenTimer, this, &URocketJumpHealthComponent::TickHealthRegen, 0.1f, true);
	}
}

void URocketJumpHealthComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RegenTimer);
		World->GetTimerManager().ClearTimer(RespawnTimer);
	}

	Super::EndPlay(EndPlayReason);
}

void URocketJumpHealthComponent::TakeDamage(float Amount)
{
	if (Amount <= 0.f || CurrentHealth <= 0.f)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		LastDamageWorldTime = World->GetTimeSeconds();
	}

	CurrentHealth = FMath::Max(0.f, CurrentHealth - Amount);
	BroadcastHealth();

	if (CurrentHealth <= 0.f)
	{
		HandleDeath();
	}
}

void URocketJumpHealthComponent::TickHealthRegen()
{
	if (!bEnableHealthRegen || CurrentHealth <= 0.f || CurrentHealth >= MaxHealth)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const float Now = World->GetTimeSeconds();
	if (Now - LastDamageWorldTime < RegenDelayAfterNoDamage)
	{
		return;
	}

	const float PreviousHealth = CurrentHealth;
	CurrentHealth = FMath::Min(MaxHealth, CurrentHealth + HealthRegenPerSecond * 0.1f);

	if (!FMath::IsNearlyEqual(PreviousHealth, CurrentHealth))
	{
		BroadcastHealth();
	}
}

void URocketJumpHealthComponent::BroadcastHealth()
{
	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
}

void URocketJumpHealthComponent::HandleDeath()
{
	if (ACharacter* Char = Cast<ACharacter>(GetOwner()))
	{
		if (APlayerController* PC = Cast<APlayerController>(Char->GetController()))
		{
			PC->DisableInput(PC);
		}

		if (UCharacterMovementComponent* Move = Char->GetCharacterMovement())
		{
			Move->DisableMovement();
		}
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(RespawnTimer, this, &URocketJumpHealthComponent::RespawnPlayer, RespawnDelay, false);
	}
}

void URocketJumpHealthComponent::RespawnPlayer()
{
	ACharacter* Char = Cast<ACharacter>(GetOwner());
	if (!Char)
	{
		return;
	}

	CurrentHealth = MaxHealth;
	if (UWorld* World = GetWorld())
	{
		LastDamageWorldTime = World->GetTimeSeconds();
	}
	BroadcastHealth();

	if (UCharacterMovementComponent* Move = Char->GetCharacterMovement())
	{
		Move->SetMovementMode(MOVE_Walking);
	}

	if (APlayerController* PC = Cast<APlayerController>(Char->GetController()))
	{
		PC->EnableInput(PC);
		PC->bShowMouseCursor = false;
		PC->SetInputMode(FInputModeGameOnly());
		PC->SetIgnoreLookInput(false);
		PC->SetIgnoreMoveInput(false);
	}

	FVector SpawnLoc = Char->GetActorLocation();
	FRotator SpawnRot = Char->GetActorRotation();

	if (AActor* Start = UGameplayStatics::GetActorOfClass(GetWorld(), APlayerStart::StaticClass()))
	{
		SpawnLoc = Start->GetActorLocation();
		SpawnRot = Start->GetActorRotation();
	}

	Char->SetActorLocation(SpawnLoc, false, nullptr, ETeleportType::TeleportPhysics);
	Char->SetActorRotation(SpawnRot);
}
