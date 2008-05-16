/*
 * (C) 2007 by Pablo Neira Ayuso <pablo@netfilter.org>
 *
 * This software may be used and distributed according to the terms
 * of the GNU General Public License, incorporated herein by reference.
 */

#include "internal.h"

static int cmp_orig(const struct nf_conntrack *ct1,
		    const struct nf_conntrack *ct2)
{
	if (test_bit(ATTR_ORIG_L3PROTO, ct1->set) &&
	    test_bit(ATTR_ORIG_L3PROTO, ct2->set) &&
	    ct1->tuple[__DIR_ORIG].l3protonum != AF_UNSPEC && 
	    ct2->tuple[__DIR_ORIG].l3protonum != AF_UNSPEC &&
	    ct1->tuple[__DIR_ORIG].l3protonum !=
	    ct2->tuple[__DIR_ORIG].l3protonum)
 	   	return 0;

	if (test_bit(ATTR_ORIG_L4PROTO, ct1->set) &&
	    test_bit(ATTR_ORIG_L4PROTO, ct2->set) &&
	    ct1->tuple[__DIR_ORIG].protonum !=
	    ct2->tuple[__DIR_ORIG].protonum)
		return 0;

	if (test_bit(ATTR_ORIG_IPV4_SRC, ct1->set) &&
	    test_bit(ATTR_ORIG_IPV4_SRC, ct2->set) &&
	    ct1->tuple[__DIR_ORIG].src.v4 !=
	    ct2->tuple[__DIR_ORIG].src.v4)
		return 0;

	if (test_bit(ATTR_ORIG_IPV4_DST, ct1->set) &&
	    test_bit(ATTR_ORIG_IPV4_DST, ct2->set) &&
	    ct1->tuple[__DIR_ORIG].dst.v4 !=
	    ct2->tuple[__DIR_ORIG].dst.v4)
		return 0;

	if (test_bit(ATTR_ORIG_IPV6_SRC, ct1->set) &&
	    test_bit(ATTR_ORIG_IPV6_SRC, ct2->set) &&
	    memcmp(&ct1->tuple[__DIR_ORIG].src.v6,
	    	   &ct2->tuple[__DIR_ORIG].src.v6,
		   sizeof(u_int32_t)*4) == 0)
		return 0;

	if (test_bit(ATTR_ORIG_IPV6_DST, ct1->set) &&
	    test_bit(ATTR_ORIG_IPV6_DST, ct2->set) &&
	    memcmp(&ct1->tuple[__DIR_ORIG].dst.v6,
	    	   &ct2->tuple[__DIR_ORIG].dst.v6,
		   sizeof(u_int32_t)*4) == 0)
		return 0;

	return 1;
}

static int cmp_repl(const struct nf_conntrack *ct1,
		    const struct nf_conntrack *ct2)
{
	if (test_bit(ATTR_REPL_L3PROTO, ct1->set) &&
	    test_bit(ATTR_REPL_L3PROTO, ct2->set) &&
	    ct1->tuple[__DIR_REPL].l3protonum != AF_UNSPEC && 
	    ct2->tuple[__DIR_REPL].l3protonum != AF_UNSPEC &&
	    ct1->tuple[__DIR_REPL].l3protonum !=
	    ct2->tuple[__DIR_REPL].l3protonum)
		return 0;

	if (test_bit(ATTR_REPL_L4PROTO, ct1->set) &&
	    test_bit(ATTR_REPL_L4PROTO, ct2->set) &&
	    ct1->tuple[__DIR_REPL].protonum !=
	    ct2->tuple[__DIR_REPL].protonum)
		return 0;

	if (test_bit(ATTR_REPL_IPV4_SRC, ct1->set) &&
	    test_bit(ATTR_REPL_IPV4_SRC, ct2->set) &&
	    ct1->tuple[__DIR_REPL].src.v4 !=
	    ct2->tuple[__DIR_REPL].src.v4)
		return 0;

	if (test_bit(ATTR_REPL_IPV4_DST, ct1->set) && 
	    test_bit(ATTR_REPL_IPV4_DST, ct2->set) &&
	    ct1->tuple[__DIR_REPL].dst.v4 !=
	    ct2->tuple[__DIR_REPL].dst.v4)
		return 0;

	if (test_bit(ATTR_REPL_IPV6_SRC, ct1->set) &&
	    test_bit(ATTR_REPL_IPV6_SRC, ct2->set) &&
	    memcmp(&ct1->tuple[__DIR_REPL].src.v6,
	    	   &ct2->tuple[__DIR_REPL].src.v6,
		   sizeof(u_int32_t)*4) == 0)
		return 0;

	if (test_bit(ATTR_REPL_IPV6_DST, ct1->set) &&
	    test_bit(ATTR_REPL_IPV6_DST, ct2->set) &&
	    memcmp(&ct1->tuple[__DIR_REPL].dst.v6,
	    	   &ct2->tuple[__DIR_REPL].dst.v6,
		   sizeof(u_int32_t)*4) == 0)
		return 0;

	return 1;
}

static int cmp_meta(const struct nf_conntrack *ct1,
		    const struct nf_conntrack *ct2,
		    unsigned int flags)
{
	if (test_bit(ATTR_ID, ct1->set) &&
	    test_bit(ATTR_ID, ct2->set) &&
	    ct1->id != ct2->id)
		return 0;

	if (test_bit(ATTR_MARK, ct1->set) && 
	    test_bit(ATTR_MARK, ct2->set) &&
	    ct1->mark != ct2->mark)
	    	return 0;

	if (test_bit(ATTR_TIMEOUT, ct1->set) &&
	    test_bit(ATTR_TIMEOUT, ct2->set)) {
	    	int ret = 0;

#define __NFCT_CMP_TIMEOUT (NFCT_CMP_TIMEOUT_LE | NFCT_CMP_TIMEOUT_GT)

		if (!(flags & __NFCT_CMP_TIMEOUT) &&
		    ct1->timeout != ct2->timeout)
		    	return 0;
		else {
			if (flags & NFCT_CMP_TIMEOUT_GT &&
			    ct1->timeout > ct2->timeout)
				ret = 1;
			else if (flags & NFCT_CMP_TIMEOUT_LT &&
				 ct1->timeout < ct2->timeout)
			    	ret = 1;
			else if (flags & NFCT_CMP_TIMEOUT_EQ &&
				 ct1->timeout == ct2->timeout)
				ret = 1;

		    	if (ret == 0)
				return 0;
		}
	}

	if (test_bit(ATTR_STATUS, ct1->set) &&
	    test_bit(ATTR_STATUS, ct2->set) &&
	    !((ct1->status & ct2->status) == ct1->status))
	    	return 0;

	if (test_bit(ATTR_TCP_STATE, ct1->set) &&
	    test_bit(ATTR_TCP_STATE, ct2->set) &&
	    ct1->protoinfo.tcp.state != ct2->protoinfo.tcp.state)
	    	return 0;

	return 1;
}

int __compare(const struct nf_conntrack *ct1,
	      const struct nf_conntrack *ct2,
	      unsigned int flags)
{
	if (flags == NFCT_CMP_ALL)
		return cmp_meta(ct1, ct2, flags) &&
		       cmp_orig(ct1, ct2) &&
		       cmp_repl(ct1, ct2);

	if (flags & NFCT_CMP_ORIG && !cmp_orig(ct1, ct2))
		return 0;

	if (flags & NFCT_CMP_REPL && !cmp_repl(ct1, ct2))
		return 0;

	return 1;
}