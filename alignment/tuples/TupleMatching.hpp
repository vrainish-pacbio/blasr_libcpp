#ifndef _BLASR_TUPLE_MATCHING_HPP_
#define _BLASR_TUPLE_MATCHING_HPP_

#include "TupleMetrics.hpp"
#include "TupleList.hpp"
#include "BaseTuple.hpp"
#include "DNATuple.hpp"
#include "TupleMatching.hpp"

template<typename Sequence, typename T_TupleList> 
int SequenceToTupleList(
    Sequence &seq, TupleMetrics &tm, T_TupleList &tupleList); 

template<typename TSequence, typename TMatch, typename T_TupleList>
int StoreMatchingPositions(
    TSequence &querySeq, TupleMetrics &tm, T_TupleList &targetTupleList, 
    std::vector<TMatch> &matchSet); 

template<typename Sequence, typename Tuple>
int StoreUniqueTuplePosList(Sequence seq, TupleMetrics &tm, 
    std::vector<int> &uniqueTuplePosList); 

#include "TupleMatchingImpl.hpp"

#endif // _BLASR_TUPLE_MATCHING_HPP_
