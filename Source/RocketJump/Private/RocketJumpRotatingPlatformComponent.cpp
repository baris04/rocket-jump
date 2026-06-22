#include "RocketJumpRotatingPlatformComponent.h"

URocketJumpRotatingPlatformComponent::URocketJumpRotatingPlatformComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void URocketJumpRotatingPlatformComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	const float DeltaDeg = RotationSpeedDegreesPerSecond * DeltaTime;
	FRotator DeltaRot(
		bRotatePitch ? DeltaDeg : 0.f,
		bRotateYaw ? DeltaDeg : 0.f,
		bRotateRoll ? DeltaDeg : 0.f);

	Owner->AddActorLocalRotation(DeltaRot);
}
