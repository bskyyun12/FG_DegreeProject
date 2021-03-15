// Fill out your copyright notice in the Description page of Project Settings.


#include "FPSCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h" // GetLifetimeReplicatedProps

#include "Animation/AnimInstance.h"
#include "AnimInstances/FPSAnimInterface.h"
#include "Components/HealthComponent.h"
#include "FPSPlayerControllerInterface.h"
#include "FPSPlayerController.h"

// Sets default values
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

	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	HealthComponent->OnDamageReceived.AddDynamic(this, &AFPSCharacter::OnDamageReceived);
	HealthComponent->OnHealthAcquired.AddDynamic(this, &AFPSCharacter::OnHealthAcquired);
	HealthComponent->OnDeath.AddDynamic(this, &AFPSCharacter::OnDeath);
}

// Called when the game starts or when spawned
void AFPSCharacter::BeginPlay()
{
	Super::BeginPlay();

	FPSCharacterMesh = GetMesh();
	if (!ensure(FPSCharacterMesh != nullptr))
	{
		return;
	}
	CapsuleComponent = GetCapsuleComponent();
	if (!ensure(CapsuleComponent != nullptr))
	{
		return;
	}

	DefaultCameraRelativeLocation = CameraContainer->GetRelativeLocation();
	DefaultCharacterMeshRelativeTransform = FPSCharacterMesh->GetRelativeTransform();
}

void AFPSCharacter::OnPossessed(AFPSPlayerController* InFPSController)
{
	FPSController = InFPSController;
}

void AFPSCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AFPSCharacter, bIsDead);
}

// Called to bind functionality to input
void AFPSCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("Pickup", IE_Pressed, this, &AFPSCharacter::Pickup);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AFPSCharacter::OnBeginFire);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &AFPSCharacter::OnEndFire);

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

		UAnimInstance* FPSAnimInstance = FPSCharacterMesh->GetAnimInstance();
		if (FPSAnimInstance != nullptr)
		{
			if (UKismetSystemLibrary::DoesImplementInterface(FPSAnimInstance, UFPSAnimInterface::StaticClass()))
			{
				IFPSAnimInterface::Execute_UpdateSpineAngle(FPSAnimInstance, CameraRot.Pitch);
			}
		}
	}
}

void AFPSCharacter::Pickup()
{
	if (CurrentFocus != nullptr)
	{
		PickupWeapon(CurrentFocus->GetWeaponType());

		// Temporary implementation
		if (bHasAnyWeapons == false)
		{
			EquipWeapon(CurrentFocus);
			bHasAnyWeapons = true;
		}
	}
}

void AFPSCharacter::OnBeginFire()
{
	if (CurrentWeapon != nullptr)
	{
		Server_OnBeginFire(CurrentWeapon, this);
	}
}

void AFPSCharacter::Server_OnBeginFire_Implementation(AFPSWeaponBase* Weapon, AFPSCharacter* FPSCharacter)
{
	UE_LOG(LogTemp, Warning, TEXT("AFPSCharacter::Server_OnBeginFire_Implementation"));
	if (Weapon != nullptr)
	{
		Weapon->Server_OnBeginFireWeapon(FPSCharacter);
	}
}

void AFPSCharacter::OnEndFire()
{
	if (CurrentWeapon != nullptr)
	{
		Server_OnEndFire(CurrentWeapon);
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

void AFPSCharacter::OnBeginOverlapWeapon_Implementation(AFPSWeaponBase* Weapon)
{
	UE_LOG(LogTemp, Warning, TEXT("AFPSCharacter::OnBeginOverlapWeapon_Implementation()"));

	NumOfOverlappingWeapons++;
	UE_LOG(LogTemp, Warning, TEXT("OnBeginOverlapWeapon() NumOfOverlappingWeapons: %i"), NumOfOverlappingWeapons);

	UWorld* World = GetWorld();
	if (!ensure(World != nullptr))
	{
		return;
	}

	if (World->GetTimerManager().IsTimerActive(PickupTraceTimerHandle))
	{
		UE_LOG(LogTemp, Warning, TEXT("Timer is already running"));
		return;
	}

	World->GetTimerManager().SetTimer(PickupTraceTimerHandle, this, &AFPSCharacter::Client_CheckForWeapon, 0.1f, true, 0.f);
}

void AFPSCharacter::Client_CheckForWeapon_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("CheckForWeapon"));

	UWorld* World = GetWorld();
	if (!ensure(World != nullptr))
	{
		return;
	}

	FHitResult Hit;
	FVector Start = FollowCamera->GetComponentLocation();
	FVector End = Start + FollowCamera->GetForwardVector() * 300.f;
	DrawDebugLine(World, Start, End, FColor::Red, false, 0.1f);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	bool bIsHit = World->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);
	if (bIsHit)
	{
		AFPSWeaponBase* FocusedWeapon = Cast<AFPSWeaponBase>(Hit.GetActor());
		if (FocusedWeapon != nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("Hit Weapon: %s"), *FocusedWeapon->GetName());
			CurrentFocus = FocusedWeapon;
		}

		DrawDebugPoint(World, Hit.ImpactPoint, 10.f, FColor::Green, false, 0.1f);
	}
	else
	{
		CurrentFocus = nullptr;
	}
}

