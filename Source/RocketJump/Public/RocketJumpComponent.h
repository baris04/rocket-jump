#pragma once

#include "Components/ActorComponent.h"
#include "InputCoreTypes.h"
#include "InputActionValue.h"
#include "TimerManager.h"
#include "RocketJumpProjectile.h"
#include "Sound/SoundBase.h"
#include "Particles/ParticleSystem.h"
#include "RocketJumpComponent.generated.h"

class UInputAction;
class UInputMappingContext;
class UPrimitiveComponent;
class USkeletalMesh;
class UStaticMesh;

/**
 * Baktigin yone roket firlatabilir (patlayarak itme) veya anlik trace modu.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class URocketJumpComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	URocketJumpComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "RocketJump")
	void PerformRocketJump();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RocketJump|Input")
	FKey RocketJumpKey = EKeys::Q;

	/** Ates etmek gibi sol tik ile de rocket jump (template'teki diger mouse baglarina dikkat). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RocketJump|Input")
	bool bBindPrimaryMouseButton = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RocketJump|Input", meta = (ClampMin = "0"))
	int32 MappingPriority = 128;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RocketJump|Launch", meta = (ClampMin = "100.0"))
	float LaunchStrength = 1650.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RocketJump|Projectile")
	bool bFireProjectileRocket = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RocketJump|Projectile")
	TSubclassOf<ARocketJumpProjectile> ProjectileClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RocketJump|Projectile", meta = (ClampMin = "100.0"))
	float ProjectileSpeed = 3400.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RocketJump|Projectile", meta = (ClampMin = "0.0"))
	float ProjectileSpawnOffset = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RocketJump|Projectile", meta = (ClampMin = "0.5"))
	float ProjectileMaxLifetime = 8.f;

	/** >0 hafif egri; 0 tam duz roket. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RocketJump|Projectile", meta = (ClampMin = "0.0"))
	float ProjectileGravityScale = 0.06f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RocketJump|Projectile", meta = (ClampMin = "50.0"))
	float RocketExplosionRadius = 450.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RocketJump|Projectile", meta = (ClampMin = "100.0"))
	float RocketExplosionStrength = 1400.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RocketJump|Projectile", meta = (ClampMin = "0.1"))
	float RocketExplosionFalloff = 1.45f;

	/** Roket patlamasinda kendi kendine hasar (TF2 rocket jump gibi). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RocketJump|Projectile")
	bool bRocketSelfDamage = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RocketJump|Projectile", meta = (ClampMin = "0.0"))
	float RocketSelfDamageMax = 22.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RocketJump|Projectile", meta = (ClampMin = "0.0"))
	float RocketSelfDamageMin = 6.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RocketJump|Projectile")
	TObjectPtr<USoundBase> ExplosionSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RocketJump|Projectile")
	TObjectPtr<UParticleSystem> ExplosionParticle;

	/** Roket kapaliysa kullanilan anlik zip (trace) buyuklugu. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RocketJump|Launch", meta = (ClampMin = "100.0"))
	float AimTraceDistance = 8000.f;

	/**
	 * 0 = tamamen yuzey normali (duvarda disari, zeminde yukari).
	 * 1 = tamamen -bakis (roket sana/arkana dogru itecek yone yakin).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RocketJump|Launch", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float AimDirectionBlend = 0.38f;

	/** Normal·Up bu esigi astiginda (zemin/egim) AimDirectionBlend en az bu kadar olur — zeminde dik zip'i onler. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RocketJump|Launch", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MinAimBlendOnHorizontalSurfaces = 0.72f;

	/** Normal·Up esigi; ustunde yuzey giderek "zemin" sayilir (egimler icin). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RocketJump|Launch", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float HorizontalSurfaceNormalDotStart = 0.25f;

	/** Son yone cok hafif dunya yukarisi karisimi (genelde 0–0.1 yeter). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RocketJump|Launch", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float FinalUpKickBlend = 0.06f;

	/** Duvara carpmazsa -bakilan yonun tersine+Yukari ile ziplat (roket sirtinda hissi). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RocketJump|Launch")
	bool bLaunchOppositeAimWhenNoHit = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RocketJump|Launch", meta = (ClampMin = "0.0"))
	float NoHitUpwardMix = 0.45f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RocketJump|Launch")
	bool bLaunchStraightUpIfNoHitAndNoOpposite = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RocketJump|Launch")
	bool bRequireGround = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RocketJump|Launch")
	bool bOverrideZ = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RocketJump|Launch")
	bool bOverrideXY = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RocketJump|Gun")
	TObjectPtr<UStaticMesh> GunMeshAsset = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RocketJump|Gun")
	TObjectPtr<USkeletalMesh> GunSkeletalMeshAsset = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RocketJump|Gun")
	FName GunAttachSocketName = TEXT("hand_r");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RocketJump|Gun")
	FVector GunRelativeScale = FVector(0.4f, 0.4f, 0.4f);

	/** Kameradan ileri/sag/asagi offset (cm). Editorde ayarla. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RocketJump|Gun")
	FVector GunCameraFallbackOffset = FVector(30.f, 14.f, -16.f);

	/** Silah donusu. SM_GrenadeLauncher namlusu ileri baksin. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RocketJump|Gun")
	FRotator GunCameraFallbackRotation = FRotator(0.f, -90.f, 0.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RocketJump|Recoil")
	bool bEnableWeaponRecoil = true;

	/** Silah mesh geri kayma (local X, namlu yonu). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RocketJump|Recoil", meta = (ClampMin = "0.0"))
	float RecoilGunKickBack = 8.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RocketJump|Recoil", meta = (ClampMin = "0.0"))
	float RecoilGunKickUpDegrees = 0.f;

	/** Kameranin yukariya kayma miktari (0 = kayma yok). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RocketJump|Recoil", meta = (ClampMin = "0.0"))
	float RecoilCameraPitchDegrees = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RocketJump|Recoil", meta = (ClampMin = "1.0"))
	float RecoilRecoverySpeed = 14.f;

	UPROPERTY(Transient)
	TObjectPtr<UPrimitiveComponent> GunMeshComponent = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UInputAction> RocketJumpAction = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UInputMappingContext> RocketJumpContext = nullptr;

	void UnbindEnhancedInput();
	void TryBindEnhancedInputRetry();
	bool AttemptBindEnhancedInput();
	void AttachGunVisual(ACharacter* Character);
	void ApplyWeaponRecoil(ACharacter* Character, const FVector& AimDir);
	void UpdateRecoilVisuals(float DeltaTime);

	void OnRocketJumpPressed(const FInputActionValue& Value);

	FVector GunBaseRelativeLocation = FVector::ZeroVector;
	FRotator GunBaseRelativeRotation = FRotator::ZeroRotator;
	FVector CurrentRecoilLocation = FVector::ZeroVector;
	FRotator CurrentRecoilRotation = FRotator::ZeroRotator;

	FTimerHandle InputBindRetryTimer;
	int32 InputBindRetriesRemaining = 0;

	bool bDidRegisterMapping = false;
};
