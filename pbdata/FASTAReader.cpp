#include <stdlib.h>
#include <string>
#include <iostream>
#include <vector>
#include <limits.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/fcntl.h>
#include <sys/stat.h>

#include "Enumerations.h"
#include "NucConversion.hpp"
#include "FASTASequence.hpp"
#include "FASTAReader.hpp"

using namespace std;

void FASTAReader::SetFileSize() {
    //
    // Seek operation returns the amount seeked forwared in bytes.
    // This is the size used to map (which is rounded up by the 
    // system).
    //
    fileSize = lseek(fileDes, 0, SEEK_END);
    lseek(fileDes, 0, SEEK_SET);
}

void FASTAReader::Init() {
    padding = 0;
    fileDes = -1;
    fileSize = 0;
    endOfReadDelim = '>';
    readStartDelim = '>';
    doToUpper = false;
    convMat = PreserveCase;
    computeMD5 = false;
    filePtr = NULL;
    curPos = 0;
}

FASTAReader::FASTAReader() {
    Init();
}

FASTAReader::FASTAReader(string &fileName) {
    Init(); // initialze defaults.
    Init(fileName); // open file.
    padding = 0;
    endOfReadDelim = '>';
    readStartDelim = '>';
}

void FASTAReader::SetSpacePadding(int _padding) {
    assert(_padding >= 0);
    padding = _padding;
}

void FASTAReader::SetToUpper() {
    doToUpper = true;
    convMat   = AllToUpper;
}

//
// Synonym for Init() for consistency.
//
int FASTAReader::Initialize(string &seqInName) {
    return Init(seqInName);
}

int FASTAReader::Init(string &seqInName, int passive) {

    // Check if file is exist and not empty (length of the file is non-zero)
    struct stat st;
    if(stat(seqInName.c_str(), &st) != 0) {
        cerr << "FASTA file " << seqInName << " doesn't exist" << endl;
        exit(1);
    }
    if (st.st_size == 0) {
        cerr << "FASTA file " << seqInName << " is empty" << endl;
        exit(1);
    }

    fileDes = open(seqInName.c_str(), O_RDONLY);
    padding = 0;
    if (fileDes == -1) {
        if (passive) {
            return 0;
        }
        else {
            cout << "Could not open FASTA file " << seqInName << endl;
            exit(1);
        }
    }
    SetFileSize();
    filePtr = (char*) mmap(0, fileSize, PROT_READ, MAP_PRIVATE, fileDes, 0);
    if (filePtr == MAP_FAILED) {
        cout << "ERROR, Fail to load FASTA file " << seqInName 
             << " to virtual memory." << endl;
        exit(1);
    }
    curPos = 0;
    return 1;
}

void FASTAReader::AdvanceToTitleStart(GenomeLength &p, char delim) {
    while (p < fileSize and 
            filePtr[p] != delim) { 
        p++;
    }
}


void FASTAReader::CheckValidTitleStart(GenomeLength &p, char delim) {
    if (p >= fileSize or filePtr[p] != delim) {
        cout << "ERROR, FASTA entry must begin with \"" << delim << "\"" << endl;
        exit(1);
    }
}

