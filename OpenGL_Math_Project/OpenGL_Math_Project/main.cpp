#define _USE_MATH_DEFINES

#include <cstdio>
#include <cmath>
#include <vector>
#include <string>
#include <iostream>

#include "../common/EsgiShader.h"
#include "Common.h"
#include "tinyobjloader/tiny_obj_loader.h"
#include "AntTweakBar.h"

#include "Quaternion.h"

TwBar* objTweakBar;

EsgiShader g_BasicShader;
EsgiShader g_SkyboxShader;

int previousTime = 0;

struct ViewProj
{
	// Les matrices
	glm::mat4 viewMatrix;
	glm::mat4 projectionMatrix;
	glm::mat4 rotationMatrix;

	// Position de la caméra
	glm::vec3 position;

	// Vecteurs orientation de la caméra
	glm::vec3 forward;
	glm::vec3 right;

	// Buffer
	GLuint UBO;
} g_Camera;

struct Object
{
	// Transform
	glm::vec3 position;
	glm::vec3 rotation;
	glm::mat4 worldMatrix;

	// Mesh
	GLuint VBO;
	GLuint IBO;
	GLuint ElementCount;
	GLenum PrimitiveType;
	GLuint VAO;

	// Material
	GLuint textureObj;

	// Champs divers
	bool autoRotate;
};

Object g_Object;
Object g_CubeMap;

float horizontalAngleCamera = 3.14f;				// Initial horizontal angle : toward -Z
float verticalAngleCamera = 0.0f;					// Initial vertical angle : none
float mouseSpeedCamera = 0.005f;
float mouseSpeedMoveObject = 0.05f;
float movementSpeed = 10.0f;

int oldX;
int oldY;
unsigned char keyState[255];
unsigned char mouseButtonsState[10];

void InitCubemap()
{
	static const float skyboxVertices[] = {
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f
	};

	static const char* skyboxFiles[] = {
		"skybox/right.jpg",
		"skybox/left.jpg",
		"skybox/top.jpg",
		"skybox/bottom.jpg",
		"skybox/back.jpg",
		"skybox/front.jpg",
	};

	LoadAndCreateCubeMap(skyboxFiles, g_CubeMap.textureObj);

	glGenVertexArrays(1, &g_CubeMap.VAO);
	glGenBuffers(1, &g_CubeMap.VBO);
	glBindVertexArray(g_CubeMap.VAO);
	glBindBuffer(GL_ARRAY_BUFFER, g_CubeMap.VBO);

	// RAPPEL: sizeof(skyboxVertices) fonctionne -cad qu'on obtient la taille totale du tableau-
	//			du fait que skyboxVertices est un tableau STATIC et donc que le compilateur peut deduire
	//			sa taille lors de la compilation. Autrement sizeof aurait du renvoyer la taille du pointeur.
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*) 0);
	glBindVertexArray(0);

	g_SkyboxShader.LoadVertexShader("skybox.vs");
	g_SkyboxShader.LoadFragmentShader("skybox.fs");
	g_SkyboxShader.Create();

	auto program = g_SkyboxShader.GetProgram();
	glUseProgram(program);

	// l'UBO binde en 0 sera egalement connecte au shader skybox
	auto blockIndex = glGetUniformBlockIndex(program, "ViewProj");
	glUniformBlockBinding(program, blockIndex, 0);
	auto cubemapIndex = glGetUniformLocation(program, "u_CubeMap");

	// pour le shader, la skybox utilisera l'unite de texture 0 mais il est possible d'utiliser 
	// un index specifique pour la cubemap
	glUniform1i(cubemapIndex, 0);
	glUseProgram(0);
}

