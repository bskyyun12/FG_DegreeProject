// Fill out your copyright notice in the Description page of Project Settings.


#include "FPSCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
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
	DOREPLIFETIME(AFPSCharacter, CurrentWeapons);
	DOREPLIFETIME(AFPSCharacter, StartWeapons);
	DOREPLIFETIME(AFPSCharacter, CurrentlyHeldWeapon);
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

	DeathCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("DeathCamera"));
	DeathCamera->SetupAttachment(RootComponent);
	DeathCamera->SetRelativeRotation(FRotator(-50.f, 0.f, 0.f));
	DeathCamera->SetActive(false);
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

	PlayerInputComponent->BindAction("MainWeapon", IE_Pressed, this, &AFPSCharacter::SwitchToMainWeapon);
	PlayerInputComponent->BindAction("SubWeapon", IE_Pressed, this, &AFPSCharacter::SwitchToSubWeapon);

	PlayerInputComponent->BindAction("Drop", IE_Pressed, this, &AFPSCharacter::Drop);

	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &AFPSCharacter::Reload);

	PlayerInputComponent->BindAction("Chat", IE_Pressed, this, &AFPSCharacter::StartChat);

	PlayerInputComponent->BindAction("Escape", IE_Pressed, this, &AFPSCharacter::OnEscapeButtonPressed);

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

// Bound to input "Left Mouse Down"
void AFPSCharacter::OnBeginFire()
{
	if (CurrentlyHeldWeapon != nullptr)
	{
		Server_OnBeginFire(CurrentlyHeldWeapon);
		CurrentlyHeldWeapon->Client_OnBeginFireWeapon();
	}
}

void AFPSCharacter::Server_OnBeginFire_Implementation(AFPSWeaponBase* Weapon)
{
	if (Weapon != nullptr)
	{
		Weapon->Server_OnBeginFireWeapon();
	}
}

// Bound to input "Left Mouse Up"
void AFPSCharacter::OnEndFire()
{
	if (CurrentlyHeldWeapon != nullptr)
	{
		Server_OnEndFire(CurrentlyHeldWeapon);
		CurrentlyHeldWeapon->Client_OnEndFireWeapon();
	}
}

void AFPSCharacter::Server_OnEndFire_Implementation(AFPSWeaponBase* Weapon)
{
	if (Weapon != nullptr)
	{
		Weapon->Server_OnEndFireWeapon();
	}
}

// Bound to input "1"
void AFPSCharacter::SwitchToMainWeapon()
{
	OnEndFire();
	if (CurrentWeapons[1] != nullptr)
	{
		PlayEquipAnim(CurrentWeapons[1]);
		Server_EquipWeapon(CurrentWeapons[1]);
	}
}

// Bound to input "2"
void AFPSCharacter::SwitchToSubWeapon()
{

	OnEndFire();
	if (CurrentWeapons[2] != nullptr)
	{
		PlayEquipAnim(CurrentWeapons[2]);
		Server_EquipWeapon(CurrentWeapons[2]);
	}
}

void AFPSCharacter::PickupWeapon(AFPSWeaponBase* const& WeaponToPickup)
{
	if (WeaponToPickup != nullptr)
	{
		bool bHasSameTypeWeapon = CurrentWeapons[(uint8)WeaponToPickup->GetWeaponInfo().WeaponType] != nullptr;
		if (bHasSameTypeWeapon)
		{
			UE_LOG(LogTemp, Warning, TEXT("Already has the same weapon type"));
		}
		else
		{
			if (CurrentlyHeldWeapon == nullptr)
			{
				OnEndFire();
				PlayEquipAnim(WeaponToPickup);
				Server_EquipWeapon(WeaponToPickup);
			}
			else
			{
				CurrentWeapons[(uint8)WeaponToPickup->GetWeaponInfo().WeaponType] = WeaponToPickup;
				if (UKismetSystemLibrary::DoesImplementInterface(WeaponToPickup, UFPSWeaponInterface::StaticClass()))
				{
					IFPSWeaponInterface::Execute_ToggleVisibility(WeaponToPickup, false);
				}
			}
		}
	}
}

void AFPSCharacter::PlayEquipAnim(AFPSWeaponBase* const& WeaponToEquip)
{
	// Play Equip animation
	if (WeaponToEquip != nullptr)
	{
		FWeaponInfo WeaponInfo = WeaponToEquip->GetWeaponInfo();
		UAnimInstance* AnimInstance = FPSArmMesh->GetAnimInstance();
		if (AnimInstance != nullptr)
		{
			if (WeaponInfo.FP_EquipAnim != nullptr)
			{
				AnimInstance->Montage_Play(WeaponInfo.FP_EquipAnim);
			}
		}
	}
}

