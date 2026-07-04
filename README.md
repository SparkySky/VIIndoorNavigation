## Overview
This repository contains the source code for an indoor navigation system designed for the visually impaired. Due to the ineffectiveness of GPS signals indoors and the high cost of alternative infrastructure-dependent solutions like RFID or Bluetooth beacons, this project proposes a low-cost, infrastructure-free, and computer-vision-based navigation system utilizing Quick Response (QR) codes.  

## Core Functionality
- QR-Based Localization: The system uses a multi-stage computer vision pipeline to identify and decode red-coloured QR codes placed on landmarks (e.g., room labels, directional boards [Turn Left to <List of destination>, Turn Right to <List of destination>]).  
- Grid Mapping: A digital grid map, preprocessed from building floor plans, represents walkable and blocked paths, allowing for precise location tracking.  
- Pathfinding: The system utilizes the $A^*$ pathfinding algorithm on a $100\times100$ matrix grid map to compute the optimal route from the user's current node to their chosen destination.  
- User Guidance: An aiding module detects the position of potential QR codes (left, centre, or right) relative to the camera frame, providing real-time audio feedback to keep the user on the correct path.  

## System Architecture
<img width="1070" height="715" alt="image" src="https://github.com/user-attachments/assets/ca9cf75b-73d2-4129-8c37-811d3f276d93" />

The system is built with a modular design to ensure performance, maintainability, and scalability. It operates in three primary phases:
- Preprocessing (Image Processing): Captures frames and uses CIELAB colour space processing to segment red regions, followed by shape analysis to isolate quadrilateral-shaped QR code candidates.
- Phase 1 (Initialization & Localization): Decodes QR codes to initiate localization, update the user’s current node on the grid map, and provide destination selection options.
- Phase 2 & 3 (Navigation, Guidance & Aiding): Computes and updates the shortest path using A* algo, while the Guidiance Module provides continuous directional feedback.

## Error Handling
The system includes error handling to manage unexpected situations gracefully, ensuring continuous support for the VI user:
- Path Deviation: If a user scans a QR code that is not part of the current path, the system performs an automatic reroute from the scanned node to the destination.
- Out-of-Sequence Scans: The system correctly localizes the user even if they scan a node out of the expected sequence.
- Invalid Inputs: Invalid QR codes that do not match the navigation data are rejected, with the system providing audio feedback to the user.  

## Technology Stack
Language: C++ (ISO C++17 Standard).  
Computer Vision: OpenCV (Version 4.11.0).  
Voice Synthesis: Microsoft Speech API (SAPI 5.4).  
Hardware: Laptop with camera/webcam and speaker, mouse for tactile input

## Source Code Location
Please note that the main source files are located within the `\Project4110` directory.
