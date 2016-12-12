#include "Shader.h"

Shader::Shader(string vFile, string fFile, string gFile)	{
	cout << "Compiling Shader..." << endl;

	program		= glCreateProgram();
	objects[SHADER_VERTEX]		= GenerateShader(vFile	 ,GL_VERTEX_SHADER);
	objects[SHADER_FRAGMENT]	= GenerateShader(fFile,GL_FRAGMENT_SHADER);
	objects[SHADER_GEOMETRY]	= 0;

	if(!gFile.empty()) {
		objects[SHADER_GEOMETRY]	= GenerateShader(gFile,GL_GEOMETRY_SHADER);
		glAttachShader(program,objects[SHADER_GEOMETRY]);
	}

	glAttachShader(program,objects[SHADER_VERTEX]);
	glAttachShader(program,objects[SHADER_FRAGMENT]);

	SetDefaultAttributes();
}

Shader::~Shader(void)	{
	for(int i = 0; i < 3; ++i) {
		glDetachShader(program, objects[i]);
		glDeleteShader(objects[i]);
	}
	glDeleteProgram(program);
}

bool	Shader::LoadShaderFile(string from, string &into)	{
	ifstream	file;
	string		temp;

	file.open(from.c_str());
	if(!file.is_open()){
		cout << "\t Error: File does not exist!" << endl;
		return false;
	}

	while(!file.eof()){
		getline(file,temp);
		into += temp + "\n";
	}

	//cout << into << endl << endl;

	file.close();
	//cout << "Loaded shader text!" << endl;
	return true;
}

GLuint	Shader::GenerateShader(string from, GLenum type)	{
	cout << "\t-> Compiling Shader: " << from << endl;

	string load;
	if(!LoadShaderFile(from,load)) {
		loadFailed = true;
		return 0;
	}

	GLuint shader = glCreateShader(type);

	const char *chars = load.c_str();
	glShaderSource(shader, 1, &chars, NULL);
	glCompileShader(shader);

	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

	if (status == GL_FALSE)	{
		cout << "\t Error: Compiling failed!" << endl;
		char error[512];
		glGetInfoLogARB(shader, sizeof(error), NULL, error);
		cout << error << endl << endl;
		loadFailed = true;
		cout << endl;
		return 0;
	}

	loadFailed = false;
	return shader;
}

bool Shader::LinkProgram()	{
	if(loadFailed) {
		return false;
	}
	glLinkProgram(program); 
	cout << "\t-> Linking Shader: ";

	GLint code;
	glGetProgramiv(program, GL_LINK_STATUS, &code);

	if (code == GL_FALSE)	{
		cout << "Failed!" << endl;
		char error[512];
		glGetInfoLogARB(program, sizeof(error), NULL, error);
		cout << error << endl << endl;
		loadFailed = true;
		return false;
	}
	
	cout << "Success!" << endl << endl;
	return true;
}

void	Shader::SetDefaultAttributes()	{
	glBindAttribLocation(program, VERTEX_BUFFER,  "position");
	glBindAttribLocation(program, COLOUR_BUFFER,  "colour");
	glBindAttribLocation(program, NORMAL_BUFFER,  "normal");
	glBindAttribLocation(program, TANGENT_BUFFER, "tangent");
	glBindAttribLocation(program, TEXTURE_BUFFER, "texCoord");

	glBindAttribLocation(program, MAX_BUFFER+1,  "transformIndex");
}