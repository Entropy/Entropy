/*
 *  Persistent.cpp
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
#include "Persistent.h"

void Persistent::add(const string& name, bool& parameter)
{
    if (bools.find(name) != bools.end()) ofLogError() << "A float named " << name << " already exists, overwriting it.";
    bools[name].set(&parameter);
}

void Persistent::add(const string& name, int& parameter, int min, int max)
{
    if (ints.find(name) != ints.end()) ofLogError() << "An int named " << name << " already exists, overwriting it.";
    ints[name].set(&parameter, min, max);
}

void Persistent::add(const string& name, unsigned& parameter, unsigned min, unsigned max)
{
    if (unsigneds.find(name) != unsigneds.end()) ofLogError() << "An unsigned named " << name << " already exists, overwriting it.";
    unsigneds[name].set(&parameter, min, max);
}

void Persistent::add(const string& name, float& parameter, float min, float max)
{
    if (floats.find(name) != floats.end()) ofLogError() << "A float named " << name << " already exists, overwriting it.";
    floats[name].set(&parameter, min, max);
}

void Persistent::add(const string& name, glm::vec2& parameter, const glm::vec2& min, const glm::vec2& max)
{
    if (vec2fs.find(name) != vec2fs.end()) ofLogError() << "An ofVec2f named " << name << " already exists, overwriting it.";
    vec2fs[name].set(&parameter, min, max);
}

void Persistent::add(const string& name, glm::vec3& parameter, const glm::vec3& min, const glm::vec3& max)
{
    if (vec3fs.find(name) != vec3fs.end()) ofLogError() << "An ofVec3f named " << name << " already exists, overwriting it.";
    vec3fs[name].set(&parameter, min, max);
}

void Persistent::add(const string& name, ofFloatColor& parameter, const ofFloatColor& min, const ofFloatColor& max)
{
    if (floatColors.find(name) != floatColors.end()) ofLogError() << "An ofFloatColor named " << name << " already exists, overwriting it.";
    floatColors[name].set(&parameter, min, max);
}

void Persistent::load(const string& fileName)
{
    ofXml xml;
    xml.load(fileName);
    xml.setTo("settings");
    
    setFromXml("bools", bools, xml);
    setFromXml("unsigneds", unsigneds, xml);
    setFromXml("ints", ints, xml);
    setFromXml("floats", floats, xml);
    setFromXml("vec2fs", vec2fs, xml);
    setFromXml("vec3fs", vec3fs, xml);
    setFromXml("floatColors", floatColors, xml);
}

void Persistent::save(const string& fileName)
{
    ofXml xml;
    xml.addChild("settings");
    xml.setTo("settings");
    
    addToXml("bools", bools, xml);
    addToXml("unsigneds", unsigneds, xml);
    addToXml("ints", ints, xml);
    addToXml("floats", floats, xml);
    addToXml("vec2fs", vec2fs, xml);
    addToXml("vec3fs", vec3fs, xml);
    addToXml("floatColors", floatColors, xml);
    
    xml.save(fileName);
}
