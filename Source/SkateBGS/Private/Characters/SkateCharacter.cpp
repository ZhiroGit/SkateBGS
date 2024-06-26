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
#include "UI/CharacterUI.h"
#include "Kismet/GameplayStatics.h"

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

	//Sphere = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Sphere"));
	//Sphere->SetupAttachment(RootComponent);
	//Sphere->SetHiddenInGame(true);
	//Sphere->SetSimulatePhysics(true);
	//Sphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	//Sphere->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);

	if (GetCharacterMovement())
	{
		bUseControllerRotationYaw = false;
		GetCharacterMovement()->bOrientRotationToMovement = false;
		GetCharacterMovement()->MaxWalkSpeed = RegularSpeed;
	}
	if (GetMesh())
	{
		GetMesh()->SetCollisionProfileName(FName("Ragdoll"));
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

	Stamina = MaxStamina;

	if (GetWorld())
	{
		APlayerController* Controller2 = GetWorld()->GetFirstPlayerController();
		if (Controller2 && HUDClass)
		{
			HUD = CreateWidget<UCharacterUI>(Controller2, HUDClass);
			HUD->AddToViewport();
			HUD->UpdateRingCount(RingCounter);
		}
	}
	GetWorldTimerManager().SetTimer(TimerHandle, this, &ASkateCharacter::CountDown, 1.f, true, 0.f);
	
}

// Called every frame
void ASkateCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	//SetPhysicsMovement();

	TraceCollision();
	if (!bIsHoldingMoveAxis)
	{
		Move(FVector2D(0.f, 0.f));
		//MovePhysics(FVector2D(0.f, 0.f));
	}

	if (bCanFlipSkate)
	{
		FlipSkate();
	}

	if (GetCharacterMovement())
	{
		const float Speed = GetVelocity().Size();

		UpdateCamera(Speed);

		if (!GetCharacterMovement()->IsFalling())
		{
			AlignSkate();
		}

		if (Speed > RegularSpeed && !bIsSpeedingUp)
		{
			GetCharacterMovement()->MaxWalkSpeed = FMath::Lerp(GetCharacterMovement()->MaxWalkSpeed, RegularSpeed, Friction / DecelerationRate);
		}
		if (Speed < RegularSpeed - 100 && GetCharacterMovement()->MaxWalkSpeed > RegularSpeed && !bIsSpeedingUp)
		{
			GetCharacterMovement()->MaxWalkSpeed = RegularSpeed;
		}
	}
}

void ASkateCharacter::CountDown()
{
	if (Seconds != 0)
	{
		Seconds -= 1;
	}
	else
	{
		if (Minutes == 0)
		{
			StopAllActions();
			CallResetMenu();
		}
		else
		{
			Minutes -= 1;
			Seconds = 59;
		}
	}

	HUD->UpdateTimer(Minutes, Seconds);

}

void ASkateCharacter::StopAllActions()
{
	GetWorldTimerManager().ClearTimer(TimerHandle);
	ForwardAxis = 0.f;
	RightAxis = 0.f;
	SlowDown();
}

void ASkateCharacter::UpdateCamera(const float& Speed)
{
	if (!GetCharacterMovement()->IsFalling())
	{
		float FOV = FMath::Clamp(Speed / 11.f, 90.f, 105.f);
		CameraFOV = FMath::Lerp(CameraFOV, FOV, 0.05f);
		FollowCamera->SetFieldOfView(CameraFOV);

		float Length = FMath::Clamp(Speed / 3.5f, 300.f, 325.f);
		ArmLength = FMath::Lerp(ArmLength, Length, 0.05f);
		CameraBoom->TargetArmLength = ArmLength;
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
		const float NewYawRotation = MovementVector.X * FMath::Clamp(CurrentSpeed * TurnPercent, 0.25f, 1000000000.f);
		AddActorWorldRotation(FRotator(0.f, NewYawRotation, 0.f));
	}
}

void ASkateCharacter::MoveTrigger(const FInputActionValue& Value)
{
	bIsHoldingMoveAxis = true;
	if (bIsHoldingSpeed)
	{
		SpeedUp();
	}
	Move(Value);
	//MovePhysics(Value);
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
	if (Stamina > MaxStamina / 6.f)
	{
		bIsHoldingSpeed = true;
		SpeedUp();
	}
}

void ASkateCharacter::SpeedUp()
{
	if (!GetCharacterMovement()->IsFalling() && ForwardAxis > 0)
	{
		bIsSpeedingUp = true;
		bIsHoldingSpeed = false;
		if (GetVelocity().Size() < RegularSpeed && ForwardScaleValue > .9f)
		{
			ForwardScaleValue -= 0.2f;
		}
		GetCharacterMovement()->MaxWalkSpeed = MaxSpeed;

		if (GetWorldTimerManager().IsTimerActive(StaminaRegenTimer))
		{
			GetWorldTimerManager().ClearTimer(StaminaRegenTimer);
		}
		GetWorldTimerManager().SetTimer(StaminaDrainTimer, this, &ASkateCharacter::DrainStamina, 0.016f, true, 0.f);
	}
}

void ASkateCharacter::SlowDown()
{
	bIsSpeedingUp = false;
	bIsHoldingSpeed = false;

	if (GetWorldTimerManager().IsTimerActive(StaminaDrainTimer))
	{
		GetWorldTimerManager().ClearTimer(StaminaDrainTimer);
	}

	GetWorldTimerManager().SetTimer(StaminaRegenTimer, this, &ASkateCharacter::RegenStamina, 0.016f, true, 0.f);
}

