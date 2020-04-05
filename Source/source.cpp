//
// COMP 371 GROUP PROJECT
//
// Created by Matthew Salaciak 29644490.
//
// Inspired by the COMP 371 Lectures and Lab 2,3 and 4 and the following tutorials
// - https://learnopengl.com/Getting-started/Hello-Triangle
// - https://learnopengl.com/Advanced-OpenGL/Advanced-GLSL (for shader class)
// - http://www.opengl-tutorial.org/beginners-tutorials/tutorial-3-matrices/

//SimeplexNoise is a library https://github.com/SRombauts/SimplexNoise
//It created by Sébastien Rombauts



#include <iostream>
#include <list>
#define GLEW_STATIC 1
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/common.hpp>
#include "vertices.h"
#include "Shader.h"
#include <time.h>
#include <vector>
#include "texture-loader.h"
#include "SimplexNoise.h"



//define namespaces for glm and c++ std
using namespace glm;
using namespace std;

//global variables and functions for the project
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void renderLight(const GLuint &lamp_Shader);
float Remap (float value, float from1, float to1, float from2, float to2);


//this is not used right now
bool textureOn = true;
bool shadowsOn = true;
vec3 lightpos (0.f, 30.0f,0.f);



//worldspace matrix

mat4 projectionMatrix = mat4(1.0f);
mat4 viewMatrix;
mat4 modelMatrix = mat4(1.0f);
mat4 modelViewProjection;


mat4 WorldTransformMatrix(1.f);
//textures

GLuint depthMap;
vector<GLuint> VAO(100);

//primatative rendering options
int primativeRender = GL_LINES;


//camera info
vec3 cameraPosition(0.0f,56.0f,30.0f);
vec3 cameraLookAt(0.0f, 0.0f, 0.0f);
vec3 cameraUp(0.0f, 1.0f, 0.0f);

//vectors that hold geomtry information




void renderTerrain(vector <GLuint> &VAO , const GLuint &shader,  int &nIndices,vec3 &cameraPosition);
void createTerrianGeometry(GLuint &VAO, int &xOffset, int &yOffset,GLuint &shader);




//params to start

int renderDistance = 10;
int xMapChunks = 10;
int yMapChunks = 10;
int mapX = 64;
int mapY = 64;



//noise options (would love to make this user definable with a GUI ...a boy can dream

int octaves = 5;
float meshHeight = 25;  // Vertical scaling
float noiseScale = 64;  // Horizontal scaling
float persistence = 0.5;
float lacunarity = 2;





int main(int argc, char*argv[])
{
    
    //random number initialization and int primativeRender is used to store the default rendering option which is GL_TRIANGLES
    srand (time(NULL));

    float  fovAngle = 45.0f;

    
    // Initialize GLFW and OpenGL version
   
    glfwInit();
 
#if defined(PLATFORM_OSX)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#else
    // On windows, we set OpenGL version to 2.1, to support more hardware
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
#endif

    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    int screenWidth, screenHeight;

    // Create Window and rendering context using GLFW, resolution is 1024x768
    GLFWwindow* window = glfwCreateWindow(1024, 768, "A1_29644490", NULL, NULL);
    if (window == NULL)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);
    

    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to create GLEW" << std::endl;
        glfwTerminate();
        return -1;
    }
    
    glfwGetFramebufferSize(window, &screenWidth, &screenHeight);
     glViewport(0, 0, screenWidth, screenHeight);



    
   
