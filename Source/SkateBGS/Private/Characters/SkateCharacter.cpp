// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/SkateCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
ASkateCharacter::ASkateCharacter()
{
	//Code From the default Unreal Character
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	//////////////////////////////////////////////////

	SkateMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SkateMesh"));
	SkateMesh->SetupAttachment(RootComponent);

	if (GetCharacterMovement())
	{
		bUseControllerRotationYaw = false;
		GetCharacterMovement()->bOrientRotationToMovement = false;
		GetCharacterMovement()->MaxWalkSpeed = RegularSpeed;
	}

}

// Called when the game starts or when spawned
void ASkateCharacter::BeginPlay()
{
	Super::BeginPlay();

	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
	
}

void ASkateCharacter::Look(const FInputActionValue& Value)
{
	//Code From the default Unreal Character
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
	///////////////////////////////////////////
}

void ASkateCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();
	ForwardAxis = MovementVector.Y;
	RightAxis = MovementVector.X;
	MovementVector.Y = FMath::Clamp(MovementVector.Y, 0, 1);
	const float CurrentSpeed = GetVelocity().Size();

	if (Controller != nullptr)
	{
		// add movement
		float DecelerationScale = GetDecelerationScale(CurrentSpeed);
		float ZForward = 0.f;
		if (!GetCharacterMovement()->IsFalling())
		{
			ZForward = FMath::Clamp(SkateMesh->GetForwardVector().Z * 3, -0.8f, 0.8f);
		}
		ForwardScaleValue = FMath::Lerp(ForwardScaleValue, MovementVector.Y - ZForward, Friction * DecelerationScale);
		AddMovementInput(GetActorForwardVector(), ForwardScaleValue);
		//RightScaleValue = FMath::Lerp(RightScaleValue, MovementVector.X, 0.02f);

		//add rotation
		const float TurnPercent = TurnRate / GetCharacterMovement()->MaxWalkSpeed;
		const float NewYawRotation = MovementVector.X * (CurrentSpeed * TurnPercent);
		AddActorWorldRotation(FRotator(0.f, NewYawRotation, 0.f));
	}
}


float ASkateCharacter::GetDecelerationScale(float CurrentSpeed)
{
	float DecelerationScale = 1.f;
	if (CurrentSpeed > 400.f && ForwardAxis == 0)
	{
		float DecMultiplier = CurrentSpeed >= RegularSpeed + 100.f ? CurrentSpeed / 30.f : 1.f;
		DecelerationScale /= DecelerationRate * DecMultiplier;
		return DecelerationScale;
	}
	return 1.f;
}

void ASkateCharacter::MoveTrigger(const FInputActionValue& Value)
{
	bIsHoldingMoveAxis = true;
	if (bIsHoldingSpeed)
	{
		SpeedUp();
	}
	Move(Value);
}

void ASkateCharacter::ReleaseTrigger()
{
	bIsHoldingMoveAxis = false;
}

void ASkateCharacter::GetFootSockets(FVector& FrontFoot, FVector& BackFoot)
{
	if (SkateMesh)
	{
		FrontFoot = SkateMesh->GetSocketLocation("FrontFootSocket");
		BackFoot = SkateMesh->GetSocketLocation("BackFootSocket");
	}
}

void ASkateCharacter::SpeedTrigger()
{
	bIsHoldingSpeed = true;
	SpeedUp();
}

void ASkateCharacter::SpeedUp()
{
	if (!GetCharacterMovement()->IsFalling() && ForwardAxis > 0)
	{
		bIsSpeedingUp = true;
		bIsHoldingSpeed = false;
		if (GetVelocity().Size() > RegularSpeed - 100.f)
		{
			ForwardScaleValue -= 0.25f;
		}
		GetCharacterMovement()->MaxWalkSpeed = MaxSpeed;
	}
}

void ASkateCharacter::StartJump()
{
	bCanFlipSkate = true;
	if (SkateMesh)
	{
		SkateMesh->SetRelativeRotation(FRotator(20.f, 0.f, 0.f));
	}
}

