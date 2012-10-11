#ifndef  UNIFORMBUFFEROBJECT_H
#define  UNIFORMBUFFEROBJECT_H
#include "Macros.h"
#include <vector>

struct UBOinterface{
	virtual void BindToShader(unsigned int programBuffer) = 0;
};

struct UBOStatics{
private:
	static UBOStatics * instance;
	UBOStatics()
		:index(0)
	{}
public:
	static UBOStatics* Get();
	int index;
	std::vector<UBOinterface *> buffers;
	std::vector<UBOinterface *>& GetBuffers();
	void BindAll(unsigned int program);
};

template <class dataStructure>
struct UniformBufferObject : public UBOinterface{
	unsigned int		uBOBindingIndex;
	unsigned int		uboIndex;
	char *				name;
	UniformBufferObject(){
		
		uBOBindingIndex = UBOStatics::Get()->index++;
		UBOStatics::Get()->buffers.push_back(this);
		Initialize();
	}
	~UniformBufferObject(){
		bool found = false;
		std::vector<UBOinterface *> allBuffers = UBOStatics::Get()->GetBuffers();
		for(auto it = allBuffers.begin(); it != allBuffers.end(); it++){
			if((*it)  == this){
				(*it) = allBuffers[allBuffers.size() -1];
				found = true;
				break;
			}
		}
		if(found)
			allBuffers.pop_back();
		if(name != NULL){
			delete [] name;
			name = NULL;
		}
		GLCALL(glDeleteBuffers(1, &uboIndex));
	}
	void Initialize(){
		GLCALL(glGenBuffers(1, &uboIndex));
		GLCALL(glBindBuffer(GL_UNIFORM_BUFFER, uboIndex));
		GLCALL(glBufferData(GL_UNIFORM_BUFFER, sizeof(dataStructure), NULL, GL_STREAM_DRAW));
		GLCALL(glBindBuffer(GL_UNIFORM_BUFFER, 0));
		GLCALL(glBindBufferRange(GL_UNIFORM_BUFFER, uBOBindingIndex, uboIndex, 0, sizeof(dataStructure)));
	}

	void BindToShader(unsigned int programBuffer){
		unsigned int uniformBlockIndex = GLCALL(glGetUniformBlockIndex(programBuffer, name));
		if(uniformBlockIndex == GL_INVALID_INDEX)
			return;
		GLCALL(glUniformBlockBinding(programBuffer, uniformBlockIndex, uBOBindingIndex));
	}
	void Update(dataStructure* data){
		GLCALL(glBindBuffer(GL_UNIFORM_BUFFER, uboIndex));
		GLCALL(glBufferData(GL_UNIFORM_BUFFER, sizeof(dataStructure), data, GL_STREAM_DRAW));
	}
};
#endif //UNIFORMBUFFEROBJECT_H
