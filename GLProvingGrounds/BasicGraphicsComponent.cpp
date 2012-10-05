#include "BasicGraphicsComponent.h"

BasicGraphicsComponent::BasicGraphicsComponent(char * vertex, char * tessalationControl, char * tessalationEvaluation,  char * geometry, char * pixel,
	VertexInfo* vInfos, int numVIs, int numVerts, PrimitiveMode pmode)
	: GraphicsComponent(vInfos, numVIs, numVerts, pmode), 
	vertex(vertex), tessalationControl(tessalationControl), tessalationEvaluation(tessalationEvaluation),  geometry(geometry), pixel(pixel)
	
{
	shader = new ShaderProgram();
}
BasicGraphicsComponent::~BasicGraphicsComponent(){
	delete shader;
}

//This should be refactored in order to allow more Per Vertex information.
void BasicGraphicsComponent::Initialize(){
	//Shader program contains up to three different shaders.  Should add tessalator, etc later.
	shader->Initialize(vertex, tessalationControl, tessalationEvaluation, geometry, pixel);
	
	//Tell the shader where the Vertex properties are going to be stored.  (Which array is which)
	for(int i = 0; i < numVInfo; ++i){
		vInfos[i].BindAttribLocation(shader->GetBuffer());
	}
	
	//Link the shaders into a program
	shader->Link();
	//Make is current
	shader->Bind();
	
	//Generate the Vertex Array Object then Bind it.  VAOs are used to store
	//a series of ArrayBuffers and an Element Array Buffer for ease of drawing
	glGenVertexArrays(1, &vao_ID);
	glBindVertexArray(vao_ID);
	//Generate other buffers (vertex, color, normal)
	GLuint* buffers = new GLuint[numVInfo];
	glGenBuffers(numVInfo, buffers);
	for(int i = 0; i < numVInfo; ++i){
		vInfos[i].iBuffer = buffers[i];
		vInfos[i].Initialize();
	}
	//Unbind VAO
	glBindVertexArray(0);

}
void BasicGraphicsComponent::EntitySpecificShaderSetup(){
	return;
}
void BasicGraphicsComponent::Render(){
	shader->Bind();
	
	for(int i = 0; i < numVInfo; ++i){
		vInfos[i].Enable();
	}

	EntitySpecificShaderSetup();
	glBindVertexArray(vao_ID);
	glDrawArrays(primMode, 0, numVerts);
	glBindVertexArray(0);
}