GenomeLength FASTAReader::ReadAllSequencesIntoOne(FASTASequence &seq, SequenceIndexDatabase<FASTASequence> *seqDBPtr) {
    seq.Free();
    GenomeLength p = curPos;
    AdvanceToTitleStart(p);
    CheckValidTitleStart(p);
    ReadTitle(p, seq); 
     
    if (seq.title == NULL) {
        cout << "ERROR, sequence must have a nonempty title." << endl;
        exit(1);
    }
    if (seqDBPtr != NULL) {
        seqDBPtr->growableName.push_back(seq.title);
    }
    GenomeLength seqLength;
    seqLength = fileSize - p;
    GenomeLength memorySize = seqLength+padding+1;

    if (memorySize > UINT_MAX) {
        cout << "ERROR! Reading fasta files greater than 4Gbytes is not supported." << endl;
        exit(1);
    }
    seq.Resize(memorySize);
    GenomeLength i;
    i = 0L;
    for (; p < fileSize; p++, i++ ) {
        seq.seq[i] = filePtr[p];
    }
    i = p = 0;
    while (p < seqLength) {
        //
        // If this is the beginning of another read, add an 'N' 
        // to delineate spaces between reads.
        //

        while (p < seqLength and
                (seq.seq[p] == ' ' or
                 seq.seq[p] == '\n' or 
                 seq.seq[p] == '\t' or
                 seq.seq[p] == '\r')) {
            p++;
        }
        if (p < seqLength and seq.seq[p] == '>') {
            seq.seq[i] = 'N';

            GenomeLength titleStartPos = p+1;
            i++;
            while (p < seqLength and seq.seq[p] != '\n') p++;
            if (seqDBPtr != NULL and p < seqLength) {
                string title;
                GenomeLength tp;
                for (tp = titleStartPos; tp < p; tp++) {
                    title.push_back(seq.seq[tp]);
                }

                seqDBPtr->growableName.push_back(title);
                seqDBPtr->growableSeqStartPos.push_back(i);
                int nSeq = seqDBPtr->growableSeqStartPos.size();
                if (nSeq > 1 and computeMD5) {
                    string md5Str;
                    MakeMD5((const char*) &seq.seq[seqDBPtr->growableSeqStartPos[nSeq-2]],
                            seqDBPtr->growableSeqStartPos[nSeq-1] - seqDBPtr->growableSeqStartPos[nSeq-2] - 1,
                            md5Str);
                    seqDBPtr->md5.push_back(md5Str);
                }
            }
        }
        else if (p < seqLength)  {
            // Otherwise, p may be at whitespace
            // advance past that as well.
            seq.seq[i] = convMat[seq.seq[p]];
            i++;
            p++;
        }
    }
    if (i > UINT_MAX) {
        cout << "ERROR! Sequences greater than 4Gbase are not supported." << endl;
        exit(1);
    }
    //
    // Append an 'N' at the end of the last sequence for consistency
    // between different orderings of reference input. 
    //
    seq.seq[i] = 'N';
    i++;
    seq.length = i;
    // fill padding.
    for (; i < memorySize; i++ ){
        seq.seq[i] = 0;
    }
    seq.deleteOnExit = true;
    if (seqDBPtr != NULL) {
        seqDBPtr->growableSeqStartPos.push_back(seq.length);
        int nSeq = seqDBPtr->growableSeqStartPos.size();
        if (nSeq > 1 and computeMD5) {
            string md5Str;
            MakeMD5((const char*) &seq.seq[seqDBPtr->growableSeqStartPos[nSeq-2]],
                    seqDBPtr->growableSeqStartPos[nSeq-1] - seqDBPtr->growableSeqStartPos[nSeq-2] - 1,
                    md5Str);
            seqDBPtr->md5.push_back(md5Str);
        }
        seqDBPtr->Finalize();
    }
    return seq.length;
}

void FASTAReader::ReadTitle(GenomeLength &p, FASTASequence & seq) {
    char * seqTitle = NULL;
    int seqTitleLen; 
    ReadTitle(p, seqTitle, seqTitleLen);
    seq.CopyTitle(seqTitle, seqTitleLen);
    if (seqTitle) {delete [] seqTitle;}
}

void FASTAReader::ReadTitle(GenomeLength &p, char *&title, int &titleLength) {
    // 
    // Extract the title.  The length of the title does not include the newline.
    //
    p++; // Move past '>'
    curPos = p;
    while (p < fileSize and
            filePtr[p] != '\n') { 
        p++;
    }
    titleLength = p - curPos;
    if (titleLength > 0) {
        if (title) {delete [] title; title = NULL;}
        title = ProtectedNew<char> (titleLength + 1);
        if (title == nullptr) {
            cout << "ERROR, unable to read FASTA file to memory. " << endl; exit(1);
        }
        int t = 0;
        for (p = curPos; p < curPos + titleLength; p++, t++) {
            title[t] = filePtr[p];
        }
        title[titleLength] = '\0';
    }
    else {
        title = NULL;
        titleLength = 0;
    }
}

