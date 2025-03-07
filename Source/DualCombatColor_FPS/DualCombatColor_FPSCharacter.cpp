// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "DualCombatColor_FPSCharacter.h"
#include "DualCombatColor_FPSProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/InputSettings.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"
#include "MainMenuWidget.h"
#include "MotionControllerComponent.h"
#include "PauseMenuWidget.h"
#include "VictoryMenuWidget.h"
#include "DefeatMenuWidget.h"
#include "PlatformPawn.h"
#include "Engine/StaticMesh.h"
#include "UI_PlayerWidget.h"
#include "VictoryPointActor.h"
#include "ParkourGameInstance.h"
#include "XRMotionControllerBase.h" // for FXRMotionControllerBase::RightHandSourceId

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

//////////////////////////////////////////////////////////////////////////
// ADualCombatColor_FPSCharacter

ADualCombatColor_FPSCharacter::ADualCombatColor_FPSCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;
	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-39.56f, 1.75f, 64.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->SetRelativeRotation(FRotator(1.9f, -19.19f, 5.2f));
	Mesh1P->SetRelativeLocation(FVector(-0.5f, -4.4f, -155.7f));

	// Create a gun mesh component
	FP_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FP_Gun"));
	FP_Gun->SetOnlyOwnerSee(true);			// only the owning player will see this mesh
	FP_Gun->bCastDynamicShadow = false;
	FP_Gun->CastShadow = false;
	// FP_Gun->SetupAttachment(Mesh1P, TEXT("GripPoint"));
	FP_Gun->SetupAttachment(RootComponent);

	FP_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocation"));
	FP_MuzzleLocation->SetupAttachment(FP_Gun);
	FP_MuzzleLocation->SetRelativeLocation(FVector(0.2f, 48.4f, -10.6f));

	// Default offset from the character location for projectiles to spawn
	GunOffset = FVector(100.0f, 0.0f, 10.0f);

	// Note: The ProjectileClass and the skeletal mesh/anim blueprints for Mesh1P, FP_Gun, and VR_Gun 
	// are set in the derived blueprint asset named MyCharacter to avoid direct content references in C++.

	// Create VR Controllers.
	R_MotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("R_MotionController"));
	R_MotionController->MotionSource = FXRMotionControllerBase::RightHandSourceId;
	R_MotionController->SetupAttachment(RootComponent);
	L_MotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("L_MotionController"));
	L_MotionController->SetupAttachment(RootComponent);

	// Create a gun and attach it to the right-hand VR controller.
	// Create a gun mesh component
	VR_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("VR_Gun"));
	VR_Gun->SetOnlyOwnerSee(true);			// only the owning player will see this mesh
	VR_Gun->bCastDynamicShadow = false;
	VR_Gun->CastShadow = false;
	VR_Gun->SetupAttachment(R_MotionController);
	VR_Gun->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));

	VR_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("VR_MuzzleLocation"));
	VR_MuzzleLocation->SetupAttachment(VR_Gun);
	VR_MuzzleLocation->SetRelativeLocation(FVector(0.000004, 53.999992, 10.000000));
	VR_MuzzleLocation->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));		// Counteract the rotation of the VR gun model.

	// Uncomment the following line to turn motion controllers on by default:
	//bUsingMotionControllers = true;
}

