#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "glm/gtx/string_cast.hpp"

#include "Shader.h"
#include "Camera.h"
#include "Model.h"

#include <iostream>



void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

//cubeMap
unsigned int loadCubemap(vector<std::string> faces);


// settings
const unsigned int SCR_WIDTH = 3840;
const unsigned int SCR_HEIGHT = 2400;

// camera
Camera camera(glm::vec3(-30.0f, 0.0f, 30.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// positions all containers
//glm::vec3 cubePositions[] = {
//    glm::vec3(0.0f,  0.0f,  0.0f),
//    glm::vec3(2.0f,  5.0f, -15.0f),
//    glm::vec3(-1.5f, -2.2f, -2.5f),
//    glm::vec3(-3.8f, -2.0f, -12.3f),
//    
//};
//position of dir light
glm::vec3 dirlightpos(-40.0f, 40.0f, 40.0f);


// positions of the point lights
glm::vec3 pointLightPositions[] = {
    glm::vec3(3.6f, -2.0f, 31.6f),
    glm::vec3(-7.3f, -2.0f, 28.5f),
    glm::vec3(-25.7f, -2.0f, 10.2f),
    glm::vec3(-28.1f, -2.0f, -0.6f),
    glm::vec3(3.8f, -2.0f, -33.5f),
    glm::vec3(36.2f,-2.0f, -1.0f),
    glm::vec3(3.6f, 23.0f, -1.05f) //top[6]

};



//prespective
//glm::mat4 myperspective(float angleInRadians, float aspectRatio, float zNear, float zFar)
//{
//    float matarray[16] = { 0 }; //4*4 matrix array
//    matarray[0] = 1 / (aspectRatio * tan(angleInRadians / 2.0f));
//    matarray[5] = 1 / tan(angleInRadians / 2.0f);
//    matarray[11] = 1.0f;
//
//    if (GLM_DEPTH_CLIP_SPACE == GLM_DEPTH_ZERO_TO_ONE)
//    {
//        matarray[10] = -zFar / (zFar - zNear);
//        matarray[14] = -(zFar * zNear) / (zFar - zNear);
//    }
//    else
//    {
//        matarray[10] = -(zFar + zNear) / (zFar - zNear);
//        matarray[14] = -(2.0f * zFar * zNear) / (zFar - zNear);
//    }
//
//    glm::mat4 projection = glm::make_mat4(matarray);
//    //std::cout << glm::to_string(projection);
//    return projection;
//}
glm::mat4 myperspective(float fov, float aspectRatio, float zNear, float zFar)
{
    glm::mat4 projection;

    float scale = 1.0 / tan(fov / 2);
    projection[0][0] = scale / aspectRatio;
    projection[1][1] = scale; // scale the y coordinates

    projection[2][3] = -1;
    //projection[2][2] = -(zFar + zNear) / (zFar - zNear);   // remap z to [0,1] 
    //projection[3][2] = (2*zFar*zNear) / (zNear - zFar);

    if (GLM_DEPTH_CLIP_SPACE == GLM_DEPTH_ZERO_TO_ONE) {
        projection[2][2] = zFar / (zNear - zFar);
        projection[3][2] = -(zFar * zNear) / (zFar - zNear);
    }
    else {
        projection[2][2] = -(zFar + zNear) / (zFar - zNear);
        projection[3][2] = -(2 * zFar * zNear) / (zFar - zNear);
    }

    return projection;
}


//scale
glm::mat4 mat_scale(glm::mat4 mat1, glm::vec3 vec1)
{
    glm::mat4 scaledMatrix;

    scaledMatrix[0] = mat1[0] * vec1[0];
    scaledMatrix[1] = mat1[1] * vec1[1];
    scaledMatrix[2] = mat1[2] * vec1[2];
    scaledMatrix[3] = mat1[3];

    //std::cout << "Scaled Matrix: \n";
    //std::cout << glm::to_string(scaledMatrix);

    return scaledMatrix;
}

//rotate
glm::mat4 mat_rotate(glm::mat4 mat1, float angle, glm::vec3 vec1)
{
    const float theta = angle; //glm::radians(angle) if input is in degree
    const float cos_theta = cos(angle);
    const float sin_theta = sin(theta);

    glm::vec3 axis = normalize(vec1);  // vec1/sqrt(vec1[0]*vec1[0] + vec1[1]*vec1[1] + vec1[2]*vec1[2])
        glm::vec3 temp = (1 - cos_theta) * axis;

    glm::mat4 Rotate;  //Generating composite transformation matrix

    Rotate[0][0] = cos_theta + temp[0] * axis[0];
    Rotate[0][1] = temp[0] * axis[1] + sin_theta * axis[2];
    Rotate[0][2] = temp[0] * axis[2] - sin_theta * axis[1];

    Rotate[1][0] = temp[1] * axis[0] - sin_theta * axis[2];
    Rotate[1][1] = cos_theta + temp[1] * axis[1];
    Rotate[1][2] = temp[1] * axis[2] + sin_theta * axis[0];

    Rotate[2][0] = temp[2] * axis[0] + sin_theta * axis[1];
    Rotate[2][1] = temp[2] * axis[1] - sin_theta * axis[0];
    Rotate[2][2] = cos_theta + temp[2] * axis[2];

    glm::mat4 Result_Mat;   //obtain rotated matrix by multiplying with CTM
    Result_Mat[0] = mat1[0] * Rotate[0][0] + mat1[1] * Rotate[0][1] + mat1[2] * Rotate[0][2];
    Result_Mat[1] = mat1[0] * Rotate[1][0] + mat1[1] * Rotate[1][1] + mat1[2] * Rotate[1][2];
    Result_Mat[2] = mat1[0] * Rotate[2][0] + mat1[1] * Rotate[2][1] + mat1[2] * Rotate[2][2];
    Result_Mat[3] = mat1[3];


   /* std::cout << "Rotated Matrix: \n";
    std::cout << glm::to_string(Result_Mat);*/

    return Result_Mat;

}

//translate
glm::mat4 mat_Translate(glm::mat4 mat1, glm::vec3 vec1)
{
    glm::mat4 Result_Mat;           // cant do mat[0] + vec[0], mat[1] + vec[1], mat[2] + vec[2]
    Result_Mat[0] = mat1[0];
    Result_Mat[1] = mat1[1];
    Result_Mat[2] = mat1[2];
    Result_Mat[3] = mat1[0] * vec1[0] + mat1[1] * vec1[1] + mat1[2] * vec1[2] + mat1[3];


    //std::cout << "Translated Matrix: \n";
    //std::cout << glm::to_string(Result_Mat);

    return Result_Mat;
}

//void LightFunction(Shader& lightingShader)
//{
//    lightingShader.use();
//    lightingShader.setInt("material.diffuse", 0);
//    lightingShader.setInt("material.specular", 1);
//}

//void SetIntensity(Shader& lightingShader, glm::vec3 lightPos, glm::vec3 lightambient)
//{
//    
//
//    // light properties
//    lightingShader.setVec3("light.position", lightPos);
//    lightingShader.setVec3("light.ambient", lightambient);
//    lightingShader.setVec3("light.diffuse", 0.8f, 0.8f, 0.8f);
//    lightingShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);
//    lightingShader.setFloat("light.constant", -0.01f);
//    lightingShader.setFloat("light.linear", -0.002f);
//    lightingShader.setFloat("light.quadratic", 0.0013f);
//
//    
//
//}

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Boudhanath Stupa", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    stbi_set_flip_vertically_on_load(true);

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);


    // build and compile shaders
    // -------------------------
    //Shader ourShader("modelVS.txt", "modelFS.txt");
    Shader skyboxShader("skyboxVS.txt", "skyboxFS.txt");
 

    //cubeMap
    float skyboxVertices[] = {
        // positions          
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

    // skybox VAO
    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    vector<std::string> faces
    {
        "skybox/right.jpg",
        "skybox/left.jpg",
        "skybox/top.jpg",
        "skybox/bottom.jpg",
        "skybox/front.jpg",
        "skybox/back.jpg"
    };

    //vector<std::string> faces
    //{
    //    "skybox/negx.jpg",
    //    "skybox/posx.jpg",
    //    "skybox/posy.jpg",
    //    "skybox/negy.jpg",
    //    "skybox/negz.jpg",
    //    "skybox/posz.jpg"
    //};

    /*vector<std::string> night_faces
    {
        "skybox/night/right.jpg",
        "skybox/night/left.jpg",
        "skybox/night/top.jpg",
        "skybox/night/bottom.jpg",
        "skybox/night/front.jpg",
        "skybox/night/back.jpg"
    };*/
    unsigned int cubemapTexture = loadCubemap(faces);
    //unsigned int night_cubemapTexture = loadCubemap(night_faces);

    skyboxShader.use();
    skyboxShader.setInt("skybox", 0);

  



    //ground plane

    const float groundPlaneVertices[] = {
        // positions  //texture coordinates
        -1, 0, -1, 0, 1,
        1, 0, -1, 1, 1,
        1, 0, 1, 1, 0,
        -1, 0, 1, 0, 0
    };

    const unsigned int groundPlaneIndices[] = {
        0, 1, 2,
        2, 3, 0
    };

    unsigned int groundPlaneVBO, groundPlaneVAO, groundPlaneEBO;
    glGenVertexArrays(1, &groundPlaneVAO);
    glGenBuffers(1, &groundPlaneVBO);
    glGenBuffers(1, &groundPlaneEBO);
    glBindVertexArray(groundPlaneVAO);

    glBindBuffer(GL_ARRAY_BUFFER, groundPlaneVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(groundPlaneVertices), groundPlaneVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, groundPlaneEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(groundPlaneIndices), groundPlaneIndices, GL_STATIC_DRAW);

    //position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    //texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // load and create ground texture 
  // -------------------------
    unsigned int groundTexture;
    glGenTextures(1, &groundTexture);
    glBindTexture(GL_TEXTURE_2D, groundTexture); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load image, create texture and generate mipmaps
    int width, height, nrChannels;
    // The FileSystem::getPath(...) is part of the GitHub repository so we can find files on any IDE/platform; replace it with your own image path.
    unsigned char* data = stbi_load("textures/bottom.jpg", &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);

    Shader groundShader("groundVS.txt", "groundFS.txt");
    //glm::mat4 groundModelMatrix = glm::translate(glm::mat4(1.f), glm::vec3(10.f, -1.5f, 0.f));
    glm::mat4 groundModelMatrix = mat_Translate(glm::mat4(1.f), glm::vec3(1.f, -1.5f, 0.f));
    groundModelMatrix = mat_scale(groundModelMatrix, glm::vec3(10.f));
    groundShader.use();
    groundShader.setMat4("model", groundModelMatrix);
    groundShader.setInt("g_texture", 0);


    float lightCubeVertices[] = {
        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,

        -0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,

        -0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,

         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,

        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f, -0.5f,

        -0.5f,  0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f, -0.5f,
    };

    unsigned int lightCubeVBO, lightCubeVAO;
    glGenVertexArrays(1, &lightCubeVAO);
    glGenBuffers(1, &lightCubeVBO);
    glBindBuffer(GL_ARRAY_BUFFER, lightCubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(lightCubeVertices), lightCubeVertices, GL_STATIC_DRAW);
    glBindVertexArray(lightCubeVAO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    Shader lightCubeShader("lightCubeVS.txt", "lightCubeFS.txt");

    // load models
    // -----------
    Model stupamodel("objects/stupa/stupa.obj");



    // draw in wireframe
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    // 
    // 
     // shader configuration
    // --------------------
    Shader lightingShader("light.VS", "light.FS");
    lightingShader.use();
    lightingShader.setInt("material.diffuse", 0);
    lightingShader.setInt("material.specular", 1);
   // LightFunction(lightingShader);

    bool isNightMode = 1;

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.96f, 1.0f, 0.98f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // be sure to activate shader when setting uniforms/drawing objects
        

        // material properties
        lightingShader.use();
        lightingShader.setVec3("viewPos", camera.Position);
        lightingShader.setFloat("material.shininess", 30.0f);
        lightingShader.setInt("dir", isNightMode);

        // directional light
       
        //lightingShader.setVec3("dirLight.direction", 0.8f, 0.8f, 0.8f);
        /*lightingShader.setVec3("dirLight.ambient", 0.8f, 0.8f, 0.8f);
        lightingShader.setVec3("dirLight.diffuse", 0.6f, 0.6f, 0.6f);
        lightingShader.setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);*/


        if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
            
                isNightMode = 1;
                lightingShader.setVec3("dirLight.ambient", 0.0f, 0.0f, 0.0f);
                lightingShader.setVec3("dirLight.diffuse", 0.0f, 0.0f, 0.0f);
                lightingShader.setVec3("dirLight.specular", 0.0f, 0.0f, 0.0f);
           
            lightingShader.setInt("dir", isNightMode);
        }

        if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS) {
            
                isNightMode = 0;
                lightingShader.setVec3("dirLight.direction", 0.8f, 0.8f, 0.8f);
                //lightingShader.setVec3("dirLight.position", 50.0f, 40.0f, 40.0f);
                lightingShader.setVec3("dirLight.ambient", 0.5f, 0.5f, 0.5f);
                lightingShader.setVec3("dirLight.diffuse", 0.5f, 0.5f, 0.5f);
                lightingShader.setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);
  

            lightingShader.setInt("dir", isNightMode);
        }




        // point light 1
        lightingShader.setVec3("pointLights[0].position", pointLightPositions[0]);
        lightingShader.setVec3("pointLights[0].ambient", 0.1f, 0.1f, 0.1f);
        lightingShader.setVec3("pointLights[0].diffuse", 0.83f, 0.68f, 0.21f);//0.8f, 0.8f, 0.8f
        lightingShader.setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
        lightingShader.setFloat("pointLights[0].constant", 1.0f);
        lightingShader.setFloat("pointLights[0].linear", 0.09);
        lightingShader.setFloat("pointLights[0].quadratic", 0.032);
        // point light 2
        lightingShader.setVec3("pointLights[1].position", pointLightPositions[1]);
        lightingShader.setVec3("pointLights[1].ambient", 0.1f, 0.1f, 0.1f);
        lightingShader.setVec3("pointLights[1].diffuse", 0.83f, 0.68f, 0.21f);
        lightingShader.setVec3("pointLights[1].specular", 1.0f, 1.0f, 1.0f);
        lightingShader.setFloat("pointLights[1].constant", 1.0f);
        lightingShader.setFloat("pointLights[1].linear", 0.09);
        lightingShader.setFloat("pointLights[1].quadratic", 0.032);
        // point light 3
        lightingShader.setVec3("pointLights[2].position", pointLightPositions[2]);
        lightingShader.setVec3("pointLights[2].ambient", 0.1f, 0.1f, 0.1f);
        lightingShader.setVec3("pointLights[2].diffuse", 0.83f, 0.68f, 0.21f);
        lightingShader.setVec3("pointLights[2].specular", 1.0f, 1.0f, 1.0f);
        lightingShader.setFloat("pointLights[2].constant", 1.0f);
        lightingShader.setFloat("pointLights[2].linear", 0.09);
        lightingShader.setFloat("pointLights[2].quadratic", 0.032);
        // point light 4
        lightingShader.setVec3("pointLights[3].position", pointLightPositions[3]);
        lightingShader.setVec3("pointLights[3].ambient", 0.1f, 0.1f, 0.1f);
        lightingShader.setVec3("pointLights[3].diffuse", 0.83f, 0.68f, 0.21f);
        lightingShader.setVec3("pointLights[3].specular", 1.0f, 1.0f, 1.0f);
        lightingShader.setFloat("pointLights[3].constant", 1.0f);
        lightingShader.setFloat("pointLights[3].linear", 0.09);
        lightingShader.setFloat("pointLights[3].quadratic", 0.032);
        // point light 5
        lightingShader.setVec3("pointLights[4].position", pointLightPositions[4]);
        lightingShader.setVec3("pointLights[4].ambient", 0.1f, 0.1f, 0.1f);
        lightingShader.setVec3("pointLights[4].diffuse", 0.83f, 0.68f, 0.21f);
        lightingShader.setVec3("pointLights[4].specular", 1.0f, 1.0f, 1.0f);
        lightingShader.setFloat("pointLights[4].constant", 1.0f);
        lightingShader.setFloat("pointLights[4].linear", 0.09);
        lightingShader.setFloat("pointLights[4].quadratic", 0.032);
        // point light 6
        lightingShader.setVec3("pointLights[5].position", pointLightPositions[5]);
        lightingShader.setVec3("pointLights[5].ambient", 0.1f, 0.1f, 0.1f);
        lightingShader.setVec3("pointLights[5].diffuse", 0.83f, 0.68f, 0.21f);
        lightingShader.setVec3("pointLights[5].specular", 1.0f, 1.0f, 1.0f);
        lightingShader.setFloat("pointLights[5].constant", 1.0f);
        lightingShader.setFloat("pointLights[5].linear", 0.09);
        lightingShader.setFloat("pointLights[5].quadratic", 0.032);
        // point light 7
        lightingShader.setVec3("pointLights[6].position", pointLightPositions[6]);
        lightingShader.setVec3("pointLights[6].ambient", 0.8f, 0.8f, 0.8f);
        lightingShader.setVec3("pointLights[6].diffuse", 0.83f, 0.68f, 0.21f);
        lightingShader.setVec3("pointLights[6].specular", 1.0f, 1.0f, 1.0f);
        lightingShader.setFloat("pointLights[6].constant", 1.0f);
        lightingShader.setFloat("pointLights[6].linear", 0.045);
        lightingShader.setFloat("pointLights[6].quadratic", 0.0075);


        // view/projection transformations
        //glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 projection = myperspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

        glm::mat4 view = camera.GetViewMatrix();
        lightingShader.setMat4("projection", projection);
        lightingShader.setMat4("view", view);

       
        // render the loaded model


        glm::mat4 model = glm::mat4(1.0f);
        ////model = glm::translate(model, glm::vec3(0.0f, -1.4f, 0.0f)); // translate it down so it's at the center of the scene
        model = mat_Translate(model, glm::vec3(0.0f, -1.4f, 0.0f)); // translate it down so it's at the center of the scene

        ////model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));  // it's a bit too big for our scene, so scale it down
       model = mat_scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
        ////model = glm::rotate(model, glm::radians(-135.0f), glm::vec3(0.0, 1.0, 0.0));
        model = mat_rotate(model, glm::radians(-135.0f), glm::vec3(0.0, 1.0, 0.0));
        lightingShader.setMat4("model", model);

        
        



        stupamodel.Draw(lightingShader);


        //plane
        glBindTexture(GL_TEXTURE_2D, groundTexture);
        groundShader.use();
        groundShader.setMat4("projection", projection);
        groundShader.setMat4("view", view);
        glBindVertexArray(groundPlaneVAO);
       //glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);


        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------


        //lightCube drawing
        lightCubeShader.use();
        lightCubeShader.setMat4("projection", projection);
        lightCubeShader.setMat4("view", view);


        // bind diffuse map
        glActiveTexture(GL_TEXTURE0);


        glBindVertexArray(lightCubeVAO);
        model = glm::mat4(1.0f);
        model = mat_Translate(model, dirlightpos);
        model = mat_scale(model, glm::vec3(0.2f)); // Make it a smaller cube
        lightCubeShader.setMat4("model", model);
       
        glDrawArrays(GL_TRIANGLES, 0, 36);


        // we now draw as many light bulbs as we have point lights.
        
        //for (unsigned int i = 0; i <= 6; i++)
        //{
        //    model = glm::mat4(1.0f);
        //    model = mat_Translate(model, pointLightPositions[i]);
        //    model = mat_scale(model, glm::vec3(0.2f)); // Make it a smaller cube
        //    lightCubeShader.setMat4("model", model);
        //    glBindVertexArray(lightCubeVAO);
        //    glDrawArrays(GL_TRIANGLES, 0, 36);
        //}

        // draw skybox as last
        glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
        skyboxShader.use();

        view = glm::mat4(glm::mat3(camera.GetViewMatrix())); // remove translation from the view matrix
        skyboxShader.setMat4("view", view);
        skyboxShader.setMat4("projection", projection);
        // skybox cube
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        /*if (isNightMode == 0) {
            glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        }
        else {
            glBindTexture(GL_TEXTURE_CUBE_MAP, night_cubemapTexture);
        }*/
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS); // set depth function back to default


        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}


// loads a cubemap texture from 6 individual texture faces
// order:
// +X (right)
// -X (left)
// +Y (top)
// -Y (bottom)
// +Z (front) 
// -Z (back)
// -------------------------------------------------------
unsigned int loadCubemap(vector<std::string> faces)
{
    stbi_set_flip_vertically_on_load(false);
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}
