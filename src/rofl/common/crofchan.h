/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/*
 * crofchan.h
 *
 *  Created on: 31.12.2013
 *  Revised on: 28.09.2015
 *      Author: andreas
 */

#ifndef CROFCHAN_H_
#define CROFCHAN_H_

#include <map>
#include <bitset>
#include <inttypes.h>

#include "rofl/common/croflexception.h"
#include "rofl/common/crofconn.h"
#include "rofl/common/openflow/messages/cofmsg.h"
#include "rofl/common/openflow/cofhelloelemversionbitmap.h"
#include "rofl/common/crandom.h"
#include "rofl/common/cauxid.h"
#include "rofl/common/cthread.hpp"

namespace rofl {

class eRofChanBase : public RoflException {
public:
	eRofChanBase(
			const std::string& __arg) :
				RoflException(__arg)
	{};
};
class eRofChanNotFound : public eRofChanBase {
public:
	eRofChanNotFound(
			const std::string& __arg) :
				eRofChanBase(__arg)
	{};
};
class eRofChanExists : public eRofChanBase {
public:
	eRofChanExists(
			const std::string& __arg) :
				eRofChanBase(__arg)
	{};
};
class eRofChanInval : public eRofChanBase {
public:
	eRofChanInval(
			const std::string& __arg) :
				eRofChanBase(__arg)
	{};
};
class eRofChanNotConnected : public eRofChanBase {
public:
	eRofChanNotConnected(
			const std::string& __arg) :
				eRofChanBase(__arg)
	{};
};
class eRofChanExhausted : public eRofChanBase {
public:
	eRofChanExhausted(
			const std::string& __arg) :
				eRofChanBase(__arg)
	{};
};



class crofchan; // forward declaration




/**
 * @interface crofconn_env
 * @ingroup common_devel_workflow
 * @brief Environment expected by a rofl::crofconn instance.
 */
class crofchan_env {
	friend class crofchan;
public:
	static
	crofchan_env&
	call_env(crofchan_env* env) {
		AcquireReadLock lock(crofchan_env::channel_envs_lock);
		if (crofchan_env::channel_envs.find(env) == crofchan_env::channel_envs.end()) {
			throw eRofSockNotFound("crofchan_env::call_env() crofchan_env instance not found");
		}
		return *(env);
	};
public:
	virtual
	~crofchan_env() {
		AcquireReadWriteLock lock(crofchan_env::channel_envs_lock);
		crofchan_env::channel_envs.erase(this);
	};
	crofchan_env() {
		AcquireReadWriteLock lock(crofchan_env::channel_envs_lock);
		crofchan_env::channel_envs.insert(this);
	};

protected:

	virtual void
	handle_established(
			crofchan& chan, uint8_t ofp_version) = 0;

	virtual void
	handle_connect_refused(
			crofchan& chan, crofconn& conn) = 0;

	virtual void
	handle_connect_failed(
			crofchan& chan, crofconn& conn) = 0;

	virtual void
	handle_accept_failed(
			crofchan& chan, crofconn& conn) = 0;

	virtual void
	handle_negotiation_failed(
			crofchan& chan, crofconn& conn) = 0;

	virtual void
	handle_closed(
			crofchan& chan, crofconn& conn) = 0;

	virtual void
	handle_send(
			crofchan& chan, crofconn& conn) = 0;

	virtual void
	handle_recv(
			crofchan& chan, crofconn& conn, rofl::openflow::cofmsg* msg) = 0;

	virtual uint32_t
	get_async_xid(
			crofchan& chan, crofconn& conn) = 0;

	virtual uint32_t
	get_sync_xid(
			crofchan& chan, uint8_t msg_type = 0, uint16_t msg_sub_type = 0) = 0;

	virtual void
	release_sync_xid(
			crofchan& chan, uint32_t xid) = 0;

