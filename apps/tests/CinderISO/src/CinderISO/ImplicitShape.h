#pragma once

#include "CiIsoParticleSys.h"
//#include "AABB.h"
//#include "AlignedTypes.h"
#include "FieldFunc.h"

#include "cinder/Color.h"
#include "cinder/Vector.h"

template <typename T, typename FieldFuncT>
class ImplicitShape {
public:

	typedef T										scalar_type;
	typedef glm::tvec4<T, glm::highp>				point_type;
	typedef glm::tvec4<T, glm::highp>				vec_type;
	typedef ci::Color								color_type;
	typedef FieldFuncT								fieldfunc_type;

	ImplicitShape()
	: mParamA( 1 ), mParamB( 1 ), mMaxDist( ImplicitShape::DefaultMaxDist() )
	{}

	ImplicitShape( CiIsoParticleSys<T> *particles, T paramA, T paramB, T maxDist = ImplicitShape::DefaultMaxDist() )
	: mParticles( particles ), mParamA( paramA ), mParamB( paramB ), mMaxDist( maxDist )
	{
		updateBounds();
	}

	static T DefaultMaxDist() {
		return static_cast<T>( 13.0 );
	}
	
	void setParticles( CiIsoParticleSys<T> *particles ) {
		mParticles = particles;
	}
	
	void setParams( T paramA, T paramB ) {
		mParamA = paramA;
		mParamB = paramB;
	}
	
	void setParamA( T paramA ) {
		mParamA = paramA;
	}
	
	void setParamB( T paramB ) {
		mParamB = paramB;
	}
	
	T getMaxDist() const {
		return mMaxDist;
	}
	
	void setMaxDist( T maxDist ) {
		mMaxDist = maxDist;
	}
	
	void updateBounds() {
        
        ci::AxisAlignedBox3f resultBounds;
		typename CiIsoParticleSys<T>::const_point_iterator  cit_pos = mParticles->positionsBegin();
		typename CiIsoParticleSys<T>::const_scalar_iterator cit_rad = mParticles->radiiBegin();	
		for( ; cit_pos != mParticles->positionsEnd(); ++cit_pos, ++cit_rad ) {
			const point_type &pos = *cit_pos;
			const scalar_type &rad = *cit_rad;
            ci::AxisAlignedBox3f bound( toVec3( pos ), rad );
			resultBounds.include( bound );
		}
		mBounds = resultBounds;
	}

	const ci::AxisAlignedBox3f & getBounds() const {
		return mBounds; 
	}
	
	bool hasInfluence( const point_type &P, T P_radius ) const {
		bool result = false;
		T P_radiusSq = P_radius*P_radius;		
		typename CiIsoParticleSys<T>::const_point_iterator  cit_pos = mParticles->positionsBegin();
		typename CiIsoParticleSys<T>::const_scalar_iterator cit_rad = mParticles->radiiBegin();	
		for( ; cit_pos != mParticles->positionsEnd(); ++cit_pos, ++cit_rad ) {
			const point_type &pos = *cit_pos;
			const scalar_type &rad = *cit_rad;
			vec_type v = pos - P;
			T radSq = rad*rad;
			T r2 = v.lengthSquared() - radSq - P_radiusSq;
			if( r2 < mMaxDist ) {
				result = true;
				break;
			}
		}
		return result;
	}
	
