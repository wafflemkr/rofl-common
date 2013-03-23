#include "cofmsg_queue_stats.h"

using namespace rofl;




cofmsg_queue_stats_request::cofmsg_queue_stats_request(
		uint8_t of_version,
		uint32_t xid,
		uint16_t flags) :
	cofmsg_stats(of_version, OFPT_STATS_REQUEST, xid, OFPST_QUEUE, flags)
{
	switch (of_version) {
	case OFP10_VERSION: {
		resize(sizeof(struct ofp10_stats_request));
	} break;
	case OFP12_VERSION: {
		resize(sizeof(struct ofp12_stats_request));
	} break;
	case OFP13_VERSION: {
		// TODO
		throw eNotImplemented();
	} break;
	default:
		throw eBadVersion();
	}
}



cofmsg_queue_stats_request::cofmsg_queue_stats_request(
		cmemory *memarea) :
	cofmsg_stats(memarea)
{
	validate();
}



cofmsg_queue_stats_request::cofmsg_queue_stats_request(
		cofmsg_queue_stats_request const& stats)
{
	*this = stats;
}



cofmsg_queue_stats_request&
cofmsg_queue_stats_request::operator= (
		cofmsg_queue_stats_request const& stats)
{
	if (this == &stats)
		return *this;

	cofmsg_stats::operator =(stats);

	return *this;
}



cofmsg_queue_stats_request::~cofmsg_queue_stats_request()
{

}



void
cofmsg_queue_stats_request::reset()
{
	cofmsg_stats::reset();
}



void
cofmsg_queue_stats_request::resize(size_t len)
{
	cofmsg::resize(len);
}



size_t
cofmsg_queue_stats_request::length() const
{
	switch (get_version()) {
	case OFP10_VERSION: {
		return (sizeof(struct ofp10_stats_request));
	} break;
	case OFP12_VERSION: {
		return (sizeof(struct ofp12_stats_request));
	} break;
	case OFP13_VERSION: {
		// TODO
		throw eNotImplemented();
	} break;
	default:
		throw eBadVersion();
	}
	return 0;
}



void
cofmsg_queue_stats_request::pack(uint8_t *buf, size_t buflen)
{
	cofmsg_stats::pack(buf, buflen); // copies common statistics header

	if ((0 == buf) || (0 == buflen))
		return;

	if (buflen < length())
		throw eInval();

	switch (get_version()) {
	case OFP10_VERSION: {
		if (buflen < length())
			throw eInval();
	} break;
	case OFP12_VERSION: {
		if (buflen < length())
			throw eInval();
	} break;
	case OFP13_VERSION: {
		// TODO
		throw eNotImplemented();
	} break;
	default:
		throw eBadVersion();
	}
}



void
cofmsg_queue_stats_request::unpack(uint8_t *buf, size_t buflen)
{
	cofmsg_stats::unpack(buf, buflen);

	validate();
}



void
cofmsg_queue_stats_request::validate()
{
	cofmsg_stats::validate(); // check generic statistics header

	switch (get_version()) {
	case OFP10_VERSION: {
		if (get_length() < sizeof(struct ofp10_stats_request))
			throw eBadSyntaxTooShort();
	} break;
	case OFP12_VERSION: {
		if (get_length() < (sizeof(struct ofp12_stats_request) + sizeof(struct ofp12_queue_stats)))
			throw eBadSyntaxTooShort();
	} break;
	case OFP13_VERSION: {
		// TODO
		throw eNotImplemented();
	} break;
	default:
		throw eBadRequestBadVersion();
	}
}






cofmsg_queue_stats_reply::cofmsg_queue_stats_reply(
		uint8_t of_version,
		uint32_t xid,
		uint16_t flags,
		std::vector<cofqueue_stats_reply> const& queue_stats) :
	cofmsg_stats(of_version, OFPT_STATS_REPLY, xid, OFPST_QUEUE, flags),
	queue_stats(queue_stats)
{
	switch (of_version) {
	case OFP10_VERSION: {
		resize(sizeof(struct ofp10_stats_reply) + queue_stats.size() * sizeof(struct ofp10_queue_stats));
		for (unsigned int i = 0; i < queue_stats.size(); i++) {
			queue_stats[i].pack(soframe() + i * sizeof(struct ofp10_stats_reply), sizeof(struct ofp10_queue_stats));
		}
	} break;
	case OFP12_VERSION: {
		resize(sizeof(struct ofp12_stats_reply) + queue_stats.size() * sizeof(struct ofp12_queue_stats));
		for (unsigned int i = 0; i < queue_stats.size(); i++) {
			queue_stats[i].pack(soframe() + i * sizeof(struct ofp12_stats_reply), sizeof(struct ofp12_queue_stats));
		}
	} break;
	case OFP13_VERSION: {
		// TODO
		throw eNotImplemented();
	} break;
	default:
		throw eBadVersion();
	}
}