void LoadOBJ(const std::string &inputFile)
{
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	std::string err = tinyobj::LoadObj(shapes, materials, inputFile.c_str());
	const std::vector<unsigned int>& indices = shapes[0].mesh.indices;
	const std::vector<float>& positions = shapes[0].mesh.positions;
	const std::vector<float>& normals = shapes[0].mesh.normals;
	const std::vector<float>& texcoords = shapes[0].mesh.texcoords;

	g_Object.ElementCount = indices.size();

	uint32_t stride = 0;

	if(positions.size())
	{
		stride += 3 * sizeof(float);
	}
	if(normals.size())
	{
		stride += 3 * sizeof(float);
	}
	if(texcoords.size())
	{
		stride += 2 * sizeof(float);
	}

	const auto count = positions.size() / 3;
	const auto totalSize = count * stride;

	glGenBuffers(1, &g_Object.IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_Object.IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), &indices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glGenBuffers(1, &g_Object.VBO);
	glBindBuffer(GL_ARRAY_BUFFER, g_Object.VBO);
	glBufferData(GL_ARRAY_BUFFER, totalSize, nullptr, GL_STATIC_DRAW);

	// glMapBuffer retourne un pointeur sur la zone memoire allouee par glBufferData 
	// du Buffer Object qui est actuellement actif - via glBindBuffer(<cible>, <id>)
	// il est imperatif d'appeler glUnmapBuffer() une fois que l'on a termine car le
	// driver peut tres bien etre amener a modifier l'emplacement memoire du BO.
	float* vertices = (float*) glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	for(auto index = 0; index < count; ++index)
	{
		if(positions.size())
		{
			memcpy(vertices, &positions[index * 3], 3 * sizeof(float));
			vertices += 3;
		}
		if(normals.size())
		{
			memcpy(vertices, &normals[index * 3], 3 * sizeof(float));
			vertices += 3;
		}
		if(texcoords.size())
		{
			memcpy(vertices, &texcoords[index * 2], 2 * sizeof(float));
			vertices += 2;
		}
	}
	glUnmapBuffer(GL_ARRAY_BUFFER);

	glGenVertexArrays(1, &g_Object.VAO);
	glBindVertexArray(g_Object.VAO);
	glBindBuffer(GL_ARRAY_BUFFER, g_Object.VBO);
	uint32_t offset = 3 * sizeof(float);
	glVertexAttribPointer(0, 3, GL_FLOAT, false, stride, nullptr);
	glEnableVertexAttribArray(0);

	if(normals.size())
	{
		glVertexAttribPointer(1, 3, GL_FLOAT, false, stride, (GLvoid *) offset);
		glEnableVertexAttribArray(1);
		offset += 3 * sizeof(float);
	}

	if(texcoords.size())
	{
		glVertexAttribPointer(2, 2, GL_FLOAT, false, stride, (GLvoid *) offset);
		glEnableVertexAttribArray(2);
		offset += 2 * sizeof(float);
	}

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	LoadAndCreateTextureRGBA(materials[0].diffuse_texname.c_str(), g_Object.textureObj);
}

void CleanObjet(Object& objet)
{
	if(objet.textureObj)
		glDeleteTextures(1, &objet.textureObj);
	if(objet.VAO)
		glDeleteVertexArrays(1, &objet.VAO);
	if(objet.VBO)
		glDeleteBuffers(1, &objet.VBO);
	if(objet.IBO)
		glDeleteBuffers(1, &objet.IBO);
}

// Initialisation et terminaison ---
static  void __stdcall ExitCallbackTw(void* clientData)
{
	glutLeaveMainLoop();
}

