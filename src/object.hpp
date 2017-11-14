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

#include <memory>
#include <string>
#include <vector>
#include "shader.hpp"
#include "texture.hpp"

class DrawList;

namespace game
{
	class Object;
	typedef std::unique_ptr<Object> ObjectPtr;

	class Object
	{
	public:
		Object();
		~Object();
		void draw(DrawList* drawlist);
		void setTexture(const char*filename);
		void setLocation(int x, int y) { loc_.x = x; loc_.y = y; }
		void attachShader(graphics::Shader* s);
		const graphics::Shader* getShader() const { return shader_; }
		int width() const { return width_; }
		int height() const { return height_; }
	private:
		std::unique_ptr<graphics::Texture> tex_;
		point loc_;
		int width_;
		int height_;
		std::vector<rect> tex_rect_;

		int frame_;
		point center_;		//!< What should be considered the center for rotation/scaling.
		//glm::u8vec4 color_;
		graphics::Shader* shader_;
	};
}
