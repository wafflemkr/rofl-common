/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "cofmatch.h"

cofmatch::cofmatch(
		uint16_t type)
{
	//WRITELOG(COFMATCH, DBG, "cofmatch(%p)::cofmatch() [1]", this);

	bzero(&match, sizeof(match));

	match.type = htobe16(type);
	match.length = htobe16(length());

	reset();

	validate();
}

cofmatch::cofmatch(
	struct ofp_match *__match, size_t __matchlen)
{
	//WRITELOG(COFMATCH, DBG, "cofmatch(%p)::cofmatch() [2]", this);

	validate();
}

cofmatch::~cofmatch()
{
	//WRITELOG(COFMATCH, DBG, "cofmatch(%p)::~cofmatch()", this);
}


cofmatch&
cofmatch::operator= (const cofmatch& m)
{
	if (this == &m)
		return *this;

#if 0
	WRITELOG(COFMATCH, DBG, "cofmatch(%p)::operator=() m:%p", this, &m);

	WRITELOG(COFMATCH, DBG, "cofmatch(%p)::operator=() [1] *this: %s", this, this->c_str());
#endif

	match.type 		= m.match.type;
	match.length 	= m.match.length;
	oxmlist			= m.oxmlist;

#if 0
	WRITELOG(COFMATCH, DBG, "cofmatch(%p)::operator=() [2] *this: %s", this, this->c_str());
#endif

	validate();

	return *this;
}


void
cofmatch::reset()
{
	//WRITELOG(COFMATCH, DBG, "cofmatch(%p)::reset()", this);

	oxmlist.clear();
	match.length = htobe16(length());

}


void
cofmatch::validate() throw (eOFmatchInval)
{
	// TODO: apply OF1.2 prerequisites here
}


size_t
cofmatch::length_internal()
{
	/*
	 * returns length of struct ofp_match including padding
	 */
	size_t match_len = 2 * sizeof(uint16_t); // first two 16bit fields in struct ofp_match

	match_len += oxmlist.length();

	if (0 != (match_len % sizeof(uint64_t)))
	{
		match_len = ((match_len / sizeof(uint64_t)) + 1) * sizeof(uint64_t);
	}
	return match_len;
}

size_t
cofmatch::length()
{
	size_t total_length = sizeof(match.type) + sizeof(match.length) + oxmlist.length();

	size_t pad = (0x7 & total_length);
	/* append padding if not a multiple of 8 */
	if (pad) {
		total_length += 8 - pad;
	}
	return total_length;
}


struct ofp_match*
cofmatch::pack(struct ofp_match* m, size_t mlen) throw (eOFmatchInval)
{
	if (mlen < length())
	{
		throw eOFmatchInval();
	}


	//match.length = htobe16(sizeof(match.type) + sizeof(match.length) + oxmlist.length());

	//match.length = htobe16(length());
	/*
	 * fill in real length, i.e. excluding padding
	 */
	match.length = htobe16(2 * sizeof(uint16_t) + oxmlist.length());

	m->type 	= match.type;
	m->length 	= match.length;

	oxmlist.pack((struct ofp_oxm_hdr*)(m->oxm_fields), oxmlist.length());

	return m;
}


void
cofmatch::unpack(struct ofp_match* m, size_t mlen) throw (eOFmatchInval)
{
	if (mlen < (sizeof(uint16_t) + sizeof(uint16_t)))
	{
		throw eOFmatchInval();
	}

	match.type		= (m->type);
	match.length	= (m->length);

	mlen -= 2 * sizeof(uint16_t);

	if (mlen > 0)
	{
		oxmlist.unpack((struct ofp_oxm_hdr*)m->oxm_fields, mlen);
	}

	validate();
}


bool 
cofmatch::overlaps(
	cofmatch const& m,
	bool strict /* = false (default) */)
{
	return (oxmlist.overlap(m.oxmlist, strict));
}

	

bool 
cofmatch::operator== (
	cofmatch& m)
{
	return ((match.type == m.match.type) && (oxmlist == m.oxmlist));
}


const char*
cofmatch::c_str()
{
	cvastring vas(3172);

	info.assign(vas("cofmatch(%p) hdr.type:%d hdr.length:%d stored:%lu oxmlist.length:%lu oxmlist:%s",
			this,
			be16toh(match.type),
			be16toh(match.length),
			length(),
			oxmlist.length(),
			oxmlist.c_str()));

	return info.c_str();
}


void
cofmatch::set_type(uint16_t type)
{
	match.type = htobe16(type);
}


uint32_t
cofmatch::get_in_port() throw (eOFmatchNotFound)
{
	if (not oxmlist.exists(OFPXMC_OPENFLOW_BASIC, OFPXMT_OFB_IN_PORT))
	{
		throw eOFmatchNotFound();
	}

	return oxmlist[OFPXMT_OFB_IN_PORT].uint32();
}