//    GLuint lamp_Shader = Shader("/Users/matthew/Documents/school/WINTER 2020/COMP 371/371_PROJECT/Source/lampShader.vs","/Users/matthew/Documents/school/WINTER 2020/COMP 371/371_PROJECT/Source/lampShader.fs");
       
    
    //texture shader for grid, olaf
    
       GLuint textureShader = Shader("/Users/matthew/Documents/school/WINTER 2020/COMP 371/371_PROJECT/Source/shader-texture.vs","/Users/matthew/Documents/school/WINTER 2020/COMP 371/371_PROJECT/Source/shader-texture.fs");
    
    
    //shader for simple shadows
    GLuint simpleShadow = Shader("/Users/matthew/Documents/school/WINTER 2020/COMP 371/assignments/A1_29644490/Assignment1_Framework/Source/simple-shadow-shader.vs", "/Users/matthew/Documents/school/WINTER 2020/COMP 371/assignments/A1_29644490/Assignment1_Framework/Source/simple-shadow-shader.fs");


    
    
    
    
    
    
    

            // Other OpenGL states to set once
            // Enable Backface culling
            glEnable(GL_CULL_FACE);
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LESS);

//  glEnable(GL_FRAMEBUFFER_SRGB);
        
            // For frame time
            float lastFrameTime = glfwGetTime();
            double lastMousePosX, lastMousePosY;
            glfwGetCursorPos(window, &lastMousePosX, &lastMousePosY);
        
            //camera information for mouse implementation
            float cameraSpeed = 0.5f;
            float cameraFastSpeed = 2 * cameraSpeed;
            float cameraHorizontalAngle = -34.0f;
            float cameraVerticalAngle = 0.0f;
    
    
    
    //shadow depth map
    const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
        unsigned int depthMapFBO;
        glGenFramebuffers(1, &depthMapFBO);
        // create depth texture

        glGenTextures(1, &depthMap);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
        // attach depth texture as FBO's depth buffer
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    
    
    
    
    //textures
            glUseProgram(textureShader);
            glUniform1i(glGetUniformLocation(textureShader, "shadowMap"), 0);
  
    


    
    
      int nIndices = mapX * mapY * 6;
      
    for (int y = 0; y < yMapChunks; y++)
                     for (int x = 0; x < xMapChunks; x++) {
                         createTerrianGeometry(VAO[x + y*xMapChunks], x, y,textureShader);

                     }
    

        
        // Entering Main Loop
        while(!glfwWindowShouldClose(window))
        {
      
            float dt = glfwGetTime() - lastFrameTime;
            lastFrameTime += dt;
            bool fastCam = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;
            float currentCameraSpeed = (fastCam) ? cameraFastSpeed : cameraSpeed;
   

            
            projectionMatrix = perspective(radians(fovAngle),1024.0f / 768.0f, 0.1f,300.0f);
             viewMatrix = lookAt(cameraPosition, cameraPosition + cameraLookAt,cameraUp );
            viewMatrix = viewMatrix * WorldTransformMatrix;

            //setting up the MVP of the world so I can place our objects within
             modelViewProjection = projectionMatrix * viewMatrix * modelMatrix;
 
            // set the background color to the greenish grey
            glClearColor(0.68f, 0.87f, 0.95f,1.0f);
    
            // clear the color and depth buffer at the beginning of each loop
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            
    


            //texture shader
            glUseProgram(textureShader);

            //set view and projection materix in shader
            GLuint viewMatrix_texture = glGetUniformLocation(textureShader, "view");
            GLuint projectionMatrix_texture = glGetUniformLocation(textureShader, "projection");
            glUniformMatrix4fv(viewMatrix_texture, 1, GL_FALSE, &viewMatrix[0][0]);
            glUniformMatrix4fv(projectionMatrix_texture, 1, GL_FALSE, &projectionMatrix[0][0]);


//            //set light and view position
            GLuint lightPositionTexture = glGetUniformLocation(textureShader, "lightPos");
            GLuint viewPositionTexture = glGetUniformLocation(textureShader, "viewPos");
            glUniform3f(lightPositionTexture, lightpos.x,lightpos.y,lightpos.z);
            glUniform3f(viewPositionTexture, cameraPosition.x,cameraPosition.y,cameraPosition.z);

            //set lightColor for textureShader
            GLuint lightColor = glGetUniformLocation(textureShader, "lightColor");
            glUniform3f(lightColor, 1.0f,1.0f,1.0f);
            
            mat4 lightProjection, lightView , lightSpaceMatrix;

                    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
                    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
                    glClear(GL_DEPTH_BUFFER_BIT);


                     //this part is largely inspired by learnopengl's shadow tutorial and lab 8
                    //render shadows from lights perspective
                      float near_plane = 1.0f, far_plane = 100.0f;

            
                      lightProjection = glm::perspective(glm::radians(130.0f), (GLfloat)SHADOW_WIDTH / (GLfloat)SHADOW_HEIGHT, near_plane, far_plane);
                      

                      lightView = glm::lookAt(lightpos, glm::vec3(0.0f), glm::vec3(0.0, 0.0, 1.0));
                      lightSpaceMatrix = lightProjection * lightView;
                      // render scene from light's point of view
                      glUseProgram(simpleShadow);
                      GLuint lightSpaceMatrixSimple = glGetUniformLocation(simpleShadow, "lightSpaceMatrix");
                      glUniformMatrix4fv(lightSpaceMatrixSimple, 1, GL_FALSE, &lightSpaceMatrix[0][0]);

                      renderTerrain(VAO,simpleShadow, nIndices, cameraPosition);

                      glBindFramebuffer(GL_FRAMEBUFFER, 0);
                      // reset viewport
                      glViewport(0, 0, 1024, 768);
                      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


                      glViewport(0, 0, 1024, 768);
                      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                       //use textureshader to render the scene, and set the  boolean in the fragment shader to render wit shadows, if shadows aren't enabled then do it without
                    glUseProgram(textureShader);
                    glUniform1ui(glGetUniformLocation(textureShader, "shadowsOn"), 1);
                    GLuint lightSpaceMatrixShader = glGetUniformLocation(textureShader, "lightSpaceMatrix");
                    glUniformMatrix4fv(lightSpaceMatrixShader, 1, GL_FALSE, &lightSpaceMatrix[0][0]);
            

            

            renderTerrain(VAO,textureShader, nIndices, cameraPosition);
            

        
            
            
                      

            // End Frame
            glfwSwapBuffers(window);
            glfwPollEvents();
            
            
            //this part here which controls the camera via the mouse X,Y inputs
            //it is edited and adapted from my solution to lab 4
            double mousePosX, mousePosY;
            glfwGetCursorPos(window, &mousePosX, &mousePosY);
                       
            double dx = mousePosX - lastMousePosX;
            double dy = mousePosY - lastMousePosY;
                       
            lastMousePosX = mousePosX;
            lastMousePosY = mousePosY;
                       
            // Convert to spherical coordinates
            const float cameraAngularSpeed = 50.0f;
            cameraHorizontalAngle -= dx * cameraAngularSpeed * dt;
            cameraVerticalAngle   -= dy * cameraAngularSpeed * dt;
                       
         
            cameraVerticalAngle = std::max(-90.0f, std::min(90.0f, cameraVerticalAngle));
            if (cameraHorizontalAngle > 360)
                {
                    cameraHorizontalAngle -= 360;
                }
            else if (cameraHorizontalAngle < -360)
                {
                    cameraHorizontalAngle += 360;
                }
                       
            float theta = radians(cameraHorizontalAngle);
            float phi = radians(cameraVerticalAngle);
                       
            cameraLookAt = vec3(cosf(phi)*cosf(theta), sinf(phi), -cosf(phi)*sinf(theta));
            vec3 cameraSideVector = cross(cameraLookAt, vec3(0.0f, 1.0f, 0.0f));
            normalize(cameraSideVector);

      
            
            //these are the following keybindings to control the olaf, the camera and the world orientation, textures, lighting and shadows
            
            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) // camera zoom in
                          {
                              cameraPosition.z -= currentCameraSpeed * dt*40;
             
                              
                          }
                                            
                      if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) // camera zoom out
                          {
                              cameraPosition.z += currentCameraSpeed * dt*40;
                           
                          }
            
            
            
            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS ) // move camera to the left
                {
                    cameraPosition.x -= currentCameraSpeed * dt*40;

                    
                }

            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) // move camera to the right
                {
                    cameraPosition.x += currentCameraSpeed * dt*40;
         
                }
            
            if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) // camera zoom in
                {
                    fovAngle = fovAngle  - 0.1f;
                }
                                  
            if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) // camera zoom out
                {
                    fovAngle = fovAngle + 0.1f;
                }

            if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) // move camera down
                {
                    cameraPosition.y -= currentCameraSpeed * dt*40;
         
                }

            if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) // move camera up
                {
                    cameraPosition.y += currentCameraSpeed * dt*40;
               
                }
           
  
            
            
    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) // primative change to trianges
                {
                    primativeRender = GL_TRIANGLES;
                }
            
            if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) // primative change to points
                {
                    primativeRender = GL_POINTS;
                }
            
            if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) // primative change to lines
                {
                primativeRender = GL_LINES;
                }
      
            
            
            
            if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) // rotate X axis in the other orientation of the world
                     {
                                                        
                float x =5.01;
                float y=5.01;
                float z=5.01;
                x+=0.01;
                y+=0.01;
                z+=0.01;
                       
                         WorldTransformMatrix = WorldTransformMatrix * rotate(mat4(1.0f), radians(x),vec3(-1.0f,0.f, 0.f));

                               
                     }
            
            if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) // rotate X axis in the other orientation of the world
                             {
                                                                
                float x =5.01;
                float y=5.01;
                float z=5.01;
                x+=0.01;
                y+=0.01;
                z+=0.01;
                 WorldTransformMatrix = WorldTransformMatrix * rotate(mat4(1.0f), radians(x),vec3(-1.0f,0.f, 0.f));
                             
            

                                     
                             }
                                        
            
            
            
            if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) //rotate Y axis of the world
                             {
                                                                
                float x =5.01;
                float y=5.01;
                float z=5.01;
                x+=0.01;
                y+=0.01;
                z+=0.01;
                WorldTransformMatrix = WorldTransformMatrix * rotate(mat4(1.0f), radians(x),vec3(0.0f,1.f, 0.f));
                             }
            
            
            if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) // rotate Y axis of the world in the other orientation
                             {
                                                                
                float x =5.01;
                float y=5.01;
                float z=5.01;
                x+=0.01;
                y+=0.01;
                z+=0.01;
                WorldTransformMatrix = WorldTransformMatrix * rotate(mat4(1.0f), radians(x),vec3(0.0f,-1.0f, 0.f));
                             }
            
            if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS) // reset world orientation to original settings
                            {
                                                                         
           
              WorldTransformMatrix = mat4(1.0f);
                                      }
            
            if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
                 glfwSetWindowShouldClose(window, true);
            
            
            
            viewMatrix = lookAt(cameraPosition, cameraPosition + cameraLookAt, cameraUp );
            projectionMatrix = perspective(radians(fovAngle),1024.0f / 768.0f, 0.1f,600.0f);
            
            
         
        }

   
        // Shutdown GLFW
        glfwTerminate();
        
        return 0;
    }

 
    
     void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
     {
         if (key == GLFW_KEY_X && action == GLFW_PRESS)
             textureOn = !textureOn;
         
         if (key == GLFW_KEY_B && action == GLFW_PRESS)
                     shadowsOn = !shadowsOn;
         
         
         if (key == GLFW_KEY_I && action == GLFW_PRESS)
                            octaves += 1;
     
         if (key == GLFW_KEY_O && action == GLFW_PRESS)
                                  octaves -= 1;
     }
   