void ADualCombatColor_FPSCharacter::BeginPlay()
{
	// Call the base class
	Super::BeginPlay();

	//UStaticMesh* staticMesh;

	//staticMesh->SetMaterial(,);

	GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &ADualCombatColor_FPSCharacter::OnComponentBeginOverlap);
	//Attach gun mesh component to Skeleton, doing it here because the skeleton is not yet created in the constructor
	FP_Gun->AttachToComponent(Mesh1P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));
	
	if (GetWorld() != nullptr)
	{
		//CheckCursorVisible();
		playerController = GetWorld()->GetFirstPlayerController();
		if (playerController != nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("GetFirstPlayerController Existe"));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("GetFirstPlayerController nulo"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("GetWorld nulo"));
	}

	// Show or hide the two versions of the gun based on whether or not we're using motion controllers.
	if (bUsingMotionControllers)
	{
		VR_Gun->SetHiddenInGame(false, true);
		Mesh1P->SetHiddenInGame(true, true);
	}
	else
	{
		VR_Gun->SetHiddenInGame(true, true);
		Mesh1P->SetHiddenInGame(false, true);
	}

	UParkourGameInstance* parkourGameInstance = Cast<UParkourGameInstance>(GetGameInstance());
	if (parkourGameInstance != nullptr) 
	{
		FdataPlayer.numberCurrentLevel = parkourGameInstance->currentData.currentLevel;
		FdataPlayer.score = parkourGameInstance->currentData.currentScore;
		FdataPlayer.life = 100;
	}
	else 
	{
		UE_LOG(LogTemp, Warning, TEXT("parkourGameInstance nulo"));
	}
	isPaused = false;
	//Created Menus
	CreatedPauseMenu();
	CreatedVictoryMenu();
	CreatedDefeatMenu();
	CreatedUI_Player();
	UI_PlayerWidget->SetScoreText(FdataPlayer.score);
	UI_PlayerWidget->SetCurrentLifeText(FdataPlayer.life);
	UI_PlayerWidget->SetCurrentLevelText(FdataPlayer.numberCurrentLevel);
	//---------------
}
void ADualCombatColor_FPSCharacter::Tick(float DeltaSeconds)
{

	CheckDie();
	if(!PauseMenuWidget)
		UE_LOG(LogTemp, Warning, TEXT("pause bad"));

	if (!VictoryMenuWidget)
		UE_LOG(LogTemp, Warning, TEXT("victory bad"));

	if (!DefeatMenuWidget)
		UE_LOG(LogTemp, Warning, TEXT("defeat bad"));

	bool MenuSuccefullyLoaded = (!DefeatMenuWidget && !VictoryMenuWidget) && !PauseMenuWidget;

	if (!MenuSuccefullyLoaded) 
	{
		if (playerController)
			CheckCursorVisible();
		else
			UE_LOG(LogTemp, Warning, TEXT("Player nulo"));
	}
	else 
	{
		UE_LOG(LogTemp, Warning, TEXT("Menu alguno es nulo"));
	}
	
}
void ADualCombatColor_FPSCharacter::Die()
{
	//Si te moris se reinicia el nivel
	UGameplayStatics::OpenLevel(GetWorld(), FName(*GetWorld()->GetName()), false);
}
void ADualCombatColor_FPSCharacter::CheckDie()
{
	if (FdataPlayer.life <= 0)
	{
		Die();
	}
}

void ADualCombatColor_FPSCharacter::OnComponentBeginOverlap(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromeSweep, const FHitResult& SweepResult)
{
	if (OtherActor->ActorHasTag("Plattform"))
	{
		APlatformPawn* platform = Cast<APlatformPawn>(OtherActor);
		if (platform != nullptr) 
		{
			if (!platform->bIsTread) 
			{
				platform->bIsTread = true;
				FdataPlayer.score = FdataPlayer.score + addScoreForPlatformTread;
				UI_PlayerWidget->SetScoreText(FdataPlayer.score);
			}
		}
	}
	if(OtherActor->ActorHasTag("VictoryPoint"))
	{
		UE_LOG(LogTemp, Warning, TEXT("NextLevel Collision"));
		AVictoryPointActor* victoryPoint = Cast<AVictoryPointActor>(OtherActor);
		if (victoryPoint != nullptr)
		{
			UParkourGameInstance* parkourGameInstance = Cast<UParkourGameInstance>(GetGameInstance());
			parkourGameInstance->currentData.currentLevel++;
			parkourGameInstance->currentData.currentScore = FdataPlayer.score;
			//parkourGameInstance->SetAssetLoaderInstance(this);
			victoryPoint->LoadNextLevel();
		}
		else 
		{
			UE_LOG(LogTemp, Warning, TEXT("Victory Point Nulo"));
		}
	}
}
//////////////////////////////////////////////////////////////////////////
// Input

void ADualCombatColor_FPSCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	check(PlayerInputComponent);

	// Bind jump events
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	// Bind fire event
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ADualCombatColor_FPSCharacter::OnFire);

	// Enable touchscreen input
	EnableTouchscreenMovement(PlayerInputComponent);

	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &ADualCombatColor_FPSCharacter::OnResetVR);

	// Bind movement events
	PlayerInputComponent->BindAxis("MoveForward", this, &ADualCombatColor_FPSCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ADualCombatColor_FPSCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &ADualCombatColor_FPSCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &ADualCombatColor_FPSCharacter::LookUpAtRate);

	//Activate Pause Menu
	PlayerInputComponent->BindAction("Pause", IE_Pressed, this, &ADualCombatColor_FPSCharacter::OpenPauseMenu);

	//Activate Win Menu (Only Debug)
	PlayerInputComponent->BindAction("Win_Button", IE_Pressed, this, &ADualCombatColor_FPSCharacter::OpenVictoryMenu);

	//Activate Lose Menu (Only Debug)
	PlayerInputComponent->BindAction("Lose_Button", IE_Pressed, this, &ADualCombatColor_FPSCharacter::OpenDefeatMenu);
}