void AFPSCharacter::OnEndOverlapWeapon_Implementation()
{
	NumOfOverlappingWeapons--;
	UE_LOG(LogTemp, Warning, TEXT("OnEndOverlapWeapon() NumOfOverlappingWeapons: %i"), NumOfOverlappingWeapons);

	if (NumOfOverlappingWeapons == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Clear Timer"));
		UWorld* World = GetWorld();
		if (!ensure(World != nullptr))
		{
			return;
		}
		World->GetTimerManager().ClearTimer(PickupTraceTimerHandle);
	}
}

void AFPSCharacter::PickupWeapon(EWeaponType WeaponType)
{
	Weapons.Add(WeaponType, 1);
	int16* Value = Weapons.Find(WeaponType);

	UE_LOG(LogTemp, Warning, TEXT("Player: %s picked up a weapon: %i, new amount: %d"), *this->GetName(), WeaponType, *Value);
}

bool AFPSCharacter::HasWeapon(EWeaponType WeaponType)
{
	return Weapons.Contains(WeaponType);
}

void AFPSCharacter::EquipWeapon(AFPSWeaponBase* Weapon)
{
	if (Weapon != nullptr && HasWeapon(Weapon->GetWeaponType()))
	{
		Server_EquipWeapon(Weapon);

		// Local
		if (!ensure(FPSArmMesh != nullptr))
		{
			return;
		}
		Weapon->Client_OnFPWeaponEquipped(this);

		CurrentWeapon = Weapon;
	}
}

void AFPSCharacter::Server_EquipWeapon_Implementation(AFPSWeaponBase* Weapon)
{
	Weapon->Server_OnTPWeaponEquipped(this);
}

void AFPSCharacter::OnDamageReceived()
{
	UE_LOG(LogTemp, Warning, TEXT("( %s ) received damage."), *this->GetName());

}

void AFPSCharacter::OnHealthAcquired()
{
	UE_LOG(LogTemp, Warning, TEXT("( %s ) acquired health."), *this->GetName());
}

void AFPSCharacter::OnDeath()
{
	bIsDead = true;	// OnRep_bIsDead()
	CollisionHandleOnDeath();

	if (FPSController != nullptr && UKismetSystemLibrary::DoesImplementInterface(FPSController, UFPSPlayerControllerInterface::StaticClass()))
	{
		IFPSPlayerControllerInterface::Execute_OnPlayerDeath(FPSController);
	}

	FTimerHandle RespawnTimer;
	GetWorld()->GetTimerManager().SetTimer(RespawnTimer, [&]()
		{
			RespawnPlayer();
		}, RespawnDelay, false);
}

void AFPSCharacter::RespawnPlayer()
{
	bIsDead = false;	// OnRep_bIsDead()
	CollisionHandleOnRespawn();

	if (HealthComponent != nullptr)
	{
		HealthComponent->Reset();
	}

	if (FPSController != nullptr && UKismetSystemLibrary::DoesImplementInterface(FPSController, UFPSPlayerControllerInterface::StaticClass()))
	{
		IFPSPlayerControllerInterface::Execute_RespawnPlayer(FPSController);
	}
}

void AFPSCharacter::OnRep_bIsDead()
{
	if (bIsDead)
	{
		CollisionHandleOnDeath();
	}
	else
	{
		CollisionHandleOnRespawn();
	}
}

void AFPSCharacter::CollisionHandleOnDeath()
{
	CapsuleComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FPSCharacterMesh->SetCollisionProfileName(TEXT("Ragdoll"));
	FPSCharacterMesh->SetSimulatePhysics(true);

	CameraContainer->SetCollisionProfileName(TEXT("IgnoreCharacter"));
	CameraContainer->SetSimulatePhysics(true);
}

void AFPSCharacter::CollisionHandleOnRespawn()
{
	CapsuleComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	FPSCharacterMesh->SetSimulatePhysics(false);
	FPSCharacterMesh->SetCollisionProfileName(TEXT("CharacterMesh"));
	FPSCharacterMesh->AttachToComponent(CapsuleComponent, FAttachmentTransformRules::SnapToTargetIncludingScale);
	FPSCharacterMesh->SetRelativeTransform(DefaultCharacterMeshRelativeTransform);

	CameraContainer->SetSimulatePhysics(false);
	CameraContainer->SetCollisionProfileName(TEXT("NoCollision"));
	CameraContainer->AttachToComponent(CapsuleComponent, FAttachmentTransformRules::SnapToTargetIncludingScale);
	CameraContainer->SetRelativeLocation(DefaultCameraRelativeLocation);
}

// Getters
USkeletalMeshComponent* AFPSCharacter::GetArmMesh()
{
	return FPSArmMesh;
}

USkeletalMeshComponent* AFPSCharacter::GetCharacterMesh()
{
	return FPSCharacterMesh;
}

FTransform AFPSCharacter::GetCameraTransform()
{
	return FollowCamera->GetComponentTransform();
}