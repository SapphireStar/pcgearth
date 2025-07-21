// DebugHelper.cpp
#include "DebugHelper.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"

FDebugHelper::FDebugHelper()
{
	// 构造函数实现
}

void FDebugHelper::DrawSphere(const FVector& Position, float Radius, const FColor& Color, float Duration)
{
	InternalDrawSphere(Position, Radius, Color, Duration);
}

void FDebugHelper::DrawBox(const FVector& Position, const FVector& Extent, const FColor& Color, float Duration)
{
	InternalDrawBox(Position, Extent, Color, Duration);
}

void FDebugHelper::DrawLine(const FVector& Start, const FVector& End, const FColor& Color, float Duration)
{
	InternalDrawLine(Start, End, Color, Duration);
}

void FDebugHelper::DrawText(const FVector& Position, const FString& Text, const FColor& Color, float Duration)
{
	InternalDrawText(Position, Text, Color, Duration);
}

// 内部实现 - 绘制球体
void FDebugHelper::InternalDrawSphere(const FVector& Position, float Radius, const FColor& Color, float Duration)
{
	if (GEngine)
	{
		// 使用DrawDebugSphere绘制球体
		DrawDebugSphere(GEngine->GetWorld(), Position, Radius, 12, Color, false, Duration);
	}
}

// 内部实现 - 绘制正方体
void FDebugHelper::InternalDrawBox(const FVector& Position, const FVector& Extent, const FColor& Color, float Duration)
{
	if (GEngine)
	{
		// 使用DrawDebugBox绘制正方体
		DrawDebugBox(GEngine->GetWorld(), Position, Extent, FQuat::Identity, Color, false, Duration);
	}
}

// 内部实现 - 绘制线条
void FDebugHelper::InternalDrawLine(const FVector& Start, const FVector& End, const FColor& Color, float Duration)
{
	if (GEngine)
	{
		// 使用DrawDebugLine绘制线条
		DrawDebugLine(GEngine->GetWorld(), Start, End, Color, false, Duration, 0, 1);
	}
}

// 内部实现 - 绘制文字
void FDebugHelper::InternalDrawText(const FVector& Position, const FString& Text, const FColor& Color, float Duration)
{
	if (GEngine)
	{
		// 使用DrawDebugString绘制文字
		DrawDebugString(GEngine->GetWorld(), Position, Text, nullptr, Color, Duration, true);
	}
}
