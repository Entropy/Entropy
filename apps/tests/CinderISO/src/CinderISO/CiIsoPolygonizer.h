#pragma once

//#include "HaiTypes.h"
#include "CiIsoParticleSys.h"

#include "cinder/TriMesh.h"

enum CiIsoFieldFunc {
	CINDER_ISO_FIELDFUNC_BLINN,
	CINDER_ISO_FIELDFUNC_METABALL,
	CINDER_ISO_FIELDFUNC_RADIUS,
	CINDER_ISO_FIELDFUNC_SOFTOBJECT,
	CINDER_ISO_FIELDFUNC_WYVILL
};

enum CiIsoMesher {
	CINDER_ISO_MESHER_MARCH_CUBES,
	CINDER_ISO_MESHER_CELL_DISTORT,
	CINDER_ISO_MESHER_MARCH_TETS,
	CINDER_ISO_MESHER_DUAL_MARCH_CUBES
};

template <typename T>
class CiIsoPolygonizer
{
	//HAI_PIMPL_DECL
public:

	CiIsoPolygonizer( int gridSize = 0 );
	~CiIsoPolygonizer();
	
	int						getNumThreads() const;
	void					setNumThreads( int numThreads );
	
	CiIsoFieldFunc			getFieldFunc() const;
	void					setFieldFunc( CiIsoFieldFunc fieldFunc );
	
	T						getFieldFuncParamA() const;
	void					setFieldFuncParamA( T A );
	T						getFieldFuncParamB() const;
	void					setFieldFuncParamB( T B );
	void					getFieldFuncParams( T* A, T* B ) const;
	void					setFieldFuncParams( T A, T B );
	
	// Sets max influence dist
	T						getMaxDist() const;
	void					setMaxDist( T maxDist );
	
	CiIsoMesher				getMesher() const;
	void					setMesher( CiIsoMesher mesher );
	
	// NOTE: setGridSize only sets the size member variables. The data
	//       initialization takes place in initForPolygonize.
	int						getGridSize() const;
	void					setGridSize( int gridSize );
	
	void					setParticles( CiIsoParticleSys<T>* particles );
	void					updateBounds();
		
	// By default, polygonize will call the Pimpl's initForPolygonize.
	// If you change the fieldFunction, grid size, or number of threads
	// initForPolygonize needs to be called after any or all of those
	// fields are changed.
	void					initForPolygonize();
	void					polygonize( ci::TriMesh *mesh, bool autoUpdateBounds = true );
	void					clearPolyonizeData();
		
	// This will force the polygonizer to operate in multithreaded
	// behavior - even if there is only one thread. This is primary
	// used to debug the threaded behavior. There is no real reason
	// to use this under normal operations.
	void					forceMultiThread( bool force = true );
	
	// Enables/disables output of some vars to the console
	void					setPrintVars( bool val );
	bool					getPrintVars() const;
	
	// Operations
	void					peakMesh( float amount, ci::TriMesh *mesh );	
};

typedef CiIsoPolygonizer<float>		CiIsoPolygonizerf;
typedef CiIsoPolygonizer<double>	CiIsoPolygonizerd;
