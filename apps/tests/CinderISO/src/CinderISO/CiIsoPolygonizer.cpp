#if defined( WIN32 )
#pragma warning( disable : 4244 )
#endif 

#include "CiIsoPolygonizer.h"
#include "CiIsoPolygonizerImpl.h"
#include "CiIsoPolygonizerImpl.inl"
/*
#include "AABB.h"
#include "MesherMarchCubes.h"
#include "CellData.h"
#include "WorkQueue.h"
#include "FieldEvaluator.h"
#include "FieldValue.h"
*/


// =========================================================================
//
// CiIsoPolygonizer<T>
//
// =========================================================================
//
// -------------------------------------------------------------------------
template <typename T>
CiIsoPolygonizer<T>::CiIsoPolygonizer( int gridSize )
: HAI_PIMPL_CTOR_1_CALL( gridSize )
{
}
// -------------------------------------------------------------------------
template <typename T>
CiIsoPolygonizer<T>::~CiIsoPolygonizer()
{
}
// -------------------------------------------------------------------------
template <typename T>
int CiIsoPolygonizer<T>::getNumThreads() const
{
	return getImpl()->getNumThreads();
}
// -------------------------------------------------------------------------
template <typename T>
void CiIsoPolygonizer<T>::setNumThreads( int numThreads )
{
	getImpl()->setNumThreads( numThreads );
}
// -------------------------------------------------------------------------
template <typename T>
CiIsoFieldFunc CiIsoPolygonizer<T>::getFieldFunc() const
{
	return getImpl()->getFieldFunc();
}
// -------------------------------------------------------------------------
template <typename T>
void CiIsoPolygonizer<T>::setFieldFunc( CiIsoFieldFunc fieldFunc )
{
	getImpl()->setFieldFunc( fieldFunc );
}
// -------------------------------------------------------------------------
template <typename T>
T CiIsoPolygonizer<T>::getFieldFuncParamA() const
{
	return getImpl()->mParamA;
}
// -------------------------------------------------------------------------
template <typename T>
void CiIsoPolygonizer<T>::setFieldFuncParamA( T A )
{
	getImpl()->mParamA = A;
}
// -------------------------------------------------------------------------
template <typename T>
T CiIsoPolygonizer<T>::getFieldFuncParamB() const
{
	return getImpl()->mParamB;
}
// -------------------------------------------------------------------------
template <typename T>
void CiIsoPolygonizer<T>::setFieldFuncParamB( T B )
{
	getImpl()->mParamB = B;
}
// -------------------------------------------------------------------------
template <typename T>
void CiIsoPolygonizer<T>::getFieldFuncParams( T *A, T *B ) const
{
	*A = getImpl()->mParamA;
	*B = getImpl()->mParamB;
}
// -------------------------------------------------------------------------
template <typename T>
void CiIsoPolygonizer<T>::setFieldFuncParams( T A, T B )
{
	getImpl()->mParamA = A;
	getImpl()->mParamB = B;
}
// -------------------------------------------------------------------------
template <typename T>
T CiIsoPolygonizer<T>::getMaxDist() const
{
	return getImpl()->mMaxDist;
}
// -------------------------------------------------------------------------
template <typename T>
void CiIsoPolygonizer<T>::setMaxDist( T maxDist )
{
	getImpl()->mMaxDist = maxDist;
}
// -------------------------------------------------------------------------
template <typename T>
CiIsoMesher CiIsoPolygonizer<T>::getMesher() const
{
	return getImpl()->getMesher();
	//return getImpl()->mMesher;
}
// -------------------------------------------------------------------------
template <typename T>
void CiIsoPolygonizer<T>::setMesher( CiIsoMesher mesher )
{
	getImpl()->setMesher( mesher );
	//getImpl()->mMesher = mesher;
}
// -------------------------------------------------------------------------
template <typename T>
int	CiIsoPolygonizer<T>::getGridSize() const
{
	return getImpl()->getGridSize();
	//return getImpl()->mGridSize;
}
// -------------------------------------------------------------------------
template <typename T>
void CiIsoPolygonizer<T>::setGridSize( int gridSize )
{
	// NOTE: setGridSize only sets the size member variables. The data
	//       initialization takes place in initForPolygonize.
	
	getImpl()->setGridSize( gridSize );
}
// -------------------------------------------------------------------------
template <typename T>
void CiIsoPolygonizer<T>::setParticles( CiIsoParticleSys<T>* particles )
{
	getImpl()->setParticles( particles );
}
// -------------------------------------------------------------------------
template <typename T>
void CiIsoPolygonizer<T>::updateBounds()
{	
	getImpl()->updateBounds();
}
// -------------------------------------------------------------------------
template <typename T>
void CiIsoPolygonizer<T>::initForPolygonize()
{
	getImpl()->initForPolygonize();
}
// -------------------------------------------------------------------------
template <typename T>
void CiIsoPolygonizer<T>::polygonize( ci::TriMesh* mesh, bool autoUpdateBounds )
{
	if( ! getImpl()->isInitialized() ) {
		getImpl()->initForPolygonize();		
	}
	
	if( autoUpdateBounds ) {
		updateBounds();
	}

	if( getNumThreads() > 1 || getImpl()->forceMultiThread() )
		getImpl()->polygonizeMultiThread( mesh );
	else
		getImpl()->polygonizeSingleThread( mesh );

}
// -------------------------------------------------------------------------
template <typename T>
void CiIsoPolygonizer<T>::clearPolyonizeData()
{
	getImpl()->clearPolyonizeData();
}
// -------------------------------------------------------------------------
template <typename T>
void CiIsoPolygonizer<T>::forceMultiThread( bool force )
{
	getImpl()->forceMultiThread( force );
}
// -------------------------------------------------------------------------
template <typename T>
void CiIsoPolygonizer<T>::setPrintVars( bool val )
{
	getImpl()->setPrintVars( val );
}
// -------------------------------------------------------------------------
template <typename T>
bool CiIsoPolygonizer<T>::getPrintVars() const
{
	return getImpl()->getPrintVars();
}
// -------------------------------------------------------------------------
template <typename T>
void CiIsoPolygonizer<T>::peakMesh( float amount, ci::TriMesh *mesh )
{
	getImpl()->peakMesh( amount, mesh );
}
// -------------------------------------------------------------------------
template class CiIsoPolygonizer<float>;
template class CiIsoPolygonizer<double>;
// -------------------------------------------------------------------------
