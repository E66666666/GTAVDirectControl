#include "Cheats.h"

#include "script.h"
#include "PlayerDirectInput.h"
#include "Settings.h"

#include "Util/MathExt.h"
#include "Util/StringFormat.h"
#include "Util/UIUtils.h"

#include "json.hpp"

#pragma warning( push )  
#pragma warning( disable : 4244 )
#pragma warning( disable : 4996 )
#include "xml2json.hpp"
#pragma warning( pop )

#include <iomanip>
#include <filesystem>
#include "Track.h"
#include "Session.h"
#include "Player.h"

using json = nlohmann::json;

extern bool gRecording;

extern std::vector<std::unique_ptr<Racer>> gRacers;
extern std::unique_ptr<PlayerRacer> gPlayerRacer;
extern std::unique_ptr<PlayerDirectInput> gPlayerDirectInput;

namespace {
Vehicle spawnVehicle(Hash hash, Vector3 coords, float heading, DWORD timeout, bool managed) {
    if (!(STREAMING::IS_MODEL_IN_CDIMAGE(hash) && STREAMING::IS_MODEL_A_VEHICLE(hash))) {
        // Vehicle doesn't exist
        return 0;
    }
    STREAMING::REQUEST_MODEL(hash);
    DWORD startTime = GetTickCount();

    while (!STREAMING::HAS_MODEL_LOADED(hash)) {
        WAIT(0);
        if (GetTickCount() > startTime + timeout) {
            // Couldn't load model
            WAIT(0);
            STREAMING::SET_MODEL_AS_NO_LONGER_NEEDED(hash);
            return 0;
        }
    }

    Vehicle veh = VEHICLE::CREATE_VEHICLE(hash, coords.x, coords.y, coords.z, heading, 0, 1);
    Vehicle copy = veh;
    VEHICLE::SET_VEHICLE_ON_GROUND_PROPERLY(veh);

    if (!managed) {
        WAIT(0);
        STREAMING::SET_MODEL_AS_NO_LONGER_NEEDED(hash);
        ENTITY::SET_ENTITY_AS_MISSION_ENTITY(veh, false, true);
        ENTITY::SET_ENTITY_AS_NO_LONGER_NEEDED(&veh);
    }
    return copy;
}

Vehicle findClosestVehicle(Vector3 p, float d) {
    std::vector<Vehicle> vehicles(1024);
    auto numFound = worldGetAllVehicles(vehicles.data(), 1024);
    vehicles.resize(numFound);
    float closestDistance = 1e6f;
    Vehicle closestVehicle = 0;
    for (auto vehicle : vehicles) {
        float distance = Distance(ENTITY::GET_ENTITY_COORDS(vehicle, true), p);
        if (distance < closestDistance) {
            closestDistance = distance;
            closestVehicle = vehicle;
        }
    }
    return closestVehicle;
}

std::unordered_map<Hash, std::string> checkCache(const std::string& cacheFile) {
    std::unordered_map<Hash, std::string> vehicleHashes;
    std::ifstream infile(cacheFile);
    if (infile.is_open()) {
        Hash hash;
        std::string name;
        while (infile >> hash >> name) {
            vehicleHashes.insert({ hash, name });
        }
    }
    return vehicleHashes;
}

std::string getVehicleModelName(Hash model) {
    std::string cacheFile = ".\\AddonSpawner\\hashes.cache";
    std::unordered_map<Hash, std::string> hashCache = checkCache(cacheFile);
    if (!hashCache.empty()) {
        return std::find_if(hashCache.begin(), hashCache.end(), [model](const auto& entry) { return entry.first == model; })->second;
    }
    return VEHICLE::GET_DISPLAY_NAME_FROM_VEHICLE_MODEL(model);
}
}

