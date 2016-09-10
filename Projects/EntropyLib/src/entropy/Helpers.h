#pragma once

#include "entropy/render/Layout.h"
#include "entropy/util/App.h"
#include "entropy/util/GLError.h"

namespace entropy
{
	//--------------------------------------------------------------
	inline util::App_ * GetApp()
	{
		return util::App::X();
	}

	//--------------------------------------------------------------
	inline string GetSharedDataPath(bool absolute = true)
	{
		static string dataPath;
		if (dataPath.empty())
		{
			dataPath = ofFilePath::addTrailingSlash("../../../Resources/data");
		}

		if (absolute)
		{
			static string dataPathAbs;
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
	inline string GetSharedAssetsPath(bool absolute = true)
	{
		static string assetsPath;
		if (assetsPath.empty())
		{
			assetsPath = ofFilePath::addTrailingSlash("../../../Resources/assets");
		}

		if (absolute)
		{
			static string assetsPathAbs;
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
	inline string GetSharedExportsPath(bool absolute = true)
	{
		static string exportsPath;
		if (exportsPath.empty())
		{
			exportsPath = ofFilePath::addTrailingSlash("../../../Resources/exports");
		}

		if (absolute)
		{
			static string exportsPathAbs;
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
	inline shared_ptr<render::Canvas> GetCanvas(render::Layout layout)
	{
		return GetApp()->getCanvas(layout);
	}

	//--------------------------------------------------------------
	inline float GetCanvasWidth(render::Layout layout)
	{
		return GetCanvas(layout)->getWidth();
	}

	//--------------------------------------------------------------
	inline float GetCanvasHeight(render::Layout layout)
	{
		return GetCanvas(layout)->getHeight();
	}

	//--------------------------------------------------------------
	inline const ofRectangle & GetCanvasViewport(render::Layout layout)
	{
		return GetCanvas(layout)->getViewport();
	}

	//--------------------------------------------------------------
	inline shared_ptr<scene::Playlist> GetPlaylist()
	{
		return GetApp()->getPlaylist();
	}

	//--------------------------------------------------------------
	inline const world::Camera::Settings & GetSavedCameraSettings(render::Layout layout)
	{
		return GetPlaylist()->getCameraSettings(layout);
	}

	//--------------------------------------------------------------
	inline shared_ptr<scene::Base> GetCurrentScene()
	{
		return GetPlaylist()->getCurrentScene();
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
}
