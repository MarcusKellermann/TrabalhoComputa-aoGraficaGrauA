/* Hello Triangle - código adaptado de https://learnopengl.com/#!Getting-started/Hello-Triangle
 *
 * Adaptado por Rossana Baptista Queiroz
 * para a disciplina de Processamento Gráfico/Computação Gráfica - Unisinos
 * Versão inicial: 7/4/2017
 * Última atualização em 01/03/2023
 *
 */

#include <iostream>
#include <string>
#include <assert.h>
#include <vector>
#include <fstream>
#include <sstream>

using namespace std;

// GLAD
#include <glad/glad.h>

// GLFW
#include <GLFW/glfw3.h>

//GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"

#include "Mesh.h"

struct Vertex
{
	glm::vec3 position;
	glm::vec3 color;
	//glm::vec3 normal;
};


// Protótipo da função de callback de teclado
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);


// Protótipo da função de callback do mouse
void mouse_callback(GLFWwindow* window, double xpos, double ypos);


// Protótipos das funções
int setupGeometry();
int loadSimpleOBJ(string filepath, int& nVerts, glm::vec3 color = glm::vec3(1.0, 0.0, 1.0));

// Dimensões da janela (pode ser alterado em tempo de execução)
const GLuint WIDTH = 1000, HEIGHT = 1000;

bool rotateX = false, rotateY = false, rotateZ = false, stopRotate = false;
bool translateF = false, translateG = false, translateH = false;// novo
bool vdd = true;

glm::vec3 cameraPos = glm::vec3(0.0, 0.0, 3.0);
glm::vec3 cameraFront = glm::vec3(0.0, 0.0, -1.0);
glm::vec3 cameraUp = glm::vec3(0.0, 1.0, 0.0);

//bool firstMouse = true;
//float lastX, lastY;
//float sensitivity = 0.05;
//float pitch = 0.0, yaw = -90.0;

float sensitivity = 0.02;
float speed = 0.5;
bool firstMouse = true;
float lastX = 0.0, lastY = 0.0;
float yaw = -90.0, pitch = 0.0;
float fov = 45.0;

int selec = 0;


vector <Vertex> vertices;
vector <GLuint> indices;
vector <glm::vec3> normals;
vector <glm::vec2> texCoord;


