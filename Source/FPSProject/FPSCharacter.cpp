// Fill out your copyright notice in the Description page of Project Settings.

#include "FPSProject.h"
#include "FPSCharacter.h"

// Sets default values
AFPSCharacter::AFPSCharacter() {
  // Set this character to call Tick() every frame.  You can turn this off to
  // improve performance if you don't need it.
  PrimaryActorTick.bCanEverTick = true;
}

AFPSCharacter::AFPSCharacter(const FObjectInitializer &ObjectInitializer)
    : Super(ObjectInitializer) {
  // Create a camera component
  ThirdPersonCameraComponent =
      ObjectInitializer.CreateDefaultSubobject<UCameraComponent>(
          this, TEXT("FirstPersonCamera"));
  ThirdPersonCameraComponent->AttachParent = GetCapsuleComponent();

  // Position the camera a bit above the eyes

  ThirdPersonCameraComponent->RelativeLocation =
      FVector(0, -30.0f, 50.0f + BaseEyeHeight);

  ParticleSystemFire =
      CreateDefaultSubobject<UParticleSystemComponent>(TEXT("FireParticles"));
  ParticleSystemFire->SetRelativeLocation(FVector(10.0f, 1.5f, -70.0f));
  static ConstructorHelpers::FObjectFinder<UParticleSystem> ParticleAsset(
      TEXT("/Game/ParticleSystem/Particles/P_Fire.P_Fire"));
  if (ParticleAsset.Succeeded()) {
    ParticleSystemFire->SetTemplate(ParticleAsset.Object);
  }
  ParticleSystemFire->AttachTo(RootComponent);
  ParticleSystemFire->bAutoActivate = false;

  ParticleSystemExplosion = CreateDefaultSubobject<UParticleSystemComponent>(
      TEXT("ExplosionParticles"));
  ParticleSystemExplosion->SetRelativeLocation(FVector(10.0f, 1.5f, -70.0f));
  static ConstructorHelpers::FObjectFinder<UParticleSystem> ParticleAssetTwo(
      TEXT("/Game/ParticleSystem/Particles/P_Explosion.P_Explosion"));
  if (ParticleAssetTwo.Succeeded()) {
    ParticleSystemExplosion->SetTemplate(ParticleAssetTwo.Object);
  }
  ParticleSystemExplosion->AttachTo(RootComponent);
  ParticleSystemExplosion->bAutoActivate = false;

  ParticleSystemSmoking = CreateDefaultSubobject<UParticleSystemComponent>(
      TEXT("SmokingParticles"));
  ParticleSystemSmoking->SetRelativeLocation(FVector(10.0f, 1.5f, -70.0f));
  static ConstructorHelpers::FObjectFinder<UParticleSystem> ParticleAssetTree(
      TEXT("/Game/ParticleSystem/Particles/P_Smoke.P_Smoke"));
  if (ParticleAssetTree.Succeeded()) {
    ParticleSystemSmoking->SetTemplate(ParticleAssetTree.Object);
  }
  ParticleSystemSmoking->AttachTo(RootComponent);
  ParticleSystemSmoking->bAutoActivate = false;

  bCanBeDamaged = false;
  InitialLifeSpan = 0;

  bEndGame = false;
  _bounceSpeed = 25;
  P_superMeter = 1.0f;
  _defaultJumpVelocity = 420;
  _superMeterSequenceState = 0;
  _superMeterIncrementValue = 0.005f;
}

// Called when the game starts or when spawned
void AFPSCharacter::BeginPlay() {
  Super::BeginPlay();
  GetCharacterMovement()->GravityScale = 0.8;

  // Use controller rotations
  bUseControllerRotationPitch = true;
  bUseControllerRotationRoll = true;
  bUseControllerRotationYaw = true;
}

// Called every frame
void AFPSCharacter::Tick(float DeltaTime) {
  Super::Tick(DeltaTime);

  const FRotator Rotation = Controller->GetControlRotation();
  const FVector Direction = FRotationMatrix(Rotation).GetScaledAxis(EAxis::Z);

  _cameraRotatorClamp =
      FRotator(FMath::ClampAngle(
                   ThirdPersonCameraComponent->RelativeRotation.Pitch, 0, 10),
               ThirdPersonCameraComponent->RelativeRotation.Yaw,
               FMath::ClampAngle(
                   ThirdPersonCameraComponent->RelativeRotation.Roll, -20, 20));

  ThirdPersonCameraComponent->RelativeRotation = _cameraRotatorClamp;

  // Keep jumping
  if (!GetCharacterMovement()->IsFalling()) {
    bPressedJump = true;
  }

  // currently tilting is restricted until character falls animation
  // is implemented
  Controller->SetControlRotation(
      FRotator(FMath::ClampAngle(Rotation.Pitch, -20.0f, 20.0f), Rotation.Yaw,
               FMath::ClampAngle(Rotation.Roll, -20.0f, 20.0f)));

  if (_superMeterSequenceState > 0) {
    SuperMeterSequence();
  }

  if (_superMeterSequenceState == 1) {
    if (!ParticleSystemSmoking->bIsActive) {
      SmokeParticleToggle();
    }
  }

  if ((_superMeterSequenceState == 0 || _superMeterSequenceState == 3) &&
      P_superMeter < 1.0f && (!FMath::IsNearlyZero(P_superMeter))) {
    IncrementSuperMeter();
  } else if (_superMeterSequenceState == 1 &&
             (!FMath::IsNearlyZero(P_superMeter))) {
    DecrementSuperMeter();
  }
  /*
   else if (FMath::IsNearlyZero(P_superMeter)) {

   // perform time out
   // perhaps add another superMeterSequenceState
   }
   */

  AddMovementInput(Direction, _bounceSpeed);
}

