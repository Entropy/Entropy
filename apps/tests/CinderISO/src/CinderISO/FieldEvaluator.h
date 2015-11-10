#pragma once

#include "EvaluatorThread.h"
#include "WorkQueue.h"

template <typename T, typename SpatialGridT>
class FieldEvaluator {
public:
	typedef void*											void_raw_ptr;
	typedef SpatialGridT									spatial_grid_type;
	typedef spatial_grid_type*								spatial_grid_raw_ptr;
	typedef typename spatial_grid_type::point_type			point_type;
	typedef typename spatial_grid_type::vec_type			vec_type;
	typedef typename spatial_grid_type::vertex_data_type	fieldvalue_type;		
	typedef EvaluatorThread<T, SpatialGridT>				evaluator_thread;
	typedef boost::shared_ptr<evaluator_thread>				evaluator_thread_ptr;
	typedef std::vector<evaluator_thread_ptr>				evaluator_thread_list;
	typedef WorkQueue										work_queue_type;
	typedef boost::shared_ptr<work_queue_type>				work_queue_ptr;		
	typedef work_queue_type*								work_queue_raw_ptr;
	
	FieldEvaluator()
	: mNumThreads( 1 ),
	  mImpShapeVoidPtr( NULL ),
	  mOutputGrid( NULL )
	{
		mWorkQueue = work_queue_ptr( new work_queue_type() );		
	}
	
	~FieldEvaluator() {
		try {
			// Will throw if cancel gets called more than once.
			mWorkQueue->cancel();	
		}
		catch( const concurrent_queue<WorkRange>::cancel_op& ) {
			// We don't really need to do anything
		}
			
		// Stop the threads
		stopAllThreads();
		
		/*
		// Cancel the work queue
		mWorkQueue->cancel();
		// Stop the threads
		std::for_each(
			mThreads.begin(),
			mThreads.end(),
			boost::bind( &evaluator_thread::stop, _1 )
		);
		*/			
	}
	
	// IMPORTANT: This needs to be called before setNumThreads.
	void setImpShapeVoidPtr( void_raw_ptr impShapeVoidPtr ) {
		mImpShapeVoidPtr = impShapeVoidPtr;
	}

	// IMPORTANT: This needs to be called before setNumThreads.
	void setOutputGrid( spatial_grid_raw_ptr outputGrid ) {
		mOutputGrid = outputGrid;
	}

	void setNumThreads( int numThreads )
	{	
		if( numThreads != mNumThreads ) {
			// Stop all the threads
			stopAllThreads();
			// Clear the threads
			mThreads.clear();	
			
			// Once all the threads are stopped and cleared, then
			// we can change it.
			mNumThreads = numThreads;
			for( int i = 0; i < mNumThreads; ++i ) {
				mThreads.push_back( evaluator_thread_ptr( new evaluator_thread( mWorkQueue, mImpShapeVoidPtr, mOutputGrid ) ) );
			}
		}
	}
	
	void stopAllThreads() {
		std::for_each(
			mThreads.begin(),
			mThreads.end(),
			boost::bind( &evaluator_thread::stop, _1 )
		);		
	}

	void evaluatePositionSingleThread( CiIsoFieldFunc fieldFunc ) {
		bool terminate = false;
		evaluator_thread::processWorkRangePosition(
			fieldFunc,
			mImpShapeVoidPtr,
			WorkRange( 0, mOutputGrid->getNumCells() ),
			addressof( terminate ),
			mOutputGrid
		);			
	}
	