void Initialize()
{
	printf("Version Pilote OpenGL : %s\n", glGetString(GL_VERSION));
	printf("Type de GPU : %s\n", glGetString(GL_RENDERER));
	printf("Fabricant : %s\n", glGetString(GL_VENDOR));
	printf("Version GLSL : %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
	int numExtensions;
	glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);

	GLenum error = glewInit();
	if(error != GL_NO_ERROR)
	{
		exit(-1);
	}

#ifdef _WIN32
	// on coupe la synchro vertical pour voir l'effet du delta time
	wglSwapIntervalEXT(0);
#endif

	// render states par defaut
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	// Pour pas la transparence
	 glEnable(GL_CULL_FACE);
	// Pour la transparence
	// glDisable(GL_CULL_FACE);

	// Pour la transparence /////////////////////////////////////////////////////////////
	 glEnable(GL_BLEND);

	 //Ca
	 glBlendEquation(GL_FUNC_ADD);
	 glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Ou ca
	// glBlendFunc(GL_ZERO, GL_SRC_COLOR);
	////////////////////////////////////////////////////////////////////////////////////

	// AntTweakBar
	TwInit(TW_OPENGL, NULL); // ou TW_OPENGL_CORE selon le cas de figure
	objTweakBar = TwNewBar("OBJ Loader");
	TwAddSeparator(objTweakBar, "Camera", "");
	TwAddVarRW(objTweakBar, "Auto Rotate Object", TW_TYPE_BOOLCPP, &g_Object.autoRotate, "");
	TwAddSeparator(objTweakBar, "Objet", "");
	TwAddVarRW(objTweakBar, "Yaw", TW_TYPE_FLOAT, &g_Object.rotation.y, "");
	TwAddSeparator(objTweakBar, "...", "");
	TwAddButton(objTweakBar, "Quitter", &ExitCallbackTw, nullptr, "");
	//TwAddVarRW(objTweakBar, "Quaternion", TW_TYPE_QUAT4F, &g_Object.rotation, "Definition");

	// Objets OpenGL
	g_BasicShader.LoadVertexShader("basic.vs");
	g_BasicShader.LoadFragmentShader("basic.fs");
	g_BasicShader.Create();	

	glGenBuffers(1, &g_Camera.UBO);
	glBindBuffer(GL_UNIFORM_BUFFER, g_Camera.UBO);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 2, nullptr, GL_STREAM_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, g_Camera.UBO);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	auto program = g_BasicShader.GetProgram();

	auto blockIndex = glGetUniformBlockIndex(program, "ViewProj");
	glUniformBlockBinding(program, blockIndex, 0);

	// Setup
	previousTime = glutGet(GLUT_ELAPSED_TIME);

	const std::string inputFile = "rock.obj";
	LoadOBJ(inputFile);
	InitCubemap();

	// Init de la caméra
	g_Camera.position = glm::vec3(0.0f, 0.0f, 5.0f);
	g_Camera.forward = glm::vec3(0.0f, 0.0f, -1.0f);
	g_Camera.right = glm::vec3(1.0f, 0.0f, 0.0f);

	// Init du clavier
	for(int i = 0; i < 256; i++)
	{
		keyState[i] = GLUT_UP;
	}

	for(int i = 0; i < 10; i++)
	{
		mouseButtonsState[i] = GLUT_UP;
	}
}

void Terminate()
{
	glDeleteBuffers(1, &g_Camera.UBO);

	CleanObjet(g_Object);
	CleanObjet(g_CubeMap);

	g_BasicShader.Destroy();
	g_SkyboxShader.Destroy();

	TwTerminate();
}

// Boucle principale
void Resize(GLint width, GLint height)
{
	glViewport(0, 0, width, height);
	g_Camera.projectionMatrix = glm::perspectiveFov(45.f, (float) width, (float) height, 0.1f, 1000.f);
	TwWindowSize(width, height);
}

void Update()
{
	auto currentTime = glutGet(GLUT_ELAPSED_TIME);
	auto delta = currentTime - previousTime;
	previousTime = currentTime;
	auto elapsedTime = delta / 1000.0f;

	if(g_Object.autoRotate)
	{
		g_Object.rotation.y += 10.f * elapsedTime;
	}

	if(keyState['z'] == GLUT_DOWN)
	{
		g_Camera.position += elapsedTime* g_Camera.forward * movementSpeed;
	}
	else if(keyState['s'] == GLUT_DOWN)
	{
		g_Camera.position -= elapsedTime* g_Camera.forward * movementSpeed;
	}
	
	if(keyState['q'] == GLUT_DOWN)
	{
		g_Camera.position -= elapsedTime* g_Camera.right * movementSpeed;
	}
	else if(keyState['d'] == GLUT_DOWN)
	{
		g_Camera.position += elapsedTime* g_Camera.right * movementSpeed;
	}

	if(keyState['e'] == GLUT_DOWN || keyState[' '] == GLUT_DOWN)
	{
		g_Camera.position.y += elapsedTime * movementSpeed;
	}
	else if(keyState['a'] == GLUT_DOWN)
	{
		g_Camera.position.y -= elapsedTime * movementSpeed;
	}

	if(keyState[27] == GLUT_DOWN)
	{
		exit(0);
	}

	glutPostRedisplay();
}

