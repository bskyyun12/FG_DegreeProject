// Copyright Epic Games, Inc. All Rights Reserved.

#include "DeathMatchCharacter.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/InputSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

#include "DeathMatchPlayerState.h"
#include "Weapons/WeaponInterface.h"
#include "Weapons/GunBase.h"
#include "PlayerControllerInterface.h"
#include "Animation/FPSAnimInterface.h"
#include "DeathMatchGameMode.h"
#include "DeathMatchPlayerController.h"

void ADeathMatchCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(ADeathMatchCharacter, BeginFire, COND_SimulatedOnly);
	DOREPLIFETIME_CONDITION(ADeathMatchCharacter, EndFire, COND_SimulatedOnly);
}

ADeathMatchCharacter::ADeathMatchCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// Create a CameraComponent	
	FP_Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FP_Camera->SetupAttachment(GetCapsuleComponent());
	FP_Camera->SetRelativeLocation(FVector(-39.56f, 1.75f, 64.f)); // Position the camera
	FP_Camera->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	ArmMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ArmsMesh"));
	ArmMesh->SetOnlyOwnerSee(true);
	ArmMesh->SetupAttachment(FP_Camera);
	ArmMesh->bCastDynamicShadow = false;
	ArmMesh->CastShadow = false;
	ArmMesh->SetRelativeRotation(FRotator(1.9f, -19.19f, 5.2f));
	ArmMesh->SetRelativeLocation(FVector(-0.5f, -4.4f, -155.7f));

	GetMesh()->SetOwnerNoSee(true);
	GetMesh()->SetCollisionProfileName(TEXT("CharacterMesh_Alive"));

	GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &ADeathMatchCharacter::OnBeginOverlap);
	GetCapsuleComponent()->SetCollisionProfileName(TEXT("Capsule_Alive"));

	DeathCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("DeathCamera"));
	DeathCamera->SetupAttachment(RootComponent);
	DeathCamera->SetRelativeRotation(FRotator(-50.f, 0.f, 0.f));
	DeathCamera->SetActive(false);
}

void ADeathMatchCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocallyControlled())
	{
		GI = GetGameInstance<UFPSGameInstance>();
		if (!ensure(GI != nullptr))
		{
			return;
		}
	}

	if (FP_Camera != nullptr)
	{
		// This is used for crouching
		CameraRelativeLocation_Default = FP_Camera->GetRelativeLocation();
	}
}

void ADeathMatchCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	check(PlayerInputComponent);

	// Bind jump events
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	// Bind fire events
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ADeathMatchCharacter::OnBeginFire);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ADeathMatchCharacter::OnEndFire);

	// Bind weapon equip events
	PlayerInputComponent->BindAction("MainWeapon", IE_Pressed, this, &ADeathMatchCharacter::EquipMainWeapon);

	PlayerInputComponent->BindAction("SubWeapon", IE_Pressed, this, &ADeathMatchCharacter::EquipSubWeapon);

	PlayerInputComponent->BindAction("Drop", IE_Pressed, this, &ADeathMatchCharacter::Drop);
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &ADeathMatchCharacter::BeginReload);
	PlayerInputComponent->BindAction("Chat", IE_Pressed, this, &ADeathMatchCharacter::StartChat);

	PlayerInputComponent->BindAction<FOneBooleanDelegate>("ScoreBoard", IE_Pressed, this, &ADeathMatchCharacter::ToggleScoreBoardWidget, true);
	PlayerInputComponent->BindAction<FOneBooleanDelegate>("ScoreBoard", IE_Released, this, &ADeathMatchCharacter::ToggleScoreBoardWidget, false);

	PlayerInputComponent->BindAction<FOneBooleanDelegate>("Crouch", IE_Pressed, this, &ADeathMatchCharacter::HandleCrouch, true);
	PlayerInputComponent->BindAction<FOneBooleanDelegate>("Crouch", IE_Released, this, &ADeathMatchCharacter::HandleCrouch, false);

	// Bind movement events
	PlayerInputComponent->BindAxis("MoveForward", this, &ADeathMatchCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ADeathMatchCharacter::MoveRight);

	// "turn" handles devices that provide an absolute delta, such as a mouse.
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &ADeathMatchCharacter::LookUp);
}

