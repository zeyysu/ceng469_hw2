#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cstdio>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <string>
#include <map>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <GL/glew.h>   // The GL Header File
#include <GL/gl.h>   // The GL Header File
#include <GLFW/glfw3.h> // The GLFW header
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
// #include FT_FREETYPE_H

#define BUFFER_OFFSET(i) ((char*)NULL + (i))

using namespace std;

int width = 900, height = 600;

GLuint skyboxvao;
GLuint dynamicCMFBO;
//0=ground 1=statue 2=ctbody 3=ctwindows 4=cttires
GLuint vao[5];
GLuint gVertexAttribBuffer[5], gIndexBuffer[5];
int gVertexDataSizeInBytes[5], gNormalDataSizeInBytes[5];
unsigned int cubemapTexture, groundTexture, dynamiccubemaptexture;

//0=stcubemap 1=ground 2=statue 3=car
GLuint gProgram[4];
struct Vertex
{
	Vertex(GLfloat inX, GLfloat inY, GLfloat inZ) : x(inX), y(inY), z(inZ) { }
	GLfloat x, y, z;
};

struct Texture
{
	Texture(GLfloat inU, GLfloat inV) : u(inU), v(inV) { }
	GLfloat u, v;
};

struct Normal
{
	Normal(GLfloat inX, GLfloat inY, GLfloat inZ) : x(inX), y(inY), z(inZ) { }
	GLfloat x, y, z;
};

struct Face
{
	Face(int v[], int t[], int n[]) {
		vIndex[0] = v[0];
		vIndex[1] = v[1];
		vIndex[2] = v[2];
		tIndex[0] = t[0];
		tIndex[1] = t[1];
		tIndex[2] = t[2];
		nIndex[0] = n[0];
		nIndex[1] = n[1];
		nIndex[2] = n[2];
	}
	GLuint vIndex[3], tIndex[3], nIndex[3];
};

vector<Vertex> gVertices[5];
vector<Texture> gTextures[5];
vector<Normal> gNormals[5];
vector<Face> gFaces[5];

float carminX, carmaxX, carminZ, carmaxZ;
float cameraAngle = 0;
float carRotAngle = 0;
glm::vec3 carPos = glm::vec3(0,0.004,0);
float velocity = 0;
glm::vec3 lightPosition[] = { glm::vec3(-1,1,1), glm::vec3(1,1,1), glm::vec3(0, 1,-1) };

