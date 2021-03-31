// Fill out your copyright notice in the Description page of Project Settings.


#include "FPSCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"

#include "Animation/AnimInstance.h"
#include "AnimInstances/FPSAnimInterface.h"
#include "Components/HealthComponent.h"
#include "FPSPlayerControllerInterface.h"
#include "FPSPlayerController.h"
#include "Weapons/FPSWeaponInterface.h"
#include "Weapons/FPSWeaponBase.h"

void AFPSCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AFPSCharacter, CurrentWeapon);
	DOREPLIFETIME(AFPSCharacter, MainWeapon);
	DOREPLIFETIME(AFPSCharacter, SubWeapon);
}

AFPSCharacter::AFPSCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	FollowCamera->SetupAttachment(RootComponent);

	FPSArmMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FPS_Arms"));
	FPSArmMesh->SetupAttachment(FollowCamera);

	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	HealthComponent->OnTakeDamage.AddDynamic(this, &AFPSCharacter::OnTakeDamage);
	HealthComponent->OnHealthAcquired.AddDynamic(this, &AFPSCharacter::OnHealthAcquired);
	HealthComponent->OnDeath.AddDynamic(this, &AFPSCharacter::OnDeath);
	HealthComponent->OnUpdateHealthArmorUI.AddDynamic(this, &AFPSCharacter::OnUpdateHealthArmorUI);
	HealthComponent->OnSpawn.AddDynamic(this, &AFPSCharacter::OnSpawn);

	GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &AFPSCharacter::OnBeginOverlap);
}

void AFPSCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (FollowCamera != nullptr)
	{
		CameraRelativeLocation_Default = FollowCamera->GetRelativeLocation();
	}
}

#pragma region Input bindings
void AFPSCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AFPSCharacter::OnBeginFire);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &AFPSCharacter::OnEndFire);

	PlayerInputComponent->BindAction("MainWeapon", IE_Pressed, this, &AFPSCharacter::EquipMainWeapon);
	PlayerInputComponent->BindAction("SubWeapon", IE_Pressed, this, &AFPSCharacter::EquipSubWeapon);

	PlayerInputComponent->BindAction("Drop", IE_Pressed, this, &AFPSCharacter::Drop);

	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &AFPSCharacter::Reload);

	PlayerInputComponent->BindAction<FOneBooleanDelegate>("ScoreBoard", IE_Pressed, this, &AFPSCharacter::ToggleScoreBoardWidget, true);
	PlayerInputComponent->BindAction<FOneBooleanDelegate>("ScoreBoard", IE_Released, this, &AFPSCharacter::ToggleScoreBoardWidget, false);

	PlayerInputComponent->BindAction<FOneBooleanDelegate>("Crouch", IE_Pressed, this, &AFPSCharacter::HandleCrouch, true);
	PlayerInputComponent->BindAction<FOneBooleanDelegate>("Crouch", IE_Released, this, &AFPSCharacter::HandleCrouch, false);

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

		if (GetMesh() != nullptr)
		{
			UAnimInstance* FPSAnimInstance = GetMesh()->GetAnimInstance();
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

void AFPSCharacter::EquipMainWeapon()
{
	UE_LOG(LogTemp, Warning, TEXT("EquipMainWeapon"));

	// Play Equip animation
	UAnimInstance* AnimInstance = FPSArmMesh->GetAnimInstance();
	if (AnimInstance != nullptr)
	{
		FWeaponInfo WeaponInfo = MainWeapon->GetWeaponInfo();
		if (WeaponInfo.FP_EquipAnim != nullptr)
		{
			AnimInstance->Montage_Play(WeaponInfo.FP_EquipAnim);
		}
	}

	Server_EquipWeapon(MainWeapon);
}

void AFPSCharacter::EquipSubWeapon()
{
	UE_LOG(LogTemp, Warning, TEXT("EquipSubWeapon"));

	// Play Equip animation
	UAnimInstance* AnimInstance = FPSArmMesh->GetAnimInstance();
	if (AnimInstance != nullptr)
	{
		FWeaponInfo WeaponInfo = SubWeapon->GetWeaponInfo();
		if (WeaponInfo.FP_EquipAnim != nullptr)
		{
			AnimInstance->Montage_Play(WeaponInfo.FP_EquipAnim);
		}
	}

	Server_EquipWeapon(SubWeapon);
}

void AFPSCharacter::Server_EquipWeapon_Implementation(AFPSWeaponBase* Weapon)
{
	if (Weapon != nullptr)
	{
		EWeaponType WeaponType = Weapon->GetWeaponInfo().WeaponType;

		if (MainWeapon != nullptr)
		{
			MainWeapon->GetRootComponent()->SetVisibility(WeaponType == EWeaponType::MainWeapon, true);
			MainWeapon->SetActorHiddenInGame(WeaponType != EWeaponType::MainWeapon);
		}
		if (SubWeapon != nullptr)
		{
			SubWeapon->GetRootComponent()->SetVisibility(WeaponType == EWeaponType::SubWeapon, true);
			SubWeapon->SetActorHiddenInGame(WeaponType != EWeaponType::SubWeapon);
		}

		CurrentWeapon = Weapon;
		CurrentWeapon->Server_OnWeaponEquipped(this);
	}
}

void AFPSCharacter::Drop()
{
	UE_LOG(LogTemp, Warning, TEXT("AFPSCharacter::Drop"));
	if (CurrentWeapon != nullptr)
	{
		Server_DropWeapon(CurrentWeapon);
		CurrentWeapon = nullptr;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("(Drop) CurrentWeapon == nulllptr"))
	}
}