void ASkateCharacter::EndJump()
{
	bCanFlipSkate = false;
	if (SkateMesh)
	{
		SkateMesh->SetRelativeRotation(FRotator(0.f, 0.f, 0.f));
	}
}

void ASkateCharacter::SlowDown()
{
	bIsSpeedingUp = false;
	bIsHoldingSpeed = false;
}

void ASkateCharacter::Jump()
{
	if (!GetCharacterMovement()->IsFalling())
	{
		if (JumpMontage)
		{
			PlayAnimMontage(JumpMontage, 1.f, FName("Jump"));
		}
		Super::Jump();
	}
}

void ASkateCharacter::FlipSkate()
{
	if (SkateMesh)
	{
		SkateMesh->AddLocalRotation(FRotator(0.f, 0.f, 20.f));
	}
}

// Called every frame
void ASkateCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!bIsHoldingMoveAxis)
	{
		Move(FVector2D(0.f, 0.f));
	}

	if (bCanFlipSkate)
	{
		FlipSkate();
	}

	if (GetCharacterMovement())
	{
		if (!GetCharacterMovement()->IsFalling())
		{
			AlignSkate();
		}

		if (GetVelocity().Size() > RegularSpeed && !bIsSpeedingUp)
		{
			GetCharacterMovement()->MaxWalkSpeed = FMath::Lerp(GetCharacterMovement()->MaxWalkSpeed, RegularSpeed, Friction / DecelerationRate);
		}
		if (GetVelocity().Size() < RegularSpeed - 100 && GetCharacterMovement()->MaxWalkSpeed > RegularSpeed && !bIsSpeedingUp)
		{
			GetCharacterMovement()->MaxWalkSpeed = RegularSpeed;
		}
	}
}

// Called to bind functionality to input
void ASkateCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	//Code From the default Unreal Character
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {

		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ASkateCharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// SpeedUp
		EnhancedInputComponent->BindAction(SpeedAction, ETriggerEvent::Started, this, &ASkateCharacter::SpeedTrigger);
		EnhancedInputComponent->BindAction(SpeedAction, ETriggerEvent::Completed, this, &ASkateCharacter::SlowDown);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASkateCharacter::MoveTrigger);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &ASkateCharacter::ReleaseTrigger);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASkateCharacter::Look);
	}
	//////////////////////////////////////////////////////////////

}

void ASkateCharacter::AlignSkate()
{
	if (SkateMesh)
	{
		//Trace the floor to align Vertical Skate orientation
		const FVector ForwardLocation = TraceFloor(SkateMesh->GetSocketLocation(FName("ForwardSocket")));
		const FVector BackwardLocation = TraceFloor(SkateMesh->GetSocketLocation(FName("BackwardSocket")));
		const FRotator NewRotationV = UKismetMathLibrary::FindLookAtRotation(BackwardLocation, ForwardLocation);

		//Trace the floor to align Horizontal Skate orientation
		const FVector LeftLocation = TraceFloor(SkateMesh->GetSocketLocation(FName("LeftWheel")));
		const FVector RightLocation = TraceFloor(SkateMesh->GetSocketLocation(FName("RightWheel")));
		const FRotator NewRotationH = UKismetMathLibrary::FindLookAtRotation(RightLocation, LeftLocation);

		const FRotator NewRotation(NewRotationV.Pitch, NewRotationV.Yaw, NewRotationH.Pitch);
		FRotator TargetRotation = FMath::RInterpTo(SkateMesh->GetComponentRotation(), NewRotation, GetWorld()->GetDeltaSeconds(), 20.f);
		SkateMesh->SetWorldRotation(TargetRotation);

	}
}

FVector ASkateCharacter::TraceFloor(const FVector Origin)
{
	const FVector TraceStart = Origin;
	const FVector TraceEnd = Origin - FVector(0.f, 0.f, 50.f);

	FHitResult HitResult;
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(this);
	UKismetSystemLibrary::LineTraceSingle(GetWorld(), TraceStart, TraceEnd, TraceTypeQuery1,
		false, ActorsToIgnore, EDrawDebugTrace::None, HitResult, true);

	if (HitResult.bBlockingHit)
	{
		return HitResult.Location;
	}
	return Origin;
}
