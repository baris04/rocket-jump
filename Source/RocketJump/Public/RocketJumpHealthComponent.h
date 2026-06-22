#pragma once

#include "Components/ActorComponent.h"
#include "RocketJumpHealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRocketJumpHealthChanged, float, CurrentHealth, float, MaxHealth);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class URocketJumpHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	URocketJumpHealthComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable, Category = "RocketJump|Health")
	void TakeDamage(float Amount);

	UFUNCTION(BlueprintPure, Category = "RocketJump|Health")
	float GetHealth() const { return CurrentHealth; }

	UFUNCTION(BlueprintPure, Category = "RocketJump|Health")
	float GetMaxHealth() const { return MaxHealth; }

	UFUNCTION(BlueprintPure, Category = "RocketJump|Health")
	bool IsAlive() const { return CurrentHealth > 0.f; }

	UPROPERTY(BlueprintAssignable, Category = "RocketJump|Health")
	FOnRocketJumpHealthChanged OnHealthChanged;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RocketJump|Health", meta = (ClampMin = "1.0"))
	float MaxHealth = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RocketJump|Health", meta = (ClampMin = "0.1"))
	float RespawnDelay = 2.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RocketJump|Health|Regen")
	bool bEnableHealthRegen = true;

	/** Bu kadar saniye hasar almayinca can yenilenmeye baslar. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RocketJump|Health|Regen", meta = (ClampMin = "0.5"))
	float RegenDelayAfterNoDamage = 4.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RocketJump|Health|Regen", meta = (ClampMin = "1.0"))
	float HealthRegenPerSecond = 18.f;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RocketJump|Health")
	float CurrentHealth = 100.f;

	void BroadcastHealth();
	void HandleDeath();
	void RespawnPlayer();
	void TickHealthRegen();

	float LastDamageWorldTime = 0.f;

	FTimerHandle RespawnTimer;
	FTimerHandle RegenTimer;
};
