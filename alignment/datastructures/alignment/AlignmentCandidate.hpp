#ifndef _ALIGNMENT_ALIGNMENT_CANDIDATE_HPP_
#define _ALIGNMENT_ALIGNMENT_CANDIDATE_HPP_

#include "Alignment.hpp"
// pbdata
#include "../../../pbdata/DNASequence.hpp"
#include "../../../pbdata/FASTQSequence.hpp"

template<typename T_TSequence=FASTASequence, typename T_QSequence=FASTASequence>
class AlignmentCandidate : public blasr::Alignment {

private:
    void ReassignSequence(DNASequence &curSeq, bool curIsSubstring,  DNASequence &newSeq) {
        //
        // If this sequence is in control of itself (it is not a substring
        // of anoter sequence), it should be freed here to avoid memory
        // leaks.
        //
        if (curIsSubstring == false) {
            curSeq.Free();
        }
        curSeq.seq = newSeq.seq;
        curSeq.length = newSeq.length;
    }

    void ReassignSequence(FASTQSequence &curSeq, bool curIsSubstring, FASTQSequence &newSeq) {
        //
        // Free the current sequence with the same rules as above.
        //
        if (curIsSubstring == false) {
            curSeq.Free();
        }
        curSeq.ReferenceSubstring(newSeq, 0, newSeq.length);
    }
    
    // TryReadingQVs is a helper function for ReadOptionalQVs. It checks if the
    // qvs that are supposed to be copied to a member of AlignmentCandidate are
    // in fact empty. If so, then don't substr or copy anything
    void TryReadingQVs(const std::string qvs, DNALength start, DNALength length,
                       std::string *memberQVs) {
        if (qvs.size() == 0) {
            memberQVs->clear();
        } else {
            *memberQVs = qvs.substr(start, length);
        }
    }



public:
    T_TSequence tAlignedSeq;
    T_QSequence qAlignedSeq;
    std::string insertionQV, deletionQV, mergeQV, substitutionQV, deletionTag, substitutionTag;
    std::vector<std::string> optionalQVNames;
    DNALength   tAlignedSeqPos, qAlignedSeqPos;
    DNALength   tAlignedSeqLength, qAlignedSeqLength;
    float       pvalVariance, weightVariance, pvalNStdDev, weightNStdDev;
    int         numSignificantClusters;
    int readIndex;
    //
    // [q/t]IsSubstring refers to whether or not the text or query
    // sequences are pointers into longer sequences.  If they are,
    // then reassigning the target/query sequences just means
    // reassigning the seq and length values.  If not, they occupy their
    // own space, and should have the same lifetime as the alignment
    // candidate object.
    //

    bool tIsSubstring, qIsSubstring;
    std::string tTitle, qTitle;
    float clusterScore, clusterWeight;
    //
    // For indexing into sequence index databases, tIndex stores which
    // target this match is from.
    //
    int tIndex;

    AlignmentCandidate&operator=(const AlignmentCandidate &rhs)  {
        tAlignedSeq         = rhs.tAlignedSeq;
        qAlignedSeq         = rhs.qAlignedSeq;
        tAlignedSeqPos      = rhs.tAlignedSeqPos;
        qAlignedSeqPos      = rhs.qAlignedSeqPos;
        tAlignedSeqLength   = rhs.tAlignedSeqLength;
        qAlignedSeqLength   = rhs.qAlignedSeqLength;
        readIndex           = rhs.readIndex;
        tIndex              = rhs.tIndex;
        mapQV               = rhs.mapQV;
        clusterScore        = rhs.clusterScore;
        clusterWeight       = rhs.clusterWeight;
        *((Alignment*)this) = ((Alignment&)rhs);

        pvalVariance = rhs.pvalVariance;
        pvalNStdDev  = rhs.pvalNStdDev;
        weightVariance=rhs.weightVariance;
        weightNStdDev= rhs.weightNStdDev;

        insertionQV = rhs.insertionQV;
        deletionQV = rhs.deletionQV;
        substitutionQV = rhs.substitutionQV;
        mergeQV = rhs.mergeQV;
        substitutionTag = rhs.substitutionTag;
        deletionTag = rhs.deletionTag;

        return *this;
    }

    AlignmentCandidate() {
        /*
         * The default configuration of an alignment candidate is to have
         * the t and q sequences be substrings.  This means that the
         * position of the substrings starts at 0 (no offset into a longer
         * string).
         */
        tIsSubstring      = true;
        qIsSubstring      = true;
        tTitle            = "";
        qTitle            = "";
        tAlignedSeqPos    = 0;
        qAlignedSeqPos    = 0;
        tAlignedSeqLength = 0;
        qAlignedSeqLength = 0;
        tIndex            = 0;
        readIndex         = 0;
        mapQV             = 50;
        clusterScore      = 0;
        clusterWeight     = 0;
        numSignificantClusters = 0;
        pvalVariance = pvalNStdDev = 0;
        weightVariance = weightNStdDev = 0;
    }

