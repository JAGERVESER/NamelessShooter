// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/NSHealthComponent.h"

UNSHealthComponent::UNSHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UNSHealthComponent::BeginPlay()
{
	Super::BeginPlay();
	Health = MaxHealth;
	OnHealthChanged.Broadcast(Health, 0.0f);
	ComponentOwner = GetOwner();
	check(MaxHealth > 0) // ������������ �������� ������ ���� ������ 0
	if (ComponentOwner)
	{
		ComponentOwner->OnTakeAnyDamage.AddDynamic(this, &UNSHealthComponent::OnTakeAnyDamage);
	}

}
// ������� �������, ���������� true ���� �������� ������ (false �����)
bool UNSHealthComponent::IsHealthFull() const
{
	return Health == MaxHealth;
}
// ���� ��� ������� �� ���� ���������
void UNSHealthComponent::OnTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	if ((Damage <= 0.0f) || IsDead() || !GetOwner()) return;
	SetHealth(Health - Damage);
	UE_LOG(LogTemp, Error, TEXT("%f"), Health);
	if (IsDead())
	{
		OnDeath.Broadcast();
	}
}
// ������������� ����� ���������� ��������
void UNSHealthComponent::SetHealth(float NewHealth)
{
	const auto NextHealth = FMath::Clamp(NewHealth, 0.0f, MaxHealth);
	const auto HealthDelta = NextHealth - Health;
	Health = NextHealth;

	OnHealthChanged.Broadcast(Health, HealthDelta);
}