	void evaluatePositionMultiThread( CiIsoFieldFunc fieldFunc ) {
		// Set the thread's field function
		std::for_each(
			mThreads.begin(),
			mThreads.end(),
			boost::bind( &evaluator_thread::setFieldFunc, _1, fieldFunc )
		);		

		// Set the mode
		std::for_each(
			mThreads.begin(),
			mThreads.end(),
			boost::bind( &evaluator_thread::setModePosition, _1 )
		);		
				
		// Initialize the work queue
		int numItems  = mOutputGrid->getNumCells();
		int rangeSize = mOutputGrid->getNumCellsX()*mOutputGrid->getNumCellsY();
		bool isOk = mWorkQueue->build( numItems, rangeSize );
		
		if( isOk ) {
			// Start the threads
			std::for_each(
				mThreads.begin(),
				mThreads.end(),
				boost::bind( &evaluator_thread::start, _1 )
			);

			// Wait til queue is empty
			//while( ! mWorkQueue->empty() ) {
			//	boost::this_thread::sleep( boost::posix_time::milliseconds( 1 ) ); 
			//}
			try {
				// Work queue throws a cancel_op if it gets canceled.
				while( ! mWorkQueue->empty() ) {
					boost::this_thread::sleep( boost::posix_time::milliseconds( 1 ) ); 
				}		
			}
			catch( const concurrent_queue<WorkRange>::cancel_op& ) {
				// We don't really need to do anything
			}
			
			// Just because the queue is empty doesn't mean we're done yet
			bool doneWorking = false;
			while( ! doneWorking ) {
				doneWorking = true;
				for( typename evaluator_thread_list::iterator it = mThreads.begin(); it != mThreads.end(); ++it ) {
					evaluator_thread_ptr evalThread = *it;
					if( evalThread->isWorking() ) {
						doneWorking = false;
						break;
					}
				}
				// Sleep 
				if( ! doneWorking ) {
					boost::this_thread::sleep( boost::posix_time::milliseconds( 1 ) ); 
				}
			}
		}			
	}

	void calculateNormalsSingleThread( CiIsoFieldFunc fieldFunc, const std::vector<ci::Vec3f>* meshVertices, std::vector<ci::Vec3f>* outputNormals ) {
		bool terminate = false;
		evaluator_thread::processWorkRangeNormal(
			fieldFunc,
			mImpShapeVoidPtr,
			WorkRange( 0, meshVertices->size() ),
			addressof( terminate ),
			meshVertices,
			outputNormals
		);			
	}
	
	void calculateNormalsMultiThread( CiIsoFieldFunc fieldFunc, const std::vector<ci::Vec3f>* meshVertices, std::vector<ci::Vec3f>* outputNormals ) {
		// Set mesh vertices and output normals
		std::for_each(
			mThreads.begin(),
			mThreads.end(),
			boost::bind( &evaluator_thread::setMeshVertices, _1, meshVertices )
		);
		
		std::for_each(
			mThreads.begin(),
			mThreads.end(),
			boost::bind( &evaluator_thread::setOutputNormals, _1, outputNormals )
		);
	
		// Set the thread's field function
		std::for_each(
			mThreads.begin(),
			mThreads.end(),
			boost::bind( &evaluator_thread::setFieldFunc, _1, fieldFunc )
		);	

		// Set the mode
		std::for_each(
			mThreads.begin(),
			mThreads.end(),
			boost::bind( &evaluator_thread::setModeNormal, _1 )
		);	
				
		// Initialize the work queue
		int numItems  = meshVertices->size();
		int rangeSize = numItems/mNumThreads;
		bool isOk = mWorkQueue->build( numItems, rangeSize );
		
		if( isOk ) {
			// Start the threads
			std::for_each(
				mThreads.begin(),
				mThreads.end(),
				boost::bind( &evaluator_thread::start, _1 )
			);

			// Wait til queue is empty
			//while( ! mWorkQueue->empty() ) {
			//	boost::this_thread::sleep( boost::posix_time::milliseconds( 1 ) ); 
			//}
			try {
				// Work queue throws a cancel_op if it gets canceled.
				while( ! mWorkQueue->empty() ) {
					boost::this_thread::sleep( boost::posix_time::milliseconds( 1 ) ); 
				}		
			}
			catch( const concurrent_queue<WorkRange>::cancel_op& ) {
				// We don't really need to do anything
			}		
			
			// Just because the queue is empty doesn't mean we're done yet
			bool doneWorking = false;
			while( ! doneWorking ) {
				doneWorking = true;
				for( typename evaluator_thread_list::iterator it = mThreads.begin(); it != mThreads.end(); ++it ) {
					evaluator_thread_ptr evalThread = *it;
					if( evalThread->isWorking() ) {
						doneWorking = false;
						break;
					}
				}
				// Sleep 
				if( ! doneWorking ) {
					boost::this_thread::sleep( boost::posix_time::milliseconds( 1 ) ); 
				}
			}
		}
	}
	
private:
	int							mNumThreads;
	work_queue_ptr				mWorkQueue;
	void_raw_ptr				mImpShapeVoidPtr;	
	spatial_grid_raw_ptr		mOutputGrid;
	
	evaluator_thread_list		mThreads;
};
