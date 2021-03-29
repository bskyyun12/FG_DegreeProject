// Fill out your copyright notice in the Description page of Project Settings.


#include "FPSCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetSystemLibrary.h"

#include "Animation/AnimInstance.h"
#include "AnimInstances/FPSAnimInterface.h"
#include "Components/HealthComponent.h"
#include "FPSPlayerControllerInterface.h"
#include "FPSPlayerController.h"
#include "Weapons/FPSWeaponInterface.h"

AFPSCharacter::AFPSCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	CameraContainer = CreateDefaultSubobject<UBoxComponent>(TEXT("CameraContainer"));
	CameraContainer->SetupAttachment(RootComponent);

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	FollowCamera->SetupAttachment(CameraContainer);

	FPSArmMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FPS_Arms"));
	FPSArmMesh->SetupAttachment(FollowCamera);

	HandCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("HandCollider"));
	HandCollider->SetupAttachment(FollowCamera);
	HandCollider->OnComponentBeginOverlap.AddDynamic(this, &AFPSCharacter::OnBeginOverlapHandCollider);
	HandCollider->OnComponentEndOverlap.AddDynamic(this, &AFPSCharacter::OnEndOverlapHandCollider);

	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	HealthComponent->OnTakeDamage.AddDynamic(this, &AFPSCharacter::OnTakeDamage);
	HealthComponent->OnHealthAcquired.AddDynamic(this, &AFPSCharacter::OnHealthAcquired);
	HealthComponent->OnDeath.AddDynamic(this, &AFPSCharacter::OnDeath);
	HealthComponent->OnUpdateHealthArmorUI.AddDynamic(this, &AFPSCharacter::OnUpdateHealthArmorUI);
}

void AFPSCharacter::BeginPlay()
{
	Super::BeginPlay();

	FPSCharacterMesh = GetMesh();
	if (!ensure(FPSCharacterMesh != nullptr))
	{
		return;
	}
	CharacterCapsuleComponent = GetCapsuleComponent();
	if (!ensure(CharacterCapsuleComponent != nullptr))
	{
		return;
	}

	DefaultCameraRelativeLocation = CameraContainer->GetRelativeLocation();
	DefaultCharacterMeshRelativeTransform = FPSCharacterMesh->GetRelativeTransform();
}

void AFPSCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AFPSCharacter::OnBeginFire);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &AFPSCharacter::OnEndFire);

	PlayerInputComponent->BindAction("Pickup", IE_Pressed, this, &AFPSCharacter::Pickup);

	PlayerInputComponent->BindAction("Drop", IE_Pressed, this, &AFPSCharacter::Drop);

	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &AFPSCharacter::Reload);

	PlayerInputComponent->BindAction<FOneBooleanDelegate>("ScoreBoard", IE_Pressed, this, &AFPSCharacter::ToggleScoreBoardWidget, true);
	PlayerInputComponent->BindAction<FOneBooleanDelegate>("ScoreBoard", IE_Released, this, &AFPSCharacter::ToggleScoreBoardWidget, false);

	PlayerInputComponent->BindAxis("MoveForward", this, &AFPSCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AFPSCharacter::MoveRight);

	PlayerInputComponent->BindAxis("Turn", this, &AFPSCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &AFPSCharacter::LookUp);
}

