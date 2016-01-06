
#pragma once

static const float g_cubeVertices[] = {
	-1.f, -1.f, 1.0f,		// 0
	1.f, -1.f, 1.0f,  		// 1	
	1.f, 1.f, 1.0f,			// 2
	-1.f, 1.f, 1.0f,		// 3	
	-1.f, -1.f, -1.0f,		// 4
	1.f, -1.f, -1.0f,		// 5
	1.f, 1.f, -1.0f,		// 6
	-1.f, 1.f, -1.0f		// 7
};

static const unsigned short g_cubeIndices[] = { 
	0, 1, 2, // avant
	2, 3, 0,
	3, 2, 6, // haut
	6, 7, 3,
	7, 6, 5, // arriere
	5, 4, 7,
	1, 5, 6, // droite
	6, 2, 1,		
	4, 0, 3, // gauche
	3, 7, 4,
	4, 5, 1, // bas
	1, 0, 4
};