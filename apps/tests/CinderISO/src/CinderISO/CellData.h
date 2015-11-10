#pragma once

struct CellData {
	bool	hasVal;
	int32_t	vertexOnEdge[12];
	
	CellData() {
		clear();
	}
	
	void clear() {
		hasVal = false;
		for( size_t i = 0; i < 12; ++i ) {
			vertexOnEdge[i] = -1;
		}
	}
};	
