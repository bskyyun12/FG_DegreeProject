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

	HandCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("HandCollider"));
	HandCollider->SetupAttachment(FollowCamera);
	HandCollider->OnComponentBeginOverlap.AddDynamic(this, &AFPSCharacter::OnBeginOverlapHandCollider);
	HandCollider->OnComponentEndOverlap.AddDynamic(this, &AFPSCharacter::OnEndOverlapHandCollider);

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

// Called to bind functionality to input
void AFPSCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AFPSCharacter::OnBeginFire);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &AFPSCharacter::OnEndFire);

	PlayerInputComponent->BindAction("Pickup", IE_Pressed, this, &AFPSCharacter::Pickup);

	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &AFPSCharacter::Reload);

	PlayerInputComponent->BindAction<FOneBooleanDelegate>("GameStatusWidget", IE_Pressed, this, &AFPSCharacter::HandleGameStatusWidget, true);
	PlayerInputComponent->BindAction<FOneBooleanDelegate>("GameStatusWidget", IE_Released, this, &AFPSCharacter::HandleGameStatusWidget, false);

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

void AFPSCharacter::OnBeginFire()
{
	if (HealthComponent != nullptr && HealthComponent->IsDead())
	{
		return;
	}

	if (CurrentWeapon != nullptr)
	{
		CurrentWeapon->Client_OnBeginFireWeapon();
		Server_OnBeginFire(CurrentWeapon);
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
	if (HealthComponent != nullptr && HealthComponent->IsDead())
	{
		return;
	}

	if (CurrentWeapon != nullptr)
	{
		CurrentWeapon->Client_OnEndFireWeapon();
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

void AFPSCharacter::Pickup()
{
	if (HealthComponent != nullptr && HealthComponent->IsDead())
	{
		return;
	}

	if (CurrentWeapon != nullptr)
	{
		// TODO: swap weapon here?
		return;
	}
	else
	{
		if (CurrentFocus != nullptr)
		{
			EquipWeapon(CurrentFocus);
		}
	}
}

void AFPSCharacter::Reload()
{
	if (HealthComponent != nullptr && HealthComponent->IsDead())
	{
		return;
	}

	if (CurrentWeapon != nullptr)
	{
		CurrentWeapon->Client_Reload();
	}
}

void AFPSCharacter::HandleGameStatusWidget(bool bDisplay)
{
	if (GetController() != nullptr && UKismetSystemLibrary::DoesImplementInterface(GetController(), UFPSPlayerControllerInterface::StaticClass()))
	{
		IFPSPlayerControllerInterface::Execute_HandleGameStatusWidget(GetController(), bDisplay);
	}
}

void AFPSCharacter::EquipWeapon(AFPSWeaponBase* Weapon)
{
	if (HealthComponent != nullptr && HealthComponent->IsDead())
	{
		return;
	}

	if (Weapon != nullptr)
	{
		Server_EquipWeapon(Weapon);
		Weapon->Client_OnFPWeaponEquipped(this);

		CurrentWeapon = Weapon;
	}
}

void AFPSCharacter::Server_EquipWeapon_Implementation(AFPSWeaponBase* Weapon)
{
	Weapon->Server_OnTPWeaponEquipped(this);
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
	UE_LOG(LogTemp, Warning, TEXT("AFPSCharacter::OnEndOverlapHandCollider"));
	UE_LOG(LogTemp, Warning, TEXT("OtherActor: %s"), *OtherActor->GetName());
	if (OtherActor != nullptr && UKismetSystemLibrary::DoesImplementInterface(OtherActor, UFPSWeaponInterface::StaticClass()))
	{
		if (CurrentFocus == IFPSWeaponInterface::Execute_GetWeapon(OtherActor))
		{
			CurrentFocus = nullptr;
		}
	}
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
	CollisionHandleOnDeath();

	FTimerHandle RespawnTimer;
	GetWorld()->GetTimerManager().SetTimer(RespawnTimer, [&]()
		{
			RespawnPlayer();
		}, RespawnDelay, false);

	if (HasAuthority())
	{
		if (GetController() != nullptr && UKismetSystemLibrary::DoesImplementInterface(GetController(), UFPSPlayerControllerInterface::StaticClass()))
		{
			IFPSPlayerControllerInterface::Execute_OnPlayerDeath(GetController());
		}
	}
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