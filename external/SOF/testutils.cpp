/* sdsl - succinct data structures library
    Copyright (C) 2008 Simon Gog 

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see http://www.gnu.org/licenses/ .
*/
#include "testutils.hpp"
#include <cxxabi.h>
#include <fstream>
#include <iostream>

namespace sdsl{

void stop_watch::start(){
	gettimeofday(&m_timeOfDay1, 0);
	getrusage(RUSAGE_SELF, &m_ruse1);
}

void stop_watch::stop(){
	getrusage(RUSAGE_SELF, &m_ruse2);
	gettimeofday(&m_timeOfDay2, 0);
}

double stop_watch::getUserTime(){
	timeval t1, t2;
	t1 = m_ruse1.ru_utime;
	t2 = m_ruse2.ru_utime;
	return ((double)(t2.tv_sec*1000000 + t2.tv_usec - (t1.tv_sec*1000000 + t1.tv_usec )))/1000.0;
}

double stop_watch::getSysTime(){
	timeval t1, t2;
	t1 = m_ruse1.ru_stime;
	t2 = m_ruse2.ru_stime;
	return ((double)(t2.tv_sec*1000000 + t2.tv_usec - (t1.tv_sec*1000000 + t1.tv_usec )))/1000.0;
}

double stop_watch::getRealTime(){
	double result = ((double)((m_timeOfDay2.tv_sec*1000000 + m_timeOfDay2.tv_usec)-(m_timeOfDay1.tv_sec*1000000 + m_timeOfDay1.tv_usec)))/1000.0;
	if( result < getSysTime() + getUserTime() )
		return getSysTime()+getUserTime();
	return result;
}

std::string clock::getTimeString(){
	time_t rawtime;
	struct tm * timeinfo;
	char buffer[1024];	
	time( &rawtime );
	timeinfo = localtime( &rawtime );

	strftime(buffer, 1024, "%Y-%m-%d-%H%M%S", timeinfo);
	return buffer;
} 
/*
uint64_t file::readText(const char *file_name, char* &c){
	if( c != NULL ){
		delete [] c;
		c = NULL;
	}
	std::ifstream in;
	in.open(file_name);
	if( in ){
		const uint64_t BLOCK_SIZE = (1<<20);
		uint64_t n=0, read = 0;
		char buf[BLOCK_SIZE], *cp;
		do{
			in.read(buf, BLOCK_SIZE);
			read = in.gcount();
			n+=read;
		}while( BLOCK_SIZE == read );
		if(n==0)
			return 0;
		c = new char[n+2];
		c[n+1] = 0;
		in.close();
		in.open(file_name);
		if(!in){ 
			delete [] c; 
			c = NULL; 
			return 0;
		}
		cp=c;
		do{
			in.read(cp, BLOCK_SIZE);
			read = in.gcount();
			cp+= read;
		}while( BLOCK_SIZE == read );
		*(c+n) = '\0';
		return n;
	}
	else
		return 0;
}
*/


} // end of namespace sdsl
