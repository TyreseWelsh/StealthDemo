// Copyright Epic Games, Inc. All Rights Reserved.

#include "StealthDemoCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Kismet/KismetMathLibrary.h"


DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// AStealthDemoCharacter

AStealthDemoCharacter::AStealthDemoCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

void AStealthDemoCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		PlayerController->bShowMouseCursor = true;

		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
			GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
		}
	}
}

void AStealthDemoCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	LookAtMouse();
}

//////////////////////////////////////////////////////////////////////////
// Input

void AStealthDemoCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) 
	{
		// Jumping
		EnhancedInputComponent->BindAction(VaultAction, ETriggerEvent::Started, this, &AStealthDemoCharacter::VaultObject);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AStealthDemoCharacter::Move);

		// Crouching
		EnhancedInputComponent->BindAction(ToggleCrouchAction, ETriggerEvent::Triggered, this, &AStealthDemoCharacter::ToggleCrouch);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AStealthDemoCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AStealthDemoCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AStealthDemoCharacter::LookAtMouse()
{
	if (Controller != nullptr)
	{
		if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
		{
			FHitResult result;
			PlayerController->GetHitResultUnderCursorByChannel(UEngineTypes::ConvertToTraceType(ECC_Camera), true, result);

			double LookToMouseRotation = 0;

			LookToMouseRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), result.Location).Yaw;
			headRotation = LookToMouseRotation - GetActorRotation().Yaw;

			if (headRotation >= 110 || headRotation <= -80)
			{
				SetActorRotation(FRotator(GetActorRotation().Pitch, LookToMouseRotation, GetActorRotation().Roll));
			}
		}
	}
}

void AStealthDemoCharacter::ToggleCrouch()
{
	switch (currentMovementState)
	{
		case AStealthDemoCharacter::Running:
			Crouch();
			break;
		case AStealthDemoCharacter::Crouching:
			Run();
			break;
		default:
			break;
	}
}

void AStealthDemoCharacter::Run()
{
	currentMovementState = EMovementState::Running;
	isCrouched = false;

	GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
}

void AStealthDemoCharacter::Crouch()
{
	currentMovementState = EMovementState::Crouching;
	isCrouched = true;

	GetCharacterMovement()->MaxWalkSpeed = CrouchSpeed;
}

// NOTE: WONT IMPLEMENT YET BUT IS AN IDEA TO REPLACE JUMPING
void AStealthDemoCharacter::VaultObject()
{
}
