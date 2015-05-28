// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "GameFramework/Character.h"
#include "Math/UnrealMathUtility.h"
#include "Kismet/KismetMathLibrary.h"
#include "FPSCharacter.generated.h"

UCLASS()
class FPSPROJECT_API AFPSCharacter : public ACharacter {
  GENERATED_BODY()

public:
  // Sets default values for this character's properties
  AFPSCharacter();

  // Parametrized constructor
  AFPSCharacter(const FObjectInitializer &ObjectInitializer);

  // Called when the game starts or when spawned
  virtual void BeginPlay() override;

  // Called every frame
  virtual void Tick(float DeltaSeconds) override;

  // Called to bind functionality to input
  virtual void
  SetupPlayerInputComponent(class UInputComponent *InputComponent) override;

  // Super meter allows player to super bounce
  UFUNCTION(BlueprintCallable, Category = "Character",
            meta = (FriendlyName = "Inc Super Meter"))
  void IncrementSuperMeter();
  UFUNCTION(BlueprintCallable, Category = "Character",
            meta = (FriendlyName = "Dec Super Meter"))
  void DecrementSuperMeter();

  // accessor methods
  UFUNCTION(BlueprintCallable, Category = "Character",
            meta = (FriendlyName = "Set Super Meter"))
  void SetP_SuperMeter(float v);
  UFUNCTION(BlueprintPure, Category = "Character",
            meta = (FriendlyName = "Get Super Meter"))
  float GetP_SuperMeter();

  UFUNCTION(BlueprintCallable, Category = "Character",
            meta = (FriendlyName = "Is Fire Active"))
  bool IsFireParticleActive();

  // first person camera
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
  UCameraComponent *ThirdPersonCameraComponent;

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character")
  float P_superMeter; // Holds current value of super jump meter
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character")
  bool bFallenOffPogoStick; // Is true when character tilts too far
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character")
  bool bMeterIsSpent; // Is true when P_superMeter is NearlyZero
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
  int32 _superMeterSequenceState;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
  bool bEndGame; // Disables Movement and Input (not sure how to do this in code
                 // so I'm sending a bool to blueprints)

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
  UParticleSystemComponent *ParticleSystemFire;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
  UParticleSystemComponent *ParticleSystemSmoking;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
  UParticleSystemComponent *ParticleSystemExplosion;

protected:
  void SetSuperJumpVelocity();
  void ResetJumpVelocity();
  void SetSuperGravity();
  void ResetGravity();
  void SetSuperBounceSpeed();
  void FireParticleToggle();
  void SmokeParticleToggle();
  void ExplosionParticleToggle();

  void MeterBurnDepress();
  void MeterBurnPress();
  void SuperMeterSequence();

  int32 _bounceSpeed;
  int16 _defaultJumpVelocity;
  float _superMeterIncrementValue;
  FRotator _cameraRotatorClamp;
};

static void jsphPrint(FString inString) {
  if (GEngine) {
    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, inString);
  }
}