    void Print(std::ostream & out = std::cout) {
        out << "An AlignmentCandidate object (mapQV " 
            << mapQV << ", clusterscore " << clusterScore 
            << ", tTitle: " << tTitle << ", qTitle: " << qTitle 
            << ")." << std::endl;
        out << "  query: " << qTitle << ", "  
            << "qName: " << qName << ","
            << "qStrand: " << qStrand << ", " 
            << "qPos: " << qPos << ", "
            << "qLen: " << qLength << ", "
            << "qAlignLength: " << qAlignLength << ", "
            << "qAlignedSeqPos:" << qAlignedSeqPos << ", "
            << "qAlignedSeqLen:" << qAlignedSeqLength << std::endl
            << "  target: " << tTitle << ", "  
            << "tName: " << tName << ","
            << "tStrand: " << tStrand << ", " 
            << "tPos: " << tPos << ", "
            << "tLen: " << tLength << ", "
            << "tAlignLength: " << tAlignLength << ", "
            << "tAlignedSeqPos:" << tAlignedSeqPos << ", " 
            << "tAlignedSeqLen:" << tAlignedSeqLength << std::endl;
        tAlignedSeq.Print(out);
    }

    AlignmentCandidate(const AlignmentCandidate &rhs) {
        *this = rhs;
    }

    DNALength GenomicTBegin() {
        return tAlignedSeqPos + tPos;
    }

    DNALength GenomicTEnd() {
        return tAlignedSeqPos + tPos + TEnd();
    }

    void GetQIntervalOnForwardStrand(int &qStart, int &qEnd) {
        GetQInterval(qStart, qEnd, true);
    }

    void GetQInterval(int &qStart, int &qEnd, bool useForwardStrand=false) {
        qStart = qEnd = 0;
        if (blocks.size() == 0) { return; }
        qStart = blocks[0].qPos + qAlignedSeqPos;
        qEnd   = QEnd() + qAlignedSeqPos;
        if (useForwardStrand and qStrand == 1) {
            int forQEnd, forQStart;
            forQStart = qLength - qEnd;
            forQEnd   = qLength - qStart;
            qStart    = forQStart;
            qEnd      = forQEnd;
        }
    }

    DNALength QAlignEnd() {
        return QEnd() + qPos + qAlignedSeqPos;
    }

    DNALength QAlignStart() {
        return qPos + qAlignedSeqPos;
    }

    DNALength TAlignStart() {
        return tPos + tAlignedSeqPos;
    }

    // Synonyms for T/QStart 
    DNALength GetQBasesToStart() {
        return qPos + qAlignedSeqPos;
    }
    DNALength GetTBasesToStart() {
        return tPos + tAlignedSeqPos;
    }
    
    // ReadOptionalQVs populates the optional QV attributes of
    // AlignmentCandidate with values read from a vector.  
    void ReadOptionalQVs(const std::vector<std::string>& optionalQVs,
                         DNALength start, DNALength length) {
      TryReadingQVs(optionalQVs[0], start, length, &insertionQV);
      TryReadingQVs(optionalQVs[1], start, length, &deletionQV);
      TryReadingQVs(optionalQVs[2], start, length, &substitutionQV);
      TryReadingQVs(optionalQVs[3], start, length, &mergeQV);
      TryReadingQVs(optionalQVs[4], start, length, &substitutionTag);
      TryReadingQVs(optionalQVs[5], start, length, &deletionTag);
    } 
    
    // CopyQVs fills a vector with optional QV attributes.
    void CopyQVs(std::vector<std::string> *optionalQVs) {  
      optionalQVNames.clear();
      optionalQVs->clear();
      optionalQVs->push_back(insertionQV);
      optionalQVNames.push_back("InsertionQV");
      optionalQVs->push_back(deletionQV);
      optionalQVNames.push_back("DeletionQV");
      optionalQVs->push_back(substitutionQV);
      optionalQVNames.push_back("SubstitutionQV");
      optionalQVs->push_back(mergeQV);
      optionalQVNames.push_back("MergeQV");
      optionalQVs->push_back(substitutionTag);
      optionalQVNames.push_back("SubstitutionTag");
      optionalQVs->push_back(deletionTag);
      optionalQVNames.push_back("DeletionTag");
    }

