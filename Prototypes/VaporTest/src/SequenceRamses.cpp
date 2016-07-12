#include "SequenceRamses.h"
#include "Constants.h"

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
		m_renderShader.bindAttribute(POSITION_ATTRIBUTE, "position");
		m_renderShader.bindAttribute(SIZE_ATTRIBUTE, "size");
		m_renderShader.bindAttribute(DENSITY_ATTRIBUTE, "density");
		m_renderShader.linkProgram();
		m_renderShader.begin();
		m_renderShader.setUniform1f("uDensityMin", m_densityMin * m_densityRange.getSpan());
		m_renderShader.setUniform1f("uDensityMax", m_densityMax * m_densityRange.getSpan());
		m_renderShader.end();

#if USE_VOXELS_COMPUTE_SHADER
		frameSettings.voxels2texture.setupShaderFromFile(GL_COMPUTE_SHADER, "shaders/voxels2texture3d.glsl");
		frameSettings.voxels2texture.linkProgram();
		auto maxNumVoxels = 200000000;
		auto voxelSizeBytes = 8;
		frameSettings.voxelsBuffer.allocate(maxNumVoxels*voxelSizeBytes, GL_STATIC_DRAW);
		frameSettings.voxelsTexture.allocateAsBufferTexture(frameSettings.voxelsBuffer, GL_RG32I);
#endif

#if USE_PARTICLES_COMPUTE_SHADER
		frameSettings.particles2texture.setupShaderFromFile(GL_COMPUTE_SHADER, "shaders/particles2texture3d.glsl");
		frameSettings.particles2texture.linkProgram();
		auto maxNumParticles = 3000000;
		frameSettings.particlesBuffer.allocate(maxNumParticles*sizeof(Particle), GL_STATIC_DRAW);
		frameSettings.particlesTexture.allocateAsBufferTexture(frameSettings.particlesBuffer, GL_RGBA32F);
