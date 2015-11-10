/*
 Entropy Toolkit
 Copyright (c) 2015, Simon Geilfus - All rights reserved.
 
 Redistribution and use in source and binary forms, with or without modification, are permitted provided that
 the following conditions are met:
 
 * Redistributions of source code must retain the above copyright notice, this list of conditions and
 the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
 the following disclaimer in the documentation and/or other materials provided with the distribution.
 
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
 */

#include "Particle.h"
#include "cinder/app/App.h"
#include "cinder/Timer.h"
#include "cinder/Rand.h"
#include "cinder/Utilities.h"
#include "glm/gtc/packing.hpp"
#include "Grid.h"

using namespace std;
using namespace ci;

/*ParticleRef Particle::create( const ci::vec3 &initial, const ci::vec3 &final, uint32_t particleId )
{
    return make_shared<Particle>( initial, final, particleId );
}*/

Particle::Particle( const ci::vec3 &initial, const ci::vec3 &final, uint32_t indice )
: mInitial( initial ), mFinal( final ), mIndice( indice )
{
    
}



/*ParticleSetRef ParticleSet::create()
{
    return make_shared<ParticleSet>();
}*/

void ParticleSet::addParticle( const ci::vec3 &initial, const ci::vec3 &final, uint32_t particleId )
{
    /*auto particle = Particle::create( initial, final, particleId );
    mParticles.push_back( particle );
    return particle;*/
    mParticles.emplace_back( Particle( initial, final, particleId ) );
}

//#define BUILD_DENSITY_FILE

uint32_t colorToIndex( const ci::ColorA8u &color )
{
    return ((color.a << 24) | (color.r << 16) | (color.g << 8) | (color.b));
}

ci::ColorA8u indexToColor( uint32_t index )
{
    return ci::ColorA8u( ( ( index >> 16 ) & 0xFF ), ( ( index >> 8 ) & 0xFF ), ( index & 0xFF ), ( ( index >> 24 ) & 0xFF ) );
}

const ci::gl::VboMeshRef& ParticleSet::getVboMesh()
{
    if( !mVbo ){
        Timer timer( true );
        vector<vec4> positions, velocities;
        vector<glm::vec4> ids;
        positions.reserve( mParticles.size() );
        velocities.reserve( mParticles.size() );
        ids.reserve( mParticles.size() );
        
        uint64_t indice = 0;
        for( const auto &particle : mParticles ){
            positions.push_back( vec4( particle.getPosition(), particle.getDensity() ) );
            velocities.push_back( vec4( particle.getVelocity(), 1.0f ) );
            ColorA8u c =  indexToColor( indice );
            ids.push_back( glm::vec4( static_cast<float>(c.r),
                                     static_cast<float>(c.g), static_cast<float>(c.b), static_cast<float>(c.a) ) / 255.0f );
            indice++;
        }
        
        // find data bounding box
        vec3 min = vec3( numeric_limits<float>::max() );
        vec3 max = vec3( numeric_limits<float>::min() );
        for( auto p : positions ){
            if( p.x > max.x ) max.x = p.x;
            if( p.y > max.y ) max.y = p.y;
            if( p.z > max.z ) max.z = p.z;
            if( p.x < min.x ) min.x = p.x;
            if( p.y < min.y ) min.y = p.y;
            if( p.z < min.z ) min.z = p.z;
        }
        
        vec3 center = AxisAlignedBox3f( min, max ).getCenter();
        //mBounds     = AxisAlignedBox3f( min - center, max - center );
        mBounds     = AxisAlignedBox3f( min, max );
        // center everything
        /*for( auto &p : positions ){
         p = vec4( vec3( p ) - center, p.w );
         }
         */
        
#ifdef BUILD_DENSITY_FILE
        
        // add particles to grid
        auto bounds = getBounds();
        auto grid   = Grid( bounds, 0 );
        for( size_t i = 0; i < positions.size(); i++ ){
            grid.addPoint( vec3( positions[i] ), i );
        }
        
        // get particles density
        vector<int> particleNeighbors;
        int maxNeighbors = 0;
        for( size_t i = 0; i < positions.size(); i++ ){
            auto cells = grid.getRegionCells( vec3( positions[i] ), 8 );
            int neighbors   = 0;
            for( Grid::Cell* cell : cells ){
                for( auto indice : *cell ){
                    if( length2( positions[indice] - positions[i] ) < 64 ){
                        neighbors++;
                    }
                }
            }
            particleNeighbors.push_back(neighbors);
            positions[i].w = neighbors;
            if( maxNeighbors < neighbors ) maxNeighbors = neighbors;
        }
        
        std::ofstream os( app::getSaveFilePath().c_str() );
        cereal::BinaryOutputArchive archive( os );
        archive( particleNeighbors );
        
        cout << "maxNeighbors: " << maxNeighbors << endl;
#endif
        
        // create a vbo mesh with the data
        geom::BufferLayout layout0;
        layout0.append( geom::POSITION, 4, 0, 0 );
        
        geom::BufferLayout layout1;
        layout1.append( geom::CUSTOM_0, 4, 0, 0 );
        
        geom::BufferLayout layout2;
        layout2.append( geom::CUSTOM_1, 4, 0, 0 );
        
        auto vbo0   = gl::Vbo::create( GL_ARRAY_BUFFER, positions.size() * sizeof(vec4), positions.data(), GL_STATIC_DRAW );
        auto vbo1   = gl::Vbo::create( GL_ARRAY_BUFFER, velocities.size() * sizeof(vec4), velocities.data(), GL_STATIC_DRAW );
        auto vbo2   = gl::Vbo::create( GL_ARRAY_BUFFER, ids.size() * sizeof(glm::vec4), ids.data(), GL_STATIC_DRAW );
        mVbo        = gl::VboMesh::create( positions.size(), GL_POINTS, { { layout0, vbo0 }, { layout1, vbo1 }, { layout2, vbo2 } } );
        cout << "Mesh construction " << timer.getSeconds() << " seconds " << endl;
    }
    
    return mVbo;
}



