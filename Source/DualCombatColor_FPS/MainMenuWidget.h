// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainMenuWidget.generated.h"

/**
 * 
 */
UCLASS()
class DUALCOMBATCOLOR_FPS_API UMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
		FName nameMap;
protected:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
		class UButton* ButtonPlay;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
		class UButton* ButtonCredits;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
		class UButton* ButtonExit;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
		class UButton* BackToMenu;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
		class UWidget* CanvasMenu;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
		class UWidget* CanvasCredits;

public:
	virtual void NativeOnInitialized() override;
	virtual void NativeDestruct() override;

	UFUNCTION()
		void OnClickedButtonPlay();

	UFUNCTION()
		void OnClickedButtonCredits();

	UFUNCTION()
		void OnClikedButtonExit();

	UFUNCTION()
		void OnClikedButtonBackToMenu();

	UFUNCTION()
		void ExitGame();

	UFUNCTION()
		void ShowMenuElements();

	UFUNCTION()
		void ShowCreditsElements();

};
