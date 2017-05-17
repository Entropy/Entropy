#pragma once

#include "ofConstants.h"
#include "ofFileUtils.h"
#include "ofImage.h"

// Stolen from ofxRulr, thanks Elliot!
// Syntactic sugar which enables struct-ofParameterGroup
#define PARAM_DECLARE(NAME, ...) bool paramDeclareConstructor \
{ [this] { this->setName(NAME), this->add(__VA_ARGS__); return true; }() };

#define OF_PARAMETER(TYPE, NAME, ...) ofParameter<TYPE> NAME{__VA_ARGS__}; \
	bool NAME##Grouper{ [this]{this->add(NAME); return true; }() };

#define OF_PARAMETER_GROUP(NAME, STRING_NAME, ...) struct NAME##Class: public ofParameterGroup { \
	NAME##Class():ofParameterGroup(STRING_NAME){} \
	__VA_ARGS__ \
} NAME;

namespace entropy
{
	//--------------------------------------------------------------
	inline void LoadTextureImage(const std::filesystem::path & path, ofTexture & texture)
	{
		ofPixels pixels;
		ofLoadImage(pixels, path);
		if (!pixels.isAllocated())
		{
			ofLogError(__FUNCTION__) << "Could not load file at path " << path;
		}

		bool wasUsingArbTex = ofGetUsingArbTex();
		ofDisableArbTex();
		{
			texture.enableMipmap();
			texture.loadData(pixels);
		}
		if (wasUsingArbTex) ofEnableArbTex();
	}
	
	//--------------------------------------------------------------
	inline std::filesystem::path GetSharedDataPath(bool absolute = true)
	{
		static std::filesystem::path dataPath;
		if (dataPath.empty())
		{
			dataPath = ofFilePath::addTrailingSlash("../../../Resources/data");
		}

		if (absolute)
		{
			static std::filesystem::path dataPathAbs;
			if (dataPathAbs.empty())
			{
				auto path = std::filesystem::path(ofFilePath::getCurrentExeDir()) / dataPath;
				dataPathAbs = ofFilePath::addTrailingSlash(canonical(path).string());
			}
			return dataPathAbs;
		}

		return dataPath;
	}

	//--------------------------------------------------------------
	inline std::filesystem::path GetSharedAssetsPath(bool absolute = true)
	{
		static std::filesystem::path assetsPath;
		if (assetsPath.empty())
		{
			assetsPath = ofFilePath::addTrailingSlash("../../../Resources/assets");
		}

		if (absolute)
		{
			static std::filesystem::path assetsPathAbs;
			if (assetsPathAbs.empty())
			{
				auto path = std::filesystem::path(ofFilePath::getCurrentExeDir()) / assetsPath;
				assetsPathAbs = ofFilePath::addTrailingSlash(canonical(path).string());
			}
			return assetsPathAbs;
		}

		return assetsPath;
	}

	//--------------------------------------------------------------
	inline std::filesystem::path GetSharedExportsPath(bool absolute = true)
	{
		static std::filesystem::path exportsPath;
		if (exportsPath.empty())
		{
			exportsPath = ofFilePath::addTrailingSlash("../../../Resources/exports");
		}

		if (absolute)
		{
			static std::filesystem::path exportsPathAbs;
			if (exportsPathAbs.empty())
			{
				auto path = std::filesystem::path(ofFilePath::getCurrentExeDir()) / exportsPath;
				exportsPathAbs = ofFilePath::addTrailingSlash(canonical(path).string());
			}
			return exportsPathAbs;
		}

		return exportsPath;
	}

	//--------------------------------------------------------------
	inline std::filesystem::path GetSceneDataPath(const string & name, const string & data = "", bool absolute = true)
	{
		return std::filesystem::path(GetSharedDataPath()) / "entropy" / "scene" / name / data;
	}

	//--------------------------------------------------------------
	inline std::filesystem::path GetSceneAssetPath(const string & name, const string & asset = "", bool absolute = true)
	{
		return std::filesystem::path(GetSharedAssetsPath()) / "entropy" / "scene" / name / asset;
	}

	//--------------------------------------------------------------
	enum class Module
	{
		Renderers,
		Canvas,
		PostEffects,
	};

	//--------------------------------------------------------------
	inline std::string ToString(Module module)
	{
		switch (module)
		{
		case Module::Renderers: return "Renderers";
		case Module::Canvas: return "Canvas";
		case Module::PostEffects: return "PostEffects";
		default: return "Unknown";
		}
	}

	//--------------------------------------------------------------
	inline std::filesystem::path GetDataPath(Module module)
	{
		namespace fs = std::filesystem;
		fs::path dataPath(GetSharedDataPath());
		dataPath = dataPath / fs::path("entropy") / fs::path("render") / fs::path(ToString(module));
		return dataPath;
	}

	//--------------------------------------------------------------
	inline std::filesystem::path GetShadersPath(Module module) {
		namespace fs = std::filesystem;
		fs::path dataPath(GetDataPath(module));
		return (dataPath / "shaders");
	}

	//--------------------------------------------------------------
	inline void thickline(const vector<glm::vec3> & data, float lineWidth, ofMesh & outVertices){
		outVertices.clear();
		outVertices.setMode(OF_PRIMITIVE_TRIANGLE_STRIP);
		auto scaledHalfW = lineWidth * 0.5;
		for(size_t i = 1; i<data.size(); i++){
			auto p = data[i].xy();
			auto prev = data[i-1].xy();
			auto dx = p.x - prev.x;
			auto dy = p.y - prev.y;
			glm::vec2 normal(-dy, dx);
			normal = glm::normalize(normal);
			normal *= scaledHalfW;
			const glm::vec2 v1 = {prev + normal};
			const glm::vec2 v2 = {prev - normal};
			const glm::vec2 v3 = {p + normal};
			const glm::vec2 v4 = {p - normal};
			outVertices.addVertex({v1.x,v1.y,0.0f});
			outVertices.addVertex({v2.x,v2.y,0.0f});
			outVertices.addVertex({v3.x,v3.y,0.0f});
			outVertices.addVertex({v4.x,v4.y,0.0f});
		}
	}
}
