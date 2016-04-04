#include "SequenceRamses.h"

namespace ent
{
    //--------------------------------------------------------------
    SequenceRamses::SequenceRamses()
		: m_bRender(true)
		, m_densityMin(0.0f)
		, m_densityMax(0.25f)
		, m_frameRate(30.0f)
    {
		clear();
    }

    //--------------------------------------------------------------
    SequenceRamses::~SequenceRamses()
    {
		clear();
    }

    //--------------------------------------------------------------
    void SequenceRamses::setup(const std::string& folder, int startIndex, int endIndex)
    {
		clear();

		int numFiles = endIndex - startIndex + 1;
		if (numFiles <= 0) {
			ofLogError("SequenceRamses::loadSequence") << "Invalid range [" << startIndex << ", " << endIndex << "]";
			return;
		}

		m_snapshots.resize(numFiles);

		m_folder = folder;
		m_startIndex = startIndex;
		m_endIndex = endIndex;

        // Load the shaders.
        m_renderShader.setupShaderFromFile(GL_VERTEX_SHADER, "shaders/render.vert");
		m_renderShader.setupShaderFromFile(GL_FRAGMENT_SHADER, "shaders/render.frag");
		m_renderShader.bindAttribute(DENSITY_ATTRIBUTE, "density");
		m_renderShader.bindDefaults();
		m_renderShader.linkProgram();

		m_bReady = true;
    }

	//--------------------------------------------------------------
	void SequenceRamses::clear()
	{
		m_snapshots.clear();
		m_startIndex = 0;
		m_endIndex = 0;

		m_coordRange.clear();
		m_sizeRange.clear();
		m_densityRange.clear();

		m_currFrame = 0;

		m_bReady = false;
	}

    //--------------------------------------------------------------
    void SequenceRamses::update()
    {

    }

    //--------------------------------------------------------------
    void SequenceRamses::draw(float scale)
    {
        if (m_bRender) 
		{
            ofSetColor(ofColor::white);
            glPointSize(1.0);

            ofPushMatrix();
            ofScale(scale / m_normalizeFactor, scale / m_normalizeFactor, scale / m_normalizeFactor);
            ofTranslate(m_originShift);
            {
                m_renderShader.begin();
				m_renderShader.setUniform1f("uDensityMin", m_densityMin * m_densityRange.getSpan());
				m_renderShader.setUniform1f("uDensityMax", m_densityMax * m_densityRange.getSpan());
				{
					getSnapshot().update(m_renderShader);
					getSnapshot().draw();
				}
				m_renderShader.end();
            }
            ofPopMatrix();
        }
    }

