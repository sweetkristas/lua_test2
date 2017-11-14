/*
#define SOL_CHECK_ARGUMENTS 1
#include "sol.hpp"

#include <iostream>
#include <cassert>
#include <cmath>

// Note that this is a bunch of if/switch statements
// for the sake of brevity and clarity
// A much more robust implementation could use a std::unordered_map
// to link between keys and desired actions for those keys on
// setting/getting
// The sky becomes the limit when you have this much control,
// so apply it wisely!

struct vec {
	double x;
	double y;

	vec() : x(0), y(0) {}
	vec(double x) : vec(x, x) {}
	vec(double x, double y) : x(x), y(y) {}

	sol::object getter(sol::stack_object key, sol::this_state L) {
		// we use stack_object for the arguments because we know
		// the values from Lua will remain on Lua's stack,
		// so long we we don't mess with it
		auto maybe_string_key = key.as<sol::optional<std::string>>();
		if (maybe_string_key) {
			const std::string& k = *maybe_string_key;
			if (k == "x") {
				// Return x
				return sol::object(L, sol::in_place, this->x);
			}
			else if (k == "y") {
				// Return y
				return sol::object(L, sol::in_place, this->y);
			}
		}

		// String keys failed, check for numbers
		auto maybe_numeric_key = key.as<sol::optional<int>>();
		if (maybe_numeric_key) {
			int n = *maybe_numeric_key;
			switch (n) {
			case 0:
				return sol::object(L, sol::in_place, this->x);
			case 1:
				return sol::object(L, sol::in_place, this->y);
			default:
				break;
			}
		}

		// No valid key: push nil
		// Note that the example above is a bit unnecessary:
		// binding the variables x and y to the usertype
		// would work just as well and we would not
		// need to look it up here,
		// but this is to show it works regardless
		return sol::object(L, sol::in_place, sol::lua_nil);
	}

	void setter(sol::stack_object key, sol::stack_object value, sol::this_state) {
		// we use stack_object for the arguments because we know
		// the values from Lua will remain on Lua's stack,
		// so long we we don't mess with it
		auto maybe_string_key = key.as<sol::optional<std::string>>();
		if (maybe_string_key) {
			const std::string& k = *maybe_string_key;
			if (k == "x") {
				// set x
				this->x = value.as<double>();
			}
			else if (k == "y") {
				// set y
				this->y = value.as<double>();
			}
		}

		// String keys failed, check for numbers
		auto maybe_numeric_key = key.as<sol::optional<int>>();
		if (maybe_numeric_key) {
			int n = *maybe_numeric_key;
			switch (n) {
			case 0:
				this->x = value.as<double>();
				break;
			case 1:
				this->y = value.as<double>();
				break;
			default:
				break;
			}
		}
	}
};

void sol_test()
{
	std::cout << "=== usertype dynamic getter/setter example ===" << std::endl;

	sol::state lua;
	lua.open_libraries();

	lua.new_usertype<vec>("vec",
		sol::constructors<vec(), vec(double), vec(double, double)>(),
		// index and newindex control what happens when a "key"
		// is looked up that is not baked into the class itself
		// it is called with the object and the key for index (get)s
		// or it is called with the object, the key, and the index (set)
		// we can use a member function to assume the "object" is of the `vec`
		// type, and then just have a function that takes 
		// the key (get) or the key + the value (set)
		sol::meta_function::index, &vec::getter,
		sol::meta_function::new_index, &vec::setter
		);

	lua.script(R"(
		v1 = vec.new(1, 0)
		v2 = vec.new(0, 1)
		
		-- observe usage of getter/setter
		print("v1:", v1.x, v1.y)
		print("v2:", v2.x, v2.y)
		-- gets 0, 1 by doing lookup into getter function
		print("changing v2...")
		v2.x = 3
		v2[1] = 5
		-- can use [0] [1] like popular
		-- C++-style math vector classes
		print("v1:", v1.x, v1.y)
		print("v2:", v2.x, v2.y)
		-- both obj.name and obj["name"]
		-- are equivalent lookup methods
		-- and both will trigger the getter
		-- if it can't find 'name' on the object
		assert(v1["x"] == v1.x)
		assert(v1[0] == v1.x)
		assert(v1["x"] == v1[0])
		assert(v1["y"] == v1.y)
		assert(v1[1] == v1.y)
		assert(v1["y"] == v1[1])
	)");


	// Can also be manipulated from C++,
	// and will call getter/setter methods:
	sol::userdata v1 = lua["v1"];
	double v1x = v1["x"];
	double v1y = v1["y"];
	assert(v1x == 1.000);
	assert(v1y == 0.000);
	v1[0] = 2.000;

	lua.script(R"(
		assert(v1.x == 2.000)
		assert(v1["x"] == 2.000)
		assert(v1[0] == 2.000)
	)");

	std::cout << std::endl;
}
*/

#include <cstdint>
#include <vector>
#include <string>
#include <memory>

#include "asserts.hpp"
#include "filesystem.hpp"
#include "shader.hpp"
#include "texture.hpp"

#include "theme_imgui.hpp"
#include "imgui.h"
#include "imgui_impl_sdl_gl3.h"
#include "IconsFontAwesome.h"
#include "IconsMaterialDesign.h"
#include "imgui_utils.hpp"
#include "TextEditor.h"
#include "object.hpp"

