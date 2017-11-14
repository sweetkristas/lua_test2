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
#include <memory>
#include <vector>

#include "geometry.hpp"

namespace graphics
{
	class Texture;
	typedef std::unique_ptr<Texture> TexturePtr;

	class Texture
	{
	public:
		Texture();
		explicit Texture(const std::string& filename);
		~Texture();
		void loadFromFile(const std::string& filename);
		void clear();
		void bind();
		TexturePtr clone();
		unsigned id() const { return *id_; }
		int width() const { return src_width_; }
		int height() const { return src_height_; }
	private:
		std::shared_ptr<unsigned> id_;
		std::string filename_;
		int src_width_;
		int src_height_;
	};
}
