#pragma once

#include "CiIso.h"
#include "Mesher.h"
#include "MarchCubesTables.h"
//#include "WorkQueue.h"
#include "FieldEvaluator.h"
#include "PolyMesh.h"
#include "SpatialGrid.h"
#include "Util.h"

#include "cinder/Color.h"
#include "cinder/Vector.h"

template <typename T, typename SpatialGridT>
struct Mesher<T, SpatialGridT, CINDER_ISO_MESHER_MARCH_CUBES>
{
	// Types
	typedef T												value_type;
	typedef glm::tvec4<T, glm::highp>						point_type;
	typedef glm::tvec4<T, glm::highp>						vec_type;
	typedef ci::Color										color_type;
	typedef SpatialGridT									spatial_grid_type;
	typedef typename spatial_grid_type::vertex_data_type	fieldvalue_type;
	typedef typename spatial_grid_type::cell_data_type		cell_data_type;

	// Functions
	void GetOffset( T fValue1, T fValue2, T fValueDesired, const point_type &a, const point_type &b, point_type &c ) {
		T mu = static_cast<T>( 0.5 );
		const T epsilon = static_cast<T>( 0.0001 );
		if( fabs( fValueDesired - fValue1 ) < epsilon ) {
			c = a;
		}
		else if( fabs( fValueDesired - fValue2 ) < epsilon ) {
			c = b;
		}
		else if( fabs( fValue1 - fValue2 ) < epsilon ) {
			c = PointMidPoint<T>( a, b );
		}
		else {
			T delta = fValue2 - fValue1;
			if( delta != 0 ) {
				mu = (fValueDesired - fValue1)/delta;
			}
			c = PointLerp<T>( a, b, mu );
		}
	}
	
