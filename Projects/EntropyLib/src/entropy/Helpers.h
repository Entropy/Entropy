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
	inline shared_ptr<entropy::scene::Manager> GetSceneManager()
	{
		return GetApp()->getSceneManager();
	}

	//--------------------------------------------------------------
	inline shared_ptr<entropy::render::Canvas> GetCanvas(entropy::render::Layout layout)
	{
		if (layout == entropy::render::Layout::Back)
		{
			return GetApp()->getCanvasBack();
		}
		return GetApp()->getCanvasFront();
	}

	//--------------------------------------------------------------
	inline float GetCanvasWidth(entropy::render::Layout layout)
	{
		return GetCanvas(layout)->getWidth();
	}

	//--------------------------------------------------------------
	inline float GetCanvasHeight(entropy::render::Layout layout)
	{
		return GetCanvas(layout)->getHeight();
	}

	//--------------------------------------------------------------
	inline const ofRectangle & GetCanvasViewport(entropy::render::Layout layout)
	{
		return GetCanvas(layout)->getViewport();
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