void AFPSCharacter::Server_DropWeapon_Implementation(AFPSWeaponBase* Weapon)
{
	UE_LOG(LogTemp, Warning, TEXT("AFPSCharacter::Server_DropWeapon_Implementation"));
	if (Weapon != nullptr)
	{
		Weapon->Server_OnWeaponDroped();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("(Server_DropWeapon) MainWeapon == nulllptr"));
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

		Multicast_PlayReloadAnim(Weapon);
	}
}

void AFPSCharacter::Multicast_PlayReloadAnim_Implementation(AFPSWeaponBase* Weapon)
{
	UWorld* World = GetWorld();
	if (!ensure(World != nullptr))
	{
		return;
	}
	if (!IsLocallyControlled())
	{
		if (GetMesh() != nullptr)
		{
			UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
			if (AnimInstance != nullptr)
			{
				FWeaponInfo WeaponInfo = Weapon->GetWeaponInfo();
				if (WeaponInfo.TP_ReloadAnim != nullptr)
				{
					AnimInstance->Montage_Play(WeaponInfo.TP_ReloadAnim);
				}
			}
		}
	}
}

void AFPSCharacter::ToggleScoreBoardWidget(bool bDisplay)
{
	if (GetController() != nullptr && UKismetSystemLibrary::DoesImplementInterface(GetController(), UFPSPlayerControllerInterface::StaticClass()))
	{
		IFPSPlayerControllerInterface::Execute_ToggleScoreBoardWidget(GetController(), bDisplay);
	}
}

void AFPSCharacter::HandleCrouch(bool bCrouchButtonDown)
{
	Server_HandleCrouch(bCrouchButtonDown);
}

void AFPSCharacter::Server_HandleCrouch_Implementation(bool bCrouchButtonDown)
{
	Multicast_HandleCrouch(bCrouchButtonDown);
}

void AFPSCharacter::Multicast_HandleCrouch_Implementation(bool bCrouchButtonDown)
{
	UE_LOG(LogTemp, Warning, TEXT("AFPSCharacter::Multicast_HandleCrouch_Implementation"));
	UWorld* World = GetWorld();
	if (!ensure(World != nullptr))
	{
		return;
	}

	if (!IsLocallyControlled())
	{
		if (GetMesh() != nullptr)
		{
			if (GetMesh()->GetAnimInstance() != nullptr && UKismetSystemLibrary::DoesImplementInterface(GetMesh()->GetAnimInstance(), UFPSAnimInterface::StaticClass()))
			{
				IFPSAnimInterface::Execute_HandleCrouch(GetMesh()->GetAnimInstance(), bCrouchButtonDown);
			}
		}
	}
	else
	{
		DesiredCameraRelativeLocation = bCrouchButtonDown ? CameraRelativeLocationOnCrouch : CameraRelativeLocation_Default;
		World->GetTimerManager().ClearTimer(CrouchTimerHandle);
		World->GetTimerManager().SetTimer(CrouchTimerHandle, this, &AFPSCharacter::CrouchTimer, World->GetDeltaSeconds(), true);
	}
}

