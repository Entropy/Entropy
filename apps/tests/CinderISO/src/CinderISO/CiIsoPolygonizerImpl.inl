#include "CiIsoPolygonizerImpl.h"

// -------------------------------------------------------------------------
HAI_PIMPL_TFUNC_CTOR( T, CiIsoPolygonizer )::Impl( int gridSize ) 
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
// -------------------------------------------------------------------------
HAI_PIMPL_TFUNC( T, CiIsoPolygonizer, bool )::isInitialized() const 
{
	return mInitialized;
}
// -------------------------------------------------------------------------
HAI_PIMPL_TFUNC( T, CiIsoPolygonizer, int )::getNumThreads() const 
{
	return mNumThreads;
}
// -------------------------------------------------------------------------
HAI_PIMPL_TFUNC( T, CiIsoPolygonizer, void )::setNumThreads( int numThreads ) 
{
	// If numThreads is not value between 2 and the number of cores on 
	// the system - force it to 1.		
	mNumThreads = clamp( numThreads, 1, ci::System::getNumCores() );
}
// -------------------------------------------------------------------------
HAI_PIMPL_TFUNC( T, CiIsoPolygonizer, void )::forceMultiThread( bool force ) 
{
	mForceMultiThread = force;
}
// -------------------------------------------------------------------------
HAI_PIMPL_TFUNC( T, CiIsoPolygonizer, bool )::forceMultiThread() const 
{
	return mForceMultiThread;
}
// -------------------------------------------------------------------------
HAI_PIMPL_TFUNC( T, CiIsoPolygonizer, CiIsoFieldFunc )::getFieldFunc() const 
{
	return mFieldFunc;
}
// -------------------------------------------------------------------------
HAI_PIMPL_TFUNC( T, CiIsoPolygonizer, void )::setFieldFunc( CiIsoFieldFunc fieldFunc ) 
{
	mFieldFunc = fieldFunc;
}
// -------------------------------------------------------------------------
HAI_PIMPL_TFUNC( T, CiIsoPolygonizer, void )::setImpShapeParams( CiIsoFieldFunc fieldFunc, T A, T B ) 
{
	switch( fieldFunc ) {
		case CINDER_ISO_FIELDFUNC_BLINN      : mImpShapeBlinn.setParams( A, B );      break;
		case CINDER_ISO_FIELDFUNC_METABALL   : mImpShapeMetaBall.setParams( A, B );   break;
		case CINDER_ISO_FIELDFUNC_RADIUS     : mImpShapeRadius.setParams( A, B );	  break;
		case CINDER_ISO_FIELDFUNC_SOFTOBJECT : mImpShapeSoftObject.setParams( A, B ); break;
		case CINDER_ISO_FIELDFUNC_WYVILL     : mImpShapeWyvill.setParams( A, B );     break;		
	}	
}
// -------------------------------------------------------------------------
HAI_PIMPL_TFUNC( T, CiIsoPolygonizer, void )::setImpShapeMaxDist( CiIsoFieldFunc fieldFunc, T maxDist ) 
{
	switch( fieldFunc ) {
		case CINDER_ISO_FIELDFUNC_BLINN      : mImpShapeBlinn.setMaxDist( maxDist );      break;
		case CINDER_ISO_FIELDFUNC_METABALL   : mImpShapeMetaBall.setMaxDist( maxDist );   break;
		case CINDER_ISO_FIELDFUNC_RADIUS     : mImpShapeRadius.setMaxDist( maxDist );	  break;
		case CINDER_ISO_FIELDFUNC_SOFTOBJECT : mImpShapeSoftObject.setMaxDist( maxDist ); break;
		case CINDER_ISO_FIELDFUNC_WYVILL     : mImpShapeWyvill.setMaxDist( maxDist );     break;		
	}	
}
// -------------------------------------------------------------------------
HAI_PIMPL_TFUNC( T, CiIsoPolygonizer, T )::getMaxDist() const
{
	return mMaxDist;
}
// -------------------------------------------------------------------------
HAI_PIMPL_TFUNC( T, CiIsoPolygonizer, void )::setMaxDist( T maxDist )
{
	mMaxDist = maxDist;
}
// -------------------------------------------------------------------------
HAI_PIMPL_TFUNC( T, CiIsoPolygonizer, CiIsoMesher )::getMesher() const
{
	return mMesher;
}
// -------------------------------------------------------------------------
HAI_PIMPL_TFUNC( T, CiIsoPolygonizer, void )::setMesher( CiIsoMesher mesher )
{
	mMesher = mesher;
}
// -------------------------------------------------------------------------
HAI_PIMPL_TFUNC( T, CiIsoPolygonizer, int )::getGridSize() const 
{
	return mGridSize;
}
// -------------------------------------------------------------------------
HAI_PIMPL_TFUNC( T, CiIsoPolygonizer, void )::setGridSize( int gridSize )
{
	mGridSize = gridSize;
}
// -------------------------------------------------------------------------
HAI_PIMPL_TFUNC( T, CiIsoPolygonizer, void* )::getImpShapeAsVoidPtr( CiIsoFieldFunc fieldFunc ) 
{
	void* result = NULL;
	switch( fieldFunc ) {
		case CINDER_ISO_FIELDFUNC_BLINN      : result = (void*)&mImpShapeBlinn;      break;
		case CINDER_ISO_FIELDFUNC_METABALL   : result = (void*)&mImpShapeMetaBall;   break;
		case CINDER_ISO_FIELDFUNC_RADIUS     : result = (void*)&mImpShapeRadius;	 break;
		case CINDER_ISO_FIELDFUNC_SOFTOBJECT : result = (void*)&mImpShapeSoftObject; break;
		case CINDER_ISO_FIELDFUNC_WYVILL     : result = (void*)&mImpShapeWyvill;     break;		
	}
	return result;
}
// -------------------------------------------------------------------------
HAI_PIMPL_TFUNC( T, CiIsoPolygonizer, typename CiIsoPolygonizer<T>::Impl::GridT* )::getGrid() 
{
	return &mGrid;
}
// -------------------------------------------------------------------------
HAI_PIMPL_TFUNC( T, CiIsoPolygonizer, typename CiIsoPolygonizer<T>::Impl::FieldEvaluatorT* )::getFieldEval() 
{
	return &mFieldEval;
}
// -------------------------------------------------------------------------
HAI_PIMPL_TFUNC( T, CiIsoPolygonizer, void )::setParticles( CiIsoParticleSys<T> *particles ) 
{
	mImpShapeBlinn.setParticles( particles );
	mImpShapeMetaBall.setParticles( particles );
	mImpShapeRadius.setParticles( particles );
	mImpShapeSoftObject.setParticles( particles );
	mImpShapeWyvill.setParticles( particles );

	AABB<T> bounds;
	switch( mFieldFunc ) {
		// Blinn
		case CINDER_ISO_FIELDFUNC_BLINN: {		
			mImpShapeBlinn.updateBounds();
			bounds = mImpShapeBlinn.getBounds();
		}
		break;
		
		// Metaball
		case CINDER_ISO_FIELDFUNC_METABALL: {		
			mImpShapeMetaBall.updateBounds();
			bounds = mImpShapeMetaBall.getBounds();
		}
		break;
		
		// 1/RadiusSquared
		case CINDER_ISO_FIELDFUNC_RADIUS: {	
			mImpShapeRadius.updateBounds();
			bounds = mImpShapeRadius.getBounds();
		}
		break;
		
		// Soft Object
		case CINDER_ISO_FIELDFUNC_SOFTOBJECT: {
			mImpShapeSoftObject.updateBounds();
			bounds = mImpShapeSoftObject.getBounds();
		}
		break;
		
		// Wyvill
		case CINDER_ISO_FIELDFUNC_WYVILL: {
			mImpShapeWyvill.updateBounds();
			bounds = mImpShapeWyvill.getBounds();
		}
		break;		
	}
	
	bounds = bounds.getCube();
	bounds.scale( static_cast<T>( 1.25 ) );	
	mBounds = bounds;
	
	//// Set the initial bounds
	//mGrid.setBounds( mBounds );	
}
// -------------------------------------------------------------------------
HAI_PIMPL_TFUNC( T, CiIsoPolygonizer, void )::updateBounds() {
	AABB<T> bounds;
	switch( mFieldFunc ) {
		// Blinn
		case CINDER_ISO_FIELDFUNC_BLINN: {		
			mImpShapeBlinn.updateBounds();
			bounds = mImpShapeBlinn.getBounds();
		}
		break;
		
		// Metaball
		case CINDER_ISO_FIELDFUNC_METABALL: {		
			mImpShapeMetaBall.updateBounds();
			bounds = mImpShapeMetaBall.getBounds();
		}
		break;
		
		// 1/RadiusSquared
		case CINDER_ISO_FIELDFUNC_RADIUS: {	
			mImpShapeRadius.updateBounds();
			bounds = mImpShapeRadius.getBounds();
		}
		break;
		
		// Soft Object
		case CINDER_ISO_FIELDFUNC_SOFTOBJECT: {
			mImpShapeSoftObject.updateBounds();
			bounds = mImpShapeSoftObject.getBounds();
		}
		break;
		
		// Wyvill
		case CINDER_ISO_FIELDFUNC_WYVILL: {
			mImpShapeWyvill.updateBounds();
			bounds = mImpShapeWyvill.getBounds();
		}
		break;
	}
	
	bounds = bounds.getCube();
	bounds.scale( static_cast<T>( 1.25 ) );	
	mBounds = bounds;
	
	//// Update the bounds and vertex point values
	//mGrid.updateBounds( mBounds );	
}
// -------------------------------------------------------------------------
HAI_PIMPL_TFUNC( T, CiIsoPolygonizer, void )::initForPolygonize() 
{
	if( getPrintVars() ) {
		std::cout << "\n";
		std::cout << "Num Threads        : " << getNumThreads() << "\n";
		std::cout << "Force Multi-Thread : " << forceMultiThread() << "\n";
		std::cout << "Grid Size          : " << mGridSize << "\n";
		std::cout << std::endl;
	}
	
	// Allocate the grid
	getGrid()->setSize( getGridSize() );	
	// Initialize the bounds for the grid - we only do this once.
	getGrid()->initializeBounds( mBounds );
	
	// Initialize the field evaluator
	getFieldEval()->setImpShapeVoidPtr( getImpShapeAsVoidPtr( getFieldFunc() ) );
	getFieldEval()->setOutputGrid( getGrid() );
	
	// Only set the FieldEvalutor's threads if we're multithreading.
	if( getNumThreads() > 1 || forceMultiThread() ) {
		getFieldEval()->setNumThreads( getNumThreads() );
	}
	
	mInitialized = true;
}
// -------------------------------------------------------------------------
HAI_PIMPL_TFUNC( T, CiIsoPolygonizer, void )::polygonizeSingleThread( ci::TriMesh* mesh ) 
{
	if( NULL == mesh )
		return;

	clearPolyonizeData();
	
	T A = mParamA;
	T B = mParamB;
	T maxDist = mMaxDist;

	setImpShapeParams( getFieldFunc(), A, B );
	setImpShapeMaxDist( getFieldFunc(), maxDist );

	// Evaluate positions
	getFieldEval()->evaluatePositionSingleThread( getFieldFunc() );
	
	// Build the mesh
	MesherT mesher; 
	mesher.polygonize( *getGrid(), mesh );

	// Evaluate normals
	const std::vector<ci::Vec3f>& meshVertices = mesh->getVertices();
	std::vector<ci::Vec3f> outputNormals( meshVertices.size() );
	// Calculate the normals
	getFieldEval()->calculateNormalsSingleThread( getFieldFunc(), &meshVertices, &outputNormals );
	// Put the normals into the mesh
	for( std::vector<ci::Vec3f>::iterator it = outputNormals.begin(); it != outputNormals.end(); ++it ) {
		mesh->appendNormal( *it );
	}	
}
// -------------------------------------------------------------------------
HAI_PIMPL_TFUNC( T, CiIsoPolygonizer, void )::polygonizeMultiThread( ci::TriMesh* mesh ) 
{
	if( NULL == mesh )
		return;

	clearPolyonizeData();
	
	T A = mParamA;
	T B = mParamB;
	T maxDist = mMaxDist;

	setImpShapeParams( getFieldFunc(), A, B );
	setImpShapeMaxDist( getFieldFunc(), maxDist );

	// Evaluate positions
	getFieldEval()->evaluatePositionMultiThread( getFieldFunc() );
	
	// Build the mesh
	MesherT mesher; 
	mesher.polygonize( *getGrid(), mesh );

	// Evaluate normals
	const std::vector<ci::Vec3f>& meshVertices = mesh->getVertices();
	std::vector<ci::Vec3f> outputNormals( meshVertices.size() );
	// Calculate the normals
	getFieldEval()->calculateNormalsMultiThread( getFieldFunc(), &meshVertices, &outputNormals );
	// Put the normals into the mesh
	for( std::vector<ci::Vec3f>::iterator it = outputNormals.begin(); it != outputNormals.end(); ++it ) {
		mesh->appendNormal( *it );
	}
}
// -------------------------------------------------------------------------
HAI_PIMPL_TFUNC( T, CiIsoPolygonizer, void )::clearPolyonizeData() 
{
	typename GridT::vertex_data_iter it_vertexData = getGrid()->vertexDataBegin();
	for( ; it_vertexData != getGrid()->vertexDataEnd(); ++it_vertexData ) {
		it_vertexData->clear();
	}
	
	typename GridT::cell_data_iter it_cellData = getGrid()->cellDataBegin();
	for( ; it_cellData != getGrid()->cellDataEnd(); ++it_cellData ) {
		it_cellData->clear();
	}	
}
// -------------------------------------------------------------------------
HAI_PIMPL_TFUNC( T, CiIsoPolygonizer, void )::setPrintVars( bool val )
{
	mPrintVars = val;
}
// -------------------------------------------------------------------------
HAI_PIMPL_TFUNC( T, CiIsoPolygonizer, bool )::getPrintVars() const
{
	return mPrintVars;
}
// -------------------------------------------------------------------------
/*
HAI_PIMPL_TFUNC( T, CiIsoPolygonizer, void )::allocateGrid( int gridSize )
{
	getGrid()->setSize( gridSize );
}
*/
// -------------------------------------------------------------------------
HAI_PIMPL_TFUNC( T, CiIsoPolygonizer, void )::peakMesh( float amount, ci::TriMesh *mesh )
{
}
// -------------------------------------------------------------------------
template struct CiIsoPolygonizer<float>::Impl;
template struct CiIsoPolygonizer<double>::Impl;
// -------------------------------------------------------------------------
