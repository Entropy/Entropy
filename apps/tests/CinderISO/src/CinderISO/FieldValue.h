#pragma once

template <typename T>
struct FieldValue {
	T			val;
	ci::Color	color;
	bool		hasVal;		
	
	FieldValue() : val( 0 ), color( 0, 0, 0 ), hasVal( false ) {}
	
	void clear() {
		val = 0;
		color = ci::Color( 0, 0, 0 );
		hasVal = false;
	}
};