	virtual void
	congestion_indication(
			crofchan& chan, crofconn& conn) = 0;

private:
	static std::set<crofchan_env*>  channel_envs;
	static crwlock                  channel_envs_lock;
};



/**
 * @ingroup common_devel_workflow
 * @brief	An OpenFlow control channel grouping multiple control connections
 */
class crofchan :
		public cthread_env,
		public rofl::crofconn_env
{
public:

	/**
	 *
	 */
	virtual
	~crofchan()
	{ clear(); };

	/**
	 *
	 */
	crofchan(
			crofchan_env *env) :
				env(env),
				last_auxid(0),
				ofp_version(rofl::openflow::OFP_VERSION_UNKNOWN)
	{};

public:

	/**
	 *
	 */
	void
	send_message(
			const cauxid& auxid, rofl::openflow::cofmsg* msg);

public:

	/**
	 *
	 */
	bool
	is_established() const;

	/**
	 *
	 */
	uint8_t
	get_version() const
	{ return ofp_version; };

public:

	/**
	 *
	 */
	size_t
	size() const {
		AcquireReadLock rwlock(conns_rwlock);
		return conns.size();
	};

	/**
	 *
	 */
	std::list<cauxid>
	get_conn_ids() const {
		AcquireReadLock rwlock(conns_rwlock);
		std::list<cauxid> ids;
		for (auto it : conns) {
			ids.push_back(it.first);
		}
		return ids;
	}

	/**
	 *
	 */
	void
	clear() {
		AcquireReadWriteLock rwlock(conns_rwlock);
		for (auto it : conns) {
			delete it.second;
		}
		conns.clear();
	};

	/**
	 *
	 */
	crofconn&
	add_conn() {
		AcquireReadWriteLock rwlock(conns_rwlock);
		unsigned int cnt = 256;
		while ((cnt--) && (conns.find(last_auxid) != conns.end())) {
			last_auxid = (last_auxid == 255) ? 0 : last_auxid + 1;
		}
		if (cnt == 0) {
			throw eRofChanExhausted("crofchan::add_conn() cauxid namespace exhausted");
		}
		(conns[last_auxid] = new crofconn(this))->set_auxid(cauxid(last_auxid));
		return *(conns[last_auxid]);
	};

	/**
	 *
	 */
	crofconn&
	add_conn(
			const cauxid& auxid) {
		AcquireReadWriteLock rwlock(conns_rwlock);
		if (conns.find(auxid) != conns.end()) {
			delete conns[auxid];
		}
		(conns[auxid] = new crofconn(this))->set_auxid(auxid);
		return *(conns[auxid]);
	};

	/**
	 *
	 */
	crofconn&
	add_conn(
			crofconn* conn) {
		if (nullptr == conn) {
			throw eRofChanInval("crofchan::add_conn() null pointer");
		}
		AcquireReadWriteLock rwlock(conns_rwlock);
		cauxid auxid(conn->get_auxid());
		if (conns.find(auxid) != conns.end()) {
			delete conns[auxid];
		}
		(conns[auxid] = conn)->set_env(this);
		return *(conns[auxid]);
	};

	/**
	 *
	 */
	crofconn&
	set_conn(
			const cauxid& auxid) {
		AcquireReadWriteLock rwlock(conns_rwlock);
		if (conns.find(auxid) == conns.end()) {
			(conns[auxid] = new crofconn(this))->set_auxid(auxid);
		}
		return *(conns[auxid]);
	};

	/**
	 *
	 */
	const crofconn&
	get_conn(
			const cauxid& auxid) const {
		AcquireReadLock rwlock(conns_rwlock);
		if (conns.find(auxid) == conns.end()) {
			throw eRofChanNotFound("crofchan::get_conn() auxid not found");
		}
		return *(conns.at(auxid));
	};

	/**
	 *
	 */
	bool
	drop_conn(
			const cauxid& auxid) {
		AcquireReadWriteLock rwlock(conns_rwlock);
		if (auxid.get_id() == 0) {
			for (auto it : conns) {
				delete it.second;
			}
			conns.clear();
		} else {
			if (conns.find(auxid) == conns.end()) {
				return false;
			}
			delete conns[auxid];
			conns.erase(auxid);
		}
		return true;
	};

	/**
	 *
	 */
	bool
	has_conn(
			const cauxid& auxid) const {
		AcquireReadLock rwlock(conns_rwlock);
		return (not (conns.find(auxid) == conns.end()));
	};

private:

	virtual void
	handle_established(
			crofconn& conn, uint8_t ofp_version)
	{
		if (conn.get_auxid().get_id() == 0) {
			ofp_version = conn.get_version();
			crofchan_env::call_env(env).handle_established(*this, ofp_version);
		}
	};

	virtual void
	handle_closed(
			crofconn& conn)
	{
		AcquireReadLock rwlock(conns_rwlock);
		if (conn.get_auxid().get_id() == 0) {
			for (auto it : conns) {
				it.second->close();
			}
		}
		crofchan_env::call_env(env).handle_closed(*this, conn);
	};

	virtual void
	handle_connect_refused(
			crofconn& conn)
	{ crofchan_env::call_env(env).handle_connect_refused(*this, conn); };

	virtual void
	handle_connect_failed(
			crofconn& conn)
	{ crofchan_env::call_env(env).handle_connect_failed(*this, conn); };

	virtual void
	handle_accept_failed(
			crofconn& conn)
	{ crofchan_env::call_env(env).handle_accept_failed(*this, conn); };

	virtual void
	handle_negotiation_failed(
			crofconn& conn)
	{ crofchan_env::call_env(env).handle_negotiation_failed(*this, conn); };

	virtual void
	handle_send(
			crofconn& conn)
	{ crofchan_env::call_env(env).handle_send(*this, conn); };

	virtual void
	handle_recv(
			crofconn& conn, rofl::openflow::cofmsg *msg)
	{ crofchan_env::call_env(env).handle_recv(*this, conn, msg); };

	virtual uint32_t
	get_async_xid(
			crofconn& conn)
	{ return crofchan_env::call_env(env).get_async_xid(*this, conn); };

	virtual uint32_t
	get_sync_xid(
			crofconn& conn, uint8_t msg_type = 0, uint16_t msg_sub_type = 0)
	{ return crofchan_env::call_env(env).get_sync_xid(*this, msg_type, msg_sub_type); };

	virtual void
	release_sync_xid(
			crofconn& conn, uint32_t xid)
	{ crofchan_env::call_env(env).release_sync_xid(*this, xid); };

	virtual void
	congestion_indication(
			crofconn& conn)
	{ crofchan_env::call_env(env).congestion_indication(*this, conn); };

public:

	friend std::ostream&
	operator<< (std::ostream& os, crofchan const& chan) {
		AcquireReadLock rwlock(chan.conns_rwlock);
		os << indent(0) << "<crofchan established:" << chan.is_established()
				<< " ofp-version: " << (int)chan.ofp_version << " >" << std::endl;
		indent i(2);
		for (std::map<cauxid, crofconn*>::const_iterator
				it = chan.conns.begin(); it != chan.conns.end(); ++it) {
			os << *(it->second);
		}
		return os;
	};


	std::string
	str() const {
		AcquireReadLock rwlock(conns_rwlock);
		std::stringstream ss;
		ss << "OFP version: " << (int)get_version() << " ";
		if (conns.empty() ||
				(conns.find(rofl::cauxid(0)) == conns.end()) ||
					(not (conns.at(rofl::cauxid(0))->is_established()))) {
			ss << " state: -disconnected- ";
		} else {
			ss << " state: -established- ";
		}
		ss << "auxids: ";
		for (std::map<cauxid, crofconn*>::const_iterator
				it = conns.begin(); it != conns.end(); ++it) {
			ss << "{" << (int)it->first.get_id() << ":"
					<< (it->second->is_established() ? "-established-" : "-disconnected-") << "} ";
		}
		return ss.str();
	};

private:

	// owner of this crofchan instance
	crofchan_env*                       env;

	// main and auxiliary connections
	std::map<cauxid, crofconn*>         conns;

	// connections set rwlock
	mutable crwlock                     conns_rwlock;

	// last auxid assigned
	uint8_t                             last_auxid;

	// OFP version negotiated
	uint8_t                             ofp_version;

	// state related flags
	std::bitset<32>                     flags;
};

}; /* namespace rofl */

#endif /* CROFCHAN_H_ */