void AFPSCharacter::Server_EquipWeapon_Implementation(AFPSWeaponBase* const& WeaponToEquip)
{
	if (WeaponToEquip != nullptr)
	{
		CurrentlyHeldWeapon = WeaponToEquip;
		uint8 CurrentHoldingWeaponIndex = (uint8)CurrentlyHeldWeapon->GetWeaponInfo().WeaponType;
		CurrentWeapons[CurrentHoldingWeaponIndex] = WeaponToEquip;

		WeaponToEquip->Server_OnWeaponEquipped(this);

		for (int i = 0; i < CurrentWeapons.Num(); i++)
		{
			if (CurrentWeapons[i] != nullptr && UKismetSystemLibrary::DoesImplementInterface(CurrentWeapons[i], UFPSWeaponInterface::StaticClass()))
			{
				IFPSWeaponInterface::Execute_ToggleVisibility(CurrentWeapons[i], CurrentWeapons[i] == CurrentlyHeldWeapon);
			}
		}
	}
}

// Bound to input "F"
void AFPSCharacter::Drop()
{
	if (CurrentlyHeldWeapon != nullptr)
	{
		GetCapsuleComponent()->SetGenerateOverlapEvents(false);
		UWorld* World = GetWorld();
		if (!ensure(World != nullptr))
		{
			return;
		}
		FTimerHandle DropTimer;
		World->GetTimerManager().SetTimer(DropTimer, [&]()
			{
				GetCapsuleComponent()->SetGenerateOverlapEvents(true);
			}, 2.f, false);

		Server_DropWeapon();
	}
}

void AFPSCharacter::Server_DropWeapon_Implementation()
{
	if (CurrentlyHeldWeapon != nullptr)
	{
		CurrentlyHeldWeapon->Server_OnWeaponDroped();

		uint8 CurrentHoldingWeaponIndex = (uint8)CurrentlyHeldWeapon->GetWeaponInfo().WeaponType;
		CurrentWeapons[CurrentHoldingWeaponIndex] = nullptr;
		CurrentlyHeldWeapon = nullptr;
	}
}

// Bound to input "R"
void AFPSCharacter::Reload()
{
	if (CurrentlyHeldWeapon != nullptr && CurrentlyHeldWeapon->CanReload())
	{
		Server_Reload(CurrentlyHeldWeapon);
		CurrentlyHeldWeapon->Client_OnReload();

		// Play ArmsReloadAnim
		UAnimInstance* AnimInstance = FPSArmMesh->GetAnimInstance();
		if (AnimInstance != nullptr)
		{
			FWeaponInfo WeaponInfo = CurrentlyHeldWeapon->GetWeaponInfo();
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

// Bound to input "Tab"
void AFPSCharacter::ToggleScoreBoardWidget(bool bDisplay)
{
	if (GetController() != nullptr && UKismetSystemLibrary::DoesImplementInterface(GetController(), UFPSPlayerControllerInterface::StaticClass()))
	{
		IFPSPlayerControllerInterface::Execute_ToggleScoreBoardWidget(GetController(), bDisplay);
	}
}

// Bound to input "Left Ctrl"
void AFPSCharacter::HandleCrouch(bool bCrouchButtonDown)
{
	if (IsLocallyControlled())
	{
		UWorld* World = GetWorld();
		if (!ensure(World != nullptr))
		{
			return;
		}
		DesiredCameraRelativeLocation = bCrouchButtonDown ? CameraRelativeLocationOnCrouch : CameraRelativeLocation_Default;
		World->GetTimerManager().ClearTimer(CrouchTimerHandle);
		World->GetTimerManager().SetTimer(CrouchTimerHandle, this, &AFPSCharacter::CrouchSimulate, World->GetDeltaSeconds(), true);

		GetCharacterMovement()->MaxWalkSpeed *= bCrouchButtonDown ? 0.5f : 2.0f;
	}

	Server_HandleCrouch(bCrouchButtonDown);
}

void AFPSCharacter::Server_HandleCrouch_Implementation(bool bCrouchButtonDown)
{
	Multicast_HandleCrouch(bCrouchButtonDown);

	FollowCamera->SetRelativeLocation(bCrouchButtonDown ? CameraRelativeLocationOnCrouch : CameraRelativeLocation_Default);
	GetCharacterMovement()->MaxWalkSpeed *= bCrouchButtonDown ? 0.5f : 2.0f;
}

void AFPSCharacter::Multicast_HandleCrouch_Implementation(bool bCrouchButtonDown)
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
			if (AnimInstance != nullptr && UKismetSystemLibrary::DoesImplementInterface(AnimInstance, UFPSAnimInterface::StaticClass()))
			{
				IFPSAnimInterface::Execute_HandleCrouch(AnimInstance, bCrouchButtonDown);
			}
		}
	}
}

