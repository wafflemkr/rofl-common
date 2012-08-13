/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/*
 * cclock.cc
 *
 *  Created on: 07.07.2012
 *      Author: andreas
 */

#include "cclock.h"

cclock::cclock(
		time_t delta_sec,
		time_t delta_nsec)
{
	ts.tv_sec 	= 0;
	ts.tv_nsec 	= 0;
	now();
	ts.tv_sec += delta_sec;
	ts.tv_nsec += delta_nsec;
}

cclock::cclock(const cclock& cc)
{
	*this = cc;
}


cclock::~cclock()
{

}


cclock&
cclock::operator =(const cclock& cc)
{
	if (this == &cc)
		return *this;

	ts.tv_sec 	= cc.ts.tv_sec;
	ts.tv_nsec	= cc.ts.tv_nsec;

	return *this;
}


cclock
cclock::operator+ (cclock const& cc)
{
	cclock clock;

	clock.ts.tv_sec		= ts.tv_sec  + cc.ts.tv_sec;
	clock.ts.tv_nsec 	= ts.tv_nsec + cc.ts.tv_nsec;

	if (clock.ts.tv_nsec > 1e9)
	{
		clock.ts.tv_sec += 1;
		clock.ts.tv_nsec -= 1e9;
	}

	return clock;
}


cclock
cclock::operator- (cclock const& cc)
{
	cclock clock;

	if (cc.ts.tv_nsec > ts.tv_nsec)
	{
		clock.ts.tv_nsec 	= ts.tv_nsec - cc.ts.tv_nsec + 1e9;
		clock.ts.tv_sec		= ts.tv_sec  - cc.ts.tv_sec  - 1;
	}
	else
	{
		clock.ts.tv_nsec	= ts.tv_nsec - cc.ts.tv_nsec;
		clock.ts.tv_sec		= ts.tv_sec  - cc.ts.tv_sec;
	}

	return clock;
}


cclock&
cclock::operator+= (cclock const& cc)
{
	ts.tv_sec	+= cc.ts.tv_sec;
	ts.tv_nsec 	+= cc.ts.tv_nsec;

	if (ts.tv_nsec > 1e9)
	{
		ts.tv_sec += 1;
		ts.tv_nsec -= 1e9;
	}

	return *this;
}


cclock&
cclock::operator-= (cclock const& cc)
{
	if (cc.ts.tv_nsec > ts.tv_nsec)
	{
		ts.tv_nsec 	= ts.tv_nsec - cc.ts.tv_nsec + 1e9;
		ts.tv_sec	= ts.tv_sec  - cc.ts.tv_sec  - 1;
	}
	else
	{
		ts.tv_nsec	= ts.tv_nsec - cc.ts.tv_nsec;
		ts.tv_sec	= ts.tv_sec  - cc.ts.tv_sec;
	}

	return *this;
}


bool
cclock::operator< (cclock const& cc)
{
	if (ts.tv_sec < cc.ts.tv_sec)
	{
		return true;
	}
	else if (ts.tv_sec > cc.ts.tv_sec)
	{
		return false;
	}

	// here: ts.tv_sec == cc.ts.tv_sec

	if (ts.tv_nsec < cc.ts.tv_nsec)
	{
		return true;
	}
	else if (ts.tv_nsec > cc.ts.tv_nsec)
	{
		return false;
	}

	// here: ts.tv_nsec == cc.ts.tv_nsec

	return false;
}


void
cclock::now()
{
	int rc = 0;

	if ((rc = clock_gettime(CLOCK_REALTIME, &ts)) < 0)
	{
		WRITELOG(CCLOCK, DBG, "cclock(%p) failed to read time: errno:%d (%s)",
				this, errno, strerror(errno));
	}
}


const char*
cclock::c_str()
{
	cvastring vas;

	cclock now;

#if 0
	info.assign(vas("cclock(%p)   creation:%ld:%ld   since:%ld:%ld",
			this,
			ts.tv_sec, ts.tv_nsec,
			now.ts.tv_sec - ts.tv_sec, now.ts.tv_nsec - ts.tv_nsec));
#endif
	info.assign(vas("since:%ld:%ld",
			now.ts.tv_sec - ts.tv_sec, now.ts.tv_nsec - ts.tv_nsec));

	return info.c_str();
}