// Mesh generation

/*float CosmicWebApp::density( const ci::vec3 &position )
{
    float smoothing = 10.1f;
    Grid::Cell cell = mGrid.getRegion( position, 1 );
    float density   = 0.0f;
    int neighbors   = 0;
    for( auto indice : cell ){
        float dist = length2( mDarkMatter.getParticles()[indice].getPosition() - position );
        if( dist < 25 ){
            density += ( 25.0f - dist ) / 25.0f;//exp( -pow( dist, 2.0f ) / 2.0f );// / pow( smoothing, 2.0f ) );
            neighbors++;
        }
    }
    if( neighbors )
        density = density / (float) neighbors;
    //cout << v << endl;
    //cout << neighbors << endl;
    //cout << cell.size() << " " << density << endl;
    return neighbors / 180.0f;
}

auto bounds = mDarkMatter.getBounds();
mGrid       = Grid( bounds, 0 );
cout << "Bounds: " << bounds.getMin() << " " << bounds.getMax() << endl;
cout << "Grid cells and cellsize: " << mGrid.getNumCells() << " " << mGrid.getCellSize() << endl;


vec3 v;
MarchingCubes marchingCubes;
float stepSize          = 5.0f / (float) mGrid.getCellSize();
const auto &particles   = mDarkMatter.getParticles();
TriMeshRef trimesh      = TriMesh::create( TriMesh::Format().positions() );

for( size_t i = 0; i < particles.size(); i++ ){
    mGrid.addPoint( particles[i].getPosition(), i );
}

auto densityFn = std::bind( &CosmicWebApp::density, this, std::placeholders::_1 );
vec3 center = bounds.getCenter();
//bounds = AxisAlignedBox3f( center - vec3( 0, 20, 20 ), center + vec3( 5, 20, 20 ) );
for( v.x = bounds.getMin().x; v.x < bounds.getMax().x; v.x+= mGrid.getCellSize() ){
    for( v.y = bounds.getMin().y; v.y < bounds.getMax().y; v.y+= mGrid.getCellSize() ){
        for( v.z = bounds.getMin().z; v.z < bounds.getMax().z; v.z+= mGrid.getCellSize() ){
            marchingCubes.march( trimesh, v, vec3(mGrid.getCellSize()), 0.2, densityFn );
            // cout << "\t " << v << " done" << endl;
        }
    }
}

cout << trimesh->getNumTriangles() << " triangles" << endl;
if( trimesh->getNumTriangles() ){
    mDarkMatterPointMesh = gl::VboMesh::create( *trimesh.get() );
}*/

void ParticleSet::write( const ci::fs::path &filepath )
{
    std::ofstream os( filepath.c_str() );
    cereal::BinaryOutputArchive archive( os );
    archive( mParticles );
}
void ParticleSet::read( const ci::fs::path &filepath, const ci::fs::path &density )
{
    {
        ifstream is( filepath.c_str() );
        cereal::BinaryInputArchive iarchive( is );
        iarchive( mParticles );
    }
    if( !density.empty() ){
        ifstream is( density.c_str() );
        cereal::BinaryInputArchive iarchive( is );
        vector<int> particleNeighbors;
        iarchive( particleNeighbors );
        for( size_t i = 0; i < mParticles.size(); i++ ){
            mParticles[i].setDensity( particleNeighbors[i] * 0.01f );
        }
    }
    cout << "Loaded " << mParticles.size() << " particles" << endl;
    getVboMesh();
}

void ParticleSet::merge( const std::vector<ci::fs::path> &filepaths, const std::vector<ci::fs::path> &densities )
{
    for( auto filepath : filepaths ){
        {
            ifstream is( filepath.c_str() );
            cereal::BinaryInputArchive iarchive( is );
            std::deque<Particle> temp;
            iarchive( temp );
            
            copy( temp.begin(), temp.end(), back_inserter( mParticles ) );
        }
    }
}


void ParticleSet::resample( const std::vector<ci::fs::path> &filepaths, uint32_t resampling )
{
    for( auto filepath : filepaths ){
        {
            ifstream is( filepath.c_str() );
            cereal::BinaryInputArchive iarchive( is );
            std::deque<Particle> temp;
            iarchive( temp );
            
            while ( temp.size() > resampling ) {
                temp.erase( temp.begin() + randInt( 0, temp.size() - 1 ) );
            }
            copy( temp.begin(), temp.end(), back_inserter( mParticles ) );
        }
    }
}
void ParticleSet::readGadgetAscii( const ci::fs::path &filepath )
{
    ifstream file( filepath.c_str(), std::ios::binary );
    string line;
    uint32_t index = 0;
    while( std::getline( file, line ) ){
        
        // split the line into strings
        auto splits = split( line, " " );
        
        // and extract the data
        vec3 position = vec3( stof( splits[1] ), stof( splits[2] ), stof( splits[3] ) );
        vec3 velocity = vec3( stof( splits[4] ), stof( splits[5] ), stof( splits[6] ) );
            
        addParticle( position, velocity, index );
        
        index++;
    }
}

ParticleSet::ParticleSet()
{
}