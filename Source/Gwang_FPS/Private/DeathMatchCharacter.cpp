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
	DOREPLIFETIME_CONDITION(ADeathMatchCharacter, CurrentlyHeldWeapon, COND_SimulatedOnly);
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

	GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &ADeathMatchCharacter::OnBeginOverlap);

	DeathCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("DeathCamera"));
	DeathCamera->SetupAttachment(RootComponent);
	DeathCamera->SetRelativeRotation(FRotator(-50.f, 0.f, 0.f));
	DeathCamera->SetActive(false);
}

FVector ADeathMatchCharacter::GetCameraLocation() const
{
	return FP_Camera->GetComponentLocation();
}

void ADeathMatchCharacter::BeginPlay()
{
	Super::BeginPlay();

	UWorld* World = GetWorld();
	if (!ensure(World != nullptr))
	{
		return;
	}

	if (GetLocalRole() == ROLE_Authority)
	{
		GM = Cast<ADeathMatchGameMode>(World->GetAuthGameMode());
		if (!ensure(GM != nullptr))
		{
			return;
		}
		GM->OnStartMatch.AddDynamic(this, &ADeathMatchCharacter::Server_OnSpawn);
	}

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
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &ADeathMatchCharacter::Reload);
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
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
}

void ADeathMatchCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	UE_LOG(LogTemp, Warning, TEXT("(Gameflow) ADeathMatchCharacter::PossessedBy"));

	if (NewController != nullptr)
	{
		PS = NewController->GetPlayerState<ADeathMatchPlayerState>();
		if (!ensure(PS != nullptr))
		{
			return;
		}
	}
}

void ADeathMatchCharacter::UnPossessed()
{
	Super::UnPossessed();

}

void ADeathMatchCharacter::EquipMainWeapon()
{
	if (CurrentWeapons[1] != nullptr)
	{
		if (GetLocalRole() == ROLE_Authority)
		{
			CurrentlyHeldWeapon = CurrentWeapons[1];	// OnRep_CurrentlyHeldWeapon
			EquipWeapon(CurrentlyHeldWeapon);
		}

		if (GetLocalRole() == ROLE_AutonomousProxy)
		{
			CurrentlyHeldWeapon = CurrentWeapons[1];
			EquipWeapon(CurrentlyHeldWeapon);
			Server_EquipWeapon(CurrentlyHeldWeapon);
		}
	}

	// This should be in pickup?
	//EWeaponType WeaponType = IWeaponInterface::Execute_GetWeaponType(Weapon);
	//uint8 WeaponIndex = (uint8)WeaponType;
	//CurrentWeapons[WeaponIndex] = Weapon;
}

void ADeathMatchCharacter::EquipSubWeapon()
{
	if (CurrentWeapons[2] != nullptr)
	{
		if (GetLocalRole() == ROLE_Authority)
		{
			CurrentlyHeldWeapon = CurrentWeapons[2];	// OnRep_CurrentlyHeldWeapon
			EquipWeapon(CurrentlyHeldWeapon);
		}

		if (GetLocalRole() == ROLE_AutonomousProxy)
		{
			CurrentlyHeldWeapon = CurrentWeapons[2];
			EquipWeapon(CurrentlyHeldWeapon);
			Server_EquipWeapon(CurrentlyHeldWeapon);
		}
	}
}

void ADeathMatchCharacter::Server_EquipWeapon_Implementation(AActor* WeaponToEquip)
{
	CurrentlyHeldWeapon = WeaponToEquip;	// OnRep_CurrentlyHeldWeapon
	EquipWeapon(CurrentlyHeldWeapon);
}

void ADeathMatchCharacter::EquipWeapon(AActor* WeaponToEquip)
{
	UE_LOG(LogTemp, Warning, TEXT("ADeathMatchCharacter::EquipWeapon => Role: (%i)"), GetLocalRole());
	IWeaponInterface::Execute_OnWeaponEquipped(WeaponToEquip, this);
}

// All Other Clients ( COND_SimulatedOnly )
void ADeathMatchCharacter::OnRep_CurrentlyHeldWeapon()
{
	if (CurrentlyHeldWeapon != nullptr)
	{
		IWeaponInterface::Execute_OnWeaponEquipped(CurrentlyHeldWeapon, this);
	}
}