#include "spdlog/spdlog.h"
#include "SDL.h"

#include "GL/gl3w.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#ifdef _DEBUG
#pragma comment(lib, "SDL2maind")
#pragma comment(lib, "SDL2-staticd")
#else
#pragma comment(lib, "SDL2main")
#pragma comment(lib, "SDL2-static")
#endif

//#pragma comment(lib, "glew32s")

#include <iostream>
#include "rapidjson/reader.h"
using namespace rapidjson;

namespace
{
	uint32_t next_pow2(uint32_t v) 
	{
		--v;
		v |= v >> 1;
		v |= v >> 2;
		v |= v >> 4;
		v |= v >> 8;
		v |= v >> 16;
		return ++v;
	}
}

struct MyHandler {
    bool Null() { std::cout << "Null()" << std::endl; return true; }
    bool Bool(bool b) { std::cout << "Bool(" << std::boolalpha << b << ")" << std::endl; return true; }
    bool Int(int i) { std::cout << "Int(" << i << ")" << std::endl; return true; }
    bool Uint(unsigned u) { std::cout << "Uint(" << u << ")" << std::endl; return true; }
    bool Int64(int64_t i) { std::cout << "Int64(" << i << ")" << std::endl; return true; }
    bool Uint64(uint64_t u) { std::cout << "Uint64(" << u << ")" << std::endl; return true; }
    bool Double(double d) { std::cout << "Double(" << d << ")" << std::endl; return true; }
    bool RawNumber(const char* str, SizeType length, bool copy) { 
        std::cout << "Number(" << str << ", " << length << ", " << std::boolalpha << copy << ")" << std::endl;
        return true;
    }
    bool String(const char* str, SizeType length, bool copy) { 
        std::cout << "String(" << str << ", " << length << ", " << std::boolalpha << copy << ")" << std::endl;
        return true;
    }
    bool StartObject() { std::cout << "StartObject()" << std::endl; return true; }
    bool Key(const char* str, SizeType length, bool copy) {
        std::cout << "Key(" << str << ", " << length << ", " << std::boolalpha << copy << ")" << std::endl;
        return true;
    }
    bool EndObject(SizeType memberCount) { std::cout << "EndObject(" << memberCount << ")" << std::endl; return true; }
    bool StartArray() { std::cout << "StartArray()" << std::endl; return true; }
    bool EndArray(SizeType elementCount) { std::cout << "EndArray(" << elementCount << ")" << std::endl; return true; }
};

void json_test()
{
    const char json[] = " { \"hello\" : \"world\", \"t\" : true , \"f\" : false, \"n\": null, \"i\":123, \"pi\": 3.1416, \"a\":[1, 2, 3, 4], } ";

    MyHandler handler;
    Reader reader;
    StringStream ss(json);
    reader.Parse(ss, handler);
}

struct SDL2Exception : public std::runtime_error
{
	SDL2Exception(const char* err) : std::runtime_error(err) {}
	SDL2Exception(const std::string& err) : std::runtime_error(err) {}
	~SDL2Exception() {}
};

ImFont* g_mono_font_16;
ImFont* g_roboto_16;
ImFont* g_font_awesome_16;
ImFont* g_material_icons_16;

void imgui_config_fonts()
{
	ImGuiIO& io = ImGui::GetIO();
	//io.Fonts->AddFontDefault();
	ImFontConfig config;
	config.OversampleH = 3;
	config.OversampleV = 1;
	config.GlyphExtraSpacing.x = 1.0f;
	g_roboto_16 = io.Fonts->AddFontFromFileTTF("..\\data\\fonts\\Roboto\\Roboto-Medium.ttf", 16.0f, &config);

	// merge in icons from Font Awesome
	static const ImWchar fa_icons_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
	ImFontConfig icons_config; icons_config.MergeMode = true; icons_config.PixelSnapH = true;
	g_font_awesome_16 = io.Fonts->AddFontFromFileTTF("..\\data\\fonts\\fontawesome-webfont.ttf", 16.0f, &icons_config, fa_icons_ranges);
	// merge in google material design icons
	static const ImWchar md_icons_ranges[] = { ICON_MIN_MD, ICON_MAX_MD, 0 };
	//ImFontConfig icons_config; icons_config.MergeMode = true; icons_config.PixelSnapH = true;
	g_material_icons_16 = io.Fonts->AddFontFromFileTTF("..\\data\\fonts\\MaterialIcons-Regular.ttf", 16.0f, &icons_config, md_icons_ranges);

	g_mono_font_16 = io.Fonts->AddFontFromFileTTF("..\\data\\fonts\\Inconsolata\\Inconsolata-Regular.ttf", 16.0f);

	/// in an imgui window somewhere...
	///ImGui::Text( ICON_FA_FILE "  File" ); // use string literal concatenation, ouputs a file icon and File as a string.
}