void ASkateCharacter::DrainStamina()
{
	Stamina = FMath::Clamp(Stamina - StaminaDrainRate, 0.f, MaxStamina);
	if (HUD)
	{
		HUD->SetStaminaPercent(Stamina / MaxStamina);
	}
	if (Stamina <= 0.f)
	{
		SlowDown();
	}
}

void ASkateCharacter::RegenStamina()
{
	Stamina = FMath::Clamp(Stamina + StaminaDrainRate / 2, 0.f, MaxStamina);
	HUD->SetStaminaPercent(Stamina / MaxStamina);
}

void ASkateCharacter::StartJump()
{
	bCanFlipSkate = true;
	if (SkateMesh)
	{
		SkateMesh->SetRelativeRotation(FRotator(20.f, 0.f, 0.f));
	}
}

void ASkateCharacter::CollectRing()
{
	if (HUD)
	{
		RingCounter += 1;
		HUD->UpdateRingCount(RingCounter);
	}

	if (RingCounter >= 33)
	{
		GetWorldTimerManager().ClearTimer(TimerHandle);
		ShowVictoryScreen();
		bHasWon = true;
	}
	RingCollected.Broadcast();
}

void ASkateCharacter::EndJump()
{
	bCanFlipSkate = false;
	if (SkateMesh)
	{
		SkateMesh->SetRelativeRotation(FRotator(0.f, 0.f, 0.f));
	}
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
	const FVector TraceStart = Origin + FVector(0.f, 0.f, 20.f);
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

void ASkateCharacter::TraceCollision()
{
	FVector TraceStart = GetActorLocation();
	FVector TraceEnd = GetActorForwardVector() * 35.f;
	TraceEnd += TraceStart;

	FHitResult HitResult;
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(this);
	UKismetSystemLibrary::BoxTraceSingle(GetWorld(), TraceStart, TraceEnd, FVector(0.f, 20.f, 60.f), GetActorRotation(), TraceTypeQuery1,
		false, ActorsToIgnore, EDrawDebugTrace::None, HitResult, true);

	if (!bHasWon && HitResult.bBlockingHit && GetVelocity().Size() > 750.f)
	{
		Die();
	}
}

void ASkateCharacter::Die()
{
	StopAllActions();
	CallResetMenu();

	if (GetMesh() && SkateMesh)
	{
		GetMesh()->SetSimulatePhysics(true);
		SkateMesh->SetSimulatePhysics(true);
	}
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->StopMovementImmediately();
	}
	if (DeathSound)
	{
		UGameplayStatics::SpawnSoundAtLocation(GetWorld(), DeathSound, GetActorLocation());
	}
	PrimaryActorTick.bCanEverTick = false;
}

void ASkateCharacter::MovePhysics(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();
	ForwardAxis = MovementVector.Y;
	RightAxis = MovementVector.X;
	MovementVector.Y = FMath::Clamp(MovementVector.Y, 0, 1);
	const float CurrentSpeed = GetVelocity().Size();

	Sphere->AddForce(GetActorForwardVector() * RegularSpeed * 110.f * MovementVector.Y);

	float RotationSpeed = RegularSpeed * 1100.f * TurnRate * MovementVector.X;
	Sphere->AddForce(GetActorRightVector() * RotationSpeed);
}

FVector ASkateCharacter::GetSimulatedVelocity()
{
	if (!Sphere) return FVector(0.f, 0.f, 0.f);

	return Sphere->GetPhysicsLinearVelocity();
}

FVector ASkateCharacter::GetFloorNormal(const FVector Origin)
{
	const FVector TraceStart = Origin;
	const FVector TraceEnd = Origin - FVector(0.f, 0.f, 100.f);

	FHitResult HitResult;
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(this);
	UKismetSystemLibrary::LineTraceSingle(GetWorld(), TraceStart, TraceEnd, TraceTypeQuery1,
		false, ActorsToIgnore, EDrawDebugTrace::ForOneFrame, HitResult, true);

	if (HitResult.bBlockingHit)
	{
		return HitResult.Normal;
	}
	return FVector(0.f, 0.f, 1.f);
}

void ASkateCharacter::SetPhysicsMovement()
{
	if (Sphere && SkateMesh)
	{
		SetActorLocation(Sphere->GetComponentLocation(), false, nullptr, ETeleportType::TeleportPhysics);
		const FVector ForwardNormal = GetFloorNormal(SkateMesh->GetSocketLocation(FName("ForwardSocket")));

		FloorNormal = UKismetMathLibrary::VLerp(FloorNormal, ForwardNormal, 0.04f);

		if (GetSimulatedVelocity().Size() > 5.f)
		{
			const FVector Velocity = GetSimulatedVelocity();
			const float Speed = GetSimulatedVelocity().Size();
			const FVector RightVector = UKismetMathLibrary::RotateAngleAxis(Velocity, 90.f, FloorNormal);

			FRotator RotationFromAxes = UKismetMathLibrary::MakeRotationFromAxes(Velocity, RightVector, FloorNormal);

			FRotator NewRotation = UKismetMathLibrary::RLerp(GetActorRotation(), RotationFromAxes, 0.1f, true);
			SetActorRotation(NewRotation);
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
