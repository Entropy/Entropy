#pragma once

#include "ofMain.h"
#include "ofxHDF5.h"
#include "ofxImGui.h"
#include "ofxRange.h"

#include "SnapshotRamses.h"

namespace ent
{
    class SequenceRamses
    {
    public:
        SequenceRamses();
        ~SequenceRamses();

        void setup(const std::string& folder, int startIndex, int endIndex);
		void clear();

        void update();
        void draw(float scale);

		bool imGui(ofVec2f& windowPos, ofVec2f& windowSize);

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

		bool isReady() const;

    protected:
		// Data
		std::vector<SnapshotRamses> m_snapshots;

		std::string m_folder;
		int m_startIndex;
		int m_endIndex;
		
		ofxRange3f m_coordRange;
		ofxRange1f m_sizeRange;
		ofxRange1f m_densityRange;

		// Playback
		float m_frameRate;
		std::size_t m_currFrame;

        // 3D Render
        bool m_bRender;

        glm::vec3 m_originShift;
        float m_normalizeFactor;

        ofShader m_renderShader;

        float m_densityMin;
        float m_densityMax;

		bool m_bReady;
    };
}
