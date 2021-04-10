// Copyright Epic Games, Inc. All Rights Reserved.

#include "DeathMatchCharacter.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/InputSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

#include "Weapons/WeaponBase.h"
#include "DeathMatchPlayerState.h"

void ADeathMatchCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(ADeathMatchCharacter, BeginFire, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(ADeathMatchCharacter, EndFire, COND_SkipOwner);
	DOREPLIFETIME(ADeathMatchCharacter, Health);
}

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

	// Spawn weapon
	if (RifleClass != nullptr)
	{
		UWorld* World = GetWorld();
		if (World != nullptr)
		{
			CurrentWeapon = World->SpawnActor<AWeaponBase>(RifleClass);
		}
	}

	// Equip the spawned weapon
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

void ADeathMatchCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	UE_LOG(LogTemp, Warning, TEXT("(Gameflow) ADeathMatchCharacter::PossessedBy"));

	if (NewController != nullptr)
	{
		PlayerState = NewController->GetPlayerState<ADeathMatchPlayerState>();
		if (!ensure(PlayerState != nullptr))
		{
			return;
		}
	}
}

void ADeathMatchCharacter::UnPossessed()
{
	Super::UnPossessed();

}

void ADeathMatchCharacter::OnBeginFire()
{
	if (CurrentWeapon == nullptr)
	{
		return;
	}
	if (GetLocalRole() == ROLE_AutonomousProxy)
	{
		UE_LOG(LogTemp, Warning, TEXT("(Client) OnBeginFire"));
		CurrentWeapon->OnBeginFire();
	}
	Server_OnBeginFire();
}

void ADeathMatchCharacter::Server_OnBeginFire_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("(Server) OnBeginFire"));
	BeginFire++;	// OnRep_BeginFire()
	CurrentWeapon->OnBeginFire();
}

// All Other Clients ( COND_SkipOwner )
void ADeathMatchCharacter::OnRep_BeginFire()
{
	UE_LOG(LogTemp, Warning, TEXT("(Client2) OnBeginFire"));
	CurrentWeapon->OnBeginFire();
}

void ADeathMatchCharacter::OnEndFire()
{
	if (CurrentWeapon == nullptr)
	{
		return;
	}
	if (GetLocalRole() == ROLE_AutonomousProxy)
	{
		UE_LOG(LogTemp, Warning, TEXT("(Client) OnEndFire"));
		CurrentWeapon->OnEndFire();
	}
	Server_OnEndFire();
}

void ADeathMatchCharacter::Server_OnEndFire_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("(Server) OnEndFire"));
	EndFire++;
	CurrentWeapon->OnEndFire();
}

void ADeathMatchCharacter::OnRep_EndFire()
{
	UE_LOG(LogTemp, Warning, TEXT("(Client2) OnEndFire"));
	CurrentWeapon->OnEndFire();
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

void ADeathMatchCharacter::Server_TakeDamage_Implementation(float DamageOnHealth, float DamageOnArmor, AActor* DamageCauser)
{
	UE_LOG(LogTemp, Warning, TEXT("ADeathMatchCharacter::TakeDamage => ( %s ) Took ( %f ) Damage By ( %s )."), *GetName(), DamageOnHealth, *DamageCauser->GetName());

	// TODO: Implement Armor
	Health -= DamageOnHealth;
	if (Health <= 0.f)
	{
		Server_OnDeath(DamageCauser);
	}
}

void ADeathMatchCharacter::Server_OnDeath_Implementation(AActor* DeathCauser)
{
	UE_LOG(LogTemp, Warning, TEXT("ADeathMatchCharacter::OnDeath => ( %s ) is killed by ( %s )."), *GetName(), *DeathCauser->GetName());

	if (!ensure(PlayerState != nullptr))
	{
		return;
	}
	PlayerState->Server_AddNumDeath();

	ADeathMatchCharacter* KillerPlayer = Cast<ADeathMatchCharacter>(DeathCauser);
	if (KillerPlayer != nullptr)
	{
		KillerPlayer->Server_OnKill(this);
	}
}

void ADeathMatchCharacter::Server_OnKill_Implementation(ADeathMatchCharacter* DeadPlayer)
{
	UE_LOG(LogTemp, Warning, TEXT("ADeathMatchCharacter::OnKill => ( %s ) killed ( %s )."), *GetName(), *DeadPlayer->GetName());

	PlayerState->Server_AddNumKill();

	if (DeadPlayer != nullptr)
	{

	}
}