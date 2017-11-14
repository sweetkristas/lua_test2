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

#include "object.hpp"

namespace game
{
	Object::Object()
		: tex_()
		, loc_()
		, width_(0)
		, height_(0)
		, tex_rect_()
		, frame_(0)
		, center_()
		, shader_(nullptr)
	{
	}

	Object::~Object()
	{
	}

	void Object::setTexture(const char*filename)
	{
		tex_ = std::make_unique<graphics::Texture>(filename);
		tex_rect_.emplace_back(rect(0, 0, 31, 31));
		width_ = 31;
		height_ = 31;
	}

	void Object::attachShader(graphics::Shader* s) 
	{ 
		shader_ = s;
	}
}
