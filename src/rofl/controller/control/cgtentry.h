/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CGTENTRY_H
#define CGTENTRY_H 1

#include <set>
#include <map>
#include <list>
#include <vector>

#ifdef __cplusplus
extern "C" {
#endif
#include "../../common/openflow/openflow12.h"
#include <inttypes.h>
#include <sys/time.h>
#ifdef __cplusplus
}
#endif

#include "../../common/ciosrv.h"
#include "../../common/cmemory.h"
#include "../../common/cvastring.h"

#include "../../common/openflow/cofbclist.h"
#include "../../common/openflow/cofbucket.h"
#include "../../common/openflow/openflow_rofl_exceptions.h"
#include "../../common/openflow/messages/cofmsg_group_mod.h"
#include "../../common/openflow/messages/cofmsg_group_stats.h"

//This include must go here due to some Exceptions defined within cgttable.h FIXME: define in another file?
#include "cgttable.h"


namespace rofl
{

// forward declarations
class cfwdelem;
class cofctl;
class cgttable;
class cgtentry_owner;

// error classes
class eGteBase : public cerror {};


class cgtentry : public ciosrv {
public:
	/** constructor
	 */
	cgtentry(cgtentry_owner *owner);

	/** constructor
	 */
	cgtentry(
		cgtentry_owner *owner,
		cgttable *grp_table,
		cofmsg_group_mod *group_mod)
			throw (eGroupModBadBucket,
					eGroupModBadType,
					eBadActionBadOutPort);

	/** destructor
  	 */
	virtual
	~cgtentry();

	/**
	 * copy constructor
	 */
	cgtentry(const cgtentry& ge)
	{
		*this = ge;
	};

	/**
	 * assignment operator
	 */
	cgtentry&
	operator=(const cgtentry& ge)
	{
		if (this == &ge)
			return *this;

		ciosrv::operator= (ge);

		grp_table = ge.grp_table;
		group_id = ge.group_id;
		group_type = ge.group_type;
		packet_count = ge.packet_count;
		byte_count = ge.byte_count;
		group_mod_area = ge.group_mod_area;
		group_mod = (struct ofp_group_mod*)group_mod_area.somem();
		buckets = ge.buckets;
#if 0
		std::vector<cofbucket*>::const_iterator it;
		for (it = ge.buckets.begin(); it != ge.buckets.end(); ++it)
		{
			buckets.push_back(new cofbucket(*(*it)));
		}
#endif

		return *this;
	};


	/**
	 *
	 */
	void
	get_group_stats(
			cmemory& body);


	/**
	 *
	 */
	void
	get_group_desc_stats(
			cmemory& body);


	/**
	 *
	 */
	const char*
	c_str();


public: // data structures

	cgtentry_owner*		owner; 			// owner of this entry
	cgttable*			grp_table; 		// group_table this entry belongs to
	uint32_t 			group_id; 		// group identifier
	uint32_t		 	group_type; 	// group type (enum ofp_group_type)
	uint32_t		 	ref_count; 		// reference count
	uint64_t		 	packet_count; 	// packet counter for group
	uint64_t		 	byte_count; 	// byte counter for group

	cofbclist 			buckets;

protected: // data structures

	cmemory group_mod_area; // group mod
	struct ofp_group_mod *group_mod; // pointer to group_mod_area.somem()

private: // data structures

	std::string info; // info string

};

/** callback class for sending notifications to cgtentry owning entities
 */
class cgtentry_owner {
public:
	virtual ~cgtentry_owner() {};

	virtual void gtentry_timeout(cgtentry *entry, uint16_t timeout) = 0;
};

}; // end of namespace

#endif