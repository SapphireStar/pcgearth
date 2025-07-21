// DebugHelper.h
#pragma once

#include "CoreMinimal.h"
#include "DrawDebugHelpers.h"

class PCG_API FDebugHelper
{
public:
    // 构造函数
    FDebugHelper();

    // 绘制球体
    static void DrawSphere(const FVector& Position, float Radius, const FColor& Color, float Duration);

    // 绘制正方体
    static void DrawBox(const FVector& Position, const FVector& Extent, const FColor& Color, float Duration);

    // 绘制线条
    static void DrawLine(const FVector& Start, const FVector& End, const FColor& Color, float Duration);

    // 绘制文字
    static void DrawText(const FVector& Position, const FString& Text, const FColor& Color, float Duration);

private:
    // 防止实例化
    static void InternalDrawSphere(const FVector& Position, float Radius, const FColor& Color, float Duration);
    static void InternalDrawBox(const FVector& Position, const FVector& Extent, const FColor& Color, float Duration);
    static void InternalDrawLine(const FVector& Start, const FVector& End, const FColor& Color, float Duration);
    static void InternalDrawText(const FVector& Position, const FString& Text, const FColor& Color, float Duration);
};