void AFPSCharacter::CrouchSimulate()
{
	if (FollowCamera != nullptr)
	{
		UWorld* World = GetWorld();
		if (!ensure(World != nullptr))
		{
			return;
		}
		FVector NewCameraRelativeLocation = FMath::VInterpTo(FollowCamera->GetRelativeLocation(), DesiredCameraRelativeLocation, World->GetDeltaSeconds(), 10.f);
		FollowCamera->SetRelativeLocation(NewCameraRelativeLocation);

		if (NewCameraRelativeLocation.Equals(DesiredCameraRelativeLocation, 2.f))
		{
			FollowCamera->SetRelativeLocation(DesiredCameraRelativeLocation);
			World->GetTimerManager().ClearTimer(CrouchTimerHandle);
			UE_LOG(LogTemp, Warning, TEXT("AFPSCharacter::CrouchSimulate (Timer Cleared)"));
		}
	}
}

void AFPSCharacter::OnEscapeButtonPressed()
{
	UE_LOG(LogTemp, Warning, TEXT("AFPSCharacter::OnEscapeButtonPressed"));
}

void AFPSCharacter::StartChat()
{
	if (GetController() != nullptr && UKismetSystemLibrary::DoesImplementInterface(GetController(), UFPSPlayerControllerInterface::StaticClass()))
	{
		IFPSPlayerControllerInterface::Execute_StartChat(GetController());
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
// Called by AFPSPlayerController::OnSpawnPlayer (From Server)
void AFPSCharacter::OnSpawnPlayer_Implementation()
{
	if (HealthComponent != nullptr)
	{
		HealthComponent->Server_OnSpawn();
	}

	Server_WeaponSetupOnSpawn();
	GetCapsuleComponent()->SetCollisionProfileName(TEXT("Capsule_Alive"));
	GetMesh()->SetCollisionProfileName(TEXT("CharacterMesh_Alive"));
}

// Bound to HealthComponent->OnSpawn, This gets called both server and clients
void AFPSCharacter::OnSpawn()
{
	if (IsLocallyControlled())
	{
		HandleCameraOnSpawn();
	}
}

void AFPSCharacter::HandleCameraOnSpawn()
{
	FollowCamera->SetActive(true);
	DeathCamera->SetActive(false);
}

void AFPSCharacter::Server_WeaponSetupOnSpawn_Implementation()
{
	if (CurrentWeapons.Num() != (uint8)EWeaponType::EnumSize)
	{
		for (uint8 i = 0; i < (uint8)EWeaponType::EnumSize; i++)
		{
			CurrentWeapons.Add(nullptr);
			StartWeapons.Add(nullptr);
		}
	}

	if (FPSGameInstance == nullptr)
	{
		FPSGameInstance = GetGameInstance<UFPSGameInstance>();
		if (!ensure(FPSGameInstance != nullptr))
		{
			return;
		}
	}

	// MainWeapon Setup
	if (StartWeapons[1] == nullptr)
	{
		if (FPSGameInstance->GetUserData().MainWeaponType == EMainWeapon::Rifle || FPSGameInstance->GetUserData().MainWeaponType == EMainWeapon::None)
		{
			if (!ensure(RifleClass != nullptr))
			{
				return;
			}
			UWorld* World = GetWorld();
			if (!ensure(World != nullptr))
			{
				return;
			}
			StartWeapons[1] = World->SpawnActor<AFPSWeaponBase>(RifleClass);
		}
	}

	// SubWeapon Setup
	if (StartWeapons[2] == nullptr)
	{
		if (FPSGameInstance->GetUserData().SubWeaponType == ESubWeapon::Pistol || FPSGameInstance->GetUserData().SubWeaponType == ESubWeapon::None)
		{
			if (!ensure(PistolClass != nullptr))
			{
				return;
			}
			UWorld* World = GetWorld();
			if (!ensure(World != nullptr))
			{
				return;
			}
			StartWeapons[2] = World->SpawnActor<AFPSWeaponBase>(PistolClass);
		}
	}

	CurrentWeapons = StartWeapons;

	for (AFPSWeaponBase* Weapon : CurrentWeapons)
	{
		if (Weapon != nullptr)
		{
			Weapon->OnReset();
		}
	}

	SwitchToMainWeapon();
}

// Called by DamageCauser ex) AFPSGunBase::Server_Fire
void AFPSCharacter::TakeDamage_Implementation(AActor* DamageCauser, float DamageOnHealth, float DamageOnArmor, FVector const& HitPoint)
{
	if (HealthComponent != nullptr)
	{
		HealthComponent->Server_TakeDamage(DamageCauser, DamageOnHealth, DamageOnArmor);
		IFPSPlayerControllerInterface::Execute_OnTakeDamage(GetController());
		Muticast_OnTakeDamage(HitPoint, HitEmitterOnTakeDamage);
	}
}

void AFPSCharacter::Muticast_OnTakeDamage_Implementation(FVector const& HitPoint, UParticleSystem* HitEmitter)
{
	UWorld* World = GetWorld();
	if (!ensure(World != nullptr))
	{
		return;
	}
	if (HitEmitter != nullptr)
	{
		UGameplayStatics::SpawnEmitterAtLocation(World, HitEmitter, HitPoint);
	}
}

// TODO: Do I really need this??
// Bound to HealthComponent->OnTakeDamage
void AFPSCharacter::OnTakeDamage(AActor* DamageSource)
{
	if (GetController() != nullptr && UKismetSystemLibrary::DoesImplementInterface(GetController(), UFPSPlayerControllerInterface::StaticClass()))
	{
		UE_LOG(LogTemp, Warning, TEXT("AFPSCharacter::OnTakeDamage"));
		UE_LOG(LogTemp, Warning, TEXT("( %s ) is attacked by ( %s )"), *this->GetName(), *DamageSource->GetName());
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
	Drop();

	if (HasAuthority())
	{
		if (GetController() != nullptr && UKismetSystemLibrary::DoesImplementInterface(GetController(), UFPSPlayerControllerInterface::StaticClass()))
		{
			UE_LOG(LogTemp, Warning, TEXT("AFPSCharacter::OnDeath"));
			UE_LOG(LogTemp, Warning, TEXT("( %s ) is killed by ( %s )"), *this->GetName(), *DeathSource->GetName());

			Multicast_OnDeath();
			IFPSPlayerControllerInterface::Execute_OnPlayerDeath(GetController());
		}
	}

	if (IsLocallyControlled())
	{
		HandleCameraOnDeath();
	}
}

void AFPSCharacter::Multicast_OnDeath_Implementation()
{
	GetCapsuleComponent()->SetCollisionProfileName(TEXT("Capsule_Dead"));
	GetMesh()->SetCollisionProfileName(TEXT("CharacterMesh_Dead"));

	if (!IsLocallyControlled())
	{
		if (GetMesh() != nullptr)
		{
			UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
			if (AnimInstance != nullptr && UKismetSystemLibrary::DoesImplementInterface(AnimInstance, UFPSAnimInterface::StaticClass()))
			{
				IFPSAnimInterface::Execute_OnDeath(AnimInstance);
			}
		}
	}
}

void AFPSCharacter::HandleCameraOnDeath()
{
	FollowCamera->SetActive(false);
	DeathCamera->SetActive(true);

	UWorld* World = GetWorld();
	if (World != nullptr)
	{
		FHitResult Hit;
		FVector Start = GetActorLocation();
		FVector End = Start + DeathCamera->GetForwardVector() * -500.f;

		if (World->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility))
		{
			DeathCamera->SetWorldLocation(Hit.ImpactPoint);
		}
		else
		{
			DeathCamera->SetWorldLocation(End);
		}
	}
}

#pragma endregion Spawn & Death

void AFPSCharacter::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	//TODO: Work on collision preset so that the log below will only show on weapon overlap
	UE_LOG(LogTemp, Warning, TEXT("OtherActor: %s"), *OtherActor->GetName());
	if (OtherActor != nullptr && UKismetSystemLibrary::DoesImplementInterface(OtherActor, UFPSWeaponInterface::StaticClass()))
	{
		AFPSWeaponBase* Weapon = IFPSWeaponInterface::Execute_GetWeapon(OtherActor);
		this->PickupWeapon(Weapon);
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