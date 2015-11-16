
#ifndef ESGI_COMMON_H
#define ESGI_COMMON_H

// Specifique a Windows
#if _WIN32
#define GLEW_STATIC 1
#include <GL/glew.h>
#include <gl/wglew.h>

#define FREEGLUT_LIB_PRAGMAS 0
#include "GL/freeglut.h"

#pragma comment(lib, "freeglut.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glew32s.lib")
#endif

#include "../common/EsgiShader.h"

#include "stb/stb_image.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
// extension glm : 
#include "glm/gtx/euler_angles.hpp"


// fonctions utilitaires ---

bool LoadAndCreateTextureRGBA(const char *filename, GLuint &texID);
bool LoadAndCreateCubeMap(const char* filesname[], GLuint &cubeMapID);

#endif // ESGI_COMMON_H