	void GetOffset( T fValue1, T fValue2, T fValueDesired, const color_type &a, const color_type &b, color_type &c ) {
		T mu = static_cast<T>( 0.5 );
		const T epsilon = static_cast<T>( 0.0001 );
		if( fabs( fValueDesired - fValue1 ) < epsilon ) {
			c = a;
		}
		else if( fabs( fValueDesired - fValue2 ) < epsilon ) {
			c = b;
		}
		else if( fabs( fValue1 - fValue2 ) < epsilon ) {
			c = ColorMidPoint( a, b );
		}
		else {
			T delta = fValue2 - fValue1;
			if( delta != 0 ) {
				mu = (fValueDesired - fValue1)/delta;
			}
			c = ColorLerp( a, b, (float)mu );
		}
	}	

	
	void polygonize( spatial_grid_type &grid, ci::TriMesh *mesh ) {
	//template <typename BlendT>
	//void polygonize( spatial_grid_type &grid, implicit_shape_type &impShape, int numThreads, ci::TriMesh *mesh ) {
	
		//typedef ImplicitShape<T, BlendT> ImplicitShapeT;
		
		//int rangeSize = grid.getNumCellsX()*grid.getNumCellsY();
		//CellWorkQueue<spatial_grid_type> cellQueue( &grid, rangeSize - 10 );
		
		//FieldEvaluator<T, spatial_grid_type, ImplicitShapeT> fieldEval( &grid, &impShape, numThreads );
		//fieldEval.evaluate();		
		
		/*
		// Walk through each cube and calculate the field values if needed be
		T cellRadius = grid.getCellRadius();
		for( typename spatial_grid_type::const_cell_bounds_iter cell_iter = grid.cellBoundsBegin(); cell_iter != grid.cellBoundsEnd(); ++cell_iter ) {
			const typename spatial_grid_type::cell_type &cell = *cell_iter;
			int cellIndex = cell.cellIndex;
			if( impShape.hasInfluence( grid.getCellCenter( cellIndex ), cellRadius ) ) {
				for( size_t i  = 0; i < 8; ++i ) {
					int vertIndex = cell[i];
					fieldvalue_type &fvi = grid.getVertexDataAt( vertIndex );
					if( ! fvi.hasVal ) {
						const point_type &P = grid.getVertexPointAt( vertIndex );
						fvi.val    = impShape.evalFieldValue( P );
						fvi.color  = impShape.evalFieldColor( P );
						fvi.hasVal = true;
					}				
				}
			}
		}
		*/
		
		// March cubes
		for( typename spatial_grid_type::cell_bounds_iter cell_iter = grid.cellBoundsBegin(); cell_iter != grid.cellBoundsEnd(); ++cell_iter ) {
			typename spatial_grid_type::cell_type &cell = *cell_iter;
		 
			// Find which vertices are inside of the surface and which are outside   
			int32_t iFlagIndex = 0;
			for( size_t i = 0; i < 8; ++i ) {
				// Remap the vertex index so that it matches Paul Bourke's
				int32_t j = kMarchCubesRemapVertexFromBourke[i];
				int32_t vertIdx = cell[j];
				if( grid.getVertexDataAt( vertIdx ).val >= 1.0f ) {
					iFlagIndex |= 1 << i;
				}
			}
						
			// Find which edges are intersected by the surface
			int32_t iEdgeFlags = kMarchCubeEdgeFlags[iFlagIndex];
			if( 0 == iEdgeFlags ) {
				continue;
			}
			
			// Find the point of intersection of the surface with each edge
			// Then find the normal to the surface at those points
			int32_t asEdgeVertexIndices[12] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 }; 
			point_type asEdgeVertex[12];
			color_type asEdgeColor[12];
			for( int32_t iEdge = 0; iEdge < 12; ++iEdge ) {
				//if there is an intersection on this edge
				if( iEdgeFlags & (1 << iEdge ) ) {
					// Look around to see if there's a cell that already has a vertex on an edge we can use
					
					int32_t remappedEdge = kMarchCubeRemapEdgeFromBourke[iEdge];
					for( size_t edgeMapIdx = 0; edgeMapIdx < 3; ++edgeMapIdx ) {
						int32_t cellDx   = kMarchCubeEdgeMap[remappedEdge][edgeMapIdx][0];
						int32_t cellDy   = kMarchCubeEdgeMap[remappedEdge][edgeMapIdx][1];
						int32_t cellDz   = kMarchCubeEdgeMap[remappedEdge][edgeMapIdx][2];
						int32_t cubeEdge = kMarchCubeEdgeMap[remappedEdge][edgeMapIdx][3];
						
						int32_t cellX = cell.x + cellDx;
						int32_t cellY = cell.y + cellDy;
						int32_t cellZ = cell.z + cellDz;
						
						bool isCellXInBounds = (cellX >= 0 && cellX < (int32_t)grid.getNumCellsX());
						bool isCellYInBounds = (cellY >= 0 && cellY < (int32_t)grid.getNumCellsY());
						bool isCellZInBounds = (cellZ >= 0 && cellZ < (int32_t)grid.getNumCellsZ());
						
						if( isCellXInBounds && isCellYInBounds && isCellZInBounds ) {
							int32_t cellIndex = ( cellZ*grid.getNumCellsX()*grid.getNumCellsY() ) + ( cellY*grid.getNumCellsX() ) + cellX;
							typename spatial_grid_type::cell_data_type &cellData = grid.getCellDataAt( cellIndex );
							int32_t potVertexIndex = cellData.vertexOnEdge[cubeEdge];
							if( -1 != potVertexIndex ) {
								asEdgeVertexIndices[iEdge] = potVertexIndex;
								break;
							}
						}
					}
					
					if( -1 == asEdgeVertexIndices[iEdge] ) {
						int32_t	bourkeIdx0 = kMarchCubeEdgeConnection[iEdge][0];
						int32_t	bourkeIdx1 = kMarchCubeEdgeConnection[iEdge][1];				 
						int32_t cubeVertIdx0 = kMarchCubesRemapVertexFromBourke[bourkeIdx0];
						int32_t cubeVertIdx1 = kMarchCubesRemapVertexFromBourke[bourkeIdx1];
						uint32_t vertIdx0 = cell[cubeVertIdx0];
						uint32_t vertIdx1 = cell[cubeVertIdx1];
						T val0 = grid.getVertexDataAt( vertIdx0 ).val;
						T val1 = grid.getVertexDataAt( vertIdx1 ).val;				
						
						const point_type &P0 = grid.getVertexPointAt( vertIdx0 );
						const point_type &P1 = grid.getVertexPointAt( vertIdx1 );
						
						const color_type &color0 = grid.getVertexDataAt( vertIdx0 ).color;
						const color_type &color1 = grid.getVertexDataAt( vertIdx1 ).color;
						
						GetOffset( val0, val1, 1.0f, P0, P1, asEdgeVertex[iEdge] );
						GetOffset( val0, val1, 1.0f, color0, color1, asEdgeColor[iEdge] );
					}
				}
			}
			
			// Make the triangles that were found.  There can be up to five per cube
			for( int32_t iTriangle = 0; iTriangle < 5; ++iTriangle ) {
				if( kMarchCubeTriangleConnection[iFlagIndex][3*iTriangle] < 0 ) {
					break;
				}
				
				int32_t iVertex[3];
				iVertex[0] = kMarchCubeTriangleConnection[iFlagIndex][3*iTriangle+0];
				iVertex[1] = kMarchCubeTriangleConnection[iFlagIndex][3*iTriangle+1];
				iVertex[2] = kMarchCubeTriangleConnection[iFlagIndex][3*iTriangle+2];

				int32_t v[3];
				v[0] = asEdgeVertexIndices[iVertex[0]];
				v[1] = asEdgeVertexIndices[iVertex[1]];
				v[2] = asEdgeVertexIndices[iVertex[2]];
				
				for( size_t i = 0; i < 3; ++i ) {
					if( -1 == v[i] ) {
						const point_type &P = asEdgeVertex[iVertex[i]];
						//vec_type N = impShape.evalFieldGradient( P );
						color_type Ci = asEdgeColor[iVertex[i]];
						mesh->appendVertex( toVec3( P ) );
						//mesh->appendNormal( toVec3( N ) );
						mesh->appendColorRGB( Ci );
						v[i] = mesh->getNumVertices() - 1;
					}
				}
				
				mesh->appendTriangle( v[0], v[1], v[2] );
				
				asEdgeVertexIndices[iVertex[0]] = v[0];
				asEdgeVertexIndices[iVertex[1]] = v[1];
				asEdgeVertexIndices[iVertex[2]] = v[2];				
			}
			
			typename spatial_grid_type::cell_data_type &cellData = grid.getCellDataAt( cell.cellIndex );
			for( size_t edge = 0; edge < 12; ++edge ) {
				int32_t remappedEdge = kMarchCubeRemapEdgeFromBourke[edge];
				cellData.vertexOnEdge[remappedEdge] = asEdgeVertexIndices[edge];
			}
		}
	}
};