GLfloat skyboxVertices[] = {        
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

GLfloat groundTexCoords[] = {
    0.0f, 40.0f,
    40.0f, 40.0f,
    40.0f, 0.0f,
    0.0f, 0.0f
};

glm::vec3 statueColors[] = {
    glm::vec3(0.58, 0.49, 0.68),
    glm::vec3(0.82, 0.57, 0.74),
    glm::vec3(1, 0.79, 0.85),
    glm::vec3(1,0.87, 0.83),
    glm::vec3(1,0.7,0.74),
    glm::vec3(1,0.87,0.73),
    glm::vec3(1,1,0.73),
    glm::vec3(0.73,1,1)
};

void drawModel(int i)
{
	glBindVertexArray(vao[i]);
	glDrawElements(GL_TRIANGLES, gFaces[i].size() * 3, GL_UNSIGNED_INT, 0);
}

void displayCar(glm::mat4 &viewing, glm::mat4 &perspective, glm::vec3 eyePos){
    glUseProgram(gProgram[3]);
    glm::mat4 scale = glm::scale(glm::mat4(1.f), glm::vec3(0.003,0.003,0.003));
    glm::mat4 rotate = glm::rotate(glm::mat4(1.f), glm::radians(carRotAngle), glm::vec3(0, 1, 0));
    glm::mat4 translate = glm::translate(glm::mat4(1.f), carPos);
    glm::mat4 model = translate * rotate *  scale ;
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, dynamiccubemaptexture);

    glUniformMatrix4fv(glGetUniformLocation(gProgram[3], "viewingMatrix"), 1, GL_FALSE, glm::value_ptr(viewing));
    glUniformMatrix4fv(glGetUniformLocation(gProgram[3], "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(perspective));
    glUniformMatrix4fv(glGetUniformLocation(gProgram[3], "modelingMatrix"), 1, GL_FALSE, glm::value_ptr(model));
    glUniform3fv(glGetUniformLocation(gProgram[3],"lightPosition"),3, glm::value_ptr(lightPosition[0]));
    glUniform3fv(glGetUniformLocation(gProgram[3],"eyePos"),1, glm::value_ptr(eyePos));
    glUniform1i( glGetUniformLocation(gProgram[3], "sampler"), 0);
    glUniform3fv(glGetUniformLocation(gProgram[3],"color"),1, glm::value_ptr(glm::vec3(1,0.75,0.8)));
    glUniform1f(glGetUniformLocation(gProgram[3], "reflectFactor"), 0.1);
    drawModel(2);
    glUniform3fv(glGetUniformLocation(gProgram[3],"color"),1, glm::value_ptr(glm::vec3(0.75,1,0.8)));
    glUniform1f(glGetUniformLocation(gProgram[3], "reflectFactor"), 0.90);
    drawModel(3);
    glUniform3fv(glGetUniformLocation(gProgram[3],"color"),1, glm::value_ptr(glm::vec3(0.2,0.2,0.2)));
    glUniform1f(glGetUniformLocation(gProgram[3], "reflectFactor"), 0.f);
    drawModel(4);
}

void displayStatues(glm::mat4 &viewing, glm::mat4 &perspective, glm::vec3 eyePos) {
    glUseProgram(gProgram[2]);

    glm::mat4 translate = glm::translate(glm::mat4(1.f), glm::vec3(0.11,0.01,0.11));
    glm::mat4 scale = glm::scale(glm::mat4(1.f), glm::vec3(0.025,0.025,0.025));

    glUniformMatrix4fv(glGetUniformLocation(gProgram[2], "viewingMatrix"), 1, GL_FALSE, glm::value_ptr(viewing));
    glUniformMatrix4fv(glGetUniformLocation(gProgram[2], "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(perspective));
    glUniform3fv(glGetUniformLocation(gProgram[2],"lightPosition"),3, glm::value_ptr(lightPosition[0]));
    glUniform3fv(glGetUniformLocation(gProgram[2],"eyePos"),1, glm::value_ptr(eyePos));

    for(int i = 0; i < 8; i++){
        glm::mat4 rotate = glm::rotate(glm::mat4(1.f), glm::radians(i*45.f), glm::vec3(0,1,0));
        glm::mat4 model = rotate * translate * scale;
        glUniformMatrix4fv(glGetUniformLocation(gProgram[2], "modelingMatrix"), 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(glGetUniformLocation(gProgram[2],"color"),1, glm::value_ptr(statueColors[i]));
        drawModel(1);
    }
}

void displayground(glm::mat4 &viewing, glm::mat4 &perspective){
    glUseProgram(gProgram[1]);
    glm::mat4 rotate = glm::rotate(glm::mat4(1.f), glm::radians(90.f), glm::vec3(1, 0, 0));
    glm::mat4 model = rotate;

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, groundTexture);

    glUniformMatrix4fv(glGetUniformLocation(gProgram[1], "viewingMatrix"), 1, GL_FALSE, glm::value_ptr(viewing));
    glUniformMatrix4fv(glGetUniformLocation(gProgram[1], "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(perspective));
    glUniformMatrix4fv(glGetUniformLocation(gProgram[1], "modelingMatrix"), 1, GL_FALSE, glm::value_ptr(model));
    glUniform1i( glGetUniformLocation(gProgram[1], "texture0"), 0);

    drawModel(0);
}

void displayCM(glm::mat4 &viewing, glm::mat4 &perspective){
    glDepthFunc(GL_LEQUAL);
    glDisable(GL_DEPTH_TEST);
    glUseProgram(gProgram[0]);

    glBindVertexArray(skyboxvao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);

    glm::mat4 model = glm::mat4(1.0f);

    glUniformMatrix4fv(glGetUniformLocation(gProgram[0], "viewingMatrix"), 1, GL_FALSE, glm::value_ptr(viewing));
    glUniformMatrix4fv(glGetUniformLocation(gProgram[0], "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(perspective));
    glUniformMatrix4fv(glGetUniformLocation(gProgram[0], "modelingMatrix"), 1, GL_FALSE, glm::value_ptr(model));
    glUniform1i( glGetUniformLocation(gProgram[0], "sampler"), 0);
    
    // glBindSampler(0, sampler); 

	glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    
}

void createDynamicCM(){
    glViewport(0,0,1200,1200);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dynamicCMFBO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, dynamiccubemaptexture);
    for(int i = 0 ; i < 6 ; i++){
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
        GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, dynamiccubemaptexture, 0);

        glm::vec4 eyePos = glm::vec4(0,0,0,1);
        glm::vec3 lookAt = glm::vec3(i==0?1:i==1?-1:0, i==2?1:i==3?-1:0,i==4?1:i==5?-1:0);
        glm::vec3 up = glm::vec3(i==2?-1:i==3?1:0,(i==3||i==2)?0:-1,0);
        glm::mat4 viewing = glm::lookAt(glm::vec3(eyePos), glm::vec3(lookAt), up);
        glm::mat4 perspective = glm::perspective(90.0f, 1.f, 0.001f, 100.0f);
        displayCM(viewing, perspective);
        glm::mat4 camX = glm::translate(glm::mat4(1.f), (carPos));
        eyePos = camX * eyePos;
        viewing = glm::lookAt(glm::vec3(eyePos), glm::vec3(lookAt),up);
        displayground(viewing, perspective);
        displayStatues(viewing, perspective, glm::vec3(eyePos));
    }
}

void display(){

    glClearColor(0, 0, 0, 1);
    glClearDepth(1.0f);
    glClearStencil(0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glm::mat4 carRotMat = glm::rotate(glm::mat4(1.f),glm::radians(carRotAngle), glm::vec3(0,1,0));
    glm::vec4 carX = carRotMat * glm::vec4(0,0,1,0);
    carPos = carPos + glm::vec3(velocity * carX);

    //dynamic CM,
    createDynamicCM();
   // glTextureBarrier();

    glViewport(0,0,width,height);

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glClearColor(0, 0, 0, 1);
    glClearDepth(1.0f);
    glClearStencil(0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glm::mat4 camRotMat =  glm::rotate(glm::mat4(1.f), glm::radians(cameraAngle), glm::vec3(0, 1, 0));
    glm::vec4 eyePos = carRotMat * camRotMat * glm::vec4(0,0,0,1); 
    glm::vec4 lookAt = carRotMat * camRotMat * glm::vec4(0,0,1,1);
    glm::mat4 viewing = glm::lookAt(glm::vec3(eyePos), glm::vec3(lookAt), glm::vec3(0,1,0));
    glm::mat4 perspective = glm::perspective(45.0f, (GLfloat)width / (GLfloat)height, 0.001f, 100.0f);
    displayCM(viewing, perspective);
    glm::mat4 camX = glm::translate(glm::mat4(1.f), carPos);
    eyePos = carRotMat * camRotMat * glm::vec4(0,0.005,-0.03,1); 
    lookAt = carRotMat * camRotMat * glm::vec4(0,0,0.03,1);
    eyePos = camX * eyePos;
    lookAt = camX * lookAt;
    viewing = glm::lookAt(glm::vec3(eyePos), glm::vec3(lookAt), glm::vec3(0,1,0));

    displayground(viewing, perspective);
    displayStatues(viewing, perspective, glm::vec3(eyePos));
    displayCar(viewing, perspective, glm::vec3(eyePos));
    
}

void loadGroundTexture(){
    glGenTextures(1, &groundTexture);
    int width, height, nrComponents;
    unsigned char *data = stbi_load("ground_texture_sand.jpg", &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, groundTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: ground_texture_sand.jpg" << std::endl;
        stbi_image_free(data);
    }
    GLuint uvCoord;
    glBindVertexArray(vao[0]);
    glGenBuffers(1, &uvCoord);
    glBindBuffer(GL_ARRAY_BUFFER,uvCoord);
    glBufferData(GL_ARRAY_BUFFER, sizeof(groundTexCoords), &groundTexCoords, GL_STATIC_DRAW);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, NULL);
}

void createCubeTexture()
{
    glGenFramebuffers(1, &dynamicCMFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dynamicCMFBO);
    glGenTextures(1, &dynamiccubemaptexture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, dynamiccubemaptexture);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    for(int i = 0; i < 6; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA8,
        1200, 1200, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

void loadCubemap(){

    glGenTextures(1, &cubemapTexture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);

    const char* images[] = {"skybox_texture_abandoned_village/right.png",
        "skybox_texture_abandoned_village/left.png",
        "skybox_texture_abandoned_village/top.png",
        "skybox_texture_abandoned_village/bottom.png",
        "skybox_texture_abandoned_village/front.png",
        "skybox_texture_abandoned_village/back.png"
    };

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < 6; i++)
    {
        unsigned char *data = stbi_load(images[i], &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 
                         0,  GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
            );
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap tex failed to load at path: " << images[i] << std::endl;
            stbi_image_free(data);
        }
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

}

bool ReadDataFromFile(
    const string& fileName, ///< [in]  Name of the shader file
    string&       data)     ///< [out] The contents of the file
{
    fstream myfile;

    // Open the input 
    myfile.open(fileName.c_str(), std::ios::in);

    if (myfile.is_open())
    {
        string curLine;

        while (getline(myfile, curLine))
        {
            data += curLine;
            if (!myfile.eof())
            {
                data += "\n";
            }
        }

        myfile.close();
    }
    else
    {
        return false;
    }

    return true;
}

void createVS(GLuint& program, const string& filename)
{
    string shaderSource;

    if (!ReadDataFromFile(filename, shaderSource))
    {
        cout << "Cannot find file name: " + filename << endl;
        exit(-1);
    }

    GLint length = shaderSource.length();
    const GLchar* shader = (const GLchar*) shaderSource.c_str();

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &shader, &length);
    glCompileShader(vs);

    char output[1024] = {0};
    glGetShaderInfoLog(vs, 1024, &length, output);
    // printf("VS compile log: %s\n", output);

    glAttachShader(program, vs);
}

void createFS(GLuint& program, const string& filename)
{
    string shaderSource;

    if (!ReadDataFromFile(filename, shaderSource))
    {
        cout << "Cannot find file name: " + filename << endl;
        exit(-1);
    }

    GLint length = shaderSource.length();
    const GLchar* shader = (const GLchar*) shaderSource.c_str();

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &shader, &length);
    glCompileShader(fs);

    char output[1024] = {0};
    glGetShaderInfoLog(fs, 1024, &length, output);
    // printf("FS compile log: %s\n", output);

    glAttachShader(program, fs);
}

void initShaders()
{
    gProgram[0] = glCreateProgram();
    gProgram[1] = glCreateProgram();
    gProgram[2] = glCreateProgram();
    gProgram[3] = glCreateProgram();

    createVS(gProgram[0], "stcmvert.glsl");
    createFS(gProgram[0], "stcmfrag.glsl");

    createVS(gProgram[1], "groundvert.glsl");
    createFS(gProgram[1], "groundfrag.glsl");

    createVS(gProgram[2], "statuevert.glsl");
    createFS(gProgram[2], "statuefrag.glsl");

    createVS(gProgram[3], "carvert.glsl");
    createFS(gProgram[3], "carfrag.glsl");
   

    glLinkProgram(gProgram[0]);
    glLinkProgram(gProgram[1]);
    glLinkProgram(gProgram[2]);
    glLinkProgram(gProgram[3]);
    glUseProgram(gProgram[0]);
}

void initSkyboxVBO(){
    GLuint skyboxvbo;
    glGenVertexArrays(1, &skyboxvao);
    assert(skyboxvao > 0);
    glBindVertexArray(skyboxvao);

    assert(glGetError() == GL_NONE);
    glGenBuffers(1, &skyboxvbo);
    assert(skyboxvbo > 0);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxvbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
}

void initVBOs()
{
    for(int i=0; i<5; i++){
    glGenVertexArrays(1, &vao[i]);
    assert(vao[i] > 0);
    glBindVertexArray(vao[i]);
    // cout << "vao = " << vao << endl;

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	assert(glGetError() == GL_NONE);

	glGenBuffers(1, &gVertexAttribBuffer[i]);
	glGenBuffers(1, &gIndexBuffer[i]);

	assert(gVertexAttribBuffer[i] > 0 && gIndexBuffer[i] > 0);

	glBindBuffer(GL_ARRAY_BUFFER, gVertexAttribBuffer[i]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIndexBuffer[i]);
    gVertexDataSizeInBytes[i] = gVertices[i].size() * 3 * sizeof(GLfloat);
	gNormalDataSizeInBytes[i] = gNormals[i].size() * 3 * sizeof(GLfloat);
	int indexDataSizeInBytes = gFaces[i].size() * 3 * sizeof(GLuint);
	GLfloat* vertexData = new GLfloat[gVertices[i].size() * 3];
	GLfloat* normalData = new GLfloat[gNormals[i].size() * 3];
	GLuint* indexData = new GLuint[gFaces[i].size() * 3];

	float minX = 1e6, maxX = -1e6;
	float minY = 1e6, maxY = -1e6;
	float minZ = 1e6, maxZ = -1e6;

	for (int k = 0; k < gVertices[i].size(); ++k)
	{
		vertexData[3 * k] = gVertices[i][k].x;
		vertexData[3 * k + 1] = gVertices[i][k].y;
		vertexData[3 * k + 2] = gVertices[i][k].z;

		minX = std::min(minX, gVertices[i][k].x);
		maxX = std::max(maxX, gVertices[i][k].x);
		minY = std::min(minY, gVertices[i][k].y);
		maxY = std::max(maxY, gVertices[i][k].y);
		minZ = std::min(minZ, gVertices[i][k].z);
		maxZ = std::max(maxZ, gVertices[i][k].z);
	}

    if(i == 2 || i==3 || i==4){
        carminX = std::min(carminX, minX);
        carminZ = std::min(carminZ, minZ);
        carmaxX = std::max(carmaxX, maxX);
        carmaxZ = std::max(carmaxZ, maxZ);
    }

	for (int k = 0; k < gNormals[i].size(); ++k)
	{
		normalData[3 * k] = gNormals[i][k].x;
		normalData[3 * k + 1] = gNormals[i][k].y;
		normalData[3 * k + 2] = gNormals[i][k].z;
	}

	for (int k = 0; k < gFaces[i].size(); ++k)
	{
		indexData[3 * k] = gFaces[i][k].vIndex[0];
		indexData[3 * k + 1] = gFaces[i][k].vIndex[1];
		indexData[3 * k + 2] = gFaces[i][k].vIndex[2];
	}


	glBufferData(GL_ARRAY_BUFFER, gVertexDataSizeInBytes[i] + gNormalDataSizeInBytes[i], 0, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, gVertexDataSizeInBytes[i], vertexData);
	glBufferSubData(GL_ARRAY_BUFFER, gVertexDataSizeInBytes[i], gNormalDataSizeInBytes[i], normalData);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexDataSizeInBytes, indexData, GL_STATIC_DRAW);

	// done copying; can free now
	delete[] vertexData;
	delete[] normalData;
	delete[] indexData;

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(gVertexDataSizeInBytes[i]));
    }
}

bool ParseObj(const string& fileName, unsigned int i)
{
    fstream myfile;

    // Open the input 
    myfile.open(fileName.c_str(), std::ios::in);

    if (myfile.is_open())
    {
        string curLine;

        while (getline(myfile, curLine))
        {
            stringstream str(curLine);
            GLfloat c1, c2, c3;
            GLuint index[9];
            string tmp;

            if (curLine.length() >= 2)
            {
                if (curLine[0] == '#') // comment
                {
                    continue;
                }
                else if (curLine[0] == 'v')
                {
                    if (curLine[1] == 't') // texture
                    {
                        str >> tmp; // consume "vt"
                        str >> c1 >> c2;
                        gTextures[i].push_back(Texture(c1, c2));
                    }
                    else if (curLine[1] == 'n') // normal
                    {
                        str >> tmp; // consume "vn"
                        str >> c1 >> c2 >> c3;
                        gNormals[i].push_back(Normal(c1, c2, c3));
                    }
                    else // vertex
                    {
                        str >> tmp; // consume "v"
                        str >> c1 >> c2 >> c3;
                        gVertices[i].push_back(Vertex(c1, c2, c3));
                    }
                }
                else if (curLine[0] == 'f') // face
                {
                    str >> tmp; // consume "f"
					char c;
					int vIndex[3],  nIndex[3], tIndex[3];
					str >> vIndex[0]; str >> c >> c; // consume "//"
					str >> nIndex[0]; 
					str >> vIndex[1]; str >> c >> c; // consume "//"
					str >> nIndex[1]; 
					str >> vIndex[2]; str >> c >> c; // consume "//"
					str >> nIndex[2]; 

					assert(vIndex[0] == nIndex[0] &&
						   vIndex[1] == nIndex[1] &&
						   vIndex[2] == nIndex[2]); // a limitation for now

					// make indices start from 0
					for (int c = 0; c < 3; ++c)
					{
						vIndex[c] -= 1;
						nIndex[c] -= 1;
						tIndex[c] -= 1;
					}

                    gFaces[i].push_back(Face(vIndex, tIndex, nIndex));
                }
                else
                {
                    cout << "Ignoring unidentified line in obj file: " << curLine << endl;
                }
            }

            //data += curLine;
            if (!myfile.eof())
            {
                //data += "\n";
            }
        }

        myfile.close();
    }
    else
    {
        return false;
    }

	assert(gVertices[i].size() == gNormals[i].size());

    return true;
}


void init() 
{

    ParseObj("obj/ground.obj",0);
    ParseObj("obj/bunny.obj", 1);
    // ParseObj("obj/cube.obj",2);
    ParseObj("obj/cybertruck/cybertruck_body.obj",2);
    ParseObj("obj/cybertruck/cybertruck_windows.obj",3);
    ParseObj("obj/cybertruck/cybertruck_tires.obj",4);
    glEnable(GL_DEPTH_TEST);
    initSkyboxVBO();
    initVBOs();
    loadCubemap();
    loadGroundTexture();
    initShaders();
    createCubeTexture();
}


void reshape(GLFWwindow* window, int w, int h)
{
    w = w < 1 ? 1 : w;
    h = h < 1 ? 1 : h;

    width = w;
    height = h;
    glViewport(0, 0, w, h);
}


void mainLoop(GLFWwindow* window)
{
    while (!glfwWindowShouldClose(window))
    {
        display();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
    if (key == GLFW_KEY_Q && action == GLFW_PRESS)
    {
        cameraAngle = -90.f;
    }
    if (key == GLFW_KEY_E && action == GLFW_PRESS)
    {
        cameraAngle = 90.f;
    }
    if (key == GLFW_KEY_R && action == GLFW_PRESS)
    {
        cameraAngle = 0.f;
    }
    if (key == GLFW_KEY_T && action == GLFW_PRESS)
    {
        cameraAngle = 180.f;
    }
    if (key == GLFW_KEY_A && action == GLFW_PRESS)
    {
        carRotAngle += 2;
    }
    if (key == GLFW_KEY_D && action == GLFW_PRESS)
    {
        carRotAngle -= 2;
    }
    if (key == GLFW_KEY_W && action == GLFW_PRESS)
    {
        velocity += 0.0001;
    }
    if (key == GLFW_KEY_S && action == GLFW_PRESS)
    {
       velocity -= 0.0001;
    }
}

int main(int argc, char** argv)   // Create Main Function For Bringing It All Together
{
    if(argc!=1){
        cout<<"usage: ./main"<<endl;
        exit(1);
    }

    GLFWwindow* window;
    if (!glfwInit())
    {
        exit(-1);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    #ifdef __APPLE__
      glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif
    window = glfwCreateWindow(width, height, "Simple Example", NULL, NULL);

    if (!window)
    {
        glfwTerminate();
        exit(-1);
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

     if (GLEW_OK != glewInit())
    {
        std::cout << "Failed to initialize GLEW" << std::endl;
        return EXIT_FAILURE;
    }

    // char rendererInfo[512] = {0};
    // strcpy(rendererInfo, (const char*) glGetString(GL_RENDERER));
    // strcat(rendererInfo, " - ");
    // strcat(rendererInfo, (const char*) glGetString(GL_VERSION));
    char title[] = "CENG469_HW2";
    glfwSetWindowTitle(window, title);

    init();

    glfwSetKeyCallback(window, keyboard);

    glfwSetWindowSizeCallback(window, reshape);

    reshape(window, width, height); // need to call this once ourselves
    mainLoop(window); // this does not return unless the window is closed

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}