class Window
{
public:
	Window() 
		: x_(SDL_WINDOWPOS_CENTERED), 
		  y_(SDL_WINDOWPOS_CENTERED), 
		  width_(1024), 
		  height_(768), 
		  flags_(0), 
		  title_(), 
		  window_(nullptr), 
		  renderer_(nullptr),
		  context_(nullptr),
		  actual_width_(0), 
		  actual_height_(0),
		  log_window_events_(false),
		  fullscreen_(false),
		  fullscreen_desktop_(false),
		  multi_sample_enable_(false),
		  multi_sample_(16)
	{
	}
	void setFlags(unsigned f) { flags_ = f; }
	void init(const std::string& title, int x, int y, int width, int height, unsigned flags) {
		title_ = title;
		x_ = x;
		y_ = y;
		width_ = width;
		height_ = height;
		flags_ = flags;
		init();
	}
	void init() {
		SDL_GL_SetAttribute (SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
		SDL_GL_SetAttribute (SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
		//SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

		//if(use16bpp()) {
		//	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
		//	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
		//	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
		//	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 1);
		//} else {
			SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
			SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
			SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
			SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
		//}
		if(multi_sample_enable_) {
			if(SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1) != 0) {
				LOG_WARN("MSAA({}) requested but mutlisample buffer couldn't be allocated.", getMultiSamples());
			} else {
				int msaa = next_pow2(getMultiSamples());
				if(SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, msaa) != 0) {
					LOG_INFO("Requested MSAA of {} but couldn't allocate", msaa);
				}
			}
		}

		unsigned flags = flags_;
		if(fullscreen_) {
			flags |= SDL_WINDOW_FULLSCREEN;
		}
		if(fullscreen_desktop_) {
			flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
		}
		flags |= SDL_WINDOW_OPENGL;

		if((window_ = SDL_CreateWindow(title_.c_str(), x_, y_, width_, height_, flags)) == nullptr) {
			throw SDL2Exception(fmt::format("Could not create window: {}\n", SDL_GetError()));
		}
		/*renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED);
		if(renderer_ == nullptr) {
			SDL_DestroyWindow(window_);
			window_ = nullptr;
			throw SDL2Exception(fmt::format("Could not create renderer: {}\n", SDL_GetError()));
		}*/

		context_ = SDL_GL_CreateContext(window_);	

        auto gl3w_err = gl3wInit();
		ASSERT_LOG(gl3w_err == 0, "Unable to initialise GL3W: {}", gl3w_err);
        gl3w_err = gl3wIsSupported(3, 3);
		ASSERT_LOG(gl3w_err != 0, "OpenGL 3.3 is not supported. {}", gl3w_err);

		//auto glew_err = glewInit();
		//ASSERT_LOG(glew_err == GLEW_OK, "Unable to initialise GLEW: {:s}", glewGetErrorString(glew_err));

		SDL_GL_GetDrawableSize(window_, &actual_width_, &actual_height_);
		glViewport(0, 0, actual_width_, actual_height_);

		ImGui_ImplSdlGL3_Init(window_);
		// need to do imgui font configuration before call xxx_NewFrame.
		imgui_config_fonts();

		printAttributes();
		printExtensions();
	}
	void clear() {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	}
	int getRequestedWidth() const { return width_; }
	int getRequestedHeight() const { return height_; }
	int getWidth() const { return actual_width_; }
	int getHeight() const { return actual_height_; }
	void setWindowSize(int width, int height) {
		ASSERT_LOG(window_ != nullptr, "Internal window was null");
		width_ = width;
		height_ = height;
		if(fullscreen_) {
			// xxx: todo
			//SDL_SetWindowDisplayMode(window_, mode);
			LOG_ERROR("XXX setting window size for fullscreen windows is currently broken, please fix.");
		} else {
			SDL_SetWindowSize(window_, width, height);
		}
	}
	void newFrame() {
		clear();
		ImGui_ImplSdlGL3_NewFrame(window_);
	}
	void swap() {
		ImGui::Render();

		ASSERT_LOG(window_ != nullptr, "Internal window was null");
		SDL_GL_SwapWindow(window_);	
	}
	void setClearColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
		glClearColor(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
	}
	bool handleWindowEvent(SDL_Event* ev) {
		ASSERT_LOG(window_ != nullptr, "Internal window was null");
		// Is this event for this window?
		auto this_id = SDL_GetWindowID(window_);
		if(this_id != ev->window.windowID) {
			return true;
		}
	    if (ev->type == SDL_WINDOWEVENT) {
			switch (ev->window.event) {
			case SDL_WINDOWEVENT_SHOWN:
				if(log_window_events_) {
					LOG_INFO("Window {} shown", ev->window.windowID);
				}
				break;
			case SDL_WINDOWEVENT_HIDDEN:
				if(log_window_events_) {
					LOG_INFO("Window {} hidden", ev->window.windowID);
				}
				break;
			case SDL_WINDOWEVENT_EXPOSED:
				if(log_window_events_) {
					LOG_INFO("Window {} exposed", ev->window.windowID);
				}
				break;
			case SDL_WINDOWEVENT_MOVED:
				x_ = ev->window.data1;
				y_ = ev->window.data2;
				if(log_window_events_) {
					LOG_INFO("Window {} moved: {}, {}", ev->window.windowID, ev->window.data1, ev->window.data2);
				}
				break;
			case SDL_WINDOWEVENT_RESIZED:
				actual_width_ = ev->window.data1;
				actual_height_ = ev->window.data2;
				if(log_window_events_) {
					LOG_INFO("Window {} resized {}x{}", ev->window.windowID, ev->window.data1, ev->window.data2);
				}
				SDL_GL_GetDrawableSize(window_, &actual_width_, &actual_height_);
				glViewport(0, 0, actual_width_, actual_height_);
				break;
			case SDL_WINDOWEVENT_SIZE_CHANGED:
				// we're handling the resized event rather than the changed event.
				if(log_window_events_) {
					LOG_INFO("Window {} size changed {}x{}", ev->window.windowID, ev->window.data1, ev->window.data2);
				}
				break;
			case SDL_WINDOWEVENT_MINIMIZED:
				if(log_window_events_) {
					LOG_INFO("Window {} minimized", ev->window.windowID);
				}
				break;
			case SDL_WINDOWEVENT_MAXIMIZED:
				if(log_window_events_) {
					LOG_INFO("Window {} maximized", ev->window.windowID);
				}
				break;
			case SDL_WINDOWEVENT_RESTORED:
				if(log_window_events_) {
					LOG_INFO("Window {} restored", ev->window.windowID);
				}
				break;
			case SDL_WINDOWEVENT_ENTER:
				if(log_window_events_) {
					LOG_INFO("Mouse entered window {}", ev->window.windowID);
				}
				break;
			case SDL_WINDOWEVENT_LEAVE:
				if(log_window_events_) {
					LOG_INFO("Mouse left window {}", ev->window.windowID);
				}
				break;
			case SDL_WINDOWEVENT_FOCUS_GAINED:
				if(log_window_events_) {
					LOG_INFO("Window {} gained keyboard focus", ev->window.windowID);
				}
				break;
			case SDL_WINDOWEVENT_FOCUS_LOST:
				if(log_window_events_) {
					LOG_INFO("Window {} lost keyboard focus", ev->window.windowID);
				}
				break;
			case SDL_WINDOWEVENT_CLOSE:
				if(log_window_events_) {
					LOG_INFO("Window {} closed", ev->window.windowID);
				}
				return false;
			case SDL_WINDOWEVENT_TAKE_FOCUS:
				if(log_window_events_) {
					LOG_INFO("Window {} is offered a focus", ev->window.windowID);
				}
				break;
			case SDL_WINDOWEVENT_HIT_TEST:
				if(log_window_events_) {
					LOG_INFO("Window {} has a special hit test", ev->window.windowID);
				}
				break;
			default:
				LOG_INFO("Window {} got unknown ev {}", ev->window.windowID, ev->window.event);
				break;
			}
		}
		return true;
	}
	void printAttributes() {
#define MAKE_STRING(s) #s
		// n.b. that between different SDL versions these attribute numbers may change.
		std::vector<std::string> attr_table{
			MAKE_STRING(SDL_GL_RED_SIZE),
			MAKE_STRING(SDL_GL_GREEN_SIZE),
			MAKE_STRING(SDL_GL_BLUE_SIZE),
			MAKE_STRING(SDL_GL_ALPHA_SIZE),
			MAKE_STRING(SDL_GL_BUFFER_SIZE),
			MAKE_STRING(SDL_GL_DOUBLEBUFFER),
			MAKE_STRING(SDL_GL_DEPTH_SIZE),
			MAKE_STRING(SDL_GL_STENCIL_SIZE),
			MAKE_STRING(SDL_GL_ACCUM_RED_SIZE),
			MAKE_STRING(SDL_GL_ACCUM_GREEN_SIZE),
			MAKE_STRING(SDL_GL_ACCUM_BLUE_SIZE),
			MAKE_STRING(SDL_GL_ACCUM_ALPHA_SIZE),
			MAKE_STRING(SDL_GL_STEREO),
			MAKE_STRING(SDL_GL_MULTISAMPLEBUFFERS),
			MAKE_STRING(SDL_GL_MULTISAMPLESAMPLES),
			MAKE_STRING(SDL_GL_ACCELERATED_VISUAL),
			MAKE_STRING(SDL_GL_RETAINED_BACKING),
			MAKE_STRING(SDL_GL_CONTEXT_MAJOR_VERSION),
			MAKE_STRING(SDL_GL_CONTEXT_MINOR_VERSION),
			MAKE_STRING(SDL_GL_CONTEXT_EGL),
			MAKE_STRING(SDL_GL_CONTEXT_FLAGS),
			MAKE_STRING(SDL_GL_CONTEXT_PROFILE_MASK),
			MAKE_STRING(SDL_GL_SHARE_WITH_CURRENT_CONTEXT),
			MAKE_STRING(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE),
			MAKE_STRING(SDL_GL_CONTEXT_RELEASE_BEHAVIOR),
			MAKE_STRING(SDL_GL_CONTEXT_RESET_NOTIFICATION),
			MAKE_STRING(SDL_GL_CONTEXT_NO_ERROR),
		};
		for(int attr = SDL_GL_RED_SIZE; attr != SDL_GL_CONTEXT_NO_ERROR + 1; attr++) {
			// querying the SDL_GL_ACCUM_xxx_SIZE variables triggers OpenGL errors, so we skip.
			if(attr != SDL_GL_ACCUM_RED_SIZE && attr != SDL_GL_ACCUM_GREEN_SIZE 
				&& attr != SDL_GL_ACCUM_BLUE_SIZE && attr != SDL_GL_ACCUM_ALPHA_SIZE) {
				int value;
				SDL_GL_GetAttribute(static_cast<SDL_GLattr>(attr), &value);
				LOG_INFO("{} : {}", attr_table[attr], value);
			}
		}
	}
	void printExtensions()
	{
		GLint max_ext;
		glGetIntegerv(GL_NUM_EXTENSIONS, &max_ext);
		std::stringstream ext_str;
		for(int n = 0; n != max_ext; ++n) {
			auto ext = glGetStringi(GL_EXTENSIONS, n);
			ext_str << ext << " ";
		}	
		LOG_INFO("Extensions: {}", ext_str.str());
	}
	~Window() {
		ImGui_ImplSdlGL3_Shutdown();
		if(renderer_) {
			SDL_DestroyRenderer(renderer_);
			renderer_ = nullptr;
		}
		if(context_) {
			SDL_GL_DeleteContext(context_);
			context_ = nullptr;
		}
		if(window_) {
			SDL_DestroyWindow(window_);
			window_ = nullptr;
		}
	}
	int	getMultiSamples() const { return multi_sample_; }
private:
	int x_;
	int y_;
	int width_;
	int height_;
	unsigned flags_;
	std::string title_;
	SDL_Window* window_;
	SDL_Renderer *renderer_;
	SDL_GLContext context_;