namespace Cheats {
void ControlCar(Vehicle vehicle) {
    if (gPlayerDirectInput) {
        showNotification("Stop controlling vehicle " + std::to_string(gPlayerDirectInput->GetVehicle()));
        gPlayerDirectInput.reset(nullptr);
    }
    else {
        if (ENTITY::DOES_ENTITY_EXIST(vehicle)) {
            showNotification("Controlling vehicle " + std::to_string(vehicle));
            gPlayerDirectInput = std::make_unique<PlayerDirectInput>(vehicle, 2);
        }
        else {
            showNotification("No vehicle to control");
        }
    }
}

void Passenger(Vehicle vehicle, Ped playerPed) {
    if (ENTITY::DOES_ENTITY_EXIST(vehicle)) {
        if (VEHICLE::GET_VEHICLE_MODEL_NUMBER_OF_SEATS(ENTITY::GET_ENTITY_MODEL(vehicle)) > 1) {
            if (VEHICLE::GET_PED_IN_VEHICLE_SEAT(vehicle, -1) == playerPed) {
                showNotification("Player to Passenger seat");
                PED::SET_PED_INTO_VEHICLE(playerPed, vehicle, -2);
            }
            else {
                showNotification("Player to Driver seat");
                Ped driver = VEHICLE::GET_PED_IN_VEHICLE_SEAT(vehicle, -1);
                if (ENTITY::DOES_ENTITY_EXIST(driver)) {
                    ENTITY::SET_ENTITY_AS_MISSION_ENTITY(driver, false, true);
                    ENTITY::DELETE_ENTITY(&driver);
                }
                PED::SET_PED_INTO_VEHICLE(playerPed, vehicle, -1);
            }
        }
        else {
            showNotification("Vehicle has only one seat");
        }
    }
}

void EnterPassenger(Ped playerPed) {
    Vector3 myCoords = ENTITY::GET_ENTITY_COORDS(playerPed, true);

    Vehicle v = findClosestVehicle(myCoords, 5.0f);
    if (ENTITY::DOES_ENTITY_EXIST(v)) {
        AI::TASK_ENTER_VEHICLE(playerPed, v, 5000, 0, 2.0f, 0, 0);
    }
}

void DebugPlayer() {
    if (gPlayerDirectInput) {
        showNotification("Toggling debug for " + std::to_string(gPlayerDirectInput->GetVehicle()));
        gPlayerDirectInput->SetDebugView(!gPlayerDirectInput->GetDebugView());
    }
}

void MakeAi(Vehicle vehicle, bool enableAi) {
    if (ENTITY::DOES_ENTITY_EXIST(vehicle)) {
        if (!enableAi) {
            auto itFound = gRacers.end();
            for (auto it = gRacers.begin(); it != gRacers.end(); ++it) {
                if ((*it)->GetVehicle() == vehicle) {
                    itFound = it;
                }
            }
            if (itFound != gRacers.end()) {
                gRacers.erase(itFound);
                showNotification(fmt("Remove AI from ~b~%s", VEHICLE::GET_VEHICLE_NUMBER_PLATE_TEXT(vehicle)));
            }
            else {
                showNotification("~r~No AI on this vehicle");
            }
        }
        else {
            bool found = false;
            for (auto& gRacer : gRacers) {
                if (gRacer->GetVehicle() == vehicle) {
                    found = true;
                }
            }
            if (!found) {
                Hash model = ENTITY::GET_ENTITY_MODEL(vehicle);
                std::string gameName = getVehicleModelName(model);

                gRacers.push_back(std::make_unique<Racer>(vehicle, gameName));

                Ped spawnedPed = PED::CREATE_RANDOM_PED_AS_DRIVER(vehicle, true);
                PED::SET_BLOCKING_OF_NON_TEMPORARY_EVENTS(spawnedPed, true);
                PED::SET_PED_KEEP_TASK(spawnedPed, true);
                PED::SET_DRIVER_ABILITY(spawnedPed, 1.0f);
                PED::SET_DRIVER_AGGRESSIVENESS(spawnedPed, 100.0f);

                ENTITY::SET_ENTITY_LOAD_COLLISION_FLAG(vehicle, true);

                gRacers.back()->SetTrack(Session::Get().GetTrack());
                showNotification(fmt("Adding AI to ~b~%s", VEHICLE::GET_VEHICLE_NUMBER_PLATE_TEXT(vehicle)));
            }
            else {
                showNotification("~r~AI already on this vehicle");
            }
        }
    }
    else {
        showNotification("~r~Player not in vehicle");
    }
}

void AddAi(Vehicle vehicle, Ped playerPed) {
    showNotification("Enter number of AIs to add");
    GAMEPLAY::DISPLAY_ONSCREEN_KEYBOARD(0, "FMMC_KEY_TIP8", "", "", "", "", "", 31);
    while (GAMEPLAY::UPDATE_ONSCREEN_KEYBOARD() == 0) WAIT(0);
    if (!GAMEPLAY::GET_ONSCREEN_KEYBOARD_RESULT()) {
        showNotification("Cancelled AI Spawn");
        return;
    }

    std::string numRacersString = GAMEPLAY::GET_ONSCREEN_KEYBOARD_RESULT();
    int numRacers = 1;
    int result = sscanf_s(numRacersString.c_str(), "%d", &numRacers);

    if (result != 1)
        numRacers = 1;

    showNotification("Enter AI model (empty = kuruma)");
    GAMEPLAY::DISPLAY_ONSCREEN_KEYBOARD(0, "FMMC_KEY_TIP8", "", "", "", "", "", 31);
    while (GAMEPLAY::UPDATE_ONSCREEN_KEYBOARD() == 0) WAIT(0);
    if (!GAMEPLAY::GET_ONSCREEN_KEYBOARD_RESULT()) {
        showNotification("Cancelled AI Spawn");
        return;
    }

    std::string racerModelName = GAMEPLAY::GET_ONSCREEN_KEYBOARD_RESULT();
    if (racerModelName.empty())
        racerModelName = "kuruma";

    Hash model = GAMEPLAY::GET_HASH_KEY((char*)racerModelName.c_str());
    if (!STREAMING::IS_MODEL_IN_CDIMAGE(model)) {
        showNotification("Model not valid: Cancelling AI Spawn");
        return;
    }

    for (auto i = 0; i < numRacers; ++i) {
        float offsetX = 0.0f;
        Vector3 modelDimMin, modelDimMax, myDimMin, myDimMax;
        GAMEPLAY::GET_MODEL_DIMENSIONS(model, &modelDimMin, &modelDimMax);

        if (ENTITY::DOES_ENTITY_EXIST(vehicle)) {
            Hash myModel = ENTITY::GET_ENTITY_MODEL(vehicle);
            GAMEPLAY::GET_MODEL_DIMENSIONS(myModel, &myDimMin, &myDimMax);
        }
        else {
            myDimMin = modelDimMin;
            myDimMax = modelDimMax;
        }

        // to the right
        // my width + ( margin + spawn width ) * iteration
        offsetX = ((myDimMax.x - myDimMin.x) / 2.0f) + (2.0f + ((modelDimMax.x - modelDimMin.x) / 2.0f)) * (float)(i + 1);

        Vector3 spawnPos = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(playerPed, offsetX, 0.0, 0);
        Vehicle spawnedVehicle = spawnVehicle(model, spawnPos, ENTITY::GET_ENTITY_HEADING(playerPed), 1000, true);

        Ped spawnedPed = PED::CREATE_RANDOM_PED_AS_DRIVER(spawnedVehicle, true);
        PED::SET_BLOCKING_OF_NON_TEMPORARY_EVENTS(spawnedPed, true);
        PED::SET_PED_KEEP_TASK(spawnedPed, true);
        PED::SET_DRIVER_ABILITY(spawnedPed, 1.0f);
        PED::SET_DRIVER_AGGRESSIVENESS(spawnedPed, 100.0f);

        ENTITY::SET_ENTITY_LOAD_COLLISION_FLAG(spawnedVehicle, true);

        gRacers.push_back(std::make_unique<Racer>(spawnedVehicle, racerModelName));
        gRacers.back()->SetTrack(Session::Get().GetTrack());
    }
}

void StartAi(bool start) {
    for (auto& racer : gRacers) {
        racer->SetActive(start);
    }
}

void FixAi() {
    for (auto& racer : gRacers) {
        racer->Fix();
    }
}

void DelAi() {
    gRacers.clear();
}

void DebugAiVis(bool show) {
    for (auto& racer : gRacers) {
        racer->SetDebugView(show);
    }
}

void DebugAiTxt(bool show) {
    for (auto& racer : gRacers) {
        racer->SetDebugText(show);
    }
}

void DebugThis(Vehicle vehicle) {
    for (auto& racer : gRacers) {
        if (racer->GetVehicle() == vehicle) {
            showNotification("Toggle debug for current");
            racer->SetDebugView(!racer->GetDebugView());
            break;
        }
    }
}

void LoadConfig(Vehicle vehicle) {
    showNotification("Enter config to load (this vehicle only)");
    GAMEPLAY::DISPLAY_ONSCREEN_KEYBOARD(0, "FMMC_KEY_TIP8", "", "", "", "", "", 31);
    while (GAMEPLAY::UPDATE_ONSCREEN_KEYBOARD() == 0) WAIT(0);
    if (!GAMEPLAY::GET_ONSCREEN_KEYBOARD_RESULT()) {
        showNotification("Cancelled cfg load");
        return;
    }
    std::string configName = GAMEPLAY::GET_ONSCREEN_KEYBOARD_RESULT();
    for (auto& racer : gRacers) {
        if (racer->GetVehicle() == vehicle) {
            showNotification("Toggle debug for current");
            racer->UpdateConfig(configName);
            break;
        }
    }
}

void RecordTrack(Ped playerPed, bool start) {
    if (start) {
        Recorder::Get().ClearPoints();
        Recorder::Get().Append(Point(ENTITY::GET_ENTITY_COORDS(playerPed, true), 5.0f));
        gRecording = true;
        showNotification("~g~Record started");
    }
    else {
        gRecording = false;
        showNotification("~r~Record stopped");
    }
}

void ClearTrack(Ped playerPed) {
    Session::Get().Reset();

    //Session::Get().GetTrack().Append(Point(ENTITY::GET_ENTITY_COORDS(playerPed, true), 5.0f));
    gRecording = false;
    showNotification("~r~Record cleared");
}

void SaveTrack() {
    GAMEPLAY::DISPLAY_ONSCREEN_KEYBOARD(0, "FMMC_KEY_TIP8", "", "", "", "", "", 64);
    while (GAMEPLAY::UPDATE_ONSCREEN_KEYBOARD() == 0) WAIT(0);
    if (!GAMEPLAY::GET_ONSCREEN_KEYBOARD_RESULT()) {
        showNotification("Cancelled save");
        return;
    }
    std::string saveFile = GAMEPLAY::GET_ONSCREEN_KEYBOARD_RESULT();

    nlohmann::json j;
    const std::vector<Point>& coords = Recorder::Get().Points();
    for (int idx = 0; idx < coords.size(); ++idx) {
        j["Data"]["Route"]["Point"].push_back({
            { "X", coords[idx].v.x },
            { "Y", coords[idx].v.y },
            { "Z", coords[idx].v.z },
            { "Wide", coords[idx].w },
            });
    }
    std::ofstream o("./DirectControl/Tracks/" + saveFile + ".json");
    o << std::setw(4) << j << std::endl;

    showNotification("~g~Track saved");
}

void LoadTrack() {
    std::string path = "./DirectControl/Tracks/";
    for (auto& p : std::filesystem::directory_iterator(path)) {
        if (p.path().extension() == ".json" || p.path().extension() == ".xml")
            showNotification(fmt("Track: ~b~%s", p.path().stem().string().c_str()));
    }

    GAMEPLAY::DISPLAY_ONSCREEN_KEYBOARD(0, "FMMC_KEY_TIP8", "", "", "", "", "", 64);
    while (GAMEPLAY::UPDATE_ONSCREEN_KEYBOARD() == 0) WAIT(0);
    if (!GAMEPLAY::GET_ONSCREEN_KEYBOARD_RESULT()) {
        showNotification("Cancelled load");
        return;
    }
    std::string loadFile = GAMEPLAY::GET_ONSCREEN_KEYBOARD_RESULT();

    json j;
    std::ifstream i(path + loadFile + ".json");
    if (!i.is_open()) {
        i.open(path + loadFile + ".xml");
        if (!i.is_open()) {
            showNotification("Couldn't find file");
            return;
        }
        std::string jStr = xml2json(std::string(std::istreambuf_iterator<char>(i), std::istreambuf_iterator<char>()).c_str());
        j = json::parse(jStr);
    }
    else {
        i >> j;
    }

    std::vector<Point> trackCoords;

    for (auto& pj : j["Data"]["Route"]["Point"]) {
        Point p;
        //Vector3 v;

        if (pj["X"].is_string()) {
            p.v.x = static_cast<float>(std::atof(pj["X"].get<std::string>().c_str()));
            p.v.y = static_cast<float>(std::atof(pj["Y"].get<std::string>().c_str()));
            p.v.z = static_cast<float>(std::atof(pj["Z"].get<std::string>().c_str()));
            //p.w = static_cast<float>(std::atof(pj["Wide"].get<std::string>().c_str()));
        }
        else {
            p.v.x = pj["X"];
            p.v.y = pj["Y"];
            p.v.z = pj["Z"];
            //p.w = pj["Wide"];
        }

        if (pj.find("Wide") != pj.end()) {
            if (pj["Wide"].is_string()) {
                p.w = static_cast<float>(std::atof(pj["Wide"].get<std::string>().c_str()));
            }
            else {
                p.w = pj["Wide"];
            }
        }
        else {
            p.w = 5.0f;
        }

        trackCoords.push_back(p);
    }

    Session::Get().SetTrack(Track("", trackCoords));

    for (auto& racer : gRacers) {
        racer->SetTrack(Session::Get().GetTrack());
    }
    if (gPlayerRacer)
        gPlayerRacer->SetTrack(Session::Get().GetTrack());

    showNotification(fmt("~g~Track loaded\n"
        "Distance: %.2f km", Session::Get().GetTrack().Length()/1000.0f));
}

void ReverseTrack() {
    Session::Get().ReverseTrack();
    for (auto& racer : gRacers) {
        racer->SetTrack(Session::Get().GetTrack());
    }
    if (gPlayerRacer)
        gPlayerRacer->SetTrack(Session::Get().GetTrack());
}

void DebugTrack() {
    gSettings.TrackShowDebug = !gSettings.TrackShowDebug;
}

void ReloadSettings() {
    gSettings.ReadSettings("./DirectControl/settings.ini");
    for (auto& racer : gRacers) {
        racer->UpdateConfig("");
    }
    showNotification("Reloaded settings");
}

}
