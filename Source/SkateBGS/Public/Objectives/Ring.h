// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Ring.generated.h"

UCLASS()
class SKATEBGS_API ARing : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ARing();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sine Parameters");
	float Amplitude = 0.25f; //This is initialization, most efficient

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sine Parameters");
	float Period = 5.f; // period = 2pi/K (How long it takes to do a wave, 0 to 1)

	UPROPERTY(EditAnywhere)
	class UNiagaraComponent* VFX;

	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* OverlapEffect;

	UPROPERTY(EditAnywhere)
	USoundBase* OverlapSound;

	float TransformedSin();

	float TransformedCosin();

	UFUNCTION()
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	virtual void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class USphereComponent* Sphere;

	UPROPERTY(EditAnywhere)
	UMaterialInstance* OnMaterial;

	UPROPERTY(EditAnywhere)
	UMaterialInstance* OffMaterial;

	void SetRingInactive();
	void SetRingActive();

private:
	float RunningTime;

};
