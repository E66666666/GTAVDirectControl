#include "Settings.h"
#include <thirdparty/SimpleIni.h>
#include "Util/Logger.hpp"
#pragma warning( disable : 4244 )

Settings gSettings;

Settings::Settings() {
    SetOptimalDefaults();
}

void Settings::SetOptimalDefaults() {
    TrackShowDebug = true;
    AIShowDebug = false;
    AIDefaultActive = true;

    AILookaheadThrottleSpeedMult = 3.5f;
    AILookaheadThrottleMinDistance = 15.0f;
    AILookaheadBrakeSpeedMult = 2.5f;
    AILookaheadBrakeMinDistance = 15.0f;
    AILookaheadSteerSpeedMult = 1.8f;
    AILookaheadSteerMinDistance = 10.0f;

    AISteerMult = 1.33f;

    AICountersteerIncreaseStartAngle = 0.0f;
    AICountersteerIncreaseEndAngle = 90.0f;

    AIThrottleDecreaseStartAngle = 45.0f;
    AIThrottleDecreaseEndAngle = 90.0f;

    AIOversteerDetectionAngle = 10.0f;

    AIBrakePointDistanceThrottleMult = 0.5f;
    AIBrakePointDistanceBrakeMult = 0.75f;

    AIBrakePointHeadingMinAngle = 30.0f;
    AIBrakePointHeadingMaxAngle = 52.5f;
    AIBrakePointHeadingMinSpeed = 20.0f;
    AIBrakePointHeadingMaxSpeed = 40.0f;

    AIBrakePointRadiusMaxSpeed = 100.0f;
    AIBrakePointRadiusMaxRadius = 1000.0f;
}

void Settings::ReadSettings(const char *file) {
    CSimpleIniA ini;
    ini.SetUnicode(true);
    SI_Error err = ini.LoadFile(file);

    if (err != SI_OK) {
        gLogger.Write(ERROR, "Error loading settings file, using defaults");
        SetOptimalDefaults();
        return;
    }

    const char* debugSection = "Debug";
    const char* paramSection = "AIParameters";

    TrackShowDebug =                    ini.GetBoolValue(debugSection, "TrackShowDebug");
    AIShowDebug =                       ini.GetBoolValue(debugSection, "AIShowDebug");
    AIDefaultActive =                   ini.GetBoolValue(debugSection, "AIDefaultActive");

    AILookaheadThrottleSpeedMult =      ini.GetDoubleValue(paramSection, "AILookaheadThrottleSpeedMult");
    AILookaheadThrottleMinDistance =    ini.GetDoubleValue(paramSection, "AILookaheadThrottleMinDistance");
    AILookaheadBrakeSpeedMult =         ini.GetDoubleValue(paramSection, "AILookaheadBrakeSpeedMult");
    AILookaheadBrakeMinDistance =       ini.GetDoubleValue(paramSection, "AILookaheadBrakeMinDistance");
    AILookaheadSteerSpeedMult =         ini.GetDoubleValue(paramSection, "AILookaheadSteerSpeedMult");
    AILookaheadSteerMinDistance =       ini.GetDoubleValue(paramSection, "AILookaheadSteerMinDistance");

    AISteerMult =                       ini.GetDoubleValue(paramSection, "AISteerMult");

    AICountersteerIncreaseStartAngle =  ini.GetDoubleValue(paramSection, "AICountersteerIncreaseStartAngle");
    AICountersteerIncreaseEndAngle =    ini.GetDoubleValue(paramSection, "AICountersteerIncreaseEndAngle");

    AIThrottleDecreaseStartAngle =      ini.GetDoubleValue(paramSection, "AIThrottleDecreaseStartAngle");
    AIThrottleDecreaseEndAngle =        ini.GetDoubleValue(paramSection, "AIThrottleDecreaseEndAngle");

    AIOversteerDetectionAngle =         ini.GetDoubleValue(paramSection, "AIOversteerDetectionAngle");

    AIBrakePointDistanceThrottleMult =  ini.GetDoubleValue(paramSection, "AIBrakePointDistanceThrottleMult");
    AIBrakePointDistanceBrakeMult =     ini.GetDoubleValue(paramSection, "AIBrakePointDistanceBrakeMult");

    AIBrakePointHeadingMinAngle =       ini.GetDoubleValue(paramSection, "AIBrakePointHeadingMinAngle");
    AIBrakePointHeadingMaxAngle =       ini.GetDoubleValue(paramSection, "AIBrakePointHeadingMaxAngle");
    AIBrakePointHeadingMinSpeed =       ini.GetDoubleValue(paramSection, "AIBrakePointHeadingMinSpeed");
    AIBrakePointHeadingMaxSpeed =       ini.GetDoubleValue(paramSection, "AIBrakePointHeadingMaxSpeed");

    AIBrakePointRadiusMaxSpeed =        ini.GetDoubleValue(paramSection, "AIBrakePointRadiusMaxSpeed");
    AIBrakePointRadiusMaxRadius =       ini.GetDoubleValue(paramSection, "AIBrakePointRadiusMaxRadius");
    gLogger.Write(INFO, "Load settings success");
}