#pragma region Getters & Setters
ADeathMatchPlayerState* ADeathMatchCharacter::GetPlayerState()
{
	// Getting PS value like this because when a player joins later, the player doesn't know previous players' PS values.
	if (PS == nullptr)
	{
		PS = ACharacter::GetPlayerState<ADeathMatchPlayerState>();
	}

	return PS;
}

AActor* ADeathMatchCharacter::GetCurrentWeapon()
{
	if (GetPlayerState() == nullptr)
	{
		return nullptr;
	}

	return GetPlayerState()->GetCurrentlyHeldWeapon();
}

AActor* ADeathMatchCharacter::GetCurrentMainWeapon()
{
	if (GetPlayerState() == nullptr)
	{
		return nullptr;
	}

	return GetPlayerState()->GetCurrentWeaponWithIndex(1);
}

AActor* ADeathMatchCharacter::GetCurrentSubWeapon()
{
	if (GetPlayerState() == nullptr)
	{
		return nullptr;
	}

	return GetPlayerState()->GetCurrentWeaponWithIndex(2);
}

bool ADeathMatchCharacter::IsDead()
{
	if (GetPlayerState() == nullptr)
	{
		return false;
	}

	return GetPlayerState()->GetIsDead();
}

FVector ADeathMatchCharacter::GetCameraLocation() const
{
	return FP_Camera->GetComponentLocation();
}

FVector ADeathMatchCharacter::GetCameraForward() const
{
	return FP_Camera->GetForwardVector();
}

void ADeathMatchCharacter::SetCurrentlyHeldWeapon(AActor* NewWeapon)
{
	if (GetPlayerState() == nullptr)
	{
		return;
	}

	GetPlayerState()->SetCurrentlyHeldWeapon(NewWeapon);
}

void ADeathMatchCharacter::SetCameraWorldRotation(const FRotator& Rotation)
{
	FP_Camera->SetWorldRotation(Rotation);
}

#pragma endregion Getters & Setters

#pragma region Weapon Pickup & Equip & Drop
void ADeathMatchCharacter::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (GetPlayerState() != nullptr)
	{
		if (OtherActor != nullptr && UKismetSystemLibrary::DoesImplementInterface(OtherActor, UWeaponInterface::StaticClass()))
		{
			UE_LOG(LogTemp, Warning, TEXT("ADeathMatchCharacter::OnBeginOverlap => OtherActor: ( %s )."), *OtherActor->GetName());

			if (GetLocalRole() == ROLE_Authority)
			{
				PickupWeapon(OtherActor);
				Multicast_PickupWeapon(OtherActor);
			}

			if (GetLocalRole() == ROLE_AutonomousProxy)
			{
				PickupWeapon(OtherActor);
				Server_PickupWeapon(OtherActor);
			}
		}
	}
}

void ADeathMatchCharacter::Server_PickupWeapon_Implementation(AActor* Weapon)
{
	Multicast_PickupWeapon(Weapon);
}

void ADeathMatchCharacter::Multicast_PickupWeapon_Implementation(AActor* Weapon)
{
	if (!IsLocallyControlled())
	{
		PickupWeapon(Weapon);
	}
}

void ADeathMatchCharacter::PickupWeapon(AActor* WeaponToPickup)
{
	if (GetPlayerState() != nullptr && WeaponToPickup != nullptr)
	{
		EWeaponType WeaponType = IWeaponInterface::Execute_GetWeaponType(WeaponToPickup);
		uint8 WeaponIndex = (uint8)WeaponType;
		bool bHasSameTypeWeapon = GetPlayerState()->GetCurrentWeaponWithIndex(WeaponIndex) != nullptr;
		if (bHasSameTypeWeapon)
		{
			UE_LOG(LogTemp, Warning, TEXT("( %s ) already has the same weapon type"), *GetName());
		}
		else
		{
			IWeaponInterface::Execute_SetVisibility(WeaponToPickup, false);
			GetPlayerState()->SetCurrentWeaponWithIndex(WeaponIndex, WeaponToPickup);

			if (GetCurrentWeapon() == nullptr)
			{
				EquipWeapon(WeaponToPickup);
			}
		}
	}
}

