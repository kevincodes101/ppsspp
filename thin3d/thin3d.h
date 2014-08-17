// Very thin API wrapper, suitable for porting UI code (like the native/ui framework) and similar but not real rendering.
// Does not involve context creation etc, that should be handled separately - only does drawing.

// The goals may change in the future though.
// MIT licensed, by Henrik Rydg�rd 2014.

#pragma once

#include <stdint.h>
#include <vector>

class Matrix4x4;

enum T3DBlendEquation : int {
	ADD,
	SUBTRACT,
	REV_SUBTRACT,
  // MIN, MAX,
};

enum T3DComparison : int {
	NEVER,
	LESS,
	EQUAL,
	LESS_EQUAL,
	GREATER,
	NOT_EQUAL,
	GREATER_EQUAL,
	ALWAYS,
};

enum T3DBlendFactor : int {
	ZERO,
	ONE,
	SRC_COLOR,
	SRC_ALPHA,
	ONE_MINUS_SRC_COLOR,
	ONE_MINUS_SRC_ALPHA,
	DST_COLOR,
	DST_ALPHA,
	ONE_MINUS_DST_COLOR,
	ONE_MINUS_DST_ALPHA,
	FIXED_COLOR,
};

enum T3DBufferUsage : int {
	VERTEXDATA = 1,
	INDEXDATA = 2,
	GENERIC = 4,

	DYNAMIC = 16,
};

enum T3DVertexDataType : uint8_t {
	INVALID,
	FLOATx2,
	FLOATx3,
	FLOATx4,
	UNORM8x4,
};

enum T3DSemantic : int {
	SEM_POSITION,
	SEM_COLOR0,
	SEM_TEXCOORD0,
	SEM_TEXCOORD1,
	SEM_NORMAL,
	SEM_TANGENT,
	SEM_BINORMAL,  // really BITANGENT
	SEM_MAX,
};

enum T3DPrimitive : int {
	PRIM_POINTS,
	PRIM_LINES,
	PRIM_TRIANGLES,
};

enum T3DVertexShaderPreset : int {
	VS_COLOR_2D,
	VS_TEXTURE_COLOR_2D,
	VS_MAX_PRESET,
};

enum T3DFragmentShaderPreset : int {
	FS_COLOR_2D,
	FS_TEXTURE_COLOR_2D,
	FS_MAX_PRESET,
};

// Predefined full shader setups.
enum T3DShaderSetPreset : int {
	SS_COLOR_2D,
	SS_TEXTURE_COLOR_2D,
	SS_MAX_PRESET,
};

enum T3DBlendStatePreset : int {
	BS_OFF,
	BS_STANDARD_ALPHA,
	BS_PREMUL_ALPHA,
	BS_ADDITIVE,
	BS_MAX_PRESET,
};

enum T3DClear : int {
	COLOR,
	DEPTH,
	STENCIL,
};

// Binary compatible with D3D11 viewport.
struct T3DViewport {
	float TopLeftX;
	float TopLeftY;
	float Width;
	float Height;
	float MinDepth;
	float MaxDepth;
};

class Thin3DObject {
public:
	Thin3DObject() : refcount_(1) {}
	virtual ~Thin3DObject() {}

	virtual void AddRef() { refcount_++; }
	virtual void Release() { refcount_--; if (!refcount_) delete this; }  // TODO: Refcount

private:
	int refcount_;
};

class Thin3DBlendState : public Thin3DObject {
public:
};

class Thin3DDepthStencilState : public Thin3DObject {
public:
};

class Thin3DBuffer : public Thin3DObject {
public:
	virtual void SetData(const uint8_t *data, size_t size) = 0;
	virtual void SubData(const uint8_t *data, size_t offset, size_t size) = 0;
};

class Thin3DTexture : public Thin3DObject {
public:
	virtual void SetImageData(int x, int y, int z, int width, int height, int depth, int level, int stride, uint8_t *data) = 0;
	virtual void AutoGenMipmaps() = 0;
};

struct Thin3DVertexComponent {
	Thin3DVertexComponent() : name(nullptr), type(T3DVertexDataType::INVALID), semantic(255), offset(255) {}
	Thin3DVertexComponent(const char *name, T3DSemantic semantic, T3DVertexDataType dataType, uint8_t offset) {
		this->name = name;
		this->semantic = semantic;
		this->type = dataType;
		this->offset = offset;
	}
	const char *name;
	T3DVertexDataType type;
	uint8_t semantic;
	uint8_t offset;
};

class Thin3DVertexFormat : public Thin3DObject {
public:
};

class Thin3DShader : public Thin3DObject {
public:
};