void Settings::WriteDefaults(const char *file) {
    SetOptimalDefaults();
    CSimpleIniA ini;
    ini.SetUnicode(true);
    ini.LoadFile(file);

    const char* debugSection = "Debug";
    const char* paramSection = "AIParameters";

    ini.SetBoolValue(debugSection, "TrackShowDebug", TrackShowDebug, 
        "; Show the track center line by default");
    ini.SetBoolValue(debugSection, "AIShowDebug", AIShowDebug, 
        "; Show the AI thinking lines by default.\n"
        "; Red: Braking, Green: Throttle, Blue: Steering\n"
        "; White: Physics heading without rotation\n"
        "; Yellow: Physics heading with rotation\n");
    ini.SetBoolValue(debugSection, "AIDefaultActive", AIDefaultActive, 
        "; Enable AI driving by default, after spawning");
    
    ini.SetDoubleValue(paramSection, "AILookaheadThrottleSpeedMult", AILookaheadThrottleSpeedMult, 
        "; Lookahead distance multiplier based on speed (m/s) in meters.");
    ini.SetDoubleValue(paramSection, "AILookaheadThrottleMinDistance", AILookaheadThrottleMinDistance,
        "; Minimal lookahead distance.");
    ini.SetDoubleValue(paramSection, "AILookaheadBrakeSpeedMult", AILookaheadBrakeSpeedMult);
    ini.SetDoubleValue(paramSection, "AILookaheadBrakeMinDistance", AILookaheadBrakeMinDistance);
    ini.SetDoubleValue(paramSection, "AILookaheadSteerSpeedMult", AILookaheadSteerSpeedMult);
    ini.SetDoubleValue(paramSection, "AILookaheadSteerMinDistance", AILookaheadSteerMinDistance);
    
    ini.SetDoubleValue(paramSection, "AISteerMult", AISteerMult,
        "; Multiplier for steering input, after calculations. Higher values make steering more abrupt.");
    
    ini.SetDoubleValue(paramSection, "AICountersteerIncreaseStartAngle", AICountersteerIncreaseStartAngle,
        "; Oversteer angle for base countersteer multiplier (1.0)");
    ini.SetDoubleValue(paramSection, "AICountersteerIncreaseEndAngle", AICountersteerIncreaseEndAngle,
        "; Oversteer angle for max countersteer multiplier (2.0)");
    
    ini.SetDoubleValue(paramSection, "AIThrottleDecreaseStartAngle", AIThrottleDecreaseStartAngle,
        "; Oversteer angle for base throttle decrease multiplier (1.0)");
    ini.SetDoubleValue(paramSection, "AIThrottleDecreaseEndAngle", AIThrottleDecreaseEndAngle,
        "; Oversteer angle for min throttle decrease multiplier (0.0)");
    
    ini.SetDoubleValue(paramSection, "AIOversteerDetectionAngle", AIOversteerDetectionAngle,
        "; Oversteer angle to start applying throttle and steering correction modifiers");
    
    ini.SetDoubleValue(paramSection, "AIBrakePointDistanceThrottleMult", AIBrakePointDistanceThrottleMult,
        "; m/s meters to throttle lookahead distance to start braking multiplier");
    ini.SetDoubleValue(paramSection, "AIBrakePointDistanceBrakeMult", AIBrakePointDistanceBrakeMult,
        "; m/s meters to brake lookahead distance for max braking multiplier");
    
    ini.SetDoubleValue(paramSection, "AIBrakePointHeadingMinAngle", AIBrakePointHeadingMinAngle,
        "; Upcoming angle to start braking at.\n"
        "; \"Upcoming angle\" is heading of track between throttle and brake lookahead coords.");
    ini.SetDoubleValue(paramSection, "AIBrakePointHeadingMaxAngle", AIBrakePointHeadingMaxAngle,
        "; Upcoming angle braking is max at");
    ini.SetDoubleValue(paramSection, "AIBrakePointHeadingMinSpeed", AIBrakePointHeadingMinSpeed,
        "; Speed for upcoming angle to start braking");
    ini.SetDoubleValue(paramSection, "AIBrakePointHeadingMaxSpeed", AIBrakePointHeadingMaxSpeed,
        "; Speed for upcoming angle for max braking");
    
    ini.SetDoubleValue(paramSection, "AIBrakePointRadiusMaxSpeed", AIBrakePointRadiusMaxSpeed,
        "; Speed to consider specified radius for.");
    ini.SetDoubleValue(paramSection, "AIBrakePointRadiusMaxRadius", AIBrakePointRadiusMaxRadius,
        "; Radius where speed is considered for. Example:\n"
        "; If a 1000 m radius corner is encountered at 100 m/s (360 kph), full brake force is used.");

    SI_Error err = ini.SaveFile(file);
    if (err == SI_FAIL) {
        gLogger.Write(ERROR, "Error saving defaults file");
    }
}