void ADeathMatchCharacter::EquipMainWeapon()
{
	if (GetCurrentMainWeapon() != nullptr)
	{
		if (GetCurrentWeapon() == GetCurrentMainWeapon())
		{
			return;
		}

		EquipWeapon(GetCurrentMainWeapon());

		if (GetLocalRole() == ROLE_Authority)
		{
			Multicast_EquipWeapon(GetCurrentMainWeapon());
		}

		if (GetLocalRole() == ROLE_AutonomousProxy)
		{
			Server_EquipWeapon(GetCurrentMainWeapon());
		}
	}
}

void ADeathMatchCharacter::EquipSubWeapon()
{
	if (GetCurrentSubWeapon() != nullptr)
	{
		if (GetCurrentWeapon() == GetCurrentSubWeapon())
		{
			return;
		}

		EquipWeapon(GetCurrentSubWeapon());

		if (GetLocalRole() == ROLE_Authority)
		{
			Multicast_EquipWeapon(GetCurrentSubWeapon());
		}

		if (GetLocalRole() == ROLE_AutonomousProxy)
		{
			Server_EquipWeapon(GetCurrentSubWeapon());
		}
	}
}

void ADeathMatchCharacter::Server_EquipWeapon_Implementation(AActor* WeaponToEquip)
{
	Multicast_EquipWeapon(WeaponToEquip);
}

void ADeathMatchCharacter::Multicast_EquipWeapon_Implementation(AActor* WeaponToEquip)
{
	if (!IsLocallyControlled())
	{
		EquipWeapon(WeaponToEquip);
	}
}

void ADeathMatchCharacter::EquipWeapon(AActor* WeaponToEquip)
{
	if (GetPlayerState() != nullptr)
	{
		// Hide All weapons
		for (AActor* Weapon : GetPlayerState()->GetCurrentWeapons())
		{
			if (Weapon != nullptr)
			{
				IWeaponInterface::Execute_SetVisibility(Weapon, false);
			}
		}

		if (WeaponToEquip == nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("ADeathMatchCharacter::EquipWeapon => ( %s ) equipped ( NONE )"), *GetName());
			SetCurrentlyHeldWeapon(nullptr);
		}

		if (WeaponToEquip != nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("ADeathMatchCharacter::EquipWeapon => ( %s ) equipped ( %s )"), *GetName(), *WeaponToEquip->GetName());
			SetCurrentlyHeldWeapon(WeaponToEquip);
			IWeaponInterface::Execute_OnWeaponEquipped(WeaponToEquip, this);
			IWeaponInterface::Execute_SetVisibility(WeaponToEquip, true);
		}

		if (IsLocallyControlled())
		{
			// Notify ( ArmMesh )'s Animation that this player's weapon is changed
			UAnimInstance* ArmsAnimInstance = GetArmMesh()->GetAnimInstance();
			if (ArmsAnimInstance != nullptr && UKismetSystemLibrary::DoesImplementInterface(ArmsAnimInstance, UFPSAnimInterface::StaticClass()))
			{
				EWeaponType WeaponType = IWeaponInterface::Execute_GetWeaponType(WeaponToEquip);
				IFPSAnimInterface::Execute_OnChangeWeapon(ArmsAnimInstance, WeaponType);
			}
		}
		else
		{
			// Notify ( CharacterMesh )'s Animation that this player's weapon is changed
			UAnimInstance* CharacterAnimInstance = GetMesh()->GetAnimInstance();
			if (CharacterAnimInstance != nullptr && UKismetSystemLibrary::DoesImplementInterface(CharacterAnimInstance, UFPSAnimInterface::StaticClass()))
			{
				EWeaponType WeaponType = IWeaponInterface::Execute_GetWeaponType(WeaponToEquip);
				IFPSAnimInterface::Execute_OnChangeWeapon(CharacterAnimInstance, WeaponType);
			}
		}
	}
}