void Render()
{
	auto width = glutGet(GLUT_WINDOW_WIDTH);
	auto height = glutGet(GLUT_WINDOW_HEIGHT);

	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// variables uniformes (constantes) 
	g_Camera.projectionMatrix = glm::perspectiveFov(45.f, (float) width, (float) height, 0.1f, 1000.f);
	glm::vec3 position = g_Camera.position;
	glm::vec3 direction = g_Camera.forward;
	g_Camera.viewMatrix = glm::lookAt(position, position + direction, glm::vec3(0.f, 1.f, 0.f));

	glBindBuffer(GL_UNIFORM_BUFFER, g_Camera.UBO);
	//glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 2, glm::value_ptr(g_Camera.viewMatrix), GL_STREAM_DRAW);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4) * 2, glm::value_ptr(g_Camera.viewMatrix));

	float yaw = glm::radians(g_Object.rotation.y);
	float pitch = glm::radians(g_Object.rotation.x);
	float roll = glm::radians(g_Object.rotation.z);
	g_Object.worldMatrix = glm::eulerAngleYXZ(yaw, pitch, roll);

	/////////////////////////////////////////////////////////////////////// Rendu des objets
	auto program = g_BasicShader.GetProgram();
	glUseProgram(program);

	auto worldLocation = glGetUniformLocation(program, "u_worldMatrix");

	glm::vec3 objectPosition = glm::vec3(0, 10, 0);

	auto offsetLocation = glGetUniformLocation(program, "u_offset");
	glUniform3f(offsetLocation, objectPosition.x, objectPosition.y, objectPosition.z);

	auto useTransparencyLocation = glGetUniformLocation(program, "u_useTransparency");
	glUniform1f(useTransparencyLocation, 0);

	glBindTexture(GL_TEXTURE_2D, g_Object.textureObj);

	glBindVertexArray(g_Object.VAO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_Object.IBO);

	glm::mat4 jefaisdestests = glm::translate(glm::mat4(1), objectPosition);
	jefaisdestests = jefaisdestests * glm::eulerAngleYXZ(yaw, pitch, roll);
	jefaisdestests = glm::translate(jefaisdestests, -objectPosition);

	glm::mat4& transform = jefaisdestests;
	glUniformMatrix4fv(worldLocation, 1, GL_FALSE, glm::value_ptr(transform));



	glDrawElements(GL_TRIANGLES, g_Object.ElementCount, GL_UNSIGNED_INT, 0);

	///////////////////////////////////////////////////////////////////////// Rendu d'un objet "repère" fixe transparent
	glUniform1f(useTransparencyLocation, 1);

	// On active/désactive des fonctions pour la transparence
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);

	//Ca
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Ou ca
	// glBlendFunc(GL_ZERO, GL_SRC_COLOR);

	objectPosition = glm::vec3(0, 0, 0);
	transform = g_Object.worldMatrix;
	glUniformMatrix4fv(worldLocation, 1, GL_FALSE, glm::value_ptr(transform));

	glUniform3f(offsetLocation, objectPosition.x, objectPosition.y, objectPosition.z);
	glDrawElements(GL_TRIANGLES, g_Object.ElementCount, GL_UNSIGNED_INT, 0);

	// On active/désactive des fonctions pour annuler la transparence
	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);

	//////////////////////////////////////////////////////////////////////// Dessin de la cubemap, de preference en dernier afin de limiter "l'overdraw"
	//////////////////////////////////////////////////////////////////////// Si on la dessine avant, on a un peu de transparence, mais moche
	glUseProgram(g_SkyboxShader.GetProgram());

	glBindTexture(GL_TEXTURE_CUBE_MAP, g_CubeMap.textureObj);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(g_CubeMap.VAO);

	// Tres important ! D'une part, comme la cubemap represente un environnement distant
	// il n'est pas utile d'ecrire dans le depth buffer (on est toujours au plus loin)
	// cependant il faut quand effectuer le test de profondeur (donc on n'a pas glDisable(GL_DEPTH_TEST)).
	// Neamoins il faut legerement changer l'operateur du test dans le cas ou 
	glDepthMask(GL_FALSE);
	glDepthFunc(GL_LEQUAL);
	glDrawArrays(GL_TRIANGLES, 0, 8 * 2 * 3);

	// on retabli ensuite les render states par defaut
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	glBindVertexArray(0);

	// dessine les tweakBar
	TwDraw();

	glutSwapBuffers();
}