    //--------------------------------------------------------------
    bool SequenceRamses::imGui(ofDefaultVec2& windowPos, ofDefaultVec2& windowSize)
    {
        ImGui::SetNextWindowPos(windowPos, ImGuiSetCond_Appearing);
        ImGui::SetNextWindowSize(ofDefaultVec2(380, 364), ImGuiSetCond_Appearing);
        if (ImGui::Begin("Cell Renderer", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) 
		{
			ImGui::Text("Frame %lu / %lu", m_currFrame, m_snapshots.size());
			ImGui::Text("%lu Cells", m_snapshots[m_currFrame].getNumCells());

			ImGui::Checkbox("Render", &m_bRender);
            ImGui::DragFloatRange2("Density Range", &m_densityMin, &m_densityMax, 0.0001f, 0.0f, 1.0f, "Min: %.4f%%", "Max: %.4f%%");
			if (ImGui::Button("Next Frame"))
			{
				setFrame(getCurrentFrame() + 1);
			}

            windowSize = ImGui::GetWindowSize();
            ImGui::End();
        }
        else 
		{
            windowSize = ofDefaultVec2(0);
        }

        ofRectangle windowBounds(windowPos, windowSize.x, windowSize.y);
        return windowBounds.inside(ofGetMouseX(), ofGetMouseY());
    }

	//--------------------------------------------------------------
	void SequenceRamses::preloadAllFrames()
	{
		for (int i = 0; i < getTotalFrames(); ++i)
		{
			loadFrame(i);
		}
	}

	//--------------------------------------------------------------
	void SequenceRamses::loadFrame(int index)
	{
		if (0 > index || index >= m_snapshots.size()) 
		{
			ofLogError("SequenceRamses::loadFrame") << "Index " << index << " out of range [0, " << m_snapshots.size() << "]";
			return;
		}

		if (!m_snapshots[index].isLoaded()) 
		{
			m_snapshots[index].setup(m_folder, m_startIndex + index);

			// Adjust the ranges.
			m_coordRange.include(m_snapshots[index].getCoordRange());
			m_sizeRange.include(m_snapshots[index].getSizeRange());
			m_densityRange.include(m_snapshots[index].getDensityRange());

			// Set normalization values to remap to [-0.5, 0.5]
			ofDefaultVec3 coordSpan = m_coordRange.getSpan();
			m_originShift = -0.5 * coordSpan - m_coordRange.getMin();

			m_normalizeFactor = MAX(MAX(coordSpan.x, coordSpan.y), coordSpan.z);
		}
	}

	//--------------------------------------------------------------
	void SequenceRamses::setFrame(int index)
	{
		if (!m_bReady) 
		{
			ofLogError("SequenceRamses::setFrame") << "Not ready, call setup() first!";
			return;
		}

		if (index < 0) 
		{
			ofLogError("SequenceRamses::setFrame") << "Index must be a positive number!";
			return;
		}

		index %= getTotalFrames();

		loadFrame(index);
		m_currFrame = index;
	}
	
	//--------------------------------------------------------------
	void SequenceRamses::setFrameForTime(float time)
	{
		return setFrameAtPercent(time / getTotalTime());
	}
	
	//--------------------------------------------------------------
	void SequenceRamses::setFrameAtPercent(float percent)
	{
		setFrame(getFrameIndexAtPercent(percent));
	}

	//--------------------------------------------------------------
	int SequenceRamses::getFrameIndexAtPercent(float percent)
	{
		if (0.0f > percent || percent > 1.0f)
		{
			percent -= floor(percent);
		}

		return MIN((int)(percent * m_snapshots.size()), m_snapshots.size() - 1);
	}

	//--------------------------------------------------------------
	float SequenceRamses::getPercentAtFrameIndex(int index)
	{
		return ofMap(index, 0, m_snapshots.size() - 1, 0.0f, 1.0f, true);
	}

	//--------------------------------------------------------------
	void SequenceRamses::setFrameRate(float frameRate)
	{
		m_frameRate = frameRate;
	}

	//--------------------------------------------------------------
	int SequenceRamses::getCurrentFrame() const
	{
		return m_currFrame;
	}

	//--------------------------------------------------------------
	int SequenceRamses::getTotalFrames() const
	{
		return m_snapshots.size();
	}

	//--------------------------------------------------------------
	float SequenceRamses::getTotalTime() const
	{
		return getTotalFrames() / m_frameRate;
	}

	//--------------------------------------------------------------
	SnapshotRamses& SequenceRamses::getSnapshot()
	{
		return m_snapshots[m_currFrame];
	}

	//--------------------------------------------------------------
	SnapshotRamses& SequenceRamses::getSnapshotForFrame(int index)
	{
		setFrame(index);
		return getSnapshot();
	}

	//--------------------------------------------------------------
	SnapshotRamses& SequenceRamses::getSnapshotForTime(float time)
	{
		setFrameForTime(time);
		return getSnapshot();
	}

	//--------------------------------------------------------------
	SnapshotRamses& SequenceRamses::getSnapshotForPercent(float percent)
	{
		setFrameAtPercent(percent);
		return getSnapshot();
	}

	//--------------------------------------------------------------
	bool SequenceRamses::isReady() const
	{
		return m_bReady;
	}
}