void ADeathMatchCharacter::Drop()
{
	if (GetCurrentWeapon() != nullptr)
	{
		DropWeapon();

		if (GetLocalRole() == ROLE_Authority)
		{
			UE_LOG(LogTemp, Warning, TEXT("( %i ) ( %s ) ADeathMatchCharacter::DropWeapon"), GetLocalRole(), *GetName());
			Multicast_DropWeapon();
		}

		if (GetLocalRole() == ROLE_AutonomousProxy)
		{
			UE_LOG(LogTemp, Warning, TEXT("( %i ) ( %s ) ADeathMatchCharacter::DropWeapon"), GetLocalRole(), *GetName());
			Server_DropWeapon();
		}
	}
}

void ADeathMatchCharacter::Server_DropWeapon_Implementation()
{
	Multicast_DropWeapon();
}

void ADeathMatchCharacter::Multicast_DropWeapon_Implementation()
{
	if (!IsLocallyControlled())
	{
		DropWeapon();
		if (GetCurrentWeapon() != nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("( %i ) ( %s ) ADeathMatchCharacter::DropWeapon"), GetLocalRole(), *GetName());
		}
	}
}

void ADeathMatchCharacter::DropWeapon()
{
	if (GetPlayerState() != nullptr && GetCurrentWeapon() != nullptr)
	{
		UWorld* World = GetWorld();
		if (World != nullptr)
		{
			// Disable Overlap Event for 1.5 seconds. => Prevents instant pick up after dropping
			FTimerHandle OverlapDisableTimer;
			GetCapsuleComponent()->SetGenerateOverlapEvents(false);
			World->GetTimerManager().SetTimer(OverlapDisableTimer, [&]()
				{
					GetCapsuleComponent()->SetGenerateOverlapEvents(true);
				}, 1.5f, false);
		}

		IWeaponInterface::Execute_OnWeaponDropped(GetCurrentWeapon());

		EWeaponType WeaponType = IWeaponInterface::Execute_GetWeaponType(GetCurrentWeapon());
		uint8 WeaponIndex = (uint8)WeaponType;
		GetPlayerState()->SetCurrentWeaponWithIndex(WeaponIndex, nullptr);
		SetCurrentlyHeldWeapon(nullptr);
	}
}
#pragma endregion Weapon Pickup & Equip & Drop

void ADeathMatchCharacter::BeginReload()
{
	Reload();

	if (GetLocalRole() == ROLE_Authority)
	{
		Multicast_BeginReload();
	}

	if (GetLocalRole() == ROLE_AutonomousProxy)
	{
		Server_BeginReload();
	}
}

void ADeathMatchCharacter::Server_BeginReload_Implementation()
{
	Multicast_BeginReload();
}

void ADeathMatchCharacter::Multicast_BeginReload_Implementation()
{
	if (!IsLocallyControlled())
	{
		Reload();
	}
}

void ADeathMatchCharacter::Reload()
{
	if (GetCurrentWeapon() != nullptr)
	{
		IWeaponInterface::Execute_BeginReload(GetCurrentWeapon());
	}
}

void ADeathMatchCharacter::ToggleScoreBoardWidget(bool bDisplay)
{
	if (GetController() != nullptr && UKismetSystemLibrary::DoesImplementInterface(GetController(), UPlayerControllerInterface::StaticClass()))
	{
		IPlayerControllerInterface::Execute_SetScoreBoardUIVisibility(GetController(), bDisplay);
	}
}

void ADeathMatchCharacter::HandleCrouch(bool bCrouchButtonDown)
{
	UWorld* World = GetWorld();
	if (!ensure(World != nullptr))
	{
		return;
	}
	DesiredCameraRelativeLocation = bCrouchButtonDown ? CameraRelativeLocationOnCrouch : CameraRelativeLocation_Default;
	World->GetTimerManager().SetTimer(CrouchTimerHandle, this, &ADeathMatchCharacter::CrouchSimulate, World->GetDeltaSeconds(), true);

	GetCharacterMovement()->MaxWalkSpeed *= bCrouchButtonDown ? 0.5f : 2.0f;

	Server_HandleCrouch(bCrouchButtonDown);
}