void createTerrianGeometry(GLuint &VAO, int &xOffset, int &yOffset, GLuint &shader) {
    vector<float> normals;
    vector<float> vertices;
    vector<float> colors;
    vector <int> indices(6 * (mapY - 1) * (mapY - 1));

  

     
     float amp  = 1;
     float freq = 1;


    //create vertices and noise
    float  rangedNoise =0;
    

    for (int y = 0; y < mapY ; y++)
        for (int x = 0; x < mapX; x++) {
            vertices.push_back(x);
                    amp  = 1;
                    freq = 1;
                    float noiseHeight = 0;
                    for (int i = 0; i < octaves; i++) {
                        float xSample = (x + xOffset * (mapX-1))  / noiseScale * freq;
                        float ySample = (y + yOffset * (mapY-1)) / noiseScale * freq;
                     
                    
                     
                                   
                        float perlinValue = SimplexNoise::noise(xSample,ySample);
                        noiseHeight += perlinValue * amp;
                           
                        amp  *= persistence;
                        freq *= lacunarity;
                    }
            rangedNoise = Remap(noiseHeight, -1.0, 1, 0, 1);
       

            vertices.push_back((rangedNoise * meshHeight));
            vertices.push_back(y);
        }




    int pointer = 0;
    for(int y = 0; y < mapY - 1; y++) {
        for(int x = 0; x < mapX - 1; x++) {
            int topLeft = (y * mapY) + x;
            int topRight = topLeft + 1;
            int bottomLeft = ((y + 1) * mapY) + x;
            int bottomRight = bottomLeft + 1;
            indices[pointer++] = topLeft;
            indices[pointer++] = bottomLeft;
            indices[pointer++] = topRight;
            indices[pointer++] = topRight;
            indices[pointer++] = bottomLeft;
            indices[pointer++] = bottomRight;
        }
    }
//calculate normals for lighting
    int pos;
   glm::vec3 normal;

   std::vector<glm::vec3> verts;

   // Get the vertices of each triangle in mesh
   // For each group of indices
   for (int i = 0; i < vertices.size(); i += 3) {

       // Get the vertices (point) for each index
       for (int j = 0; j < 3; j++) {
           pos = vertices[i+j]*3;
           verts.push_back(glm::vec3(vertices[pos], vertices[pos+1], vertices[pos+2]));
       }

//        Get vectors of two edges of triangle
       glm::vec3 U = verts[i+1] - verts[i];
       glm::vec3 V = verts[i+2] - verts[i];

       // Calculate normal
       normal = glm::normalize(-glm::cross(U, V));
       normals.push_back(normal.x);
       normals.push_back(normal.y);
       normals.push_back(normal.z);
   }

 
   

    

    
   
    
 
  GLuint VBO[3], EBO;
  
  // Create buffers and arrays
  glGenBuffers(3, VBO);
  glGenBuffers(1, &EBO);
  glGenVertexArrays(1, &VAO);
  
  // Bind vertices to VBO
  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);
  
  // Create element buffer
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), &indices[0], GL_STATIC_DRAW);
  
  // Configure vertex position attribute
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);
  