int FASTAReader::GetNext(FASTASequence &seq) {
    if (curPos == fileSize) {
        return 0;
    }

    seq.Free(); //Free seq before read

    // 
    // Extract the title of the current record.
    //
    GenomeLength p = curPos;

    AdvanceToTitleStart(p);

    // 
    // Make sure there is a '>'
    //
    CheckValidTitleStart(p);

    ReadTitle(p, seq); 

    //
    // Read in the next sequence.
    //

    // Count the length of the sequence.

    GenomeLength seqLength = 0;
    curPos = p;
    char c;
    while (p < fileSize and
            (c = filePtr[p]) != endOfReadDelim) {
        if (c != ' ' and
                c != '\t' and
                c != '\n' and
                c != '\r') {
            seqLength++;
        }
        p++;
    }
    if (seqLength > UINT_MAX) {
        cout << "ERROR! Reading sequences stored in more than 4Gbytes of space is not supported." << endl;
        exit(1);
    }

    seq.length = 0;
    if (seqLength > 0) {
        seq.length = seqLength;
        seq.seq = ProtectedNew <Nucleotide>(seqLength+padding+1);
        p = curPos;
        seq.deleteOnExit = true;
        GenomeLength s = 0;
        while (p < fileSize and
                (c = filePtr[p]) != endOfReadDelim) {
            if (c != ' ' and
                    c != '\t' and
                    c != '\n' and
                    c != '\r') {
                seq.seq[s] = convMat[static_cast<unsigned char>(filePtr[p])];
                s++;
            }
            p++;
        }
        seq.seq[seqLength] = 0;
    }
    curPos = p;

    if (computeMD5) {
        MakeMD5((const char*) &seq.seq, seq.length, curReadMD5);
    }
    return 1;
}
/*
   Advance to the read nSeq forward.

input: nSeq, the number of sequences to skip.
output: 
returns 1 if after advancing nSeq sequences, the file pointer is pointing to 
a new sequence.
0 otherwise. 
A return value of 0 will signal that the file is done being processed if it is 
iterting over reads.
*/
int FASTAReader::Advance(int nSeq) {

    int nAdvanced = 0;
    GenomeLength p = curPos;
    // base case -- it's always ok to advance 0
    if (nSeq == 0) { return 1; }

    // Make sure the advance starts at 
    // the beginning of a sequence.
    while(p < fileSize and filePtr[p] != endOfReadDelim) p++;
    // No sequence was found, coudln't advance.
    if (p >= fileSize) { return 0; }

    p++;
    nAdvanced = 1;
    while (nAdvanced <= nSeq and p < fileSize) {
        if (filePtr[p] == endOfReadDelim) {
            if (nAdvanced == nSeq) {
                //
                // Want to end with the reader on a '>', so return before
                // hitting the rest of the loop.
                //
                curPos = p;
                return 1;
            }
            else {
                nAdvanced++; 
            }
        }
        p++;
    }
    curPos = p;
    return 0;
}

int FASTAReader::CriticalGetNext(FASTASequence &seq) {
    if (!GetNext(seq)) {
        cout << "Could not read a sequence." << endl;
        exit(1);
    }
    return 1;
}

int FASTAReader::ConcatenateNext(FASTASequence &cur) {
    FASTASequence next;
    int retVal;	
    if ((retVal = GetNext(next))) {
        next.CleanupASCII();
        cur.Concatenate((Nucleotide*) "N");
        cur.Concatenate(next);	
    }
    next.Free();
    return retVal;
}

void FASTAReader::Close() {
    if (fileDes == -1) {
        cout << "ERROR, calling close on an uninitialized fasta reader" << endl;
        exit(1);
    }
    else {
        munmap(filePtr, fileSize);
        close(fileDes);
        fileDes = -1;
    }
}

void FASTAReader::ReadAllSequences(vector<FASTASequence> &sequences) {
    //
    // Step 1, compute the number of reads in the file.
    // 

    GenomeLength p;
    p = 0;
    int nSeq = 0;
    while (p < fileSize) { 
        if (filePtr[p] == '>') {
            ++nSeq;
        }
        p++;
    }

    p = 0;
    sequences.resize(nSeq);
    int curSeq = 0;
    while(GetNext(sequences[curSeq])) {
        ++curSeq;
    }
}
