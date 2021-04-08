// Copyright Epic Games, Inc. All Rights Reserved.

#include "DeathMatchCharacter.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/InputSettings.h"
#include "Kismet/GameplayStatics.h"

#include "Weapons/WeaponBase.h"

ADeathMatchCharacter::ADeathMatchCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-39.56f, 1.75f, 64.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	ArmMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ArmsMesh"));
	ArmMesh->SetOnlyOwnerSee(true);
	ArmMesh->SetupAttachment(FirstPersonCameraComponent);
	ArmMesh->bCastDynamicShadow = false;
	ArmMesh->CastShadow = false;
	ArmMesh->SetRelativeRotation(FRotator(1.9f, -19.19f, 5.2f));
	ArmMesh->SetRelativeLocation(FVector(-0.5f, -4.4f, -155.7f));

	GetMesh()->SetOwnerNoSee(true);
}

FVector ADeathMatchCharacter::GetCameraLocation() const
{
	return FirstPersonCameraComponent->GetComponentLocation();
}

void ADeathMatchCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	if (RifleClass != nullptr)
	{
		UWorld* World = GetWorld();
		if (World != nullptr)
		{
			CurrentWeapon = World->SpawnActor<AWeaponBase>(RifleClass);
		}
	}

	if (CurrentWeapon != nullptr)
	{
		CurrentWeapon->OnWeaponEquipped(this);
	}
}

void ADeathMatchCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	check(PlayerInputComponent);

	// Bind jump events
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	// Bind fire event
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ADeathMatchCharacter::OnBeginFire);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ADeathMatchCharacter::OnEndFire);

	// Bind movement events
	PlayerInputComponent->BindAxis("MoveForward", this, &ADeathMatchCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ADeathMatchCharacter::MoveRight);

	// "turn" handles devices that provide an absolute delta, such as a mouse.
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
}

void ADeathMatchCharacter::OnBeginFire()
{
	if (CurrentWeapon != nullptr)
	{
		CurrentWeapon->OnBeginFire();

		if (GetLocalRole() == ROLE_Authority)
		{
			Multicast_OnBeginFire();
		}
		else
		{
			Server_OnBeginFire();
		}
	}
}

void ADeathMatchCharacter::Server_OnBeginFire_Implementation()
{
	Multicast_OnBeginFire();
}

void ADeathMatchCharacter::Multicast_OnBeginFire_Implementation()
{
	if (IsLocallyControlled())
	{
		return;
	}

	if (CurrentWeapon != nullptr)
	{
		CurrentWeapon->OnBeginFire();
	}
}

void ADeathMatchCharacter::OnEndFire()
{
	if (CurrentWeapon != nullptr)
	{
		CurrentWeapon->OnEndFire();

		if (GetLocalRole() == ROLE_Authority)
		{
			Multicast_OnEndFire();
		}
		else
		{
			Server_OnEndFire();
		}
	}
}

void ADeathMatchCharacter::Server_OnEndFire_Implementation()
{
	Multicast_OnEndFire();
}

void ADeathMatchCharacter::Multicast_OnEndFire_Implementation()
{
	if (IsLocallyControlled())
	{
		return;
	}

	if (CurrentWeapon != nullptr)
	{
		CurrentWeapon->OnEndFire();
	}
}

void ADeathMatchCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void ADeathMatchCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
}