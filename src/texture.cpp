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

#include "asserts.hpp"
#include "texture.hpp"

#include "GL/gl3w.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

namespace graphics
{
	namespace
	{
		typedef std::map<std::string, std::weak_ptr<unsigned>> texture_cache;

		texture_cache& get_texture_cache()
		{
			static texture_cache res;
			return res;
		}
	}

	Texture::Texture()
		: id_(nullptr),
		  filename_()
	{
	}

	Texture::Texture(const std::string& filename)
		: id_(nullptr),
		  filename_(filename)
	{
		loadFromFile(filename);
	}

	void Texture::loadFromFile(const std::string& filename)
	{
		filename_ = filename;
		// if we're re-using this instance then clear the old texture.
		clear();

		auto it = get_texture_cache().find(filename);
		if(it != get_texture_cache().end()) {
			id_ = it->second.lock();
			if(id_ != nullptr) {
				return;
			}
		}

		int x, y, n;
		unsigned char *data = stbi_load(filename.c_str(), &x, &y, &n, 0);
		ASSERT_LOG(data != nullptr, "No data loading file: {}", filename);
		LOG_INFO("Loaded file {}, size {}x{}, {}-bits per pixel", filename, x, y, n * 8);
		
		GLenum internal_format = GL_RGBA8;
		GLenum format;
		GLenum type = GL_UNSIGNED_BYTE;
		switch(n) {
			case 1:	// 8-bit greyscale	
				format = GL_RED;
				break;
			case 2: // 8-bit greyscale, 8-bit alpha
				format =  GL_RG;
				break;
			case 3: // 8-bit red, 8-bit green, 8-bit blue
				format = GL_RGB;
				break;
			case 4: // 8-bit red, 8-bit green, 8-bit blue, 8-bit alpha
				format = GL_RGBA;
				break;
			default:
				ASSERT_LOG(false, "Unknown number of components per pixel for image {} was detected: {}", filename, n);
		}

		GLuint new_id;
		glGenTextures(1, &new_id);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, new_id);
		glTexImage2D(GL_TEXTURE_2D, 0, internal_format, x, y, 0, format, type, data);

		// todo: set
		// border
		// border color
		// filtering
		// lod bias
		// mipmaps
		// wrap

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		stbi_image_free(data);
		id_ = std::shared_ptr<unsigned>(new unsigned(new_id), [](unsigned* p) { glDeleteTextures(1, p); free(p); });

		get_texture_cache()[filename_] = id_;
	}

	void Texture::clear()
	{
		if(id_ != nullptr) {
			id_.reset();
			filename_.clear();
		}
	}

	void Texture::bind()
	{
		ASSERT_LOG(id_ != nullptr, "Texture is marked invalid.");
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, *id_);
	}

	TexturePtr Texture::clone()
	{
		return std::make_unique<Texture>(*this);
	}

	Texture::~Texture()
	{
		clear();
	}
}
