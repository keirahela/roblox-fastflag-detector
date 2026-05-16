#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace fflag::flags {

enum class FlagType { Bool, Int, String, Log, Unknown };

struct PrefixEntry {
    std::string_view prefix;
    FlagType         type;
};

// Order matters: "DF*" must come before "F*" or "DFFlagFoo" would be misread as
// "F" + "FlagFoo".
inline constexpr std::array<PrefixEntry, 8> kPrefixes{{
    {"DFString", FlagType::String},
    {"DFFlag",   FlagType::Bool},
    {"DFInt",    FlagType::Int},
    {"DFLog",    FlagType::Log},
    {"FString",  FlagType::String},
    {"FFlag",    FlagType::Bool},
    {"FInt",     FlagType::Int},
    {"FLog",     FlagType::Log},
}};

// Flags allowed to differ from the stable baseline. Entries can be either a
// full FastFlag name ("FFlagFoo") or a bare name as shown in the diff output
// ("Foo") — the matcher accepts both.
inline constexpr std::array<std::string_view, 38> kAllowlist{{
    // User-tunable graphics, debug toggles, fullscreen
    "DFIntCSGLevelOfDetailSwitchingDistance",
    "DFIntCSGLevelOfDetailSwitchingDistanceL12",
    "DFIntCSGLevelOfDetailSwitchingDistanceL23",
    "DFIntCSGLevelOfDetailSwitchingDistanceL34",
    "FFlagHandleAltEnterFullscreenManually",
    "DFFlagTextureQualityOverrideEnabled",
    "DFIntTextureQualityOverride",
    "FIntDebugForceMSAASamples",
    "DFFlagDisableDPIScale",
    "FFlagDebugGraphicsPreferD3D11",
    "FFlagDebugSkyGray",
    "DFFlagDebugPauseVoxelizer",
    "DFIntDebugFRMQualityLevelOverride",
    "FIntFRMMaxGrassDistance",
    "FIntFRMMinGrassDistance",
    "FFlagDebugGraphicsPreferVulkan",
    "FFlagDebugGraphicsPreferOpenGL",
    "FIntGrassMovementReducedMotionFactor",

    // Roblox-side A/B rollout + telemetry. Flip with cohort assignment.
    "BatchLogEventSenderLinearLoggingUniverseSamplingPerMille",
    "AssetProviderUseNewBatchADClientForRefresh3",
    "FixRootScaleWithoutRetargeting",
    "ChannelName",
    "FRMRemoveIndirectQualityLevelUsage",
    "FixNotifyIconOrdering",
    "EnableContextualPlayabilityJoinErrors",
    "ValidateTextChannelParent",
    "LuauCompilePropagateTableProps2",
    "RemoveLegacyChatInNewStudioExp",
    "RewardedVideoAvailabilityResultEventThrottleHundrethsPercent",
    "SlimRollout",
    "EnableDataModelChangeTrackingHundredthPercent",
    "EnableInputRecorder4",
    "EnableDMRTelemetry2",
    "MoveMacWinTelemetryShutdownToAppBridge",
    "EnableDataModelChangeTracking7",
    "EnableExperienceStateCaptureDMRecorder5",
    "VideoCaptureMaxLongSide",
    "VideoCaptureMaxShortSide",
}};

struct Stripped {
    std::string bare;
    FlagType    type;
};

std::optional<Stripped> strip_prefix(std::string_view name);

// 4 bytes is the max safe read per flag cell — covers int32 exactly and bool's
// low byte without bleeding into the next cell in packed int arrays.
std::string render_raw(std::uint32_t raw);

}