void AFPSCharacter::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AFPSCharacter::MoveRight(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

void AFPSCharacter::Turn(float Value)
{
	AddControllerYawInput(Value);
}

void AFPSCharacter::LookUp(float Value)
{
	AddControllerPitchInput(Value);

	Server_LookUp(GetControlRotation());
}

void AFPSCharacter::Server_LookUp_Implementation(FRotator CameraRot)
{
	Multicast_LookUp(CameraRot);
}

void AFPSCharacter::Multicast_LookUp_Implementation(FRotator CameraRot)
{
	if (!IsLocallyControlled())
	{
		FollowCamera->SetWorldRotation(CameraRot);

		if (FPSCharacterMesh != nullptr)
		{
			UAnimInstance* FPSAnimInstance = FPSCharacterMesh->GetAnimInstance();
			if (FPSAnimInstance != nullptr && UKismetSystemLibrary::DoesImplementInterface(FPSAnimInstance, UFPSAnimInterface::StaticClass()))
			{
				IFPSAnimInterface::Execute_UpdateSpineAngle(FPSAnimInstance, CameraRot.Pitch);
			}
		}
	}
}

void AFPSCharacter::OnBeginFire()
{
	if (CurrentWeapon != nullptr)
	{
		Server_OnBeginFire(CurrentWeapon);
		CurrentWeapon->Client_OnBeginFireWeapon();
	}
}

void AFPSCharacter::Server_OnBeginFire_Implementation(AFPSWeaponBase* Weapon)
{
	UE_LOG(LogTemp, Warning, TEXT("AFPSCharacter::Server_OnBeginFire_Implementation"));
	if (Weapon != nullptr)
	{
		Weapon->Server_OnBeginFireWeapon();
	}
}

void AFPSCharacter::OnEndFire()
{
	if (CurrentWeapon != nullptr)
	{
		Server_OnEndFire(CurrentWeapon);
		CurrentWeapon->Client_OnEndFireWeapon();
	}
}

void AFPSCharacter::Server_OnEndFire_Implementation(AFPSWeaponBase* Weapon)
{
	UE_LOG(LogTemp, Warning, TEXT("AFPSCharacter::Server_OnEndFire_Implementation"));
	if (Weapon != nullptr)
	{
		Weapon->Server_OnEndFireWeapon();
	}
}

void AFPSCharacter::Pickup()
{
	if (CurrentFocus != nullptr && CurrentWeapon == nullptr)
	{
		EquipWeapon(CurrentFocus.Get());
	}
}

void AFPSCharacter::EquipWeapon(AFPSWeaponBase* Weapon)
{
	if (Weapon != nullptr)
	{
		CurrentWeapon = Weapon;

		Server_EquipWeapon(Weapon);

		// Play Equip animation
		UAnimInstance* AnimInstance = FPSArmMesh->GetAnimInstance();
		if (AnimInstance != nullptr)
		{
			FWeaponInfo WeaponInfo = Weapon->GetWeaponInfo();
			if (WeaponInfo.FP_EquipAnim != nullptr)
			{
				AnimInstance->Montage_Play(WeaponInfo.FP_EquipAnim);
			}
		}
	}
}

void AFPSCharacter::Server_EquipWeapon_Implementation(AFPSWeaponBase* Weapon)
{
	if (Weapon != nullptr)
	{
		Weapon->Server_OnWeaponEquipped(this);
	}
}

void AFPSCharacter::Drop()
{
	if (CurrentWeapon != nullptr)
	{
		Server_DropWeapon(CurrentWeapon);
		CurrentWeapon = nullptr;
	}
}

void AFPSCharacter::Server_DropWeapon_Implementation(AFPSWeaponBase* Weapon)
{
	if (Weapon != nullptr)
	{
		Weapon->Server_OnWeaponDroped();
	}
}

void AFPSCharacter::Reload()
{
	if (CurrentWeapon != nullptr && CurrentWeapon->CanReload())
	{
		Server_Reload(CurrentWeapon);
		CurrentWeapon->Client_OnReload();

		// Play ArmsReloadAnim
		UAnimInstance* AnimInstance = FPSArmMesh->GetAnimInstance();
		if (AnimInstance != nullptr)
		{
			FWeaponInfo WeaponInfo = CurrentWeapon->GetWeaponInfo();
			if (WeaponInfo.FP_ArmsReloadAnim != nullptr)
			{
				AnimInstance->Montage_Play(WeaponInfo.FP_ArmsReloadAnim);
			}
		}
	}
}

void AFPSCharacter::Server_Reload_Implementation(AFPSWeaponBase* Weapon)
{
	if (Weapon != nullptr)
	{
		Weapon->Server_OnReload();

		// TODO: Play TPCharacterReloadAnim (For other clients)
	}
}

void AFPSCharacter::ToggleScoreBoardWidget(bool bDisplay)
{
	if (GetController() != nullptr && UKismetSystemLibrary::DoesImplementInterface(GetController(), UFPSPlayerControllerInterface::StaticClass()))
	{
		IFPSPlayerControllerInterface::Execute_ToggleScoreBoardWidget(GetController(), bDisplay);
	}
}

float AFPSCharacter::GetHealth_Implementation()
{
	if (HealthComponent != nullptr)
	{
		return HealthComponent->GetHealth();
	}

	return -1.f;
}

float AFPSCharacter::GetArmor_Implementation()
{
	if (HealthComponent != nullptr)
	{
		return HealthComponent->GetArmor();
	}

	return -1.f;
}

void AFPSCharacter::OnBeginOverlapHandCollider(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor != nullptr && UKismetSystemLibrary::DoesImplementInterface(OtherActor, UFPSWeaponInterface::StaticClass()))
	{
		CurrentFocus = IFPSWeaponInterface::Execute_GetWeapon(OtherActor);
	}
	// TODO: What if OtherActor is not a weapon? It could be a wall to climb.
}

void AFPSCharacter::OnEndOverlapHandCollider(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor != nullptr && UKismetSystemLibrary::DoesImplementInterface(OtherActor, UFPSWeaponInterface::StaticClass()))
	{
		if (CurrentFocus == IFPSWeaponInterface::Execute_GetWeapon(OtherActor))
		{
			CurrentFocus = nullptr;
		}
	}
}