//  // Bind vertices to VBO
  glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
  glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(float), &normals[0], GL_STATIC_DRAW);

  // Configure vertex normals attribute
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(1);
    



    


    
}




void renderTerrain(vector <GLuint> &VAO, const GLuint &shader,  int &nIndices, vec3 &cameraPosition) {

    
    
 glUseProgram(shader);
    GLuint modelViewProjection_terrain = glGetUniformLocation(shader, "mvp");
    glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, depthMap);
    
    

 


  

    for (int y = 0; y < yMapChunks; y++)
        for (int x = 0; x < xMapChunks; x++) {
    

               mat4 mvp = glm::mat4(1.0f);
            mvp = translate(mvp, glm::vec3(-mapX / 2.0 + (mapX - 1) * x, 0.0, -mapY / 2.0 + (mapY - 1) * y));

            

                glUniformMatrix4fv(modelViewProjection_terrain, 1, GL_FALSE, &mvp[0][0]);
                
    
           
                glBindVertexArray(VAO[x + y*xMapChunks]);

//                glUniform3f(colors,1.0f,0.5f,1.0f);
                glDrawElements(primativeRender, nIndices, GL_UNSIGNED_INT, 0);
 
                
            glBindVertexArray(0);
               


        }
            

    
    }
 
//helper function to map values to stay within a range
float Remap (float value, float from1, float to1, float from2, float to2)
{
return (value - from1) / ((to1 - from1) * (to2 - from2) + from2);
 }
