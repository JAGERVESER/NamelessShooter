// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/NSBaseWeapon.h"
#include "NSBow.generated.h"

/**
 * 
 */
UCLASS()
class NAMELESSSHOOTER_API ANSBow : public ANSBaseWeapon
{
	GENERATED_BODY()

public:
	ANSBow();

protected:

	virtual bool StartAim() override;
	virtual bool StopAim() override;
	virtual void Shot() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Animations")
	TSubclassOf<UAnimInstance> BowAnimInstance;

private:

	bool CanShot = false;

};
