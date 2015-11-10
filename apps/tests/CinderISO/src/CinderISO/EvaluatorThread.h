#pragma once

#include "ImplicitShape.h"
#include "Util.h"

#include "cinder/Thread.h"

enum EvalMode {
	EVAL_MODE_POSITION,
	EVAL_MODE_NORMAL
};

template <typename T, typename SpatialGridT>
class EvaluatorThread {
public:
	typedef void*											void_raw_ptr;
	typedef SpatialGridT									spatial_grid_type;
	typedef spatial_grid_type*								spatial_grid_raw_ptr;
	typedef typename spatial_grid_type::point_type			point_type;
	typedef typename spatial_grid_type::vec_type			vec_type;
	typedef typename spatial_grid_type::vertex_data_type	fieldvalue_type;	
	typedef WorkQueue										work_queue_type;
	typedef std::shared_ptr<work_queue_type>				work_queue_ptr;		
	typedef work_queue_type*								work_queue_raw_ptr;
	typedef std::thread									thread_type;
	typedef std::shared_ptr<thread_type>					thread_ptr;
	typedef thread_type*									thread_raw_ptr;	
	
	enum Mode {
	};

	EvaluatorThread( work_queue_ptr workQueue, void_raw_ptr impShapeVoidPtr, spatial_grid_raw_ptr outputGrid )
	: mWorkQueue( workQueue ), 
	  mImpShapeVoidPtr( impShapeVoidPtr ), 
	  mOutputGrid( outputGrid ),
	  mWorking( false ),
	  mTerminate( false )
	{}

	~EvaluatorThread() {
		mWorking = false;
		if( mThread && mThread->joinable() ) {
			mThread->join();
		}
	}
	
	void setMeshVertices( const std::vector<ci::Vec3f>* meshVertices ) {
		mMeshVertices = meshVertices;
	}
	
	void setOutputNormals( std::vector<ci::Vec3f>* outputNormals ) {
		mOutputNormals = outputNormals;
	}
	
	void setFieldFunc( CiIsoFieldFunc fieldFunc ) {
		mFieldFunc = fieldFunc;
	}
	
	EvalMode getMode() const { 
		return mMode;
	}
	
	bool isModePosition() const {
		return EVAL_MODE_POSITION == getMode();
	}
	
	bool isModeNormal() const {
		return EVAL_MODE_NORMAL == getMode();
	}
	
	void setMode( EvalMode mode ) {
		mMode = mode;
	}
	
	void setModePosition() {
		setMode( EVAL_MODE_POSITION );
	}

	void setModeNormal() {
		setMode( EVAL_MODE_NORMAL );
	}
	
	bool isWorking() const {
		return mWorking;
	}

	void stop() {
		mTerminate = true;
	}
		
	void start() {
		if( ! mThread ) {
			mThread = thread_ptr( new thread_type( boost::bind( &EvaluatorThread::run, this ) ) );
		}		
	}

	template <typename ImplicitShapeT>
	static void runEvaluationPosition( 
		ImplicitShapeT* impShape,
		const WorkRange& workRange,
		const bool* terminate,
		spatial_grid_raw_ptr outputGrid 
	) 
	{
		T cellRadius = outputGrid->getCellRadius();
		for( int cellIndex = workRange.getStart(); cellIndex < workRange.getEnd(); ++cellIndex ) {
			const typename spatial_grid_type::cell_type &cell = outputGrid->getCellAt( cellIndex );
			if( impShape->hasInfluence( outputGrid->getCellCenter( cellIndex ), cellRadius ) ) {
				for( size_t i  = 0; i < 8; ++i ) {
					int vertIndex = cell[i];
					fieldvalue_type &fvi = outputGrid->getVertexDataAt( vertIndex );
					if( ! fvi.hasVal ) {
						const point_type &P = outputGrid->getVertexPointAt( vertIndex );
						fvi.val    = impShape->evalFieldValue( P );
						fvi.color  = impShape->evalFieldColor( P );
						fvi.hasVal = true;
					}	
					
					// Pointer - remember to dereference
					if( *terminate )
						break;
				}
			}
			
			// Pointer - remember to dereference
			if( *terminate )
				break;				
		}	
	}
	
	template <typename ImplicitShapeT>
	static void runEvaluationNormal( 
		ImplicitShapeT* impShape, 
		const WorkRange& workRange, 
		const bool* terminate, 
		const std::vector<ci::Vec3f>* meshVertices, 
		std::vector<ci::Vec3f>* outputNormals 
	) 
	{
		for( int i = workRange.getStart(); i < workRange.getEnd(); ++i ) {
			point_type P = toPoint4( (*meshVertices)[i] );
			vec_type N = impShape->evalFieldGradient( P );
			(*outputNormals)[i] = toVec3( N );
			
			// Pointer - remember to dereference
			if( *terminate )
				break;
		}
	}
	