	int actual_width_;
	int actual_height_;

	bool log_window_events_;
	bool fullscreen_;
	bool fullscreen_desktop_;
	bool multi_sample_enable_;

	int multi_sample_;
};

class SDL2;
typedef std::unique_ptr<SDL2> SDL2Ptr;
class SDL2
{
public:
	static SDL2Ptr Init(Uint32 flags) {
		return std::make_unique<SDL2>(flags);
	}
	~SDL2() {
		SDL_Quit();
	}
	SDL2(Uint32 flags) {
		int sdl_error = SDL_Init(flags);
		if(sdl_error != 0) {			
			throw SDL2Exception(fmt::format("Unable to initialize SDL: {}\n", SDL_GetError()));
		}
	}
	static std::unique_ptr<Window> CreateWin(const std::string& title, int x, int y, int width, int height) {
		auto wnd = std::make_unique<Window>();
		wnd->init(title, x, y, width, height, SDL_WINDOW_RESIZABLE /*| SDL_WINDOW_ALLOW_HIGHDPI*/);
		return wnd;
	}
private:
	SDL2() = delete;
};

static GLuint vao;
static bool g_theme_imgui_ui = false;
static bool g_show_fps = false;
static bool g_show_text_editor = false;
static bool g_show_main_menu_bar = true;