#endif

		namespace fs = std::filesystem;
		m_lastVertTime = fs::last_write_time(ofFile("shaders/render.vert", ofFile::Reference));
		m_lastFragTime = fs::last_write_time(ofFile("shaders/render.frag", ofFile::Reference));
		m_lastIncludesTime = fs::last_write_time(ofFile("shaders/defines.glsl", ofFile::Reference));

		m_volumeDensity = 20.0;
		m_volumeQuality = 1;
		m_volumetricsShader.load("shaders/volumetrics_vertex.glsl", "shaders/volumetrics_frag.glsl");
		m_volumetricsShader.begin();
		m_volumetricsShader.setUniform1f("minDensity", m_densityMin * m_densityRange.getSpan());
		m_volumetricsShader.setUniform1f("maxDensity", m_densityMax * m_densityRange.getSpan());
		m_volumetricsShader.end();

		frameSettings.worldsize = 512;
		m_clearData.assign(frameSettings.worldsize*frameSettings.worldsize*frameSettings.worldsize,0);
		frameSettings.volumeTexture.allocate(frameSettings.worldsize, frameSettings.worldsize, frameSettings.worldsize, GL_R16F);
		frameSettings.volumeTexture.loadData(
		    m_clearData.data(),
		    frameSettings.worldsize, frameSettings.worldsize, frameSettings.worldsize, 0,0,0, GL_RED
		);

		volumetrics.setup(&frameSettings.volumeTexture, ofVec3f(1,1,1), m_volumetricsShader);
		volumetrics.setRenderSettings(1, m_volumeQuality, m_volumeDensity, 0);
		glBindTexture(frameSettings.volumeTexture.texData.textureTarget,frameSettings.volumeTexture.texData.textureID);
		glTexParameteri(frameSettings.volumeTexture.texData.textureTarget, GL_TEXTURE_SWIZZLE_R, GL_RED);
		glTexParameteri(frameSettings.volumeTexture.texData.textureTarget, GL_TEXTURE_SWIZZLE_G, GL_RED);
		glTexParameteri(frameSettings.volumeTexture.texData.textureTarget, GL_TEXTURE_SWIZZLE_B, GL_RED);
		glTexParameteri(frameSettings.volumeTexture.texData.textureTarget, GL_TEXTURE_SWIZZLE_A, GL_GREEN);
		glBindTexture(frameSettings.volumeTexture.texData.textureTarget,0);


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
		namespace fs = std::filesystem;
		auto vertTime = fs::last_write_time(ofFile("shaders/render.vert", ofFile::Reference));
		auto fragTime = fs::last_write_time(ofFile("shaders/render.frag", ofFile::Reference));
		auto includesTime = fs::last_write_time(ofFile("shaders/defines.glsl", ofFile::Reference));

		if(vertTime > m_lastVertTime || fragTime > m_lastFragTime || includesTime > m_lastIncludesTime){
			ofShader newShader;
			// Load the shaders.
			auto loaded = newShader.setupShaderFromFile(GL_VERTEX_SHADER, "shaders/render.vert") &&
			    newShader.setupShaderFromFile(GL_FRAGMENT_SHADER, "shaders/render.frag");
			if(loaded){
				m_renderShader.bindAttribute(POSITION_ATTRIBUTE, "position");
				m_renderShader.bindAttribute(SIZE_ATTRIBUTE, "size");
				m_renderShader.bindAttribute(DENSITY_ATTRIBUTE, "density");
				loaded &= newShader.bindDefaults() && newShader.linkProgram();
			}
			if(loaded){
				m_renderShader = newShader;
				m_renderShader.begin();
				m_renderShader.setUniform1f("uDensityMin", m_densityMin * m_densityRange.getSpan());
				m_renderShader.setUniform1f("uDensityMax", m_densityMax * m_densityRange.getSpan());
				m_renderShader.end();
				m_volumetricsShader.begin();
				m_volumetricsShader.setUniform1f("minDensity", m_densityMin * m_densityRange.getSpan());
				m_volumetricsShader.setUniform1f("maxDensity", m_densityMax * m_densityRange.getSpan());
				m_volumetricsShader.end();
			}

			m_lastVertTime = vertTime;
			m_lastFragTime = fragTime;
			m_lastIncludesTime = includesTime;
		}
    }

    //--------------------------------------------------------------
    void SequenceRamses::draw(float scale)
    {
        if (m_bRender) 
		{
            ofSetColor(ofColor::white);
			//glPointSize(10.0);

            ofPushMatrix();
            ofScale(scale / m_normalizeFactor, scale / m_normalizeFactor, scale / m_normalizeFactor);
            ofTranslate(m_originShift.x, m_originShift.y, m_originShift.z);
            {
				m_renderShader.begin();
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
	void SequenceRamses::drawOctree(float scale)
	{
		if (m_bRender)
		{
			ofSetColor(ofColor::white);

			ofPushMatrix();
			ofScale(scale / m_normalizeFactor, scale / m_normalizeFactor, scale / m_normalizeFactor);
			ofTranslate(m_originShift.x, m_originShift.y, m_originShift.z);
			getSnapshot().drawOctree(m_densityMin * m_densityRange.getSpan(), m_densityMax * m_densityRange.getSpan());
			ofPopMatrix();
		}
	}

	//--------------------------------------------------------------
	void SequenceRamses::drawTexture(float scale){
		auto original_scale = std::max(getSnapshot().getCoordRange().getSpan().x, std::max(getSnapshot().getCoordRange().getSpan().y, getSnapshot().getCoordRange().getSpan().z));
		auto box_scale = std::max(getSnapshot().m_boxRange.getSpan().x, std::max(getSnapshot().m_boxRange.getSpan().y, getSnapshot().m_boxRange.getSpan().z));
		auto ratio = box_scale/original_scale;
		volumetrics.setRenderSettings(1, m_volumeQuality, m_volumeDensity/(ratio*ratio), 0);
		volumetrics.drawVolume(0, 0, 0, scale * ratio, 0);

		ofSetColor(ofColor::white);
		ofNoFill();
		ofPushMatrix();
		ofScale(scale / m_normalizeFactor, scale / m_normalizeFactor, scale / m_normalizeFactor);
		ofTranslate(m_originShift.x, m_originShift.y, m_originShift.z);
		ofDrawBox(getSnapshot().m_boxRange.center, getSnapshot().m_boxRange.size.x, getSnapshot().m_boxRange.size.y, getSnapshot().m_boxRange.size.z);
		ofPopMatrix();
		ofFill();
	}

    //--------------------------------------------------------------
    bool SequenceRamses::imGui(ofVec2f& windowPos, ofVec2f& windowSize)
    {
        ImGui::SetNextWindowPos(windowPos, ImGuiSetCond_Appearing);
		ImGui::SetNextWindowSize(ofVec2f(380, 364), ImGuiSetCond_Appearing);
        if (ImGui::Begin("Cell Renderer", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) 
		{
			ImGui::Text("Frame %lu / %lu", m_currFrame, m_snapshots.size());
			ImGui::Text("%lu Cells", m_snapshots[m_currFrame].getNumCells());

			ImGui::Checkbox("Render", &m_bRender);
			if(ImGui::DragFloatRange2("Density Range", &m_densityMin, &m_densityMax, 0.0001f, 0.0f, 1.0f, "Min: %.4f%%", "Max: %.4f%%")){
				m_renderShader.begin();
				m_renderShader.setUniform1f("uDensityMin", m_densityMin * m_densityRange.getSpan());
				m_renderShader.setUniform1f("uDensityMax", m_densityMax * m_densityRange.getSpan());
				m_renderShader.end();
				m_volumetricsShader.begin();
				m_volumetricsShader.setUniform1f("minDensity", m_densityMin * m_densityRange.getSpan());
				m_volumetricsShader.setUniform1f("maxDensity", m_densityMax * m_densityRange.getSpan());
				m_volumetricsShader.end();
			}
			if (ImGui::Button("Next Frame"))
			{
				setFrame(getCurrentFrame() + 1);
			}

			if(ImGui::SliderFloat("Volume Quality", &m_volumeQuality, 0, 2) | ImGui::SliderFloat("Volume Density", &m_volumeDensity, 0, 50)){
				volumetrics.setRenderSettings(1, m_volumeQuality, m_volumeDensity, 0);
			}

            windowSize = ImGui::GetWindowSize();
            ImGui::End();
        }
        else 
		{
            windowSize = ofVec2f(0);
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

		frameSettings.folder = m_folder;
		frameSettings.frameIndex = m_startIndex + index;
		frameSettings.minDensity = m_densityMin;
		frameSettings.maxDensity = m_densityMax;
		frameSettings.volumeTexture.loadData(
		    m_clearData.data(),
		    frameSettings.worldsize, frameSettings.worldsize, frameSettings.worldsize, 0,0,0, GL_RED
		);
		m_snapshots[index].setup(frameSettings);

		// Adjust the ranges.
		m_coordRange.include(m_snapshots[index].getCoordRange());
		m_sizeRange.include(m_snapshots[index].getSizeRange());
		m_densityRange.include(m_snapshots[index].getDensityRange());

		// Set normalization values to remap to [-0.5, 0.5]
		glm::vec3 coordSpan = m_coordRange.getSpan();
		m_originShift = -0.5f * coordSpan - m_coordRange.getMin();

		m_normalizeFactor = MAX(MAX(coordSpan.x, coordSpan.y), coordSpan.z);
		m_renderShader.begin();
		m_renderShader.setUniform1f("uDensityMin", m_densityMin * m_densityRange.getSpan());
		m_renderShader.setUniform1f("uDensityMax", m_densityMax * m_densityRange.getSpan());
		m_renderShader.end();
		m_volumetricsShader.begin();
		m_volumetricsShader.setUniform1f("minDensity", m_densityMin * m_densityRange.getSpan());
		m_volumetricsShader.setUniform1f("maxDensity", m_densityMax * m_densityRange.getSpan());
		m_volumetricsShader.end();
		cout << "min density " << m_densityMin * m_densityRange.getSpan() <<
		        " max density " << m_densityMax * m_densityRange.getSpan() << endl;
	}

	//--------------------------------------------------------------
	void SequenceRamses::setFrameRate(float frameRate)
	{
		m_frameRate = frameRate;
	}

	//--------------------------------------------------------------
	float SequenceRamses::getFrameRate() const
	{
		return m_frameRate;
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

		if(m_currFrame == index){
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