	static void processWorkRangePosition(
		CiIsoFieldFunc fieldFunc,
		void_raw_ptr impShapeVoidPtr,
		const WorkRange& workRange, 
		const bool* terminate, 
		spatial_grid_raw_ptr outputGrid
	) 
	{
		// Process items based on the field function
		switch( fieldFunc ) {
			// Blinn
			case CINDER_ISO_FIELDFUNC_BLINN: {
				typedef FieldFunc<T, CINDER_ISO_FIELDFUNC_BLINN> FieldFuncT;
				typedef ImplicitShape<T, FieldFuncT> ImplicitShapeT;
				runEvaluationPosition( 
					reinterpret_cast<ImplicitShapeT *>( impShapeVoidPtr ), 
					workRange, 
					terminate, 
					outputGrid
				);
			}
			break;
			
			// Metaball
			case CINDER_ISO_FIELDFUNC_METABALL: {
				typedef FieldFunc<T, CINDER_ISO_FIELDFUNC_METABALL> FieldFuncT;
				typedef ImplicitShape<T, FieldFuncT> ImplicitShapeT;	
				runEvaluationPosition( 
					reinterpret_cast<ImplicitShapeT *>( impShapeVoidPtr ), 
					workRange, 
					terminate, 
					outputGrid
				);			
			}
			break;
			
			// 1/RadiusSquared
			case CINDER_ISO_FIELDFUNC_RADIUS: {
				typedef FieldFunc<T, CINDER_ISO_FIELDFUNC_RADIUS> FieldFuncT;
				typedef ImplicitShape<T, FieldFuncT> ImplicitShapeT;	
				runEvaluationPosition( 
					reinterpret_cast<ImplicitShapeT *>( impShapeVoidPtr ), 
					workRange, 
					terminate, 
					outputGrid
				);
			}
			break;
			
			// Soft Object
			case CINDER_ISO_FIELDFUNC_SOFTOBJECT: {
				typedef FieldFunc<T, CINDER_ISO_FIELDFUNC_SOFTOBJECT> FieldFuncT;
				typedef ImplicitShape<T, FieldFuncT> ImplicitShapeT;	
				runEvaluationPosition( 
					reinterpret_cast<ImplicitShapeT *>( impShapeVoidPtr ), 
					workRange, 
					terminate, 
					outputGrid
				);
			}
			break;
			
			// Wyvill
			case CINDER_ISO_FIELDFUNC_WYVILL: {
				typedef FieldFunc<T, CINDER_ISO_FIELDFUNC_WYVILL> FieldFuncT;
				typedef ImplicitShape<T, FieldFuncT> ImplicitShapeT;	
				runEvaluationPosition( 
					reinterpret_cast<ImplicitShapeT *>( impShapeVoidPtr ), 
					workRange, 
					terminate, 
					outputGrid
				);
			}
			break;		
		}
	}
	