void
cofmatch::set_in_port(uint32_t in_port)
{
	oxmlist[OFPXMT_OFB_IN_PORT] = coxmatch_ofb_in_port(in_port);
}


uint32_t
cofmatch::get_in_phy_port() throw (eOFmatchNotFound)
{
	if (not oxmlist.exists(OFPXMC_OPENFLOW_BASIC, OFPXMT_OFB_IN_PHY_PORT))
	{
		throw eOFmatchNotFound();
	}

	return oxmlist[OFPXMT_OFB_IN_PHY_PORT].uint32();
}


void
cofmatch::set_in_phy_port(uint32_t in_port)
{
	oxmlist[OFPXMT_OFB_IN_PHY_PORT] = coxmatch_ofb_in_phy_port(in_port);
}


uint64_t
cofmatch::get_metadata() throw (eOFmatchNotFound)
{
	if (not oxmlist.exists(OFPXMC_OPENFLOW_BASIC, OFPXMT_OFB_METADATA))
	{
		throw eOFmatchNotFound();
	}

	return oxmlist[OFPXMT_OFB_METADATA].uint64();
}


void
cofmatch::set_metadata(uint64_t metadata)
{
	oxmlist[OFPXMT_OFB_METADATA] = coxmatch_ofb_metadata(metadata);
}


cmacaddr
cofmatch::get_eth_dst() throw (eOFmatchNotFound)
{
	if (not oxmlist.exists(OFPXMC_OPENFLOW_BASIC, OFPXMT_OFB_ETH_DST))
	{
		throw eOFmatchNotFound();
	}

	cmacaddr maddr(oxmlist[OFPXMT_OFB_ETH_DST].oxm_uint48t->value, OFP_ETH_ALEN);

	return maddr;
}


cmacaddr
cofmatch::get_eth_src() throw (eOFmatchNotFound)
{
	if (not oxmlist.exists(OFPXMC_OPENFLOW_BASIC, OFPXMT_OFB_ETH_SRC))
	{
		throw eOFmatchNotFound();
	}

	cmacaddr maddr(oxmlist[OFPXMT_OFB_ETH_SRC].oxm_uint48t->value, OFP_ETH_ALEN);

	return maddr;
}


void
cofmatch::set_eth_dst(cmacaddr const& maddr)
{
	oxmlist[OFPXMT_OFB_ETH_DST] = coxmatch_ofb_eth_dst(maddr);
}


void
cofmatch::set_eth_src(cmacaddr const& maddr)
{
	oxmlist[OFPXMT_OFB_ETH_SRC] = coxmatch_ofb_eth_src(maddr);
}


uint16_t
cofmatch::get_eth_type() throw (eOFmatchNotFound)
{
	if (not oxmlist.exists(OFPXMC_OPENFLOW_BASIC, OFPXMT_OFB_ETH_TYPE))
	{
		throw eOFmatchNotFound();
	}

	return oxmlist[OFPXMT_OFB_ETH_TYPE].uint16();
}


void
cofmatch::set_eth_type(
		uint16_t dl_type)
{
	oxmlist[OFPXMT_OFB_ETH_TYPE] = coxmatch_ofb_eth_type(dl_type);
}


#ifndef NDEBUG
void
cofmatch::test()
{
	cofmatch m;

	m.oxmlist[OFPXMT_OFB_ETH_SRC] 	= coxmatch_ofb_eth_src(cmacaddr("11:11:11:11:11:11"), cmacaddr("33:33:33:33:33:33"));
	m.oxmlist[OFPXMT_OFB_VLAN_VID] 	= coxmatch_ofb_vlan_vid(1000);
	m.oxmlist[OFPXMT_OFB_IP_DSCP] 	= coxmatch_ofb_ip_dscp(6);

	cmemory mem(m.length());

	m.pack((struct ofp_match*)mem.somem(), mem.memlen());
	fprintf(stderr, "match: %s\nmem:%s\n\n", m.c_str(), mem.c_str());

	cofmatch tm(m);

	fprintf(stderr, "tm: %s\n\n", tm.c_str());

	cofmatch cm;

	cm.unpack((struct ofp_match*)mem.somem(), mem.memlen());
	fprintf(stderr, "unpack: %s\n\n", cm.c_str());

	{
		cofmatch m;

		m.set_in_port(47);
		m.set_in_phy_port(47);
		m.set_eth_dst(cmacaddr("11:11:11:11:11:11"));
		m.set_eth_src(cmacaddr("22:22:22:22:22:22"));

		fprintf(stderr, "cofmatch: %s\n\n", m.c_str());
	}
}
#endif

