#include "CiIsoParticleSys.h"
#include "Util.h"

#include "cinder/Rand.h"
namespace ci = cinder;

template <typename T>
CiIsoParticleSys<T>::CiIsoParticleSys( size_t n ) 
: m_positions( n ), m_radii( n ), m_colors( n )
{

}

template <typename T>
CiIsoParticleSys<T>::CiIsoParticleSys( const CiIsoParticleSys &obj )
: m_positions( obj.m_positions ), m_radii( obj.m_radii ), m_colors( obj.m_colors )
{

}

template <typename T>
CiIsoParticleSys<T> & CiIsoParticleSys<T>::operator=( const CiIsoParticleSys &rhs ) 
{
	if( &rhs != this ) {
		m_positions = rhs.m_positions;
		m_radii = rhs.m_radii;
		m_colors = rhs.m_colors;
	}
	return *this;
}

	
template <typename T>
size_t CiIsoParticleSys<T>::getNumParticles() const 
{ 
	return m_positions.size(); 
}

template <typename T>
void CiIsoParticleSys<T>::setNumParticles( size_t n ) 
{ 
	m_positions.resize( n );
	m_radii.resize( n );
	m_colors.resize( n );
}

template <typename T>
void CiIsoParticleSys<T>::clear()
{
	m_positions.clear();
	m_radii.clear();
	m_colors.clear();
}

template <typename T>
void CiIsoParticleSys<T>::addParticle( const glm::tvec3<T, glm::highp> &P, T r, const color_type &color  )
{
    m_positions.push_back( ci::vec4( P, 1.0f ) );
	m_radii.push_back( r );
	m_colors.push_back( color );
}

template <typename T>
void CiIsoParticleSys<T>::generateRandomData( size_t n, T minRad, T maxRad, const point_type &P, T xSpan, T ySpan, T zSpan )
{
	m_positions.clear();
	m_radii.clear();
	m_colors.clear();

	for( size_t i = 0; i < n; ++i ) {
        T x = ci::Rand::randFloat()*xSpan - xSpan*0.5f;
        T y = ci::Rand::randFloat()*ySpan - ySpan*0.5f;
        T z = ci::Rand::randFloat()*zSpan - zSpan*0.5f;

		T rad = ci::Rand::randFloat( (float)minRad, (float)maxRad );
		
		T r = ci::Rand::randFloat( 0.1f, 1.0f );
		T g = ci::Rand::randFloat( 0.1f, 1.0f );
		T b = ci::Rand::randFloat( 0.1f, 1.0f );

		
		//std::cout << "r=" << r << ", g=" << g << ", b=" << b << std::endl;
		
		addParticle( glm::tvec3<T, glm::highp>( x, y, z ), rad, color_type( (float)r, (float)g, (float)b ) );
		
		//m_positions.push_back( point_type( x, y, z, 1 ) );
		//m_radii.push_back( rad );
    }
}

template class CiIsoParticleSys<float>;
template class CiIsoParticleSys<double>;
