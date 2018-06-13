#include <sb7.h>
#include <vmath.h>
#include <vector>

#include <ctype.h>
#include <stdlib.h>

#include <iostream>
#include <fstream>

using namespace vmath;
using namespace std;

void MessageCallback( GLenum source,
                      GLenum type,
                      GLuint id,
                      GLenum severity,
                      GLsizei length,
                      const GLchar* message,
                      const void* userParam )
{
  fprintf( stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
           ( type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "" ),
            type, severity, message );
exit(0);
}

class Mesh : public sb7::application
{	
	public:
		vector< unsigned int > vertexIndices, normalIndices;
		vector < vec3 > temp_vertices,temp_normals;
		vector < vec3 > out_vertices, out_normals;
		mat4 rotationMatrix;
		vec3 modelPos;
		vec3 axisRotation;
		float rotationX, rotationY, rotationZ;
		
		vec3 cameraPos;
		float pitch,yaw;
		float grades;
		float rotation;

	private:
		GLuint vertexbuffer;
		GLuint normalbuffer;
		GLuint vao;
		GLuint mvp[3];
		GLuint program;
		GLuint colorLoc;
		GLuint position_buffer;
		GLuint normal_buffer;
	

	virtual void init()
	{
		
		static const char title[] = "Tarea 1�";
		sb7::application::init();
		memcpy(info.title, title, sizeof(title));
		
	}


        virtual void onKey(int key, int action)
	{

		if (key == GLFW_KEY_Q && (action == GLFW_PRESS || action == GLFW_REPEAT))
		{
				rotationZ+=10;
		}

		else if (key == GLFW_KEY_W && (action == GLFW_PRESS || action == GLFW_REPEAT))
		{
				rotationX+=10;
		}

		else if (key == GLFW_KEY_E && (action == GLFW_PRESS || action == GLFW_REPEAT))
		{
				rotationZ-=10;
		}

		else if (key == GLFW_KEY_A && (action == GLFW_PRESS || action == GLFW_REPEAT))
		{
				rotationY+=10;
		}

		else if (key == GLFW_KEY_S && (action == GLFW_PRESS || action == GLFW_REPEAT))
		{
				rotationX-=10;
		}

		else if (key == GLFW_KEY_D && (action == GLFW_PRESS || action == GLFW_REPEAT))
		{
				rotationY-=10;
		}
    }

	virtual void startup()
	{
		
		glEnable              ( GL_DEBUG_OUTPUT );
		glDebugMessageCallback( (GLDEBUGPROC) MessageCallback, 0 );
		

		static const char *vs_source[] =
		{
			"#version 420 core                                    \n"
			"uniform mat4 model;                                  \n"
    			"uniform mat4 view;                                   \n"
   			"uniform mat4 projection;                             \n"
			"in vec4 position;				      \n"
			"in vec3 normal;                                      \n"
			"in vec4 color;                                       \n"
			"out vec3 vs_normal;                                  \n"
			"out vec3 vl_diff;                                    \n"
			"out vec3 vs_position;                                \n"
			"out vec4 vs_color;                                   \n"
			"uniform vec3 light_pos = vec3(25);                   \n"
			"void main(void)                                      \n"
			"{                                                    \n"
			"	vec4 pos = model * position;                  \n"
			"       vs_normal = mat3(model) * normal;             \n"
			"	vl_diff = light_pos - pos.xyz;                \n"
			"	vs_color = color;                             \n"
			"                                                     \n"
			"       vs_position = pos.xyz * -1;                   \n"
			"       gl_Position = projection * view * pos;        \n"
			"}                                                    \n"
		};

		static const char *fs_source[] =
		{
			"#version 420 core                                    \n"
			"out vec4 fs_color;                                   \n"
			"in vec3 vs_normal;                                   \n"
			"in vec3 vl_diff;                                     \n"
			"in vec3 vs_position;                                 \n"
			"in vec4 vs_color;                                    \n"
			"uniform vec3 diffuse_albedo = vec3(0.4);             \n"
			"uniform vec3 specular_albedo = vec3(0.6);            \n"

			"void main(void)                                      \n"
			"{                                                    \n"
			"	vec3 vs_normal = normalize(vs_normal);                                       \n"
			"	vec3 vl_diff = normalize(vl_diff);                                           \n"
			"	vec3 vs_position = normalize(vs_position);                                   \n"
			"       vec3 H = normalize(vl_diff + vs_position);                                   \n"
			"                                                                                    \n"
			"       vec3 diffuse = max(dot(vs_normal, vl_diff), 0.0) * diffuse_albedo;           \n"
			"       vec3 specular = pow(max(dot(vs_normal, H), 0.0), 150) * specular_albedo;     \n"
			"       fs_color = vs_color * vec4(specular + diffuse, 1.0);                         \n"
			"}\n"
		};

		program = glCreateProgram();


		GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fs, 1, fs_source, NULL);
		glCompileShader(fs);

		glAttachShader(program, fs);