void AFPSCharacter::TakeDamage_Implementation(AActor* DamageCauser, float DamageOnHealth, float DamageOnArmor)
{
	if (HealthComponent != nullptr)
	{
		HealthComponent->Server_TakeDamage(DamageCauser, DamageOnHealth, DamageOnArmor);
	}
}

// HealthComponent Delegate binding
void AFPSCharacter::OnTakeDamage(AActor* DamageSource)
{
	if (GetController() != nullptr && UKismetSystemLibrary::DoesImplementInterface(GetController(), UFPSPlayerControllerInterface::StaticClass()))
	{
		UE_LOG(LogTemp, Warning, TEXT("( %s ) is attacked by ( %s )"), *this->GetName(), *DamageSource->GetName());
		IFPSPlayerControllerInterface::Execute_OnTakeDamage(GetController());
	}
}

// HealthComponent Delegate binding
void AFPSCharacter::OnHealthAcquired(AActor* HealthSource)
{
	UE_LOG(LogTemp, Warning, TEXT("( %s ) acquired health from ( %s )"), *this->GetName(), *HealthSource->GetName());
}

// HealthComponent Delegate binding
void AFPSCharacter::OnDeath(AActor* DeathSource)
{	
	if (GetController() != nullptr && UKismetSystemLibrary::DoesImplementInterface(GetController(), UFPSPlayerControllerInterface::StaticClass()))
	{
		UE_LOG(LogTemp, Warning, TEXT("( %s ) is killed by ( %s )"), *this->GetName(), *DeathSource->GetName());
		CollisionHandleOnDeath();
		IFPSPlayerControllerInterface::Execute_OnUpdateHealthArmorUI(GetController(), IsDead());
		IFPSPlayerControllerInterface::Execute_OnPlayerDeath(GetController());
	}
}

// HealthComponent Delegate binding
void AFPSCharacter::OnUpdateHealthArmorUI()
{
	if (GetController() != nullptr && UKismetSystemLibrary::DoesImplementInterface(GetController(), UFPSPlayerControllerInterface::StaticClass()))
	{
		IFPSPlayerControllerInterface::Execute_OnUpdateHealthArmorUI(GetController(), IsDead());
	}
}

bool AFPSCharacter::IsDead()
{
	if (HealthComponent != nullptr)
	{
		return HealthComponent->IsDead();
	}
	return false;
}

void AFPSCharacter::RespawnPlayer()
{
	CollisionHandleOnRespawn();

	if (HasAuthority())
	{
		if (HealthComponent != nullptr)
		{
			HealthComponent->Reset();
		}

		if (GetController() != nullptr && UKismetSystemLibrary::DoesImplementInterface(GetController(), UFPSPlayerControllerInterface::StaticClass()))
		{
			IFPSPlayerControllerInterface::Execute_RespawnPlayer(GetController());
		}
	}
}

void AFPSCharacter::CollisionHandleOnDeath()
{
	CharacterCapsuleComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FPSCharacterMesh->SetCollisionProfileName(TEXT("Ragdoll"));
	FPSCharacterMesh->SetSimulatePhysics(true);

	CameraContainer->SetCollisionProfileName(TEXT("IgnoreCharacter"));
	CameraContainer->SetSimulatePhysics(true);
}

void AFPSCharacter::CollisionHandleOnRespawn()
{
	CharacterCapsuleComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	FPSCharacterMesh->SetSimulatePhysics(false);
	FPSCharacterMesh->SetCollisionProfileName(TEXT("CharacterMesh"));
	FPSCharacterMesh->AttachToComponent(CharacterCapsuleComponent, FAttachmentTransformRules::SnapToTargetIncludingScale);
	FPSCharacterMesh->SetRelativeTransform(DefaultCharacterMeshRelativeTransform);

	CameraContainer->SetSimulatePhysics(false);
	CameraContainer->SetCollisionProfileName(TEXT("NoCollision"));
	CameraContainer->AttachToComponent(CharacterCapsuleComponent, FAttachmentTransformRules::SnapToTargetIncludingScale);
	CameraContainer->SetRelativeLocation(DefaultCameraRelativeLocation);
}

// IFPSCharacterInterface
FTransform AFPSCharacter::GetCameraTransform_Implementation()
{
	return FollowCamera->GetComponentTransform();
}

USkeletalMeshComponent* AFPSCharacter::GetCharacterMesh_Implementation()
{
	return FPSCharacterMesh;
}

USkeletalMeshComponent* AFPSCharacter::GetArmMesh_Implementation()
{
	return FPSArmMesh;
}