void AFPSCharacter::CrouchTimer()
{
	if (FollowCamera != nullptr)
	{
		FVector NewCameraRelativeLocation = FMath::LerpStable(FollowCamera->GetRelativeLocation(), DesiredCameraRelativeLocation, 0.05f);
		FollowCamera->SetRelativeLocation(NewCameraRelativeLocation);

		if (NewCameraRelativeLocation.Equals(DesiredCameraRelativeLocation, 2.f))
		{
			FollowCamera->SetRelativeLocation(DesiredCameraRelativeLocation);
			UWorld* World = GetWorld();
			if (!ensure(World != nullptr))
			{
				return;
			}
			World->GetTimerManager().ClearTimer(CrouchTimerHandle);
		}
	}
}

#pragma endregion Input bindings

#pragma region Getters
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

FTransform AFPSCharacter::GetCameraTransform_Implementation()
{
	return FollowCamera->GetComponentTransform();
}

USkeletalMeshComponent* AFPSCharacter::GetCharacterMesh_Implementation()
{
	return GetMesh();
}

USkeletalMeshComponent* AFPSCharacter::GetArmMesh_Implementation()
{
	return FPSArmMesh;
}
#pragma endregion Getters

#pragma region Spawn & Death
// Called by AFPSPlayerController::OnSpawnPlayer
void AFPSCharacter::OnSpawnPlayer_Implementation()
{
	if (HealthComponent != nullptr)
	{
		HealthComponent->Server_OnSpawn();
	}

	Server_WeaponSetupOnSpawn();
}

// Bound to HealthComponent->OnSpawn, This gets called both server and clients
void AFPSCharacter::OnSpawn()
{
	UE_LOG(LogTemp, Warning, TEXT("AFPSCharacter::OnSpawn"));
	HandleCollisionOnSpawn();
}

void AFPSCharacter::Server_WeaponSetupOnSpawn_Implementation()
{
	if (FPSGameInstance == nullptr)
	{
		FPSGameInstance = GetGameInstance<UFPSGameInstance>();
		if (!ensure(FPSGameInstance != nullptr))
		{
			return;
		}
	}

	// MainWeapon Setup
	if (MainWeapon == nullptr)
	{
		if (FPSGameInstance->UserData.MainWeapon == EMainWeapon::Rifle || FPSGameInstance->UserData.MainWeapon == EMainWeapon::None)
		{
			if (!ensure(RifleClass != nullptr))
			{
				return;
			}
			AFPSWeaponBase* SpawnedWeapon = GetWorld()->SpawnActor<AFPSWeaponBase>(RifleClass);
			MainWeapon = SpawnedWeapon;
			MainWeapon->GetRootComponent()->SetVisibility(false, true);
		}
	}
	else if (MainWeapon != nullptr && CurrentWeapon == nullptr)
	{
		// TODO: Reset ammo of the MainWeapon
	}
	else if (MainWeapon != CurrentWeapon)
	{
		// TODO: Drop current and equip main? or maybe it will be taken by it's original owner?
	}

	// SubWeapon Setup
	if (SubWeapon == nullptr)
	{
		if (FPSGameInstance->UserData.SubWeapon == ESubWeapon::Pistol || FPSGameInstance->UserData.SubWeapon == ESubWeapon::None)
		{
			if (!ensure(PistolClass != nullptr))
			{
				return;
			}
			AFPSWeaponBase* SpawnedWeapon = GetWorld()->SpawnActor<AFPSWeaponBase>(PistolClass);
			SubWeapon = SpawnedWeapon;
			SubWeapon->GetRootComponent()->SetVisibility(false, true);
		}
	}
	else if (SubWeapon != nullptr && CurrentWeapon == nullptr)
	{
		// TODO: Reset ammo of the MainWeapon
	}
	else if (SubWeapon != CurrentWeapon)
	{
		// TODO: Drop current and equip main? or maybe it will be taken by it's original owner?
	}

	//EquipMainWeapon();
}

