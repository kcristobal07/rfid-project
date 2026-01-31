# RFID Point-of-Sale System (Uniqlo 2 Prototype)

**IoT & Backend Development for Self-Checkout System**
---
## Overview 
This project is a prototype point-of-sale system that replaces traditional barcode scanning with RFID-based self-checkout. 
It demonstrates real-time IoT data integration, backend programming, and cloud database communication.

The system consists of:
- **Backend (this repo):** Arduino/ESP32 code for RFID scanning, subtotal calculation, and communication with the cloud database.
- **Frontend (peer repo):** Web interface for displaying purchase data and analytics, running on a Flask server hosted on AWS EC2.

Frontend repo: [Link to peer’s repository](https://github.com/mhayescs19/uniqlo2)
---
## Technologies & Skills
- **Hardware:** ESP32 TTGO32, MFRC522 RFID Reader
- **Programming:** C++, Arduino IDE
- **Cloud Services:** AWS EC2, HTTP API integration
- **Database:** PostgreSQL (or SQLite — specify if needed)
- **IoT Communication:** Wi-Fi HTTP requests, device-to-server integration
- **Other Skills:** Subtotal calculation, UID filtering logic, hardware-software interfacing
---
## Features
- Scan RFID tags and retrieve product data (size, color, price) from a cloud database.
- Display real-time subtotal on ESP32 TTGO32 built-in display.
- Anti-collision logic ensures reliable scanning of multiple tags.
- Sends purchase data to frontend server for analytics (handled by peer repo).
- Fully Wi-Fi enabled IoT integration with cloud backend.
---
## System Architecture
![System Architecture]()

**Workflow:**
1. ESP32 TTGO32 reads RFID tag UIDs.
2. Device sends HTTP requests to AWS backend with tag info.
3. Backend queries database for product details.
4. Backend sends response to ESP32.
5. ESP32 displays subtotal and communicates purchase data to frontend server.
---
## My Contributions
- Wired and programmed ESP32 TTGO32 for RFID scanning and subtotal display.
- Implemented HTTP communication with AWS backend.
- Developed UID filtering and anti-collision logic.
- Tested and validated IoT device functionality with multiple RFID tags.
- Collaborated with peer for frontend integration and data consistency.
---
## Setup Instructions
1. Flash Arduino code to ESP32 TTGO32 using Arduino IDE.
2. Connect device to Wi-Fi.
3. Ensure backend database is set up on AWS and accessible.
4. Scan RFID tags to test real-time data retrieval.
---
## Skills Demonstrated
- Embedded Systems Programming
- IoT Device-Cloud Communication
- Backend Development (HTTP, APIs, DB)
- Hardware-Software Integration
- Collaborative Software Engineering
---
RFID tags (Hex Values): 
- Tag UID:  04 26 6C BA 74 1D 91
- Tag UID:  04 2E 6C BA 74 1D 91
- Tag UID:  04 1E 6C BA 74 1D 91
- Tag UID:  04 2A 6C BA 74 1D 91
- Tag UID:  04 22 6C BA 74 1D 91
