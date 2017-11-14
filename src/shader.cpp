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

#include <map>
#include <memory>

#include "asserts.hpp"
#include "shader.hpp"

namespace
{
	const std::string basic_vert_shader = 
		"#version 330 core\n"
		"layout (location = 0) in vec2 position;\n"
		"layout (location = 1) in vec2 texcoord;\n"
		"layout (location = 2) in vec2 normal;\n"
		"layout (location = 3) in vec4 color;\n"
		"uniform mat4 u_projmatrix;\n"
		"out vec2 v_texcoord;\n"
		"out vec2 v_normal;\n"
		"out vec4 v_color;\n"
		"void main()\n"
		"{\n"
		"    gl_Position = u_projmatrix * vec4(position,0.0,1.0);\n"
		"    v_texcoord = texcoord;\n"
		"    v_normal = normal;\n"
		"    v_color = color;\n"
		"}\n";
	const std::string basic_frag_shader =
		"#version 330 core\n"
		"in vec2 v_texcoord;\n"
		"in vec2 v_normal;\n"
		"in vec4 v_color;\n"
		//"uniform vec4 u_color;\n"
		"uniform sampler2D u_tex;\n"
		"out vec4 out_color;\n"
		"void main()\n"
		"{\n"
		"    vec4 tc = texture(u_tex, v_texcoord);\n"
		"    out_color = tc;\n" // v_color * u_color * 
		"}\n";

}

namespace graphics
{
	namespace 
	{
		typedef std::map<std::string, std::unique_ptr<Shader>> ShaderMap;
		ShaderMap& get_shadermap()
		{
			static ShaderMap res;
			return res;
		}

		void load_shaders_from_file()
		{
			if(get_shadermap().empty()) {
				std::vector<shader_descriptor> desc{ 
					{ GL_VERTEX_SHADER, basic_vert_shader }, 
					{ GL_FRAGMENT_SHADER, basic_frag_shader } 
				};
				get_shadermap().emplace("basic", std::make_unique<Shader>("basic", desc));
			}
		}
	}

	Shader::Shader(const std::string& name, const std::vector<shader_descriptor>& descriptors)
		: name_(name),
		  object_(0)
	{
		std::vector<GLuint> shader_ids;
		for(const auto& desc : descriptors) {
			GLuint shaderp = compile(desc.shader_type, desc.shader_code);
			ASSERT_LOG(shaderp != 0, "In '{}' Shader, shader of type: {} didn't compile.", name_, desc.shader_type);
			shader_ids.emplace_back(shaderp);
		}
		bool linked_ok = link(descriptors, shader_ids);
		ASSERT_LOG(linked_ok, "Error linking shader program: {}", name_);
	}

	Shader* Shader::getShader(const std::string& name)
	{
		load_shaders_from_file();
		ShaderMap& sm = get_shadermap();
		auto shader = sm.find(name);
		ASSERT_LOG(shader != sm.end(), "Shader named {} not found.", name);
		return shader->second.get();
	}

	GLuint Shader::compile(GLenum type, const std::string& code)
	{
		GLint compiled;

		ASSERT_LOG(glCreateShader != nullptr, "Something bad happened with Glew shader not initialised.");
		GLuint shader = glCreateShader(type);
		if(shader == 0) {
			LOG_ERROR("Unable to create shader: {}", name());
			return shader;
		}
		const char* shader_code = code.c_str();
		glShaderSource(shader, 1, &shader_code, nullptr);
		glCompileShader(shader);
		glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
		if(!compiled) {
			GLint info_len = 0;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &info_len);
			if(info_len > 1) {
				std::vector<char> info_log;
				info_log.resize(info_len);
				glGetShaderInfoLog(shader, static_cast<GLsizei>(info_log.capacity()), nullptr, &info_log[0]);
				std::string s(info_log.begin(), info_log.end());
				LOG_ERROR("Error compiling shader({}): {}", name(), s);
			}
			glDeleteShader(shader);
			shader = 0;
		}
		return shader;
	}

	bool Shader::link(const std::vector<shader_descriptor>& desc, const std::vector<GLuint>& ids)
	{
		if(object_ != 0) {
			glDeleteProgram(object_);
		}
		object_ = glCreateProgram();
		ASSERT_LOG(object_ != 0, "Unable to create program object.");
		for(const auto id : ids) {
			glAttachShader(object_, id);
		}
		glLinkProgram(object_);
		GLint linked = 0;
		glGetProgramiv(object_, GL_LINK_STATUS, &linked);
		if(!linked) {
			GLint info_len = 0;
			glGetProgramiv(object_, GL_INFO_LOG_LENGTH, &info_len);
			if(info_len > 1) {
				std::vector<char> info_log;
				info_log.resize(info_len);
				glGetProgramInfoLog(object_, static_cast<GLsizei>(info_log.capacity()), nullptr, &info_log[0]);
				std::string s(info_log.begin(), info_log.end());
				LOG_ERROR("Error linking object: {}", s);
			}
			glDeleteProgram(object_);
			object_ = 0;
			return false;
		}
		return true;
	}

	GLint Shader::getUniformId(const std::string& id) const
	{
		auto location = glGetUniformLocation(object_, id.c_str());
		ASSERT_LOG(location != -1, "Failed to get uniform '{}' for program '{}'", id, name());
		return location;
	}
}