	static void processWorkRangeNormal(
		CiIsoFieldFunc fieldFunc,
		void_raw_ptr impShapeVoidPtr,
		const WorkRange& workRange, 
		const bool* terminate, 
		const std::vector<ci::Vec3f>* meshVertices, 
		std::vector<ci::Vec3f>* outputNormals 		
	) 
	{
		// Process items based on the field function
		switch( fieldFunc ) {
			// Blinn
			case CINDER_ISO_FIELDFUNC_BLINN: {
				typedef FieldFunc<T, CINDER_ISO_FIELDFUNC_BLINN> FieldFuncT;
				typedef ImplicitShape<T, FieldFuncT> ImplicitShapeT;
				runEvaluationNormal( 
					reinterpret_cast<ImplicitShapeT *>( impShapeVoidPtr ), 
					workRange, 
					terminate,
					meshVertices, 
					outputNormals
				);
			}
			break;
			
			// Metaball
			case CINDER_ISO_FIELDFUNC_METABALL: {
				typedef FieldFunc<T, CINDER_ISO_FIELDFUNC_METABALL> FieldFuncT;
				typedef ImplicitShape<T, FieldFuncT> ImplicitShapeT;	
				runEvaluationNormal( 
					reinterpret_cast<ImplicitShapeT *>( impShapeVoidPtr ), 
					workRange, 
					terminate,
					meshVertices, 
					outputNormals
				);
			}
			break;
			
			// 1/RadiusSquared
			case CINDER_ISO_FIELDFUNC_RADIUS: {
				typedef FieldFunc<T, CINDER_ISO_FIELDFUNC_RADIUS> FieldFuncT;
				typedef ImplicitShape<T, FieldFuncT> ImplicitShapeT;	
				runEvaluationNormal( 
					reinterpret_cast<ImplicitShapeT *>( impShapeVoidPtr ), 
					workRange, 
					terminate,
					meshVertices, 
					outputNormals
				);
			}
			break;
			
			// Soft Object
			case CINDER_ISO_FIELDFUNC_SOFTOBJECT: {
				typedef FieldFunc<T, CINDER_ISO_FIELDFUNC_SOFTOBJECT> FieldFuncT;
				typedef ImplicitShape<T, FieldFuncT> ImplicitShapeT;	
				runEvaluationNormal( 
					reinterpret_cast<ImplicitShapeT *>( impShapeVoidPtr ), 
					workRange, 
					terminate,
					meshVertices, 
					outputNormals
				);
			}
			break;
			
			// Wyvill
			case CINDER_ISO_FIELDFUNC_WYVILL: {
				typedef FieldFunc<T, CINDER_ISO_FIELDFUNC_WYVILL> FieldFuncT;
				typedef ImplicitShape<T, FieldFuncT> ImplicitShapeT;	
				runEvaluationNormal( 
					reinterpret_cast<ImplicitShapeT *>( impShapeVoidPtr ), 
					workRange, 
					terminate,
					meshVertices, 
					outputNormals
				);
			}
			break;		
		}
	}		
	
	
private:

	work_queue_ptr					mWorkQueue;
	void_raw_ptr					mImpShapeVoidPtr;
	spatial_grid_raw_ptr			mOutputGrid;
	
	typedef std::vector<ci::Vec3f>	vec3_list;
	const vec3_list*				mMeshVertices;
	vec3_list*						mOutputNormals;
	
	CiIsoFieldFunc					mFieldFunc;
	EvalMode						mMode;
	
	bool							mWorking;
	bool							mTerminate;
	thread_ptr						mThread;
	