cofmsg_queue_stats_reply::cofmsg_queue_stats_reply(
		cmemory *memarea) :
	cofmsg_stats(memarea)
{
	validate();
}



cofmsg_queue_stats_reply::cofmsg_queue_stats_reply(
		cofmsg_queue_stats_reply const& stats)
{
	*this = stats;
}



cofmsg_queue_stats_reply&
cofmsg_queue_stats_reply::operator= (
		cofmsg_queue_stats_reply const& stats)
{
	if (this == &stats)
		return *this;

	cofmsg_stats::operator =(stats);

	ofh_queue_stats = soframe();

	return *this;
}



cofmsg_queue_stats_reply::~cofmsg_queue_stats_reply()
{

}



void
cofmsg_queue_stats_reply::reset()
{
	cofmsg_stats::reset();
}



void
cofmsg_queue_stats_reply::resize(size_t len)
{
	cofmsg::resize(len);
	switch (get_version()) {
	case OFP10_VERSION: {
		ofh_queue_stats = soframe() + sizeof(struct ofp10_stats_reply);
	} break;
	case OFP12_VERSION: {
		ofh_queue_stats = soframe() + sizeof(struct ofp12_stats_reply);
	} break;
	case OFP13_VERSION: {
		// TODO
		throw eNotImplemented();
	} break;
	default:
		throw eBadVersion();
	}

}



size_t
cofmsg_queue_stats_reply::length() const
{
	switch (get_version()) {
	case OFP10_VERSION: {
		return (sizeof(struct ofp10_stats_reply) + queue_stats.size() * sizeof(struct ofp10_queue_stats));
	} break;
	case OFP12_VERSION: {
		return (sizeof(struct ofp12_stats_reply) + queue_stats.size() * sizeof(struct ofp12_queue_stats));
	} break;
	case OFP13_VERSION: {
		// TODO
		throw eNotImplemented();
	} break;
	default:
		throw eBadVersion();
	}
	return 0;
}



void
cofmsg_queue_stats_reply::pack(uint8_t *buf, size_t buflen)
{
	cofmsg_stats::pack(buf, buflen); // copies common statistics header

	if ((0 == buf) || (0 == buflen))
		return;

	if (buflen < length())
		throw eInval();

	switch (get_version()) {
	case OFP10_VERSION: {
		if (buflen < length())
			throw eInval();
		for (unsigned int i = 0; i < queue_stats.size(); i++) {
			queue_stats[i].pack(soframe() + i * sizeof(struct ofp10_stats_reply), sizeof(struct ofp10_queue_stats));
		}
	} break;
	case OFP12_VERSION: {
		if (buflen < length())
			throw eInval();
		for (unsigned int i = 0; i < queue_stats.size(); i++) {
			queue_stats[i].pack(soframe() + i * sizeof(struct ofp12_stats_reply), sizeof(struct ofp12_queue_stats));
		}
	} break;
	case OFP13_VERSION: {
		// TODO
		throw eNotImplemented();
	} break;
	default:
		throw eBadVersion();
	}
}



void
cofmsg_queue_stats_reply::unpack(uint8_t *buf, size_t buflen)
{
	cofmsg_stats::unpack(buf, buflen);

	validate();
}



void
cofmsg_queue_stats_reply::validate()
{
	cofmsg_stats::validate(); // check generic statistics header

	queue_stats.clear();

	switch (get_version()) {
	case OFP10_VERSION: {
		if (get_length() < sizeof(struct ofp10_stats_reply))
			throw eBadSyntaxTooShort();
		for (unsigned int i = 0; i < ((get_length() - sizeof(struct ofp10_stats_reply)) / sizeof(struct ofp10_queue_stats)); i++) {
			cofqueue_stats_reply queue_stats_reply;
			queue_stats_reply.unpack(soframe() + sizeof(struct ofp10_stats_reply) + i * sizeof(struct ofp10_queue_stats), sizeof(struct ofp10_queue_stats));
			queue_stats.push_back(queue_stats_reply);
		}
	} break;
	case OFP12_VERSION: {
		if (get_length() < (sizeof(struct ofp12_stats_reply) + sizeof(struct ofp12_queue_stats)))
			throw eBadSyntaxTooShort();
		for (unsigned int i = 0; i < ((get_length() - sizeof(struct ofp12_stats_reply)) / sizeof(struct ofp12_queue_stats)); i++) {
			cofqueue_stats_reply queue_stats_reply;
			queue_stats_reply.unpack(soframe() + sizeof(struct ofp12_stats_reply) + i * sizeof(struct ofp12_queue_stats), sizeof(struct ofp12_queue_stats));
			queue_stats.push_back(queue_stats_reply);
		}
	} break;
	case OFP13_VERSION: {
		// TODO
		throw eNotImplemented();
	} break;
	default:
		throw eBadRequestBadVersion();
	}
}



std::vector<cofqueue_stats_reply>&
cofmsg_queue_stats_reply::get_queue_stats()
{
	return queue_stats;
}