void test1()
{
	float positions[] = {
		-0.5f, -0.5f,		0.0f, 1.0f,
		 0.5f, -0.5f,		1.0f, 1.0f,
		 0.5f,  0.5f,		1.0f, 0.0f,
		-0.5f,  0.5f,		0.0f, 0.0f,
	};
	unsigned short indicies[] = {
		0, 1, 2,
		2, 3, 0
	};

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	unsigned vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);

	const intptr_t stride = sizeof(float) * 4;
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, 0);
	const intptr_t offset = sizeof(float) * 2;
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<const GLvoid*>(offset));

	uint32_t ibo;
	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indicies), indicies, GL_STATIC_DRAW);

	// need to unbind the vertax array object before the buffers, because the VAO 
	// *will* remember the last item bound.
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void init_text_editor(TextEditor *editor, const char* filename)
{
	auto lang = TextEditor::LanguageDefinition::Lua();

	//// set your own known preprocessor nanmes
	//static const char* ppnames[] = { "NULL", "PM_REMOVE",
	//	"ZeroMemory", "DXGI_SWAP_EFFECT_DISCARD", "D3D_FEATURE_LEVEL", "D3D_DRIVER_TYPE_HARDWARE", "WINAPI","D3D11_SDK_VERSION",
	//	"WM_SIZE", "SIZE_MINIMIZED", "SC_KEYMENU", "WNDCLASSEX", "CS_CLASSDC", "WM_SIZE", "WM_SYSCOMMAND", "WM_DESTROY", "IDC_ARROW", "WS_OVERLAPPEDWINDOW",
	//	"WM_QUIT", "D3D11_RTV_DIMENSION_TEXTURE2D", "DIRECTINPUT_VERSION", "SW_SHOWDEFAULT", "S_OK", "E_FAIL", "D3D_FEATURE_LEVEL_11_0",
	//	"DXGI_FORMAT_R8G8B8A8_UNORM", "DXGI_USAGE_RENDER_TARGET_OUTPUT", "DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH", "DXGI_FORMAT_R8G8B8A8_UNORM", "TRUE" };
	//for (auto i : ppnames) {
	//	lang.mPreprocIdentifiers.insert(i);
	//}

	//// set your own identifiers
	//static const char* identifiers[] = {
	//	"HRESULT", "LPRESULT","D3D11_RENDER_TARGET_VIEW_DESC", "HWND", "DXGI_SWAP_CHAIN_DESC","MSG","LRESULT","WPARAM", "LPARAM","UINT","LPVOID",
	//	"ID3D11Device", "ID3D11DeviceContext", "ID3D11Buffer", "ID3D11Buffer", "ID3D10Blob", "ID3D11VertexShader", "ID3D11InputLayout", "ID3D11Buffer",
	//	"ID3D10Blob", "ID3D11PixelShader", "ID3D11SamplerState", "ID3D11ShaderResourceView", "ID3D11RasterizerState", "ID3D11BlendState", "ID3D11DepthStencilState",
	//	"IDXGISwapChain", "ID3D11RenderTargetView", "ID3D11Texture2D", };
	//for (auto i : identifiers) {
	//	lang.mIdentifiers.insert(i);
	//}

	editor->SetLanguageDefinition(lang);

	//TextEditor::ErrorMarkers markers;
	//markers.insert(std::make_pair<int, std::string>(14, "Example error here:\nInclude file not found: \"TextEditor.h\""));
	//markers.insert(std::make_pair<int, std::string>(41, "Another example error"));
	//editor.SetErrorMarkers(markers);

	// "breakpoint" markers
	//TextEditor::Breakpoints bpts;
	//bpts.insert(24);
	//bpts.insert(47);
	//editor.SetBreakpoints(bpts);

	auto str = sys::read_file(filename);
	editor->SetText(str);
	editor->SetFileName(filename);
}