// Called by DamageCauser ex) AFPSGunBase::Server_Fire
void AFPSCharacter::TakeDamage_Implementation(AActor* DamageCauser, float DamageOnHealth, float DamageOnArmor)
{
	if (HealthComponent != nullptr)
	{
		HealthComponent->Server_TakeDamage(DamageCauser, DamageOnHealth, DamageOnArmor);
	}
}

// Bound to HealthComponent->OnTakeDamage
void AFPSCharacter::OnTakeDamage(AActor* DamageSource)
{
	if (GetController() != nullptr && UKismetSystemLibrary::DoesImplementInterface(GetController(), UFPSPlayerControllerInterface::StaticClass()))
	{
		UE_LOG(LogTemp, Warning, TEXT("( %s ) is attacked by ( %s )"), *this->GetName(), *DamageSource->GetName());
		IFPSPlayerControllerInterface::Execute_OnTakeDamage(GetController());
	}
}

// Bound to HealthComponent->OnHealthAcquired
void AFPSCharacter::OnHealthAcquired(AActor* HealthSource)
{
	UE_LOG(LogTemp, Warning, TEXT("AFPSCharacter::OnHealthAcquired"));
	UE_LOG(LogTemp, Warning, TEXT("( %s ) acquired health from ( %s )"), *this->GetName(), *HealthSource->GetName());
}

// Bound to HealthComponent->OnUpdateHealthArmorUI
void AFPSCharacter::OnUpdateHealthArmorUI()
{
	if (GetController() != nullptr && UKismetSystemLibrary::DoesImplementInterface(GetController(), UFPSPlayerControllerInterface::StaticClass()))
	{
		IFPSPlayerControllerInterface::Execute_OnUpdateHealthArmorUI(GetController(), IsDead());
	}
}

// Bound to HealthComponent->OnDeath
void AFPSCharacter::OnDeath(AActor* DeathSource)
{
	HandleCollisionOnDeath();

	if (HasAuthority())
	{
		if (GetController() != nullptr && UKismetSystemLibrary::DoesImplementInterface(GetController(), UFPSPlayerControllerInterface::StaticClass()))
		{
			UE_LOG(LogTemp, Warning, TEXT("AFPSCharacter::OnDeath"));
			UE_LOG(LogTemp, Warning, TEXT("( %s ) is killed by ( %s )"), *this->GetName(), *DeathSource->GetName());

			UWorld* World = GetWorld();
			if (!ensure(World != nullptr))
			{
				return;
			}
			FTimerHandle DeathTimer;
			World->GetTimerManager().SetTimer(DeathTimer, [&]()
				{
					IFPSPlayerControllerInterface::Execute_OnUpdateHealthArmorUI(GetController(), IsDead());
					IFPSPlayerControllerInterface::Execute_OnPlayerDeath(GetController());
				}, 2.f, false);
		}
	}
}
#pragma endregion Spawn & Death

// TODO: Pick up weapon by overlapping instead of "E" button
void AFPSCharacter::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor != nullptr && UKismetSystemLibrary::DoesImplementInterface(OtherActor, UFPSWeaponInterface::StaticClass()))
	{
		AFPSWeaponBase* Weapon = IFPSWeaponInterface::Execute_GetWeapon(OtherActor);
		UE_LOG(LogTemp, Warning, TEXT("AFPSCharacter::OnBeginOverlap: Overlapped ( %s )"), *Weapon->GetName());
	}
}

void AFPSCharacter::HandleCollisionOnSpawn()
{
	if (GetCapsuleComponent() != nullptr)
	{
		GetCapsuleComponent()->SetCollisionProfileName("Pawn");
	}
}

void AFPSCharacter::HandleCollisionOnDeath()
{
	if (GetCapsuleComponent() != nullptr)
	{
		GetCapsuleComponent()->SetCollisionProfileName("IgnoreCharacter");
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