void ADeathMatchCharacter::Drop()
{
	if (CurrentlyHeldWeapon != nullptr)
	{
		UWorld* World = GetWorld();
		if (World != nullptr)
		{
			// Disable Overlap Event for 2 seconds. => Prevents instant pick up after dropping
			FTimerHandle OverlapDisableTimer;
			GetCapsuleComponent()->SetGenerateOverlapEvents(false);
			World->GetTimerManager().SetTimer(OverlapDisableTimer, [&]()
				{
					GetCapsuleComponent()->SetGenerateOverlapEvents(true);
				}, 2.f, false);
		}

		if (IsLocallyControlled())
		{
			DropWeapon();
		}

		if (GetLocalRole() == ROLE_Authority)
		{
			Multicast_DropWeapon();
		}

		if (GetLocalRole() == ROLE_AutonomousProxy)
		{
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
	}
}

void ADeathMatchCharacter::DropWeapon()
{
	UE_LOG(LogTemp, Warning, TEXT("ADeathMatchCharacter::DropWeapon => Role: (%i)"), GetLocalRole());

	if (CurrentlyHeldWeapon != nullptr)
	{
		EWeaponType WeaponType = IWeaponInterface::Execute_GetWeaponType(CurrentlyHeldWeapon);
		uint8 WeaponIndex = (uint8)WeaponType;
		CurrentWeapons[WeaponIndex] = nullptr;

		IWeaponInterface::Execute_OnWeaponDropped(CurrentlyHeldWeapon);
		CurrentlyHeldWeapon = nullptr;
	}
}

void ADeathMatchCharacter::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Warning, TEXT("ADeathMatchCharacter::OnBeginOverlap => OtherActor: ( %s )."), *OtherActor->GetName());
	if (OtherActor != nullptr && UKismetSystemLibrary::DoesImplementInterface(OtherActor, UWeaponInterface::StaticClass()))
	{
		PickupWeapon(OtherActor);
	}
}

void ADeathMatchCharacter::PickupWeapon(AActor* const& WeaponToPickup)
{
	if (WeaponToPickup != nullptr)
	{
		EWeaponType WeaponType = IWeaponInterface::Execute_GetWeaponType(WeaponToPickup);
		uint8 WeaponIndex = (uint8)WeaponType;
		bool bHasSameTypeWeapon = CurrentWeapons[WeaponIndex] != nullptr;
		if (bHasSameTypeWeapon)
		{
			UE_LOG(LogTemp, Warning, TEXT("Already has the same weapon type"));
		}
		else
		{
			if (CurrentlyHeldWeapon == nullptr)
			{
				IWeaponInterface::Execute_OnWeaponEquipped(WeaponToPickup, this);
				//Server_EquipWeapon(WeaponToPickup);
			}
			else
			{
				CurrentWeapons[WeaponIndex] = WeaponToPickup;
				//WeaponToPickup->SetVisibility(false);
			}
		}
	}
}

void ADeathMatchCharacter::Reload()
{
	if (CurrentlyHeldWeapon != nullptr)
	{
		IWeaponInterface::Execute_Reload(CurrentlyHeldWeapon);
		Server_Reload();
	}
}

void ADeathMatchCharacter::Server_Reload_Implementation()
{
	Multicast_Reload();
}

void ADeathMatchCharacter::Multicast_Reload_Implementation()
{
	if (!IsLocallyControlled())
	{
		IWeaponInterface::Execute_Reload(CurrentlyHeldWeapon);
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
	AddControllerPitchInput(Value);
	Server_LookUp(Value);
}

void ADeathMatchCharacter::Server_LookUp_Implementation(const float& Value)
{
	Multicast_LookUp(Value);
}

void ADeathMatchCharacter::Multicast_LookUp_Implementation(const float& Value)
{
	if (!IsLocallyControlled())
	{
		AddControllerPitchInput(Value);

		FP_Camera->SetWorldRotation(GetControlRotation());
		UAnimInstance* CharacterAnimInstance = GetMesh()->GetAnimInstance();
		if (CharacterAnimInstance != nullptr && UKismetSystemLibrary::DoesImplementInterface(CharacterAnimInstance, UFPSAnimInterface::StaticClass()))
		{
			IFPSAnimInterface::Execute_UpdateSpineAngle(CharacterAnimInstance, GetControlRotation().Pitch);
		}
	}
}

void ADeathMatchCharacter::OnBeginFire()
{
	if (CurrentlyHeldWeapon == nullptr)
	{
		return;
	}

	if (GetLocalRole() == ROLE_AutonomousProxy)
	{
		UE_LOG(LogTemp, Warning, TEXT("(ROLE_AutonomousProxy) OnBeginFire"));
		IWeaponInterface::Execute_BeginFire(CurrentlyHeldWeapon);
	}
	Server_OnBeginFire();
}

void ADeathMatchCharacter::Server_OnBeginFire_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("(Server) OnBeginFire"));
	BeginFire++;	// OnRep_BeginFire()
	IWeaponInterface::Execute_BeginFire(CurrentlyHeldWeapon);
}