//------------------------------Game Functions----------------------------------------//

void ADualCombatColor_FPSCharacter::PauseGame()
{
	isPaused = !isPaused;
	UGameplayStatics::SetGamePaused(GetWorld(), isPaused);
}
void ADualCombatColor_FPSCharacter::CheckCursorVisible()
{
	//if(PauseMenuWidget->Visibility == ESlateVisibility::Visible 
	//	|| VictoryMenuWidget->Visibility == ESlateVisibility::Visible 
	//	|| DefeatMenuWidget->Visibility == ESlateVisibility::Visible)
	//{
	//	//Muestra el cursor del mouse.
	//	//playerController->bShowMouseCursor = true;
	//	
	//}
	//else
	//{
	//	//Oculta el cursor del mouse.
	//	//playerController->bShowMouseCursor = false;
	//}

	/* if (PauseMenuWidget->Visibility == ESlateVisibility::Visible)
	{
		UE_LOG(LogTemp, Warning, TEXT("Visible"));
	}
	else if(PauseMenuWidget->Visibility == ESlateVisibility::Hidden)
	{
		UE_LOG(LogTemp, Warning, TEXT("Invisible"));
	}*/
}
//-------------------------------UI - FUNCTIONS---------------------------------------//

void ADualCombatColor_FPSCharacter::CreatedUI_Player()
{
	if (UI_PlayerWidget_Class != nullptr)
	{
		UI_PlayerWidget = CreateWidget<UUI_PlayerWidget>(Cast<APlayerController>(GetOwner()), UI_PlayerWidget_Class, FName("UI_PlayerWidget"));
		UI_PlayerWidget->AddToViewport();
	}
}

void ADualCombatColor_FPSCharacter::CreatedPauseMenu()
{
	if (PauseMenuWidget_Class != nullptr)
	{
		PauseMenuWidget = CreateWidget<UPauseMenuWidget>(Cast<APlayerController>(GetOwner()), PauseMenuWidget_Class, FName("PauseMenuWidget"));
		PauseMenuWidget->AddToViewport();
	}
}

void ADualCombatColor_FPSCharacter::CreatedVictoryMenu()
{
	if (VictoryMenuWidget_Class != nullptr)
	{
		VictoryMenuWidget = CreateWidget<UVictoryMenuWidget>(Cast<APlayerController>(GetOwner()), VictoryMenuWidget_Class, FName("VictoryMenuWidget"));
		VictoryMenuWidget->AddToViewport();
	}
}

void ADualCombatColor_FPSCharacter::CreatedDefeatMenu()
{
	if (DefeatMenuWidget_Class != nullptr)
	{
		DefeatMenuWidget = CreateWidget<UDefeatMenuWidget>(Cast<APlayerController>(GetOwner()), DefeatMenuWidget_Class, FName("DefeatMenuWidget"));
		DefeatMenuWidget->AddToViewport();
	}
}

void ADualCombatColor_FPSCharacter::OpenVictoryMenu()
{
	if (VictoryMenuWidget != nullptr)
	{
		VictoryMenuWidget->ActivateMe();
		VictoryMenuWidget->SetScoreText(FdataPlayer.score);
		PauseGame();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("VictoryMenuWidget Nulo"));
	}
}

void ADualCombatColor_FPSCharacter::OpenDefeatMenu()
{
	if (DefeatMenuWidget != nullptr)
	{
		DefeatMenuWidget->ActivateMe();
		DefeatMenuWidget->SetScoreText(FdataPlayer.score);
		PauseGame();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("DefeatMenuWidgetMenuWidget Nulo"));
	}
}

void ADualCombatColor_FPSCharacter::OpenPauseMenu()
{
	UE_LOG(LogTemp, Warning, TEXT("P"));
	if (PauseMenuWidget != nullptr) 
	{
		PauseMenuWidget->ActivateMe();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("PauseMenuWidget Nulo"));
	}
	PauseGame();
}
//-------------------------------------------------------------------------------------//

