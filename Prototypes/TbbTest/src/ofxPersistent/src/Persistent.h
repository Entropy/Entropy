/*
 *  ofxPersistent.h
 *
 *  Copyright (c) 2016, Neil Mendoza, http://www.neilmendoza.com
 *  All rights reserved. 
 *  
 *  Redistribution and use in source and binary forms, with or without 
 *  modification, are permitted provided that the following conditions are met: 
 *  
 *  * Redistributions of source code must retain the above copyright notice, 
 *    this list of conditions and the following disclaimer. 
 *  * Redistributions in binary form must reproduce the above copyright 
 *    notice, this list of conditions and the following disclaimer in the 
 *    documentation and/or other materials provided with the distribution. 
 *  * Neither the name of Neil Mendoza nor the names of its contributors may be used 
 *    to endorse or promote products derived from this software without 
 *    specific prior written permission. 
 *  
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 *  POSSIBILITY OF SUCH DAMAGE. 
 *
 */
#pragma once

#include "ofMain.h"
#include "PersistentType.h"

// TODO
// add defaults to add()
// add ordering and hierarchy

class Persistent
{
public:
    void add(const string& name, bool& parameter);
    void add(const string& name, int& parameter, int min, int max);
    void add(const string& name, unsigned& parameter, unsigned min, unsigned max);
    void add(const string& name, float& parameter, float min, float max);
    void add(const string& name, glm::vec2& parameter, const glm::vec2& min, const glm::vec2& max);
    void add(const string& name, glm::vec3& parameter, const glm::vec3& min, const glm::vec3& max);
    void add(const string& name, ofFloatColor& parameter, const ofFloatColor& min, const ofFloatColor& max);

    void load(const string& fileName = "settings.xml");
    void save(const string& fileName = "settings.xml");
    
    map<string, PersistentBool>& getBools() { return bools; }
    map<string, PersistentInt>& getInts() { return ints; }
    map<string, PersistentUnsigned>& getUnsigneds() { return unsigneds; }
    map<string, PersistentFloat>& getFloats() { return floats; }
    map<string, PersistentVec2f>& getVec2fs() { return vec2fs; }
    map<string, PersistentVec3f>& getVec3fs() { return vec3fs; }
    map<string, PersistentFloatColor>& getFloatColors() { return floatColors; }
    
private:
    template<typename T>
    void addToXml(const string& tagName, map<string, PersistentType<T> > params, ofXml& xml)
    {
        xml.addChild(tagName);
        if (xml.setTo(tagName))
        {
            for (auto& pair : params)
            {
                xml.addValue(pair.first, *pair.second.getValue());
            }
            xml.setToParent();
        }
        else ofLogError() << "addToXml(): xml.setTo(" << tagName << ") failed";;
    }
    
    template<typename T>
    void setFromXml(const string& tagName, map<string, PersistentType<T> > params, ofXml& xml)
    {
        if (xml.setTo(tagName))
        {
            for (auto& pair : params)
            {
                *pair.second.getValue() = xml.getValue<T>(pair.first);
            }
            xml.setToParent();
        }
        else ofLogError() << "setFromXml(): xml.setTo(" << tagName << ") failed";;
    }
    
    map<string, PersistentBool> bools;
    map<string, PersistentInt> ints;
    map<string, PersistentUnsigned> unsigneds;
    map<string, PersistentFloat> floats;
    map<string, PersistentVec2f> vec2fs;
    map<string, PersistentVec3f> vec3fs;
    map<string, PersistentFloatColor> floatColors;
};