void mouse(int button, int state, int x, int y)
{
	mouseButtonsState[button] = state;
	if(!TwEventMouseButtonGLUT(button, state, x, y))
	{
		if(state == GLUT_UP)
		{

		}
		else if(state == GLUT_DOWN)
		{
			oldX = x;
			oldY = y;
		}
	}
	glutPostRedisplay();
}

void motion(int x, int y)
{
	if(!TwEventMouseMotionGLUT(x, y))
	{
		int deltaX = oldX - x;
		int deltaY = oldY - y;

		int width = glutGet(GLUT_WINDOW_WIDTH);
		int height = glutGet(GLUT_WINDOW_HEIGHT);

		Quaternion qX(
			sin(((y) * M_PI) / height),
			0,
			0,
			cos(((y) * M_PI) / height)
			);
		Quaternion qY(
			0,
			sin(((x) * M_PI) / width),
			0,
			cos(((x) * M_PI) / width)
			);

		Quaternion rotation = qY * qX;

		// Rotation objet
		if(mouseButtonsState[GLUT_LEFT_BUTTON] == GLUT_DOWN)
		{
			// TODO: bof, quand l'objet est tourné, la rotation devient galère
			g_Object.rotation.x -= deltaY;
			g_Object.rotation.y -= deltaX;
		}
		// Rotation camera
		else if(mouseButtonsState[GLUT_RIGHT_BUTTON] == GLUT_DOWN)
		{
			horizontalAngleCamera += mouseSpeedCamera * deltaX;
			verticalAngleCamera += mouseSpeedCamera * deltaY;

			// Vecteur avant
			glm::vec3 forward = glm::vec3(
				cos(verticalAngleCamera) * sin(horizontalAngleCamera),
				sin(verticalAngleCamera),
				cos(verticalAngleCamera) * cos(horizontalAngleCamera)
				);

			// Vecteur droite
			glm::vec3 right = glm::vec3(
				sin(horizontalAngleCamera - M_PI / 2.0f),
				0,
				cos(horizontalAngleCamera - M_PI / 2.0f)
				);

			// Vecteur haut
			glm::vec3 up = glm::cross(g_Camera.forward, g_Camera.right);

			g_Camera.forward = forward;
			g_Camera.right = right;
			g_Camera.rotationMatrix = rotation.toRotationMatrix();
		}

		oldX = x;
		oldY = y;
	}

	glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y)
{
	keyState[key] = GLUT_DOWN;
}

void keyboardUp(unsigned char key, int x, int y)
{
	keyState[key] = GLUT_UP;
}

int main(int argc, char* argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(800, 600);
	glutCreateWindow("OBJ Loader");

#ifdef FREEGLUT
	// Note: glutSetOption n'est disponible qu'avec freeGLUT
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE,
				  GLUT_ACTION_GLUTMAINLOOP_RETURNS);
#endif
	Initialize();

	glutReshapeFunc(Resize);
	glutIdleFunc(Update);
	glutDisplayFunc(Render);

	// redirection pour AntTweakBar
	// dans le cas ou vous utiliseriez deja ces callbacks
	// il suffit d'appeler l'event d'AntTweakBar depuis votre fonction de rappel
	glutMouseFunc((GLUTmousebuttonfun) mouse);
	glutMotionFunc((GLUTmousemotionfun) motion);
	glutPassiveMotionFunc((GLUTmousemotionfun) TwEventMouseMotionGLUT);
	glutKeyboardFunc((GLUTkeyboardfun) keyboard);
	glutKeyboardUpFunc(keyboardUp);
	glutSpecialFunc((GLUTspecialfun) TwEventSpecialGLUT);
	TwGLUTModifiersFunc(glutGetModifiers);

	glutMainLoop();

	Terminate();

	return 0;
}