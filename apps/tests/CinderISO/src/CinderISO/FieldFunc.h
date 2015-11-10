#pragma once

//#include "HaiTypes.h"
#include "Util.h"

template <typename T, int>
struct FieldFunc {};

template <typename T>
struct FieldFunc<T, CINDER_ISO_FIELDFUNC_BLINN> 
{
	// ---------------------------------------------------------------------
	// NOTES:
	// B - blobbiness factor
	// Reasonable values: A=0.5, B=3.0
	// ---------------------------------------------------------------------

	/*
	// Function
	static T eval( T x2, T radius, T A, T B ) {
		// Make sure B is negative
		B = -B;
		T r2 = 1.0f/(radius*radius);
		float d = (B*r2)*x2 - B;
		v4sf ed = exp_ps( _mm_set_ps1( d ) );
		float expVal = ((float*)&ed)[0];
		T result = A*((T)expVal);
		return result;	
	}
	
	// Derivative
	static T deriv( T x2, T radius, T A, T B ) {
		// Make sure B is negative
		B = -B;
		T r2 = 1.0f/(radius*radius);
		T d  = (B*r2)*x2 - B;
		v4sf ed = exp_ps( _mm_set_ps1( d ) );
		float expVal = ((float*)&ed)[0];
		T result = A*B*r2*((T)expVal);
		return result;		
	}
	*/	

	// Function
	static T eval( T x2, T radius, T A, T B ) {
		// Make sure B is negative
		B = -B;
		T r2 = 1.0f/(radius*radius);
		T d  = (B*r2)*x2 - B;
		if( d >= 9.0f ) 
		 	return 0;
		T result = A*((T)expf( (float)d ));
		return result;	
	}
	
	// Derivative
	static T deriv( T x2, T radius, T A, T B ) {
		// Make sure B is negative
		B = -B;
		T r2 = 1.0f/(radius*radius);
		T d  = (B*r2)*x2 - B;
		if( d >= 9.0f )
			return 0;
		T result = A*B*r2*((T)expf( (float)d ));
		return result;		
	}


	/*
	// Function
	static T eval( T x2, T radius, T A, T B ) {
		// Make sure B is negative
		B = -B;
		T r2 = 1.0f/(radius*radius);
		T d  = (B*r2)*x2 - B;
		//if( d >= 9.0f ) 
		// 	return 0;
		T result = A*((T)expf( (T)d ));
		return result;	
	}
	
	// Derivative
	static T deriv( T x2, T radius, T A, T B ) {
		// Make sure B is negative
		B = -B;
		T r2 = 1.0f/(radius*radius);
		T d  = (B*r2)*x2 - B;
		//if( d >= 9.0f )
		//	return 0;
		T result = A*B*r2*((T)expf( (T)d ));
		return result;		
	}
	*/
};

template <typename T>
struct FieldFunc<T, CINDER_ISO_FIELDFUNC_METABALL>
{
	// ---------------------------------------------------------------------
	// NOTES:
	// Reasonable values: A=[1.25 B=5.0
	// ---------------------------------------------------------------------
	
	// Function
	static T eval( T x2, T radius, T A, T B ) {
		T OneOverB = 1.0f/B;
		T BOver3 = B*0.333333333333f;
		if( x2 >= 0 && x2 < BOver3 ) {
			return A*(1.0f - (3.0f*x2*x2)*(OneOverB*OneOverB));
		}
		else if( x2 >= BOver3 && x2 < B ) {
			return 1.5f*A*((1.0f - x2*OneOverB)*(1.0f - x2*OneOverB));
		}
		return 0;
	}
	
	// Derivative
	static T deriv( T x2, T radius, T A, T B ) {
		T OneOverB = 1.0f/B;
		T BOver3 = B*0.333333333333f;
		if( x2 >= 0 && x2 < BOver3 ) {
			return -6.0f*A*x2*(OneOverB*OneOverB);
		}
		else if( x2 >= BOver3 && x2 < B ) {
			return -3.0f*A*(B - x2)*(OneOverB*OneOverB);
		}
		return 0;
	}
};

template <typename T>
struct FieldFunc<T, CINDER_ISO_FIELDFUNC_RADIUS>
{
	// ---------------------------------------------------------------------
	// NOTES:
	// Reasonable values: A=1.0, B=1.0
	// ---------------------------------------------------------------------
	
	// Function
	static T eval( T x2, T radius, T A, T B ) {
		T RR = radius*radius;
		x2 = x2/RR;
		x2 = x2 - 1;
		x2 = 1.0f/(x2*x2);
		return A*x2;
	}
	
	// Derivative
	static T deriv( T x2, T radius, T A, T B ) {
		//T x = sqrt( x2 );
		T RR = radius*radius;
		x2 = x2/RR;
		x2 = x2 - 1;
		return A*-2.0f/(x2*x2*x2);
	}
};

template <typename T>
struct FieldFunc<T, CINDER_ISO_FIELDFUNC_SOFTOBJECT>
{

	// ---------------------------------------------------------------------
	// NOTES:
	// Reasonable values: A=2.0, B=1.5
	// ---------------------------------------------------------------------
	
	// Function
	static T eval( T x2, T radius, T A, T B ) {
		if( x2 > B )
			return 0;
		x2 = x2*radius;
		T b2 = 1.0f/(B*B);
		T t0 = (4.0f/9.0f)*(x2*x2*x2*b2*b2*b2);
		T t1 = (17.0f/9.0f)*(x2*x2*b2*b2);
		T t2 = (22.0f/9.0f)*(x2*b2);
		T result = A*(1.0f - t0 + t1 - t2);
		return result;
	}
	
	// Derivative
	static T deriv( T x2, T radius, T A, T B ) {
		if( x2 > B )
			return 0;
		x2 = x2*radius;
		T b2 = 1.0f/(B*B);
		T t0 = (24.0f/9.0f)*(x2*x2*b2*b2*b2);
		T t1 = (68.0f/9.0f)*(x2*b2*b2);
		T t2 = (44.0f/9.0f)*(b2);
		T result = A*(-t0 + t1 - t2);
		return result;
	}
};

template <typename T>
struct FieldFunc<T, CINDER_ISO_FIELDFUNC_WYVILL>
{

	// ---------------------------------------------------------------------
	// NOTES:
	// Reasonable values: A=2.0, B=1.5
	// ---------------------------------------------------------------------
	
	// Function
	static T eval( T x2, T radius, T A, T B ) {
		if( x2 > B )
			return 0;
		T b2 = 1.0f/(B*B);
		//T RR = radius; //radius*radius;
		x2 = x2*radius;
		T t = 1.0f - (x2*b2);
		return A*(t*t*t);
	}
	
	// Derivative
	static T deriv( T x2, T radius, T A, T B ) {
		if( x2 > B )
			return 0;
		T b2 = 1.0f/(B*B);
		x2 = x2*radius;
		T t = 1.0f - (x2*b2);
		return A*-6.0f*(t*t)*b2;		
	}	
};