    void AppendAlignment(AlignmentCandidate &next) {

        //
        // If the next alignment is empty, just return now.
        //
        if (next.blocks.size() == 0) {
            return;
        }
        //
        // It is necessary to determine how much after the first alignment
        // the second alignment starts, in order to 
        assert(GetTBasesToStart() <= next.GetTBasesToStart());
        assert(GetQBasesToStart() <= next.GetQBasesToStart());
        assert(GetTBasesToStart() + TEnd() <= next.GetTBasesToStart());
        assert(GetQBasesToStart() + QEnd() <= next.GetQBasesToStart());

        //
        // qOffset is the offset of the frame of reference for the 'next'
        // alignment relative to this one.  If the frame of reference is
        // the same (qOffset == 0 and tOffset == 0), then the alignment
        // blocks may simply be appended.  If it is non zero, then the
        // location in the query or target where the alignment has started
        // is different, and the values of qPos or tPos need to be
        // adjusted accordingly.

        DNALength qOffset = next.qPos + next.qAlignedSeqPos - qPos - qAlignedSeqPos;
        DNALength tOffset = next.tPos + next.tAlignedSeqPos - tPos - tAlignedSeqPos;
        DNALength origQEnd = QEnd();
        DNALength origTEnd = TEnd();

        /*    DNALength tGap = next.GetTBasesToStart() + next.blocks[0].tPos - (GetTBasesToStart() + TEnd());
              DNALength qGap = next.GetQBasesToStart() + next.blocks[0].qPos - (GetQBasesToStart() + QEnd());
              */
        DNALength tGap = next.GetTBasesToStart() - (GetTBasesToStart() + TEnd());
        DNALength qGap = next.GetQBasesToStart() - (GetQBasesToStart() + QEnd());

        //

        if ( gaps.size() > 0 and next.gaps.size() > 0 ) {

            //
            // Determine the gap between the two alignments in order to assign
            // the gap between them.
            DNALength commonGap = 0;
            DNALength gapDiff = 0;

            if (tGap >= qGap) {
                gapDiff   = tGap - qGap;
                commonGap = tGap - gapDiff;
                tGap      = gapDiff;
                qGap      = 0;
            }
            else {
                gapDiff   = qGap - tGap;
                commonGap = qGap - gapDiff;
                qGap      = gapDiff;
                tGap      = 0;
            }

            if (commonGap > 0) {
                // There is some portion of sequence between the two alignments
                // that is a common gap. Add a block representing this.

                blasr::Block block;
                block.qPos = origQEnd;
                block.tPos = origTEnd;
                block.length = commonGap;
                blocks.push_back(block);
                blasr::GapList emptyGap;
                gaps.push_back(emptyGap);
            }

            blasr::GapList endGapList;
            //
            // When gapDiff == 0, an empty list is appended.
            //
            if (next.gaps.size() > 0) {
                //
                // The first gap in the next alignment is already handled by
                // the gap created with the endGap.  So get rid of the first
                // gaps. 
                //        next.gaps[0].resize(0);
                AppendAlignmentGaps(next);
            }
        }

        //
        // Finally append all the blocks.
        //
        AppendAlignmentBlocks(next, qOffset, tOffset);

    }

    void FreeSubsequences() {
        if (tIsSubstring == false) {
            tAlignedSeq.Free();
        }
        if (qIsSubstring == false) {
            qAlignedSeq.Free();
        }
    }

    void ReassignTSequence(DNASequence &newSeq) {
        ReassignSequence(tAlignedSeq, tIsSubstring, newSeq);
    }

    template<typename T_Sequence>
        void ReassignQSequence(T_Sequence &newSeq) {
            ReassignSequence(qAlignedSeq, qIsSubstring, newSeq);
        }

    ~AlignmentCandidate() {
        qAlignedSeq.Free();
        tAlignedSeq.Free();
    }
};
//
// Define a default alignment candidate for aligning quality-sequence
// to a reference without quality.
//
typedef AlignmentCandidate<DNASequence,FASTQSequence> T_AlignmentCandidate;

class SortAlignmentPointersByScore {
public:
    /// Sort AlignmentCandidate pointer by score, then by target position.
    int operator()(T_AlignmentCandidate *lhs, T_AlignmentCandidate* rhs);
};

class SortAlignmentPointersByMapQV {
public:
    /// Sort AlignmentCandidate pointer by mapQV, then by target position.
    int operator()(T_AlignmentCandidate *lhs, T_AlignmentCandidate* rhs);
};

#endif // _ALIGNMENT_ALIGNMENT_CANDIDATE_HPP_
