/* 
	Copyright 2017 Kristina Simpson<sweet.kristas@gmail.com>

	Permission is hereby granted, free of charge, to any person obtaining a 
	copy of this software and associated documentation files (the "Software"), 
	to deal in the Software without restriction, including without 
	limitation the rights to use, copy, modify, merge, publish, distribute, 
	sublicense, and/or sell copies of the Software, and to permit persons to 
	whom the Software is furnished to do so, subject to the following conditions:

		The above copyright notice and this permission notice shall be included 
		in all copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL 
	THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
	FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
	DEALINGS IN THE SOFTWARE.
*/
#pragma once

#include <string>
#include <vector>

#include "GL/gl3w.h"


namespace graphics
{
	struct shader_descriptor 
	{
		shader_descriptor(GLenum type, const std::string& code) : shader_type(type), shader_code(code) {}
		GLenum shader_type;
		std::string shader_code;
	};

	class Shader
	{
	public:
		Shader(const std::string& name, const std::vector<shader_descriptor>& desc);
		~Shader() {}
		static Shader* getShader(const std::string& name);
		GLuint compile(GLenum type, const std::string& code);
		bool link(const std::vector<shader_descriptor>& desc, const std::vector<GLuint>& ids);
		const std::string& name() const { return name_; }
		void apply() const { 
			ASSERT_LOG(object_ != 0, "Generation of shader program has not been completed.");
			glUseProgram(object_); 
		}
		GLint getUniformId(const std::string& id);
	private:
		std::string name_;
		GLuint object_;
		Shader() = delete;
	};
}