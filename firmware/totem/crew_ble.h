#pragma once

#include <Arduino.h>
#include "totem_config.h"

// ============================================================================
// BLE: HEART RATE CLIENT FOR YOU + UP TO 4 FRIENDS (totem only)
// ============================================================================
//
// The totem follows your Whoop (TARGET_WHOOP_MAC) plus the CREW_MAC_* straps in totem_config.h,
// each over its own connection, since a heart rate strap only sends its BPM
// to a device connected to it, not in its advertisement.
//
// All the BLE work runs in its own task on core 0, so a slow connect to a
// friend at the edge of range never freezes the LEDs.

// You.
extern volatile int currentBPM;
extern volatile bool hrConnected;

struct CrewMember {
  char initial;
  volatile int bpm;          // 0 when not connected or gone quiet
  volatile bool connected;
};
extern CrewMember crew[CREW_MAX];
extern int crewCount;        // configured friends (non-empty MACs)

// Starts the BLE task: scans for every configured strap and keeps them connected.
void bleSetup();

// Nothing to do: the BLE task does the work.
inline void bleLoop() {}