// Função MAIN
int main()
{
	// Inicialização da GLFW
	glfwInit();

	//Muita atenção aqui: alguns ambientes não aceitam essas configurações
	//Você deve adaptar para a versão do OpenGL suportada por sua placa
	//Sugestão: comente essas linhas de código para desobrir a versão e
	//depois atualize (por exemplo: 4.5 com 4 e 5)
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//Essencial para computadores da Apple
//#ifdef __APPLE__
//	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
//#endif

	// Criação da janela GLFW
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Ola 3D!", nullptr, nullptr);
	glfwMakeContextCurrent(window);

	// Fazendo o registro da função de callback para a janela GLFW
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_callback);

	//glfwSetScrollCallback(window, scroll_callback);//novo

	//glfwSetCursorPos(window,WIDTH / 2, HEIGHT / 2);


	//Desabilita o desenho do cursor 
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);


	// GLAD: carrega todos os ponteiros d funções da OpenGL
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;

	}

	// Obtendo as informações de versão
	const GLubyte* renderer = glGetString(GL_RENDERER); /* get renderer string */
	const GLubyte* version = glGetString(GL_VERSION); /* version as a string */
	cout << "Renderer: " << renderer << endl;
	cout << "OpenGL version supported " << version << endl;

	// Definindo as dimensões da viewport com as mesmas dimensões da janela da aplicação
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);


	// Compilando e buildando o programa de shader
	//GLuint shader.ID = setupShader();
	Shader shader("Phong.vs", "Phong.fs");

	// novo
	glm::mat4 model = glm::mat4(1); //matriz identidade;
	GLint modelLoc = glGetUniformLocation(shader.ID, "model");
	//
	model = glm::rotate(model, /*(GLfloat)glfwGetTime()*/glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	glUniformMatrix4fv(modelLoc, 1, FALSE, glm::value_ptr(model));

	glUseProgram(shader.ID);// Aparece as figura

	//Matriz de view -- posição e orientação da câmera
	glm::mat4 view = glm::lookAt(glm::vec3(0.0, 0.0, 3.0), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
	shader.setMat4("view", value_ptr(view));
	GLint viewLoc = glGetUniformLocation(shader.ID, "view");//novo
	glUniformMatrix4fv(viewLoc, 1, FALSE, glm::value_ptr(view));//novo

	//Matriz de projeção perspectiva - definindo o volume de visualização (frustum)
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
	shader.setMat4("projection", glm::value_ptr(projection));
	GLint projLoc = glGetUniformLocation(shader.ID, "projection");//novo
	glUniformMatrix4fv(projLoc, 1, FALSE, glm::value_ptr(projection));//novo


	glEnable(GL_DEPTH_TEST);

	//novo
	model = glm::mat4(1);
	glUseProgram(shader.ID);



	int nVerts;
	int nVertices1;
	GLuint VAO = loadSimpleOBJ("../../3D_models/Suzanne/suzanneTriLowPoly.obj", nVertices1, glm::vec3(0.0, 1.0, 1.0));
	GLuint VAO2 = loadSimpleOBJ("../../3D_models/Suzanne/suzanneTriLowPoly.obj", nVerts);
	//GLuint VAO3 = loadSimpleOBJ("../../3D_models/Suzanne/suzanneTriLowPoly.obj", nVerts, glm::vec3(1.0, 1.0, 0.0));

	Mesh suzanne1, suzanne2, suzanne3;
	suzanne1.initialize(VAO, nVertices1, &shader, glm::vec3(-2.75, 0.0, 0.0));
	suzanne2.initialize(VAO2, nVerts, &shader);
	//suzanne3.initialize(VAO3, nVerts, &shader,glm::vec3(2.75, 0.0, 0.0));

	Mesh pikachu;
	GLuint VAO3 = loadSimpleOBJ("../../3D_models/Pokemon/Pikachu.obj", nVerts, glm::vec3(1.0, 1.0, 0.0));
	pikachu.initialize(VAO3, nVerts, &shader, glm::vec3(2.0, -1.0, 0.0), glm::vec3(0.5, 0.5, 0.5));


	//Definindo as propriedades do material da superficie
	shader.setFloat("ka", 0.4);
	shader.setFloat("kd", 0.5);
	shader.setFloat("ks", 0.5);
	shader.setFloat("q", 10.0);

	//Definindo a fonte de luz pontual
	shader.setVec3("lightPos", -2.0, 10.0, 2.0);
	shader.setVec3("lightColor", 1.0, 1.0, 1.0);


	//novo
	//int i = 0;

	//Mesh desenho[2];

	//mandar cor pro shader
	//desenho[0].initialize(VAO, nVerts, &shader);
	//desenho[1].initialize(VAO2, nVerts, &shader, glm::vec3(3.0, 0, 0.0));


	glEnable(GL_DEPTH_TEST);

	float light_y = -10;
	float light_x = -10;


	// Loop da aplicação - "game loop"
	while (!glfwWindowShouldClose(window))
	{
		// Checa se houveram eventos de input (key pressed, mouse moved etc.) e chama as funções de callback correspondentes
		glfwPollEvents();

		// Limpa o buffer de cor
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f); //cor de fundo
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glLineWidth(10);
		glPointSize(20);

		float angle = (GLfloat)glfwGetTime();


		//novo
		if (light_x > 10) {
			speed *= -1;
		}
		else if (light_x < -10) {
			speed *= -1;
		}

		light_x += speed;
		light_y = sqrt(-(light_x * light_x) + 100);

		if (speed < 0) {
			light_y *= -1;
		}

		shader.setVec3("lightPos", light_x, light_y, 0);


		float angles = (GLfloat)glfwGetTime() * 2;

		

		
		
		
		
			if (selec == 1)
			{

				if (rotateX)
				{
					//model = glm::rotate(model, angle, glm::vec3(1.0f, 0.0f, 0.0f));
					suzanne1.initialize(VAO, nVertices1, &shader, glm::vec3(-3.0, 0.0, 0.0), glm::vec3(-1.0, -1.0, -1.0), angle, glm::vec3(1.0f, 0.0f, 0.0f));
					suzanne1.update();
					suzanne1.draw();
				}

				if (rotateY)
				{
					//model = glm::rotate(model, angle, glm::vec3(1.0f, 0.0f, 0.0f));
					suzanne1.initialize(VAO, nVertices1, &shader, glm::vec3(-3.0, 0.0, 0.0), glm::vec3(-1.0, -1.0, -1.0), angle, glm::vec3(0.0f, 1.0f, 0.0f));
					suzanne1.update();
					suzanne1.draw();
				}

				if (rotateZ)
				{
					//model = glm::rotate(model, angle, glm::vec3(1.0f, 0.0f, 0.0f));
					suzanne1.initialize(VAO, nVertices1, &shader, glm::vec3(-3.0, 0.0, 0.0), glm::vec3(-1.0, -1.0, -1.0), angle, glm::vec3(0.0f, 0.0f, 1.0f));
					suzanne1.update();
					suzanne1.draw();
				}

				if (translateF)
				{
					//model = glm::rotate(model, angle, glm::vec3(1.0f, 0.0f, 0.0f));
					suzanne1.initialize(VAO, nVertices1, &shader, glm::vec3(-3.0, 0.0, 0.0));
					suzanne1.update();
					suzanne1.draw();
				}
				else if (translateG)
				{
					suzanne1.initialize(VAO, nVerts, &shader, glm::vec3(0.0, 3.0, 0.0));
					suzanne1.update();
					suzanne1.draw();
				}

				else if (translateH)
				{
					suzanne1.initialize(VAO, nVerts, &shader, glm::vec3(0.0, 0.0, 3.0));
					suzanne1.update();
					suzanne1.draw();
				}

			}
		
			else if (selec ==2 ) {
				if (rotateX)
				{
				//model = glm::rotate(model, angle, glm::vec3(1.0f, 0.0f, 0.0f));
					suzanne2.initialize(VAO2, nVerts, &shader, glm::vec3(0.0, 0.0, 0.0), glm::vec3(-1.0, -1.0, -1.0), angle, glm::vec3(1.0f, 0.0f, 0.0f));
					suzanne2.update();
					suzanne2.draw();
				}

				if (rotateY)
				{
					suzanne2.initialize(VAO2, nVerts, &shader, glm::vec3(0.0, 0.0, 0.0), glm::vec3(-1.0, -1.0, -1.0), angle, glm::vec3(0.0f, 1.0f, 0.0f));
					suzanne2.update();
					suzanne2.draw();
				}

				if (rotateZ)
				{
				//model = glm::rotate(model, angle, glm::vec3(1.0f, 0.0f, 0.0f));
					suzanne2.initialize(VAO2, nVerts, &shader, glm::vec3(0.0, 0.0, 0.0), glm::vec3(-1.0, -1.0, -1.0), angle, glm::vec3(0.0f, 0.0f, 1.0f));
					suzanne2.update();
					suzanne2.draw();
				}

				if (translateF)
				{
					//model = glm::rotate(model, angle, glm::vec3(1.0f, 0.0f, 0.0f));
					suzanne2.initialize(VAO2, nVerts, &shader, glm::vec3(0.0, 0.0, 0.0));
					suzanne2.update();
					suzanne2.draw();
				}
				else if (translateG)
				{
					suzanne2.initialize(VAO2, nVerts, &shader, glm::vec3(-3.0, 0.0, 0.0));
					suzanne2.update();
					suzanne2.draw();
				}
				else if (translateH)
				{
					suzanne2.initialize(VAO2, nVerts, &shader, glm::vec3(-3.0, 0.0, 0.0));
					suzanne2.update();
					suzanne2.draw();
				}

		
		
			}
			else if (selec == 3) {
			
				if (rotateX)
				{
					//model = glm::rotate(model, angle, glm::vec3(1.0f, 0.0f, 0.0f));
					pikachu.initialize(VAO3, nVerts, &shader, glm::vec3(3.0, 0.0, 0.0), glm::vec3(-1.0, -1.0, -1.0), angle, glm::vec3(1.0f, 0.0f, 0.0f));
					pikachu.update();
					pikachu.draw();
				}

				if (rotateY)
				{
					pikachu.initialize(VAO3, nVerts, &shader, glm::vec3(3.0, 0.0, 0.0), glm::vec3(-1.0, -1.0, -1.0), angle, glm::vec3(0.0f, 1.0f, 0.0f));
					pikachu.update();
					pikachu.draw();
				}

				if (rotateZ)
				{
					pikachu.initialize(VAO3, nVerts, &shader, glm::vec3(3.0, 0.0, 0.0), glm::vec3(-1.0, -1.0, -1.0), angle, glm::vec3(0.0f, 0.0f, 1.0f));
					pikachu.update();
					pikachu.draw();
				}

				if (translateF)
				{
					pikachu.initialize(VAO3, nVerts, &shader, glm::vec3(0.0, 0.0, 0.0));
					pikachu.update();
					pikachu.draw();
				}
				else if (translateG)
				{
					pikachu.initialize(VAO3, nVerts, &shader, glm::vec3(0.0, -3.0, 0.0));
					pikachu.update();
					pikachu.draw();
				}
				else if (translateH)
				{
					pikachu.initialize(VAO3, nVerts, &shader, glm::vec3(0.0, -3.0, -3.0));
					pikachu.update();
					pikachu.draw();;
				}
			


			
			}

		


		

		

		

		


		glUniformMatrix4fv(modelLoc, 1, FALSE, glm::value_ptr(model));

		//Atualizando a posição e orientação da câmera
		glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
		//shader.setMat4("view", glm::value_ptr(view));
		glUniformMatrix4fv(viewLoc, 1, FALSE, glm::value_ptr(view));


		glm::mat4 projection = glm::perspective(glm::radians(fov), (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);
		glUniformMatrix4fv(projLoc, 1, FALSE, glm::value_ptr(projection));


		

		//Atualizando o shader com a posição da câmera
		shader.setVec3("cameraPos", cameraPos.x, cameraPos.y, cameraPos.z);

		// Chamada de desenho - drawcall
		suzanne1.update();
		suzanne1.draw();

		suzanne2.update();
		suzanne2.draw();

		
		pikachu.update();
		pikachu.draw();

		// Troca os buffers da tela
		glfwSwapBuffers(window);
	}
	// Pede pra OpenGL desalocar os buffers
	glDeleteVertexArrays(1, &VAO);
	// Finaliza a execução da GLFW, limpando os recursos alocados por ela
	glfwTerminate();
	return 0;
}

// Função de callback de teclado - só pode ter uma instância (deve ser estática se
// estiver dentro de uma classe) - É chamada sempre que uma tecla for pressionada
// ou solta via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key == GLFW_KEY_P && (action == GLFW_PRESS))
	{
		speed += 0.1;
	}

	if (key == GLFW_KEY_O && action == GLFW_PRESS)
	{
		speed -= 0.1;
	}

	if (key == GLFW_KEY_1)
	{
		vdd == false;
		selec = 1;
		
	}

	if (key == GLFW_KEY_2)
	{
		selec = 2;
	}

	if (key == GLFW_KEY_3)
	{
		selec = 3;
	}
	if (key == GLFW_KEY_4)
	{
		selec = 4;
	}


	if (key == GLFW_KEY_X )
	{
		rotateX = true;
		rotateY = false;
		rotateZ = false;
	}

	if (key == GLFW_KEY_Y )
	{
		rotateX = false;
		rotateY = true;
		rotateZ = false;
	}

	if (key == GLFW_KEY_Z )
	{
		rotateX = false;
		rotateY = false;
		rotateZ = true;
	}


	if (key == GLFW_KEY_F)
	{
		translateF = true;
		translateG = false;
		translateH = false;
	}

	if (key == GLFW_KEY_G )
	{
		translateF = false;
		translateG = true;
		translateH = false;
	}

	if (key == GLFW_KEY_H)
	{
		translateF = false;
		translateG = false;
		translateH = true;
	}

	if (key == GLFW_KEY_W)
	{
		cameraPos += cameraFront * float(0.1);
	}

	if (key == GLFW_KEY_S)
	{
		cameraPos -= cameraFront * float(0.1);
	}

	if (key == GLFW_KEY_A)
	{
		cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * float(0.1);
	}

	if (key == GLFW_KEY_D)
	{
		cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * float(0.1);
	}

	

	
}


