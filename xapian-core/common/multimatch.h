/* multimatch.h: class for performing a match
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 BrightStation PLC
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 * -----END-LICENCE-----
 */

#ifndef OM_HGUARD_MULTIMATCH_H
#define OM_HGUARD_MULTIMATCH_H

#include "match.h"
#include "stats.h"
#include "omrefcnt.h"

#include <vector>
#include <memory>  // auto_ptr

class MultiDatabase;
class SingleMatch;

/** Class for performing a match over multiple SingleMatch objects.
 */
class MultiMatch
{
    private:
	/// Vector of the items
	std::vector<OmRefCntPtr<SingleMatch> > leaves;

	/// The database
	const MultiDatabase * multi_database;

	/// stats gatherer
	std::auto_ptr<StatsGatherer> gatherer;

	/// Comparison functor for sorting MSet
	OmMSetCmp mcmp;

	/// Construct a SingleMatch object from an IRDatabase
	OmRefCntPtr<SingleMatch> make_match_from_database(IRDatabase *db);

	/** Change all the document IDs in the given mset to be valid
	 *  globally, rather than within the sub-match which produced them.
	 *
	 *  @param mset              The MSet to alter.
	 *  @param leaf_number       The sub-match which this mset comes from.
	 *  			     (Starting at 1)
	 */
	void change_docids_to_global(std::vector<OmMSetItem> & mset,
				     om_doccount leaf_number);

	/** Return true if the specified key has not been seen before.
	 *
	 *  If the given key has a null value, this method returns true.
	 *
	 *  Otherwise, the given key is searched for in collapse_entries.
	 *  If it is not found it is inserted into this set, and the
	 *  method returns true, otherwise the method returns false.
	 *
	 *  @param collapse_entries  The set of keys which have been seen.
	 *  @param new_key           The new key to look for.
	 */
	bool have_not_seen_key(std::set<OmKey> & collapse_entries,
			       const OmKey & new_key);

        /** Merge two msets together.
	 *
	 *  @param mset      The mset to put the results in.
	 *  @param more_mset
	 */
	void merge_msets(std::vector<OmMSetItem> &mset,
			 std::vector<OmMSetItem> &more_mset,
			 om_doccount lastitem);

	/** Add the next mset available to the current mset, if it is available.
	 *
	 *  FIXME: this method should be refactored to reduce the number
	 *  of parameters.
	 */
	bool add_next_sub_mset(SingleMatch * leaf,
			       om_doccount leaf_number,
			       om_doccount lastitem,
			       const OmMatchDecider *mdecider,
			       OmMSet & mset,
			       bool nowait);

	/** Prepare all the sub matchers.
	 *
	 *  This calls prepare_match() on all the sub matchers until they
	 *  all return true to signal that they are prepared.
	 */
	void prepare_matchers();

	/** Collect the msets generated by all the leaves, and put them into
	 *  a single mset.
	 *
	 *  This runs the match process for each leaf, and merges the results
	 *  into a single mset.  If collapse keys are present in the
	 *  OmMSetItems, duplicates are removed based upon their values.
	 *
	 *  The first result in the combined mset will always be the highest
	 *  ranked document (there is no way to specify that only the items
	 *  after a given position should be returned).
	 *
	 *  @param lastitem  The number of items to retrieve into the mset.
	 *  @param mdecider  The decision functor for the match process.
	 *  @param mset      The MSet object to put the combined msets into.
	 */
	void collect_msets(om_doccount lastitem,
			   const OmMatchDecider *mdecider,
			   OmMSet & mset);

	/** Remove elements from the start of the list of items in the mset.
	 *
	 *  This removes the given number of elements from the start of the
	 *  items in the mset.  It also increments mset.firstitem
	 *  appropriately, so that it still has the correct value.
	 *
	 *  If number_to_remove is greater than the number of elements in
	 *  the mset, the resulting mset is empty.
	 *
	 *  @param number_to_remove  Number of items to remove.
	 *  @param mset              MSet object to modify.
	 */
	void remove_leading_elements(om_doccount number_to_remove,
				     OmMSet & mset);

	/// Copying is not permitted.
	MultiMatch(const MultiMatch &);

	/// Assignment is not permitted.
	void operator=(const MultiMatch &);


	void set_query(const OmQueryInternal * query);
	void set_rset(const OmRSet & omrset);
	void set_weighting(IRWeight::weight_type wt_type);
	void set_options(const OmMatchOptions & moptions);
    public:
	/** MultiMatch constructor.
	 *
	 *  Caller is responsible for ensuring that the MultiDatabase
	 *  object pointed to remains valid until the multimatch object
	 *  is destroyed, and for deallocating the object afterwards.
	 *
	 *  @param database_ A pointer to the database to use.
	 *
	 *  @param gatherer_ An auto_ptr to a StatsGatherer instance.
	 *                   The MultiMatch takes ownership of the
	 *                   StatsGatherer.  This argument is optional;
	 *                   the default is to use a LocalStatsGatherer,
	 *                   suitable for non-network use.
	 */
	MultiMatch(const MultiDatabase * multi_database_,
		   const OmQueryInternal * query,
		   const OmRSet & omrset,
		   IRWeight::weight_type wt_type,
		   const OmMatchOptions & moptions,
		   std::auto_ptr<StatsGatherer> gatherer_
		       = std::auto_ptr<StatsGatherer>(new LocalStatsGatherer()));
	~MultiMatch();

	void match(om_doccount first,
		   om_doccount maxitems,
		   OmMSet & mset,
		   const OmMatchDecider *mdecider);
};

#endif /* OM_HGUARD_MULTIMATCH_H */