void show_text_editor(TextEditor *editor)
{
	ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(.13f, .13f, .13f, 1.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::PushFont(g_mono_font_16);

	auto cpos = editor->GetCursorPosition();

	ImGui::Begin("Text Editor", nullptr, ImGuiWindowFlags_HorizontalScrollbar);
	if (ImGui::BeginMenuBar()) {
		ImGui::EndMenuBar();
	}
	ImGui::Text("%6d/%-6d %6d lines  %s %s | %s | %s", cpos.mLine + 1, cpos.mColumn + 1, editor->GetTotalLines(),
		editor->IsOverwrite() ? "Ovr" : "Ins",
		editor->CanUndo() ? "*" : " ",
		editor->GetLanguageDefinition().mName.c_str(), 
		editor->GetFileName().c_str());

	editor->Render("TextEditor");
	ImGui::End();
	ImGui::PopFont();
	ImGui::PopStyleVar();
	ImGui::PopStyleColor();
}

#include "geometry.hpp"

class DrawList;

class DrawCommand
{
public:
	DrawCommand() 
		: clip_rect_()
		, texture_id_(0)
		, element_count_(0)
		, user_callback_(nullptr)
	{
	}
	DrawCommand(unsigned tid, int elemcnt, const rect& cliprect=rect())
		: clip_rect_(cliprect)
		, texture_id_(tid)
		, element_count_(elemcnt)
		, user_callback_(nullptr)
	{
	}
	~DrawCommand() {}
	void addElements(int ec) { element_count_ += ec; }
	void setTextureId(unsigned tid) { texture_id_ = tid; }
	void setClipRect(const rect& cr) { clip_rect_ = cr; }
	int getElementCount() const { return element_count_; }
	unsigned getTextureId() const { return texture_id_; }
	const rect& getClipRect() const { return clip_rect_; }
private:
	rect clip_rect_;
	unsigned texture_id_;
	int element_count_;
	std::function<void(DrawList*, DrawCommand*)> user_callback_;
};

struct DrawVertex
{
	DrawVertex(const glm::vec2& p, const glm::vec2& uv, const glm::vec2& n, const uint32_t c)
		: position_(p)
		, uv_(uv)
		, normal_(n)
		, color_(c)
	{
	}
	glm::vec2 position_;
	glm::vec2 uv_;
	glm::vec2 normal_;
	uint32_t color_;
};

typedef unsigned short DrawIndex;

struct LocalCommand {
	DrawCommand command;
	std::vector<DrawVertex> vertices;
	std::vector<DrawIndex> indices;
};

class DrawList
{
public:
	DrawList() 
		: vao_(0)
		, vbo_(0)
		, ibo_(0)
		, draw_cmds_()
	{
		glGenVertexArrays(1, &vao_);
		glGenBuffers(1, &vbo_);
		glGenBuffers(1, &ibo_);

		glBindVertexArray(vao_);
		glBindBuffer(GL_ARRAY_BUFFER, vbo_);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_);

		const intptr_t stride = sizeof(DrawVertex);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<const GLvoid*>(offsetof(DrawVertex, position_)));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<const GLvoid*>(offsetof(DrawVertex, uv_)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<const GLvoid*>(offsetof(DrawVertex, normal_)));
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 4, GL_UNSIGNED_BYTE, GL_TRUE, stride, reinterpret_cast<const GLvoid*>(offsetof(DrawVertex, color_)));

		// need to unbind the vertex array object before the buffers, because the VAO 
		// *will* remember the last item bound.
		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
	~DrawList() {
		glDeleteVertexArrays(1, &vao_);
		glDeleteBuffers(1, &vbo_);
		glDeleteBuffers(1, &ibo_);
	}
	unsigned getVertexArrayObj() const { return vao_; }
	unsigned getVertexBufferObj() const { return vbo_; }
	unsigned getIndexBufferObj() const { return ibo_; }
	//const std::vector<DrawCommand>& getCommands() const { return commands_; }
	//const std::vector<DrawVertex>& getVertices() const { return vertices_; }
	//const std::vector<DrawIndex>& getIndices() const { return indices_; }
	// number 1 of the addSprite overloads -- most basic case
	void addSprite(const graphics::Texture* tex, const point& loc, int width, int height, const rect& tr, uint32_t color = 0xffffffff);

	void clear() { draw_cmds_.clear(); }

	typedef std::unordered_map<unsigned, LocalCommand>::iterator iterator;
	typedef std::unordered_map<unsigned, LocalCommand>::const_iterator const_iterator;

	iterator begin() { return draw_cmds_.begin(); }
	iterator end() { return draw_cmds_.end(); }
	const_iterator begin() const { return draw_cmds_.begin(); }
	const_iterator end() const { return draw_cmds_.end(); }

	const_iterator cbegin() const { return draw_cmds_.cbegin(); }
	const_iterator cend() const { return draw_cmds_.cend(); }