// All Other Clients ( COND_SimulatedOnly )
void ADeathMatchCharacter::OnRep_BeginFire()
{
	UE_LOG(LogTemp, Warning, TEXT("(Simulated Client) OnBeginFire"));
	IWeaponInterface::Execute_BeginFire(CurrentlyHeldWeapon);
}

void ADeathMatchCharacter::OnEndFire()
{
	if (CurrentlyHeldWeapon == nullptr)
	{
		return;
	}
	if (GetLocalRole() == ROLE_AutonomousProxy)
	{
		UE_LOG(LogTemp, Warning, TEXT("(ROLE_AutonomousProxy) OnEndFire"));
		IWeaponInterface::Execute_EndFire(CurrentlyHeldWeapon);
	}
	Server_OnEndFire();
}

void ADeathMatchCharacter::Server_OnEndFire_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("(Server) OnEndFire"));
	EndFire++;
	IWeaponInterface::Execute_EndFire(CurrentlyHeldWeapon);
}

// All Other Clients ( COND_SimulatedOnly )
void ADeathMatchCharacter::OnRep_EndFire()
{
	UE_LOG(LogTemp, Warning, TEXT("(Simulated Client) OnEndFire"));
	IWeaponInterface::Execute_EndFire(CurrentlyHeldWeapon);
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

void ADeathMatchCharacter::Server_TakeDamage_Implementation(const uint8& DamageOnHealth, const uint8& DamageOnArmor, AActor* DamageCauser)
{
	if (PS == nullptr)
	{
		PS = GetPlayerState<ADeathMatchPlayerState>();
	}

	uint8 CurrentHealth = PS->GetCurrentHealth();
	uint8 CurrentArmor = PS->GetCurrentArmor();

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

	PS->Server_UpdateHealthArmor(CurrentHealth, CurrentArmor);

	UE_LOG(LogTemp, Warning, TEXT("ADeathMatchCharacter::TakeDamage => ( %s ) Took ( %i ) Damage By ( %s ). CurrentHealth: %i"), *GetName(), DamageOnHealth, *DamageCauser->GetName(), CurrentHealth);

	if (CurrentHealth <= 0.f)
	{
		Server_OnDeath(DamageCauser);
	}
}

void ADeathMatchCharacter::Client_UpdateHealthArmorUI_Implementation(const uint8& Health, const uint8& Armor)
{
	if (IsLocallyControlled())
	{
		ADeathMatchPlayerController* PC = Cast<ADeathMatchPlayerController>(GetController());
		PC->UpdateHealthArmorUI(Health, Armor);
	}
}

// This is bound to ADeathMatchGameMode::OnStartMatch delegate call
void ADeathMatchCharacter::Server_OnSpawn_Implementation()
{
	if (!ensure(GM != nullptr))
	{
		return;
	}

	if (!ensure(PS != nullptr))
	{
		return;
	}

	Multicast_WeaponSetupOnSpawn(GM->GetWeaponClass());
}

void ADeathMatchCharacter::Multicast_WeaponSetupOnSpawn_Implementation(const FWeaponClass& WeaponClass)
{
	GetCapsuleComponent()->SetCollisionProfileName(TEXT("Capsule_Alive"));
	GetMesh()->SetCollisionProfileName(TEXT("CharacterMesh_Alive"));

	if (IsLocallyControlled())
	{
		HandleCameraOnSpawn();
	}

	UE_LOG(LogTemp, Warning, TEXT("ADeathMatchCharacter::Multicast_OnSpawn => Role: (%i)"), GetLocalRole());

	// Initialize the weapon array 
	if (CurrentWeapons.Num() != (uint8)EWeaponType::EnumSize)
	{
		for (uint8 i = 0; i < (uint8)EWeaponType::EnumSize; i++)
		{
			CurrentWeapons.Add(nullptr);
			StartWeapons.Add(nullptr);
		}
	}

	if (PS == nullptr)
	{
		PS = GetPlayerState<ADeathMatchPlayerState>();
	}

	// Set up StartWeapons
	UWorld* World = GetWorld();
	if (World != nullptr)
	{
		// MainWeapon Setup
		if (StartWeapons[1] == nullptr)
		{
			switch (PS->GetStartMainWeapon())
			{
			default:
				break;
			case EMainWeapon::M4A1:
				if (!ensure(WeaponClass.M4A1Class != nullptr))
				{
					return;
				}
				StartWeapons[1] = World->SpawnActor<AActor>(WeaponClass.M4A1Class);
				break;
			case EMainWeapon::AK47:
				if (!ensure(WeaponClass.AK47Class != nullptr))
				{
					return;
				}
				StartWeapons[1] = World->SpawnActor<AActor>(WeaponClass.AK47Class);
				break;
			}
		}

		// SubWeapon Setup
		if (StartWeapons[2] == nullptr)
		{
			switch (PS->GetStartSubWeapon())
			{
			case ESubWeapon::Pistol:
				if (!ensure(WeaponClass.PistolClass != nullptr))
				{
					return;
				}
				StartWeapons[2] = World->SpawnActor<AActor>(WeaponClass.PistolClass);
				break;
			}
		}
	}

	// Override CurrentWeapons to StartWeapons
	CurrentWeapons = StartWeapons;

	// Equip the weapon except the SimulatedProxy. They will equip it when "OnRep_CurrentlyHeldWeapon" gets called
	if (GetLocalRole() == ROLE_Authority || GetLocalRole() == ROLE_AutonomousProxy)
	{
		CurrentlyHeldWeapon = CurrentWeapons[1]; 	// (If Server) OnRep_CurrentlyHeldWeapon()
		EquipWeapon(CurrentlyHeldWeapon);
	}
}

void ADeathMatchCharacter::HandleCameraOnSpawn()
{
	FP_Camera->SetActive(true);
	DeathCamera->SetActive(false);
}

void ADeathMatchCharacter::Server_OnDeath_Implementation(AActor* DeathCauser)
{
	UE_LOG(LogTemp, Warning, TEXT("ADeathMatchCharacter::OnDeath => ( %s ) is killed by ( %s )."), *GetName(), *DeathCauser->GetName());

	// Call OnKill() for the killer player
	ADeathMatchCharacter* KillerPlayer = Cast<ADeathMatchCharacter>(DeathCauser);
	if (KillerPlayer != nullptr)
	{
		KillerPlayer->Server_OnKill(this);
	}

	// Update PlayerState
	PS->Server_OnDeath();

	Multicast_OnDeath();
}

void ADeathMatchCharacter::Multicast_OnDeath_Implementation()
{
	GetCapsuleComponent()->SetCollisionProfileName(TEXT("Capsule_Dead"));
	GetMesh()->SetCollisionProfileName(TEXT("CharacterMesh_Dead"));

	// Drop Weapon
	DropWeapon();

	if (IsLocallyControlled())
	{
		// Activate DeathCamera
		HandleCameraOnDeath();
	}

	if (!IsLocallyControlled())
	{
		// Notify player's death to AnimInstance. So it can play death animation. 
		// TODO: Should I just play death anim here?
		UAnimInstance* CharacterAnimInstance = GetMesh()->GetAnimInstance();
		if (CharacterAnimInstance != nullptr && UKismetSystemLibrary::DoesImplementInterface(CharacterAnimInstance, UFPSAnimInterface::StaticClass()))
		{
			IFPSAnimInterface::Execute_OnDeath(CharacterAnimInstance);
		}
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
}

void ADeathMatchCharacter::Server_OnKill_Implementation(ADeathMatchCharacter* DeadPlayer)
{
	UE_LOG(LogTemp, Warning, TEXT("ADeathMatchCharacter::OnKill => ( %s ) killed ( %s )."), *GetName(), *DeadPlayer->GetName());

	PS->Server_OnKillPlayer();

	if (DeadPlayer != nullptr)
	{
		// TODO: DeadPlayer is probably not needed here. Maybe remove?
	}
}