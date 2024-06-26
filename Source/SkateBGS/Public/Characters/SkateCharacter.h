// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SkateCharacter.generated.h"

class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
class USpringArmComponent;
class UCameraComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRingCollected);

UCLASS()
class SKATEBGS_API ASkateCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ASkateCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	void MoveTrigger(const FInputActionValue& Value);

	void MovePhysics(const FInputActionValue& Value);

	void ReleaseTrigger();

	void SlowDown();

	virtual void Jump() override;

	void SpeedUp();

	UFUNCTION(BlueprintCallable)
	void StartJump();

	UFUNCTION(BlueprintCallable)
	void EndJump();

	UFUNCTION(BlueprintPure)
	void GetFootSockets(FVector &FrontFoot, FVector &BackFoot);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float ForwardAxis;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float RightAxis;

	UPROPERTY(EditAnywhere, category = "Movement")
	float RegularSpeed = 900.f;

	UPROPERTY(EditAnywhere, category = "Movement")
	float MaxSpeed = 1200.f;

	UPROPERTY(EditAnywhere, category = "Movement")
	float MaxStamina = 100.f;

	UPROPERTY(EditAnywhere, category = "Movement")
	float StaminaDrainRate = 1.f;

	UPROPERTY(EditAnywhere, category = "Movement")
	float TurnRate = 1.5f;

	UPROPERTY(EditAnywhere, category = "Movement")
	float Friction = 0.01f;

	UPROPERTY(EditAnywhere, category = "Movement")
	float DecelerationRate = 5.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, category = "Movement")
	float ForwardScaleValue;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, category = "Movement")
	bool bIsSpeedingUp = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, category = "Movement")
	float Stamina = 100.f;

	UPROPERTY(EditAnywhere, category = "Animations")
	UAnimMontage* JumpMontage;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class UCharacterUI> HUDClass;

	UCharacterUI* HUD;

	UPROPERTY(EditAnywhere, category = "Time")
	int32 Minutes = 1.f;

	UPROPERTY(EditAnywhere, category = "Time")
	int32 Seconds = 59.f;

	UFUNCTION(BlueprintImplementableEvent)
	void CallResetMenu();

	UFUNCTION(BlueprintImplementableEvent)
	void ShowVictoryScreen();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bHasWon = false;

	UPROPERTY(EditAnywhere)
	USoundBase* DeathSound;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//Code From the default Unreal Character
	// 
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SpeedAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	///////////////////////////////////////////
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* SkateMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* Sphere;

	FOnRingCollected RingCollected;

	void CollectRing();

private:
	bool bIsHoldingMoveAxis = false;
	bool bIsHoldingSpeed = false;
	bool bCanFlipSkate = false;
	float RightScaleValue;
	float GetDecelerationScale(float CurrentSpeed);
	FVector FloorNormal = FVector(0.f, 0.f, 1.f);

	void AlignSkate();
	FVector TraceFloor(const FVector Origin);
	void SpeedTrigger();
	void FlipSkate();

	FVector GetSimulatedVelocity();
	FVector GetFloorNormal(const FVector Origin);
	void SetPhysicsMovement();

	void UpdateCamera(const float& Speed);
	float CameraFOV = 90.f;
	float ArmLength = 300.f;

	FTimerHandle StaminaDrainTimer;
	FTimerHandle StaminaRegenTimer;
	FTimerHandle TimerHandle;
	void DrainStamina();
	void RegenStamina();

	int32 RingCounter = 0;
	void CountDown();
	void StopAllActions();
	void TraceCollision();
	void Die();
};