private:
	unsigned vao_;
	unsigned vbo_;
	unsigned ibo_;
	std::unordered_map<unsigned, LocalCommand> draw_cmds_;
};

namespace
{
	static const std::vector<unsigned short> indicies_rect{
		0, 1, 2,
		2, 3, 0
	};
}
void DrawList::addSprite(const graphics::Texture* tex, const point& loc, int width, int height, const rect& tr, uint32_t color)
{
	const float trf[4]{ static_cast<float>(tr.x1()) / static_cast<float>(tex->width()), 
		static_cast<float>(tr.y1()) / static_cast<float>(tex->width()), 
		static_cast<float>(tr.x2()) / static_cast<float>(tex->height()), 
		static_cast<float>(tr.y2()) / static_cast<float>(tex->height()) };

	auto& cmd = draw_cmds_[tex->id()];
	cmd.vertices.emplace_back(glm::vec2(loc.x, loc.y), glm::vec2(trf[0], trf[1]), glm::vec2(), color);
	cmd.vertices.emplace_back(glm::vec2(loc.x+width, loc.y), glm::vec2(trf[2], trf[1]), glm::vec2(), color);
	cmd.vertices.emplace_back(glm::vec2(loc.x+width, loc.y+height), glm::vec2(trf[2], trf[3]), glm::vec2(), color);
	cmd.vertices.emplace_back(glm::vec2(loc.x, loc.y+height), glm::vec2(trf[0], trf[3]), glm::vec2(), color);

	std::copy(indicies_rect.cbegin(), indicies_rect.cend(), std::back_inserter(cmd.indices));
	cmd.command.addElements(6);
	if(cmd.command.getTextureId() == 0) {
		cmd.command.setTextureId(tex->id());
	}
}

void game::Object::draw(DrawList* drawlist)
{
	ASSERT_LOG(!tex_rect_.empty(), "No rects defined for texture.");
	const rect& tr = tex_rect_[frame_];
	drawlist->addSprite(tex_.get(), loc_, width_, height_, tr);
}

#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

GLuint g_proj_matrix_loc = -1;
int g_width = 0, g_height = 0;

void render(const game::Object* obj, const DrawList* drawlist)
{
	obj->getShader()->apply();

    glViewport(0, 0, (GLsizei)g_width, (GLsizei)g_height);
	auto ortho_projection = glm::ortho(0.f, static_cast<float>(g_width), static_cast<float>(g_height), 0.f);
    /*const float ortho_projection[4][4] =
    {
        { 2.0f/g_width, 0.0f,                   0.0f, 0.0f },
        { 0.0f,                  2.0f/-g_height, 0.0f, 0.0f },
        { 0.0f,                  0.0f,                  -1.0f, 0.0f },
        {-1.0f,                  1.0f,                   0.0f, 1.0f },
    };*/
    glUniformMatrix4fv(g_proj_matrix_loc, 1, GL_FALSE, glm::value_ptr(ortho_projection));

	//static GLuint color_id = obj->getShader()->getUniformId("u_color");
	//glUniform4f(color_id, 1.0f, 1.0f, 1.0f, 1.0f);
	static GLuint tex_id = obj->getShader()->getUniformId("u_tex");
	glUniform1i(tex_id, 0);

	glActiveTexture(GL_TEXTURE0);

	for(const auto& item : *drawlist) {
		const auto& cmd = item.second;
		glBindVertexArray(drawlist->getVertexArrayObj());
		glBindBuffer(GL_ARRAY_BUFFER, drawlist->getVertexBufferObj());
		glBufferData(GL_ARRAY_BUFFER, cmd.vertices.size() * sizeof(DrawVertex), cmd.vertices.data(), GL_STREAM_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, drawlist->getIndexBufferObj());
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, cmd.indices.size() * sizeof(DrawIndex), cmd.indices.data(), GL_STREAM_DRAW);

		//if(cmd->user_callback_) {
		//	cmd->user_callback_(&this, cmd);
		//} else {
		glBindTexture(GL_TEXTURE_2D, cmd.command.getTextureId());
		const auto& cr = cmd.command.getClipRect();
		if(!cr.empty()) {
			glScissor(cr.x(), cr.y(), cr.w(), cr.h());
		}
		glDrawElements(GL_TRIANGLES, cmd.command.getElementCount(), sizeof(DrawIndex) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, 0);
	}

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}


