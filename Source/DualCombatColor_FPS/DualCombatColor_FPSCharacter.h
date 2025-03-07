// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "DualCombatColor_FPSCharacter.generated.h"

class UInputComponent;
class UPauseMenuWidget;
class UVictoryMenuWidget;
class UDefeatMenuWidget;
class UUI_PlayerWidget;

USTRUCT()
struct FDataPlayer
{
	GENERATED_BODY()
public:
	UPROPERTY()
	int score;
	UPROPERTY()
	int numberCurrentLevel;
	UPROPERTY()
	int life;
};

UCLASS(config=Game)
class ADualCombatColor_FPSCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category=Mesh)
	class USkeletalMeshComponent* Mesh1P;

	/** Gun mesh: 1st person view (seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USkeletalMeshComponent* FP_Gun;

	/** Location on gun mesh where projectiles should spawn. */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USceneComponent* FP_MuzzleLocation;

	/** Gun mesh: VR view (attached to the VR controller directly, no arm, just the actual gun) */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USkeletalMeshComponent* VR_Gun;

	/** Location on VR gun mesh where projectiles should spawn. */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USceneComponent* VR_MuzzleLocation;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FirstPersonCameraComponent;

	/** Motion controller (right hand) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UMotionControllerComponent* R_MotionController;

	/** Motion controller (left hand) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UMotionControllerComponent* L_MotionController;

public:
	ADualCombatColor_FPSCharacter();

	UPROPERTY(EditAnywhere, Category = "UI HUD")
		TSubclassOf<UUI_PlayerWidget> UI_PlayerWidget_Class;
	UPROPERTY()
	UUI_PlayerWidget* UI_PlayerWidget;

	UPROPERTY(EditAnywhere, Category = "UI HUD")
		TSubclassOf<UPauseMenuWidget> PauseMenuWidget_Class;

	UPROPERTY()
	UPauseMenuWidget* PauseMenuWidget;

	UPROPERTY(EditAnywhere, Category = "UI HUD")
		TSubclassOf<UVictoryMenuWidget> VictoryMenuWidget_Class;

	UPROPERTY()
	UVictoryMenuWidget* VictoryMenuWidget;

	UPROPERTY(EditAnywhere, Category = "UI HUD")
		TSubclassOf<UDefeatMenuWidget> DefeatMenuWidget_Class;

	UPROPERTY()
	UDefeatMenuWidget* DefeatMenuWidget;

	UPROPERTY()
		FDataPlayer FdataPlayer;
protected:

	virtual void BeginPlay();
	virtual void Tick(float DeltaSeconds) override;
	//Game Play Functions and Variables
	//int score;
	UPROPERTY(EditAnywhere)
		int addScoreForPlatformTread = 10;
	bool isPaused;
	void PauseGame();
	

	UPROPERTY()
	APlayerController* playerController;
	//-------------------
	//Collision Function
	UFUNCTION()
		void OnComponentBeginOverlap(class UPrimitiveComponent* HitComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp,
			int32 OtherBodyIndex, bool bFromeSweep, const FHitResult& SweepResult);

	// Menu Functions
	void CreatedUI_Player();

	void CreatedPauseMenu();
	
	void CreatedVictoryMenu();
	
	void CreatedDefeatMenu();

	void OpenPauseMenu();

	void OpenVictoryMenu();

	void OpenDefeatMenu();

	void CheckCursorVisible();
	//-------------------

	void CheckDie();

	void Die();
public:
	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	/** Gun muzzle's offset from the characters location */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	FVector GunOffset;

	/** Projectile class to spawn */
	UPROPERTY(EditDefaultsOnly, Category=Projectile)
	TSubclassOf<class ADualCombatColor_FPSProjectile> ProjectileClass;

	/** Sound to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	class USoundBase* FireSound;

	/** AnimMontage to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	class UAnimMontage* FireAnimation;

	/** Whether to use motion controller location for aiming. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	uint32 bUsingMotionControllers : 1;

protected:
	/** Fires a projectile. */
	void OnFire();

	/** Resets HMD orientation and position in VR. */
	void OnResetVR();

	/** Handles moving forward/backward */
	void MoveForward(float Val);

	/** Handles stafing movement, left and right */
	void MoveRight(float Val);

	/**
	 * Called via input to turn at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	struct TouchData
	{
		TouchData() { bIsPressed = false;Location=FVector::ZeroVector;}
		bool bIsPressed;
		ETouchIndex::Type FingerIndex;
		FVector Location;
		bool bMoved;
	};
	void BeginTouch(const ETouchIndex::Type FingerIndex, const FVector Location);
	void EndTouch(const ETouchIndex::Type FingerIndex, const FVector Location);
	void TouchUpdate(const ETouchIndex::Type FingerIndex, const FVector Location);
	TouchData	TouchItem;
	
protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	// End of APawn interface

	/* 
	 * Configures input for touchscreen devices if there is a valid touch interface for doing so 
	 *
	 * @param	InputComponent	The input component pointer to bind controls to
	 * @returns true if touch controls were enabled.
	 */
	bool EnableTouchscreenMovement(UInputComponent* InputComponent);

public:
	/** Returns Mesh1P subobject **/
	FORCEINLINE class USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	/** Returns FirstPersonCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

};