void ADeathMatchCharacter::CrouchSimulate()
{
	if (FP_Camera != nullptr)
	{
		UWorld* World = GetWorld();
		if (!ensure(World != nullptr))
		{
			return;
		}
		FVector NewCameraRelativeLocation = FMath::VInterpTo(FP_Camera->GetRelativeLocation(), DesiredCameraRelativeLocation, World->GetDeltaSeconds(), 10.f);
		FP_Camera->SetRelativeLocation(NewCameraRelativeLocation);

		if (NewCameraRelativeLocation.Equals(DesiredCameraRelativeLocation, 2.f))
		{
			FP_Camera->SetRelativeLocation(DesiredCameraRelativeLocation);
			World->GetTimerManager().ClearTimer(CrouchTimerHandle);
			UE_LOG(LogTemp, Warning, TEXT("AFPSCharacter::CrouchSimulate (Timer Cleared)"));
		}
	}
}

void ADeathMatchCharacter::Server_HandleCrouch_Implementation(bool bCrouchButtonDown)
{
	Multicast_HandleCrouch(bCrouchButtonDown);

	FP_Camera->SetRelativeLocation(bCrouchButtonDown ? CameraRelativeLocationOnCrouch : CameraRelativeLocation_Default);
}

void ADeathMatchCharacter::Multicast_HandleCrouch_Implementation(bool bCrouchButtonDown)
{
	UWorld* World = GetWorld();
	if (!ensure(World != nullptr))
	{
		return;
	}

	if (!IsLocallyControlled())
	{
		// Notify ( CharacterMesh )'s Animation that this player is crouching
		UAnimInstance* CharacterAnimInstance = GetMesh()->GetAnimInstance();
		if (CharacterAnimInstance != nullptr && UKismetSystemLibrary::DoesImplementInterface(CharacterAnimInstance, UFPSAnimInterface::StaticClass()))
		{
			IFPSAnimInterface::Execute_HandleCrouch(CharacterAnimInstance, bCrouchButtonDown);
		}

		GetCharacterMovement()->MaxWalkSpeed *= bCrouchButtonDown ? 0.5f : 2.0f;
	}
}

void ADeathMatchCharacter::StartChat()
{
	if (GetController() != nullptr && UKismetSystemLibrary::DoesImplementInterface(GetController(), UPlayerControllerInterface::StaticClass()))
	{
		IPlayerControllerInterface::Execute_StartChat(GetController());
	}
}

void ADeathMatchCharacter::LookUp(float Value)
{
	if (Value == 0.f)
	{
		return;
	}

	AddControllerPitchInput(Value);

	if (GetLocalRole() == ROLE_Authority)
	{
		Multicast_LookUp(GetControlRotation());
	}

	if (GetLocalRole() == ROLE_AutonomousProxy)
	{
		Server_LookUp(GetControlRotation());
	}
}

void ADeathMatchCharacter::Server_LookUp_Implementation(const FRotator& CameraRotation)
{
	Multicast_LookUp(CameraRotation);
}

void ADeathMatchCharacter::Multicast_LookUp_Implementation(const FRotator& CameraRotation)
{
	if (!IsLocallyControlled())
	{
		FP_Camera->SetWorldRotation(CameraRotation);

		// Notify ( CharacterMesh )'s Animation that this player's camera pitch is changed
		UAnimInstance* CharacterAnimInstance = GetMesh()->GetAnimInstance();
		if (CharacterAnimInstance != nullptr && UKismetSystemLibrary::DoesImplementInterface(CharacterAnimInstance, UFPSAnimInterface::StaticClass()))
		{
			IFPSAnimInterface::Execute_UpdateSpineAngle(CharacterAnimInstance, CameraRotation.Pitch);
		}
	}
}

