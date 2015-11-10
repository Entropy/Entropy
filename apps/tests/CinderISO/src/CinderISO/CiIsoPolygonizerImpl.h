#pragma once

#include "CiIsoPolygonizer.h"
//#include "AABB.h"
#include "MesherMarchCubes.h"
#include "CellData.h"
#include "WorkQueue.h"
#include "FieldEvaluator.h"
#include "FieldValue.h"

#include "cinder/System.h"
namespace ci = cinder;

// =========================================================================
// CiIsoPolygonizer Impl
//
// =========================================================================
//
// -------------------------------------------------------------------------
HAI_PIMPL_BEGIN_TMPL( T, CiIsoPolygonizer )
{	
	typedef SpatialGrid<T, CellData,  FieldValue<T> >			GridT;
	typedef Mesher<T, GridT, CINDER_ISO_MESHER_MARCH_CUBES>		MesherT;
	typedef FieldFunc<T, CINDER_ISO_FIELDFUNC_BLINN>			FieldFuncBlinnT;
	typedef FieldFunc<T, CINDER_ISO_FIELDFUNC_METABALL>			FieldFuncMetaBallT;
	typedef FieldFunc<T, CINDER_ISO_FIELDFUNC_RADIUS>			FieldFuncRadiusT;
	typedef FieldFunc<T, CINDER_ISO_FIELDFUNC_SOFTOBJECT>		FieldFuncSoftObjectT;
	typedef FieldFunc<T, CINDER_ISO_FIELDFUNC_WYVILL>			FieldFuncWyvillT;
	typedef FieldEvaluator<T, GridT>							FieldEvaluatorT;
	
	bool									mInitialized;
	
	int										mNumThreads;
	bool									mForceMultiThread;
	
	int										mGridSize;
	CiIsoFieldFunc							mFieldFunc;
	CiIsoMesher								mMesher;
	T										mParamA;
	T										mParamB;
	T										mMaxDist;
	AABB<T>									mBounds;	
	CiIsoParticleSys<T>*					mParticles;

	// Implicit shapes with their respective field functions
	ImplicitShape<T, FieldFuncBlinnT>		mImpShapeBlinn;
	ImplicitShape<T, FieldFuncMetaBallT>	mImpShapeMetaBall;
	ImplicitShape<T, FieldFuncRadiusT>		mImpShapeRadius;
	ImplicitShape<T, FieldFuncSoftObjectT>	mImpShapeSoftObject;
	ImplicitShape<T, FieldFuncWyvillT>		mImpShapeWyvill;

	// Grid
	GridT									mGrid;

	// Field Evaluator
	FieldEvaluatorT							mFieldEval;
	
	// Enables/disables output of some vars to the console
	bool									mPrintVars;

	// ---------------------------------------------------------------------
	Impl( int gridSize );
	/*
	HAI_PIMPL_CTOR_1_DECL( int gridSize ) 
	: mInitialized( false ),
	  mNumThreads( 1 ),
	  mForceMultiThread( false ),
	  mGridSize( gridSize ),
	  mFieldFunc( CINDER_ISO_FIELDFUNC_BLINN ),
	  mMesher( CINDER_ISO_MESHER_MARCH_CUBES ),
	  // NOTE: Becareful when changing the defaults for these.
	  // It might mess with some user's expectation of the mesh.
	  mParamA( 0.5f ),
	  mParamB( 3.0f ),
	  mMaxDist( 10 ),
	  mPrintVars( false )
	{
	}
	*/
	// ---------------------------------------------------------------------
	
	bool						isInitialized() const;
	
	int							getNumThreads() const;
	void						setNumThreads( int numThreads );	
	void						forceMultiThread( bool force );
	bool						forceMultiThread() const;
	
	CiIsoFieldFunc				getFieldFunc() const;
	void						setFieldFunc( CiIsoFieldFunc fieldFunc );

	void						setImpShapeParams( CiIsoFieldFunc fieldFunc, T A, T B );
	void						setImpShapeMaxDist( CiIsoFieldFunc fieldFunc, T maxDist );
	
	T							getMaxDist() const;
	void						setMaxDist( T maxDist );	
	
	CiIsoMesher					getMesher() const;
	void						setMesher( CiIsoMesher mesher );	
	
	int							getGridSize() const;
	void						setGridSize( int gridSize );
	
	void						setParticles( CiIsoParticleSys<T> *particles );
	void						updateBounds();
	
	void						initForPolygonize();
	void						polygonizeSingleThread( ci::TriMesh* mesh );
	void						polygonizeMultiThread( ci::TriMesh* mesh );
	void						clearPolyonizeData();
	
	void*						getImpShapeAsVoidPtr( CiIsoFieldFunc fieldFunc );	
	GridT*						getGrid();
	//void						allocateGrid( int gridSize );	
	FieldEvaluatorT*			getFieldEval();	
		
	void						setPrintVars( bool val );
	bool						getPrintVars() const;
	
	void						peakMesh( float amount, ci::TriMesh *mesh );
};
