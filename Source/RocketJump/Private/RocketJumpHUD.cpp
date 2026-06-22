#include "RocketJumpHUD.h"

#include "Engine/Canvas.h"
#include "GameFramework/PlayerController.h"
#include "RocketJumpHealthComponent.h"

void ARocketJumpHUD::DrawHUD()
{
	Super::DrawHUD();

	if (!Canvas)
	{
		return;
	}

	const float Margin = 24.f;
	const float BarW = 280.f;
	const float BarH = 22.f;
	const float X = Margin;
	const float Y = Margin;

	float Health = 100.f;
	float MaxHealth = 100.f;
	if (APlayerController* PC = GetOwningPlayerController())
	{
		if (APawn* Pawn = PC->GetPawn())
		{
			if (URocketJumpHealthComponent* HC = Pawn->FindComponentByClass<URocketJumpHealthComponent>())
			{
				Health = HC->GetHealth();
				MaxHealth = HC->GetMaxHealth();
			}
		}
	}

	const float Ratio = MaxHealth > 0.f ? FMath::Clamp(Health / MaxHealth, 0.f, 1.f) : 0.f;

	DrawRect(HealthBarBgColor, X, Y, BarW, BarH);
	DrawRect(HealthBarColor, X, Y, BarW * Ratio, BarH);

	const FString HealthText = FString::Printf(TEXT("CAN: %.0f / %.0f"), Health, MaxHealth);
	UFont* TextFont = GEngine ? GEngine->GetMediumFont() : nullptr;
	DrawText(HealthText, FLinearColor::White, X + 8.f, Y + 2.f, TextFont, 1.1f, false);

	UFont* HintFont = GEngine ? GEngine->GetSmallFont() : nullptr;
	DrawText(TEXT("Sol tik / Q: Roket (kendi hasarin)  |  Taretler ates ediyor!"), FLinearColor(0.85f, 0.85f, 0.85f, 1.f), X, Y + BarH + 10.f, HintFont, 1.f, false);

	if (Ratio < 0.35f)
	{
		DrawRect(DamageFlashColor, 0.f, 0.f, Canvas->SizeX, Canvas->SizeY);
	}

	// --- Crosshair ---
	const float CX = Canvas->SizeX * 0.5f;
	const float CY = Canvas->SizeY * 0.5f;
	const float LineLen  = 10.f;
	const float LineThick = 2.f;
	const float Gap      = 4.f;
	const FLinearColor CrossColor(1.f, 1.f, 1.f, 0.9f);
	const FLinearColor CrossShadow(0.f, 0.f, 0.f, 0.5f);

	// Gölge (1 px kaydırılmış)
	DrawRect(CrossShadow, CX - LineLen - Gap + 1, CY - LineThick * 0.5f + 1, LineLen, LineThick);
	DrawRect(CrossShadow, CX + Gap + 1,           CY - LineThick * 0.5f + 1, LineLen, LineThick);
	DrawRect(CrossShadow, CX - LineThick * 0.5f + 1, CY - LineLen - Gap + 1, LineThick, LineLen);
	DrawRect(CrossShadow, CX - LineThick * 0.5f + 1, CY + Gap + 1,           LineThick, LineLen);

	// Sol çizgi
	DrawRect(CrossColor, CX - LineLen - Gap, CY - LineThick * 0.5f, LineLen, LineThick);
	// Sağ çizgi
	DrawRect(CrossColor, CX + Gap,           CY - LineThick * 0.5f, LineLen, LineThick);
	// Üst çizgi
	DrawRect(CrossColor, CX - LineThick * 0.5f, CY - LineLen - Gap, LineThick, LineLen);
	// Alt çizgi
	DrawRect(CrossColor, CX - LineThick * 0.5f, CY + Gap,           LineThick, LineLen);

	// Merkez nokta
	DrawRect(CrossColor, CX - 1.5f, CY - 1.5f, 3.f, 3.f);
}