void ADeathMatchCharacter::OnBeginFire()
{
	if (GetCurrentWeapon() != nullptr)
	{
		if (GetLocalRole() == ROLE_AutonomousProxy)
		{
			UE_LOG(LogTemp, Warning, TEXT("(ROLE_AutonomousProxy) OnBeginFire"));
			IWeaponInterface::Execute_BeginFire(GetCurrentWeapon());
		}
		Server_OnBeginFire();
	}
}

void ADeathMatchCharacter::Server_OnBeginFire_Implementation()
{
	if (GetCurrentWeapon() != nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("(Server) OnBeginFire"));
		BeginFire++;	// OnRep_BeginFire()
		IWeaponInterface::Execute_BeginFire(GetCurrentWeapon());
	}
}

// All Other Clients ( COND_SimulatedOnly )
void ADeathMatchCharacter::OnRep_BeginFire()
{
	if (GetCurrentWeapon() != nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("(Simulated Client) OnBeginFire"));
		IWeaponInterface::Execute_BeginFire(GetCurrentWeapon());
	}
}

void ADeathMatchCharacter::OnEndFire()
{
	if (GetCurrentWeapon() != nullptr)
	{
		if (GetLocalRole() == ROLE_AutonomousProxy)
		{
			UE_LOG(LogTemp, Warning, TEXT("(ROLE_AutonomousProxy) OnEndFire"));
			IWeaponInterface::Execute_EndFire(GetCurrentWeapon());
		}
		Server_OnEndFire();
	}
}

void ADeathMatchCharacter::Server_OnEndFire_Implementation()
{
	if (GetCurrentWeapon() != nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("(Server) OnEndFire"));
		EndFire++;
		IWeaponInterface::Execute_EndFire(GetCurrentWeapon());
	}
}