		GLuint vs = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vs, 1, vs_source, NULL);
		glCompileShader(vs);


		glAttachShader(program, vs);
		

		glLinkProgram(program);
		colorLoc = glGetAttribLocation(program, "color");
		GLuint posLoc = glGetAttribLocation(program, "position");
		GLuint normalLoc = glGetAttribLocation(program, "normal");

		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
		load();
 
		glGenBuffers(1, &position_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, position_buffer);
		glBufferData(GL_ARRAY_BUFFER, out_vertices.size() * sizeof(vec3), &out_vertices[0], GL_STATIC_DRAW);
		glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 0, NULL);
		glEnableVertexAttribArray(posLoc);

		glGenBuffers(1, &normal_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, normal_buffer);
		glBufferData(GL_ARRAY_BUFFER, out_normals.size() * sizeof(vec3), &out_normals[0], GL_STATIC_DRAW);
		glVertexAttribPointer(normalLoc, 3, GL_FLOAT, GL_FALSE, 0, NULL);
		glEnableVertexAttribArray(normalLoc);

		rotationX = 0;
		rotationY = 0;
		rotationZ = 0;

		modelPos=vec3(0,0,0);
		cameraPos=vec3(0,0,7);
		
		pitch=0;
		yaw=0;
		mvp[0] = glGetUniformLocation(program, "model");
		mvp[1] = glGetUniformLocation(program, "view");
		mvp[2] = glGetUniformLocation(program, "projection");
		glEnable(GL_DEPTH_TEST); 
	}

	void print(vector < vec3 > vertices)
	{
		for( int i=0; i<vertices.size(); i++ )
		{
			cout << "Coordenada X"<< ":" << vertices[i][0] << " Coordenada Y"  << ":" << vertices[i][1] << "Coordenada Z" << ":" << vertices[i][2] << endl;
		}
	}
	
	void load()
	{
		ifstream theFile("media/modelData.txt");

		string type;
		float x;
		float y;
		float z;

		if(theFile.good())
		{
			
			while(!theFile.eof())
			{
				theFile >> type;
				if(type.compare("v")==0)
				{
					theFile >> x >> y >> z;
					vec3 vertex(x,y,z);
					temp_vertices.push_back(vertex);
				}

				else if(type.compare("vn")==0)
				{
					theFile >> x >> y >> z;
					vec3 normal(x,y,z);
					temp_normals.push_back(normal);
				}

				else if(type.compare("f")==0)
				{
					streampos oldpos = theFile.tellg();  
					string currentSentence;
					getline (theFile,currentSentence);
					int numero = getSpaces(currentSentence);
			
					//verificar que las caras estan formadas por 3 vectores
					if(numero == 3)
					{
					
						theFile.seekg (oldpos);

						//Asignar los datos a los vectores
						string data1,data2,data3;
						theFile >> data1 >> data2 >> data3;
					 
						unsigned int vertexIndex[3], normalIndex[3];
						
						if(data1.find("/") == std::string::npos || data2.find("/") == std::string::npos || data3.find("/") == std::string::npos)
						{
							cout << "El formato de la face " << data1 << " , " << data2 << " , " << data3 << " no cumple con el formato" << endl; 
							abort(); 
						}
					
						
						vertexIndex[0] = stoi(data1.substr(0,data1.find('/')));
						normalIndex[0] = stoi(data1.substr(data1.find_last_of("/")+1, data1.length()));
						vertexIndex[1] = stoi(data2.substr(0,data2.find('/')));
						normalIndex[1] = stoi(data2.substr(data2.find_last_of("/")+1, data2.length()));
						vertexIndex[2] = stoi(data3.substr(0,data3.find('/')));
						normalIndex[2] = stoi(data3.substr(data3.find_last_of("/")+1, data3.length()));
						
						vertexIndices.push_back(vertexIndex[0]);
						vertexIndices.push_back(vertexIndex[1]);
						vertexIndices.push_back(vertexIndex[2]);
						normalIndices.push_back(normalIndex[0]);
						normalIndices.push_back(normalIndex[1]);
						normalIndices.push_back(normalIndex[2]);
					}
					
					else
					{
						cerr << currentSentence << " no cumple formato" << endl;
					}
				}
			}

			for( unsigned int i=0; i<vertexIndices.size(); i++ )
			{

				unsigned int vertexIndex = vertexIndices[i];
				unsigned int normalIndex = normalIndices[i];
				
				vec3 vertex = temp_vertices[ vertexIndex-1 ];
				vec3 normal = temp_normals[ normalIndex-1 ];
				
				out_vertices.push_back(vertex);
				out_normals .push_back(normal);
				
			}
		}	
	}

	

	int getSpaces(string currentSentence)
	{
		int spaces = 0;
		for (auto& iter : currentSentence)
		{
			if (iter == ' ')
			{
				spaces++;
			}
		}

		return spaces;
	}

	virtual void render(double currentTime)
        {
		static const GLfloat black[] = {0.0f,0.0f,0.0f, 1.0f};
		GLfloat red[] = { 1.0f,0.0f,0.0f, 1.0f };
		GLfloat green[] = { 0.0f,1.0f,0.0f, 1.0f };
		GLfloat blue[] = { 0.0f,0.0f,1.0f, 1.0f };

		glClearColor(black[0],black[1],black[2],black[3]);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(program);

		glVertexAttrib4fv(colorLoc,green);
		
		draw();	
    	}

	void draw()
	{
		vec3 cameraFront=vec3(
				cos(radians(pitch))*sin(radians(yaw)),
				sin(radians(pitch)),
				-cos(radians(pitch))*cos(radians(yaw))
				);

		mat4 view=lookat(cameraPos,
				cameraPos+cameraFront,
				vec3(0,1,0));
		
		mat4 model = translate(modelPos) * rotate(rotationX,rotationY,rotationZ);

		mat4 projection=perspective(60,800.0/600.0,1,-1);

		glUniformMatrix4fv(mvp[0], 1, GL_FALSE, model);
		glUniformMatrix4fv(mvp[1], 1, GL_FALSE, view);
		glUniformMatrix4fv(mvp[2], 1, GL_FALSE, projection);
		glUniform3fv(glGetUniformLocation(program, "cameraPos"), 1,cameraPos);
		glDrawArrays(GL_TRIANGLES, 0, out_vertices.size()); 
		
	}


};

DECLARE_MAIN(Mesh)