void ADualCombatColor_FPSCharacter::OnFire()
{
	// try and fire a projectile
	if (ProjectileClass != NULL)
	{
		UWorld* const World = GetWorld();
		if (World != NULL)
		{
			if (bUsingMotionControllers)
			{
				const FRotator SpawnRotation = VR_MuzzleLocation->GetComponentRotation();
				const FVector SpawnLocation = VR_MuzzleLocation->GetComponentLocation();
				World->SpawnActor<ADualCombatColor_FPSProjectile>(ProjectileClass, SpawnLocation, SpawnRotation);
			}
			else
			{
				const FRotator SpawnRotation = GetControlRotation();
				// MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
				const FVector SpawnLocation = ((FP_MuzzleLocation != nullptr) ? FP_MuzzleLocation->GetComponentLocation() : GetActorLocation()) + SpawnRotation.RotateVector(GunOffset);

				//Set Spawn Collision Handling Override
				FActorSpawnParameters ActorSpawnParams;
				ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

				// spawn the projectile at the muzzle
				ADualCombatColor_FPSProjectile* projectile = World->SpawnActor<ADualCombatColor_FPSProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, ActorSpawnParams);
				projectile->bShooterPlayer = true;
			}
		}
	}

	// try and play the sound if specified
	if (FireSound != NULL)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
	}

	// try and play a firing animation if specified
	if (FireAnimation != NULL)
	{
		// Get the animation object for the arms mesh
		UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance();
		if (AnimInstance != NULL)
		{
			AnimInstance->Montage_Play(FireAnimation, 1.f);
		}
	}
}

void ADualCombatColor_FPSCharacter::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void ADualCombatColor_FPSCharacter::BeginTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (TouchItem.bIsPressed == true)
	{
		return;
	}
	if ((FingerIndex == TouchItem.FingerIndex) && (TouchItem.bMoved == false))
	{
		OnFire();
	}
	TouchItem.bIsPressed = true;
	TouchItem.FingerIndex = FingerIndex;
	TouchItem.Location = Location;
	TouchItem.bMoved = false;
}

void ADualCombatColor_FPSCharacter::EndTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (TouchItem.bIsPressed == false)
	{
		return;
	}
	TouchItem.bIsPressed = false;
}

//Commenting this section out to be consistent with FPS BP template.
//This allows the user to turn without using the right virtual joystick

//void ADualCombatColor_FPSCharacter::TouchUpdate(const ETouchIndex::Type FingerIndex, const FVector Location)
//{
//	if ((TouchItem.bIsPressed == true) && (TouchItem.FingerIndex == FingerIndex))
//	{
//		if (TouchItem.bIsPressed)
//		{
//			if (GetWorld() != nullptr)
//			{
//				UGameViewportClient* ViewportClient = GetWorld()->GetGameViewport();
//				if (ViewportClient != nullptr)
//				{
//					FVector MoveDelta = Location - TouchItem.Location;
//					FVector2D ScreenSize;
//					ViewportClient->GetViewportSize(ScreenSize);
//					FVector2D ScaledDelta = FVector2D(MoveDelta.X, MoveDelta.Y) / ScreenSize;
//					if (FMath::Abs(ScaledDelta.X) >= 4.0 / ScreenSize.X)
//					{
//						TouchItem.bMoved = true;
//						float Value = ScaledDelta.X * BaseTurnRate;
//						AddControllerYawInput(Value);
//					}
//					if (FMath::Abs(ScaledDelta.Y) >= 4.0 / ScreenSize.Y)
//					{
//						TouchItem.bMoved = true;
//						float Value = ScaledDelta.Y * BaseTurnRate;
//						AddControllerPitchInput(Value);
//					}
//					TouchItem.Location = Location;
//				}
//				TouchItem.Location = Location;
//			}
//		}
//	}
//}

void ADualCombatColor_FPSCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void ADualCombatColor_FPSCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void ADualCombatColor_FPSCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ADualCombatColor_FPSCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

bool ADualCombatColor_FPSCharacter::EnableTouchscreenMovement(class UInputComponent* PlayerInputComponent)
{
	if (FPlatformMisc::SupportsTouchInput() || GetDefault<UInputSettings>()->bUseMouseForTouch)
	{
		PlayerInputComponent->BindTouch(EInputEvent::IE_Pressed, this, &ADualCombatColor_FPSCharacter::BeginTouch);
		PlayerInputComponent->BindTouch(EInputEvent::IE_Released, this, &ADualCombatColor_FPSCharacter::EndTouch);

		//Commenting this out to be more consistent with FPS BP template.
		//PlayerInputComponent->BindTouch(EInputEvent::IE_Repeat, this, &ADualCombatColor_FPSCharacter::TouchUpdate);
		return true;
	}
	
	return false;
}