int main(int argc, char* argv[])
{
	auto console = spdlog::stdout_color_mt("console");

	// create a vector for command line arguments.
	std::vector<std::string> args;
	for(int n = 0; n != argc; ++n) {
		args.emplace_back(argv[n]);
	}

	//json_test();

	SDL2Ptr sdl = SDL2::Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO);

	auto wnd = SDL2::CreateWin("lua_test2", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1600, 900);
	LOG_INFO("Requested Window Size: {}x{}", wnd->getRequestedWidth(), wnd->getRequestedHeight());
	LOG_INFO("Actual Window Size: {}x{}", wnd->getWidth(), wnd->getHeight());
	g_width = wnd->getWidth();
	g_height = wnd->getHeight();

    double t = 0.0;
    double dt = 0.05;

    double currentTime = (double)SDL_GetPerformanceCounter() / SDL_GetPerformanceFrequency();
    double accumulator = 0.0;

	auto bshader = graphics::Shader::getShader("basic");
	bshader->apply();

	g_proj_matrix_loc = bshader->getUniformId("u_projmatrix");

	//test1();

	std::unique_ptr<graphics::Texture> tex = std::make_unique<graphics::Texture>("..\\images\\test1.png");
	tex->bind();

	wnd->setClearColor(0, 0, 0, 255);

	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_SCISSOR_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	theme_imgui_default(true, 1.0f);

	ImGui::FrameTimeHistogram frame_time;

	TextEditor editor;
	init_text_editor(&editor, "..\\data\\test1.lua");

	game::ObjectPtr player = std::make_unique<game::Object>();
	player->setTexture("..\\images\\image1.png");
	player->attachShader(bshader);
	// should this be as follows :-
	// player->attachShader(bshader->clone());
	DrawList drawlist;

	int px = g_width / 2 - player->width() / 2;
	int py = g_height / 2 - player->height() / 2;

	SDL_Event ev;
	bool running = true;
	while(running) {
		while(SDL_PollEvent(&ev)) {
			ImGui_ImplSdlGL3_ProcessEvent(&ev);

			const auto mod = SDL_GetModState();
			//fmt::print("0x{:x}\n", ev.type);
			if(ev.type == SDL_KEYUP) {
				const auto key = ev.key.keysym.sym;
				if(ev.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
					fmt::print("ESC pressed.\n");
					running = false;
				} else if(key == SDLK_F9) {
					g_theme_imgui_ui = !g_theme_imgui_ui;
					if(mod & KMOD_CTRL) {
						// resets to default ui and shows it
						theme_imgui_default(true, 1.0);
						g_theme_imgui_ui = true;
					}
				} else if(key == SDLK_F3) {
					g_show_fps = !g_show_fps;
				} else if(key == SDLK_F2) {
					g_show_text_editor = !g_show_text_editor;
				} else if (key == SDLK_BACKQUOTE) {
					g_show_main_menu_bar = !g_show_main_menu_bar;
				}
			} else if (ev.type == SDL_KEYDOWN) {
				const auto key = ev.key.keysym.sym;

				if (key == SDLK_UP) {
					py -= 10;
				} else if (key == SDLK_DOWN) {
					py += 10;
				} else if (key == SDLK_LEFT) {
					px -= 10;
				} else if (key == SDLK_RIGHT) {
					px += 10;
				}

			} else if(ev.type == SDL_WINDOWEVENT) {
				running = wnd->handleWindowEvent(&ev);
			}
		}

        double newTime = (double)SDL_GetPerformanceCounter() / SDL_GetPerformanceFrequency();
        double frameTime = newTime - currentTime;
        if(frameTime > 0.25) {
            frameTime = 0.25;
		}
        currentTime = newTime;

        accumulator += frameTime;

        while(accumulator >= dt) {
            //previousState = currentState;
            //integrate(currentState, t, dt);
            t += dt;
            accumulator -= dt;
        }

        const double alpha = accumulator / dt;

		frame_time.Update(static_cast<float>(frameTime));

        //State state = currentState * alpha + previousState * ( 1.0 - alpha );

		wnd->newFrame();

		player->setLocation(px, py);
		player->draw(&drawlist);
		render(player.get(), &drawlist);
		drawlist.clear();
		
		if(g_show_main_menu_bar && ImGui::BeginMainMenuBar()) {
			if(ImGui::BeginMenu("File")) {
				ImGui::EndMenu();
			}
			if(ImGui::BeginMenu("Examples")) {
				ImGui::MenuItem("Show FPS", "F3", &g_show_fps);
				ImGui::MenuItem("Show Imgui Theme controls", "F9", &g_theme_imgui_ui);
				ImGui::MenuItem("Show Text Editor", "F2", &g_show_text_editor);
				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}

		if(g_theme_imgui_ui) {
			imgui_theme_ui();
		}

		if(g_show_fps) {
			static bool frame_time_is_open = false;
			frame_time.Draw("Frame Time Histogram", &frame_time_is_open);

			ImGui::Begin("testing");
			static bool checked = false;
			ImGui::CheckBoxTick("Some Test", &checked);
			ImGui::End();
		}

		if(g_show_text_editor) {
			show_text_editor(&editor);
		}

		wnd->swap();

		//fmt::print("frame time: {}\n", frameTime * 1000.0);
	}
	return 0;
}