void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	//cout << xpos << " " << ypos << endl;
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float offsetx = xpos - lastX;
	float offsety = lastY - ypos;

	lastX = xpos;
	lastY = ypos;

	offsetx *= sensitivity;
	offsety *= sensitivity;

	pitch += offsety;
	yaw += offsetx;

	glm::vec3 front;
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront = glm::normalize(front);

}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	fov -= yoffset;
}

// Esta função está bastante harcoded - objetivo é criar os buffers que armazenam a 
// geometria de um triângulo
// Apenas atributo coordenada nos vértices
// 1 VBO com as coordenadas, VAO com apenas 1 ponteiro para atributo
// A função retorna o identificador do VAO
int setupGeometry()
{
	// Aqui setamos as coordenadas x, y e z do triângulo e as armazenamos de forma
	// sequencial, já visando mandar para o VBO (Vertex Buffer Objects)
	// Cada atributo do vértice (coordenada, cores, coordenadas de textura, normal, etc)
	// Pode ser arazenado em um VBO único ou em VBOs separados
	GLfloat vertices[] = {

		//Base da pirâmide: 2 triângulos
		//x    y    z    r    g    b
		-0.5, -0.5, -0.5, 1.0, 1.0, 0.0,
		-0.5, -0.5,  0.5, 0.0, 1.0, 1.0,
		 0.5, -0.5, -0.5, 1.0, 0.0, 1.0,

		 -0.5, -0.5, 0.5, 1.0, 1.0, 0.0,
		  0.5, -0.5,  0.5, 0.0, 1.0, 1.0,
		  0.5, -0.5, -0.5, 1.0, 0.0, 1.0,

		  //
		  -0.5, -0.5, -0.5, 1.0, 1.0, 0.0,
		   0.0,  0.5,  0.0, 1.0, 1.0, 0.0,
		   0.5, -0.5, -0.5, 1.0, 1.0, 0.0,

		   -0.5, -0.5, -0.5, 1.0, 0.0, 1.0,
		   0.0,  0.5,  0.0, 1.0, 0.0, 1.0,
		   -0.5, -0.5, 0.5, 1.0, 0.0, 1.0,

			-0.5, -0.5, 0.5, 1.0, 1.0, 0.0,
		   0.0,  0.5,  0.0, 1.0, 1.0, 0.0,
		   0.5, -0.5, 0.5, 1.0, 1.0, 0.0,

			0.5, -0.5, 0.5, 0.0, 1.0, 1.0,
		   0.0,  0.5,  0.0, 0.0, 1.0, 1.0,
		   0.5, -0.5, -0.5, 0.0, 1.0, 1.0,

		   //Chão
		   //x    y    z    r    g    b
		   -5.0, -0.5, -5.0, 0.5, 0.5, 0.5,
		   -5.0, -0.5,  5.0, 0.5, 0.5, 0.5,
			5.0, -0.5, -5.0, 0.5, 0.5, 0.5,

		   -5.0, -0.5,  5.0, 0.5, 0.5, 0.5,
			5.0, -0.5,  5.0, 0.5, 0.5, 0.5,
			5.0, -0.5, -5.0, 0.5, 0.5, 0.5

	};

	GLuint VBO, VAO;

	//Geração do identificador do VBO
	glGenBuffers(1, &VBO);

	//Faz a conexão (vincula) do buffer como um buffer de array
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	//Envia os dados do array de floats para o buffer da OpenGl
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	//Geração do identificador do VAO (Vertex Array Object)
	glGenVertexArrays(1, &VAO);

	// Vincula (bind) o VAO primeiro, e em seguida  conecta e seta o(s) buffer(s) de vértices
	// e os ponteiros para os atributos 
	glBindVertexArray(VAO);

	//Para cada atributo do vertice, criamos um "AttribPointer" (ponteiro para o atributo), indicando: 
	// Localização no shader * (a localização dos atributos devem ser correspondentes no layout especificado no vertex shader)
	// Numero de valores que o atributo tem (por ex, 3 coordenadas xyz) 
	// Tipo do dado
	// Se está normalizado (entre zero e um)
	// Tamanho em bytes 
	// Deslocamento a partir do byte zero 

	//Atributo posição (x, y, z)
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	//Atributo cor (r, g, b)
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);



	// Observe que isso é permitido, a chamada para glVertexAttribPointer registrou o VBO como o objeto de buffer de vértice 
	// atualmente vinculado - para que depois possamos desvincular com segurança
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Desvincula o VAO (é uma boa prática desvincular qualquer buffer ou array para evitar bugs medonhos)
	glBindVertexArray(0);

	return VAO;
}

