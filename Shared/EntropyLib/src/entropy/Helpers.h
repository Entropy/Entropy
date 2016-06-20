#pragma once

#include "entropy/util/App.h"

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
	inline string GetSharedDataPath()
	{
		static string dataPath;
		if (dataPath.empty())
		{
			dataPath = ofFilePath::addTrailingSlash(ofToDataPath("../../../../Shared/data"));
		}
		return dataPath;
	}
}