// All Other Clients ( COND_SimulatedOnly )
void ADeathMatchCharacter::OnRep_EndFire()
{
	if (GetCurrentWeapon() != nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("(Simulated Client) OnEndFire"));
		IWeaponInterface::Execute_EndFire(GetCurrentWeapon());
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

#pragma region Spawn and Death
// Called after ADeathMatchPlayerState::Server_OnSpawn
void ADeathMatchCharacter::Server_OnSpawnPlayer_Implementation()
{
	Multicast_OnSpawn();
}

void ADeathMatchCharacter::Multicast_OnSpawn_Implementation()
{
	GetCapsuleComponent()->SetCollisionProfileName(TEXT("Capsule_Alive"));
	GetMesh()->SetCollisionProfileName(TEXT("CharacterMesh_Alive"));
	GetCapsuleComponent()->SetGenerateOverlapEvents(true);

	if (IsLocallyControlled())
	{
		HandleCameraOnSpawn();
	}

	EquipWeapon(GetCurrentMainWeapon());
}

void ADeathMatchCharacter::HandleCameraOnSpawn()
{
	FP_Camera->SetActive(true);
	DeathCamera->SetActive(false);

	ArmMesh->SetOwnerNoSee(false);
	GetMesh()->SetOwnerNoSee(true);
}

void ADeathMatchCharacter::Server_TakeDamage_Implementation(const uint8& DamageOnHealth, const uint8& DamageOnArmor, AActor* DamageCauser)
{
	if (GetPlayerState() != nullptr)
	{
		uint8 CurrentHealth = GetPlayerState()->GetCurrentHealth();
		uint8 CurrentArmor = GetPlayerState()->GetCurrentArmor();

		uint8 TotalDamageOnHealth = DamageOnHealth;
		uint8 TotalDamageOnArmor = DamageOnArmor;

		// Take damage on health when incoming armor damage is greater than current armor
		if (DamageOnArmor > CurrentArmor)
		{
			TotalDamageOnHealth += DamageOnArmor - CurrentArmor;
		}

		// Make sure total damage doesn't go below 0
		TotalDamageOnHealth = (TotalDamageOnHealth > CurrentHealth) ? CurrentHealth : TotalDamageOnHealth;
		TotalDamageOnArmor = (TotalDamageOnArmor > CurrentArmor) ? CurrentArmor : TotalDamageOnArmor;

		CurrentHealth -= TotalDamageOnHealth;
		CurrentArmor -= TotalDamageOnArmor;

		GetPlayerState()->Server_UpdateHealthArmor(CurrentHealth, CurrentArmor);

		UE_LOG(LogTemp, Warning, TEXT("ADeathMatchCharacter::TakeDamage => ( %s ) Took ( %i ) Damage By ( %s ). CurrentHealth: %i"), *GetName(), DamageOnHealth, *DamageCauser->GetName(), CurrentHealth);

		if (CurrentHealth <= 0.f)
		{
			Server_OnDeath(DamageCauser);
		}

		Client_OnTakeDamage();
	}
}

void ADeathMatchCharacter::Client_OnTakeDamage_Implementation()
{
	if (IsLocallyControlled())
	{
		if (GetController() != nullptr && UKismetSystemLibrary::DoesImplementInterface(GetController(), UPlayerControllerInterface::StaticClass()))
		{
			IPlayerControllerInterface::Execute_VignetteEffectOnTakeDamage(GetController());
		}
	}

	// TODO: TP damage recieving anim
}

void ADeathMatchCharacter::Server_OnDeath_Implementation(AActor* DeathCauser)
{
	UE_LOG(LogTemp, Warning, TEXT("ADeathMatchCharacter::OnDeath => ( %s ) is killed by ( %s )."), *GetName(), *DeathCauser->GetName());

	// Call "Server_OnKill()" for the killer player
	ADeathMatchCharacter* KillerPlayer = Cast<ADeathMatchCharacter>(DeathCauser);
	if (KillerPlayer != nullptr)
	{
		KillerPlayer->Server_OnKill(this);
	}

	// Update PlayerState
	if (GetPlayerState() != nullptr)
	{
		GetPlayerState()->Server_OnDeath();
	}

	// Notify PlayerController that this player is dead
	if (GetController() != nullptr && UKismetSystemLibrary::DoesImplementInterface(GetController(), UPlayerControllerInterface::StaticClass()))
	{
		IPlayerControllerInterface::Execute_OnPlayerDeath(GetController());
	}

	Multicast_OnDeath();
}

void ADeathMatchCharacter::Multicast_OnDeath_Implementation()
{
	GetCapsuleComponent()->SetCollisionProfileName(TEXT("Capsule_Dead"));
	GetMesh()->SetCollisionProfileName(TEXT("CharacterMesh_Dead"));
	GetCapsuleComponent()->SetGenerateOverlapEvents(false);

	// Drop Weapon
	DropWeapon();

	if (IsLocallyControlled())
	{
		// Activate DeathCamera
		HandleCameraOnDeath();
	}

	// Notify ( CharacterMesh )'s Animation that this player is dead => Play Death Anim
	UAnimInstance* CharacterAnimInstance = GetMesh()->GetAnimInstance();
	if (CharacterAnimInstance != nullptr && UKismetSystemLibrary::DoesImplementInterface(CharacterAnimInstance, UFPSAnimInterface::StaticClass()))
	{
		IFPSAnimInterface::Execute_OnDeath(CharacterAnimInstance);
	}
}

void ADeathMatchCharacter::HandleCameraOnDeath()
{
	FP_Camera->SetActive(false);
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

	ArmMesh->SetOwnerNoSee(true);
	GetMesh()->SetOwnerNoSee(false);
}

void ADeathMatchCharacter::Server_OnKill_Implementation(ADeathMatchCharacter* DeadPlayer)
{
	UE_LOG(LogTemp, Warning, TEXT("ADeathMatchCharacter::OnKill => ( %s ) killed ( %s )."), *GetName(), *DeadPlayer->GetName());

	if (GetPlayerState() != nullptr)
	{
		GetPlayerState()->Server_OnKillPlayer();
	}

	if (DeadPlayer != nullptr)
	{
		// TODO: DeadPlayer is probably not needed here. Maybe remove?
	}
}
#pragma endregion