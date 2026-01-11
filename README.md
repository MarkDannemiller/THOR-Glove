# THOR Glove

An assistive powered glove that amplifies grip strength and finger flexion/extension using fingertip force-sensing resistors (FSRs) that drive tendon-linked servos. The design targets therapy, industrial safety, and daily-use scenarios where users need added grip endurance. This repository contains the firmware, quick demos, and hardware references that accompany the “Total Hand Orientation Reinforcement (T.H.O.R.) Glove for Assisted Mobility and Strength” paper.

## Project at a Glance
- Adds up to ~16 lb of assistive grip via five tendon-driven miniature servos housed in a wrist/forearm pod.
- User intent comes from FSRs mounted near the nail bed; servo current sensing enforces hold-force targets and over-current safety.
- Teensy 4.0 (Cortex-M7, 600 MHz) runs a 100 Hz control loop with per-finger velocity commands, current override, and global current limits.
- Power pod uses a 3S Li-ion pack feeding LM2596 buck regulators; harness carries five servo supplies plus logic rails.
- Mechanical stack: breathable liner + flexible TPU exoshell + low-friction external tendons (100 lb line) routed to each digit.

## Repository Layout
- `250228-150111-teensy40/` — PlatformIO firmware for the Teensy 4.0 production glove. Key files:
  - `src/lab4.ino` — main 100 Hz control loop, safety logic, button/LED UI, and debug printer.
  - `src/finger.h/.cpp` — Finger class (servo + FSR + current sensor abstraction, inversion support, limit enforcement).
  - `examples/` — small motion/reading demos (`finger_demo.ino`, `pointer-demo.ino`).
  - `docs/` — schematic (`SCH_GLOVE-PCB_2025-04-27.pdf`) and brief (`project-description.md`).
  - `platformio.ini` — Teensy 4.0 target configuration.
- `opamp-servo-control/` — Arduino sketch exploring dual-servo drive from a single FSR.
- `THOR Glove Paper.pdf` — primary design document; see summary highlights below.

## Control Behavior (Teensy Firmware)
- **Loop rate:** 100 Hz (10 ms period) in `lab4.ino`.
- **Per-finger intent:** FSR voltage compared to neutral (2 s average on boot). Above deadband (0.2 V) commands extension; below commands flexion. Velocity gain `K_FSR = 2000 deg/s/V`.
- **Current override:** If servo current exceeds user hold setpoint, velocity is driven open using `K_I = 100 deg/s/A` until current drops below target.
- **Current limits:** Per finger hard limit 2.5 A (`MAX_CURRENT_PER_SERVO`); system limit 12 A. Over-current >150 ms drives FAULT (servos detached) or RELAX mode.
- **Angles and limits:** Logical angles clamped per digit (e.g., pinky 15–250 deg, ring 5–190 deg, thumb/index 5–250 deg). Inversion flag handles reversed servo orientation before writing pulses (500–2500 µs over 0–270 deg).
- **UI:** Buttons on pins 8/9 adjust hold current (0.3–1.5 A in 0.1 A steps). Pin 10 toggles RELAX. LED on pin 13 lights when not in ACTIVE mode. Serial debug prints a table of FSR, current, angle, and velocity.
- **Startup:** 5 s delay, neutral FSR calibration (keep hand relaxed), then ACTIVE_ASSIST.

## Pin Map (Teensy 4.0 firmware)
- **Buttons:** GRIP- `8`, GRIP+ `9`, RELAX `10` (INPUT_PULLDOWN).
- **LEDs:** `12`, `13` (13 used for mode indication).
- **Servos / FSRs / Current sensors (per finger):**
  - Pinky: servo `3`, FSR `14`, current `17`
  - Ring: servo `5`, FSR `16`, current `21`
  - Middle: servo `2`, FSR `18`, current `15`
  - Index: servo `7`, FSR `20`, current `23`
  - Thumb: servo `4`, FSR `22`, current `19`

## Build and Flash (PlatformIO)
1) Install PlatformIO (VS Code extension or CLI).  
2) Connect Teensy 4.0 via USB.  
3) From `250228-150111-teensy40/`, build and upload:
   - `pio run -t upload`
4) Open a serial monitor at 9600 baud to view the live status table (ANSI clear codes are used; a terminal that supports them renders best).

Libraries: relies on the Arduino Servo library bundled with the Teensy platform (see `lib_extra_dirs` if you keep a separate Arduino libraries folder).

## Quick Demos
- `examples/finger_demo.ino` — simple servo sweep with FSR and current readout (115200 baud).  
- `examples/pointer-demo.ino` — cycles servos with basic FSR/current logging.  
- `opamp-servo-control/opamp-servo-control.ino` — two-servo motion driven by one FSR for early validation.

## Operating the Glove
1) Power on (11.1 V pack feeding the buck-regulated pod) and connect USB for debug if desired.  
2) Keep fingers relaxed during the 2 s neutral FSR calibration after the 5 s boot delay.  
3) Use GRIP- / GRIP+ to set the hold-current target (defaults to 1.5 A).  
4) RELAX toggles tendon release; FAULT auto-enters RELAX on current limit violations.  
5) Monitor `printDebug()` output for FSR voltage, servo current, angles, and summed totals. If total current approaches 12 A, reduce grip or load.

## Hardware Highlights (from the paper)
- Tendon-driven assist with external 100 lb lines routed over a TPU exoshell; soft knit liner for comfort and skin protection.
- Wrist/forearm actuator pod with five high-torque servos on a custom four-layer PCB; 12-conductor silicone harness to a carbon-fiber PET-G power pack.
- Power pack: 3S Li-ion source, LM2596 buck modules (6.3 V servo rail) plus 5 V regulator for logic; XT60 input, designed for up to ~15–20 A total draw.
- Mechanical safeguards: tendon mechanical stops, over-extension limits, and detachable tendons on FAULT/RELAX.

## Known Limitations / Next Steps
- Current prototype does not stabilize wrist rotation; future work targets a conformal cuff or Boa-style closure.  
- Exposed tendons can tug the pod under high load; Bowden tubes and smaller servos are being explored.  
- Alternative actuators (shape-memory alloy bundles) are under investigation to reduce mass and noise.

## References
- Full paper: `THOR Glove Paper.pdf`.  
- Firmware brief: `250228-150111-teensy40/docs/project-description.md`.  
- Schematic: `250228-150111-teensy40/docs/SCH_GLOVE-PCB_2025-04-27.pdf`.