	void run() {
		while( ! mTerminate ) {
			// Get the work and start
			bool workQueueCanceled = false;
			WorkRange workRange;					
			try {
				// If the work queue gets canceled here, it will
				// throw a cancel_op.
				mWorkQueue->wait_and_pop( workRange );
			}
			catch( const concurrent_queue<WorkRange>::cancel_op& ) {
				workQueueCanceled = true;
			}
			// If the workqueue cancels - we need to exit the the thread 
			// because the system is more than likely asking us to stop
			// so it can destroy the work queue.
			if( workQueueCanceled )
				break;
			
			// If we get here - we're "working"
			mWorking = true;
			// Process items based on the field function
			if( isModePosition() ) {
				processWorkRangePosition( 
					mFieldFunc,
					mImpShapeVoidPtr, 
					workRange, 
					addressof( mTerminate ),
					mOutputGrid
				);
			}
			else if( isModeNormal() ) {
				processWorkRangeNormal( 
					mFieldFunc,
					mImpShapeVoidPtr, 
					workRange, 
					addressof( mTerminate ), 
					mMeshVertices, 
					mOutputNormals
				);
			}

			/*
			switch( mFieldFunc ) {
				// Blinn
				case CINDER_ISO_FIELDFUNC_BLINN: {
					typedef FieldFunc<T, CINDER_ISO_FIELDFUNC_BLINN> FieldFuncT;
					typedef ImplicitShape<T, FieldFuncT> ImplicitShapeT;
					if( isModePosition() )
						runEvaluationPosition( (ImplicitShapeT*)mImpShapeVoidPtr, workRange, addressof( mTerminate ), mOutputGrid );
					else if( isModeNormal() )
						runEvaluationNormal( (ImplicitShapeT*)mImpShapeVoidPtr, workRange, addressof( mTerminate ), mMeshVertices, mOutputNormals );
				}
				break;
				
				// Metaball
				case CINDER_ISO_FIELDFUNC_METABALL: {
					typedef FieldFunc<T, CINDER_ISO_FIELDFUNC_METABALL> FieldFuncT;
					typedef ImplicitShape<T, FieldFuncT> ImplicitShapeT;	
					if( isModePosition() )
						runEvaluationPosition( (ImplicitShapeT*)mImpShapeVoidPtr, workRange, addressof( mTerminate ), mOutputGrid );
					else if( isModeNormal() )
						runEvaluationNormal( (ImplicitShapeT*)mImpShapeVoidPtr, workRange, addressof( mTerminate ), mMeshVertices, mOutputNormals );
				}
				break;
				
				// 1/RadiusSquared
				case CINDER_ISO_FIELDFUNC_RADIUS: {
					typedef FieldFunc<T, CINDER_ISO_FIELDFUNC_RADIUS> FieldFuncT;
					typedef ImplicitShape<T, FieldFuncT> ImplicitShapeT;	
					if( isModePosition() )
						runEvaluationPosition( (ImplicitShapeT*)mImpShapeVoidPtr, workRange, addressof( mTerminate ), mOutputGrid );
					else if( isModeNormal() )
						runEvaluationNormal( (ImplicitShapeT*)mImpShapeVoidPtr, workRange, addressof( mTerminate ), mMeshVertices, mOutputNormals );
				}
				break;
				
				// Soft Object
				case CINDER_ISO_FIELDFUNC_SOFTOBJECT: {
					typedef FieldFunc<T, CINDER_ISO_FIELDFUNC_SOFTOBJECT> FieldFuncT;
					typedef ImplicitShape<T, FieldFuncT> ImplicitShapeT;	
					if( isModePosition() )
						runEvaluationPosition( (ImplicitShapeT*)mImpShapeVoidPtr, workRange, addressof( mTerminate ), mOutputGrid );
					else if( isModeNormal() )
						runEvaluationNormal( (ImplicitShapeT*)mImpShapeVoidPtr, workRange, addressof( mTerminate ), mMeshVertices, mOutputNormals );
				}
				break;
				
				// Wyvill
				case CINDER_ISO_FIELDFUNC_WYVILL: {
					typedef FieldFunc<T, CINDER_ISO_FIELDFUNC_WYVILL> FieldFuncT;
					typedef ImplicitShape<T, FieldFuncT> ImplicitShapeT;	
					if( isModePosition() )
						runEvaluationPosition( (ImplicitShapeT*)mImpShapeVoidPtr, workRange, addressof( mTerminate ), mOutputGrid );
					else if( isModeNormal() )
						runEvaluationNormal( (ImplicitShapeT*)mImpShapeVoidPtr, workRange, addressof( mTerminate ), mMeshVertices, mOutputNormals );
				}
				break;		
			}
			*/
			
			// All the work should be done at this point
			mWorking = false;				
		}
		// If we get here - we've exited out of the thread's loop.
		mWorking = false;
	}
	
	/*
	//template <typename FieldFuncT>
	//void runEvaluationPosition( ImplicitShape<T, FieldFuncT> *impShape, const WorkRange& cellRange ) {
	template <typename ImplicitShapeT>
	void runEvaluationPosition( ImplicitShapeT* impShape, const WorkRange& cellRange ) {
		T cellRadius = mOutputGrid->getCellRadius();
		for( int cellIndex = cellRange.getStart(); cellIndex < cellRange.getEnd(); ++cellIndex ) {
			const typename spatial_grid_type::cell_type &cell = mOutputGrid->getCellAt( cellIndex );
			if( impShape->hasInfluence( mOutputGrid->getCellCenter( cellIndex ), cellRadius ) ) {
				for( size_t i  = 0; i < 8; ++i ) {
					int vertIndex = cell[i];
					fieldvalue_type &fvi = mOutputGrid->getVertexDataAt( vertIndex );
					if( ! fvi.hasVal ) {
						const point_type &P = mOutputGrid->getVertexPointAt( vertIndex );
						fvi.val    = impShape->evalFieldValue( P );
						fvi.color  = impShape->evalFieldColor( P );
						fvi.hasVal = true;
					}	
					
					if( mTerminate )
						break;
				}
			}
			if( mTerminate )
				break;				
		}	
	}
	
	//template <typename FieldFuncT>
	//void runEvaluationNormal( ImplicitShape<T, FieldFuncT> *impShape, const WorkRange& cellRange ) {
	template <typename ImplicitShapeT>
	void runEvaluationNormal( ImplicitShapeT* impShape, const WorkRange& cellRange ) {
		for( int i = cellRange.getStart(); i < cellRange.getEnd(); ++i ) {
			point_type P = toPoint4( (*mMeshVertices)[i] );
			vec_type N = impShape->evalFieldGradient( P );
			(*mOutputNormals)[i] = toVec3( N );
			if( mTerminate )
				break;
		}
	}
	*/	
	
};