int loadSimpleOBJ(string filepath, int& nVerts, glm::vec3 color)
{
	vector <Vertex> vertices;
	vector <GLuint> indices;
	vector <glm::vec2> texCoords;
	vector <glm::vec3> normals;
	vector <GLfloat> vbuffer;

	ifstream inputFile;
	inputFile.open(filepath.c_str());
	if (inputFile.is_open())
	{
		char line[100];
		string sline;



		while (!inputFile.eof())
		{
			inputFile.getline(line, 100);
			sline = line;

			string word;

			istringstream ssline(line);
			ssline >> word;

			//cout << word << " ";
			if (word == "v")
			{
				Vertex v;

				ssline >> v.position.x >> v.position.y >> v.position.z;
				v.color.r = color.r;  v.color.g = color.g;  v.color.b = color.b;

				vertices.push_back(v);
			}
			if (word == "vt")
			{
				glm::vec2 vt;

				ssline >> vt.s >> vt.t;

				texCoords.push_back(vt);
			}
			if (word == "vn")
			{
				glm::vec3 vn;

				ssline >> vn.x >> vn.y >> vn.z;

				normals.push_back(vn);
			}
			if (word == "f")
			{
				string tokens[3];

				ssline >> tokens[0] >> tokens[1] >> tokens[2];

				for (int i = 0; i < 3; i++)
				{
					//Recuperando os indices de v
					int pos = tokens[i].find("/");
					string token = tokens[i].substr(0, pos);
					int index = atoi(token.c_str()) - 1;
					indices.push_back(index);

					vbuffer.push_back(vertices[index].position.x);
					vbuffer.push_back(vertices[index].position.y);
					vbuffer.push_back(vertices[index].position.z);
					vbuffer.push_back(vertices[index].color.r);
					vbuffer.push_back(vertices[index].color.g);
					vbuffer.push_back(vertices[index].color.b);

					//Recuperando os indices de vts
					tokens[i] = tokens[i].substr(pos + 1);
					pos = tokens[i].find("/");
					token = tokens[i].substr(0, pos);
					index = atoi(token.c_str()) - 1;

					vbuffer.push_back(texCoords[index].s);
					vbuffer.push_back(texCoords[index].t);

					//Recuperando os indices de vns
					tokens[i] = tokens[i].substr(pos + 1);
					index = atoi(tokens[i].c_str()) - 1;

					vbuffer.push_back(normals[index].x);
					vbuffer.push_back(normals[index].y);
					vbuffer.push_back(normals[index].z);
				}
			}

		}

	}
	else
	{
		cout << "Problema ao encontrar o arquivo " << filepath << endl;
	}
	inputFile.close();

	GLuint VBO, VAO;

	nVerts = vbuffer.size() / 11; //Provisório

	//Geração do identificador do VBO
	glGenBuffers(1, &VBO);

	//Faz a conexão (vincula) do buffer como um buffer de array
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	//Envia os dados do array de floats para o buffer da OpenGl
	glBufferData(GL_ARRAY_BUFFER, vbuffer.size() * sizeof(GLfloat), vbuffer.data(), GL_STATIC_DRAW);

	//Geração do identificador do VAO (Vertex Array Object)
	glGenVertexArrays(1, &VAO);

	// Vincula (bind) o VAO primeiro, e em seguida  conecta e seta o(s) buffer(s) de vértices
	// e os ponteiros para os atributos 
	glBindVertexArray(VAO);

	//Para cada atributo do vertice, criamos um "AttribPointer" (ponteiro para o atributo), indicando: 
	// Localização no shader * (a localização dos atributos devem ser correspondentes no layout especificado no vertex shader)
	// Numero de valores que o atributo tem (por ex, 3 coordenadas xyz) 
	// Tipo do dado
	// Se está normalizado (entre zero e um)
	// Tamanho em bytes 
	// Deslocamento a partir do byte zero 

	//Atributo posição (x, y, z)
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	//Atributo cor (r, g, b)
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	//Atributo coordenada de textura (s, t)
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);

	//Atributo normal do vértice (x, y, z)
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(8 * sizeof(GLfloat)));
	glEnableVertexAttribArray(3);


	// Observe que isso é permitido, a chamada para glVertexAttribPointer registrou o VBO como o objeto de buffer de vértice 
	// atualmente vinculado - para que depois possamos desvincular com segurança
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Desvincula o VAO (é uma boa prática desvincular qualquer buffer ou array para evitar bugs medonhos)
	glBindVertexArray(0);

	return VAO;

}

