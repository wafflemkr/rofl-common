/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */


#include "cofportlist.h"

cofportlist::cofportlist()
{
	WRITELOG(COFINST, DBG, "cofportlist(%p)::cofportlist()", this);
}


cofportlist::~cofportlist()
{
	WRITELOG(COFINST, DBG, "cofportlist(%p)::~cofportlist()", this);
}


std::vector<cofport>&
cofportlist::unpack(
		struct ofp_port *ports,
		size_t portlen)
throw (ePortListInval)
{
	reset(); // clears bcvec

	// sanity check: bclen must be of size at least of ofp_bucket
	if (portlen < (int)sizeof(struct ofp_port))
		return elems;

	// first instruction
	struct ofp_port *porthdr = ports;


	while (portlen > 0)
	{
		if (portlen < sizeof(struct ofp_port))
			throw ePortListInval();

		next() = cofport(porthdr, sizeof(struct ofp_port) );

		portlen -= sizeof(struct ofp_port);
		porthdr = (struct ofp_port*)(((uint8_t*)porthdr) + sizeof(struct ofp_port));
	}

	return elems;
}


struct ofp_port*
cofportlist::pack(
	struct ofp_port *ports,
	size_t portlen) throw (ePortListInval)
{
	size_t needed_inlen = length();

	if (portlen < needed_inlen)
		throw ePortListInval();

	struct ofp_port *porthdr = ports; // first ofp_port header

	cofportlist::iterator it;
	for (it = elems.begin(); it != elems.end(); ++it)
	{
		cofport& port = (*it);

		porthdr = (struct ofp_port*)
				((uint8_t*)(port.pack(porthdr, port.length())) + port.length());
	}

	return ports;
}


size_t
cofportlist::length()
{
	size_t inlen = 0;
	cofportlist::iterator it;
	for (it = elems.begin(); it != elems.end(); ++it)
	{
		inlen += (*it).length();
	}
	return inlen;
}


const char*
cofportlist::c_str()
{
	cvastring vas(1024);
	info.assign(vas("cofportlist(%p) %d port(s): ", this, elems.size()));
	cofportlist::iterator it;
	for (it = elems.begin(); it != elems.end(); ++it)
	{
		info.append(vas("\n  %s ", (*it).c_str()));
	}

	return info.c_str();
}


cofport&
cofportlist::find_port(
		uint32_t port_no)
throw (ePortListNotFound)
{
	cofportlist::iterator it;

	for (it = elems.begin(); it != elems.end(); ++it)
	{
		WRITELOG(COFINST, DBG, "cofportlist(%p)::find_port() %d => %s", this, elems.size(), (*it).c_str());
	}

	if ((it = find_if(elems.begin(), elems.end(),
			cofport_find_port_no(port_no))) == elems.end())
	{
		throw ePortListNotFound();
	}
	return (*it);
}



void
cofportlist::test()
{
	cofport p1;

	p1.port_no = 17;
	p1.hwaddr = cmacaddr("00:11:11:11:11:11");
	p1.name.assign("geb0");
	p1.advertised = 0xaaaa;
	p1.curr = 0xbbbb;
	p1.config = 0xcccc;

	cofportlist pl1;

	pl1[0] = cofport();
	pl1[1] = cofport();
	pl1[2] = cofport();
	pl1[3] = p1;

	pl1[0].port_no = 8;
	pl1[0].name.assign("ge1");
	pl1[0].advertised = 77777;
	pl1[0].hwaddr = cmacaddr("00:22:22:22:22:22");

	fprintf(stderr, "portlist => %s\n", pl1.c_str());

	cmemory mem(pl1.length());

	pl1.pack((struct ofp_port*)mem.somem(), mem.memlen());

	fprintf(stderr, "portlist.packed => %s\n", mem.c_str());

	cofportlist pl2;

	pl2.unpack((struct ofp_port*)mem.somem(), mem.memlen());

	fprintf(stderr, "portlist.unpacked => %s\n", pl2.c_str());

#if 0
	inlist[0] = cofinst_write_actions();
	inlist[0].actions[0] = cofaction_set_mpls_label(111111);

	fprintf(stderr, "XXX => %s\n", inlist.c_str());

	fprintf(stderr, "--------------------------\n");

	cofportlist inlist2;

	inlist2[0] = cofinst_apply_actions();
	inlist2[0].actions[0] = cofaction_output(1);
	inlist2[1] = cofinst_clear_actions();
	inlist2[2] = cofinst_write_actions();
	inlist2[2].actions[0] = cofaction_set_vlan_vid(1111);
	inlist2[2].actions[1] = cofaction_set_mpls_tc(7);

	fprintf(stderr, "YYY => %s\n", inlist2.c_str());

	fprintf(stderr, "--------------------------\n");

	inlist2 = inlist;

	fprintf(stderr, "ZZZ => %s\n", inlist2.c_str());

	fprintf(stderr, "--------------------------\n");
#endif
}
