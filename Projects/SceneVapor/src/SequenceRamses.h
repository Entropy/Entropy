#pragma once

#include "ofMain.h"
#include "ofxHDF5.h"
#include "ofxRange.h"

#include "SnapshotRamses.h"
#include "ofxVolumetrics3D.h"
#include "ofxTexture3d.h"

namespace ent
{
    class SequenceRamses
    {
    public:
        SequenceRamses();
        ~SequenceRamses();

        void setup(const std::string& folder, int startIndex, int endIndex);
		void setupRemote(const std::string& urlFolder, const std::string& folder, int startIndex, int endIndex);
		void clear();

        void update();
        void draw(float scale);
		void drawOctree(float scale);
		void drawOctreeDensities(const ofTrueTypeFont & ttf, const ofCamera & camera, float scale);
		void drawTexture(float scale);
		ofMesh getOctreeMesh(float scale) const;

		void preloadAllFrames();
		void loadFrame(int index);

		void setFrameRate(float frameRate);
		float getFrameRate() const;

		void setFrame(int index);
		void setFrameForTime(float time);
		void setFrameAtPercent(float percent);
		
		int getCurrentFrame() const;
		int getTotalFrames() const;
		float getTotalTime() const;

		int getFrameIndexAtPercent(float percent);
		float getPercentAtFrameIndex(int index);

		SnapshotRamses& getSnapshot();
		SnapshotRamses& getSnapshotForFrame(int index);
		SnapshotRamses& getSnapshotForTime(float time);
		SnapshotRamses& getSnapshotForPercent(float percent);

		const SnapshotRamses& getSnapshot() const;

		bool isReady() const;

		float getNormalizeFactor() const{
			return m_normalizeFactor;
		}

    protected:
		void setup(int startIndex, int endIndex);

		// Data
		SnapshotRamses m_snapshot;

		std::string m_folder;
		std::string m_urlFolder;
		int m_startIndex;
		int m_endIndex;
		int m_currentIndex;
		
		ofxRange3f m_coordRange;
		ofxRange1f m_sizeRange;
		ofxRange1f m_densityRange;

		ofParameter<float> m_densityMin{"density min", 0.f, 0.f, 1.f, ofParameterScale::Logarithmic};
		ofParameter<float> m_densityMax{"density max", 0.25f, 0.f, 1.f, ofParameterScale::Logarithmic};
		ofParameter<float> m_volumeQuality{"volume quality", 1.f, 0, 10, ofParameterScale::Logarithmic};
		ofParameter<float> m_volumeDensity{"volume density", 20.f, 0, 50, ofParameterScale::Logarithmic};

		// Playback
		float m_frameRate;
		std::size_t m_currFrame;

        // 3D Render
		ofParameter<bool> m_bRender{"render", true};

        glm::vec3 m_originShift;
        float m_normalizeFactor;

        ofShader m_renderShader;
		ofShader m_volumetricsShader;

		bool m_bReady;

		std::filesystem::file_time_type m_lastVertTime;
		std::filesystem::file_time_type m_lastFragTime;
		std::filesystem::file_time_type m_lastIncludesTime;

		ofxVolumetrics3D volumetrics;

		std::vector<float> m_clearData;

		SnapshotRamses::Settings frameSettings;

		std::vector<ofEventListener> listeners;

	public:
		ofParameterGroup parameters{
			"Sequence Ramses",
			m_bRender,
			m_densityMin,
			m_densityMax,
			m_volumeQuality,
			m_volumeDensity,
		};
    };
}