	T evalFieldValue( const point_type &P ) const {
		/*
		T result = 0;
		
		HaiVector<scalar_type> arrayDist, arrayRad;
		
		typename CiIsoParticleSys<T>::const_point_iterator  cit_pos = mParticles->positionsBegin();
		typename CiIsoParticleSys<T>::const_scalar_iterator cit_rad = mParticles->radiiBegin();	
		for( ; cit_pos != mParticles->positionsEnd(); ++cit_pos, ++cit_rad ) {
			const point_type &pos = *cit_pos;
			const scalar_type &rad = *cit_rad;
			vec_type v = pos - P;
			T distSq = v.lengthSquared();
			if( distSq < getMaxDist() )  
			{
				arrayRad.push_back( rad );
				arrayDist.push_back( distSq );
				
				//float fieldVal = fieldfunc_type::eval( distSq, rad, mParamA, mParamB );
				//result += fieldVal;
			}
		}
		
		if( ! arrayDist.empty() ) {
			T *ptrDist = &(arrayDist[0]);
			T *ptrRad  = &(arrayRad[0]);
			int n = arrayRad.size();
			for( int i = 0; i < n; ++i ) {
				result += fieldfunc_type::eval( ptrDist[i], ptrRad[i], mParamA, mParamB );
			}
		}
		
		return result;
		*/
		
		T result = 0;
		typename CiIsoParticleSys<T>::const_point_iterator  cit_pos = mParticles->positionsBegin();
		typename CiIsoParticleSys<T>::const_scalar_iterator cit_rad = mParticles->radiiBegin();	
		for( ; cit_pos != mParticles->positionsEnd(); ++cit_pos, ++cit_rad ) {
			const point_type  &pos = *cit_pos;
			const scalar_type &rad = *cit_rad;
			vec_type v = pos - P;
			T distSq = v.lengthSquared();
			if( distSq >= getMaxDist() )
				continue; 
			T fieldVal = fieldfunc_type::eval( distSq, rad, mParamA, mParamB );
			result += fieldVal;
		}		
		return result;
	}

	/*
	T evalFieldValue( const point_type &P ) const {
		T result = 0;
		Ci = color_type( 0, 0, 0 );
		float distAccum = 0;
		typename CiIsoParticleSys<T>::const_point_iterator  cit_pos = mParticles->positionsBegin();
		typename CiIsoParticleSys<T>::const_scalar_iterator cit_rad = mParticles->radiiBegin();		
		typename CiIsoParticleSys<T>::const_color_iterator  cit_color = mParticles->colorsBegin();
		for( ; cit_pos != mParticles->positionsEnd(); ++cit_pos, ++cit_rad ) {
			const point_type &pos   = *cit_pos;
			const scalar_type &rad  = *cit_rad;
			const color_type &color = *cit_color;
			vec_type v = pos - P;
			T distSq = v.lengthSquared();
			if( distSq < getMaxDist() )  
			{
				float fieldVal = fieldfunc_type::eval( distSq, rad, mParamA, mParamB );
				result += fieldVal;
			}
		}
		return result;
	}
	*/
	
	vec_type evalFieldGradient( const point_type &P ) const {
		vec_type result = vec_type::zero();
		T fieldValue = 0;
		typename CiIsoParticleSys<T>::const_point_iterator  cit_pos = mParticles->positionsBegin();
		typename CiIsoParticleSys<T>::const_scalar_iterator cit_rad = mParticles->radiiBegin();	
		for( ; cit_pos != mParticles->positionsEnd(); ++cit_pos, ++cit_rad ) {
			const point_type &pos = *cit_pos;
			const scalar_type &rad = *cit_rad;
			vec_type v = pos - P;
			T distSq = v.lengthSquared();
			if( distSq >= getMaxDist() ) 
				continue;
			T coeff = fieldfunc_type::deriv( distSq, rad, mParamA, mParamB );
			fieldValue += coeff;
			result.x += v.x*coeff;
			result.y += v.y*coeff;
			result.z += v.z*coeff;
		}
		//result.normalize();
		return result;
	}
	
	color_type evalFieldColor( const point_type &P ) const {
		color_type result = color_type( 0, 0, 0 );
		typename CiIsoParticleSys<T>::const_point_iterator  cit_pos = mParticles->positionsBegin();
		typename CiIsoParticleSys<T>::const_scalar_iterator cit_rad = mParticles->radiiBegin();		
		typename CiIsoParticleSys<T>::const_color_iterator  cit_color = mParticles->colorsBegin();
		for( ; cit_pos != mParticles->positionsEnd(); ++cit_pos, ++cit_rad, ++cit_color ) {
			const point_type &pos   = *cit_pos;
			//const scalar_type &rad  = *cit_rad;
			const color_type &color = *cit_color;
			vec_type v = pos - P;
			T distSq = v.lengthSquared();
			//if( distSq < getMaxDist() )  
			{
				float oneOver = 1.0f/((float)distSq + 0.001f);
				result.r += oneOver*color.r;
				result.g += oneOver*color.g;
				result.b += oneOver*color.b;
			}
		}
		return result;		
	}

private:
	ci::AxisAlignedBox3f    mBounds;
	CiIsoParticleSys<T>		*mParticles;
	T						mParamA;
	T						mParamB;
	T						mMaxDist;
};