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
/*! \file testutils.hpp
 *  \brief testutils.hpp contains a "stopwatch" class for performance meassurement of program pieces.   
 *  \author Simon Gog
 */
#ifndef INCLUDE_SDSL_TESTUTILS
#define INCLUDE_SDSL_TESTUTILS

#include <sys/time.h> // for struct timeval
#include <sys/resource.h> // for struct rusageA
#include <string>

namespace sdsl{

//! A helper class to meassure the time consumption of program pieces.
/*! stop_watch is a stopwatch based on the commands getrusage and
 *  gettimeofday. Where getrusage is used to determine the user and system time
 *  and gettimeofday to determine the elapsed real time. 
 */
class stop_watch{
	private:
		rusage m_ruse1, m_ruse2;
		timeval m_timeOfDay1, m_timeOfDay2;
	public:
		//! Start the stopwatch.
		/*! \sa stop
		 */
		void start();

		//! Stop the stopwatch.
		/*! \sa start
		 */
		void stop();

		//! Get the elapsed user time in milliseconds between start and stop.
		/*! \sa start, stop, getRealTime, getSysTime
		 */
		double getUserTime();

		//! Get the elapsed system time in milliseconds between start and stop.
		/*! \sa start, stop, getRealTime, getUserTime
		 */
		double getSysTime();

		//! Get the elapsed real time in milliseconds between start and stop.
		/*! \sa start, stop, getSysTime, getUserTime
		 */
		double getRealTime();
};

//! A helper class to get time information.
class clock{
	public:
		static std::string getTimeString();
};

//! A helper class to handle files.
class file{
	public:
		//! Read the file with the given file_name
		/*! \param file_name The file name of the text to read.
		 *  \param c A char pointer which will point to the text that was read.
		 *           New memory is allocated for the text. So free c if readFile
	     *           was successful and c is not needed anymore. If 
	     *	\return len The number of readed bits. If this is zero, now memory is 
		 *          allocated for c. And c equals NULL.
		 *  \pre c has to be initalized to NULL.
		 *  \post If len > 0  c[len]=0 and the memory for c was allocated with "new" else c=NULL.
		 */
		//static uint64_t readText(const char *file_name, char* &c);
};

} // end of namespace sdsl

#endif
