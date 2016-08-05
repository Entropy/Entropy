#pragma once

#include "entropy/util/App.h"
#include "entropy/util/GLError.h"

#include <experimental/filesystem>

namespace entropy
{
	//--------------------------------------------------------------
	inline util::App_ * GetApp()
	{
		return util::App::X();
	}

	//--------------------------------------------------------------
	inline shared_ptr<entropy::scene::Manager> GetSceneManager()
	{
		return GetApp()->getSceneManager();
	}

	//--------------------------------------------------------------
	inline shared_ptr<entropy::render::Canvas> GetCanvas()
	{
		return GetApp()->getCanvas();
	}

	//--------------------------------------------------------------
	inline float GetCanvasWidth()
	{
		return GetCanvas()->getWidth();
	}

	//--------------------------------------------------------------
	inline float GetCanvasHeight()
	{
		return GetCanvas()->getHeight();
	}

	//--------------------------------------------------------------
	inline const ofRectangle & GetCanvasViewport()
	{
		return GetCanvas()->getViewport();
	}

	//--------------------------------------------------------------
	inline string GetSharedDataPath(bool absolute = true)
	{
		static string dataPath;
		if (dataPath.empty())
		{
			dataPath = ofFilePath::addTrailingSlash("../../../Shared/data");
		}
		
		if (absolute)
		{
			static string dataPathAbs;
			if (dataPathAbs.empty())
			{
				auto path = std::experimental::filesystem::path(ofFilePath::getCurrentExeDir()) / dataPath;
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
			assetsPath = ofFilePath::addTrailingSlash("../../../Shared/assets");
		}
		
		if (absolute)
		{
			static string assetsPathAbs;
			if (assetsPathAbs.empty())
			{
				auto path = std::experimental::filesystem::path(ofFilePath::getCurrentExeDir()) / assetsPath;
				assetsPathAbs = ofFilePath::addTrailingSlash(canonical(path).string());
			}
			return assetsPathAbs;
		}

		return assetsPath;
	}

	//--------------------------------------------------------------
	inline string GetCurrentSceneDataPath(const string & file = "")
	{
		auto currentScene = GetSceneManager()->getCurrentScene();
		if (currentScene)
		{
			return currentScene->getDataPath(file);
		}

		ofLogWarning(__FUNCTION__) << "No active Scene found!";
		return GetSharedDataPath();
	}

	//--------------------------------------------------------------
	inline string GetCurrentSceneAssetsPath(const string & file = "")
	{
		auto currentScene = GetSceneManager()->getCurrentScene();
		if (currentScene)
		{
			return currentScene->getAssetsPath(file);
		}

		ofLogWarning(__FUNCTION__) << "No active Scene found!";
		return GetSharedAssetsPath();
	}
}
