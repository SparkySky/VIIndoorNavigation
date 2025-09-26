## Shared Contribution ##
Source.cpp - Yee Jia Hao and Khor Jia En
	- This file contain the main navigation logic and windows partitioning.
	- Based the sub-chapter 3.1 Overall Arcitecture 

## Individual Contribution ##
Following source code includes "Source Files/modules/CODENAME.cpp" and "Header Files/CODENAME.h":
1. ColourProcessing		- Khor Jia En
	- Colour processing module using CIELAB and perform boundary box drawing from ShapeAnalysis

2. ShapeAnalysis		- Yee Jia Hao
	- Shape analysis module to determine possible QR and thier position (Left, centre and right) in frame
	
3. GridRouteReader		- Long Lin Tean
	- To load the grid map and node mapping

4. Graph.cpp			- Long Lin Tean
	- A* pathfinding algorithm for the shortest path finding and route generation

5. Navigator.cpp		- Yee Jia Hao
	- Handles node-based pathfinding and mapping logic

6. MapVisualizer		- Yee Wen Xian
	- UI: Visualize the grid map, nodes and the route for the navigation.

7. QRDetector			- Yee Wen Xian
	- Processing and QR decoding.

8. Text2Speech			- Yee Jia Hao
	- A priority based narration system using SAPI

9. UIForVI				- Yee Jia Hao
	- Provide UI via feedback and and controller-based input, including a terminal-based menu system.

10. Trip Manager		- Yee Jia Hao
	- Manages navigation progress along a predefined path. (During Trip Navigation)


## Other Files ##
NodeMapping.txt - Long Lin Tean
RouteGrid.txt - Long Lin Tean
supp.cpp and .h 