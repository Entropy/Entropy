#pragma once

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

    //--------------------------------------------------------------
    inline string GetSharedAssetsPath()
    {
        static string assetsPath;
        if (assetsPath.empty())
        {
            assetsPath = ofFilePath::addTrailingSlash(ofToDataPath("../../../../Shared/assets"));
        }
        return assetsPath;
    }
}
