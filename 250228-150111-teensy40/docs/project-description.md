# THOR Glove – Project Description & Requirements

## 1. Purpose

The **THOR Glove** is a wearable assistive‑force device that augments a user’s grip by actively flexing the fingers with miniature linear‑servo tendons.  It is designed for therapy, industrial safety, and daily‑living assistance where reduced hand strength limits function.

---

## 2. System Overview

| Sub‑system         | Components                                                                   | Function                                                                                                        |
| ------------------ | ---------------------------------------------------------------------------- | --------------------------------------------------------------------------------------------------------------- |
| **Actuation**      | • 5 micro linear‑servos (one per digit)• Tendon lines routed along palm side | Pull each finger into flexion on command.                                                                       |
| **Sensing**        | • 5 FSRs (nail‑bed side)• 5 inline motor‑current sensors                     | ‑ FSRs detect user intent (open vs. close).‑ Current sensors estimate opposition force & provide safety limits. |
| **Control & MCU**  | Teensy 4.0 (600 MHz Cortex‑M7)                                               | Runs 100 Hz closed‑loop firmware; handles calibration, control, safety & UI.                                    |
| **User Interface** | 3 momentary buttons (GRIP –, GRIP +, RELAX) Built‑in LED indicator           | Adjust hold‑force, toggle relax mode, display system state.                                                     |
| **Power & Safety** | 2.5 A per finger (hard limit)8 A combined system limit                       | Firmware cuts power if limits exceeded > 150 ms; all tendons released.                                          |

---

## 3. Functional Requirements

### 3.1 Finger Control Loop (per digit)

1. **Sampling rate:** 100 Hz (10 ms period).
2. **Intention detection:**

   * *Neutral force* established at start‑up (2 s average of FSR voltages).
   * ΔFSR > +dead‑band → **Open** intent.ΔFSR < –dead‑band → **Close** intent.
3. **Velocity command:**

   * `v_fsr =  K_FSR × ΔFSR` (deg / s).
   * Default `K_FSR = 450 deg·s⁻¹·V⁻¹`.
4. **Current override:**

   * If `I_servo > I_hold` → open until `I_servo ≈ I_hold` using `v_I = K_I × (I_servo – I_hold)`.
   * `I_hold` user‑adjustable 0.5 – 2.0 A (0.1 A steps).
5. **Position integration & limits:**

   * `θ_new = θ_prev + (v_cmd × dt)` constrained to mechanical limits per finger.
6. **Over‑current fault:**

   * `I_servo > 2.5 A` for > 150 ms → detach servo & flag fault.

### 3.2 Global Safety Logic

* Sum of **all** motor currents `I_sum` must stay < 8 A.
* If `I_sum` exceeds limit → immediate **system release** & enter *Relax* mode.

### 3.3 Modes & LED Indication

| Mode                        | Behaviour                                                     | LED |
| --------------------------- | ------------------------------------------------------------- | --- |
| **Active Assist** (default) | Closed‑loop finger control active.                            | OFF |
| **Relax**                   | All servos commanded open, detached; user can stretch freely. | ON  |
| **Fault** (over‑current)    | Same behaviour as Relax; auto‑entered.                        | ON  |

### 3.4 Buttons

| Button           | Action (on press)             | Notes                  |
| ---------------- | ----------------------------- | ---------------------- |
| **GRIP –**       | Decrease `I_hold` by 0.1 A    | Min 0.3 A              |
| **GRIP +**       | Increase `I_hold` by 0.1 A    | Max 1.5 A              |
| **RELAX Toggle** | Toggle between Active & Relax | Also resets from Fault |

---

## 4. Mechanical & Electrical Limits

* **Servo travel windows (deg):**

  * Pinky 15–250, Ring 15–190, Middle 15–190, Index 15–250, Thumb 15–250.
* **PWM range:** 500 µs (0 °) – 2500 µs (270 °) mapped linearly.
* **ADC resolution:** 12‑bit; `V_CC = 3.3 V`.
* **FSR operating range:** 0–3 V ≈ 0–100 N.

---

## 5. Firmware Architecture

```
+----------------------------------------------------+
|              main loop  (lab4.ino)                |
|  pollButtons()  →  controlLoop() @100 Hz          |
+-------------------+----------------+--------------+
                    |                |
        +-----------v----+   +-------v-----------+
        | Finger class   |   | Safety supervisor |
        | (finger.cpp)   |   | (per‑finger + sum)|
        +----------------+   +-------------------+
```

* **Calibration** phase before controlLoop enters steady state.
* **Finger class** encapsulates servo commands & sensor I/O with documented API.

---

## 6. Performance Targets

| Metric                                  | Target                                |
| --------------------------------------- | ------------------------------------- |
| Response latency (user intent → motion) | ≤ 30 ms                               |
| Grip hold accuracy                      | ±0.1 A around `I_hold`                |
| Fail‑safe reaction                      | ≤ 150 ms on over‑current              |
| Battery life goal                       | ≥ 2 h typical use (assumes < 2 A avg) |

---

## 7. Verification & Testing

1. **Unit tests** for Finger class using sensor stubs.
2. **Bench test** with spring scales to validate I\_hold regulation.
3. **Stress test** by artificial obstruction to trigger over‑current fault.
4. **User trial** to adjust default gains & neutral calibration window.

---

## 8. Open Items / Future Work

* Integrate inertial sensor for wrist‑angle detection (auto‑extension assist).
* Explore lighter actuator alternatives (e.g., SMA or pneumatic muscles).

---

*Document generated 2025‑05‑05 from current firmware and system brief.*
