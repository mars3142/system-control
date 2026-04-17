# Matter over Thread Architecture

This document outlines the architectural decision and technical implementation strategy for integrating external end devices (e.g., display-less components like lighthouses, signals) into the System Control ecosystem.

## 1. Architectural Decision: Why Matter over Thread?

We have decided to use **Matter over Thread** as the standard communication protocol between the main controller (ESP) and external end devices. 

### Key Benefits
*   **Standardization:** Matter provides a standardized application layer (Clusters and Endpoints). A "light" or "blinking" function is mapped to standard clusters (e.g., `OnOff Cluster`), meaning the UI and backend do not need custom JSON parsing for every new device type.
*   **Ecosystem Compatibility (Multi-Admin):** Matter's Multi-Admin feature allows a single end device to be controlled by multiple controllers simultaneously. This means a device can be paired to the ESP's web UI **and** directly to Apple Home or Google Home at the same time.
*   **Robust Infrastructure:** Thread provides a self-healing IPv6 mesh network. The ESP acts as the Thread Border Router and Matter Commissioner.

## 2. System Roles

### Main Controller (ESP)
1.  **Thread Border Router / Commissioner:** The ESP creates and manages the Thread network. It exposes a "Permit Join" window (accessible via the Web UI) to allow new, display-less devices to join the network.
2.  **Matter Controller:** The ESP acts as a Matter Controller. Once a device is on the Thread network, the ESP discovers its capabilities (Clusters) and exposes them to the Web UI for configuration and control.

### End Devices (e.g., Lighthouse)
*   Act as standard **Matter Accessories**.
*   Join the Thread network during the "Permit Join" window using their setup code (PSKd).
*   Expose their capabilities as standard Matter endpoints (e.g., Endpoint 1: Light, Endpoint 2: Blinking feature).

## 3. UI Workflow

The web interface separates network infrastructure from application control:

1.  **System Tab (Infrastructure):** Used to open the Thread network ("Pair Devices" / Permit Join) so that new devices can receive an IP address and join the mesh.
2.  **Configuration Tab -> Devices (Discovery):** Used to discover Matter devices that have joined the network, authenticate them via their Setup PIN, and map them to the system database.
3.  **Control Tab (Application):** Dynamically displays controls (buttons, toggles) based on the discovered Matter clusters of the paired devices.

## 4. Development and Certification Notes (DIY / Hobby)

For private, DIY, and development purposes, **no official CSA (Connectivity Standards Alliance) certification is required.**

We utilize **Test Vendor IDs (VID)** (e.g., `0xFFF1`) and **Test Product IDs (PID)**.

### Ecosystem Behavior with Test Certificates
*   **Apple Home (iOS):** Apple allows pairing of uncertified Matter accessories. During the pairing process, iOS will display a warning ("This accessory is uncertified"). The user can simply click **"Add anyway"** to bypass this and use the device normally.
*   **Google Home:** Google requires explicit permission to add uncertified devices. Developers must log into the free **Google Home Developer Console** and register their Test VID and PID. Once registered, the Google Home app on the developer's smartphone will allow pairing without errors.
*   **Our ESP Controller:** The ESP Matter SDK will be configured to accept test certificates during the Attestation phase (PASE/CASE), ensuring seamless pairing without warnings.

---
*Documented to ensure a shared understanding of the IoT communication strategy within the project team.*