class Thin3DShaderSet : public Thin3DObject {
public:
	// TODO: Make some faster way of doing these. Support uniform buffers (and fake them on GL 2.0?)
	virtual void SetVector(const char *name, float *value, int n) = 0;
	virtual void SetMatrix4x4(const char *name, const Matrix4x4 &value) = 0;
};

struct T3DBlendStateDesc {
	bool enabled;
	T3DBlendEquation eqCol;
	T3DBlendFactor srcCol;
	T3DBlendFactor dstCol;
	T3DBlendEquation eqAlpha;
	T3DBlendFactor srcAlpha;
	T3DBlendFactor dstAlpha;
	// int colorMask;
};

enum T3DTextureType : uint8_t {
	LINEAR1D,
	LINEAR2D,
	LINEAR3D,
	CUBE,
	ARRAY1D,
	ARRAY2D,
};

enum T3DImageFormat : uint8_t {
	RGBA8888,
	DXT1,
	ETC1,  // Needs simulation on many platforms
	D16,
	D24S8,
	D24X8,
};

class Thin3DContext : public Thin3DObject {
public:
	virtual ~Thin3DContext();

	virtual Thin3DDepthStencilState *CreateDepthStencilState(bool depthTestEnabled, bool depthWriteEnabled, T3DComparison depthCompare) = 0;
	virtual Thin3DBlendState *CreateBlendState(const T3DBlendStateDesc &desc) = 0;
	virtual Thin3DBuffer *CreateBuffer(size_t size, uint32_t usageFlags) = 0;
	virtual Thin3DShaderSet *CreateShaderSet(Thin3DShader *vshader, Thin3DShader *fshader) = 0;
	virtual Thin3DVertexFormat *CreateVertexFormat(const std::vector<Thin3DVertexComponent> &components, int stride) = 0;
	virtual Thin3DTexture *CreateTexture(T3DTextureType type, T3DImageFormat format, int width, int height, int depth, int mipLevels) = 0;

	// Common Thin3D function, uses CreateTexture
	Thin3DTexture *CreateTextureFromFile(const char *filename);

	// Note that these DO NOT AddRef so you must not ->Release presets unless you manually AddRef them.
	Thin3DBlendState *GetBlendStatePreset(T3DBlendStatePreset preset) { return bsPresets_[preset]; }
	Thin3DShader *GetVshaderPreset(T3DVertexShaderPreset preset) { return fsPresets_[preset]; }
	Thin3DShader *GetFshaderPreset(T3DFragmentShaderPreset preset) { return vsPresets_[preset]; }
	Thin3DShaderSet *GetShaderSetPreset(T3DShaderSetPreset preset) { return ssPresets_[preset]; }

	// The implementation makes the choice of which shader code to use.
	virtual Thin3DShader *CreateVertexShader(const char *glsl_source, const char *hlsl_source) = 0;
	virtual Thin3DShader *CreateFragmentShader(const char *glsl_source, const char *hlsl_source) = 0;

	// Bound state objects. Too cumbersome to add them all as parameters to Draw.
	virtual void SetBlendState(Thin3DBlendState *state) = 0;
	virtual void SetTextures(int start, int count, Thin3DTexture **textures) = 0;

	void SetTexture(int stage, Thin3DTexture *texture) {
		SetTextures(stage, 1, &texture);
	}  // from sampler 0 and upwards

	// Raster state
	virtual void SetScissorEnabled(bool enable) = 0;
	virtual void SetScissorRect(int left, int top, int width, int height) = 0;
	virtual void SetViewports(int count, T3DViewport *viewports) = 0;

	// TODO: Add more sophisticated draws with buffer offsets, and multidraws.
	virtual void Draw(T3DPrimitive prim, Thin3DShaderSet *pipeline, Thin3DVertexFormat *format, Thin3DBuffer *vdata, int vertexCount, int offset) = 0;
	virtual void DrawIndexed(T3DPrimitive prim, Thin3DShaderSet *pipeline, Thin3DVertexFormat *format, Thin3DBuffer *vdata, Thin3DBuffer *idata, int vertexCount, int offset) = 0;
	virtual void Clear(int mask, uint32_t colorval, float depthVal, int stencilVal) = 0;

protected:
	void CreatePresets();

	Thin3DShader *vsPresets_[VS_MAX_PRESET];
	Thin3DShader *fsPresets_[FS_MAX_PRESET];
	Thin3DBlendState *bsPresets_[BS_MAX_PRESET];
	Thin3DShaderSet *ssPresets_[SS_MAX_PRESET];

private:
};

Thin3DContext *T3DCreateGLContext();

#ifdef _WIN32
struct IDirect3DDevice9;
Thin3DContext *T3DCreateDX9Context(IDirect3DDevice9 *device);
#endif