// Called to bind functionality to input
void AFPSCharacter::SetupPlayerInputComponent(
    class UInputComponent *InputComponent) {
  Super::SetupPlayerInputComponent(InputComponent);

  // Use mouse to control camera direction
  InputComponent->BindAxis("TiltRoll", this,
                           &AFPSCharacter::AddControllerRollInput);
  InputComponent->BindAxis("TiltPitch", this,
                           &AFPSCharacter::AddControllerPitchInput);
  // Use 'A' and 'D' keys to rotate yaw
  InputComponent->BindAxis("RotateYaw", this,
                           &AFPSCharacter::AddControllerYawInput);
  // call meterburnpress when button is pressed
  InputComponent->BindAction("SuperJump", IE_Pressed, this,
                             &AFPSCharacter::MeterBurnPress);
  // call meterburndepress when button is released
  InputComponent->BindAction("SuperJump", IE_Released, this,
                             &AFPSCharacter::MeterBurnDepress);

  // testing fire toggle
  InputComponent->BindAction("FireToggle", IE_Pressed, this,
                             &AFPSCharacter::SmokeParticleToggle);
}

// reaction of press of super meter burn
void AFPSCharacter::MeterBurnPress() {
  if (P_superMeter > 0) {
    SetSuperGravity();
    _superMeterSequenceState = 1;
  }
}

// reaction of depress of super meter burn
void AFPSCharacter::MeterBurnDepress() { _superMeterSequenceState = 2; }

/*
Sequence of events related to super meter by state:
 0 = No super meter, character is bouncing normally
 1 = Super meter button is being pressed and quick bouncing is being performed
 2 = Button has been depressed perform high jump
*/
void AFPSCharacter::SuperMeterSequence() {
  if (_superMeterSequenceState == 2 && (!GetCharacterMovement()->IsFalling())) {
    SmokeParticleToggle();
    ExplosionParticleToggle();
    ResetGravity();
    SetSuperJumpVelocity();
    _superMeterSequenceState++;
  } else if (_superMeterSequenceState == 3 &&
             (!GetCharacterMovement()->IsFalling())) {
    ResetJumpVelocity();
    _superMeterSequenceState = 0;
    ExplosionParticleToggle();
    if (ParticleSystemSmoking->bIsActive) {
      SmokeParticleToggle();
    }
  }
}

void AFPSCharacter::FireParticleToggle() {
  if (ParticleSystemFire && ParticleSystemFire->Template) {
    ParticleSystemFire->ToggleActive();
  }
}

void AFPSCharacter::SmokeParticleToggle() {
  if (ParticleSystemSmoking && ParticleSystemSmoking->Template) {
    ParticleSystemSmoking->ToggleActive();
  }
}

void AFPSCharacter::ExplosionParticleToggle() {
  if (ParticleSystemExplosion && ParticleSystemExplosion->Template) {
    ParticleSystemExplosion->ToggleActive();
  }
}

// accessor methods:
void AFPSCharacter::SetSuperJumpVelocity() {
  GetCharacterMovement()->JumpZVelocity = _defaultJumpVelocity * 1.8;
}

void AFPSCharacter::ResetJumpVelocity() {
  GetCharacterMovement()->JumpZVelocity = _defaultJumpVelocity;
}

void AFPSCharacter::SetSuperGravity() {
  GetCharacterMovement()->GravityScale = 3;
}

void AFPSCharacter::ResetGravity() {
  GetCharacterMovement()->GravityScale = 0.8;
}

void AFPSCharacter::IncrementSuperMeter() {
  if (P_superMeter < 1) {
    P_superMeter += _superMeterIncrementValue;
  }
}

void AFPSCharacter::DecrementSuperMeter() {
  if (P_superMeter > 0) {
    P_superMeter -= _superMeterIncrementValue;
  }
}

bool AFPSCharacter::IsFireParticleActive() {
  return ParticleSystemFire->bIsActive;
}

void AFPSCharacter::SetP_SuperMeter(int32 v) { P_superMeter = v; }

int32 AFPSCharacter::GetP_SuperMeter() { return P_superMeter; }
